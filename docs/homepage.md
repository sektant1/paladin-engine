@mainpage Coagula Engine

@htmlonly
<pre class="coa-ascii" aria-hidden="true">
   ______   ______    ___     ______   __  __   __       ___
  / ____/  / __  /   /   |   / ____/  / / / /  / /      /   |
 / /      / / / /   / /| |  / / __   / / / /  / /      / /| |
/ /___   / /_/ /   / ___ | / /_/ /  / /_/ /  / /___   / ___ |
\____/   \____/   /_/  |_| \____/   \____/  /_____/  /_/  |_|

       -=[ solvet  ::  et  ::  coagula ]=-
</pre>
@endhtmlonly

> _Visita Interiora Terrae Rectificando Invenies Occultum Lapidem._

Coagula is a teaching/learning codebase. A digital grimoire.
It reduces the complexity of 3D engines to their base essence.
Small enough to see the whole. Large enough to manifest a world.

@htmlonly<div class="coa-divider" aria-hidden="true"></div>@endhtmlonly

## Get going

@htmlonly
<pre class="coa-ascii" aria-hidden="true">
+--[ Coagula (Coagulate/Bind): ]----------------------+
    The act of binding, solidifying, or synthesizing
    the purified components back together into a new,
    more perfect, and fixed form.
+-----------------------------------------------------+
</pre>
@endhtmlonly

| If you want to...                    | Go to                        |
|--------------------------------------|------------------------------|
| Compile and run                      | @ref build                   |
| Know what lives where                | @ref layout                  |
| Understand the entrypoint            | @ref starting_point          |
| Follow a step-by-step walk           | @ref tutorials               |
| Copy-paste a quick snippet           | @ref recipes                 |
| Read a longer explanation            | @ref guides                  |
| Regenerate this site                 | @ref build_docs              |
| Check the license                    | @ref license                 |
| Browse the API by subsystem          | @ref overview                |

@htmlonly<div class="coa-divider" aria-hidden="true"></div>@endhtmlonly

## One-minute tour

@htmlonly
<pre class="coa-ascii" aria-hidden="true">
+--[ ONE-MINUTE-TOUR ]------------------------------+
|  When the student is ready, the code speaks.      |
+---------------------------------------------------+
</pre>
@endhtmlonly

```cpp
#include "COA.h"

class Game : public COA::Application {
public:
    bool Init() override {
        m_scene = COA::Scene::Load("scenes/scene.json");
        COA::Engine::GetInstance().SetScene(m_scene.get());
        return true;
    }
    void RegisterTypes() override {}
    void Update(float dt) override { m_scene->Update(dt); }
    void Destroy() override {}
private:
    std::shared_ptr<COA::Scene> m_scene;
};

int main() {
    auto &e = COA::Engine::GetInstance();
    e.SetApplication(new Game());
    e.Init(1280, 720);
    e.Run();
    e.Destroy();
}
```

Then:

```sh
./compile.sh
```

That is the whole boot sequence. Everything else — rendering, physics,
input, scene graph — is driven by the `Scene` and its `Component`s.

@htmlonly<div class="coa-divider" aria-hidden="true"></div>@endhtmlonly

## Features at a glance

@htmlonly
@endhtmlonly

- **Singleton engine** with window, GL context, input, render queue, physics.
- **Command-queue renderer** — `Mesh` + `Material` paired at submit time.
- **Component scene graph** — `GameObject` tree, JSON-loadable.
- **Bullet physics** — static / dynamic / kinematic rigid bodies, capsule character controller.
- **glTF import** — mesh + skeletal animation, one call.
- **Shadertoy-style shader sandbox** — `iTime`, `iResolution`, `iMouse` uniforms wired for quick experiments.

@htmlonly<div class="coa-divider" aria-hidden="true"></div>@endhtmlonly

## Status

Educational. No warranty. See @ref license.
