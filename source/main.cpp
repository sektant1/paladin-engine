#include "ENG.h"
#include "Game.h"

int main()
{
    Game        *game   = new Game();
    ENG::Engine &engine = ENG::Engine::GetInstance();
    engine.SetApplication(game);

    if (engine.Init(1280, 720)) {
        engine.Run();
    }

    engine.Destroy();

    return 0;
}

