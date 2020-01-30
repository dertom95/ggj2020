#pragma once

#include <project_options.h>
#include <Urho3D/Core/Object.h>

using namespace Urho3D;

namespace Urho3D {
    class ScriptFile;
    class LuaScript;
}

class LuaScripting : public Object
{
    URHO3D_OBJECT(LuaScripting,Object);
public:
    LuaScripting(Context* ctx);
    void Init(String initialScriptFilename);

#ifdef GAME_ENABLE_DEBUG_TOOLS
    void HandleConsole(StringHash eventType, VariantMap& eventData);
#endif

private:
    SharedPtr<ScriptFile> mLuaScriptFile;
    SharedPtr<LuaScript> mLuaScript;
};
