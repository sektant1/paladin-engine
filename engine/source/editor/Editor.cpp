#include <algorithm>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "editor/Editor.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Log.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "graphics/RenderSettings.h"
#include "imgui.h"
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
    if (!ImGui_ImplOpenGL3_Init(kGlslVersionDirective))
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
    GLFWwindow *w = glfwGetCurrentContext();
    {
        static bool wasDown = false;
        bool        down    = (w != nullptr) && (glfwGetKey(w, GLFW_KEY_F1) == GLFW_PRESS);
        if (down && !wasDown)
        {
            ToggleVisible();
        }
        wasDown = down;
    }

    // Detach cursor when editor open so the mouse can click ImGui widgets instead
    // of driving the in-game camera. Flip back to disabled (locked+hidden) on close.
    if (w != nullptr)
    {
        int desired = m_visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
        int current = glfwGetInputMode(w, GLFW_CURSOR);
        if (current != desired)
        {
            glfwSetInputMode(w, GLFW_CURSOR, desired);
            // Re-sync cached cursor position so the first frame after a mode flip
            // doesn't produce a huge delta that spins the camera.
            f64 x = 0, y = 0;
            glfwGetCursorPos(w, &x, &y);
            auto &im = Engine::GetInstance().GetInputManager();
            vec2  p(static_cast<f32>(x), static_cast<f32>(y));
            im.SetMousePositionOld(p);
            im.SetMousePositionCurrent(p);
            im.SetMousePositionChanged(false);
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

// ---- Layout ----------------------------------------------------------
//
// Stock ImGui master is vendored (no docking branch), so we simulate a
// docked engine-style layout by pinning each panel to a fixed region of
// the viewport. Widths are tuned for a 1280x720 default window; they
// follow the viewport on resize.

namespace
{
constexpr f32 kLeftWidth    = 300.0F;  // Hierarchy column
constexpr f32 kRightWidth   = 200.0F;  // Inspector column
constexpr f32 kBottomHeight = 300.0F;  // Console + Render row

constexpr ImGuiWindowFlags kDockedFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

struct DockRects
{
    ImVec2 hierarchyPos, hierarchySize;
    ImVec2 inspectorPos, inspectorSize;
    ImVec2 consolePos, consoleSize;
    ImVec2 renderPos, renderSize;
    ImVec2 statsPos, statsSize;
};

DockRects ComputeDock(f32 menuH)
{
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    f32                  vx = vp->WorkPos.x;
    f32                  vy = vp->WorkPos.y + menuH;
    f32                  vw = vp->WorkSize.x;
    f32                  vh = vp->WorkSize.y - menuH;

    f32 leftW   = (vw < kLeftWidth + kRightWidth + 200.0F) ? vw * 0.22F : kLeftWidth;
    f32 rightW  = (vw < kLeftWidth + kRightWidth + 200.0F) ? vw * 0.26F : kRightWidth;
    f32 bottomH = (vh < kBottomHeight + 200.0F) ? vh * 0.28F : kBottomHeight;

    DockRects r {};
    // Left column: Render (full height, no gap at the bottom).
    r.renderPos  = ImVec2(vx, vy);
    r.renderSize = ImVec2(leftW, vh);

    r.inspectorPos  = ImVec2(vx + vw - rightW, vy);
    r.inspectorSize = ImVec2(rightW, vh);

    f32 bottomY = vy + vh - bottomH;
    f32 bottomX = vx + leftW;
    f32 bottomW = vw - leftW - rightW;
    f32 halfW   = bottomW * 0.5F;

    // Bottom row: Console (left half), Hierarchy (right half).
    r.consolePos  = ImVec2(bottomX, bottomY);
    r.consoleSize = ImVec2(halfW, bottomH);

    r.hierarchyPos  = ImVec2(bottomX + halfW, bottomY);
    r.hierarchySize = ImVec2(bottomW - halfW, bottomH);

    // Stats: compact overlay top-left of viewport area (inside the 3D view).
    r.statsPos  = ImVec2(vx + leftW + 10.0F, vy + 10.0F);
    r.statsSize = ImVec2(190.0F, 90.0F);
    return r;
}
}  // namespace

void Editor::Draw()
{
    if (!m_initialized || !m_visible)
    {
        return;
    }

    DrawMenuBar();

    f32       menuH = ImGui::GetFrameHeight();
    DockRects d     = ComputeDock(menuH);

    if (m_showHierarchy)
    {
        ImGui::SetNextWindowPos(d.hierarchyPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(d.hierarchySize, ImGuiCond_Always);
        DrawHierarchy();
    }
    if (m_showInspector)
    {
        ImGui::SetNextWindowPos(d.inspectorPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(d.inspectorSize, ImGuiCond_Always);
        DrawInspector();
    }
    if (m_showConsole)
    {
        ImGui::SetNextWindowPos(d.consolePos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(d.consoleSize, ImGuiCond_Always);
        DrawConsole();
    }
    if (m_showRender)
    {
        ImGui::SetNextWindowPos(d.renderPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(d.renderSize, ImGuiCond_Always);
        DrawRender();
    }
    if (m_showStats)
    {
        ImGui::SetNextWindowPos(d.statsPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(d.statsSize, ImGuiCond_Always);
        DrawStats();
    }
    if (m_showDemo)
    {
        ImGui::ShowDemoWindow(&m_showDemo);
    }

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
        ImGui::MenuItem("Console", nullptr, &m_showConsole);
        ImGui::MenuItem("Render", nullptr, &m_showRender);
        ImGui::MenuItem("Stats", nullptr, &m_showStats);
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
    if (children.empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (m_selected == obj)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

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
    if (!ImGui::Begin("Hierarchy", &m_showHierarchy, kDockedFlags))
    {
        ImGui::End();
        return;
    }
    Scene *scene = Engine::GetInstance().GetScene();
    if (scene == nullptr)
    {
        ImGui::TextUnformatted("No active scene");
    } else
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
    if (ImGui::SliderFloat("FOV", &fov, 10.0F, 170.0F))
    {
        c->SetFov(fov);
    }
    if (ImGui::DragFloat("Near", &near, 0.01F, 0.001F, far - 0.01F))
    {
        c->SetNearPlane(near);
    }
    if (ImGui::DragFloat("Far", &far, 1.0F, near + 0.01F, 100000.0F))
    {
        c->SetFarPlane(far);
    }
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
    f32 ms   = c->GetMoveSpeed();
    f32 sens = c->GetSensitivity();
    f32 jump = c->GetJumpSpeed();
    if (ImGui::DragFloat("Move Speed", &ms, 0.05F, 0.0F, 1000.0F))
    {
        c->SetMoveSpeed(ms);
    }
    if (ImGui::DragFloat("Sensitivity", &sens, 0.1F, 0.0F, 500.0F))
    {
        c->SetSensitivity(sens);
    }
    if (ImGui::DragFloat("Jump Speed", &jump, 0.05F, 0.0F, 1000.0F))
    {
        c->SetJumpSpeed(jump);
    }
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
    if (!ImGui::Begin("Inspector", &m_showInspector, kDockedFlags))
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

    if (auto *cam = m_selected->GetComponent<CameraComponent>())
    {
        InspectCamera(cam);
    }
    if (auto *lit = m_selected->GetComponent<LightComponent>())
    {
        InspectLight(lit);
    }
    if (auto *pc = m_selected->GetComponent<PlayerControllerComponent>())
    {
        InspectPlayer(pc);
    }
    if (auto *mc = m_selected->GetComponent<MeshComponent>())
    {
        InspectMesh(mc);
    }

    ImGui::End();
}

// ---- Console ----------------------------------------------------------

void Editor::DrawConsole()
{
    if (!ImGui::Begin("Console", &m_showConsole, kDockedFlags))
    {
        ImGui::End();
        return;
    }

    static bool showInfo    = true;
    static bool showWarn    = true;
    static bool showError   = true;
    static bool autoScroll  = true;
    static char filter[128] = {0};

    ImGui::Checkbox("Info", &showInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warn", &showWarn);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &showError);
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &autoScroll);
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
    {
        LogClear();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.0F);
    ImGui::InputTextWithHint("##filter", "filter", filter, sizeof(filter));

    ImGui::Separator();

    if (ImGui::BeginChild("console_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const auto &entries = LogGetEntries();
        for (const auto &e : entries)
        {
            bool pass = false;
            switch (e.level)
            {
                case LogLevel::Info:
                    pass = showInfo;
                    break;
                case LogLevel::Warn:
                    pass = showWarn;
                    break;
                case LogLevel::Error:
                case LogLevel::Fatal:
                    pass = showError;
                    break;
            }
            if (!pass)
            {
                continue;
            }
            if (filter[0] != 0 && e.text.find(filter) == std::string::npos)
            {
                continue;
            }

            ImVec4 col;
            switch (e.level)
            {
                case LogLevel::Info:
                    col = ImVec4(0.75F, 0.85F, 0.75F, 1.0F);
                    break;
                case LogLevel::Warn:
                    col = ImVec4(1.00F, 0.85F, 0.40F, 1.0F);
                    break;
                case LogLevel::Error:
                    col = ImVec4(1.00F, 0.45F, 0.45F, 1.0F);
                    break;
                case LogLevel::Fatal:
                    col = ImVec4(1.00F, 0.20F, 0.20F, 1.0F);
                    break;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, col);
            ImGui::TextUnformatted(e.text.c_str());
            ImGui::PopStyleColor();
        }
        if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0F)
        {
            ImGui::SetScrollHereY(1.0F);
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

// ---- Render settings --------------------------------------------------

void Editor::DrawRender()
{
    if (!ImGui::Begin("Render", &m_showRender, kDockedFlags))
    {
        ImGui::End();
        return;
    }

    RenderSettings &rs = Engine::GetInstance().GetRenderSettings();

    ImGui::SeparatorText("Clear");
    ImGui::ColorEdit4("Clear color", glm::value_ptr(rs.clearColor));

    ImGui::SeparatorText("Internal resolution (pixelation)");
    ImGui::Checkbox("Use internal resolution FBO", &rs.useInternalRes);
    ImGui::DragInt("Internal width", &rs.internalW, 1, kInternalResMin, kInternalResMax);
    ImGui::DragInt("Internal height", &rs.internalH, 1, kInternalResMin, kInternalResMax);
    if (ImGui::Button("160x120"))
    {
        rs.internalW = kInternalPresetTinyW;
        rs.internalH = kInternalPresetTinyH;
    }
    ImGui::SameLine();
    if (ImGui::Button("320x240"))
    {
        rs.internalW = kInternalPresetLowW;
        rs.internalH = kInternalPresetLowH;
    }
    ImGui::SameLine();
    if (ImGui::Button("640x480"))
    {
        rs.internalW = kInternalPresetMedW;
        rs.internalH = kInternalPresetMedH;
    }
    ImGui::SameLine();
    if (ImGui::Button("Native"))
    {
        rs.useInternalRes = false;
    }

    ImGui::SeparatorText("PSX shader uniforms");
    ImGui::DragFloat("uSnapResolutionX", &rs.snapX, 1.0F, 0.0F, 4096.0F);
    ImGui::DragFloat("uSnapResolutionY", &rs.snapY, 1.0F, 0.0F, 4096.0F);
    ImGui::DragFloat("uFogStart", &rs.fogStart, 0.1F, 0.0F, 10000.0F);
    ImGui::DragFloat("uFogEnd", &rs.fogEnd, 0.1F, 0.0F, 10000.0F);
    ImGui::SliderFloat("uAmbient", &rs.ambient, 0.0F, 1.0F);
    ImGui::DragFloat3("uLightDir", glm::value_ptr(rs.lightDir), 0.01F);
    ImGui::SliderFloat("uColorDepth", &rs.colorDepth, 0.0F, 64.0F);
    ImGui::SliderFloat("uDitherStrength", &rs.ditherStrength, 0.0F, 1.0F);

    ImGui::End();
}

// ---- Stats ------------------------------------------------------------

void Editor::DrawStats()
{
    if (!ImGui::Begin("Stats", &m_showStats, kDockedFlags | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::End();
        return;
    }
    f32 dt        = ImGui::GetIO().DeltaTime;
    f32 fps       = (dt > 0.0F) ? (1.0F / dt) : 0.0F;
    m_fpsSmoothed = (m_fpsSmoothed == 0.0F) ? fps : (m_fpsSmoothed * 0.9F + fps * 0.1F);

    ImGui::Text("FPS:        %6.1f", m_fpsSmoothed);
    ImGui::Text("Frame time: %6.2f ms", dt * 1000.0F);
    ImGui::Text("Draw calls: %d", m_lastDrawCount);
    ImGui::End();
}

}  // namespace COA
