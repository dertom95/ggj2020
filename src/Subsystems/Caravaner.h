#pragma once

#include <project_options.h>
#include <Urho3D/Core/Object.h>


class GameNavigation;
class GameLogic;

using namespace Urho3D;

namespace Urho3D {
    class Scene;
}

class Caravaner : public Object
{
    URHO3D_OBJECT(Caravaner,Object);
public:
    Caravaner(Context* ctx);

    void InitLevel(String name);
private:
    SharedPtr<Scene> mScene;
    SharedPtr<GameLogic> mGameLogic;
};
