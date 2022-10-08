#pragma once

#include "./fp.hpp"

template<typename T>
class Vec2Base
{
public:
    template<typename U>
    constexpr Vec2Base(Vec2Base<U> const& other) noexcept
        : x{static_cast<U>(other.x)}
        , y{static_cast<U>(other.y)}
    {}

    constexpr Vec2Base() noexcept
        : x{0}
        , y{0}
    {}

    constexpr Vec2Base(T x, T y) noexcept
        : x{x}
        , y{y}
    {}

    constexpr Vec2Base(Vec2Base const&) noexcept = default;
    constexpr Vec2Base(Vec2Base &&) noexcept = default;
    Vec2Base<T>& operator=(Vec2Base<T> const& other) noexcept
    {
        x = other.x;
        y = other.y;
        return *this;
    }

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