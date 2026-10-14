# Asset provenance and handling

## Active browser delivery — 2026-10-14

The user explicitly retained the same supplied ZIP drone as the browser game's default appearance. Its source FBX (`QuadDrone [mesh].fbx`) and six PNG textures remain under ignored `local-assets/drone`; browser assembly copies them only into ignored `dist/web`. Keep originals intact. The six maps provide diffuse, yellow emission, gloss, metalness, normals and occlusion. Actual Chrome inspection showed the supplied model with red/white surfaces and all six material maps; this browser evidence is separate from the historical Unreal material failure. Successful file loading alone is insufficient for visual acceptance.

The supplied archive's redistribution terms are still unverified. Do not commit its FBX, textures, converted derivatives or generated bundle to GitHub and do not publish them to a hosted site. Local use authorized by the user does not establish public redistribution permission. These graphics do not calibrate the reference vehicle's mass, inertia, propulsion or flight behavior.

Electric Dreams integration is suspended following the user's switch away from Unreal. Its [official Fab listing](https://www.fab.com/listings/d79688f5-29be-4fb2-a650-2d4a813f5306) explicitly designates the content UE-only. The [Epic content terms](https://www.unrealengine.com/eula/content) restrict that category to Unreal-based products. Do not export/convert the acquired content into the browser game. The browser forest uses independently created geometry; local Electric Dreams files and earlier migration evidence are preserved without continuing migration.

## Historical Unreal asset work

The Generic Quadrotor X-01 references `/Game/Drone/SM_DefaultDrone`. The user supplied an archive containing a drone FBX and six textures, extracted locally under ignored `local-assets/drone`. FBX and all six textures have been imported successfully, with the import PASS marker captured in the local Unreal logs. The Unreal pawn targets the imported mesh. First-package visual acceptance FAILED: `M_DefaultDrone` had an unconnected OneMinus input, failed compilation and rendered with a white fallback material. The importer and packaging checks are being repaired; the earlier import marker did not establish material/rendering correctness. Product-scene acceptance is tracked in PROJECT_STATUS. The visual model does not calibrate mass, inertia, propulsion or control parameters and does not establish real-aircraft fidelity.

Externally supplied archives must be inspected for their original license, attribution and permitted redistribution before publication. Local import for the user-authorized evaluation does not establish redistribution permission. Possession of an archive is not evidence of a redistribution license. Record archive/content hashes and license text when a specific archive is adopted. Do not commit raw third-party `.uasset`, `.umap`, meshes or textures until their actual terms have been checked. Git LFS handles size; it does not grant permission.

The user requested Electric Dreams as the product environment. Acquisition through Epic Launcher into ignored `local-assets/ElectricDreamsEnv` is in progress; download completion, import, compatibility, rendering and performance are NOT_TESTED until actual validation. Epic lists 64 GB recommended and 32 GB minimum system memory. Core physics is first tested in a simple scene, then the requested environment must be assessed on this 32 GB host. Epic content terms restrict source-format sharing; evaluate the actual asset entitlement and applicable license before distribution. A cooked application and an editable content repository have different distribution implications.

Sources checked during the preimplementation review:

- [Electric Dreams official requirements](https://dev.epicgames.com/documentation/en-us/unreal-engine/electric-dreams-environment-in-unreal-engine)
- [Epic Content License Agreement](https://www.unrealengine.com/eula/content)
- [UE packaging guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project)

The supplied archive's redistribution license is unverified. Its FBX/textures and generated Unreal content remain local; do not publish them to GitHub. Electric Dreams content likewise remains local pending the applicable entitlement/license review. Successful local drone import does not establish redistribution permission; Electric Dreams import and runtime remain unverified while acquisition is in progress.
