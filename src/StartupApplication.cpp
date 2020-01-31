#include "StartupApplication.h"

#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Urho3DAll.h>

#include "Components/ComponentsActivator.h"

URHO3D_DEFINE_APPLICATION_MAIN(StartupApplication)

StartupApplication::StartupApplication(Context* ctx) : Application(ctx)
{
    setbuf(stdout, NULL);
    SubscribeToEvents();
}

void StartupApplication::Setup()
{
    // register game
    game_ = new GameLogic(context_);
    context_->RegisterSubsystem(game_);
    // setup game
    game_->Setup(engineParameters_);
    ComponentsActivator::RegisterComponents(context_);
}

void StartupApplication::Start()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    game_->Start();

#ifdef GAME_ENABLE_DEBUG_TOOLS
    // Get default style
    XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    // Create console
    Console* console = engine_->CreateConsole();
    console->SetDefaultStyle(xmlFile);
    console->GetBackground()->SetOpacity(0.8f);
    console->SetNumBufferedRows(512);

    // Create debug HUD.
    DebugHud* debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(xmlFile);

  #ifdef GAME_ENABLE_COMPONENT_EXPORTER
    SetupComponentExporter();
    // export registered components
    ExportComponents(String(PROJECT_NAME)+"_components.json");
  #endif


#endif
}

void StartupApplication::Stop()
{
    engine_->DumpResources(true);
}

void StartupApplication::SubscribeToEvents()
{
    // Called after engine initialization. Setup application & subscribe to events here
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(StartupApplication, HandleKeyDown));
}

void StartupApplication::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
    int key = eventData[P_KEY].GetInt();

    if (key == KEY_ESCAPE)
        engine_->Exit();

#ifdef GAME_ENABLE_DEBUG_TOOLS
    if (key == KEY_F10)
        GetSubsystem<DebugHud>()->ToggleAll();

    if (key == KEY_F9){

        Console* console = GetSubsystem<Console>();
        console->Toggle();
    }
#endif

}

#ifdef GAME_ENABLE_COMPONENT_EXPORTER
void StartupApplication::SetupComponentExporter()
{
    auto exporter = new Urho3DNodeTreeExporter(context_);
    context_->RegisterSubsystem(exporter);

    // set whitelist-mode to tell the exporter what components exactly to include for export
    exporter->SetExportMode(Urho3DNodeTreeExporter::WhiteList);

    // include all Components that inherit from LogicComponent
    exporter->AddSuperComponentHashToFilterList(LogicComponent::GetTypeStatic());


    exporter->AddMaterialFolder("Materials");
    exporter->AddTechniqueFolder("Techniques");
    exporter->AddTextureFolder("Textures");
    exporter->AddModelFolder("Models");
    exporter->AddAnimationFolder("Models");
    exporter->AddParticleFolder("Particle");
    exporter->AddSoundFolder("Sounds");

    // explicitly export those components
    exporter->AddComponentHashToFilterList(Light::GetTypeStatic());
    exporter->AddComponentHashToFilterList(Camera::GetTypeStatic());
    exporter->AddComponentHashToFilterList(SoundSource::GetTypeStatic());
    exporter->AddComponentHashToFilterList(RigidBody::GetTypeStatic());
    exporter->AddComponentHashToFilterList(CollisionShape::GetTypeStatic());
    exporter->AddComponentHashToFilterList(Navigable::GetTypeStatic());
    exporter->AddComponentHashToFilterList(NavArea::GetTypeStatic());
    exporter->AddComponentHashToFilterList(NavigationMesh::GetTypeStatic());
    exporter->AddComponentHashToFilterList(DynamicNavigationMesh::GetTypeStatic());
    exporter->AddComponentHashToFilterList(CrowdAgent::GetTypeStatic());
    exporter->AddComponentHashToFilterList(CrowdManager::GetTypeStatic());
    exporter->AddComponentHashToFilterList(Obstacle::GetTypeStatic());
    exporter->AddComponentHashToFilterList(Octree::GetTypeStatic());
    exporter->AddComponentHashToFilterList(PhysicsWorld::GetTypeStatic());
    exporter->AddComponentHashToFilterList(DebugRenderer::GetTypeStatic());
    exporter->AddComponentHashToFilterList(Zone::GetTypeStatic());
    exporter->AddComponentHashToFilterList(AnimationController::GetTypeStatic());
    exporter->AddComponentHashToFilterList(ParticleEmitter::GetTypeStatic());
    exporter->AddComponentHashToFilterList(RibbonTrail::GetTypeStatic());
    exporter->AddComponentHashToFilterList(LuaScriptInstance::GetTypeStatic());
    exporter->AddComponentHashToFilterList(SoundSource3D::GetTypeStatic());
  //  exporter->AddComponentHashToFilterList(ParticleEmitter2D::GetTypeStatic());
}

void StartupApplication::ExportComponents(const String& outputPath)
{
    Urho3DNodeTreeExporter* exporter = GetSubsystem<Urho3DNodeTreeExporter>();
    exporter->Export(outputPath,true,false);
}
#endif
