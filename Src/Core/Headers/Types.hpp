#pragma once

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace AnA
{
    struct Transform
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};

        glm::mat3 mat3() const
        {
            const float c3 = cosf(rotation.z);
            const float s3 = sinf(rotation.z);
            const float c2 = cosf(rotation.x);
            const float s2 = sinf(rotation.x);
            const float c1 = cosf(rotation.y);
            const float s1 = sinf(rotation.y);
            return glm::mat3
            {
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2),
                }
            };
        }
        glm::mat4 mat4() const
        {
            const float c3 = cosf(rotation.z);
            const float s3 = sinf(rotation.z);
            const float c2 = cosf(rotation.x);
            const float s2 = sinf(rotation.x);
            const float c1 = cosf(rotation.y);
            const float s1 = sinf(rotation.y);
            return glm::mat4
            {
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2),
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}
            };
        }
    };
    typedef glm::vec<2, uint32_t> Range;
    template<typename T>
    struct Var
    {
        T value;
        Var() : value(0)
        {

        }
        Var(T v) : value(v)
        {

        }

        Var(T& v) : value(v)
        {

        }

        operator T&()
        {
            return value;
        }

        operator const T&() const
        {
            return value;
        }

        operator T() const
        {
            return value;
        }
        Var& operator=(T v)
        {
            value = v;
            return *this;
        }
        T* operator&() { return &value; };
        const T* operator&() const { return &value; };

        template<typename U>
        U As() const
        {
            return static_cast<U>(value);
        }

        Var operator+(const Var& var) const { return Var(value + var.value); }
        Var operator-(const Var& var) const { return Var(value - var.value); }
        Var operator*(const Var& var) const { return Var(value * var.value); }
        Var operator/(const Var& var) const { return Var(value / var.value); }
        Var operator+(const T& v) const { return Var(value + v); }
        Var operator-(const T& v) const { return Var(value - v); }
        Var operator*(const T& v) const { return Var(value * v); }
        Var operator/(const T& v) const { return Var(value / v); }

        Var& operator+=(const Var& var) { value += var.value; return *this; }
        Var& operator-=(const Var& var)  { value -= var.value; return *this; }
        Var& operator*=(const Var& var)  { value *= var.value; return *this; }
        Var& operator/=(const Var& var)  { value /= var.value; return *this; }
        Var& operator+=(const T& v) { value += v; return *this; }
        Var& operator-=(const T& v) { value -= v; return *this; }
        Var& operator*=(const T& v) { value *= v; return *this; }
        Var& operator/=(const T& v) { value /= v; return *this; }


        bool operator==(const Var& var) const { return value == var.value; }
        bool operator!=(const Var& var) const { return value != var.value; }
        bool operator<(const Var& var) const { return value < var.value; }
        bool operator>(const Var& var) const { return value > var.value; }
        bool operator<=(const Var& var) const { return value <= var.value; }
        bool operator>=(const Var& var) const { return value >= var.value; }
        bool operator==(const T& v) const { return value == v; }
        bool operator!=(const T& v) const { return value != v; }
        bool operator<(const T& v) const { return value < v; }
        bool operator>(const T& v) const { return value > v; }
        bool operator<=(const T& v) const { return value <= v; }
        bool operator>=(const T& v) const { return value >= v; }

    };
    typedef Var<int8_t> Int8;
    typedef Var<uint8_t> UInt8;
    typedef Var<int16_t> Int16;
    typedef Var<uint16_t> Uint16;
    typedef Var<int32_t> Int32;
    typedef Var<uint32_t> UInt32;
    typedef Var<size_t> SizeT;
    typedef Var<float> Float;
    typedef Var<double> Double;

    struct CursorPosition
    {
        Double x;
        Double y;
    };

    template<typename T, size_t N>
    struct Vec
    {
        T data[N];

        Vec() = default;

        template<typename... Args>
        Vec(Args... args) : data{{static_cast<T>(args)...}}
        {
            static_assert(sizeof...(args) == N, "Incorrect number of components");
        }

        T& operator[](size_t index) { return data[index]; }
        const T& operator[](size_t index) const { return data[index]; }

        Vec operator+(const Vec& other) const
        {
            Vec result;
            for (size_t i = 0; i < N; ++i)
                result[i] = data[i] + other[i];
            return result;
        }

        Vec operator-(const Vec& other) const
        {
            Vec result;
            for (size_t i = 0; i < N; ++i)
                result[i] = data[i] - other[i];
            return result;
        }

        Vec operator*(T scalar) const
        {
            Vec result;
            for (size_t i = 0; i < N; ++i)
                result[i] = data[i] * scalar;
            return result;
        }

        Vec operator/(T scalar) const
        {
            if (scalar == T(0))
                return Vec{};

            Vec result;
            for (size_t i = 0; i < N; ++i)
                result[i] = data[i] / scalar;
            return result;
        }

        Vec& operator+=(const Vec& other)
        {
            for (size_t i = 0; i < N; ++i)
                data[i] = data[i] + other[i];
            return data;
        }

        Vec& operator-=(const Vec& other)
        {
            for (size_t i = 0; i < N; ++i)
                data[i] = data[i] - other[i];
            return *this;
        }

        Vec& operator*=(T scalar)
        {
            for (size_t i = 0; i < N; ++i)
                data[i] = data[i] * scalar;
            return *this;
        }

        Vec& operator/=(T scalar)
        {
            if (scalar == T(0))
            {
                data = Vec{};
                return *this;
            }

            for (size_t i = 0; i < N; ++i)
                data[i] = data[i] / scalar;
            return *this;
        }

        T Length() const
        {
            T sum = T(0);
            for (size_t i = 0; i < N; ++i)
                sum += data[i] * data[i];
            return std::sqrt(sum);
        }

        Vec Normalized() const
        {
            T len = Length();
            if (len == T(0))
                return Vec{};
            return *this / len;
        }
    };

    template<typename T>
    struct Vec<T, 2>
    {
        T data[2];
        Vec()
        {

        }
        Vec(T _x, T _y) : data{_x, _y}
        {

        }

        T& x() { return data[0]; }
        T& y() { return data[1]; }

        const T& x() const { return data[0]; }
        const T& y() const { return data[1]; }

        T& operator[](size_t i) { return data[i]; }
        const T& operator[](size_t i) const { return data[i]; }

        Vec operator+(const Vec& other) const
        {
            return Vec{data[0] + other.data[0], data[1] + other.data[1]};
        }
        Vec operator-(const Vec& other) const
        {
            return Vec{data[0] - other.data[0], data[1] - other.data[1]};
        }
        Vec operator*(const Vec& other) const
        {
            return Vec{data[0] * other.data[0], data[1] * other.data[1]};
        }
        Vec operator/(const Vec& other) const
        {
            return Vec{other.data[0] ? data[0] / other.data[0] : 0, other.data[1] ? data[1] / other.data[1] : 0};
        }
        Vec operator+(const T& scalar) const
        {
            return Vec{data[0] + scalar, data[1] + scalar};
        }
        Vec operator-(const T& scalar) const
        {
            return Vec{data[0] - scalar, data[1] - scalar};
        }
        Vec operator*(const T& scalar) const
        {
            return Vec{data[0] * scalar, data[1] * scalar};
        }
        Vec operator/(const T& scalar) const
        {
            return Vec{data[0] / scalar, data[1] / scalar};
        }

        Vec& operator+=(const Vec& other)
        {
            data[0] += other.data[0];
            data[1] += other.data[1];
            return *this;
        }
        Vec& operator-=(const Vec& other)
        {
            data[0] -= other.data[0];
            data[1] -= other.data[1];
            return *this;
        }
        Vec& operator*=(const Vec& other)
        {
            data[0] *= other.data[0];
            data[1] *= other.data[1];
            return *this;
        }
        Vec& operator/=(const Vec& other)
        {
            data[0] /= other.data[0];
            data[1] /= other.data[1];
            return *this;
        }
    };

    template<typename T>
    struct Vec<T, 3>
    {
        T data[3];

        T& x() { return data[0]; }
        T& y() { return data[1]; }
        T& z() { return data[2]; }

        const T& x() const { return data[0]; }
        const T& y() const { return data[1]; }
        const T& z() const { return data[2]; }

        T& operator[](size_t i) { return data[i]; }
        const T& operator[](size_t i) const { return data[i]; }

    };
    template<typename T>
    struct Vec<T, 4>
    {
        T data[4];

        T& x() { return data[0]; }
        T& y() { return data[1]; }
        T& z() { return data[2]; }
        T& w() { return data[3]; }

        const T& x() const { return data[0]; }
        const T& y() const { return data[1]; }
        const T& z() const { return data[2]; }
        const T& w() const { return data[2]; }

        T& operator[](size_t i) { return data[i]; }
        const T& operator[](size_t i) const { return data[i]; }

    };
    typedef Vec<Float, 2> Vec2;
    typedef Vec<Float, 3> Vec3;
    typedef Vec<Float, 4> Vec4;
}
