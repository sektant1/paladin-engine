#include <iostream>

#include "ENG.h"
#include "Lab.h"

int main()
{
    Lab *lab = new Lab();

    ENG::Engine &engine = ENG::Engine::GetInstance();

    engine.SetApplication(lab);

    if (engine.Init(1280, 720)) {
        engine.Run();
    }

    engine.Destroy();

    return 0;
}
