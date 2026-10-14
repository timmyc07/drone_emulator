# Vendored dependencies

Unmodified source libraries are pinned and retained for reproducible offline builds. These licenses apply only to their respective libraries; they do not change the product or Unreal asset licenses.

| Library | Version / exact revision | License | Official source |
| --- | --- | --- | --- |
| Eigen | 5.0.0, tag commit `549bf8c75b6aae071cde2f28aa48f16ee3ae60b0` | MPL-2.0; see eigen/COPYING files | https://gitlab.com/libeigen/eigen |
| nlohmann/json | 3.12.0 | MIT | https://github.com/nlohmann/json/releases/tag/v3.12.0 |
| PicoSHA2 | `161cb3fc4170fa7a3eca9e582cebd27cc4d1fe29` | MIT | https://github.com/okdshin/PicoSHA2 |

Eigen 5.0.0 official archive SHA256: `795a5412b6ead1b82c43704b640d4642d75b5755a08d8f12b4f13615fea3f36d`.

nlohmann/json single header SHA256: `aaf127c04cb31c406e5b04a63f1ae89369fccde6d8fa7cdda1ed4f32dfc5de63`, checked against the official release checksum before use.

Only Eigen's `Eigen/` headers and applicable notices are included. No runtime dependency download or system installation is required by CMake.
