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

// a simple rotator-component as test for exporting components as json
#pragma once

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Navigation/CrowdAgent.h>

using namespace Urho3D;



const float DEFAULT_CART_SPEED = 1.0f;

struct Status{
public:
    float water;
    float exhaust;
    float livePower;
};

struct SoldierStatus {
public:
    float shild;
    float attack;
};




class Guy : public LogicComponent {
    URHO3D_OBJECT(Guy,LogicComponent)
public:
    enum GuyType {
        GT_Soldier=0, GT_Gatherer=1, GT_King=2, GT_Bandit=3
    };

    enum WorkMode {
        WM_Idle=0, WM_PickupWood=1, WM_PickupBridgePart=2, WM_BRINGBACK_WOOD=3, WM_PUTBRIDGE_PIECE=4
    };

    Guy(Context* ctx);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);

    void DelayedStart() override;

    Status status;



    Vector3 MoveTo(Node* target);
    void Select(bool select);
    void Tick(float dt);



    void HandleCrowdAgent(StringHash eventType,VariantMap& data);

    void RequestWorkMode(WorkMode newMode, bool force=false);
    inline void SetRequestWorkTarget(Node* node) { mRequestWorkTarget = node; }

    /// Set the partition type used for polygon generation.
    void SetGuyType(GuyType guyType) { mGuyType = guyType; }

    /// Return Partition Type.
    GuyType GetGuyType() const { return mGuyType; }

    float GetDistanceToWorkTarget();

private:
    bool CheckModeChange(bool force);


    WorkMode mWorkmode;
    WorkMode mRequestedWorkmode;
    SharedPtr<Node> mRequestWorkTarget;

    GuyType mGuyType;
    SharedPtr<Node> mTarget;
    SharedPtr<CrowdAgent> mCrowdAgent;
    bool mSelected;
    SharedPtr<Node> mSelector;
    float mTimer;

    SharedPtr<Node> mWorkTarget;
    Vector3 mWorkDestinationPos;
};

class Cart : public LogicComponent {
    URHO3D_OBJECT(Cart,LogicComponent)
public:
    Cart(Context* ctx);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);

    void DelayedStart() override;

    void HandleCrowdManager(StringHash event,VariantMap& data);

    void Tick(float dt);
    void SetPath(const PODVector<Node*>& pathNodes);

    void CheckPath();

    void SetMoving(bool moving);

    Status status;

    Node* GetFreeSlot();
    void ReleaseSlot(Node* node);

private:
    float mSpeed;

    PODVector<Node*> mWheels;
    PODVector<Node*> mFreeSlots;

    Vector<Vector3> mPath;
    Vector3 mCurrentTarget;
    SharedPtr<CrowdAgent> mCrowdAgent;
    int mPathIdx;

    bool mMoving;
};
