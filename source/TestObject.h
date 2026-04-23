#pragma once

#include "COA.h"

class TestObject : public COA::GameObject
{
public:
    TestObject();

    void Update(COA::f32 deltaTime) override;

private:
};
