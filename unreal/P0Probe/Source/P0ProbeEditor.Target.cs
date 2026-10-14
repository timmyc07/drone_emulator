using UnrealBuildTool;
public class P0ProbeEditorTarget : TargetRules
{
    public P0ProbeEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("P0Probe");
    }
}
