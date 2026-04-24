/**
 * @file COA.h
 * @brief Umbrella include for the entire Engine library.
 *
 * Game / application code should include only this header.
 * It transitively pulls in every public engine subsystem:
 *
 * | Include pulled in          | What it gives you                              |
 * |----------------------------|------------------------------------------------|
 * | Application.h              | Base class to subclass for your game loop      |
 * | Engine.h                   | Singleton that owns all subsystems             |
 * | Types.h                    | Primitive type aliases (f32, vec3, mat4 …)     |
 * | Log.h                      | LOG_INFO / LOG_WARN / LOG_ERROR / LOG_FATAL    |
 * | Common.h                   | CameraData, LightData transfer structs         |
 * | graphics/GraphicsAPI.h     | Low-level GL wrapper (shaders, buffers)        |
 * | graphics/ShaderProgram.h   | GLSL program + uniform upload                  |
 * | graphics/Texture.h         | 2D texture + TextureManager cache              |
 * | graphics/VertexLayout.h    | VBO attribute layout description               |
 * | input/InputManager.h       | Keyboard + mouse polling                       |
 * | io/FileReader.h            | Simple file reader utility                     |
 * | io/FileSystem.h            | Asset-relative path resolution                 |
 * | physics/PhysicsManager.h   | Bullet world singleton + step                  |
 * | physics/RigidBody.h        | Rigid body wrapper                             |
 * | physics/Collider.h         | Collision shape wrapper                        |
 * | render/Builder.h           | MeshData factory helpers                       |
 * | render/Material.h          | Shader + uniform container                     |
 * | render/Mesh.h              | GPU mesh (VAO/VBO/EBO)                         |
 * | render/RenderQueue.h       | Per-frame command submission + draw            |
 * | scene/Component.h          | Base component + COMPONENT macro               |
 * | scene/GameObject.h         | Scene node with transform + components         |
 * | scene/Scene.h              | Owns the object graph, camera, lights          |
 * | scene/components/…         | Built-in components (camera, mesh, light …)    |
 */

#pragma once

#include "Application.h"
#include "Common.h"
#include "Engine.h"
#include "Log.h"
#include "Types.h"
#include "audio/AudioManager.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "graphics/VertexLayout.h"
#include "input/InputManager.h"
#include "io/FileReader.h"
#include "io/FileSystem.h"
#include "physics/Collider.h"
#include "physics/KinematicCharacterController.h"
#include "physics/PhysicsManager.h"
#include "physics/RigidBody.h"
#include "render/Builder.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include "render/RenderQueue.h"
#include "scene/Component.h"
#include "scene/GameObject.h"
#include "scene/Scene.h"
#include "scene/components/AnimationComponent.h"
#include "scene/components/CameraComponent.h"
#include "scene/components/LightComponent.h"
#include "scene/components/MeshComponent.h"
#include "scene/components/PhysicsComponent.h"
#include "scene/components/PlayerControllerComponent.h"
