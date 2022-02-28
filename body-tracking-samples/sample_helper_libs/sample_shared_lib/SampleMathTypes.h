// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <k4a/k4atypes.h>
#include <cmath>

namespace Samples
{
    struct Vector
    {
        float X;
        float Y;
        float Z;

        Vector(float x, float y, float z)
            : X(x)
            , Y(y)
            , Z(z)
        {
        }

        Vector(const k4a_float3_t& v)
            : X(v.xyz.x)
            , Y(v.xyz.y)
            , Z(v.xyz.z)
        {
        }

        float Dot(const Vector& other) const
        {
            return X * other.X + Y * other.Y + Z * other.Z;
        }

        float SquareLength() const
        {
            return X * X + Y * Y + Z * Z;
        }

        float Length() const
        {
            return std::sqrt(SquareLength());
        }

        Vector operator*(float c) const
        {
            return { X * c, Y * c, Z * c };
        }

        Vector operator/(float c) const
        {
            return *this * (1 / c);
        }

        Vector Normalized() const
        {
            return *this / Length();
        }

        float Angle(const Vector& other) const
        {
            return std::acos(Dot(other) / Length() / other.Length());
        }
    };

    inline Vector operator-(const Vector& v1, const Vector& v2)
    {
        return { v1.X - v2.X, v1.Y - v2.Y, v1.Z - v2.Z };
    }

    inline Vector operator+(const Vector& v1, const Vector& v2)
    {
        return { v1.X + v2.X, v1.Y + v2.Y, v1.Z + v2.Z };
    }

    inline Vector operator*(float c, const Vector& v)
    {
        return { v.X * c, v.Y * c, v.Z * c };
    }

    inline Vector operator*(const Vector& v1, const Vector& v2)
    {
        return { v1.Y * v2.Z - v1.Z * v2.Y, v1.Z * v2.X - v1.X * v2.Z, v1.X * v2.Y - v1.Y * v2.X };
    }

    struct Plane
    {
        using Point = Vector;

        Vector Normal;
        Point Origin;
        float C;

        static Plane Create(Vector n, Point p)
        {
            float c = n.X * p.X + n.Y * p.Y + n.Z * p.Z;
            return { n, p, -c };
        }

        static Plane Create(const Point& p1, const Point& p2, const Point& p3)
        {
            Vector v1 = p2 - p1;
            Vector v2 = p2 - p3;
            Vector n = v1 * v2;
            return Create(n, p1);
        }

        Vector ProjectVector(const Vector& v) const
        {
            return v - Normal * (v.Dot(Normal) / Normal.SquareLength());
        }

        Vector ProjectPoint(const Point& p) const
        {
            return Origin + ProjectVector(p - Origin);
        }

        float AbsDistance(const Point& p) const
        {
            return std::abs(p.X * Normal.X + p.Y * Normal.Y + p.Z * Normal.Z + C) / Normal.Length();
        }
    };
}