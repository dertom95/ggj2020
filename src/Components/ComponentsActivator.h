
#include <Urho3D/Core/Object.h>
#include "Rotator.h"
#include "GroupInstance.h"
#include "ParentBone.h"
#include "RotationFix.h"
#include "LookToMoveDirection.h"
#include "PlayAnimation.h"

#include "CarawanComponents.h"

using namespace Urho3D;

class ComponentsActivator{
public:
    static void RegisterComponents(Context* context);
};

void ComponentsActivator::RegisterComponents(Context *context)
{
    // mandataory for some exporter features
    ParentBone::RegisterObject(context);
    GroupInstance::RegisterObject(context);
    RotationFix::RegisterObject(context);

    // custom components
    Rotator::RegisterObject(context);
    LookToMoveDirection::RegisterObject(context);
    PlayAnimation::RegisterObject(context);
   // Rotator2::RegisterObject(context);

    Cart::RegisterObject(context);
    Guy::RegisterObject(context);
}
