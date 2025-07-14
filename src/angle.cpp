#include "angle.hpp"
#include "constants.hpp"

namespace {

float convert_from_degrees(float angle, enum Angle::Type dest_t)
{
    switch (dest_t) {

    case Angle::Type::DEGREES:
        return angle;

    case Angle::Type::RADIANS:
        return angle * Consts::deg_2_rad_mul;
    }
}

float convert_from_radians(float angle, enum Angle::Type dest_t)
{
    switch (dest_t) {

    case Angle::Type::DEGREES:
        return angle * Consts::rad_2_deg_mul;

    case Angle::Type::RADIANS:
        return angle;
    }
}

} // namespace

Angle::Angle(float angle, enum Type angle_t)
    : m_radians(convert(angle, angle_t, Type::RADIANS))
{}

void Angle::set(float angle, enum Type angle_t)
{
    this->m_radians = convert(angle, angle_t, Type::RADIANS);
}

float Angle::get(enum Type angle_t) const
{
    return convert(this->m_radians, Type::RADIANS, angle_t);
}

float Angle::convert(float angle, enum Type src_t, enum Type dest_t)
{
    switch (src_t) {

    case Angle::Type::DEGREES:
        return convert_from_degrees(angle, dest_t);

    case Angle::Type::RADIANS:
        return convert_from_radians(angle, dest_t);
    }
}

Angle Angle::operator+(const Angle &other) const
{
    return Angle(this->m_radians + other.m_radians);
}

Angle Angle::operator+=(const Angle &other)
{
    return *this = *this + other;
}

Angle Angle::operator-(const Angle &other) const
{
    return Angle(this->m_radians - other.m_radians);
}

Angle Angle::operator-=(const Angle &other)
{
    return *this = *this - other;
}

EulerAngle::EulerAngle(Angle x, Angle y, Angle z) : x(x), y(y), z(z) {}
