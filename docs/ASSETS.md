# Asset provenance and handling

The Generic Quadrotor X-01 references `/Game/Drone/SM_DefaultDrone`. The user supplied an archive containing a drone FBX and six textures, extracted locally under ignored `local-assets/drone`. FBX and all six textures have been imported successfully, with the import PASS marker captured in the local Unreal logs. The Unreal pawn targets the imported mesh. Rendered appearance and product-scene acceptance remain separately tracked in PROJECT_STATUS. The visual model does not calibrate mass, inertia, propulsion or control parameters and does not establish real-aircraft fidelity.

Externally supplied archives must be inspected for their original license, attribution and permitted redistribution before publication. Local import for the user-authorized evaluation does not establish redistribution permission. Possession of an archive is not evidence of a redistribution license. Record archive/content hashes and license text when a specific archive is adopted. Do not commit raw third-party `.uasset`, `.umap`, meshes or textures until their actual terms have been checked. Git LFS handles size; it does not grant permission.

The user requested Electric Dreams as the product environment. Acquisition through Epic Launcher into ignored `local-assets/ElectricDreamsEnv` is in progress; download completion, import, compatibility, rendering and performance are NOT_TESTED until actual validation. Epic lists 64 GB recommended and 32 GB minimum system memory. Core physics is first tested in a simple scene, then the requested environment must be assessed on this 32 GB host. Epic content terms restrict source-format sharing; evaluate the actual asset entitlement and applicable license before distribution. A cooked application and an editable content repository have different distribution implications.

Sources checked during the preimplementation review:

- [Electric Dreams official requirements](https://dev.epicgames.com/documentation/en-us/unreal-engine/electric-dreams-environment-in-unreal-engine)
- [Epic Content License Agreement](https://www.unrealengine.com/eula/content)
- [UE packaging guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project)

The supplied archive's redistribution license is unverified. Its FBX/textures and generated Unreal content remain local; do not publish them to GitHub. Electric Dreams content likewise remains local pending the applicable entitlement/license review. Successful local drone import does not establish redistribution permission; Electric Dreams import and runtime remain unverified while acquisition is in progress.
