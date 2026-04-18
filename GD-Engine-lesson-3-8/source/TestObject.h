#pragma once

#include <eng.h>

class TestObject : public eng::GameObject
{
public:
    TestObject();

    void Update(float deltaTime) override;

private:
};