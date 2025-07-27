#pragma once

// a terminal emulator that prints to the framebuffer
// currently doesn't really emulate a terminal, but just provides a set of funcs
// to print text to the framebuffer

#include "color.hpp"
#include "frame.hpp"
#include "vector/vec2.hpp"
#include <string_view>

class FTerm {

    Frame &frame;
    Vec2i cursor = Vec2i::zero(); // measured in terms of characters, not pixels

    Vec2i get_px_cursor() const;
    Vec2i max_cursor() const;
    void inc_cursor();
    void new_line();
    void render_letter(char c);
    void print_letter(char c);

public:
    Color foreground = Color(255, 255, 255);

    explicit FTerm(Frame &frame);
    FTerm(Frame &frame, const Color &foreground);

    void move_cursor(Vec2i char_pos);
    void print_char(int c);
    void print_str(std::string_view str);
};
