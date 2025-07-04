#include "triangle.hpp"

bool Triangle::point_inside(Vec2 &p) const
{
    // barycentric coordinates are used
    float s = 1.f / (2.f * -this->signed_area()) *
              (this->scr_vs[0].y * this->scr_vs[2].x -
               this->scr_vs[0].x * this->scr_vs[2].y +
               (this->scr_vs[2].y - this->scr_vs[0].y) * p.x +
               (this->scr_vs[0].x - this->scr_vs[2].x) * p.y);
    float t = 1.f / (2.f * -this->signed_area()) *
              (this->scr_vs[0].x * this->scr_vs[1].y -
               this->scr_vs[0].y * this->scr_vs[1].x +
               (this->scr_vs[0].y - this->scr_vs[1].y) * p.x +
               (this->scr_vs[1].x - this->scr_vs[0].x) * p.y);

    bool s_in_range = 0.f <= s && s <= 1.f;
    bool t_in_range = 0.f <= t && t <= 1.f;
    bool total_in_range = s + t <= 1.f;

    return s_in_range && t_in_range && total_in_range;
}
