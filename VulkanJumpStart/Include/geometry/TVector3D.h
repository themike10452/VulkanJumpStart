#pragma once

template <typename T, size_t size = 2>
union TVector2D
{
public:
    struct { T x, y; };
    struct { T a, b; };

    TVector2D( T xValue = 0, T yValue = 0, T zValue = 0 )
    {
        x = xValue , y = yValue;
    }

    T& operator []( int i )
    {
        static_assert(i >= 0 && i < size, "");
        return mData[i];
    }

private:
    T mData[size];
};

template <typename T, size_t size = 3>
union TVector3D
{
public:
    struct { T x, y, z; };
    struct { T a, b, c; };

    TVector3D( T xValue = 0, T yValue = 0, T zValue = 0 )
    {
        x = xValue , y = yValue , z = zValue;
    }

    T& operator []( int i )
    {
        static_assert(i >= 0 && i < size, "");
        return mData[i];
    }

private:
    T mData[size];
};

typedef TVector2D<int>    Vector2DI;
typedef TVector2D<float>  Vector2DF;

typedef TVector3D<int>    Vector3DI;
typedef TVector3D<float>  Vector3DF;