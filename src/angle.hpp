#pragma once

class Angle {

    // the angle is internally stored in radians
    float m_radians;

public:
    enum class Type {
        DEGREES,
        RADIANS,
    };

    explicit Angle(float angle, enum Type angle_t = Type::RADIANS);

    void set(float angle, enum Type angle_t = Type::RADIANS);
    float get(enum Type angle_t = Type::RADIANS) const;

    Angle operator+(const Angle &other) const;
    Angle operator+=(const Angle &other);
    Angle operator-(const Angle &other) const;
    Angle operator-=(const Angle &other);
    Angle operator-() const;
    bool operator<(const Angle &other) const;
    bool operator<=(const Angle &other) const;
    bool operator>(const Angle &other) const;
    bool operator>=(const Angle &other) const;
    bool operator==(const Angle &other) const;

    static float convert(float angle, enum Type src_t, enum Type dest_t);
};

// describes an angle in 3D space
class EulerAngle {

public:
    Angle x, y, z;

    EulerAngle(Angle x, Angle y, Angle z);

    EulerAngle operator-() const;
};
