#pragma once

#include "../screen/screen.hpp"

namespace Input {

enum class Key {

    W,
    A,
    S,
    D,
    SPACE,
    Q,
    E,
    UP,
    DOWN,
    LEFT,
    RIGHT,

    R,
    F,

};

bool is_key_down(Key key, Screen &screen);
bool key_pressed_once(Key key, Screen &screen);

} // namespace Input
