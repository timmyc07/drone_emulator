#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "uavsim/simulation.hpp"
#include "UAVSimRuntime.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUAVSim, Log, All);

// Reflection of world and body Y axes; the only SI/Unreal conversion boundary.
namespace UAVSimAdapter
{
    inline FVector Position(const uavsim::Vec3& P) { return FVector(P.x(), -P.y(), P.z()) * 100.0; }
    inline FQuat Rotation(const uavsim::Quat& Q) { return FQuat(-Q.x(), Q.y(), -Q.z(), Q.w()); }
    // Prototype replay acceptance limits, not aircraft or engine capability claims.
    constexpr double MaxReplayPositionM = 1e6;
    constexpr double MaxReplayVelocityMps = 1e4;
    constexpr double MaxReplayAngularRate = 1e4;
    constexpr double MaxReplayOriginCm = 1e8;
    UAVSIMRUNTIME_API bool ValidReplayOrigin(const FVector& Origin);
    UAVSIMRUNTIME_API uavsim::Snapshot DecodeReplayFrame(const std::array<double,28>& Values,const uavsim::VehicleConfig& Config);
}

UCLASS()
class UAVSIMRUNTIME_API AUAVSimPawn : public APawn
{
    GENERATED_BODY()
public:
    AUAVSimPawn();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void StartFlight();
    void TogglePause();
    void ResetFlight();
    void ReturnToMenu();
    void ToggleArm();
    void CycleMode();
    bool SaveSession();
    void ReplaySession();
    void SetPilotAxes(double Right, double Forward, double YawRight, double Climb);
    void SetSensitivity(float Value);
    FString StatusText() const;
    bool InMenu = true;
    bool Paused = true;
    bool Replaying = false;
    FString Notice;
    float Sensitivity = 0.6f;
    bool ImperialUnits = false;
    TArray<FKey> FlightKeys;
    uavsim::Simulation Simulation;
    uavsim::Input Input;
    UPROPERTY() TObjectPtr<class UStaticMeshComponent> DroneMesh;
    UPROPERTY() TObjectPtr<class UUAVSimWidget> Interface;
private:
    double Accumulator = 0.0;
    double ReplayTime = 0.0;
    double AltitudeTarget = 1.0;
    double ClimbInput = 0.0;
    uavsim::Snapshot Previous;
    TArray<uavsim::Snapshot> Recording;
    TArray<uavsim::Snapshot> Playback;
    int32 ReplayIndex = 0;
    int32 SmokeFrames = 0;
    int32 SmokeStage = 0;
    bool Smoke = false;
    bool ConfigValid = false;
    FString ConfigHash;
    FVector Origin = FVector::ZeroVector;
    void PollInput(float DeltaSeconds);
    void ApplySnapshot(const uavsim::Snapshot& A, const uavsim::Snapshot& B, double Alpha);
};

UCLASS()
class UAVSIMRUNTIME_API UUAVSimWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaSeconds) override;
    UPROPERTY() TObjectPtr<AUAVSimPawn> Drone;
private:
    UPROPERTY() TObjectPtr<class UTextBlock> Readout;
    UPROPERTY() TObjectPtr<class UTextBlock> Message;
    UPROPERTY() TObjectPtr<class UVerticalBox> Menu;
    UPROPERTY() TObjectPtr<class UComboBoxString> EnvironmentChoice;
    UPROPERTY() TObjectPtr<class UComboBoxString> GraphicsChoice;
    UPROPERTY() TObjectPtr<class UComboBoxString> ResolutionChoice;
    UPROPERTY() TObjectPtr<class UComboBoxString> UnitsChoice;
    UPROPERTY() TArray<TObjectPtr<class UInputKeySelector>> KeySelectors;
    UPROPERTY() TObjectPtr<class USlider> SensitivityControl;
    UFUNCTION() void StartClicked();
    UFUNCTION() void PauseClicked();
    UFUNCTION() void ResetClicked();
    UFUNCTION() void MenuClicked();
    UFUNCTION() void ArmClicked();
    UFUNCTION() void ModeClicked();
    UFUNCTION() void SaveClicked();
    UFUNCTION() void ReplayClicked();
    UFUNCTION() void ApplyClicked();
    UFUNCTION() void QuitClicked();
};

UCLASS()
class UAVSIMRUNTIME_API AUAVSimGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AUAVSimGameMode();
};
