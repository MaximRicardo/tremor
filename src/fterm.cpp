#include "fterm.hpp"
#include "color.hpp"
#include "font.hpp"
#include "index.hpp"
#include "resolution.hpp"
#include "vector/vec2.hpp"
#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string_view>

namespace {

bool char_drawable(char c)
{
    return c >= Font::bitmap_ascii_first && c <= Font::bitmap_ascii_last;
}

// DOESN'T MODIFY ARGS
int get_printf_len(const char *fmt, va_list &args)
{
    va_list args_cpy;
    va_copy(args_cpy, args);
    int len = vsnprintf(nullptr, 0, fmt, args_cpy);
    va_end(args_cpy);

    return len;
}

} // namespace

FTerm::FTerm(Frame &frame) : frame(frame) {}
FTerm::FTerm(Frame &frame, const Color &foreground)
    : frame(frame), foreground(foreground)
{}

Vec2i FTerm::get_px_cursor() const
{
    return this->cursor * Font::char_size;
}

Vec2i FTerm::max_cursor() const
{
    int32_t max_x = Res::width / Font::char_width - 1;
    int32_t max_y = Res::height / Font::char_height - 1;
    return Vec2i(max_x, max_y);
}

void FTerm::inc_cursor()
{
    if (this->cursor.x < this->max_cursor().x)
        ++this->cursor.x;
    else
        this->new_line();
}

void FTerm::new_line()
{
    this->cursor.x = 0;
    ++this->cursor.y;
    assert(this->cursor.y < this->max_cursor().y);
}

void FTerm::render_letter(char c)
{
    assert(char_drawable(c));

    // all the characters in the bitmap have this offset
    c -= Font::bitmap_ascii_first;

    Vec2i start = this->get_px_cursor();

    for (int32_t y = start.y; y < start.y + Font::char_height; ++y) {
        for (int32_t x = start.x; x < start.x + Font::char_width; ++x) {
            if (!Font::active_pixel(c, Vec2i(x, y) - start))
                continue;

            this->frame.pixels[Index::to_1d(Vec2i(x, y), Res::width)] =
                this->foreground;
        }
    }
}

void FTerm::print_letter(char c)
{
    this->render_letter(c);
    this->inc_cursor();
}

void FTerm::print_char(int c)
{
    if (c == '\0')
        return;
    else if (c == '\r')
        this->cursor.x = 0;
    else if (c == '\n')
        this->new_line();
    else
        this->print_letter(c);
}

void FTerm::print_str(std::string_view str)
{
    for (auto c : str) {
        this->print_char(c);
    }
}

void FTerm::move_cursor(Vec2i char_pos)
{
    this->cursor = char_pos;
}

int FTerm::c_putchar(int c)
{
    this->print_char(c);
    return c;
}

int FTerm::c_puts(char *str)
{
    this->print_str(str);
    return 0;
}

int FTerm::c_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int len = get_printf_len(fmt, args);

    auto str = std::make_unique<char[]>(len + 1);
    vsprintf(str.get(), fmt, args);

    this->print_str(str.get());

    va_end(args);

    return len;
}
