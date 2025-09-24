// vector.hpp
#pragma once
#include <numbers>
#include <cmath>
#include <Windows.h>
#include "view_matrix.hpp" // Incluir la definición de view_matrix_t

class Vector
{
public:
    constexpr Vector(
        const float x = 0.f,
        const float y = 0.f,
        const float z = 0.f) noexcept :
        x(x), y(y), z(z) {
    }

    constexpr const Vector& operator-(const Vector& other) const noexcept;
    constexpr const Vector& operator+(const Vector& other) const noexcept;
    constexpr const Vector& operator/(const float factor) const noexcept;
    constexpr const Vector& operator*(const float factor) const noexcept;

    // 3d -> 2d, explanations already exist.
    const static bool world_to_screen(view_matrix_t view_matrix, Vector& in, Vector& out);

    const bool IsZero();

    float x, y, z;
};