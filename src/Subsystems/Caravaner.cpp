#include "Caravaner.h"

#include "../GameLogic.h"
#include "GameNavigation.h"
#include "../Components/CarawanComponents.h"
#include "CaravanerEvents.h"

Caravaner::Caravaner(Context* ctx)
    : Object(ctx),
      mRunning(true),
      mGameOver(false),
      mSelectionMode(false),
      uiInitialize(false)
{
    mGameLogic = GetSubsystem<GameLogic>();
    SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(Caravaner,HandleUpdate));
    SubscribeToEvent(E_ANIMATIONTRIGGER,URHO3D_HANDLER(Caravaner,HandleSoundTrigger));
    SubscribeToEvent(E_CARAVAN_FINISHED,URHO3D_HANDLER(Caravaner,HandleFinish));

}

void Caravaner::CreateUI()
{
    auto mUiRoot = GetSubsystem<UI>()->GetRoot();
    auto* cache = GetSubsystem<ResourceCache>();
    auto* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    mUiRoot->SetDefaultStyle(style);


    Window* mWindow = new Window(context_);
    mUiRoot->AddChild(mWindow);

    // Set Window size and layout settings
    mWindow->SetMinWidth(800);
    mWindow->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    mWindow->SetAlignment(HA_CENTER, VA_BOTTOM);
    mWindow->SetName("progress");

    // Create Window 'titlebar' container
    auto* titleBar = new UIElement(context_);
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    // Create the Window title Text
    mProgressbar = new ProgressBar(context_);
    mProgressbar->SetRange(100);
    mProgressbar->SetValue(0);
    mProgressbar->SetMinSize(600,55);

    titleBar->AddChild(mProgressbar);


    mWindow->AddChild(titleBar);

    // Apply styles
    mWindow->SetStyleAuto();
    mProgressbar->SetStyleAuto();

    uiInitialize = true;
}

void Caravaner::InitLevel(String sceneName)
{
    if (!uiInitialize) {
        CreateUI();
    }
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

    mEndParticle = mScene->GetChild("EndParticle",true);

    //GetComponentsRecursiver<Guy>(mGuys);

    SetSelectionMode(false);
}

void Caravaner::StartLevel()
{
    mRunning=true;
    mGameOver=false;
    mWon = false;
    mCart->SetMoving(true);
    mScene->GetComponents<Guy>(mGuys,true);
    mCart->status.livePower=100.0f;
    mTargetsInUse.Clear();
}

Node* Caravaner::GetNearestGuyCart(Node *from,float maxDist)
{
    float nearest = 100000000;
    float distSqr = maxDist * maxDist;
    Node* node = nullptr;

    Vector3 pos = from->GetWorldPosition();

    for(int i=0;i<mGuys.Size();i++){
       Guy* g = mGuys[i];
        if (g->GetNode()==from || g->mGuyType!=Guy::GT_Gatherer) {
           continue;
       }

       float dist = (mGuys[i]->GetNode()->GetWorldPosition() - pos).LengthSquared();
       if (dist < distSqr && dist < nearest){
           node = mGuys[i]->GetNode();
           nearest = dist;
       }
    }
    return node;
}

void Caravaner::HandleFinish(StringHash eventType, VariantMap &data)
{
    mRunning = false;
    mGameOver = true;
    mWon = true;
    GameLogic* gl = GetSubsystem<GameLogic>();
    gl->PlaySound("applause.ogg");

}

void Caravaner::RemoveGuy(Guy *guy,bool playSound)
{
    mGuys.Remove(guy);
    guy->GetNode()->Remove();
    if (playSound) {
        GameLogic* gl = GetSubsystem<GameLogic>();
        gl->PlaySound("die.ogg");
    }
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
            col = Color::RED;
        else if (power < 50)
            col = Color::MAGENTA;

        gl->SetUIText("Cart Condition:"+String(power)+"    (green=>attack bandits/mills,orange=>trees)      Music by Kevin MacLeod", col );
        if (mProgressbar) {
            mProgressbar->SetValue(power);
            mProgressbar->SetColor(col);
        }

        if (mCart->status.livePower <= 0){
            mGameOver = true;
            mRunning = false;
            gl->SetUIText("You lost! Shame on you  ;)      Restart by pressing ENTER");

            ParticleEmitter* em = mEndParticle->GetComponent<ParticleEmitter>(true);
            if (em){
                mEndParticle->SetWorldPosition(mCart->GetNode()->GetWorldPosition());
                em->SetEnabled(true);
            }
            mCart->GetNode()->Remove();
            gl->PlaySound("crash.ogg",0.3f);
            return;
        }


        if (input->GetMouseButtonPress(MOUSEB_LEFT) || input->GetNumTouches()){
            RigidBody* hitbody;
            Vector3 hitPos;
            // TODO?
            String tag = mSelectionMode ? "" : "";
            if (gl->MouseOrTouchPhysicsRaycast(2500.0f,hitPos,hitbody)){
                int a=0;
                Node* n = hitbody->GetNode();
                String name = n->GetName();
            //    URHO3D_LOGINFOF("HIT: %s",name.CString());

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
        if (mCart){
            mCameraNode->LookAt(mCart->GetNode()->GetWorldPosition());
            //mCameraNode->Rotate(Quaternion(0,0,0));
        }
        mLastCartPos = cartPos;

        for (Guy* guy : mGuys) {
            guy->Tick(dt);
        }
    } else {
        if (mWon) {
            GameLogic* gl = GetSubsystem<GameLogic>();
            gl->SetUIText("Hooray! You made it to the Finish!!! You and the cargo you carried saved us all! ;)  Thx for playing...   Restart with Enter/Touch");
        }
    }
}

void Caravaner::HandleSoundTrigger(StringHash eventType, VariantMap &data)
{
    using namespace AnimationTrigger;
    Node* node = static_cast<Node*>(data[P_NODE].GetPtr());
    String name = data[P_NAME].GetString();
   // URHO3D_LOGINFOF("SOUNDTRIGGER Node:%s Name:%s",node->GetName().CString(),name.CString());
    GameLogic* gl = GetSubsystem<GameLogic>();
}

void Caravaner::CheckSelectedGuyWork(Node* n)
{
    if (mSelectedGuy->mGuyType == Guy::GT_Gatherer && n->HasTag("soldier_only")){
        return;
    }
    if (mSelectedGuy->mGuyType == Guy::GT_Soldier && !n->HasTag("soldier_only")){
        return;
    }

    if (n->HasTag("bandit")){
        mSelectedGuy->Select(false);
        SetSelectionMode(false);
        mSelectedGuy->SetRequestWorkTarget(n);
        mSelectedGuy->RequestWorkMode(Guy::WM_ATTACK,true);

        mTargetsInUse.Insert(n);
    }
    else if (n->HasTag("resource")){
        mSelectedGuy->Select(false);
        SetSelectionMode(false);
        mSelectedGuy->SetRequestWorkTarget(n);
        mSelectedGuy->RequestWorkMode(Guy::WM_PickupWood,true);

        mTargetsInUse.Insert(n);
    }
}

void Caravaner::SetSelectionMode(bool setit,bool gatherer){
    mSelectionMode = setit;

    PODVector<Node*> selectors;
    mScene->GetChildrenWithTag(selectors,"selector",true);

    if (setit){
        for (Node* node : selectors){
            if (mTargetsInUse.Contains(node) || mTargetsInUse.Contains(node->GetParent())) {
                node->SetEnabled(false);
                continue;
            }

            if (gatherer && node->HasTag("soldier_only")) {
                node->SetEnabled(false);
                continue;
            }
            else if (!gatherer && !node->HasTag("soldier_only")) {
                node->SetEnabled(false);
                continue;
            }

            node->SetEnabled(true);
        }

        if ( !gatherer ){
            for (Guy* guy : mGuys){
                if (guy->mGuyType == Guy::GT_Bandit){
                    guy->Select(true);
                }
            }
        }

    } else {
        for (Node* node : selectors){
            node->SetEnabled(false);
        }
        if ( !gatherer ){
            for (Guy* guy : mGuys){
                if (guy->mGuyType == Guy::GT_Bandit){
                    guy->Select(false);
                }
            }
        }

    }
}
