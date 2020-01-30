#include "LuaScripting.h"

#include <Urho3D/Urho3DAll.h>

LuaScripting::LuaScripting(Context *ctx)
        : Object(ctx)
{

}

void LuaScripting::Init(String scriptFile)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    if (cache->Exists(scriptFile)){
        mLuaScript = new LuaScript(context_);
        context_->RegisterSubsystem(mLuaScript);

        // If script loading is successful, proceed to main loop
        if (mLuaScript->ExecuteFile(scriptFile))
        {
            mLuaScript->ExecuteFunction("Start");
            return;
        }
    }

#ifdef GAME_ENABLE_DEBUG_TOOLS
    SubscribeToEvent(E_CONSOLECOMMAND,URHO3D_HANDLER(LuaScripting,HandleConsole));
#endif
}

#ifdef GAME_ENABLE_DEBUG_TOOLS
void LuaScripting::HandleConsole(StringHash eventType, VariantMap &eventData)
{
    using namespace ConsoleCommand;
    String command = eventData[P_COMMAND].GetString();
    String id = eventData[P_ID].GetString();

    if (command != ""){
        URHO3D_LOGINFOF("LuaCommand: %s",command.CString());

        mLuaScript->ExecuteString(command);
    }
}
#endif
