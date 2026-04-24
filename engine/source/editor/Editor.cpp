#include "editor/Editor.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine.h"
#include "Log.h"
#include "graphics/RenderSettings.h"
#include "input/InputManager.h"
#include "io/FileSystem.h"
#include "scene/GameObject.h"
#include "scene/Scene.h"
#include "scene/components/CameraComponent.h"
#include "scene/components/LightComponent.h"
#include "scene/components/MeshComponent.h"
#include "scene/components/PlayerControllerComponent.h"

namespace COA
{

bool Editor::Init(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // install_callbacks=true chains our existing GLFW callbacks — they still
    // feed InputManager, and ImGui gets a copy of the event.
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        LOG_ERROR("ImGui GLFW backend init failed");
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330 core"))
    {
        LOG_ERROR("ImGui OpenGL3 backend init failed");
        return false;
    }
    m_initialized = true;
    LOG_INFO("Editor initialized");
    return true;
}

void Editor::Shutdown()
{
    if (!m_initialized)
    {
        return;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
}

void Editor::BeginFrame()
{
    if (!m_initialized)
    {
        return;
    }
    // F1 toggles visibility (read via GLFW directly so it works even when ImGui captures keyboard).
    {
        static bool wasDown = false;
        GLFWwindow *w       = glfwGetCurrentContext();
        bool        down    = (w != nullptr) && (glfwGetKey(w, GLFW_KEY_F1) == GLFW_PRESS);
        if (down && !wasDown)
        {
            ToggleVisible();
        }
        wasDown = down;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Editor::Draw()
{
    if (!m_initialized || !m_visible)
    {
        return;
    }

    DrawMenuBar();
    if (m_showHierarchy) DrawHierarchy();
    if (m_showInspector) DrawInspector();
    if (m_showAssets)    DrawAssets();
    if (m_showRender)    DrawRender();
    if (m_showStats)     DrawStats();
    if (m_showDemo)      ImGui::ShowDemoWindow(&m_showDemo);

    for (auto &p : m_customPanels)
    {
        if (ImGui::Begin(p.first.c_str()))
        {
            p.second();
        }
        ImGui::End();
    }
}

void Editor::EndFrame()
{
    if (!m_initialized)
    {
        return;
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Editor::WantsCaptureMouse() const
{
    return m_initialized && m_visible && ImGui::GetIO().WantCaptureMouse;
}

bool Editor::WantsCaptureKeyboard() const
{
    return m_initialized && m_visible && ImGui::GetIO().WantCaptureKeyboard;
}

// ---- Menu bar ---------------------------------------------------------

void Editor::DrawMenuBar()
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }
    if (ImGui::BeginMenu("Windows"))
    {
        ImGui::MenuItem("Hierarchy", nullptr, &m_showHierarchy);
        ImGui::MenuItem("Inspector", nullptr, &m_showInspector);
        ImGui::MenuItem("Assets",    nullptr, &m_showAssets);
        ImGui::MenuItem("Render",    nullptr, &m_showRender);
        ImGui::MenuItem("Stats",     nullptr, &m_showStats);
        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", nullptr, &m_showDemo);
        ImGui::EndMenu();
    }
    ImGui::Text("  |  F1: toggle editor");
    ImGui::EndMainMenuBar();
}

// ---- Hierarchy --------------------------------------------------------

void Editor::DrawObjectNode(GameObject *obj)
{
    if (obj == nullptr)
    {
        return;
    }
    const auto &children = obj->GetChildren();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (children.empty())       flags |= ImGuiTreeNodeFlags_Leaf;
    if (m_selected == obj)      flags |= ImGuiTreeNodeFlags_Selected;

    bool open = ImGui::TreeNodeEx(obj, flags, "%s", obj->GetName().c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        m_selected = obj;
    }
    if (open)
    {
        for (auto &c : children)
        {
            DrawObjectNode(c.get());
        }
        ImGui::TreePop();
    }
}

void Editor::DrawHierarchy()
{
    if (!ImGui::Begin("Hierarchy", &m_showHierarchy))
    {
        ImGui::End();
        return;
    }
    Scene *scene = Engine::GetInstance().GetScene();
    if (scene == nullptr)
    {
        ImGui::TextUnformatted("No active scene");
    }
    else
    {
        for (auto &root : scene->GetRootObjects())
        {
            DrawObjectNode(root.get());
        }
    }
    ImGui::End();
}

// ---- Inspector --------------------------------------------------------

static void InspectTransform(GameObject *obj)
{
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    vec3 pos = obj->GetPosition();
    if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.05F))
    {
        obj->SetPosition(pos);
    }

    quat rot   = obj->GetRotation();
    vec3 euler = glm::degrees(glm::eulerAngles(rot));
    if (ImGui::DragFloat3("Rotation (deg)", glm::value_ptr(euler), 0.5F))
    {
        obj->SetRotation(quat(glm::radians(euler)));
    }

    vec3 s = obj->GetScale();
    if (ImGui::DragFloat3("Scale", glm::value_ptr(s), 0.01F, 0.0F, 0.0F))
    {
        obj->SetScale(s);
    }
}

static void InspectCamera(CameraComponent *c)
{
    if (!ImGui::CollapsingHeader("CameraComponent", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    f32 fov  = c->GetFov();
    f32 near = c->GetNearPlane();
    f32 far  = c->GetFarPlane();
    if (ImGui::SliderFloat("FOV",   &fov,  10.0F, 170.0F))  c->SetFov(fov);
    if (ImGui::DragFloat  ("Near",  &near, 0.01F, 0.001F, far - 0.01F)) c->SetNearPlane(near);
    if (ImGui::DragFloat  ("Far",   &far,  1.0F,  near + 0.01F, 100000.0F)) c->SetFarPlane(far);
}

static void InspectLight(LightComponent *c)
{
    if (!ImGui::CollapsingHeader("LightComponent", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    vec3 col = c->GetColor();
    if (ImGui::ColorEdit3("Color", glm::value_ptr(col), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR))
    {
        c->SetColor(col);
    }
}

static void InspectPlayer(PlayerControllerComponent *c)
{
    if (!ImGui::CollapsingHeader("PlayerControllerComponent", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    f32 ms   = c->GetMS();
    f32 sens = c->GetSensitivity();
    f32 jump = c->GetJumpSpeed();
    if (ImGui::DragFloat("Move Speed",  &ms,   0.05F, 0.0F, 1000.0F)) c->SetMS(ms);
    if (ImGui::DragFloat("Sensitivity", &sens, 0.1F,  0.0F, 500.0F))  c->SetSensitivity(sens);
    if (ImGui::DragFloat("Jump Speed",  &jump, 0.05F, 0.0F, 1000.0F)) c->SetJumpSpeed(jump);
    ImGui::Text("On ground: %s", c->OnGround() ? "yes" : "no");
}

static void InspectMesh(MeshComponent *)
{
    if (!ImGui::CollapsingHeader("MeshComponent"))
    {
        return;
    }
    ImGui::TextUnformatted("(mesh + material — read-only)");
}

void Editor::DrawInspector()
{
    if (!ImGui::Begin("Inspector", &m_showInspector))
    {
        ImGui::End();
        return;
    }
    if (m_selected == nullptr)
    {
        ImGui::TextUnformatted("Nothing selected");
        ImGui::End();
        return;
    }

    ImGui::Text("Name: %s", m_selected->GetName().c_str());

    bool active = m_selected->IsActive();
    if (ImGui::Checkbox("Active", &active))
    {
        m_selected->SetActive(active);
    }

    InspectTransform(m_selected);

    if (auto *cam = m_selected->GetComponent<CameraComponent>()) InspectCamera(cam);
    if (auto *lit = m_selected->GetComponent<LightComponent>())  InspectLight(lit);
    if (auto *pc  = m_selected->GetComponent<PlayerControllerComponent>()) InspectPlayer(pc);
    if (auto *mc  = m_selected->GetComponent<MeshComponent>())   InspectMesh(mc);

    ImGui::End();
}

// ---- Assets -----------------------------------------------------------

void Editor::DrawAssets()
{
    if (!ImGui::Begin("Assets", &m_showAssets))
    {
        ImGui::End();
        return;
    }

    auto root = Engine::GetInstance().GetFileSystem().GetAssetsFolder();
    ImGui::Text("Root: %s", root.string().c_str());
    ImGui::Separator();

    std::map<std::string, std::vector<std::string>> byExt;
    std::error_code                                  ec;
    if (std::filesystem::exists(root, ec))
    {
        for (auto it = std::filesystem::recursive_directory_iterator(root, ec);
             it != std::filesystem::recursive_directory_iterator(); it.increment(ec))
        {
            if (ec) break;
            if (!it->is_regular_file(ec))
            {
                continue;
            }
            std::string ext = it->path().extension().string();
            if (ext.empty())
            {
                ext = "(no ext)";
            }
            std::string rel = std::filesystem::relative(it->path(), root, ec).string();
            byExt[ext].push_back(std::move(rel));
        }
    }

    for (auto &group : byExt)
    {
        std::sort(group.second.begin(), group.second.end());
        if (ImGui::TreeNode(group.first.c_str(), "%s (%zu)", group.first.c_str(), group.second.size()))
        {
            for (auto &p : group.second)
            {
                ImGui::Selectable(p.c_str());
                if (ImGui::BeginPopupContextItem(p.c_str()))
                {
                    if (ImGui::MenuItem("Copy path"))
                    {
                        ImGui::SetClipboardText(p.c_str());
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

// ---- Render settings --------------------------------------------------

void Editor::DrawRender()
{
    if (!ImGui::Begin("Render", &m_showRender))
    {
        ImGui::End();
        return;
    }

    RenderSettings &rs = Engine::GetInstance().GetRenderSettings();

    ImGui::SeparatorText("Clear");
    ImGui::ColorEdit4("Clear color", glm::value_ptr(rs.clearColor));

    ImGui::SeparatorText("Internal resolution (pixelation)");
    ImGui::Checkbox("Use internal resolution FBO", &rs.useInternalRes);
    ImGui::DragInt("Internal width",  &rs.internalW, 1, 16, 4096);
    ImGui::DragInt("Internal height", &rs.internalH, 1, 16, 4096);
    if (ImGui::Button("160x120"))   { rs.internalW = 160;  rs.internalH = 120; } ImGui::SameLine();
    if (ImGui::Button("320x240"))   { rs.internalW = 320;  rs.internalH = 240; } ImGui::SameLine();
    if (ImGui::Button("640x480"))   { rs.internalW = 640;  rs.internalH = 480; } ImGui::SameLine();
    if (ImGui::Button("Native"))    { rs.useInternalRes = false; }

    ImGui::SeparatorText("PSX shader uniforms");
    ImGui::DragFloat("uSnapResolutionX", &rs.snapX, 1.0F, 0.0F, 4096.0F);
    ImGui::DragFloat("uSnapResolutionY", &rs.snapY, 1.0F, 0.0F, 4096.0F);
    ImGui::DragFloat("uFogStart",        &rs.fogStart, 0.1F, 0.0F, 10000.0F);
    ImGui::DragFloat("uFogEnd",          &rs.fogEnd,   0.1F, 0.0F, 10000.0F);
    ImGui::SliderFloat("uAmbient",       &rs.ambient,  0.0F, 1.0F);
    ImGui::DragFloat3("uLightDir",       glm::value_ptr(rs.lightDir), 0.01F);
    ImGui::SliderFloat("uColorDepth",    &rs.colorDepth, 0.0F, 64.0F);
    ImGui::SliderFloat("uDitherStrength", &rs.ditherStrength, 0.0F, 1.0F);

    ImGui::End();
}

// ---- Stats ------------------------------------------------------------

void Editor::DrawStats()
{
    if (!ImGui::Begin("Stats", &m_showStats))
    {
        ImGui::End();
        return;
    }
    f32 dt  = ImGui::GetIO().DeltaTime;
    f32 fps = (dt > 0.0F) ? (1.0F / dt) : 0.0F;
    m_fpsSmoothed = (m_fpsSmoothed == 0.0F) ? fps : (m_fpsSmoothed * 0.9F + fps * 0.1F);

    ImGui::Text("FPS:        %6.1f", m_fpsSmoothed);
    ImGui::Text("Frame time: %6.2f ms", dt * 1000.0F);
    ImGui::Text("Draw calls: %d", m_lastDrawCount);
    ImGui::End();
}

}  // namespace COA
