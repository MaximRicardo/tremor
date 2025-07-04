#include "triangle.hpp"

float Triangle::signed_area() const
{
    return -0.5f *
           (-this->scr_vs[1].y * this->scr_vs[2].x +
            this->scr_vs[0].y * (-this->scr_vs[1].x + this->scr_vs[2].x) +
            this->scr_vs[0].x * (this->scr_vs[1].y - this->scr_vs[2].y) +
            this->scr_vs[1].x * this->scr_vs[2].y);
}
