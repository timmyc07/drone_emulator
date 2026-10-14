#include "UAVSimRuntime.h"
#include "uavsim/config.hpp"
#include "uavsim/version.hpp"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/ConstructorHelpers.h"
#include <exception>
#include <stdexcept>

DEFINE_LOG_CATEGORY(LogUAVSim);
IMPLEMENT_MODULE(FDefaultModuleImpl, UAVSimRuntime);

bool UAVSimAdapter::ValidReplayOrigin(const FVector& Value)
{
    return FMath::IsFinite(Value.X) && FMath::IsFinite(Value.Y) && FMath::IsFinite(Value.Z) && FMath::Abs(Value.X)<=MaxReplayOriginCm && FMath::Abs(Value.Y)<=MaxReplayOriginCm && FMath::Abs(Value.Z)<=MaxReplayOriginCm;
}

uavsim::Snapshot UAVSimAdapter::DecodeReplayFrame(const std::array<double,28>& X,const uavsim::VehicleConfig& Config)
{
    for(double Value : X) if(!std::isfinite(Value)) throw std::invalid_argument("Non-finite replay data");
    if(X[0]<0 || X[0]>600.01 || (X[14]!=0 && X[14]!=1) || (X[15]!=0 && X[15]!=1 && X[15]!=2)) throw std::invalid_argument("Invalid replay time, arming or mode");
    for(int I=0;I<3;++I)
    {
        // Check components before multiplication or norms, which can overflow for hostile values.
        if(std::abs(X[1+I])>MaxReplayPositionM || std::abs(X[4+I])>MaxReplayVelocityMps || std::abs(X[11+I])>MaxReplayAngularRate) throw std::invalid_argument("Replay state exceeds supported prototype range");
    }
    uavsim::Snapshot S;
    S.time=X[0]; S.state.position_m={X[1],X[2],X[3]}; S.state.velocity_mps={X[4],X[5],X[6]}; S.state.orientation={X[7],X[8],X[9],X[10]}; S.state.angular_velocity_radps={X[11],X[12],X[13]}; S.armed=X[14]!=0; S.mode=static_cast<uavsim::FlightMode>(static_cast<int>(X[15]));
    uavsim::validate_state(S.state);
    S.state.orientation.normalize();
    for(int I=0;I<4;++I)
    {
        const auto& Rotor=Config.rotors[I];
        const double MaxThrust=Rotor.k_thrust*Rotor.max_speed_radps*Rotor.max_speed_radps;
        const double SpeedTolerance=FMath::Max(1.0,Rotor.max_speed_radps)*1e-9;
        const double ThrustTolerance=FMath::Max(1.0,MaxThrust)*1e-9;
        if(X[16+I]<0 || X[16+I]>1 || X[20+I]<0 || X[20+I]>Rotor.max_speed_radps+SpeedTolerance || X[24+I]<0 || X[24+I]>MaxThrust+ThrustTolerance) throw std::invalid_argument("Replay motor data exceeds configured limits");
        S.motor_commands[I]=X[16+I]; S.rotor_speeds[I]=X[20+I]; S.thrusts[I]=X[24+I];
    }
    return S;
}

AUAVSimGameMode::AUAVSimGameMode() { DefaultPawnClass = AUAVSimPawn::StaticClass(); }

AUAVSimPawn::AUAVSimPawn()
{
    FlightKeys = {EKeys::W,EKeys::S,EKeys::D,EKeys::A,EKeys::E,EKeys::Q,EKeys::SpaceBar,EKeys::LeftControl};
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SimulationOrigin"));
    DroneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneVisual"));
    DroneMesh->SetupAttachment(RootComponent);
    DroneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DroneMesh->SetSimulatePhysics(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Fallback(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    DroneMesh->SetStaticMesh(Fallback.Object);
    DroneMesh->SetRelativeLocation(FVector(0, 0, 12));
    auto* Arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FollowArm"));
    Arm->SetupAttachment(RootComponent);
    Arm->TargetArmLength = 350;
    Arm->SetRelativeRotation(FRotator(-18, 0, 0));
    Arm->bInheritRoll = false;
    Arm->bInheritPitch = false;
    Arm->bDoCollisionTest = false;
    auto* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Arm);
    Camera->FieldOfView = 80;
}

void AUAVSimPawn::BeginPlay()
{
    Super::BeginPlay();
    Origin = GetActorLocation();
    if(GetWorld()->GetMapName().Contains(TEXT("UAVSim_Test"))) Origin.Z=0;
    if (auto* Model = LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Drone/SM_DefaultDrone.SM_DefaultDrone"))) DroneMesh->SetStaticMesh(Model);
    else UE_LOG(LogUAVSim, Warning, TEXT("Default drone model missing; using visibly simplified fallback"));
    GConfig->GetFloat(TEXT("UAVSim"), TEXT("Sensitivity"), Sensitivity, GGameUserSettingsIni);
    GConfig->GetBool(TEXT("UAVSim"),TEXT("ImperialUnits"),ImperialUnits,GGameUserSettingsIni);
    for (int32 I=0;I<FlightKeys.Num();++I)
    {
        FString KeyName;
        if(GConfig->GetString(TEXT("UAVSim"),*FString::Printf(TEXT("FlightKey%d"),I),KeyName,GGameUserSettingsIni))
        {
            const FKey Key(*KeyName); if(Key.IsValid()) FlightKeys[I]=Key;
        }
    }
    Sensitivity = FMath::Clamp(Sensitivity, 0.1f, 1.0f);
    try
    {
        const FString ConfigPath=FPaths::ProjectDir()/TEXT("Config/Vehicles/generic_quadrotor_x01.json");
        const auto Loaded=uavsim::load_vehicle(std::filesystem::path(*ConfigPath));
        Simulation=uavsim::Simulation(Loaded.vehicle);
        ConfigHash=UTF8_TO_TCHAR(Loaded.sha256.c_str());
        ConfigValid=true;
    }
    catch(const std::exception& Error) { UE_LOG(LogUAVSim,Error,TEXT("Vehicle configuration load failed: %s"),UTF8_TO_TCHAR(Error.what())); }
    if (DroneMesh->GetStaticMesh())
    {
        const float Diameter = DroneMesh->GetStaticMesh()->GetBounds().BoxExtent.GetMax() * 2;
        if (Diameter > KINDA_SMALL_NUMBER) DroneMesh->SetRelativeScale3D(FVector(65.0f / Diameter));
    }
    if (auto* PC = Cast<APlayerController>(GetController()))
    {
        Interface = CreateWidget<UUAVSimWidget>(PC, UUAVSimWidget::StaticClass());
        Interface->Drone = this;
        Interface->AddToViewport();
        PC->bShowMouseCursor = true;
        FInputModeGameAndUI Mode;
        Mode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(Mode);
    }
    ResetFlight();
    if(!ConfigValid) Notice=TEXT("Vehicle configuration is missing or invalid. Flight is disabled; restore the packaged Config/Vehicles file.");
    Smoke = FParse::Param(FCommandLine::Get(), TEXT("UAVSimSmoke"));
    if (Smoke) { if(!ConfigValid) { FPlatformMisc::RequestExitWithStatus(false,1); return; } StartFlight(); }
    UE_LOG(LogUAVSim, Display, TEXT("UAVSIM_BEGIN_PLAY core_dt=%.3f model=%s"), Simulation.timestep(), *GetNameSafe(DroneMesh->GetStaticMesh()));
}

void AUAVSimPawn::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Interface) Interface->RemoveFromParent();
    Super::EndPlay(Reason);
}

void AUAVSimPawn::SetSensitivity(float Value)
{
    Sensitivity = FMath::Clamp(Value, 0.1f, 1.0f);
    GConfig->SetFloat(TEXT("UAVSim"), TEXT("Sensitivity"), Sensitivity, GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}

void AUAVSimPawn::StartFlight()
{
    if(!ConfigValid) { UE_LOG(LogUAVSim,Error,TEXT("Start rejected: invalid vehicle configuration")); return; }
    InMenu = false;
    Paused = false;
    Replaying = false;
    Input.armed = true;
    Input.mode = uavsim::FlightMode::AltitudeHold;
    Input.altitude_m = AltitudeTarget;
    Notice = TEXT("Armed. Space / Ctrl: climb / descend. WASD: tilt. Q/E: yaw.");
    UE_LOG(LogUAVSim, Display, TEXT("Flight started, altitude target %.2f m"), AltitudeTarget);
}

void AUAVSimPawn::TogglePause()
{
    if (InMenu) return;
    Paused = !Paused;
    Accumulator = 0;
    UE_LOG(LogUAVSim, Display, TEXT("Session paused=%d"), Paused);
}

void AUAVSimPawn::ResetFlight()
{
    Simulation.reset();
    Input = {};
    Input.mode = uavsim::FlightMode::AltitudeHold;
    AltitudeTarget = 1.0;
    ClimbInput = 0.0;
    Input.altitude_m = AltitudeTarget;
    Input.armed = !InMenu;
    Accumulator = 0;
    Replaying = false;
    Previous = Simulation.snapshot();
    Recording.Reset();
    Recording.Add(Previous);
    ApplySnapshot(Previous, Previous, 0);
    Notice = TEXT("Flight reset. Altitude target: 1 m.");
    UE_LOG(LogUAVSim, Display, TEXT("Flight reset; controller integrators and recording cleared"));
}

void AUAVSimPawn::ReturnToMenu()
{
    InMenu = true;
    Paused = true;
    Input.armed = false;
    Replaying = false;
    UE_LOG(LogUAVSim, Display, TEXT("Returned to menu"));
}
void AUAVSimPawn::ToggleArm()
{
    if (InMenu || Replaying) return;
    Input.armed = !Input.armed;
    UE_LOG(LogUAVSim, Display, TEXT("Armed=%d"), Input.armed);
}
void AUAVSimPawn::CycleMode()
{
    if (Replaying) return;
    Input.mode = static_cast<uavsim::FlightMode>((static_cast<int>(Input.mode) + 1) % 3);
    AltitudeTarget = FMath::Max(0.0, Simulation.snapshot().state.position_m.z());
    UE_LOG(LogUAVSim, Display, TEXT("Flight mode=%s"), UTF8_TO_TCHAR(uavsim::flight_mode_name(Input.mode)));
}

void AUAVSimPawn::PollInput(float DeltaSeconds)
{
    auto* PC = Cast<APlayerController>(GetController());
    if (!PC) return;
    if (PC->WasInputKeyJustPressed(EKeys::Escape)) ReturnToMenu();
    if (PC->WasInputKeyJustPressed(EKeys::P)) TogglePause();
    if (PC->WasInputKeyJustPressed(EKeys::R)) ResetFlight();
    if (PC->WasInputKeyJustPressed(EKeys::F)) ToggleArm();
    if (PC->WasInputKeyJustPressed(EKeys::M)) CycleMode();
    if (InMenu || Paused || Replaying) return;
    const auto Axis = [PC](FKey Positive, FKey Negative, FKey Gamepad)
    {
        return FMath::Clamp((PC->IsInputKeyDown(Positive) ? 1.0 : 0.0) - (PC->IsInputKeyDown(Negative) ? 1.0 : 0.0) + PC->GetInputAnalogKeyState(Gamepad), -1.0, 1.0);
    };
    SetPilotAxes(Axis(FlightKeys[2],FlightKeys[3],EKeys::Gamepad_RightX),Axis(FlightKeys[0],FlightKeys[1],EKeys::Gamepad_RightY),Axis(FlightKeys[4],FlightKeys[5],EKeys::Gamepad_LeftX),Axis(FlightKeys[6],FlightKeys[7],EKeys::Gamepad_LeftY));
}

void AUAVSimPawn::SetPilotAxes(double Right,double Forward,double YawRight,double Climb)
{
    Input.roll=FMath::Clamp(Right,-1.0,1.0)*Sensitivity;
    Input.pitch=FMath::Clamp(Forward,-1.0,1.0)*Sensitivity;
    Input.yaw=-FMath::Clamp(YawRight,-1.0,1.0)*Sensitivity;
    ClimbInput=FMath::Clamp(Climb,-1.0,1.0);
    double MaximumThrust=0;
    for(const auto& Rotor : Simulation.config().rotors) MaximumThrust+=Rotor.k_thrust*Rotor.max_speed_radps*Rotor.max_speed_radps;
    const double Hover=Simulation.config().mass_kg*Simulation.environment().gravity_mps2/MaximumThrust;
    Input.collective=ClimbInput>=0 ? Hover+ClimbInput*(1-Hover) : Hover+ClimbInput*Hover;
}

void AUAVSimPawn::ApplySnapshot(const uavsim::Snapshot& A, const uavsim::Snapshot& B, double Alpha)
{
    const FVector P = FMath::Lerp(UAVSimAdapter::Position(A.state.position_m), UAVSimAdapter::Position(B.state.position_m), Alpha);
    const FQuat Q = FQuat::Slerp(UAVSimAdapter::Rotation(A.state.orientation), UAVSimAdapter::Rotation(B.state.orientation), Alpha).GetNormalized();
    SetActorLocationAndRotation(Origin + P, Q, false, nullptr, ETeleportType::TeleportPhysics);
}

void AUAVSimPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Smoke) PollInput(DeltaSeconds);
    if (InMenu || Paused) return;
    try
    {
        if (Replaying)
        {
            ReplayTime += DeltaSeconds;
            while (ReplayIndex + 1 < Playback.Num() && Playback[ReplayIndex + 1].time <= ReplayTime) ++ReplayIndex;
            if (ReplayIndex + 1 >= Playback.Num()) { ApplySnapshot(Playback.Last(),Playback.Last(),0); Paused = true; Notice = TEXT("State playback complete."); }
            else
            {
                const auto& A = Playback[ReplayIndex];
                const auto& B = Playback[ReplayIndex + 1];
                ApplySnapshot(A, B, (ReplayTime - A.time) / (B.time - A.time));
            }
            return;
        }
        if (DeltaSeconds > 0.25f) UE_LOG(LogUAVSim, Warning, TEXT("Frame stall: %.3f s; retaining elapsed time for fixed-step catch-up"), DeltaSeconds);
        Accumulator += static_cast<double>(DeltaSeconds);
        int32 CatchupSteps=0;
        while (Accumulator >= Simulation.timestep() && CatchupSteps<256)
        {
            AltitudeTarget=FMath::Clamp(AltitudeTarget+ClimbInput*Simulation.timestep()*2.0,0.0,1000.0);
            Input.altitude_m=AltitudeTarget;
            Simulation.set_input(Input);
            Previous = Simulation.snapshot();
            Simulation.step();
            Accumulator -= Simulation.timestep();
            ++CatchupSteps;
            if (Recording.Num() < 150001) Recording.Add(Simulation.snapshot());
            else if (Recording.Num() == 150001)
            {
                Notice = TEXT("Recording limit: first 10 minutes retained. Save, then reset for a new session.");
            }
        }
        if(Accumulator>=Simulation.timestep())
        {
            Notice=FString::Printf(TEXT("Simulation catching up: %.2f s retained after a frame stall."),Accumulator);
            UE_LOG(LogUAVSim,Warning,TEXT("Fixed-step backlog retained: %.3f s"),Accumulator);
        }
        ApplySnapshot(Previous, Simulation.snapshot(), FMath::Clamp(Accumulator / Simulation.timestep(),0.0,1.0));
        if (Smoke)
        {
            ++SmokeFrames;
            if (SmokeStage == 0 && Simulation.snapshot().time >= 3.0)
            {
                const double Height = Simulation.snapshot().state.position_m.z();
                if (Height < 0.2 || Height > 2.0) throw std::runtime_error("Smoke: altitude controller did not reach flight envelope");
                TogglePause();
                const auto Before = Simulation.snapshot().step;
                Tick(0.1f);
                if (Simulation.snapshot().step != Before) throw std::runtime_error("Smoke: paused state advanced");
                TogglePause();
                if(!SaveSession()) throw std::runtime_error("Smoke: session persistence failed");
                ResetFlight();
                if (Simulation.snapshot().time != 0) throw std::runtime_error("Smoke: reset clock failed");
                SmokeStage = 1;
            }
            if (SmokeStage == 1 && Simulation.snapshot().time >= 1.0)
            {
                ReplaySession();
                if(!Replaying || Playback.Num()<2) throw std::runtime_error("Smoke: replay did not load saved states");
                Tick(static_cast<float>(Playback.Last().time+1));
                if(!Paused || !GetActorLocation().Equals(Origin+UAVSimAdapter::Position(Playback.Last().state.position_m),1e-4)) throw std::runtime_error("Smoke: replay final pose mismatch");
                UE_LOG(LogUAVSim, Display, TEXT("UAVSIM_RUNTIME_PASS frames=%d altitude=%.4f pause/reset/save/replay verified"), SmokeFrames, Simulation.snapshot().state.position_m.z());
                FPlatformMisc::RequestExit(false);
            }
        }
    }
    catch (const std::exception& Error)
    {
        Paused = true;
        Notice = FString::Printf(TEXT("Simulation stopped: %s"), UTF8_TO_TCHAR(Error.what()));
        UE_LOG(LogUAVSim, Error, TEXT("%s"), *Notice);
        if (Smoke) FPlatformMisc::RequestExitWithStatus(false, 1);
    }
}

FString AUAVSimPawn::StatusText() const
{
    const auto S = Replaying && Playback.IsValidIndex(ReplayIndex) ? Playback[ReplayIndex] : Simulation.snapshot();
    const auto Forward = S.state.orientation * uavsim::Vec3::UnitX();
    double Heading = FMath::RadiansToDegrees(std::atan2(Forward.x(), Forward.y()));
    if (Heading < 0) Heading += 360;
    const FRotator Attitude = UAVSimAdapter::Rotation(S.state.orientation).Rotator();
    const double Scale=ImperialUnits ? 3.280839895 : 1.0;
    const TCHAR* Unit=ImperialUnits ? TEXT("ft") : TEXT("m");
    return FString::Printf(TEXT("UAVSIM  /  GENERIC QUADROTOR X-01\n%s  |  %s  |  %s\nALT %.2f %s   GROUND SPEED %.2f %s/s   V/S %+.2f %s/s\nHEADING %.0f deg   ROLL %+.1f   PITCH %+.1f\nTIME %.2f s   TARGET %.1f %s   WIND 0 %s/s\nFlat ground contact model  |  Engineering estimates"),
        InMenu ? TEXT("MENU") : (Paused ? TEXT("PAUSED") : (Replaying ? TEXT("RECORDED STATE PLAYBACK") : TEXT("LIVE PHYSICS"))),
        UTF8_TO_TCHAR(uavsim::flight_mode_name(S.mode)), S.armed ? TEXT("ARMED") : TEXT("DISARMED"),
        S.state.position_m.z()*Scale,Unit,S.state.velocity_mps.head<2>().norm()*Scale,Unit,S.state.velocity_mps.z()*Scale,Unit,Heading,
        Attitude.Roll, Attitude.Pitch, S.time, AltitudeTarget*Scale,Unit,Unit);
}

bool AUAVSimPawn::SaveSession()
{
    if (Recording.Num() < 2) { Notice = TEXT("Fly before saving a session."); return false; }
    auto Root = MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("format"), TEXT("uavsim-state-playback-v1"));
    Root->SetStringField(TEXT("vehicle"), UTF8_TO_TCHAR(Simulation.config().id.c_str()));
    Root->SetStringField(TEXT("version"), UTF8_TO_TCHAR(UAVSIM_VERSION_STRING));
    Root->SetStringField(TEXT("config_sha256"),ConfigHash);
    Root->SetStringField(TEXT("scenario"),GetWorld()->GetMapName());
    Root->SetStringField(TEXT("coordinates"),TEXT("ENU/FLU; SI; Hamilton quaternion body-to-world"));
    Root->SetStringField(TEXT("wind_mps"),TEXT("0,0,0"));
    Root->SetNumberField(TEXT("origin_x_cm"),Origin.X); Root->SetNumberField(TEXT("origin_y_cm"),Origin.Y); Root->SetNumberField(TEXT("origin_z_cm"),Origin.Z);
    Root->SetNumberField(TEXT("dt"), Simulation.timestep());
    TArray<TSharedPtr<FJsonValue>> Frames;
    FString Csv = TEXT("time_s,east_m,north_m,up_m,ve_mps,vn_mps,vu_mps,qw,qx,qy,qz,wx_radps,wy_radps,wz_radps,armed,mode,motor0,motor1,motor2,motor3,rotor0_radps,rotor1_radps,rotor2_radps,rotor3_radps,thrust0_n,thrust1_n,thrust2_n,thrust3_n\n");
    for (const auto& S : Recording)
    {
        TArray<TSharedPtr<FJsonValue>> Row;
        const auto& P = S.state.position_m; const auto& V = S.state.velocity_mps; const auto& Q = S.state.orientation; const auto& W = S.state.angular_velocity_radps;
        const double Values[] = { S.time, P.x(), P.y(), P.z(), V.x(), V.y(), V.z(), Q.w(), Q.x(), Q.y(), Q.z(), W.x(), W.y(), W.z(), S.armed ? 1.0 : 0.0, static_cast<double>(S.mode), S.motor_commands[0], S.motor_commands[1], S.motor_commands[2], S.motor_commands[3], S.rotor_speeds[0], S.rotor_speeds[1], S.rotor_speeds[2], S.rotor_speeds[3], S.thrusts[0], S.thrusts[1], S.thrusts[2], S.thrusts[3] };
        for (int32 I = 0; I < UE_ARRAY_COUNT(Values); ++I) { Row.Add(MakeShared<FJsonValueNumber>(Values[I])); Csv += FString::Printf(TEXT("%.12g%s"), Values[I], I + 1 == UE_ARRAY_COUNT(Values) ? TEXT("\n") : TEXT(",")); }
        Frames.Add(MakeShared<FJsonValueArray>(Row));
    }
    Root->SetArrayField(TEXT("frames"), Frames);
    FString Json; FJsonSerializer::Serialize(Root, TJsonWriterFactory<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>::Create(&Json));
    const FString Folder = FPaths::ProjectSavedDir() / TEXT("Sessions");
    IFileManager::Get().MakeDirectory(*Folder, true);
    const FString Name = FDateTime::UtcNow().ToString(TEXT("%Y%m%d-%H%M%S"))+TEXT("-")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    const FString JsonPath=Folder/(Name+TEXT(".json"));
    const FString CsvPath=Folder/(Name+TEXT(".csv"));
    const FString LatestTemp=Folder/(Name+TEXT(".latest.tmp"));
    if (!FFileHelper::SaveStringToFile(Json,*JsonPath) || !FFileHelper::SaveStringToFile(Csv,*CsvPath) || IFileManager::Get().FileSize(*JsonPath)<=0 || IFileManager::Get().FileSize(*CsvPath)<=0 || !FFileHelper::SaveStringToFile(Json,*LatestTemp) || !IFileManager::Get().Move(*(Folder/TEXT("latest.json")),*LatestTemp,true))
    {
        Notice = TEXT("Session save failed; check available disk space and permissions."); UE_LOG(LogUAVSim, Error, TEXT("%s"), *Notice); return false;
    }
    Notice = TEXT("Saved JSON playback and CSV telemetry to Saved/Sessions.");
    UE_LOG(LogUAVSim, Display, TEXT("Session saved: %d frames"), Recording.Num());
    return true;
}

void AUAVSimPawn::ReplaySession()
{
    FString Json; TSharedPtr<FJsonObject> Root;
    const TArray<TSharedPtr<FJsonValue>>* Frames = nullptr;
    const FString Filename=FPaths::ProjectSavedDir()/TEXT("Sessions/latest.json");
    const int64 FileSize=IFileManager::Get().FileSize(*Filename);
    FString Format,Scenario,Version,Hash; double OriginX=0,OriginY=0,OriginZ=0;
    if (FileSize<=0 || FileSize>100000000 || !FFileHelper::LoadFileToString(Json,*Filename) || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid() || !Root->TryGetStringField(TEXT("format"),Format) || Format!=TEXT("uavsim-state-playback-v1") || !Root->TryGetStringField(TEXT("scenario"),Scenario) || Scenario!=GetWorld()->GetMapName() || !Root->TryGetStringField(TEXT("version"),Version) || Version!=UTF8_TO_TCHAR(UAVSIM_VERSION_STRING) || !Root->TryGetStringField(TEXT("config_sha256"),Hash) || Hash!=ConfigHash || !Root->TryGetNumberField(TEXT("origin_x_cm"),OriginX) || !Root->TryGetNumberField(TEXT("origin_y_cm"),OriginY) || !Root->TryGetNumberField(TEXT("origin_z_cm"),OriginZ) || !UAVSimAdapter::ValidReplayOrigin(FVector(OriginX,OriginY,OriginZ)) || !Root->TryGetArrayField(TEXT("frames"), Frames) || Frames->Num() < 2 || Frames->Num() > 150001)
    {
        Notice = TEXT("No valid session for this environment. Select its original scene, or fly and save first."); UE_LOG(LogUAVSim, Warning, TEXT("Replay rejected: missing, invalid, oversized or different-scenario session")); return;
    }
    TArray<uavsim::Snapshot> Loaded;
    try
    {
        for (const auto& Frame : *Frames)
        {
            const TArray<TSharedPtr<FJsonValue>>* Row = nullptr;
            if (!Frame->TryGetArray(Row) || Row->Num() != 28) throw std::runtime_error("Invalid replay frame shape");
            std::array<double,28> X{};
            for (int I = 0; I < 28; ++I) if (!(*Row)[I]->TryGetNumber(X[I])) throw std::runtime_error("Non-numeric replay data");
            const auto S=UAVSimAdapter::DecodeReplayFrame(X,Simulation.config());
            if(!Loaded.IsEmpty() && S.time<=Loaded.Last().time) throw std::runtime_error("Replay timestamps must increase");
            Loaded.Add(S);
        }
    }
    catch (const std::exception& Error) { Notice = TEXT("Saved session is invalid; playback cancelled."); UE_LOG(LogUAVSim, Error, TEXT("Replay rejected: %s"), UTF8_TO_TCHAR(Error.what())); return; }
    Origin=FVector(OriginX,OriginY,OriginZ);
    Playback = MoveTemp(Loaded); ReplayIndex = 0; ReplayTime = Playback[0].time; Replaying = true; InMenu = false; Paused = false;
    ApplySnapshot(Playback[0],Playback[0],0);
    Notice = TEXT("Recorded state playback. Flight controls disabled; this is not deterministic input replay.");
    UE_LOG(LogUAVSim, Display, TEXT("Playback started: %d states"), Playback.Num());
}
