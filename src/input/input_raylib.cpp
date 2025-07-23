#ifdef m_RAYLIB

#include "input.hpp"
#include <assert.h>
#include <raylib.h>

namespace {

int key_enum_to_raylib(Input::Key key)
{
    switch (key) {

    case Input::Key::W:
        return KEY_W;

    case Input::Key::A:
        return KEY_A;

    case Input::Key::S:
        return KEY_S;

    case Input::Key::D:
        return KEY_D;

    case Input::Key::SPACE:
        return KEY_SPACE;

    case Input::Key::Q:
        return KEY_Q;

    case Input::Key::E:
        return KEY_E;

    case Input::Key::UP:
        return KEY_UP;

    case Input::Key::DOWN:
        return KEY_DOWN;

    case Input::Key::LEFT:
        return KEY_LEFT;

    case Input::Key::RIGHT:
        return KEY_RIGHT;

    case Input::Key::R:
        return KEY_R;

    case Input::Key::F:
        return KEY_F;
    }
}

} // namespace

bool Input::is_key_down(Key key, Screen &screen)
{
    (void)screen;
    return IsKeyDown(key_enum_to_raylib(key));
}

bool Input::key_pressed_once(Key key, Screen &screen)
{
    (void)screen;
    return IsKeyPressed(key_enum_to_raylib(key));
}

#endif
