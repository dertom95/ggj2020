//
// Copyright (c) 2008-2019 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "CarawanComponents.h"

#include <Urho3D/Urho3DAll.h>
#include "../Subsystems/GameNavigation.h"
#include "../Subsystems/CaravanerEvents.h"

const float DEFAULT_GUY_TIMER = 0.5f;
const float DEFAULT_GUY_TIMER_MAX = 1.0f;

const char* guyTypeNames[] =
{
    "Soldier",
    "Gatherer",
    "King",
    "Bandit",
    "",
    nullptr
};

Guy::Guy(Context* ctx)
    : LogicComponent(ctx),
      mWorkmode(WM_Idle),
      mRequestedWorkmode(WM_Idle),
      mSelected(false),
      mTimer(0.0f)

{
}

void Guy::RegisterObject(Context* context)
{
    context->RegisterFactory<Guy>("Guy");

    URHO3D_ENUM_ACCESSOR_ATTRIBUTE("Guy Type", GetGuyType, SetGuyType, GuyType, guyTypeNames,
        GT_Soldier, AM_DEFAULT);

}

void Guy::DelayedStart()
{
    PODVector<Node*> pods;
    node_->GetChildrenWithTag(pods,"guy_selector",true);
    if (pods.Size()>0){
        mSelector = pods[0];
    }

    mCrowdAgent = node_->GetComponent<CrowdAgent>(true);
    Select(false);
    RequestWorkMode(WM_Idle,true);
    SubscribeToEvent(E_CROWD_AGENT_STATE_CHANGED,URHO3D_HANDLER(Guy,HandleCrowdAgent));
}

void Guy::Select(bool select)
{
    if (mSelector){
        mSelector->SetEnabled(select);
    }
}

void Guy::RequestWorkMode(WorkMode newMode, bool force)
{
    mRequestedWorkmode = newMode;
    if (force) {
        CheckModeChange(true);
    }
}

bool Guy::CheckModeChange(bool force=false)
{
    if (mWorkmode != mRequestedWorkmode || force){
        if (mWorkmode == WM_Idle) {
            if (mWorkTarget){
                Cart* cart = GetSubsystem<Cart>();
                cart->ReleaseSlot(mWorkTarget);
                mWorkTarget = nullptr;
            }
        }


        mWorkmode = mRequestedWorkmode;
        mWorkTarget = mRequestWorkTarget;

        if (mRequestedWorkmode == WM_Idle){
            if (mGuyType == GT_Gatherer) {
                Cart* cart = GetSubsystem<Cart>();
                mWorkTarget = cart->GetFreeSlot();
            }
            else if (mGuyType == GT_Soldier) {
                Cart* cart = GetSubsystem<Cart>();
                mWorkTarget = cart->GetFreeSlot(true);
            }
        }
        else if (mRequestedWorkmode == WM_PickupWood){
            mCrowdAgent->SetMaxAccel(10.0f);
            mCrowdAgent->SetMaxSpeed(4.5f);
            mWorkDestinationPos = MoveTo(mWorkTarget);
        }
        else if (mRequestedWorkmode == WM_WorkWood){
            mTimer = 2.0;
        }
        else if (mRequestedWorkmode == WM_BRINGBACK_WOOD) {
            Cart* cart = GetSubsystem<Cart>();
            mWorkTarget = cart->GetResourceSlot();
            mWorkDestinationPos = MoveTo(mWorkTarget);
            mTimer = 1.0f;
        }
        return true;
    }
    return false;
}

float Guy::GetDistanceToWorkTarget()
{
    if (!mWorkTarget) return -1.0f;

    float distance = (node_->GetWorldPosition() - mWorkDestinationPos).Length();
    return distance;
}

void Guy::Tick(float dt){
    bool stateChange = CheckModeChange();


    if (mWorkmode == WM_Idle){
        mTimer -= dt;

        if (mTimer <= 0){
            mTimer = Random(DEFAULT_GUY_TIMER,DEFAULT_GUY_TIMER_MAX);
        } else {
            return;
        }

        if (mWorkTarget){
            mWorkDestinationPos = MoveTo(mWorkTarget);
        } else {
            Cart* cart = GetSubsystem<Cart>();

            if (mGuyType == GT_Gatherer) {
                Cart* cart = GetSubsystem<Cart>();
                mWorkTarget = cart->GetFreeSlot();
            }
            else if (mGuyType == GT_Soldier) {
                Cart* cart = GetSubsystem<Cart>();
                mWorkTarget = cart->GetFreeSlot(true);
            }
        }

        if (mWorkTarget){
            float distance = GetDistanceToWorkTarget();
            //URHO3D_LOGINFOF("Distance:%f",distance);
            if (distance > 4){
                mCrowdAgent->SetMaxAccel(10.0f);
                mCrowdAgent->SetMaxSpeed(3.5f);
            } else {
                mCrowdAgent->SetMaxAccel(10.0f);
                mCrowdAgent->SetMaxSpeed(1.85f);
            }
        }
    }
    else if (mWorkmode == WM_PickupWood) {
        float distance = GetDistanceToWorkTarget();
        URHO3D_LOGINFOF("WoodPick:%f",distance);
        if ( distance < 1.1f) {
            RequestWorkMode(WM_WorkWood);
        }
    }
    else if (mWorkmode == WM_WorkWood) {

        mTimer -= dt;
        URHO3D_LOGINFOF("WM_WorkWood:%f",mTimer);
        if (mTimer <= 0){
            mWorkTarget->Remove();
            RequestWorkMode(WM_BRINGBACK_WOOD);
        }
    }
    else if (mWorkmode == WM_BRINGBACK_WOOD) {
        Cart* cart = GetSubsystem<Cart>();
        MoveTo(cart->GetResourceSlot());

        float distance = GetDistanceToWorkTarget();
        URHO3D_LOGINFOF("WM_BRINGBACK_WOOD:%f",distance);
        if (distance < 1.25f){
            Cart* cart = GetSubsystem<Cart>();
            cart->AddResource(5.0f);
            RequestWorkMode(WM_Idle);
            return;
        }
    }

}

Vector3 Guy::MoveTo(Node* target)
{
    mTarget = target;

    GameNavigation* gN = GetSubsystem<GameNavigation>();
    mWorkDestinationPos = gN->MoveTo(mTarget->GetWorldPosition(),node_);
    return mWorkDestinationPos;
}

void Guy::HandleCrowdAgent(StringHash eventType, VariantMap &data)
{
    using namespace CrowdAgentStateChanged;
    int mode = data[P_CROWD_AGENT_STATE].GetInt();
    int target = data[P_CROWD_TARGET_STATE].GetInt();
    Vector3 pos = data[P_POSITION].GetVector3();
    String nodeName = static_cast<Node*>(data[P_NODE].GetPtr())->GetName();
    URHO3D_LOGINFOF("CROWND: %i:%i pos:%s node:%s",mode,target,pos.ToString().CString(),nodeName.CString());
}

Cart::Cart(Context* ctx)
    : LogicComponent(ctx),
      mSpeed(DEFAULT_CART_SPEED),
      mPathIdx(0),
      mMoving(false)
{};

void Cart::RegisterObject(Context* context)
{
    context->RegisterFactory<Cart>("Carawan");

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
    URHO3D_ATTRIBUTE("Speed", float, mSpeed, DEFAULT_CART_SPEED, AM_FILE);
}

void Cart::DelayedStart()
{
    node_->GetChildrenWithTag(mWheels,"wheel",true);
    mCrowdAgent = node_->GetComponent<CrowdAgent>(true);

    Node* slot = nullptr;
    int counter=1;

    while ( slot = node_->GetChild(String("slot")+String(counter++),true)){
        mFreeSlots.Push(slot);
        slot->SetEnabled(false);
    }

    counter=1;

    while ( slot = node_->GetChild(String("s_slot")+String(counter++),true)){
        mSoldierSlots.Push(slot);
        slot->SetEnabled(false);
    }


    PODVector<Node*> pods;
    node_->GetChildrenWithTag(pods,"bring_resource",true);
    if (pods.Size()>0){
        mBringResource = pods[0];
    }

    mParticleBurst = node_->GetComponent<ParticleEmitter>(true);

    SubscribeToEvent(node_,E_CROWD_AGENT_STATE_CHANGED,URHO3D_HANDLER(Cart,HandleCrowdManager));
}

Node* Cart::GetFreeSlot(bool soldierSlot)
{
    if (!soldierSlot){
        if (mFreeSlots.Size() == 0) return nullptr;

        Node* slot = mFreeSlots[0];
        mFreeSlots.Erase(0);
        return slot;
    } else {
        if (mSoldierSlots.Size() == 0) return nullptr;

        Node* slot = mSoldierSlots[0];
        mSoldierSlots.Erase(0);
        return slot;
    }
}

void Cart::ReleaseSlot(Node *node){
    if (node->HasTag("slot")){
        mFreeSlots.Insert(0,node);
    } else {
        mSoldierSlots.Insert(0,node);
    }
}

void Cart::AddResource(float f)
{
    status.livePower += f;
    EnableParticleBurst();
}

void Cart::EnableParticleBurst(){
    mParticleTimer = 1.0f;
    mParticleBurst->SetEnabled(true);
}

void Cart::Tick(float dt)
{
    if (mParticleTimer > 0) {
        mParticleTimer -= dt;
        if (mParticleTimer <= 0){
            mParticleBurst->SetEnabled(false);
        }
    }

    for (Node* wheel : mWheels){
         wheel->Roll(-dt * mSpeed,TS_PARENT);
     }
     CheckPath();

     status.livePower -= dt;

}

void Cart::SetPath(const PODVector<Node*>& pathNodes) {
    GameNavigation* gN = GetSubsystem<GameNavigation>();
    NavigationMesh* navMesh = gN->GetNavMesh();

    mPath.Clear();
    for (Node* n : pathNodes){
        mPath.Push(navMesh->FindNearestPoint(n->GetWorldPosition()));
    }
    mPathIdx=1;
    mCurrentTarget = mPath[1];

    Vector3 firstPos = mPath[0];
    node_->SetWorldPosition(firstPos);
}

void Cart::HandleCrowdManager(StringHash event, VariantMap &data)
{
    using namespace CrowdAgentStateChanged;

    int state = data[P_CROWD_AGENT_STATE].GetInt();
    int stateTarget = data[P_CROWD_TARGET_STATE].GetInt();
}

void Cart::CheckPath()
{
    float distance = mCurrentTarget==Vector3::ZERO ? 0.0f : (node_->GetWorldPosition() - mCurrentTarget).Length();
//URHO3D_LOGINFOF("Distance:%f",distance);
    if (distance < 1.0f){
        mPathIdx++;
        if (mPathIdx < mPath.Size()){
            mCurrentTarget = mPath[mPathIdx];
            if (mMoving){
                SetMoving(true); // trigger new path
            }
        } else {
            // finished
            SendEvent(E_CARAVAN_FINISHED);
            mMoving = false;
        }
    }
}

void Cart::SetMoving(bool moving)
{
    GameNavigation* gN = GetSubsystem<GameNavigation>();
    if (moving){
        CheckPath();
        gN->MoveTo(mCurrentTarget,node_);
    } else {
        gN->MoveTo(node_->GetWorldPosition());
    }
    mMoving = moving;
}
