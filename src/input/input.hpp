#pragma once

#include "../screen/screen.hpp"

namespace Input {

enum class Key {

    W,
    A,
    S,
    D,
    SPACE,
    LEFT,
    RIGHT,

};

bool is_key_down(Key key, Screen &screen);

} // namespace Input
