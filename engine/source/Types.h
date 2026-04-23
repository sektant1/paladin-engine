/**
 * @file Types.h
 * @ingroup coa_types
 * @brief Engine-wide primitive type aliases and GLM math imports.
 *
 * Every engine subsystem uses these aliases instead of raw stdint or GLM types.
 * Keeping all aliases here means a single include gives consistent sizing and
 * naming across the entire codebase (both the Engine library and game code).
 *
 * Math types (vec3, mat4, quat …) are thin aliases over GLM and can be passed
 * directly to any OpenGL uniform upload helper in ShaderProgram.
 */

#ifndef TYPES_H
#define TYPES_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace COA
{

/// @defgroup Types Primitive Type Aliases
/// @brief Sized integer, float, and math type aliases used everywhere in the engine.
/// @{

using u8  = uint8_t;   ///< Unsigned  8-bit integer.
using u16 = uint16_t;  ///< Unsigned 16-bit integer.
using u32 = uint32_t;  ///< Unsigned 32-bit integer — also the GL index type.
using u64 = uint64_t;  ///< Unsigned 64-bit integer.

using i8  = int8_t;   ///< Signed  8-bit integer.
using i16 = int16_t;  ///< Signed 16-bit integer.
using i32 = int32_t;  ///< Signed 32-bit integer — used for GL enums and sizes.
using i64 = int64_t;  ///< Signed 64-bit integer.

using f32 = float;   ///< 32-bit IEEE float — the standard GL scalar type.
using f64 = double;  ///< 64-bit double — used for high-precision timing.

using usize = size_t;  ///< Platform-native unsigned size type.

using str = std::string;  ///< Convenience alias for std::string.

/// @defgroup MathTypes GLM Math Type Aliases
/// @brief Floating-point vector, matrix, and quaternion types backed by GLM.
/// @{
using vec2 = glm::vec2;  ///< 2-component float vector (e.g. UV coordinates, mouse position).
using vec3 = glm::vec3;  ///< 3-component float vector (e.g. world position, RGB colour, normals).
using vec4 = glm::vec4;  ///< 4-component float vector (e.g. RGBA colour, homogeneous coords).

using ivec2 = glm::ivec2;  ///< 2-component signed integer vector.
using ivec3 = glm::ivec3;  ///< 3-component signed integer vector.
using ivec4 = glm::ivec4;  ///< 4-component signed integer vector.

using uvec2 = glm::uvec2;  ///< 2-component unsigned integer vector.
using uvec3 = glm::uvec3;  ///< 3-component unsigned integer vector.
using uvec4 = glm::uvec4;  ///< 4-component unsigned integer vector.

using mat3 = glm::mat3;  ///< 3×3 column-major float matrix (e.g. normal matrix).
using mat4 = glm::mat4;  ///< 4×4 column-major float matrix (model/view/projection).

using quat = glm::quat;  ///< Unit quaternion for rotation (avoids gimbal lock).
/// @}

/// @brief Bring frequently used GLM free functions into the ENG namespace.
using glm::angleAxis;
using glm::cross;
using glm::degrees;
using glm::distance;
using glm::dot;
using glm::inverse;
using glm::length;
using glm::lookAt;
using glm::mat4_cast;
using glm::normalize;
using glm::ortho;
using glm::perspective;
using glm::quat_cast;
using glm::radians;
using glm::rotate;
using glm::scale;
using glm::translate;
using glm::transpose;
using glm::value_ptr;

/// @}

}  // namespace COA

#endif  // TYPES_H
