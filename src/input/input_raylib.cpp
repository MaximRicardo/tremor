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
    }
}

} // namespace

bool Input::is_key_down(Key key, [[maybe_unused]] Screen &screen)
{
    return IsKeyDown(key_enum_to_raylib(key));
}

#endif
