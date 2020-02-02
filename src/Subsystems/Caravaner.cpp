#include "Caravaner.h"

#include "../GameLogic.h"
#include "GameNavigation.h"
#include "../Components/CarawanComponents.h"
#include "CaravanerEvents.h"

Caravaner::Caravaner(Context* ctx)
    : Object(ctx),
      mRunning(true),
      mSelectionMode(false)
{
    mGameLogic = GetSubsystem<GameLogic>();
    SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(Caravaner,HandleUpdate));
}

void Caravaner::InitLevel(String sceneName)
{
    // Create scene subsystem components
    mGameLogic->LoadFromFile(sceneName);
    GameNavigation* gameNavigation = GetSubsystem<GameNavigation>();
    gameNavigation->Init();

    mScene = GetSubsystem<Scene>();

    mCameraNode = mScene->GetChild("Camera",true);

    if (!mCameraNode) {
        URHO3D_LOGERRORF("NO CAMERA NODE in Scene: %s",sceneName.CString());
    }
    mGameLogic->SetCameraNode( mCameraNode );

    PODVector<Node*> navonlyNodes;
    mScene->GetNodesWithTag(navonlyNodes,"navonly");
    for (Node* node : navonlyNodes){
        node->Remove();
    }
    mCart = mScene->GetComponent<Cart>(true);
    context_->RegisterSubsystem(mCart);

    PODVector<Node*> nodes;
    if (!mScene->GetNodesWithTag(nodes,"path_master")){
        URHO3D_LOGERROR("NO path_master");
        return;
    }
    mPathMaster = nodes[0];

    int size = mPathMaster->GetChildren().Size();
    for (int i=0; i < size; i++){
        Node* pathElem = mPathMaster->GetChild(String("p")+String(i));
        if (pathElem){
            mPath.Push(pathElem);
        }
    }

    if (mPath.Size()==0){
        URHO3D_LOGERROR("NO PATH!");
        return;
    }

    mCart->SetPath(mPath);
    mLastCartPos = mCart->GetNode()->GetWorldPosition();
    mCamTarget = mCameraNode->GetWorldPosition();

    //GetComponentsRecursiver<Guy>(mGuys);

    SetSelectionMode(false);
}

void Caravaner::StartLevel()
{
    mCart->SetMoving(true);
    mScene->GetComponents<Guy>(mGuys,true);
    mCart->status.livePower=100.0f;
}

void Caravaner::HandleUpdate(StringHash eventType, VariantMap &data)
{
    using namespace Update;
    float dt = data[P_TIMESTEP].GetFloat();

    if (mRunning){
        mCart->Tick(dt);



        Input* input = GetSubsystem<Input>();
        GameLogic* gl = GetSubsystem<GameLogic>();

        int power = CeilToInt(mCart->status.livePower);
        Color col = Color::WHITE;
        if (power < 30)
            col = Color::MAGENTA;
        else if (power < 10)
            col = Color::RED;

        gl->SetUIText("Cart Condition:"+String(power), col );

        if (input->GetMouseButtonPress(MOUSEB_LEFT)){
            RigidBody* hitbody;
            Vector3 hitPos;
            // TODO?
            String tag = mSelectionMode ? "" : "";
            if (gl->MouseOrTouchPhysicsRaycast(2500.0f,hitPos,hitbody)){
                int a=0;
                Node* n = hitbody->GetNode();
                String name = n->GetName();
                URHO3D_LOGINFOF("HIT: %s",name.CString());

                Guy* guy = n->GetComponent<Guy>();
                if (guy && guy->mWorkmode==Guy::WM_Idle && ( guy->mGuyType==Guy::GT_Gatherer || guy->mGuyType==Guy::GT_Soldier)){
                    SetSelectionMode(true,guy->mGuyType==Guy::GT_Gatherer);
                    if (mSelectedGuy){
                        mSelectedGuy->Select(false);
                    }
                    mSelectedGuy = guy;
                    mSelectedGuy->Select(true);
                }
                else if (mSelectionMode && mSelectedGuy){
                    CheckSelectedGuyWork(n);
                }
            } else {
                SetSelectionMode(false);
                if (mSelectedGuy){
                    mSelectedGuy->Select(false);
                }
            }
        }

        Vector3 cartPos = mCart->GetNode()->GetWorldPosition();
        Vector3 cartDelta = cartPos - mLastCartPos;
        cartDelta.y_ = 0;
        mCamTarget += cartDelta;

        Vector3 newCampos = mCameraNode->GetWorldPosition().Lerp(mCamTarget,0.05);
        mCameraNode->SetWorldPosition(newCampos);
        mLastCartPos = cartPos;

        for (Guy* guy : mGuys) {
            guy->Tick(dt);
        }
    }
}

void Caravaner::CheckSelectedGuyWork(Node* n)
{
    if (mSelectedGuy->mGuyType == Guy::GT_Gatherer && n->HasTag("soldier_only")){
        return;
    }

    if (n->HasTag("resource")){
        mSelectedGuy->Select(false);
        SetSelectionMode(false);
        mSelectedGuy->SetRequestWorkTarget(n);
        mSelectedGuy->RequestWorkMode(Guy::WM_PickupWood,true);

        mTargetsInUse.Insert(n);
    }
}

void Caravaner::SetSelectionMode(bool setit,bool gathererOnly){
    mSelectionMode = setit;

    PODVector<Node*> selectors;
    mScene->GetChildrenWithTag(selectors,"selector",true);

    if (setit){
        for (Node* node : selectors){
            if (mTargetsInUse.Contains(node) || mTargetsInUse.Contains(node->GetParent())) {
                node->SetEnabled(false);
                continue;
            }

            if (gathererOnly && node->HasTag("soldier_only")) {
                node->SetEnabled(false);
                continue;
            }

            node->SetEnabled(true);
        }
    } else {
        for (Node* node : selectors){
            node->SetEnabled(false);
        }
    }
}
