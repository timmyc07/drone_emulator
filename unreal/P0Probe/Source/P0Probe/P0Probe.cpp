#include "ProbeActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HAL/PlatformMisc.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogP0Probe, Log, All);
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, P0Probe, "P0Probe");

AP0ProbeActor::AP0ProbeActor()
{
    PrimaryActorTick.bCanEverTick = true;
    auto* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProbeCube"));
    RootComponent = Mesh;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) Mesh->SetStaticMesh(Cube.Object);
    else UE_LOG(LogP0Probe, Error, TEXT("P0 cube asset missing"));
}
void AP0ProbeActor::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogP0Probe, Display, TEXT("P0_BEGIN_PLAY world=%s"), *GetWorld()->GetName());
}
void AP0ProbeActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    ++ProbeTicks;
    // This standalone P0 fixture exits after real game frames; never use it as product gameplay.
    if (GetWorld()->IsGameWorld() && ProbeTicks == 180)
    {
        UE_LOG(LogP0Probe, Display, TEXT("P0_RUNTIME_PASS ticks=%d"), ProbeTicks);
        FPlatformMisc::RequestExit(false);
    }
}

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "FileHelpers.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Tests/AutomationEditorCommon.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FP0LevelTest, "UAVSim.P0.LevelActor", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FP0LevelTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    if (!TestNotNull(TEXT("New simple editor level"), World)) return false;
    AP0ProbeActor* Actor = World->SpawnActor<AP0ProbeActor>(FVector(300, 0, 0), FRotator::ZeroRotator);
    if (!TestNotNull(TEXT("C++ actor spawned"), Actor)) return false;
    Actor->Tick(1.0f / 60.0f);
    TestEqual(TEXT("Actor tick updates state"), Actor->ProbeTicks, 1);
    World->SpawnActor<ADirectionalLight>(FVector(0, 0, 300), FRotator(-45, 0, 0));
    const FString Filename = FPaths::ProjectContentDir() / TEXT("P0Smoke.umap");
    IFileManager::Get().MakeDirectory(*FPaths::ProjectContentDir(), true);
    TestTrue(TEXT("Saved generated simple level"), FEditorFileUtils::SaveLevel(World->PersistentLevel, Filename));
    UE_LOG(LogP0Probe, Display, TEXT("P0_EDITOR_LEVEL_ACTOR_PASS"));
    return !HasAnyErrors();
}
#endif
