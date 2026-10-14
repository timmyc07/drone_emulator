# Asset provenance and handling

## Active browser delivery — 2026-10-14

The user explicitly retained the same supplied ZIP drone as the browser game's default appearance. Its source FBX (`QuadDrone [mesh].fbx`) and six PNG textures remain under ignored `local-assets/drone`; browser assembly copies them only into ignored `dist/web`. Keep originals intact. The six maps provide diffuse, yellow emission, gloss, metalness, normals and occlusion. Actual Chrome inspection showed the supplied model with red/white surfaces and all six material maps; this browser evidence is separate from the historical Unreal material failure. Successful file loading alone is insufficient for visual acceptance.

The supplied archive's redistribution terms are still unverified. Do not commit its FBX, textures, converted derivatives or generated bundle to GitHub and do not publish them to a hosted site. Local use authorized by the user does not establish public redistribution permission. These graphics do not calibrate the reference vehicle's mass, inertia, propulsion or flight behavior.

Electric Dreams is excluded from the browser. Its [official Fab listing](https://www.fab.com/listings/d79688f5-29be-4fb2-a650-2d4a813f5306) designates the content UE-only, and the [Epic content terms](https://www.unrealengine.com/eula/content) restrict that category to Unreal-based products. Do not export/convert the acquired content into the browser game. The browser forest uses independently created geometry; local Electric Dreams files remain in ignored `local-assets/`.

## Historical Unreal asset work

The Unreal application and its generated content were removed by user direction on 2026-10-14. Historical import/package results below remain for provenance only. The supplied source drone and Electric Dreams files under ignored `local-assets/` were preserved; only the prepared browser bundle uses the drone in current development.

The historical Generic Quadrotor X-01 referenced `/Game/Drone/SM_DefaultDrone`. The user supplied an archive containing a drone FBX and six textures, extracted locally under ignored `local-assets/drone`. FBX and textures were imported for the former Unreal app; its generated project content was removed on 2026-10-14. First-package visual acceptance FAILED: `M_DefaultDrone` had an unconnected OneMinus input, failed compilation and rendered with a white fallback material. These records describe the retired desktop app. The browser currently loads the original FBX and maps directly. The visual model does not calibrate mass, inertia, propulsion or control parameters and does not establish real-aircraft fidelity.

Externally supplied archives must be inspected for their original license, attribution and permitted redistribution before publication. Local import for the user-authorized evaluation does not establish redistribution permission. Possession of an archive is not evidence of a redistribution license. Record archive/content hashes and license text when a specific archive is adopted. Do not commit raw third-party `.uasset`, `.umap`, meshes or textures until their actual terms have been checked. Git LFS handles size; it does not grant permission.

Historical desktop scope: Electric Dreams content was acquired into ignored `local-assets/ElectricDreamsEnv`. It was not integrated into the browser game; import, compatibility, rendering and performance claims remain unverified. Epic content terms restrict source-format sharing; evaluate the actual asset entitlement and applicable license before any distribution.

Sources checked during the preimplementation review:

- [Electric Dreams official requirements](https://dev.epicgames.com/documentation/en-us/unreal-engine/electric-dreams-environment-in-unreal-engine)
- [Epic Content License Agreement](https://www.unrealengine.com/eula/content)
- [UE packaging guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project)

The supplied archive's redistribution license is unverified. Its original FBX/textures remain local; the generated Unreal copy was removed with the retired project. Do not publish them to GitHub. Electric Dreams source content likewise remains in ignored `local-assets/` pending the applicable entitlement/license review. Successful local drone import does not establish redistribution permission. Electric Dreams is not used by the browser game.
