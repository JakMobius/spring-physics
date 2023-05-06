#pragma once

#pragma once

/**
 * An extremely fast implementation of four-dimensional
 * vector.
 *
 * Authored:                by KingCakeTheFruity
 * Vectorization:           by AlexRoar
 * Debug and inlining help: by JakMobius
 */

#include "matrix4.hpp"
#include <cmath>
#include <iostream>

template <typename T>
struct Vec4 {
    typedef T content4 __attribute__((ext_vector_type(4)));

    union {
        content4 content;
        struct {
            T x, y, z, w;
        };
    };

    Vec4<T>()
        : content{0, 0, 0, 0} {};

    Vec4<T>(content4 newContent)
        : content(newContent) {}

    Vec4<T>(T x, T y, T z, T w)
        : content{x, y, z, w} {}

    inline T len() const {
        return sqrt(len_squared());
    }

    inline T len_squared() const {
        const auto squared = content * content;
        return squared[0] + squared[1] + squared[2] + squared[3];
    }

    inline Vec4<T> normal() const {
        T l = len();
        if (l < 1e-5)
            return {0, 0, 0, 0};
        else
            return {content / l};
    }

    inline void normalize() {
        T l = len();
        if (l < 1e-5)
            content = {0, 0, 0, 0};
        else
            content /= l;
    }

    [[nodiscard]] inline bool is_zero() const {
        const auto squared = (content * content) < 1e-5;
        return squared[0] * squared[1] * squared[2] * squared[3];
    }

    inline T dot(const Vec4<T>& other) const {
        const auto res = (content * other.content);
        return res[0] + res[1] + res[2] + res[3];
    }

    void set(int i, T x) {
        content[i] = x;
    }

    T operator[](const int i) const {
        return content[i];
    }

    inline Vec4<T> operator+() {
        return *this;
    }

    inline Vec4<T> operator-() const {
        return {content * -1};
    }

    inline Vec4<T> operator+(const Vec4<T>& second) const {
        return {content + second.content};
    }

    inline Vec4<T> operator-(const Vec4<T>& second) const {
        return {content - second.content};
    }

    inline Vec4<T> operator*(const Vec4<T>& second) const {
        return {content * second.content};
    }

    inline Vec4<T> operator/(const Vec4<T>& second) const {
        return {content / second.content};
    }

    inline Vec4<T> operator*(const T k) const {
        return Vec4{content * k};
    }

    inline Vec4<T> operator/(T k) const {
        return {content / k};
    }

    inline Vec4<T> operator&(T k) const {
        return {content & k};
    }

    inline Vec4<T> operator|(T k) const {
        return {content | k};
    }

    inline Vec4<T>& operator+=(const Vec4<T>& second) {
        content += second.content;
        return *this;
    }

    inline Vec4<T>& operator-=(const Vec4<T>& second) {
        content -= second.content;
        return *this;
    }

    inline Vec4<T>& operator*=(const Vec4<T>& second) {
        content *= second.content;
        return *this;
    }

    inline Vec4<T>& operator*=(const T k) {
        content *= k;
        return *this;
    }

    inline Vec4<T>& operator/=(const Vec4<T>& second) {
        content /= second.content;
        return *this;
    }

    inline Vec4<T>& operator/=(const T k) {
        content /= k;
        return *this;
    }

    inline Vec4<T>& operator&=(const T k) {
        content &= k;
        return *this;
    }

    inline Vec4<T>& operator|=(const T k) {
        content |= k;
        return *this;
    }

    inline bool operator==(const Vec4<T>& second) const {
        const auto res = content - second.content;
        return (res[0]) < 1e-5 && (res[1]) < 1e-5 && (res[2]) < 1e-5 && res[3] < 1e-5;
    }

    template <typename V>
    inline explicit operator Vec4<V>() const {
        return Vec4<V>{(V)content[0], (V)content[1], (V)content[2], (V)content[3]};
    }
};

namespace std {
template<typename T>
struct hash<Vec4<T>> {
    size_t operator()(const Vec4<T>& pos) const {
        size_t x_hash = hash<T>()(pos.x);
        size_t y_hash = hash<T>()(pos.y);
        size_t z_hash = hash<T>()(pos.z);
        size_t w_hash = hash<T>()(pos.w);
        return x_hash ^ (y_hash << 1) ^ (z_hash << 2) ^ (w_hash << 3);
    }
};
}

typedef Vec4<double> Vec4d;
typedef Vec4<float> Vec4f;
typedef Vec4<int> Vec4i;