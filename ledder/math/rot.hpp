#pragma once

#include "./vec.hpp"
#include "./sin.hpp"

// precomputed rotation
struct Rot2
{
    fp_t sina;
    fp_t cosa;

    constexpr explicit Rot2(fp_t angle) noexcept
        : sina{Sin(angle)}
        , cosa{Cos(angle)}
    {}

    constexpr Rot2(fp_t sina, fp_t cosa) noexcept
        : sina{sina}
        , cosa{cosa}
    {}

    constexpr Vec2 Apply(Vec2 const& v) const;
};

constexpr Vec2 Rot2::Apply(Vec2 const& v) const
{
    return Vec2{v.x * cosa - v.y * sina, v.x * sina + v.y * cosa};
}