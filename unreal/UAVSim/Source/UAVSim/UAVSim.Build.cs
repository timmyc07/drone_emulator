using UnrealBuildTool;
public class UAVSim : ModuleRules
{
    public UAVSim(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "UAVSimRuntime" });
    }
}
