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
    class ParticleEmitter;
    class ProgressBar;
}


class Caravaner : public Object
{
    URHO3D_OBJECT(Caravaner,Object)
public:
    Caravaner(Context* ctx);

    void InitLevel(String name);
    void StartLevel();
    void HandleUpdate(StringHash eventType,VariantMap& data);
    void HandleSoundTrigger(StringHash eventType, VariantMap &data);
    void HandleFinish(StringHash eventType,VariantMap& data);

    inline void SetRunning(bool running) { mRunning = running; }
    inline bool IsRunning() { return mRunning; }
    inline bool IsGameOver() { return mGameOver; }
    void SetSelectionMode(bool setit,bool gathererOnly=false);
    Node* GetNearestGuyCart(Node* from,float maxDist);
    void RemoveGuy(Guy* guy,bool playSound=false);
    void DeselectAllGuys();

    Vector3 mCamTarget;
    SharedPtr<Guy> mSelectedGuy;
private:
    void CheckSelectedGuyWork(Node* n);
    void CreateUI();
    SharedPtr<ProgressBar> mProgressbar;
    SharedPtr<Scene> mScene;
    SharedPtr<GameLogic> mGameLogic;
    SharedPtr<Cart> mCart;

    HashSet<Node*> mTargetsInUse;

    SharedPtr<Node> mEndParticle;

    PODVector<Guy*> mGuys;

    bool mRunning;
    bool mGameOver;
    bool mWon;

    bool uiInitialize;

    SharedPtr<Node> mPathMaster;
    PODVector<Node*> mPath;
    Node* mCurrentPathElem;

    bool mSelectionMode;


    Node* mCameraNode;
    Vector3 mLastCartPos;





};
