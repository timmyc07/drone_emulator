using UnrealBuildTool;
public class P0ProbeTarget : TargetRules
{
    public P0ProbeTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("P0Probe");
    }
}
