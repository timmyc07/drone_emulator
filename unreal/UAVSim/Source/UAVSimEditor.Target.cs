using UnrealBuildTool;
public class UAVSimEditorTarget : TargetRules
{
    public UAVSimEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("UAVSim");
    }
}
