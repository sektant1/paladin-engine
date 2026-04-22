/**
 * @file ShaderProgram.h
 * @brief Wrapper around a linked OpenGL GLSL program with uniform caching.
 *
 * ShaderProgram objects are created by GraphicsAPI::CreateShaderProgram() from
 * GLSL source strings (loaded from .vert / .frag files at runtime). Once built,
 * they are owned by shared_ptr and shared by Material instances.
 *
 * ## Uniform upload flow
 * @code
 *   shader->Bind();
 *   shader->SetUniform("uModel", modelMatrix);   // cached location lookup
 *   shader->SetTexture("uDiffuse", texture);     // binds to next texture unit
 * @endcode
 *
 * Uniform locations are cached after the first lookup to avoid repeated
 * glGetUniformLocation calls, which are expensive if issued every frame.
 *
 * @see GraphicsAPI::CreateShaderProgram, Material
 */

#pragma once
#include <string>
#include <unordered_map>

#include <GL/glew.h>

#include "Types.h"

namespace ENG
{
class Texture;

/**
 * @brief Linked GLSL program with cached uniform locations.
 *
 * Non-copyable — each GL program handle is unique. Created exclusively by
 * GraphicsAPI and typically stored in a std::shared_ptr<ShaderProgram>.
 */
class ShaderProgram
{
public:
    ShaderProgram()                                 = delete;
    ShaderProgram(const ShaderProgram &)            = delete;
    ShaderProgram &operator=(const ShaderProgram &) = delete;

    /**
     * @brief Wraps an existing linked GL program.
     * @param shaderProgramID A GL program handle returned by glCreateProgram().
     */
    explicit ShaderProgram(GLuint shaderProgramID);

    /// Calls glDeleteProgram() to release the GPU resource.
    ~ShaderProgram();

    /// Activate this program for subsequent draw calls (glUseProgram).
    void Bind();

    /**
     * @brief Look up a uniform location, caching the result for future calls.
     * @param name The GLSL uniform variable name.
     * @return The GL uniform location, or -1 if not found.
     */
    GLint GetUniformLocation(const std::string &name);

    /// @name Uniform Setters
    /// Upload typed values to named GLSL uniforms.
    /// @{
    void SetUniform(const std::string &name, float value);                               ///< Upload a float uniform.
    void SetUniform(const std::string &name, float v0, float v1);                        ///< Upload a vec2 uniform.
    void SetUniform(const std::string &name, float v0, float v1, float v2);              ///< Upload a vec3 uniform.
    void SetUniform(const std::string &name, float v0, float v1, float v2, float v3);   ///< Upload a vec4 uniform.
    void SetUniform(const std::string &name, const mat4 &mat);                           ///< Upload a 4×4 matrix (column-major).
    void SetUniform(const std::string &name, const vec3 &value);                         ///< Upload a vec3 from a glm::vec3.

    /**
     * @brief Bind a texture to the next available texture unit and set the sampler uniform.
     * @param name    GLSL sampler uniform name (e.g. "uDiffuse").
     * @param texture Texture to bind; must not be nullptr.
     */
    void SetTexture(const std::string &name, Texture *texture);
    /// @}

private:
    std::unordered_map<std::string, GLint> m_uniformLocationCache; ///< Name → GL location cache.
    GLuint                                 m_shaderProgramID    = 0; ///< Underlying GL program handle.
    int                                    m_currentTextureUnit = 0; ///< Next free texture unit index.
};

}  // namespace ENG
