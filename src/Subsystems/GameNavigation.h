#pragma once

#include <project_options.h>
#include <Urho3D/Core/Object.h>

namespace Urho3D {
    class NavigationMesh;
    class CrowdManager;
    class Node;
    class Scene;
}

using namespace Urho3D;

class GameNavigation : public Object
{
    URHO3D_OBJECT(GameNavigation,Object);

public:
    GameNavigation(Context* ctx);

    void Init();
    bool FindPath(Vector3 from, Vector3 to, PODVector<Vector3>& dest);
    Vector3 MoveTo(Vector3 gotoPosition,Node* node=nullptr);
    inline void ShowDebug(bool show) { mDrawDebug = show; }
    inline bool IsShowDebug() { return mDrawDebug; }
    SharedPtr<NavigationMesh> GetNavMesh() { return mNavMesh; }

#ifdef GAME_ENABLE_DEBUG_TOOLS
    inline void EnableTestMode(bool enable) { tempTestMode = enable; }
#endif

private:
    void HandlePostRender(StringHash eventType,VariantMap& data);
    void HandleUpdate(StringHash eventType,VariantMap& data);

    /// Handle problems with crowd agent placement.
    void HandleCrowdAgentFailure(StringHash eventType, VariantMap& eventData);
    /// Handle crowd agent reposition.
    void HandleCrowdAgentReposition(StringHash eventType, VariantMap& eventData);
    /// Handle crowd agent formation.
    void HandleCrowdAgentFormation(StringHash eventType, VariantMap& eventData);

    SharedPtr<NavigationMesh> mNavMesh;
    SharedPtr<CrowdManager> mCrowdManager;
    SharedPtr<Scene> mScene;

    bool mDrawDebug;

    PODVector<Vector3> tempPathPoints;

#ifdef GAME_ENABLE_DEBUG_TOOLS
    bool tempTestMode;
    Vector3 tempFrom;
    Vector3 tempTo;
    Vector3 tempMoveTo;
#endif

};
