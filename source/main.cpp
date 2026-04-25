#include "COA.h"
#include "Game.h"
#include "GameConstants.h"

int main()
{
    LOG_INFO("Application starting");

    Game *game = new Game();

    COA::Engine &engine = COA::Engine::GetInstance();

    engine.SetApplication(game);

    if (engine.Init(kWindowWidth, kWindowHeight)) {
        engine.Run();
    } else {
        LOG_ERROR("Engine init failed, exiting");
    }

    engine.Destroy();

    LOG_INFO("Application exiting");
    return 0;
}
