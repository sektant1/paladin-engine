#include "io/ModelImporter.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Engine.h"
#include "Log.h"
#include "animation/IdleClip.h"
#include "animation/Pose.h"
#include "animation/Skeleton.h"
#include "animation/SkeletalAnimationClip.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "graphics/VertexLayout.h"
#include "io/FileSystem.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include "scene/GameObject.h"
#include "scene/Scene.h"
#include "scene/components/AnimationComponent.h"
#include "scene/components/MeshComponent.h"
#include "scene/components/SkinnedMeshComponent.h"

namespace mnd
{

namespace
{

constexpr u32 kMaxBonesPerVertex = 4;
constexpr u32 kMaxBonesPerSkeleton = 128;
constexpr f32 kDefaultTicksPerSecond = 25.0f;

glm::mat4 ToGlm(const aiMatrix4x4 &m)
{
    return {
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4,
    };
}

glm::vec3 ToGlm(const aiVector3D &v) { return {v.x, v.y, v.z}; }
glm::quat ToGlm(const aiQuaternion &q) { return {q.w, q.x, q.y, q.z}; }

std::shared_ptr<ShaderProgram> &SkinnedShaderCache()
{
    static std::shared_ptr<ShaderProgram> sp;
    return sp;
}

std::shared_ptr<ShaderProgram> GetSkinnedShader()
{
    auto &cache = SkinnedShaderCache();
    if (cache)
    {
        return cache;
    }
    auto &fs           = Engine::GetInstance().GetFileSystem();
    auto  vertexSource = fs.LoadAssetFileText("shaders/skinned.vert");
    auto  fragmentSource = fs.LoadAssetFileText("shaders/skinned.frag");
    if (vertexSource.empty() || fragmentSource.empty())
    {
        LOG_ERROR("ModelImporter could not load skinned shader sources from assets/shaders/skinned.{vert,frag}");
        return nullptr;
    }
    cache = Engine::GetInstance().GetGraphicsAPI().CreateShaderProgram(vertexSource, fragmentSource);
    return cache;
}

/// Pre-pass: collect the set of nodes that participate in any skeleton.
void CollectBoneNodes(const aiNode *node, const std::unordered_map<std::string, const aiBone *> &boneByName, std::unordered_map<const aiNode *, bool> &include)
{
    bool isBone = boneByName.find(node->mName.C_Str()) != boneByName.end();
    bool anyChildIncluded = false;
    for (u32 i = 0; i < node->mNumChildren; ++i)
    {
        CollectBoneNodes(node->mChildren[i], boneByName, include);
        if (include[node->mChildren[i]])
        {
            anyChildIncluded = true;
        }
    }
    include[node] = isBone || anyChildIncluded;
}

/// Build a flat skeleton in DFS order so parents always precede children.
void BuildSkeletonRecursive(const aiNode                                          *node,
                            i32                                                   parentIndex,
                            const std::unordered_map<std::string, const aiBone *> &boneByName,
                            const std::unordered_map<const aiNode *, bool>        &include,
                            Skeleton                                              &skeleton,
                            std::unordered_map<const aiNode *, i32>               &nodeToBone)
{
    auto incIt = include.find(node);
    if (incIt == include.end() || !incIt->second)
    {
        // Subtree contains no bones; keep walking but don't promote nodes.
        for (u32 i = 0; i < node->mNumChildren; ++i)
        {
            BuildSkeletonRecursive(node->mChildren[i], parentIndex, boneByName, include, skeleton, nodeToBone);
        }
        return;
    }

    Bone bone;
    bone.name        = node->mName.C_Str();
    bone.parentIndex = parentIndex;
    bone.localBind   = ToGlm(node->mTransformation);

    auto bIt = boneByName.find(bone.name);
    if (bIt != boneByName.end())
    {
        bone.offsetMatrix = ToGlm(bIt->second->mOffsetMatrix);
    }

    skeleton.AddBone(bone);
    i32 myIndex          = static_cast<i32>(skeleton.Size()) - 1;
    nodeToBone[node]     = myIndex;

    for (u32 i = 0; i < node->mNumChildren; ++i)
    {
        BuildSkeletonRecursive(node->mChildren[i], myIndex, boneByName, include, skeleton, nodeToBone);
    }
}

struct VertexBoneSlot
{
    i32 boneIndex = 0;
    f32 weight    = 0.0f;
};

void InsertBoneWeight(std::array<VertexBoneSlot, kMaxBonesPerVertex> &slots, i32 boneIndex, f32 weight)
{
    // Replace the smallest existing weight if the incoming one is bigger.
    auto minIt = std::min_element(slots.begin(), slots.end(),
                                  [](const VertexBoneSlot &a, const VertexBoneSlot &b) { return a.weight < b.weight; });
    if (weight > minIt->weight)
    {
        minIt->boneIndex = boneIndex;
        minIt->weight    = weight;
    }
}

void NormalizeBoneSlots(std::array<VertexBoneSlot, kMaxBonesPerVertex> &slots)
{
    f32 sum = 0.0f;
    for (auto &s : slots)
    {
        sum += s.weight;
    }
    if (sum <= 0.0f)
    {
        return;
    }
    for (auto &s : slots)
    {
        s.weight /= sum;
    }
}

std::shared_ptr<Texture> LoadDiffuseTexture(const aiMaterial *mat, const std::filesystem::path &assetFolder)
{
    if (mat->GetTextureCount(aiTextureType_DIFFUSE) == 0 &&
        mat->GetTextureCount(aiTextureType_BASE_COLOR) == 0)
    {
        return nullptr;
    }

    aiString aiPath;
    if (mat->GetTexture(aiTextureType_BASE_COLOR, 0, &aiPath) != aiReturn_SUCCESS &&
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath) != aiReturn_SUCCESS)
    {
        return nullptr;
    }

    std::string raw = aiPath.C_Str();
    if (raw.empty() || raw[0] == '*')
    {
        // Embedded textures (e.g. "*0") not supported in v1.
        LOG_WARN("ModelImporter: embedded textures not supported, skipping '%s'", raw.c_str());
        return nullptr;
    }

    // FBX exporters often emit absolute Windows paths. Reduce to filename
    // and resolve against the model's folder.
    std::filesystem::path candidate(raw);
    auto                  filename = candidate.filename();
    auto                  full     = assetFolder / filename;

    return Engine::GetInstance().GetTextureManager().GetOrLoadTexture(full.string());
}

/// Pack one mesh into engine GPU resources. Optionally maps bone names → skeleton indices.
struct ImportedMesh
{
    std::shared_ptr<Mesh>     mesh;
    std::shared_ptr<Material> material;
    bool                      isSkinned = false;
};

ImportedMesh ImportMesh(const aiMesh                                        *aimesh,
                        const aiScene                                       *aiscene,
                        const std::filesystem::path                         &assetFolder,
                        const Skeleton                                      &skeleton)
{
    ImportedMesh out;

    const bool hasNormals = aimesh->HasNormals();
    const bool hasUV      = aimesh->HasTextureCoords(0);
    const bool hasColor   = aimesh->HasVertexColors(0);
    const bool hasBones   = aimesh->HasBones();

    VertexLayout layout;
    auto AddElem = [&](u32 index, u32 size, GLuint type)
    {
        VertexElement el;
        el.index  = index;
        el.size   = size;
        el.type   = type;
        el.offset = layout.stride;
        layout.elements.push_back(el);
        layout.stride += size * (type == GL_INT ? sizeof(i32) : sizeof(f32));
    };

    AddElem(VertexElement::PositionIndex, 3, GL_FLOAT);
    if (hasColor)  AddElem(VertexElement::ColorIndex,  3, GL_FLOAT);
    if (hasUV)     AddElem(VertexElement::UVIndex,     2, GL_FLOAT);
    if (hasNormals) AddElem(VertexElement::NormalIndex, 3, GL_FLOAT);
    if (hasBones)
    {
        AddElem(VertexElement::BoneIndicesIndex, 4, GL_INT);
        AddElem(VertexElement::BoneWeightsIndex, 4, GL_FLOAT);
    }

    // Pre-aggregate bone weights per vertex.
    std::vector<std::array<VertexBoneSlot, kMaxBonesPerVertex>> boneSlots;
    if (hasBones)
    {
        boneSlots.resize(aimesh->mNumVertices);
        for (u32 b = 0; b < aimesh->mNumBones; ++b)
        {
            const aiBone *bone   = aimesh->mBones[b];
            i32           boneIx = skeleton.FindBoneIndex(bone->mName.C_Str());
            if (boneIx < 0)
            {
                continue;
            }
            for (u32 w = 0; w < bone->mNumWeights; ++w)
            {
                const auto &vw = bone->mWeights[w];
                if (vw.mWeight <= 0.0f) continue;
                InsertBoneWeight(boneSlots[vw.mVertexId], boneIx, vw.mWeight);
            }
        }
        for (auto &slots : boneSlots)
        {
            NormalizeBoneSlots(slots);
        }
    }

    // Pack interleaved vertex buffer. Bone indices are packed as i32 bytes
    // into f32 slots via memcpy; Mesh.cpp routes integer-typed elements
    // through glVertexAttribIPointer so the shader sees ivec4.
    const u32 floatsPerVertex = layout.stride / sizeof(f32);
    std::vector<f32> vertices(static_cast<size_t>(aimesh->mNumVertices) * floatsPerVertex, 0.0f);

    for (u32 v = 0; v < aimesh->mNumVertices; ++v)
    {
        f32 *vp = &vertices[v * floatsPerVertex];
        for (const auto &el : layout.elements)
        {
            f32 *dst = reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(vp) + el.offset);
            switch (el.index)
            {
                case VertexElement::PositionIndex:
                {
                    auto p = aimesh->mVertices[v];
                    dst[0] = p.x; dst[1] = p.y; dst[2] = p.z;
                    break;
                }
                case VertexElement::ColorIndex:
                {
                    auto c = aimesh->mColors[0][v];
                    dst[0] = c.r; dst[1] = c.g; dst[2] = c.b;
                    break;
                }
                case VertexElement::UVIndex:
                {
                    auto t = aimesh->mTextureCoords[0][v];
                    dst[0] = t.x; dst[1] = t.y;
                    break;
                }
                case VertexElement::NormalIndex:
                {
                    auto n = aimesh->mNormals[v];
                    dst[0] = n.x; dst[1] = n.y; dst[2] = n.z;
                    break;
                }
                case VertexElement::BoneIndicesIndex:
                {
                    i32 idx[4] = {
                        boneSlots[v][0].boneIndex, boneSlots[v][1].boneIndex,
                        boneSlots[v][2].boneIndex, boneSlots[v][3].boneIndex,
                    };
                    std::memcpy(dst, idx, sizeof(idx));
                    break;
                }
                case VertexElement::BoneWeightsIndex:
                {
                    dst[0] = boneSlots[v][0].weight;
                    dst[1] = boneSlots[v][1].weight;
                    dst[2] = boneSlots[v][2].weight;
                    dst[3] = boneSlots[v][3].weight;
                    break;
                }
                default: break;
            }
        }
    }

    std::vector<u32> indices;
    indices.reserve(static_cast<size_t>(aimesh->mNumFaces) * 3);
    for (u32 f = 0; f < aimesh->mNumFaces; ++f)
    {
        const auto &face = aimesh->mFaces[f];
        if (face.mNumIndices != 3)
        {
            continue;
        }
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    out.mesh      = std::make_shared<Mesh>(layout, vertices, indices);
    out.isSkinned = hasBones;

    auto material = std::make_shared<Material>();
    if (hasBones)
    {
        auto skinShader = GetSkinnedShader();
        if (skinShader)
        {
            material->SetShaderProgram(skinShader);
        } else
        {
            material->SetShaderProgram(Engine::GetInstance().GetGraphicsAPI().GetDefaultShaderProgram());
        }
    } else
    {
        material->SetShaderProgram(Engine::GetInstance().GetGraphicsAPI().GetDefaultShaderProgram());
    }
    material->SetParam("color", vec3(1.0f, 1.0f, 1.0f));

    if (aimesh->mMaterialIndex < aiscene->mNumMaterials)
    {
        auto *aiMat = aiscene->mMaterials[aimesh->mMaterialIndex];
        if (auto tex = LoadDiffuseTexture(aiMat, assetFolder))
        {
            material->SetParam("baseColorTexture", tex);
        }
    }

    out.material = material;
    return out;
}

void BuildSceneHierarchy(const aiNode                                  *node,
                         const aiScene                                 *aiscene,
                         GameObject                                    *parent,
                         Scene                                         *scene,
                         const std::vector<ImportedMesh>               &meshes,
                         const std::shared_ptr<Skeleton>               &skeleton,
                         AnimationComponent                            *animOwnerComp,
                         std::vector<SkinnedMeshComponent *>           &skinnedOut,
                         const std::filesystem::path                   & /*assetFolder*/)
{
    auto *go = scene->CreateObject(node->mName.C_Str(), parent);

    // Decompose node->mTransformation into TRS and write to GameObject.
    aiVector3D   aiPos;
    aiQuaternion aiRot;
    aiVector3D   aiScale;
    node->mTransformation.Decompose(aiScale, aiRot, aiPos);
    go->SetPosition(ToGlm(aiPos));
    go->SetRotation(ToGlm(aiRot));
    go->SetScale(ToGlm(aiScale));

    for (u32 i = 0; i < node->mNumMeshes; ++i)
    {
        u32 meshIdx = node->mMeshes[i];
        if (meshIdx >= meshes.size())
        {
            continue;
        }
        const auto &im = meshes[meshIdx];
        if (im.isSkinned && skeleton)
        {
            auto *comp = new SkinnedMeshComponent(im.material, im.mesh, skeleton);
            comp->SetPaletteSource(animOwnerComp);
            go->AddComponent(comp);
            skinnedOut.push_back(comp);
        } else
        {
            go->AddComponent(new MeshComponent(im.material, im.mesh));
        }
    }

    for (u32 i = 0; i < node->mNumChildren; ++i)
    {
        BuildSceneHierarchy(node->mChildren[i], aiscene, go, scene, meshes, skeleton, animOwnerComp, skinnedOut, /*assetFolder*/ {});
    }
}

}  // namespace

GameObject *ModelImporter::Import(const std::string &path, Scene *scene)
{
    if (!scene)
    {
        LOG_ERROR("ModelImporter::Import called with null scene (path='%s')", path.c_str());
        return nullptr;
    }

    auto fullPath    = Engine::GetInstance().GetFileSystem().GetAssetsFolder() / path;
    auto assetFolder = std::filesystem::path(fullPath).remove_filename();

    Assimp::Importer imp;
    const aiScene  *aiscene = imp.ReadFile(
        fullPath.string(),
        aiProcess_Triangulate
            | aiProcess_GenSmoothNormals
            | aiProcess_LimitBoneWeights
            | aiProcess_JoinIdenticalVertices
            | aiProcess_ImproveCacheLocality
            | aiProcess_GlobalScale
            | aiProcess_PopulateArmatureData);

    if (!aiscene || (aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0 || !aiscene->mRootNode)
    {
        LOG_ERROR("ModelImporter::Import failed for '%s': %s", path.c_str(), imp.GetErrorString());
        return nullptr;
    }

    // -- Skeleton ----------------------------------------------------------
    std::unordered_map<std::string, const aiBone *> boneByName;
    for (u32 m = 0; m < aiscene->mNumMeshes; ++m)
    {
        const aiMesh *am = aiscene->mMeshes[m];
        for (u32 b = 0; b < am->mNumBones; ++b)
        {
            boneByName[am->mBones[b]->mName.C_Str()] = am->mBones[b];
        }
    }

    auto skeleton = std::make_shared<Skeleton>();
    std::unordered_map<const aiNode *, i32> nodeToBone;
    if (!boneByName.empty())
    {
        std::unordered_map<const aiNode *, bool> include;
        CollectBoneNodes(aiscene->mRootNode, boneByName, include);
        BuildSkeletonRecursive(aiscene->mRootNode, -1, boneByName, include, *skeleton, nodeToBone);

        if (skeleton->Size() > kMaxBonesPerSkeleton)
        {
            LOG_ERROR("ModelImporter: '%s' has %zu bones, exceeds cap of %u (skinned shader limit)",
                      path.c_str(), skeleton->Size(), kMaxBonesPerSkeleton);
            return nullptr;
        }
        LOG_INFO("ModelImporter: skeleton with %zu bones for '%s'", skeleton->Size(), path.c_str());
    }

    // -- Meshes ------------------------------------------------------------
    std::vector<ImportedMesh> meshes;
    meshes.reserve(aiscene->mNumMeshes);
    for (u32 m = 0; m < aiscene->mNumMeshes; ++m)
    {
        meshes.push_back(ImportMesh(aiscene->mMeshes[m], aiscene, assetFolder, *skeleton));
    }

    // -- Animations --------------------------------------------------------
    std::vector<std::shared_ptr<SkeletalAnimationClip>> skelClips;
    std::vector<std::shared_ptr<AnimationClip>>         nodeClips;

    for (u32 a = 0; a < aiscene->mNumAnimations; ++a)
    {
        const aiAnimation *anim = aiscene->mAnimations[a];
        const f32          tps  = (anim->mTicksPerSecond > 0.0) ? static_cast<f32>(anim->mTicksPerSecond) : kDefaultTicksPerSecond;

        auto skelClip      = std::make_shared<SkeletalAnimationClip>();
        skelClip->name     = anim->mName.length > 0 ? anim->mName.C_Str() : "noname";
        skelClip->duration = static_cast<f32>(anim->mDuration) / tps;

        auto nodeClip      = std::make_shared<AnimationClip>();
        nodeClip->name     = skelClip->name;
        nodeClip->duration = skelClip->duration;

        for (u32 c = 0; c < anim->mNumChannels; ++c)
        {
            const aiNodeAnim *ch       = anim->mChannels[c];
            std::string       nodeName = ch->mNodeName.C_Str();
            i32               boneIx   = skeleton->FindBoneIndex(nodeName);

            if (boneIx >= 0)
            {
                BoneTrack track;
                track.boneIndex = boneIx;
                track.positions.reserve(ch->mNumPositionKeys);
                for (u32 k = 0; k < ch->mNumPositionKeys; ++k)
                {
                    track.positions.push_back({static_cast<f32>(ch->mPositionKeys[k].mTime) / tps, ToGlm(ch->mPositionKeys[k].mValue)});
                }
                track.rotations.reserve(ch->mNumRotationKeys);
                for (u32 k = 0; k < ch->mNumRotationKeys; ++k)
                {
                    track.rotations.push_back({static_cast<f32>(ch->mRotationKeys[k].mTime) / tps, ToGlm(ch->mRotationKeys[k].mValue)});
                }
                track.scales.reserve(ch->mNumScalingKeys);
                for (u32 k = 0; k < ch->mNumScalingKeys; ++k)
                {
                    track.scales.push_back({static_cast<f32>(ch->mScalingKeys[k].mTime) / tps, ToGlm(ch->mScalingKeys[k].mValue)});
                }
                skelClip->tracks.push_back(std::move(track));
            } else
            {
                // Non-bone channel — drives a plain GameObject by name.
                TransformTrack track;
                track.targetName = nodeName;
                track.positions.reserve(ch->mNumPositionKeys);
                for (u32 k = 0; k < ch->mNumPositionKeys; ++k)
                {
                    track.positions.push_back({static_cast<f32>(ch->mPositionKeys[k].mTime) / tps, ToGlm(ch->mPositionKeys[k].mValue)});
                }
                track.rotations.reserve(ch->mNumRotationKeys);
                for (u32 k = 0; k < ch->mNumRotationKeys; ++k)
                {
                    track.rotations.push_back({static_cast<f32>(ch->mRotationKeys[k].mTime) / tps, ToGlm(ch->mRotationKeys[k].mValue)});
                }
                track.scales.reserve(ch->mNumScalingKeys);
                for (u32 k = 0; k < ch->mNumScalingKeys; ++k)
                {
                    track.scales.push_back({static_cast<f32>(ch->mScalingKeys[k].mTime) / tps, ToGlm(ch->mScalingKeys[k].mValue)});
                }
                nodeClip->tracks.push_back(std::move(track));
            }
        }

        if (!skelClip->tracks.empty())
        {
            skelClips.push_back(skelClip);
        }
        if (!nodeClip->tracks.empty())
        {
            nodeClips.push_back(nodeClip);
        }
    }

    // -- Hierarchy & components -------------------------------------------
    auto *root = scene->CreateObject(std::filesystem::path(path).stem().string());

    AnimationComponent *animComp = nullptr;
    if (!skelClips.empty() || !nodeClips.empty() || skeleton->Size() > 0)
    {
        animComp = new AnimationComponent();
        root->AddComponent(animComp);
        animComp->SetSkeleton(skeleton);
        for (auto &c : nodeClips)
        {
            animComp->RegisterClip(c->name, c);
        }
        for (auto &c : skelClips)
        {
            animComp->RegisterSkeletalClip(c->name, c);
        }

        // If the asset shipped rigged but with no skeletal clips, generate a
        // procedural idle so the model is alive by default. Game code can
        // still override by registering its own clips and calling Play().
        if (skelClips.empty() && skeleton->Size() > 0)
        {
            auto idle = MakeBreathingIdleClip(*skeleton);
            animComp->RegisterSkeletalClip(idle->name, idle);
            LOG_INFO("ModelImporter: synthesized procedural 'idle' clip for '%s' (skeleton has %zu bones, no baked anims)",
                     path.c_str(), skeleton->Size());
        }
    }

    std::vector<SkinnedMeshComponent *> skinned;
    for (u32 i = 0; i < aiscene->mRootNode->mNumChildren; ++i)
    {
        BuildSceneHierarchy(aiscene->mRootNode->mChildren[i], aiscene, root, scene, meshes, skeleton, animComp, skinned, assetFolder);
    }
    // If the root has its own meshes, attach them to root directly.
    for (u32 i = 0; i < aiscene->mRootNode->mNumMeshes; ++i)
    {
        u32 meshIdx = aiscene->mRootNode->mMeshes[i];
        if (meshIdx >= meshes.size())
        {
            continue;
        }
        const auto &im = meshes[meshIdx];
        if (im.isSkinned && skeleton)
        {
            auto *comp = new SkinnedMeshComponent(im.material, im.mesh, skeleton);
            comp->SetPaletteSource(animComp);
            root->AddComponent(comp);
            skinned.push_back(comp);
        } else
        {
            root->AddComponent(new MeshComponent(im.material, im.mesh));
        }
    }

    LOG_INFO("ModelImporter: '%s' loaded — meshes=%u skinned=%zu skelClips=%zu nodeClips=%zu",
             path.c_str(), aiscene->mNumMeshes, skinned.size(), skelClips.size(), nodeClips.size());

    return root;
}

}  // namespace mnd
