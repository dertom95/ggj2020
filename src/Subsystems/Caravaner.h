#pragma once

#include <project_options.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>





class GameNavigation;
class GameLogic;
class Cart;
class Guy;

using namespace Urho3D;

namespace Urho3D {
    class Scene;
}


class Caravaner : public Object
{
    URHO3D_OBJECT(Caravaner,Object)
public:
    Caravaner(Context* ctx);

    void InitLevel(String name);
    void StartLevel();
    void HandleUpdate(StringHash eventType,VariantMap& data);
    inline void SetRunning(bool running) { mRunning = running; }
    inline bool IsRunning() { return mRunning; }

    void SetSelectionMode(bool setit,bool gathererOnly=false);
private:
    void CheckSelectedGuyWork(Node* n);

    SharedPtr<Scene> mScene;
    SharedPtr<GameLogic> mGameLogic;
    SharedPtr<Cart> mCart;

    HashSet<Node*> mTargetsInUse;



    PODVector<Guy*> mGuys;

    bool mRunning;

    SharedPtr<Node> mPathMaster;
    PODVector<Node*> mPath;
    Node* mCurrentPathElem;

    bool mSelectionMode;


    Node* mCameraNode;
    Vector3 mLastCartPos;
    Vector3 mCamTarget;

    SharedPtr<Guy> mSelectedGuy;


};
