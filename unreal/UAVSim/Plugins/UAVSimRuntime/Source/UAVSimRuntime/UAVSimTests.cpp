#include "UAVSimRuntime.h"
#include "Misc/AutomationTest.h"
#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Tests/AutomationEditorCommon.h"
#include "FileHelpers.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerStart.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include <limits>
#include <exception>
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUAVSimReplayTest,"UAVSim.Product.ReplayValidation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FUAVSimReplayTest::RunTest(const FString& Parameters)
{
    const auto Config=uavsim::reference_quadrotor();
    std::array<double,28> Valid{}; Valid[7]=1; Valid[14]=1; Valid[15]=2;
    const auto Accepted=[&Config](const std::array<double,28>& Values)
    {
        try { UAVSimAdapter::DecodeReplayFrame(Values,Config); return true; }
        catch(const std::exception&) { return false; }
    };
    TestTrue(TEXT("Ordinary saved state accepted"),Accepted(Valid));
    auto Boundary=Valid;
    for(int I=0;I<3;++I) { Boundary[1+I]=UAVSimAdapter::MaxReplayPositionM; Boundary[4+I]=-UAVSimAdapter::MaxReplayVelocityMps; Boundary[11+I]=UAVSimAdapter::MaxReplayAngularRate; }
    for(int I=0;I<4;++I) { Boundary[16+I]=1; Boundary[20+I]=Config.rotors[I].max_speed_radps; Boundary[24+I]=Config.rotors[I].k_thrust*Boundary[20+I]*Boundary[20+I]; }
    TestTrue(TEXT("Documented state and configured motor boundaries accepted"),Accepted(Boundary));
    const FVector BoundaryOrigin(UAVSimAdapter::MaxReplayOriginCm);
    TestTrue(TEXT("Documented origin boundary accepted"),UAVSimAdapter::ValidReplayOrigin(BoundaryOrigin));
    const auto Parsed=UAVSimAdapter::DecodeReplayFrame(Boundary,Config);
    TestFalse(TEXT("Largest accepted transform stays finite"),(BoundaryOrigin+UAVSimAdapter::Position(Parsed.state.position_m)).ContainsNaN());
    for(int Index : {1,2,3,4,5,6,11,12,13})
    {
        auto Invalid=Boundary; Invalid[Index]*=1.000001;
        TestFalse(TEXT("Just beyond supported component range rejected"),Accepted(Invalid));
    }
    for(int Index : {1,4,7,11,16,20,24})
    {
        auto Invalid=Valid; Invalid[Index]=std::numeric_limits<double>::max();
        TestFalse(TEXT("Huge finite values rejected before rendering"),Accepted(Invalid));
        Invalid[Index]=std::numeric_limits<double>::quiet_NaN();
        TestFalse(TEXT("NaN rejected"),Accepted(Invalid));
        Invalid[Index]=std::numeric_limits<double>::infinity();
        TestFalse(TEXT("Infinity rejected"),Accepted(Invalid));
    }
    for(int I=0;I<4;++I)
    {
        auto Invalid=Boundary; Invalid[20+I]*=1.000001;
        TestFalse(TEXT("Rotor overspeed rejected"),Accepted(Invalid));
        Invalid=Boundary; Invalid[24+I]*=1.000001;
        TestFalse(TEXT("Thrust above configured rotor maximum rejected"),Accepted(Invalid));
        for(int Index : {16+I,20+I,24+I}) { Invalid=Valid; Invalid[Index]=-0.001; TestFalse(TEXT("Negative propulsion data rejected"),Accepted(Invalid)); }
    }
    for(int Index : {14,15}) { auto Invalid=Valid; Invalid[Index]=0.5; TestFalse(TEXT("Fractional arming and flight mode rejected"),Accepted(Invalid)); }
    auto InvalidTime=Valid; InvalidTime[0]=601; TestFalse(TEXT("Beyond recording duration rejected"),Accepted(InvalidTime));
    auto InvalidQuaternion=Valid; InvalidQuaternion[7]=0; TestFalse(TEXT("Zero quaternion rejected"),Accepted(InvalidQuaternion));
    auto RoundedQuaternion=Valid; RoundedQuaternion[7]=1.0+1e-8;
    TestTrue(TEXT("Accepted rounded quaternion normalized"),FMath::Abs(UAVSimAdapter::DecodeReplayFrame(RoundedQuaternion,Config).state.orientation.norm()-1)<1e-12);
    TestFalse(TEXT("Out-of-range origin rejected"),UAVSimAdapter::ValidReplayOrigin(FVector(UAVSimAdapter::MaxReplayOriginCm+1,0,0)));
    TestFalse(TEXT("Non-finite origin rejected"),UAVSimAdapter::ValidReplayOrigin(FVector(std::numeric_limits<double>::infinity(),0,0)));
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUAVSimAdapterTest,"UAVSim.Product.AdapterAndLifecycle",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FUAVSimAdapterTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("ENU metres to reflected centimetres"),UAVSimAdapter::Position({1,2,3}).Equals(FVector(100,-200,300)));
    const uavsim::Quat Q(Eigen::AngleAxisd(0.5,uavsim::Vec3::UnitZ()));
    const auto Expected = UAVSimAdapter::Position(Q * uavsim::Vec3::UnitX()) / 100;
    TestTrue(TEXT("Quaternion reflection matches vector mapping"),UAVSimAdapter::Rotation(Q).RotateVector(FVector(1,0,0)).Equals(Expected,1e-6));
    const uavsim::Quat Roll(Eigen::AngleAxisd(0.1,uavsim::Vec3::UnitX()));
    const uavsim::Quat Pitch(Eigen::AngleAxisd(0.1,uavsim::Vec3::UnitY()));
    const uavsim::Quat Yaw(Eigen::AngleAxisd(-0.1,uavsim::Vec3::UnitZ()));
    TestTrue(TEXT("Positive roll tilts thrust right in Unreal"),UAVSimAdapter::Position(Roll*uavsim::Vec3::UnitZ()).Y>0);
    TestTrue(TEXT("Positive pitch tilts thrust forward in Unreal"),UAVSimAdapter::Position(Pitch*uavsim::Vec3::UnitZ()).X>0);
    TestTrue(TEXT("Negative FLU yaw turns nose right in Unreal"),UAVSimAdapter::Position(Yaw*uavsim::Vec3::UnitX()).Y>0);
    uavsim::Simulation Sim;
    uavsim::Input Input; Input.armed=true; Input.mode=uavsim::FlightMode::AltitudeHold; Input.altitude_m=1;
    Sim.set_input(Input); for(int I=0;I<750;++I) Sim.step();
    TestTrue(TEXT("Input lifts vehicle through physical core"),Sim.snapshot().state.position_m.z()>0.2);
    Sim.set_paused(true); const auto Step=Sim.snapshot().step; Sim.step(); TestEqual(TEXT("Pause stops fixed step"),Sim.snapshot().step,Step);
    Sim.reset(); TestEqual(TEXT("Reset clears time"),Sim.snapshot().time,0.0);
    UWorld* World=FAutomationEditorCommonUtils::CreateNewMap();
    if(!TestNotNull(TEXT("Editor map"),World)) return false;
    auto* Pawn=World->SpawnActor<AUAVSimPawn>();
    TestNotNull(TEXT("Product pawn spawned"),Pawn);
    if(Pawn) TestFalse(TEXT("Chaos cannot control drone"),Pawn->DroneMesh->IsSimulatingPhysics());
    if(Pawn)
    {
        Pawn->InMenu=false; Pawn->Paused=false;
        const auto PrepareHover=[Pawn]()
        {
            Pawn->ResetFlight();
            uavsim::State Initial; Initial.position_m.z()=1;
            Pawn->Simulation.reset(Initial,Pawn->Simulation.hover_rotor_speeds());
            Pawn->Input.mode=uavsim::FlightMode::Attitude;
        };
        PrepareHover(); Pawn->SetPilotAxes(1,0,0,0); Pawn->Tick(0.2f);
        TestTrue(TEXT("Actual pilot right command accelerates toward Unreal right"),UAVSimAdapter::Position(Pawn->Simulation.snapshot().state.velocity_mps).Y>0);
        PrepareHover(); Pawn->SetPilotAxes(0,1,0,0); Pawn->Tick(0.2f);
        TestTrue(TEXT("Actual pilot forward command accelerates forward"),Pawn->Simulation.snapshot().state.velocity_mps.x()>0);
        PrepareHover(); Pawn->SetPilotAxes(0,0,1,0); Pawn->Tick(0.2f);
        TestTrue(TEXT("Actual pilot right yaw turns nose right"),UAVSimAdapter::Position(Pawn->Simulation.snapshot().state.orientation*uavsim::Vec3::UnitX()).Y>0);
        TArray<uavsim::Snapshot> Results;
        for(int Rate : {30,60,144})
        {
            Pawn->ResetFlight(); Pawn->SetPilotAxes(0,0,0,0.5);
            for(int Frame=0;Frame<Rate;++Frame) Pawn->Tick(1.0f/Rate);
            Results.Add(Pawn->Simulation.snapshot());
        }
        for(const auto& Result : Results)
        {
            TestTrue(TEXT("Render rate preserves simulated duration within one fixed step"),FMath::Abs(Result.time-1.0)<=0.004001);
            TestTrue(TEXT("Held pilot input trajectory is render independent within one step"),(Result.state.position_m-Results[0].state.position_m).norm()<0.02);
        }
        Pawn->ResetFlight(); Pawn->SetPilotAxes(0,0,0,0); Pawn->Tick(0.5f);
        TestTrue(TEXT("500ms frame does not discard simulation time"),FMath::Abs(Pawn->Simulation.snapshot().time-0.5)<=0.004001);
        Pawn->TogglePause(); const auto PausedStep=Pawn->Simulation.snapshot().step; Pawn->Tick(0.5f);
        TestEqual(TEXT("Actual pawn pause stops stepping"),Pawn->Simulation.snapshot().step,PausedStep);
        Pawn->TogglePause(); Pawn->ResetFlight();
        TestEqual(TEXT("Actual pawn reset clears clock"),Pawn->Simulation.snapshot().time,0.0);
    }
    return !HasAnyErrors();
}
#endif
