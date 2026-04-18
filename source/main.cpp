#include "ENG.h"
#include "Game.h"

int main()
{
    LOG_INFO("Application starting");

    Game *game = new Game();

    ENG::Engine &engine = ENG::Engine::GetInstance();

    engine.SetApplication(game);

    if (engine.Init(1280, 720)) {
        engine.Run();
    } else {
        LOG_ERROR("Engine init failed, exiting");
    }

    engine.Destroy();

    LOG_INFO("Application exiting");
    return 0;
}
