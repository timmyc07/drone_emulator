#include "UAVSimRuntime.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ComboBoxString.h"
#include "Components/InputKeySelector.h"
#include "Components/ScrollBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/PackageName.h"
#include "Misc/ConfigCacheIni.h"

TSharedRef<SWidget> UUAVSimWidget::RebuildWidget()
{
    if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this);
    KeySelectors.Reset();
    auto* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;
    const auto Text = [this](UPanelWidget* Parent, const FString& Value, int32 Size = 18)
    {
        auto* Item = WidgetTree->ConstructWidget<UTextBlock>();
        Item->SetText(FText::FromString(Value));
        auto Font = Item->GetFont(); Font.Size = Size; Item->SetFont(Font);
        Item->SetAutoWrapText(true); Item->SetColorAndOpacity(FSlateColor(FLinearColor(0.90f,0.95f,1.0f)));
        Parent->AddChild(Item); return Item;
    };
    const auto Button = [this, &Text](UPanelWidget* Parent, const FString& Label, FName Function)
    {
        auto* Item = WidgetTree->ConstructWidget<UButton>();
        Item->SetBackgroundColor(FLinearColor(0.08f,0.19f,0.25f));
        Text(Item, Label, 18);
        FScriptDelegate Action; Action.BindUFunction(this, Function); Item->OnClicked.Add(Action);
        Parent->AddChild(Item);
    };
    const auto Panel = [this, Canvas](FVector2D Min, FVector2D Max)
    {
        auto* Border = WidgetTree->ConstructWidget<UBorder>();
        Border->SetBrushColor(FLinearColor(0.015f,0.035f,0.05f,0.92f));
        Border->SetPadding(FMargin(16));
        auto* Slot = Canvas->AddChildToCanvas(Border);
        Slot->SetAnchors(FAnchors(Min.X, Min.Y, Max.X, Max.Y)); Slot->SetOffsets(FMargin(0));
        auto* Scroll = WidgetTree->ConstructWidget<UScrollBox>(); Border->SetContent(Scroll);
        auto* Column = WidgetTree->ConstructWidget<UVerticalBox>(); Scroll->AddChild(Column);
        return Column;
    };
    auto* Hud = Panel(FVector2D(0.025,0.03), FVector2D(0.60,0.30));
    Readout = Text(Hud, TEXT("UAVSIM"), 20);
    Menu = Panel(FVector2D(0.63,0.03), FVector2D(0.975,0.94));
    Text(Menu, TEXT("FLIGHT DESK"), 28);
    Text(Menu, TEXT("Aircraft"), 20);
    auto* Aircraft = WidgetTree->ConstructWidget<UComboBoxString>();
    Aircraft->AddOption(TEXT("Generic Quadrotor X-01 / Sci-Fi Drone")); Aircraft->SetSelectedIndex(0); Menu->AddChild(Aircraft);
    Text(Menu, TEXT("1.5 kg  /  X quadrotor\nEngineering estimate; visual model is cosmetic.\nRate, attitude and altitude-hold controls."));
    Text(Menu, TEXT("Environment"), 20);
    EnvironmentChoice = WidgetTree->ConstructWidget<UComboBoxString>();
    EnvironmentChoice->AddOption(TEXT("Open test range"));
    if (FPackageName::DoesPackageExist(TEXT("/Game/Levels/ElectricDreams_Env"))) EnvironmentChoice->AddOption(TEXT("Electric Dreams"));
    EnvironmentChoice->SetSelectedIndex(GetWorld()->GetMapName().Contains(TEXT("ElectricDreams")) ? 1 : 0); Menu->AddChild(EnvironmentChoice);
    Text(Menu, TEXT("Clear weather / calm wind / start at local origin.\nTerrain and obstacle contact are not modeled; flat ground contact only."));
    Button(Menu, TEXT("START SIMULATION"), TEXT("StartClicked"));
    Button(Menu, TEXT("Replay latest saved session"), TEXT("ReplayClicked"));
    Text(Menu, TEXT("Settings"), 23);
    GraphicsChoice = WidgetTree->ConstructWidget<UComboBoxString>();
    for (const auto* Value : {TEXT("Low"),TEXT("Medium"),TEXT("High"),TEXT("Epic")}) GraphicsChoice->AddOption(Value);
    GraphicsChoice->SetSelectedIndex(FMath::Clamp(GEngine->GetGameUserSettings()->GetOverallScalabilityLevel(),0,3)); Menu->AddChild(GraphicsChoice);
    Text(Menu,TEXT("Window resolution"));
    ResolutionChoice=WidgetTree->ConstructWidget<UComboBoxString>();
    for(const auto* Value : {TEXT("1280 x 720"),TEXT("1920 x 1080"),TEXT("2560 x 1440")}) ResolutionChoice->AddOption(Value);
    const auto CurrentResolution=GEngine->GetGameUserSettings()->GetScreenResolution();
    ResolutionChoice->SetSelectedIndex(CurrentResolution.X>=2560 ? 2 : (CurrentResolution.X>=1920 ? 1 : 0)); Menu->AddChild(ResolutionChoice);
    Text(Menu,TEXT("Display units"));
    UnitsChoice=WidgetTree->ConstructWidget<UComboBoxString>(); UnitsChoice->AddOption(TEXT("Metres / m per second")); UnitsChoice->AddOption(TEXT("Feet / ft per second")); UnitsChoice->SetSelectedIndex(Drone && Drone->ImperialUnits ? 1 : 0); Menu->AddChild(UnitsChoice);
    Text(Menu, TEXT("Control sensitivity"));
    SensitivityControl = WidgetTree->ConstructWidget<USlider>(); SensitivityControl->SetMinValue(0.1f); SensitivityControl->SetMaxValue(1); SensitivityControl->SetValue(Drone ? Drone->Sensitivity : 0.6f); Menu->AddChild(SensitivityControl);
    Text(Menu,TEXT("Flight bindings (click a key to change)"));
    const TCHAR* Labels[]={TEXT("Pitch forward"),TEXT("Pitch back"),TEXT("Roll right"),TEXT("Roll left"),TEXT("Yaw right"),TEXT("Yaw left"),TEXT("Climb / more thrust"),TEXT("Descend / less thrust")};
    for(int32 I=0;I<8;++I)
    {
        Text(Menu,Labels[I],16);
        auto* Selector=WidgetTree->ConstructWidget<UInputKeySelector>();
        Selector->SetAllowModifierKeys(false); Selector->SetAllowGamepadKeys(false);
        if(Drone) Selector->SetSelectedKey(FInputChord(Drone->FlightKeys[I]));
        Menu->AddChild(Selector); KeySelectors.Add(Selector);
    }
    Button(Menu, TEXT("Apply and save settings"), TEXT("ApplyClicked"));
    Text(Menu, TEXT("Controls"), 23);
    Text(Menu, TEXT("Default: W/S pitch, A/D roll, Q/E yaw.\nDefault: Space / Left Ctrl climb / descend.\nF: arm / disarm   M: change mode\nP: pause / resume   R: reset   Esc: menu\nGamepad: left stick yaw/climb, right stick tilt.\nKeyboard is active after clicking the world.\nIn rate/attitude mode, climb keys set collective thrust.\nGamepad hardware not yet verified."));
    Button(Menu, TEXT("Exit application"), TEXT("QuitClicked"));
    auto* Bottom = Panel(FVector2D(0.025,0.75), FVector2D(0.60,0.97));
    Message = Text(Bottom, TEXT("Ready"), 17);
    auto* Actions = WidgetTree->ConstructWidget<UWrapBox>(); Actions->SetInnerSlotPadding(FVector2D(6,6)); Bottom->AddChild(Actions);
    Button(Actions,TEXT("Pause / Resume"),TEXT("PauseClicked"));
    Button(Actions,TEXT("Reset"),TEXT("ResetClicked"));
    Button(Actions,TEXT("Arm"),TEXT("ArmClicked"));
    Button(Actions,TEXT("Mode"),TEXT("ModeClicked"));
    Button(Actions,TEXT("Save session"),TEXT("SaveClicked"));
    Button(Actions,TEXT("Menu"),TEXT("MenuClicked"));
    return Super::RebuildWidget();
}

void UUAVSimWidget::NativeTick(const FGeometry& Geometry, float DeltaSeconds)
{
    Super::NativeTick(Geometry, DeltaSeconds);
    if (!Drone) return;
    Readout->SetText(FText::FromString(Drone->StatusText()));
    Message->SetText(FText::FromString(Drone->Notice));
    // Collapse the panel's border too; otherwise it would obscure the flight view.
    if (Menu && Menu->GetParent() && Menu->GetParent()->GetParent()) Menu->GetParent()->GetParent()->SetVisibility(Drone->InMenu ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
void UUAVSimWidget::StartClicked()
{
    if (!Drone) return;
    const bool Electric = EnvironmentChoice->GetSelectedOption() == TEXT("Electric Dreams");
    const bool CurrentElectric = GetWorld()->GetMapName().Contains(TEXT("ElectricDreams"));
    if (Electric != CurrentElectric)
    {
        UE_LOG(LogUAVSim,Display,TEXT("Environment change requested: %s"),*EnvironmentChoice->GetSelectedOption());
        UGameplayStatics::OpenLevel(this, Electric ? FName(TEXT("/Game/Levels/ElectricDreams_Env")) : FName(TEXT("/Game/Maps/UAVSim_Test")),true,TEXT("game=/Script/UAVSimRuntime.UAVSimGameMode"));
        return;
    }
    Drone->StartFlight();
}
void UUAVSimWidget::PauseClicked() { if (Drone) Drone->TogglePause(); }
void UUAVSimWidget::ResetClicked() { if (Drone) Drone->ResetFlight(); }
void UUAVSimWidget::MenuClicked() { if (Drone) Drone->ReturnToMenu(); }
void UUAVSimWidget::ArmClicked() { if (Drone) Drone->ToggleArm(); }
void UUAVSimWidget::ModeClicked() { if (Drone) Drone->CycleMode(); }
void UUAVSimWidget::SaveClicked() { if (Drone) Drone->SaveSession(); }
void UUAVSimWidget::ReplayClicked() { if (Drone) Drone->ReplaySession(); }
void UUAVSimWidget::ApplyClicked()
{
    TSet<FKey> UsedKeys;
    for(const auto& Selector : KeySelectors)
    {
        const FKey Key=Selector->GetSelectedKey().Key;
        if(!Key.IsValid() || UsedKeys.Contains(Key) || Key==EKeys::P || Key==EKeys::R || Key==EKeys::M || Key==EKeys::F || Key==EKeys::Escape)
        {
            if(Drone) Drone->Notice=TEXT("Each flight key must be unique; P, R, M, F and Esc are reserved.");
            UE_LOG(LogUAVSim,Warning,TEXT("Settings rejected: invalid or conflicting key binding")); return;
        }
        UsedKeys.Add(Key);
    }
    auto* Settings = GEngine->GetGameUserSettings();
    Settings->SetOverallScalabilityLevel(GraphicsChoice->GetSelectedIndex());
    const FIntPoint Resolutions[]={FIntPoint(1280,720),FIntPoint(1920,1080),FIntPoint(2560,1440)};
    Settings->SetFullscreenMode(EWindowMode::Windowed); Settings->SetScreenResolution(Resolutions[FMath::Clamp(ResolutionChoice->GetSelectedIndex(),0,2)]);
    Settings->ApplySettings(false); Settings->SaveSettings();
    if (Drone)
    {
        Drone->ImperialUnits=UnitsChoice->GetSelectedIndex()==1;
        GConfig->SetBool(TEXT("UAVSim"),TEXT("ImperialUnits"),Drone->ImperialUnits,GGameUserSettingsIni);
        for(int32 I=0;I<KeySelectors.Num();++I) { Drone->FlightKeys[I]=KeySelectors[I]->GetSelectedKey().Key; GConfig->SetString(TEXT("UAVSim"),*FString::Printf(TEXT("FlightKey%d"),I),*Drone->FlightKeys[I].ToString(),GGameUserSettingsIni); }
        Drone->SetSensitivity(SensitivityControl->GetValue()); Drone->Notice = TEXT("Graphics, units and input settings saved.");
    }
    UE_LOG(LogUAVSim,Display,TEXT("User settings applied"));
}
void UUAVSimWidget::QuitClicked() { UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false); }
