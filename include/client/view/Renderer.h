#pragma once
#include "logic/engine/GameSnapshot.h"

// Strategy interface for turning a GameSnapshot into a visible display. No
// concrete implementation exists yet (see CLAUDE.md: business logic only,
// no graphics yet) — a future renderer built on a graphics library
// implements this without any change to GameEngine or GameSnapshot.
class Renderer {
public:
    virtual ~Renderer() = default;
    virtual void render(const GameSnapshot& snapshot) = 0;
};
