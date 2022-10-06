#pragma once

#include "./fp.hpp"

template<typename T>
class Vec2Base
{
public:
    constexpr Vec2Base(T x, T y) noexcept
        : x{x}
        , y{y}
    {}

    constexpr Vec2Base(Vec2Base const&) noexcept = default;
    constexpr Vec2Base(Vec2Base &&) noexcept = default;
    constexpr Vec2Base& operator=(Vec2Base const&) noexcept = default;
    constexpr Vec2Base& operator=(Vec2Base &&) noexcept = default;

    constexpr Vec2Base operator+(Vec2Base const& v) const noexcept
    {
        return Vec2Base{x + v.x, y + v.y};
    }

    constexpr Vec2Base operator-(Vec2Base const& v) const noexcept
    {
        return Vec2Base{x - v.x, y - v.y};
    }

    constexpr Vec2Base operator*(T k) const noexcept
    {
        return Vec2Base{k * x, k * y};
    }

    constexpr Vec2Base operator*(Vec2Base const& v) const noexcept
    {
        return Vec2Base{x * v.x, y * v.y};
    }

    constexpr Vec2Base operator/(T k) const noexcept
    {
        return Vec2Base{x / k, y / k};
    }

    constexpr Vec2Base operator/(Vec2Base const& v) const noexcept
    {
        return Vec2Base{x / v.x, y / v.y};
    }

    T x, y;
};

template<typename T>
constexpr Vec2Base<T> operator*(T k, Vec2Base<T> const& v) noexcept
{
    return v * k;
}

using Vec2 = Vec2Base<fp_t>;