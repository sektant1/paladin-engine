/**
 * @file Material.h
 * @ingroup mnd_render
 * @brief Container that pairs a ShaderProgram with typed uniform parameters.
 *
 * A Material answers the question "how should this surface look?". It holds:
 * - A ShaderProgram (the GLSL pipeline to execute).
 * - Named float / vec2 / vec3 / vec4 parameters → uploaded as uniforms.
 * - Named Texture parameters → bound to texture units before each draw.
 *
 * ## Typical per-frame usage
 * @code
 *   material->SetParam("uTime",   iTime);
 *   material->SetParam("uColor",  1.0f, 0.5f, 0.2f);
 *   material->SetParam("uAlbedo", myTexture);
 *   // Then submit a RenderCommand — RenderQueue calls Bind() internally.
 * @endcode
 *
 * Material::Bind() uploads all stored params to the active ShaderProgram.
 * It is called by RenderQueue::Draw() just before each draw call.
 *
 * @note Material does not own a Mesh. The two are coupled only at submit time
 *       through a RenderCommand. This allows the same material to be reused
 *       across different meshes without duplication.
 *
 * @see RenderCommand, RenderQueue, ShaderProgram, Texture
 */

#pragma once
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <glm/mat4x4.hpp>

#include "Types.h"
#include "graphics/Texture.h"

namespace mnd
{
class ShaderProgram;
class Texture;

/**
 * @brief Shader + uniform storage that defines how a surface is rendered.
 *
 * Parameters are stored in typed maps and uploaded to the GPU lazily during
 * Material::Bind(). Between frames, SetParam() calls simply overwrite the
 * stored values without touching the GPU.
 */
class Material
{
 public:
    /**
     * @brief Assign the GLSL program this material will use when drawn.
     * @param shaderProgram Compiled shader; falls back to the default if unset.
     */
    void SetShaderProgram(const std::shared_ptr<ShaderProgram> &shaderProgram);

    /// @name Uniform Parameter Setters
    /// Store a named uniform value. Uploaded to the GPU in Bind().
    /// @{
    void SetParam(const std::string &name, float value);                              ///< Set a float uniform.
    void SetParam(const std::string &name, float v0, float v1);                       ///< Set a vec2 uniform.
    void SetParam(const std::string &name, const vec3 &value);                        ///< Set a vec3 uniform.
    void SetParam(const std::string &name, const vec4 &value);                        ///< Set a vec4 uniform.
    void SetParam(const std::string &name, const std::shared_ptr<Texture> &texture);  ///< Set a sampler2D uniform.
    void SetParam(const std::string &name, const std::vector<glm::mat4> &matrices);   ///< Set a mat4[] uniform (e.g. bone palette).
    /// @}

    /**
     * @brief Upload all stored parameters to the active ShaderProgram.
     *
     * Called automatically by RenderQueue::Draw() before each draw call.
     * Binds textures to consecutive texture units starting from 0.
     */
    void Bind();

    /// Returns the shader program assigned to this material.
    ShaderProgram *GetShaderProgram();

    /**
     * @brief Load a material from a descriptor file.
     * @param path Path to the material asset file.
     * @return Shared pointer to the loaded Material.
     */
    static std::shared_ptr<Material> Load(const str &path);

 private:
    std::shared_ptr<ShaderProgram>                            m_shaderProgram;  ///< GLSL program used for rendering.
    std::unordered_map<std::string, float>                    m_floatParams;    ///< float uniforms.
    std::unordered_map<std::string, std::tuple<float, float>> m_float2Params;   ///< vec2 uniforms.
    std::unordered_map<std::string, vec3>                     m_float3Params;   ///< vec3 uniforms.
    std::unordered_map<std::string, vec4>                     m_float4Params;   ///< vec4 uniforms.
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;       ///< Sampler2D uniforms.
    std::unordered_map<std::string, std::vector<glm::mat4>>   m_mat4ArrayParams; ///< mat4[] uniforms (bone palettes etc.).
};

}  // namespace mnd
