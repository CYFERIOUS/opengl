# CGMASTER — OpenGL Workshop

C++ / OpenGL teaching repository for computer graphics. Each **git branch** is a milestone: check out a branch to get the code at that stage of the course.

Remote: [github.com/CYFERIOUS/opengl](https://github.com/CYFERIOUS/opengl)

---

## Branch map

```
setup and window seed
        │
        ├── master                 → bare GLFW/GLAD window + .gitignore
        │       └── vcpkgInstall   → same seed, dependencies via vcpkg
        │
        └── origin vector lines
                │
                ├── introCG        → 2D raster (Bresenham) + ImGui
                │
                └── originClass    → 3D world, GLM, responsive viewport
                        │
                        └── CG     → textures, camera, glTF models  ← current
```

| Branch | Role | Focus |
|--------|------|--------|
| **`master`** | Project seed | Minimal GLFW window, GLAD loader, Visual Studio solution |
| **`vcpkgInstall`** | Dependency tooling | Manifest-mode vcpkg (`glfw3`, `glad`, `glm`, `stb`, `fastgltf`) |
| **`originClass`** | 3D workshop base | Coordinate axes (`Origin`), GLM math, shaders, depth / responsive window |
| **`introCG`** | 2D / raster track | Pixel grid, Bresenham line & circle, ImGui color controls |
| **`CG`** | Main 3D track (tip) | Textures, WASD + mouse-look camera, glTF loading (character models) |

Default remote branch is `master`. Day-to-day development for the 3D pipeline lives on **`CG`**.

---

## What each branch contains

### `master`
Starting point after the first window setup.

- `main.cpp` — create window, GL context
- Bundled headers under `libraries/include/` (GLFW, GLAD)
- `opengl.sln` / project files
- `.gitignore` for VS / build / vcpkg artifacts

### `vcpkgInstall`
Same seed as `master`, but dependencies are declared for [vcpkg](https://vcpkg.io/) instead of (or in addition to) vendored includes.

- `vcpkg.json` — package manifest  
- `vcpkg-configuration.json` — registry / baseline  

Use this branch when you want to install packages with:

```bash
vcpkg install --triplet x64-windows
```

### `originClass`
First shared 3D workshop after drawing origin axes.

- `Origin.h` / `origin.cpp` — VAO/VBO helper for XYZ axis lines  
- GLM for transforms  
- `shaders/vertex.vert`, `shaders/fragment.frag`  
- Responsive framebuffer + triangle depth (Z)

### `introCG`
Parallel track for **2D rasterization** and UI (forks after origin lines; does not continue into `CG`).

- Bresenham line and circle on a pixel-style grid  
- Shader helpers (`ShaderClass.h`, `shaderClass.cpp`)  
- Dear ImGui for interactive line color  
- Vendored ImGui under `libraries/include/imgui/`

Local tip may be one commit ahead of `origin/introCG` (ImGui line-color UI).

### `CG` (current)
Continuation of `originClass` into a small 3D viewer.

| Area | Files / folders |
|------|------------------|
| Entry | `main.cpp` |
| Shaders | `Shaders.h` / `Shaders.cpp`, `shaders/*.glsl` |
| Axes | `Origin.h` / `origin.cpp` |
| Models | `GltfModel.h` / `GltfModel.cpp`, `models/*/` (glTF + textures) |
| Images | `images/` |
| Deps | `vcpkg.json` (+ some headers still under `libraries/include/`) |

**Controls (typical):** WASD move, mouse look (yaw/pitch).

---

## Suggested learning path

1. **`master`** — window + context  
2. **`vcpkgInstall`** (optional) — manage libraries with vcpkg  
3. **`originClass`** — axes, GLM, basic 3D  
4. Then either:  
   - **`introCG`** — 2D algorithms + ImGui, or  
   - **`CG`** — textures, camera, glTF  

```bash
git checkout master
git checkout originClass
git checkout introCG   # 2D track
git checkout CG        # 3D track
```

---

## Build notes

- **IDE:** Visual Studio solution `opengl.sln` (Windows).  
- **Packages:** Prefer `vcpkg.json` on `vcpkgInstall` / `CG`. Older branches may ship headers under `libraries/include/`.  
- **Ignore:** Build outputs, `.vs/`, and `vcpkg_installed/` are gitignored; glTF `scene.bin` buffers under `models/` are kept.

---

## Naming note

- Git remote name: `origin`  
- C++ axis helper: `Origin` / `origin.cpp`  

They are unrelated; do not confuse the remote with the geometry class.
