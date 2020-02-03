#include "GameNavigation.h"

#include <project_options.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Drawable.h>
#include <Urho3D/Input/Input.h>

#include "../GameLogic.h"

GameNavigation::GameNavigation(Context *ctx)
    : Object(ctx),
      mDrawDebug(false)
#ifdef GAME_ENABLE_DEBUG_TOOLS
    ,tempTestMode(false)
#endif
{
    SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(GameNavigation,HandleUpdate));
#ifdef GAME_ENABLE_DEBUG_TOOLS
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(GameNavigation,HandlePostRender));
#endif
}


void GameNavigation::Init()
{
    mScene = GetSubsystem<Scene>();
    mNavMesh = mScene->GetDerivedComponent<NavigationMesh>(true);

    if (!mNavMesh) {
        URHO3D_LOGERROR("Couldnot locate navmesh in scene!");
        return;
    }

    mNavMesh->Build();

    mCrowdManager = mScene->GetComponent<CrowdManager>(true);
    if (mCrowdManager){
        // -- obstacle avoidance -- ( not know what this is doing)
        CrowdObstacleAvoidanceParams params = mCrowdManager->GetObstacleAvoidanceParams(0);
        // Set the params to "High (66)" setting
        params.velBias = 0.5f;
        params.adaptiveDivs = 7;
        params.adaptiveRings = 3;
        params.adaptiveDepth = 3;
        mCrowdManager->SetObstacleAvoidanceParams(0, params);

        // -- subscribe to crowdmanager-events --
        // Subscribe HandleCrowdAgentFailure() function for resolving invalidation issues with agents, during which we
        // use a larger extents for finding a point on the navmesh to fix the agent's position
        SubscribeToEvent(E_CROWD_AGENT_FAILURE, URHO3D_HANDLER(GameNavigation, HandleCrowdAgentFailure));

        // Subscribe HandleCrowdAgentReposition() function for controlling the animation
        SubscribeToEvent(E_CROWD_AGENT_REPOSITION, URHO3D_HANDLER(GameNavigation, HandleCrowdAgentReposition));

        // Subscribe HandleCrowdAgentFormation() function for positioning agent into a formation
        SubscribeToEvent(E_CROWD_AGENT_FORMATION, URHO3D_HANDLER(GameNavigation, HandleCrowdAgentFormation));

    }

}


void GameNavigation::HandleUpdate(StringHash eventType, VariantMap &data)
{
#ifdef GAME_ENABLE_DEBUG_TOOLS
    if (tempTestMode) {
        Input* input = GetSubsystem<Input>();
        GameLogic* gl = GetSubsystem<GameLogic>();

        if (input->GetMouseButtonPress(MOUSEB_LEFT)){
            Drawable* hitDrawable;
            if (gl->MouseRaycast(2500.0f,tempFrom,hitDrawable)){
            }
        }
        if (input->GetMouseButtonPress(MOUSEB_RIGHT)){
            Drawable* hitDrawable;
            if (gl->MouseRaycast(2500.0f,tempTo,hitDrawable)){

                FindPath(tempFrom,tempTo,tempPathPoints);
            }
        }

        if (input->GetMouseButtonPress(MOUSEB_MIDDLE)){
            Drawable* hitDrawable;
            if (gl->MouseRaycast(2500.0f,tempMoveTo,hitDrawable)){
                // move all CrowdAgents of the scene ( you can specify a node from which all crowdagents will be targeted)
                MoveTo(tempMoveTo);
            }
        }

        if (input->GetNumTouches()>0){
            Drawable* hitDrawable;
            if (gl->TouchRaycast(0,2500.0f,tempMoveTo,hitDrawable)){
                // move all CrowdAgents of the scene ( you can specify a node from which all crowdagents will be targeted)
                MoveTo(tempMoveTo);
            }
        }

    }
#endif
}

void GameNavigation::HandlePostRender(StringHash eventType, VariantMap &data)
{
#ifdef GAME_ENABLE_DEBUG_TOOLS
    if (mDrawDebug && mNavMesh) {
        mNavMesh->DrawDebugGeometry(false);

        Scene* scene_ = GetSubsystem<Scene>();
        DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();

        debug->AddSphere(Sphere(tempFrom,3.0f),Color::RED);
        debug->AddSphere(Sphere(tempTo,3.0f),Color::GREEN);

        if (tempPathPoints.Size())
        {
            // Visualize the current calculated path

            // Draw the path with a small upward bias so that it does not clip into the surfaces
            Vector3 bias(0.0f, 0.05f, 0.0f);
//            debug->AddLine(jackNode_->GetPosition() + bias, currentPath_[0] + bias, Color(1.0f, 1.0f, 1.0f));

            if (tempPathPoints.Size() > 1)
            {
                for (unsigned i = 0; i < tempPathPoints.Size() - 1; ++i)
                    debug->AddLine(tempPathPoints[i] + bias, tempPathPoints[i + 1] + bias, Color(1.0f, 1.0f, 1.0f));
            }
        }

        if (mCrowdManager){
            mCrowdManager->DrawDebugGeometry(true);
        }
    }
#endif
}


void GameNavigation::HandleCrowdAgentFailure(StringHash eventType, VariantMap& eventData)
{
    using namespace CrowdAgentFailure;

    auto* node = static_cast<Node*>(eventData[P_NODE].GetPtr());
    auto agentState = (CrowdAgentState)eventData[P_CROWD_AGENT_STATE].GetInt();

    // If the agent's state is invalid, likely from spawning on the side of a box, find a point in a larger area
    if (agentState == CA_STATE_INVALID)
    {
        // Get a point on the navmesh using more generous extents
        Vector3 newPos = mNavMesh->FindNearestPoint(node->GetPosition(), Vector3(5.0f, 5.0f, 5.0f));
        // Set the new node position, CrowdAgent component will automatically reset the state of the agent
        node->SetPosition(newPos);
    }
}

void GameNavigation::HandleCrowdAgentReposition(StringHash eventType, VariantMap& eventData)
{
    using namespace CrowdAgentReposition;

    auto* node = static_cast<Node*>(eventData[P_NODE].GetPtr());
    auto* agent = static_cast<CrowdAgent*>(eventData[P_CROWD_AGENT].GetPtr());
    Vector3 velocity = eventData[P_VELOCITY].GetVector3();
    float timeStep = eventData[P_TIMESTEP].GetFloat();
}

void GameNavigation::HandleCrowdAgentFormation(StringHash eventType, VariantMap& eventData)
{    using namespace CrowdAgentFormation;

     unsigned index = eventData[P_INDEX].GetUInt();
     unsigned size = eventData[P_SIZE].GetUInt();
     Vector3 position = eventData[P_POSITION].GetVector3();

     // The first agent will always move to the exact position, all other agents will select a random point nearby
     if (index)
     {
         auto* crowdManager = static_cast<CrowdManager*>(GetEventSender());
         auto* agent = static_cast<CrowdAgent*>(eventData[P_CROWD_AGENT].GetPtr());
         eventData[P_POSITION] = crowdManager->GetRandomPointInCircle(position, agent->GetRadius(), agent->GetQueryFilterType());
     }
}

bool GameNavigation::FindPath(Vector3 from_, Vector3 to_, PODVector<Vector3>& dest)
{
    if (!mNavMesh) return false;

    Vector3 from = mNavMesh->FindNearestPoint(from_);
    Vector3 to = mNavMesh->FindNearestPoint(to_);
    mNavMesh->FindPath(dest,from,to);
    return dest.Size() > 0;
}

Vector3 GameNavigation::MoveTo(Vector3 gotoPosition,Node* node)
{
    if (mCrowdManager){
        Vector3 posOnNavmesh = mNavMesh->FindNearestPoint(gotoPosition,Vector3(10,10,10));
        mCrowdManager->SetCrowdTarget(posOnNavmesh,node);
        return posOnNavmesh;
    }
    return Vector3::ZERO;
}

