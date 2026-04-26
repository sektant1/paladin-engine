# Authoring Guide

How to extend `monad-engine`: add models, drive animations, wire actions, build new gameplay systems, drop in enemies, and lay out dungeons. Pairs with `CLAUDE.md` (engine architecture) — read that first if unfamiliar with the lifecycle.

---

## 1. Adding Models

### 1.1 Static models (no rig)

The engine accepts **glTF**, **FBX**, and **OBJ**. glTF goes through the legacy cgltf path; FBX/OBJ/anything-else goes through the Assimp-backed `ModelImporter` (`engine/source/io/ModelImporter.cpp`).

Drop the asset folder under `assets/models/`:

```
assets/models/crate/
  crate.fbx
  crate_albedo.png
```

Reference it from a scene JSON file (`assets/scenes/*.json`):

```json
{
  "name": "WoodCrate",
  "type": "fbx",
  "path": "models/crate/crate.fbx",
  "position": { "x": 0, "y": 1, "z": 0 },
  "scale":    { "x": 1, "y": 1, "z": 1 }
}
```

The `type` value picks the loader:

| value | loader | notes |
|-------|--------|-------|
| `gltf` | cgltf (legacy) | unchanged glTF path |
| `fbx`  | Assimp | recommended for FBX |
| `model` | Assimp | format auto-detected from extension; use for `.obj`, `.dae`, etc. |

The importer applies `aiProcess_Triangulate | GenSmoothNormals | LimitBoneWeights | JoinIdenticalVertices | ImproveCacheLocality | GlobalScale | PopulateArmatureData`. FBX units (typically cm) are folded into the transform via `GlobalScale`.

### 1.2 Texture resolution rules

`baseColorTexture` (diffuse) is the only texture wired automatically. Filenames are resolved against the **model's folder**, with the directory part of any absolute Windows-style path stripped — so an FBX exported with `D:\proj\arms\arm_albedo_pale.png` still loads correctly from `assets/models/arms/arm_albedo_pale.png`.

Embedded textures (`*0`, `*1`, …) are not yet supported — keep textures as sidecar files for now.

### 1.3 Skinned (rigged) models

Same JSON entry — the importer detects bones automatically. When `aiMesh::HasBones()` is true:

- A `Skeleton` is built in DFS order (parents always precede children — required for `ComputePalette`).
- `VertexLayout` gains `BoneIndices(ivec4)` + `BoneWeights(vec4)` slots (locations 4, 5).
- The mesh is paired with a `SkinnedMeshComponent` instead of `MeshComponent`.
- The shader is auto-swapped to `assets/shaders/skinned.{vert,frag}`.
- The model root receives an `AnimationComponent` carrying the skeleton.

Bone cap is **128**. Above that the import is rejected with a clear error — bump `MAX_BONES` in `skinned.vert` and `kMaxBonesPerSkeleton` in `ModelImporter.cpp` if you need more (and consider moving to a UBO/SSBO at that point).

### 1.4 Visual debug

`F1` opens the editor. Hierarchy panel shows the loaded GameObject tree; selecting the model root reveals attached components (MeshComponent, SkinnedMeshComponent, AnimationComponent).

---

## 2. Animations

### 2.1 Two animation lanes

`AnimationComponent` runs two independent lanes simultaneously:

| Lane | Source | Drives |
|------|--------|--------|
| **Node** | `AnimationClip` + `TransformTrack[]` (target-by-name) | Child GameObject TRS |
| **Skeletal** | `SkeletalAnimationClip` + `BoneTrack[]` (target-by-bone-index) | GPU bone palette read by `SkinnedMeshComponent` |

`Play(name)` looks up the name in **both** lanes — if the same name matches a node clip and a skeletal clip, both run synchronized.

### 2.2 Importing baked animations

If the source file has clips, the importer auto-splits each `aiAnimation` into a node clip and a skeletal clip based on which channels target bones vs free nodes. To play one:

```cpp
auto *anim = handsObject->GetComponent<mnd::AnimationComponent>();
anim->Play("idle", true);   // looping
anim->Play("fire", false);  // one-shot
```

### 2.3 Procedural / synthesized clips

When an asset ships rigged but **without** baked clips (common for FPS hand packs), the importer auto-registers a procedural `idle` clip via `MakeBreathingIdleClip(skeleton)`. This targets canonical bone names (`shoulder.{r,l}`, `bicep.{r,l}`, `forearm.{r,l}`, `wrist.{r,l}`, `root`) with phase-offset sinusoidal rotations — bones it doesn't find are silently skipped, so it works on full bodies, FPS arms, and partial rigs alike.

To author a custom procedural clip yourself:

```cpp
#include "animation/SkeletalAnimationClip.h"
#include "animation/Skeleton.h"

auto BuildWalk(const mnd::Skeleton &skel) {
    auto clip = std::make_shared<mnd::SkeletalAnimationClip>();
    clip->name     = "walk";
    clip->duration = 1.0f;
    clip->looping  = true;

    mnd::i32 bone = skel.FindBoneIndex("hip");
    if (bone < 0) return clip;

    mnd::BoneTrack track;
    track.boneIndex = bone;
    for (int i = 0; i <= 30; ++i) {
        float t   = i / 30.0f;
        float yaw = 0.1f * std::sin(t * 6.283185f);
        track.rotations.push_back({ t, glm::angleAxis(yaw, glm::vec3(0,1,0)) });
    }
    clip->tracks.push_back(std::move(track));
    return clip;
}

// Then:
anim->RegisterSkeletalClip("walk", BuildWalk(*anim->GetSkeleton()));
anim->Play("walk", true);
```

**Critical:** missing TRS channels in a track fall back to the bone's **bind-pose component** (cached at load via `Skeleton::AddBone`). A rotation-only track does **not** zero out the bone's bind translation. This is what makes single-channel synthesized clips work without exploding the rig.

### 2.4 Tuning at runtime

`F1` → **Hands Tweaker** panel exposes:
- Position / rotation / uniform scale drag-sliders (live-apply to the GameObject).
- Idle duration & amplitude scalar (multiplies amps inside `kIdleSpecs`).
- "Regenerate idle" rebuilds the clip with current params.
- "Print JSON" dumps current transform to the console in pasteable form.

Reuse the pattern (`Editor::RegisterPanel`) for any other model that needs tuning — see `Player::RegisterHandsTweakerPanel` in `source/Player.cpp` for the template.

---

## 3. Actions (input → behavior)

### 3.1 Polling input

```cpp
auto &input = mnd::Engine::GetInstance().GetInputManager();

if (input.IsKeyPressed(mnd::Key::E))         { /* edge: just-pressed this frame */ }
if (input.IsKeyDown(mnd::Key::W))            { /* held: every frame */ }
if (input.IsMouseButtonPressed(mnd::MouseButton::Left)) { /* fire */ }
```

Key codes live in `engine/source/input/Key.h` (mostly GLFW-aligned: `Num1=49`, `W=87`, …).

### 3.2 Where action handling lives

For player actions, override `Update(deltaTime)` on the player class:

```cpp
void Player::Update(mnd::f32 dt) {
    auto &input = mnd::Engine::GetInstance().GetInputManager();

    if (input.IsKeyPressed(mnd::Key::Num1)) { SetViewmodel(true);  }
    if (input.IsKeyPressed(mnd::Key::Num2)) { SetViewmodel(false); }
    // …
    mnd::GameObject::Update(dt);  // recurse to children
}
```

Always end with `GameObject::Update(dt)` so child GameObjects and their components still tick.

### 3.3 Triggering animations + sound from an action

Pattern from `Player::Update` (firing the carbine):

```cpp
if (input.IsMouseButtonPressed(mnd::MouseButton::Left)) {
    if (m_animationComponent && !m_animationComponent->IsPlaying()) {
        m_animationComponent->Play(kAnimShoot, false);
        m_audioComponent->Play(kSfxShoot);
        // spawn bullet, etc.
    }
}
```

Pattern: gate one-shot anims on `!IsPlaying()` so spam-clicking doesn't restart them every frame.

---

## 4. Systems (Components)

### 4.1 Anatomy

A Component is a chunk of behavior attached to a GameObject. Its lifecycle:

1. `Init()` — once after attach (`GameObject::AddComponent` calls it).
2. `Update(deltaTime)` — every frame while owner active.
3. `LoadProperties(json)` — when the scene JSON has a `components: [{ "type": "MyComp", ... }]` entry.

The `COMPONENT(MyComp)` macro generates `TypeId()` and a static `Register()` factory that lets the scene loader instantiate by name.

### 4.2 Adding a new component

```cpp
// engine/source/scene/components/SpinComponent.h
#pragma once
#include "scene/Component.h"

namespace mnd {
class SpinComponent : public Component {
    COMPONENT(SpinComponent)
public:
    void Update(f32 dt) override;
    void LoadProperties(const nlohmann::json &json) override;
private:
    f32 m_speed = 1.0f;
};
}
```

```cpp
// .cpp
#include "scene/components/SpinComponent.h"
#include "scene/GameObject.h"
#include <glm/gtc/quaternion.hpp>

namespace mnd {
void SpinComponent::Update(f32 dt) {
    auto rot = GetOwner()->GetRotation();
    rot      = rot * glm::angleAxis(m_speed * dt, glm::vec3(0,1,0));
    GetOwner()->SetRotation(rot);
}
void SpinComponent::LoadProperties(const nlohmann::json &json) {
    m_speed = json.value("speed", 1.0f);
}
}
```

Register the type so scene JSON can instantiate it. Add to `Scene::Init`:

```cpp
SpinComponent::Register();
```

Use in JSON:

```json
{ "type": "SpinComponent", "speed": 3.14 }
```

### 4.3 When to write a Component vs a GameObject subclass

| Need | Choice |
|------|--------|
| Reusable behavior across many object types (Audio, Physics, Mesh, Animation) | **Component** |
| Custom object with its own children, init logic, scene-managed lifetime (Player, Bullet, JumpPlatform, Enemy) | **GameObject subclass** |
| Engine-wide systems with no per-object state | Static singletons (look at `Engine::Get*`) |

---

## 5. Enemies (custom GameObject subclasses)

### 5.1 Skeleton subclass

```cpp
// source/Skeleton.h
#pragma once
#include "Monad.h"

class Skeleton : public mnd::GameObject {
    GAMEOBJECT(Skeleton)
public:
    void Init();
    void Update(mnd::f32 dt) override;

private:
    mnd::PhysicsComponent *m_physics = nullptr;
    mnd::f32  m_health      = 50.0f;
    mnd::vec3 m_homePos;
};
```

```cpp
// source/Skeleton.cpp
#include "Skeleton.h"
#include "Monad.h"

using namespace mnd;

void Skeleton::Init() {
    m_physics  = GetComponent<PhysicsComponent>();
    m_homePos  = GetPosition();
}

void Skeleton::Update(f32 dt) {
    // Trivial AI: drift back toward home.
    auto pos    = GetPosition();
    auto toHome = m_homePos - pos;
    if (glm::length(toHome) > 0.1f) {
        SetPosition(pos + glm::normalize(toHome) * 1.5f * dt);
    }
    GameObject::Update(dt);
}
```

### 5.2 Register the type

In `Game::RegisterTypes` (`source/Game.cpp`):

```cpp
void Game::RegisterTypes() {
    Player::Register();
    JumpPlatform::Register();
    Bullet::Register();
    Skeleton::Register();   // ← add
}
```

`Register()` is generated by the `GAMEOBJECT(Skeleton)` macro; it enrolls the type in `GameObjectFactory` so the scene loader can spawn one when it encounters `"type": "Skeleton"` in JSON.

### 5.3 Use from a scene file

```json
{
  "name": "Skeleton1",
  "type": "Skeleton",
  "position": { "x": -23, "y": 1, "z": 0 },
  "components": [
    { "type": "fbx", "path": "models/skeleton/skeleton.fbx" },
    {
      "type": "PhysicsComponent",
      "collider": { "type": "capsule", "r": 0.4, "h": 1.6 },
      "body": { "mass": 1, "friction": 0.5, "type": "kinematic" }
    },
    {
      "type": "AudioComponent",
      "audio": [ { "name": "hurt", "path": "audio/skeleton_hurt.wav" } ]
    }
  ]
}
```

(For models, prefer attaching as a `child` rather than a component — see scene examples.)

### 5.4 Damage / contact reactions

Hook into physics callbacks the way `JumpPlatform` does (`source/JumpPlatform.cpp`):

```cpp
class Skeleton : public mnd::GameObject, public mnd::ContactListener {
    // ...
    void OnContact(mnd::CollisionObject *obj, const mnd::vec3 &pos, const mnd::vec3 &n) override;
};

void Skeleton::Init() {
    if (auto *p = GetComponent<mnd::PhysicsComponent>()) {
        p->GetRigidBody()->AddContactListener(this);
    }
}
```

Filter contacts by `obj->GetCollisionObjectType()` to react only to the relevant kinds (player, bullet, terrain).

---

## 6. Dungeons (scene authoring)

### 6.1 File layout

Scenes are JSON under `assets/scenes/`. Pick which one runs by editing `Game::Init()`:

```cpp
auto scene = mnd::Scene::Load(kDungeonScenePath);
```

Available constants in `source/GameConstants.h`:
- `kInitialScenePath` → `scenes/scene.json`
- `kTestScenePath` → `scenes/testing.json`
- `kDungeonScenePath` → `scenes/dungeon.json`
- `kShaderTestScenePath` → `scenes/shader_test.json`

### 6.2 Top-level shape

```json
{
  "name": "MyDungeon",
  "camera": "MainPlayer",
  "objects": [ /* GameObjects */ ]
}
```

`camera` is the name of a GameObject that owns a `CameraComponent`.

### 6.3 GameObject entry shape

```json
{
  "name": "Wall_North",
  "type": "GameObject",        // optional; subclass name (Player, Skeleton, …) for custom types; "gltf"/"fbx"/"model" for model loaders
  "position": { "x": 0, "y": 4.5, "z": -10 },
  "rotation": { "x": 0, "y": 0, "z": 0, "w": 1 },     // quaternion
  "scale":    { "x": 1, "y": 1, "z": 1 },
  "components": [ /* see below */ ],
  "children":   [ /* nested GameObjects */ ]
}
```

### 6.4 Component palette (cheat sheet)

| `type` | purpose | key params |
|--------|---------|-----------|
| `MeshComponent` | Renderable primitive or loaded mesh | `mesh: { type:"box"\|"sphere", x,y,z\|r }`, `material: { path, params }` |
| `PhysicsComponent` | Bullet rigid body | `collider: { type, ... }`, `body: { mass, friction, type:"static"\|"dynamic"\|"kinematic" }` |
| `LightComponent` | Point light | `color: { r,g,b }` |
| `CameraComponent` | View source | (none — uses owner transform) |
| `AudioComponent` | One or more named sounds | `audio: [ { name, path } ]` |
| `AudioListenerComponent` | Receiver | (none) |
| `PlayerControllerComponent` | FPS-style movement | (tunables in source) |
| `AnimationComponent` | Auto-attached by model loaders | (don't add manually) |
| `SkinnedMeshComponent` | Auto-attached by model loaders | (don't add manually) |

### 6.5 Material setup in JSON

```json
"material": {
  "path": "materials/lit_pixel.mat",
  "params": {
    "float":  [ { "name": "uRimStrength", "value": 1.5 } ],
    "float3": [ { "name": "color", "value0": 0.5, "value1": 0.3, "value2": 0.2 } ],
    "textures": [ { "name": "baseColorTexture", "path": "textures/wall.png" } ]
  }
}
```

Materials live in `assets/materials/*.mat` (also JSON, point to shaders + define defaults). Shaders are GLSL pairs in `assets/shaders/*.{vert,frag}`.

### 6.6 Building a dungeon room — minimal recipe

```json
{
  "name": "Crypt",
  "objects": [
    /* floor */
    { "name": "F", "position": { "x":0, "y":0, "z":0 },
      "components": [
        { "type": "MeshComponent",
          "mesh": { "type":"box", "x":10, "y":1, "z":10 },
          "material": { "path": "materials/dungeon_floor.mat" } },
        { "type": "PhysicsComponent",
          "collider": { "type":"box", "x":10, "y":1, "z":10 },
          "body": { "mass":0, "type":"static" } }
      ] },

    /* ceiling — duplicate floor entry, y:9 */

    /* walls — boxes scaled thin on one axis, surrounding the room */

    /* lights — sphere meshes + LightComponent */
    { "name": "Torch",
      "position": { "x":0, "y":6, "z":0 },
      "components": [
        { "type": "LightComponent", "color": { "r":1, "g":0.7, "b":0.4 } },
        { "type": "MeshComponent",
          "mesh": { "type":"sphere", "r":0.2 },
          "material": { "path":"materials/light_bulb.mat" } }
      ] },

    /* the player */
    { "name":"MainPlayer", "type":"Player",
      "position": { "x":0, "y":2, "z":0 },
      "components": [
        { "type":"CameraComponent" },
        { "type":"PlayerControllerComponent" },
        { "type":"AudioListenerComponent" }
      ] },

    /* enemies */
    { "name":"Skel1", "type":"Skeleton",
      "position": { "x":3, "y":1, "z":-2 },
      "children": [
        { "name":"Body", "type":"fbx", "path":"models/skeleton/skeleton.fbx",
          "scale": { "x":1, "y":1, "z":1 } }
      ] }
  ]
}
```

`assets/scenes/dungeon.json` is a complete, working reference — copy it as a starting template.

### 6.7 Iteration loop

1. Edit the JSON.
2. `./compile.sh` — assets are symlinked into `bin/<type>/assets`, no rebuild needed for asset-only changes.
3. `F1` in-game → Hierarchy panel to inspect what loaded.
4. Use the **Hands Tweaker** as a template for any per-room tuning panels (positions, light colors, idle params).

---

## 7. Quick-reference: file map

| Path | When to edit |
|------|--------------|
| `engine/source/animation/IdleClip.cpp` | Tweak procedural idle bone amps / target list |
| `engine/source/animation/Pose.cpp` | Bone palette math, TRS evaluation |
| `engine/source/io/ModelImporter.cpp` | Asset import flags, texture types, embedded textures |
| `engine/source/scene/Scene.cpp` | New `type` keywords (`fbx`, `gltf`, …), JSON parsing |
| `engine/source/scene/components/*.{h,cpp}` | Add/extend reusable behaviors |
| `source/Game.cpp` | Pick which scene boots; register custom GameObject types |
| `source/GameConstants.h` | Names of bones, animations, child objects, audio cues |
| `source/Player.cpp` | Player-specific input / view-model logic / debug panels |
| `assets/scenes/*.json` | Level layout |
| `assets/materials/*.mat` | Shader + uniform presets |
| `assets/shaders/*.{vert,frag}` | GLSL surface code |

---

## 8. Conventions worth respecting

- All engine code is in `namespace mnd`; game code is not.
- Use `mnd::f32`, `mnd::i32`, `mnd::u32`, `mnd::usize`, `mnd::str` (see `engine/source/Types.h`) over raw stdint.
- Logging: `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`, `LOG_FATAL` (from `engine/source/Log.h`). `LOG_FATAL` aborts.
- Don't add comments that just restate what code does. Add them only when WHY isn't obvious from the names.
- Mesh and Material are paired only at submit time inside `RenderCommand` — neither owns the other. Reuse aggressively.
- Skinned meshes need a unique Material instance per draw (so `uBones[]` doesn't get stomped by other skinned draws sharing the material).
- Bone array is topologically ordered: parents always precede children. `ComputePalette` depends on this.

That's the lot. Read `CLAUDE.md` for the engine internals if you're refactoring; otherwise this guide is enough to add levels, enemies, and animations without touching the engine layer.
