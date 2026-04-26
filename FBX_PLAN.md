# Plan: FBX Drop-in via Assimp + Skeletal Animation & GPU Skinning

## Context

Engine load `.gltf` only (`GameObject::LoadGLTF`, cgltf). Existing `AnimationComponent` drive **node-transform animation only** — no bone palette, no skin weights, no `mat4[]` uniform path, vertex layout has no bone slots. User want full FBX support **plus uplift**: skeletal animation + GPU skinning so rigged FBX (and rigged glTF) render correctly. Switch loader to **Assimp** — battle-tested in AAA, single API for FBX+glTF+OBJ+DAE, exposes bones/weights/animations cleanly.

This is engine surgery, not a sprinkle. Touches loader, vertex layout, mesh attrib setup, material uniform system, animation component, render queue, shaders. Plan in vertical layers so each compile.

## Architecture

```
                 ┌─────────────────────────┐
   FBX file ───▶ │  ModelImporter (Assimp) │
                 └──────────┬──────────────┘
                            │ produces
              ┌─────────────┼──────────────┐
              ▼             ▼              ▼
        SkinnedMesh    Skeleton    SkeletalAnimClip
        (verts +       (bones,     (per-bone TRS
         bone idx +     parent,     keyframe tracks)
         weights)       invBind)
              │             │              │
              └─────────┬───┴──────────────┘
                        ▼
              SkinnedMeshComponent ── owns ref ──▶ AnimationComponent
                        │                                │
                        │ submit                         │ Update():
                        ▼                                │   eval bones
                  RenderQueue                            │   compute palette
                        │                                │   write to material
                        ▼                                ▼
                  uBones[N] uniform   ◀──────  mat4[] param
```

## Files

### New thirdparty
- `engine/thirdparty/assimp/` — Assimp 5.x as git submodule or unpacked source. Header-only path (cgltf style) **not viable** — Assimp is a real library.

### New engine sources
- `engine/source/animation/Skeleton.h/.cpp` — `Bone { str name; i32 parentIndex; mat4 offsetMatrix; mat4 localBind; }`, `Skeleton { vector<Bone> bones; unordered_map<str, u32> nameToIndex; }`.
- `engine/source/animation/SkeletalAnimationClip.h/.cpp` — clip with per-bone-index `TransformTrack` (reuse `KeyFrameVec3/Quat`).
- `engine/source/animation/Pose.h/.cpp` — `Pose { vector<mat4> localTransforms; vector<mat4> finalPalette; }` + helpers `EvaluatePose(clip, t, pose)` and `ComputePalette(skeleton, pose)` (concat parent → multiply by `offsetMatrix`).
- `engine/source/scene/components/SkinnedMeshComponent.h/.cpp` — like `MeshComponent` but holds `shared_ptr<Skeleton>` + observer pointer to `AnimationComponent` for current palette. On Update push palette via `Material::SetParam("uBones", paletteVector)`.
- `engine/source/io/ModelImporter.h/.cpp` — Assimp wrapper. `static GameObject *Import(const str &path, Scene*)` dispatched by extension. Internal helpers: `ImportMesh`, `ImportSkeleton`, `ImportAnimations`, `BuildHierarchy`. Hides Assimp behind translation unit boundary.

### New shaders
- `assets/shaders/skinned.vert` — same as `lit_pixel.vert` but include `layout(location=4) in ivec4 aBoneIndices; layout(location=5) in vec4 aBoneWeights; uniform mat4 uBones[128];` and skin position+normal.
- `assets/shaders/skinned.frag` — copy of `lit_pixel.frag`.

### Modify
- `engine/CMakeLists.txt` (after line 98):
  ```cmake
  set(ASSIMP_BUILD_TESTS         OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_ASSIMP_TOOLS  OFF CACHE BOOL "" FORCE)
  set(ASSIMP_INSTALL             OFF CACHE BOOL "" FORCE)
  set(ASSIMP_NO_EXPORT           ON  CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_FBX_IMPORTER  ON  CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_GLTF_IMPORTER ON  CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_OBJ_IMPORTER  ON  CACHE BOOL "" FORCE)
  add_subdirectory(thirdparty/assimp)
  ```
  Add `assimp` to `target_link_libraries(Engine ...)` near line 149.

- `engine/source/graphics/VertexLayout.h:54` — extend slots:
  ```cpp
  static constexpr int BoneIndicesIndex = 4;  // layout(location=4) ivec4
  static constexpr int BoneWeightsIndex = 5;  // layout(location=5) vec4
  ```

- `engine/source/render/Mesh.cpp:30` and `:153` — branch on `element.type`: integer types (`GL_BYTE/UNSIGNED_BYTE/SHORT/UNSIGNED_SHORT/INT/UNSIGNED_INT`) use `glVertexAttribIPointer` (no normalization arg), float types stay on `glVertexAttribPointer`. Required so `aBoneIndices` reach shader as `ivec4` not `vec4`.

- `engine/source/render/Material.{h,cpp}` — add `std::unordered_map<str, std::vector<glm::mat4>> m_mat4ArrayParams;`, `SetParam(const str&, const std::vector<glm::mat4>&)`, and in `Material::Bind()` (Material.cpp:171) upload via `glUniformMatrix4fv(loc, count, GL_FALSE, ptr)`. Add `ShaderProgram::SetMat4Array(name, count, ptr)`.

- `engine/source/scene/components/AnimationComponent.{h,cpp}` — generalize: keep current `targetName→GameObject` path for unrigged glTF, ADD `SkeletalAnimationClip` lane that writes into a `Pose` owned by component. Component grows `shared_ptr<Skeleton> m_skeleton; Pose m_pose;` and `const vector<mat4>& GetPalette() const`. `SkinnedMeshComponent` reads palette each `Update()` after AnimationComponent has run (component update order: SkinnedMesh after Animation — verify `GameObject::Update` order matches insertion order, which it does at `GameObject.cpp:82`; just attach AnimationComponent before SkinnedMeshComponent).

- `engine/source/Constants.h:122` — add `inline constexpr const char *kSceneTypeFbx = "fbx"; inline constexpr const char *kSceneTypeModel = "model";` (the latter generic — Assimp dispatches by extension so all formats can use one type; keep `gltf` and `fbx` for backward compat).

- `engine/source/scene/Scene.cpp:151` — extend type switch: `kSceneTypeGltf | kSceneTypeFbx | kSceneTypeModel` → `ModelImporter::Import(path, this)`. Optionally retire `LoadGLTF` once Assimp-glTF parity confirmed (keep both initially, prefer Assimp for new assets).

- `engine/source/scene/GameObject.{h,cpp}` — declare `static GameObject *LoadModel(const str &path, Scene*)` thin wrapper around `ModelImporter::Import`. Leave `LoadGLTF` for now (deprecate later).

## ModelImporter behavior

```cpp
Assimp::Importer imp;
const aiScene* sc = imp.ReadFile(fullPath,
    aiProcess_Triangulate
  | aiProcess_GenSmoothNormals
  | aiProcess_CalcTangentSpace
  | aiProcess_LimitBoneWeights      // clamp to 4 weights/vert
  | aiProcess_JoinIdenticalVertices
  | aiProcess_ImproveCacheLocality
  | aiProcess_GlobalScale           // honor FBX unit (cm→m)
  | aiProcess_FlipUVs               // FBX/D3D vs GL convention; flag-test
  | aiProcess_PopulateArmatureData);
```

**Mesh import.** For each `aiMesh`:
- Build `VertexLayout` with Position, Normal, UV(0), Color(0) where present, plus BoneIndices(ivec4)+BoneWeights(vec4) if `mesh->HasBones()`.
- Walk `mesh->mBones[b]`. Resolve bone name → skeleton index (created on first sight, parent linked via `aiNode` walk). Store `offsetMatrix = mBones[b]->mOffsetMatrix`.
- Per vertex aggregate weights: each bone's `mWeights[w]` add `(weights[v], boneIdx)` into a fixed-size `(idx[4], w[4])` slot, sorted by weight descending, normalize sum to 1.0. (Triangulate done by assimp; `aiProcess_LimitBoneWeights` already cap at 4.)
- Pack interleaved float buffer respecting layout. Bone indices written as `i32` reinterpret in the float buffer (works because `glVertexAttribIPointer` reads raw bytes by stride/offset — verify alignment, OR use parallel `vector<u8>` buffer; cleaner to switch `Mesh` ctor to accept `vector<u8>` of raw bytes — out of scope, instead pack `int32` bits into `f32` slots via memcpy and rely on `IPointer` reading correct type).
- Index buffer from `aiFace::mIndices` (always 3 due to `aiProcess_Triangulate`).

**Material.** For each `aiMaterial`:
- `aiTextureType_DIFFUSE` → `baseColorTexture`.
- `aiTextureType_NORMALS` (or `aiTextureType_HEIGHT` for FBX legacy) → `normalTexture` (optional v1).
- Embedded textures (`scene->mTextures[idx]`): write to memory texture loader (extend `TextureManager` later if needed; v1 require external image files since current `TextureManager::GetOrLoadTexture(path)` is path-based).
- Pick shader: `skinned.{vert,frag}` if mesh has bones, else default.

**Skeleton.** Walk `aiScene::mRootNode`. For each node referenced by any mesh's bone list (or its ancestor up to common root), produce a `Bone`. Parent index resolved by recursion. `localBind` = `node->mTransformation`. `offsetMatrix` taken from any mesh's `aiBone` of that name. One skeleton per imported model in v1 (multi-skeleton later).

**Animations.** For each `aiAnimation`:
- `duration = mDuration / mTicksPerSecond` (handle `mTicksPerSecond==0` → default 25 fps).
- Per `aiNodeAnim`: resolve channel name → bone index (skip if not in skeleton — those still drive plain GameObjects via existing path).
- Convert `aiVectorKey/aiQuatKey` → `KeyFrameVec3/Quat` with time divided by ticksPerSecond.

**Hierarchy.** Walk `aiNode` tree. Each node → `GameObject` child. Meshes referenced by node attached as `MeshComponent` (or `SkinnedMeshComponent` if mesh has bones). Root gets `AnimationComponent` carrying skeleton + clips.

## Render path

`SkinnedMeshComponent::Update`:
```cpp
const auto& palette = m_anim->GetPalette();   // mat4 vector, size = bones.size()
m_material->SetParam("uBones", palette);
RenderQueue::Submit({m_mesh.get(), m_material.get(), m_owner->GetWorldTransform()});
```

Skinned vertex shader:
```glsl
layout(location=0) in vec3 aPosition;
layout(location=3) in vec3 aNormal;
layout(location=4) in ivec4 aBoneIndices;
layout(location=5) in vec4  aBoneWeights;
uniform mat4 uBones[128];

void main() {
  mat4 skin = uBones[aBoneIndices.x] * aBoneWeights.x
            + uBones[aBoneIndices.y] * aBoneWeights.y
            + uBones[aBoneIndices.z] * aBoneWeights.z
            + uBones[aBoneIndices.w] * aBoneWeights.w;
  vec4 wpos = uModel * skin * vec4(aPosition, 1.0);
  vec3 wnrm = mat3(uModel * skin) * aNormal;
  ...
}
```

Cap MAX_BONES=128 v1 (uniform array). Future: SSBO/UBO for >128.

## Reuse

- `Mesh` ctor (`engine/source/render/Mesh.cpp:10`) — same call site, just enriched layout.
- `Material::SetParam` family — extend, not replace.
- `TextureManager::GetOrLoadTexture` (`GameObject.cpp:396`) — unchanged.
- `KeyFrameVec3/Quat`, `TransformTrack` (`AnimationComponent.h:52-79`) — reused for skel tracks (targetName field replaced by boneIndex in skel variant — keep struct, treat name as bone name).
- `Engine::GetInstance().GetFileSystem().GetAssetsFolder()` for path resolution.
- `lit_pixel.{vert,frag}` as base for skinned shaders.

## Verification

1. **Build:** `./compile.sh` clean. Assimp first-build is slow (~1–2 min); subsequent incrementals fast.
2. **Static glTF parity:** load existing `models/carbine/scene.gltf` via new `ModelImporter` (temporarily flip `Scene.cpp` gltf branch). Verify visually identical to current cgltf path. Catches mesh/material/transform regressions before introducing skinning.
3. **Static FBX:** drop a non-rigged FBX (e.g. crate) into `assets/models/crate.fbx`, scene JSON `{ "type": "fbx", "path": "models/crate.fbx" }`. Confirm renders, correct scale (FBX units cm), correct UV orientation (toggle `aiProcess_FlipUVs` if upside-down).
4. **Rigged FBX, bind pose:** load rigged character (e.g. Mixamo `.fbx`) without playing animation. Bone palette = identity-ish (all bones at bind). Confirm character not exploded — proves skin weights + offset matrices correct. Inspect with `LOG_INFO` of bone count, vertex weight sums (should ≈1.0).
5. **Rigged FBX + animation:** call `anim->Play("mixamo.com", true)`. Confirm smooth playback, no jitter (slerp working), no stretching (palette compose order: `parentWorld * localBind * trsKey * offsetMatrix` — verify by toggling each multiply).
6. **Multi-mat mesh:** asset with 2+ materials per mesh. Assimp splits into separate `aiMesh`; verify each draws with right texture.
7. **Stress:** load multiple rigged FBX in same scene, confirm no uniform-name collisions (each `Material` is per-instance).
8. **Edge cases:**
   - `mTicksPerSecond == 0` (some FBX exporters): fallback handled.
   - `>4 weights/vert`: `aiProcess_LimitBoneWeights` clamps; verify normalization re-sums to 1.
   - `>128 bones`: log error, refuse load with clear message (until SSBO upgrade).
   - Embedded textures: log warning skip in v1.
9. **Smoke run:** `./compile.sh` and run `dungeon.json` (existing scene) — confirm no regression to non-FBX rendering.

## Out of scope (v1)

Inverse kinematics, blend trees, animation state machines, root motion extraction, morph targets / blendshapes, embedded FBX textures (require in-memory `stbi_load_from_memory` path on `TextureManager`), dual-quaternion skinning, compute-shader skinning, multi-skeleton merge, retargeting between skeletons. All cleanly buildable on top of v1 once Pose + Skeleton land.

## Risks / call-outs

- **Assimp build cost:** adds ~minute to clean builds, ~6MB to binary. Acceptable for offline tool path; if hot iteration suffers, revisit by precompiling Assimp as static lib once, never rebuild.
- **Bone-index packing into float VBO:** technique works but subtle. Cleaner long-term: refactor `Mesh` to take a `vector<u8>` raw byte buffer + layout. Defer the refactor; document the memcpy hack at the call site.
- **`offsetMatrix` semantics:** Assimp's offset = inverse bind in mesh space. Convention here must be `finalPalette[b] = globalBoneTransform[b] * offsetMatrix[b]`. Test thoroughly with a known-good Mixamo rig.
- **Coordinate / unit conversions:** FBX often Y-up cm, glTF Y-up m. `aiProcess_GlobalScale` + Assimp's metadata read should normalize. If models render too big/small, expose an import scale param on the scene JSON entry.
