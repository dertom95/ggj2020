#include "Caravaner.h"

#include "../GameLogic.h"
#include "GameNavigation.h"

Caravaner::Caravaner(Context* ctx)
    : Object(ctx)
{
    mGameLogic = GetSubsystem<GameLogic>();
}

void Caravaner::InitLevel(String sceneName)
{
    // Create scene subsystem components
    mGameLogic->LoadFromFile(sceneName);
    GameNavigation* gameNavigation = GetSubsystem<GameNavigation>();
    gameNavigation->Init();

    mScene = GetSubsystem<Scene>();

    Node* camNode = mScene->GetChild("Camera",true);

    if (!camNode) {
        URHO3D_LOGERRORF("NO CAMERA NODE in Scene: %s",sceneName.CString());
    }
    mGameLogic->SetCameraNode( camNode );

    PODVector<Node*> navonlyNodes;
    mScene->GetNodesWithTag(navonlyNodes,"navonly");
    for (Node* node : navonlyNodes){
        node->Remove();
    }
}

