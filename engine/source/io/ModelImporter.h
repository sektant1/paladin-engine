/**
 * @file ModelImporter.h
 * @ingroup mnd_io
 * @brief Assimp-backed model loader. Handles FBX / glTF / OBJ.
 *
 * `Import()` parses the asset, builds Mesh + Material + Skeleton +
 * SkeletalAnimationClip data, and constructs a GameObject hierarchy
 * mirroring the file's node tree.
 *
 * Static meshes get a MeshComponent. Skinned meshes get a
 * SkinnedMeshComponent and the model root gets an AnimationComponent
 * carrying the skeleton + clips, wired as the palette source for every
 * SkinnedMeshComponent in the hierarchy.
 *
 * The scene file's "type" field can be "fbx", "model", or any future
 * extension Assimp supports — the actual format is detected by Assimp.
 */

#pragma once

#include <string>

namespace mnd
{

class GameObject;
class Scene;

class ModelImporter
{
public:
    /// Load `path` (relative to assets folder) and build a GameObject hierarchy under `scene`. Returns nullptr on failure.
    static GameObject *Import(const std::string &path, Scene *scene);
};

}  // namespace mnd
