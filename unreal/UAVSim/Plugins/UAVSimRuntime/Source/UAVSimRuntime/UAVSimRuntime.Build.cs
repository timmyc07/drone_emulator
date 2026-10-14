using UnrealBuildTool;
using System.IO;
public class UAVSimRuntime : ModuleRules
{
    public UAVSimRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bEnableExceptions = true;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore", "Json" });
        string Root = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../../../.."));
        PublicIncludePaths.Add(Path.Combine(Root, "include"));
        PublicSystemIncludePaths.Add(Path.Combine(Root, "third_party/eigen"));
        PrivateIncludePaths.Add(Path.Combine(Root, "third_party"));
        RuntimeDependencies.Add("$(ProjectDir)/Config/Vehicles/generic_quadrotor_x01.json", Path.Combine(Root, "configs/vehicles/generic_quadrotor_x01.json"), StagedFileType.NonUFS);
        if (Target.bBuildEditor) PrivateDependencyModuleNames.AddRange(new[] { "UnrealEd" });
    }
}
