# UAVSim browser game

UAVSim is a local browser drone game. It runs the portable C++20 flight core through WebAssembly and renders the user's supplied drone with Three.js. The current browser checks and limits are in [PROJECT_STATUS.md](docs/PROJECT_STATUS.md); operating instructions are in [WEB_GAME.md](docs/WEB_GAME.md).

## Build and launch

Follow the pinned Node.js, Emscripten and Three.js setup in [WEB_GAME.md](docs/WEB_GAME.md). From the repository root:

```powershell
node tools/web/prepare-vendor.mjs
pwsh -NoProfile -File tools/web/build-core.ps1
node tools/web/build-site.mjs
node tools/web/serve.mjs
```

Open `http://127.0.0.1:4173/`. The server exposes only prepared `dist/web` files. Do not start a generic static server from the repository root or publish the local drone assets; their redistribution license is unverified.

## Core validation

Run the native core checks on the matching host:

```powershell
./tools/validation/validate_windows.ps1
```

```sh
bash tools/validation/validate_macos.sh
```

Browser-specific checks and captured evidence are documented in [WEB_GAME.md](docs/WEB_GAME.md) and [PROJECT_STATUS.md](docs/PROJECT_STATUS.md). Native tests and CI validate the portable core; they do not establish browser rendering or interaction acceptance.

## Project records

- [Assets and licensing](docs/ASSETS.md)
- [Dependencies](docs/DEPENDENCIES.md)
- [Engineering decisions](docs/DECISIONS.md)
- [Roadmap](docs/ROADMAP.md)

The former Unreal desktop application and its Unreal-specific build, package and validation tools have been retired. Historical acceptance evidence remains in the project records and local `reports/` directory.
