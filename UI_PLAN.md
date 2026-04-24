Context

 Goal: in-game editor/viewer layered on top of
 the existing GL render so a developer can
 inspect and tweak live state —
 camera/player/object transforms, lights,
 shader/retro parameters, asset loading, and
 pixel-art internal resolution — without
 rebuilding.

 ImGui is already vendored and compiled as an
 imgui static target
 (engine/CMakeLists.txt:66-78, linked at line
 155). Backends imgui_impl_glfw.cpp and
 imgui_impl_opengl3.cpp are present. No editor
 yet — rendering goes straight to the default
 framebuffer, and there is no FBO/post-process
 path.

 The retro PSX shader
 (assets/shaders/psx.{vert,frag}) already does
 vertex snapping + color quantization at the
 shader level, but "internal resolution" in the
  PSX sense means rendering to a low-res FBO
 and upscaling with nearest-neighbor — that's
 the missing piece.

 Scope

 One new subsystem COA::Editor living in
 engine/source/editor/, plus a small
 COA::RenderTarget in engine/source/graphics/
 for the internal-resolution FBO. Engine main
 loop gets two short insertions. No changes to
 game-side (source/) code required, but Game is
  free to push custom panels into the editor.

 ---
 1. New files

 engine/source/editor/Editor.h / Editor.cpp

 Singleton-ish, owned by Engine (like
 InputManager/GraphicsAPI). Public API:

 namespace COA {
 class Editor {
 public:
     bool Init(GLFWwindow* window);        //
 ImGui::CreateContext +
 ImGui_ImplGlfw_InitForOpenGL(win, true) +
 ImGui_ImplOpenGL3_Init("#version 330 core")
     void BeginFrame();                    //
 ImGui_ImplOpenGL3_NewFrame +
 ImGui_ImplGlfw_NewFrame + ImGui::NewFrame
     void Draw();                          //
 builds all panels
     void EndFrame();                      //
 ImGui::Render +
 ImGui_ImplOpenGL3_RenderDrawData
     void Shutdown();

     bool IsVisible() const { return m_visible;
  }
     void ToggleVisible() { m_visible =
 !m_visible; }

     // Panels can be toggled individually
     bool WantsCaptureMouse() const;       //
 forwards ImGui::GetIO().WantCaptureMouse
     bool WantsCaptureKeyboard() const;

     // Game-side extension hook: register
 extra panels
     using PanelFn = std::function<void()>;
     void RegisterPanel(std::string name,
 PanelFn fn);

 private:
     bool  m_visible = true;
     bool  m_showScene = true, m_showInspector
 = true, m_showAssets = true,
           m_showRender = true, m_showStats =
 true, m_showDemo = false;
     GameObject* m_selected = nullptr;
     std::vector<std::pair<std::string,
 PanelFn>> m_customPanels;

     void DrawMenuBar();
     void DrawSceneHierarchy();            //
 tree of Scene::m_objects, click to select
     void DrawInspector();                 //
 transform + per-component editors for
 m_selected
     void DrawAssets();                    //
 filesystem walk of ASSETS_DIR
     void DrawRenderSettings();            //
 clear color, PSX uniforms, FBO resolution
     void DrawStats();                     //
 fps, frametime, draw count
 };
 }

 engine/source/editor/panels/ — split-file
 implementations

 Keep Editor.cpp thin. One file per panel so
 they can be reviewed in isolation:
 - HierarchyPanel.cpp — recursive walk of
 Scene::m_objects using GameObject::m_children
 (scene/GameObject.h:213). Selection by raw
 GameObject* pointer.
 - InspectorPanel.cpp — for m_selected:
   - Transform: GetPosition/Rotation/Scale +
 SetPosition/SetRotation/SetScale
 (GameObject.h:156-163). Rotation: store Euler
 angles in a scratch buffer, convert to/from
 quat on edit.
   - CameraComponent: expose m_fov,
 m_nearPlane, m_farPlane
 (CameraComponent.h:60-62) via new
 getters/setters (add if missing).
   - LightComponent: GetColor/SetColor
 (LightComponent.h:47,54) — ImGui::ColorEdit3.
   - PlayerControllerComponent: m_sensitivity,
 m_moveSpeed, m_jumpSpeed
 (PlayerControllerComponent.h:47-51). SetMS
 already exists; add simple setters for the
 other two.
   - MeshComponent: display mesh/material names
  only (read-only for v1; swapping material is
 a later increment).
 - AssetsPanel.cpp —
 std::filesystem::recursive_directory_iterator(
 Engine::GetFileSystem().GetAssetsFolder())
 (io/FileSystem.h:44). Group by extension
 (.gltf/.glb, .png/.jpg, .frag/.vert, .json).
 Click-to-preview (name copy for v1; drag-drop
 into inspector is later).
 - RenderPanel.cpp — controls for:
   - Clear color (currently hardcoded 1,1,1,1
 at Engine.cpp:164; move into Engine as
 m_clearColor field and bind ImGui to it).
   - Internal resolution width/height (feeds
 RenderTarget resize).
   - PSX shader uniforms: uSnapResolutionX/Y,
 uFogStart/End, uAmbient, uColorDepth,
 uDitherStrength. Store these in a
 RenderSettings struct on Engine; RenderQueue
 reads it before issuing draws and uploads if
 the bound shader has the uniform (silent skip
 otherwise — glGetUniformLocation returns -1).
 - StatsPanel.cpp — running-average frametime,
 FPS, draw-call counter (requires exposing a
 counter on RenderQueue::Draw).

 engine/source/graphics/RenderTarget.h / .cpp

 Thin FBO wrapper:

 class RenderTarget {
 public:
     bool Create(int w, int h);            //
 gen FBO + color (RGBA8) + depth (DEPTH24)
     void Resize(int w, int h);            //
 recreate attachments on change
     void Bind();                          //
 glBindFramebuffer + glViewport(0,0,w,h)
     static void BindDefault(int winW, int 
 winH);
     GLuint ColorTex() const;
     int Width() const; int Height() const;
     void Destroy();
 private:
     GLuint m_fbo=0, m_color=0, m_depth=0;
     int m_w=0, m_h=0;
 };

 Blit helper (in the same TU): a tiny
 fullscreen-triangle pass with a
 nearest-neighbor sampled texture. One-time
 vertex-less shader (gl_VertexID trick) avoids
 touching the engine's VertexLayout. Lives next
  to RenderTarget.

 ---
 2. Edits to existing files

 engine/source/Engine.h / Engine.cpp

 Add members:
 Editor        m_editor;
 RenderTarget  m_sceneTarget;
 struct RenderSettings { int internalW=320,
 internalH=240; bool useInternalRes=true;
                         float snapX=320,
 snapY=240, fogStart=0, fogEnd=0,
 ambient=0.35f;
                         int colorDepth=5;
 float dither=0.25f;
                         vec3 lightDir{0,-1,0};
  vec4 clearColor{0,0,0,1}; } m_renderSettings;

 Accessors: GetEditor(), GetRenderSettings(),
 GetSceneTarget().

 Init (around current Engine.cpp:124 after
 GraphicsAPI init): create m_sceneTarget at
 initial internal resolution, then
 m_editor.Init(m_window).

 Run main loop — insertions at precise points
 in the current loop (Engine.cpp:142-217):

   155  glfwPollEvents();
        m_editor.BeginFrame();
           // NEW: must come after poll

   162  m_application->Update(deltaTime);
        // NEW: gate input into app/player via
 Editor::WantsCaptureMouse/Keyboard
        //      — implemented inside
 PlayerControllerComponent/InputManager by
 reading Editor state

        // NEW: bind internal-res FBO (if
 enabled) before clear/draw
        if (settings.useInternalRes)
 m_sceneTarget.Bind();
        else RenderTarget::BindDefault(winW,
 winH);

   164  m_graphicsAPI.SetClearColor(m_renderSet
 tings.clearColor...);
   165  m_graphicsAPI.ClearBuffers();
   ...
   209  m_renderQueue.Draw(m_graphicsAPI,
 cameraData, lights);

        // NEW: blit FBO → default framebuffer
 with nearest-neighbor upscale
        if (settings.useInternalRes) {
 RenderTarget::BindDefault(winW, winH);

 BlitNearest(m_sceneTarget.ColorTex(), winW,
 winH); }

        // NEW: editor always draws against the
  default framebuffer
        m_editor.Draw();
        m_editor.EndFrame();

   211  glfwSwapBuffers(m_window);

 Destroy: m_editor.Shutdown() before
 glfwTerminate.

 engine/source/input/InputManager.h (small)

 Add bool WantGameInput() const that returns
 true when
 Engine::GetEditor().WantsCaptureKeyboard() is
 false. PlayerControllerComponent and Player
 gate on this so ImGui-focused typing doesn't
 rotate the camera. Keep the existing GLFW
 callbacks as-is — do not call
 ImGui_ImplGlfw_InstallCallbacks; instead pass
 install_callbacks=true to
 ImGui_ImplGlfw_InitForOpenGL which chains
 automatically with our existing callbacks (the
  backend preserves prior callbacks). Verify
 once at init that InputManager still receives
 events; if not, switch to explicit manual
 forwarding (set install_callbacks=false and
 call ImGui_ImplGlfw_KeyCallback etc. from our
 callbacks).

 engine/source/render/RenderQueue.cpp

 Before each material draw, query
 Engine::GetRenderSettings() and upload the
 PSX-shader uniforms if the locations exist
 (glGetUniformLocation(prog,
 "uSnapResolutionX") etc.). Silent skip on -1
 so non-PSX shaders aren't affected. Increment
 a m_drawsThisFrame counter exposed to the
 StatsPanel; reset at the top of Draw().

 engine/source/scene/components/*

 Add missing public setters so the Inspector
 can write without becoming a friend:
 - CameraComponent: SetFov/SetNear/SetFar
 (reads are needed — add getters too if
 absent).
 - PlayerControllerComponent: SetSensitivity,
 SetJumpSpeed (move speed already has SetMS).

 engine/CMakeLists.txt

 Source files in engine/source/ are picked up
 by GLOB_RECURSE CONFIGURE_DEPENDS, so new
 files in engine/source/editor/ and
 engine/source/graphics/RenderTarget.{h,cpp}
 are auto-included. Only change needed: ensure
 the imgui target is transitively reachable —
 it already is
 (target_link_libraries(${PROJECT_NAME} ...
 imgui) at line 155). Add #include "imgui.h"
 includes via the imgui target's INTERFACE
 include dirs (already configured at line
 75-76).

 ---
 3. Default keybinds

 F1 toggles editor visibility. Set in
 Editor::BeginFrame by reading InputManager
 (pre-ImGui capture). One entry added to
 CLAUDE.md if the user wants it documented.

 ---
 4. Verification

 1. ./compile.sh — debug build must succeed
 (CMake glob picks up new files).
 2. Run ./bin/debug/coagula-engine:
   - ImGui demo toggle produces the stock ImGui
  demo → backend works.
   - Hierarchy panel lists all objects in
 assets/scenes/scene.json after load.
   - Select Player/camera → Inspector shows
 transform; drag positions → object moves live.
   - Light panel → change color → scene tint
 changes immediately.
   - Render panel → set internal res to 160×120
  → game looks chunky; 1920×1080 → crisp.
   - Render panel → uColorDepth=2,
 uDitherStrength=0.5 → color banding + dither
 visible.
   - Typing in an ImGui text field does not
 steer the camera (input gating works).
 3. ./compile.sh --release — release build must
  also succeed.
 4. Close window → Engine::Destroy runs
 m_editor.Shutdown() without GL errors (check
 stderr).

 No test framework is wired up, so verification
  is manual + visual.

 ---
 5. Critical files

 Modify:
 - engine/source/Engine.h,
 engine/source/Engine.cpp — loop insertions,
 new members.
 - engine/source/render/RenderQueue.cpp — PSX
 uniform upload + draw counter.
 - engine/source/input/InputManager.h/.cpp —
 WantGameInput().
 - engine/source/scene/components/CameraCompone
 nt.h/.cpp, PlayerControllerComponent.h/.cpp —
 add setters/getters.

 Create:
 - engine/source/editor/Editor.{h,cpp}
 - engine/source/editor/panels/{Hierarchy,Inspe
 ctor,Assets,Render,Stats}Panel.cpp
 - engine/source/graphics/RenderTarget.{h,cpp}
     Modify:                
     - engine/source/Engine.h,
     engine/source/Engine.cpp — loop insertions,
      new members.                             
     - engine/source/render/RenderQueue.cpp —
     PSX uniform upload + draw counter. 
     - engine/source/input/InputManager.h/.cpp —
      WantGameInput().             
     - engine/source/scene/components/CameraComp
     onent.h/.cpp,              
     PlayerControllerComponent.h/.cpp — add
     setters/getters.
                                    
     Create:
     - engine/source/editor/Editor.{h,cpp} 
     - engine/source/editor/panels/{Hierarchy,In
     spector,Assets,Render,Stats}Panel.cpp
     -                                  
     engine/source/graphics/RenderTarget.{h,cpp}
      (+ tiny blit helper inline)             
                                            
     Reuse (no change):
     - engine/thirdparty/imgui/ (already      
     compiled).                     
     - engine/source/io/FileSystem.h:44
     (GetAssetsFolder) for the Assets panel.    
     - engine/source/scene/Scene.h +
     GameObject.h for tree walk and transform 
     edits.                         
     - PSX shader uniforms already declared in
     assets/shaders/psx.{vert,frag}.
                                 
     ---                                       
     6. Deliberately out of scope for v1
                                
     - Gizmos (ImGuizmo) — worth a second pass,
     skipped now to keep this one contained.
     - Scene save/load from the editor — JSON  
     loader exists (nlohmann_json wired) but
     serialization round-trip is its own task.
     - Docking/viewports — needs the ImGui
     docking branch; stock master is vendored.
     Use fixed side-by-side windows.
     - Hot-reload of shaders/models — asset
     picker lists only; loading is future work.
     - Undo/redo.


