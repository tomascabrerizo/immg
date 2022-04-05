#ifndef TC_MATH_H
#define TC_MATH_H

#include "imm_core.h"

// TODO: implement v2 operations and functions
struct v2
{
    f32 x, y;
};

struct v3
{
    f32 x, y, z;
};

struct v4
{
    f32 x, y, z, w;
};

struct m4
{
    f32 m[4][4];
};

struct rect2d
{
    v2 min;
    v2 max;
};

//
// vector2 functions
//

inline v2 _v2(f32 x, f32 y)
{
    v2 result = {x, y};
    return result;
}

inline v2 operator+(v2 a, v2 b)
{
    v2 result = {a.x + b.x, a.y + b.y};
    return result;
}

inline v2 operator-(v2 a, v2 b)
{
    v2 result = {a.x - b.x, a.y - b.y};
    return result;
}

inline v2 operator*(v2 a, f32 v)
{
    v2 result = {a.x * v, a.y * v};
    return result;
}

inline v2 v2_lerp(v2 a, v2 b, f32 t)
{
    v2 result = a * (1-t) + b * t;
    return result;
}

//
// vector3 functions
//

inline v3 _v3(f32 x, f32 y, f32 z)
{
    v3 result = {x, y, z};
    return result;
}

inline v3 operator+(v3 a, v3 b)
{
    v3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}

inline v3 operator-(v3 a, v3 b)
{
    v3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}

inline v3 operator-(v3 v)
{
    v3 result = {-v.x, -v.y, -v.z};
    return result;
}

inline v3 operator*(v3 a, f32 b)
{
    v3 result = {a.x * b, a.y * b, a.z * b};
    return result;
}

inline v3 operator*(f32 b, v3 a)
{
    v3 result = {a.x * b, a.y * b, a.z * b};
    return result;
}

inline v3 operator/(v3 a, f32 b)
{
    v3 result = {a.x / b, a.y / b, a.z / b};
    return result;
}

inline f32 v3_dot(v3 a, v3 b)
{
    f32 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    return result;
}

inline v3 v3_cross(v3 a, v3 b)
{
    v3 result = {};
    result.x = a.y * b.z - a.z * b.y; 
    result.y = a.z * b.x - a.x * b.z; 
    result.z = a.x * b.y - a.y * b.x; 
    return result;
}

inline f32 v3_length_sqr(v3 a)
{
    f32 result = (a.x * a.x) + (a.y * a.y) + (a.z * a.z);
    return result;
}

inline f32 v3_length(v3 a)
{
    f32 result = f32_sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
    return result;
}

inline v3 v3_normalize(v3 v)
{
    v3 result = v / v3_length(v);
    return result;
}

inline v3 v3_lerp(v3 a, v3 b, f32 t)
{
    v3 result = a * (1-t) + b * t; 
    return result;
}

inline v3 operator*(m4 b, v3 a)
{
    v3 result = {};
    result.x = a.x * b.m[0][0] + a.y * b.m[0][1] + a.z * b.m[0][2] + b.m[0][3];
    result.y = a.x * b.m[1][0] + a.y * b.m[1][1] + a.z * b.m[1][2] + b.m[1][3];
    result.z = a.x * b.m[2][0] + a.y * b.m[2][1] + a.z * b.m[2][2] + b.m[2][3];
    f32 w = a.x * b.m[3][0] + a.y * b.m[3][1] + a.z * b.m[3][2] + b.m[3][3];
    if(w && (w != 1.0f))
    {
        result.x /= w;
        result.y /= w;
        result.z /= w;
    }
    return result;
}

//
// vector4 functions
//

#define V4_AXIS(v, i) ((f32 *)&(v))[i]

inline v4 _v4(f32 x, f32 y, f32 z, f32 w)
{
    v4 result = {x, y, z, w};
    return result;
}

inline v4 to_v4(v3 v)
{
    v4 result = {v.x, v.y, v.z, 1.0f};
    return result;
}

inline v4 operator+(v4 a, v4 b)
{
    v4 result = {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    return result;
}

inline v4 operator-(v4 a, v4 b)
{
    v4 result = {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    return result;
}

inline v4 operator*(v4 a, f32 b)
{
    v4 result = {a.x * b, a.y * b, a.z * b, a.w * b};
    return result;
}

inline v4 operator*(f32 b, v4 a)
{
    v4 result = {a.x * b, a.y * b, a.z * b, a.w * b};
    return result;
}

inline v4 operator/(v4 a, f32 b)
{
    v4 result = {a.x / b, a.y / b, a.z / b, a.w / b};
    return result;
}

inline v4 operator*(m4 b, v4 a)
{
    v4 result = {};
    result.x = a.x * b.m[0][0] + a.y * b.m[0][1] + a.z * b.m[0][2] + a.w * b.m[0][3];
    result.y = a.x * b.m[1][0] + a.y * b.m[1][1] + a.z * b.m[1][2] + a.w * b.m[1][3];
    result.z = a.x * b.m[2][0] + a.y * b.m[2][1] + a.z * b.m[2][2] + a.w * b.m[2][3];
    result.w = a.x * b.m[3][0] + a.y * b.m[3][1] + a.z * b.m[3][2] + a.w * b.m[3][3];
    return result;
}

inline v4 v4_lerp(v4 a, v4 b, f32 t)
{
    v4 result = a * (1 - t) + b * t;
    return result;
}

inline f32 v4_dot(v4 a, v4 b)
{
    f32 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
    return result;
}

inline f32 v4_length_sqr(v4 a)
{
    f32 result = (a.x * a.x) + (a.y * a.y) + (a.z * a.z) + (a.w * a.w);
    return result;
}

inline f32 v4_length(v4 a)
{
    f32 result = f32_sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z) + (a.w * a.w));
    return result;
}

//
// matrix4 functions
//
// NOTE: all matrix function are column major
// NOTE: angles must be in radians

inline m4 operator*(m4 a, m4 b)
{
    m4 result = {};
    for(u32 j = 0; j < 4; ++j)
    {
        for(u32 i = 0; i < 4; ++i)
        {
            result.m[j][i] = a.m[j][0] * b.m[0][i] +
                             a.m[j][1] * b.m[1][i] +
                             a.m[j][2] * b.m[2][i] +
                             a.m[j][3] * b.m[3][i];
        }
    }
    return result;
}

inline m4 m4_identity()
{
    m4 result = {};
    result.m[0][0] = 1.0f;
    result.m[1][1] = 1.0f;
    result.m[2][2] = 1.0f;
    result.m[3][3] = 1.0f;
    return result;
}

inline m4 m4_translate(v3 t)
{
    m4 result = {};
    result.m[0][3] = t.x;
    result.m[1][3] = t.y;
    result.m[2][3] = t.z;
    result.m[0][0] = 1.0f;
    result.m[1][1] = 1.0f;
    result.m[2][2] = 1.0f;
    result.m[3][3] = 1.0f;
    return result;
}

inline m4 m4_scale(v3 s)
{
    m4 result = {};
    result.m[0][0] = s.x;
    result.m[1][1] = s.y;
    result.m[2][2] = s.z;
    result.m[3][3] = 1.0f;
    return result;
}

inline m4 m4_rotate_x(f32 a)
{
    f32 ca = f32_cos(a);
    f32 sa = f32_sin(a);
    m4 result = {};
    result.m[0][0] = 1.0f;
    result.m[1][1] = ca;
    result.m[1][2] = -sa;
    result.m[2][1] = sa;
    result.m[2][2] = ca;
    result.m[3][3] = 1.0f;
    return result;
}

inline m4 m4_rotate_y(f32 a)
{
    f32 ca = f32_cos(a);
    f32 sa = f32_sin(a);
    m4 result = {};
    result.m[0][0] = ca;
    result.m[1][1] = 1.0f;
    result.m[0][2] = sa;
    result.m[2][0] = -sa;
    result.m[2][2] = ca;
    result.m[3][3] = 1.0f;
    return result;
}

inline m4 m4_rotate_z(f32 a)
{
    f32 ca = f32_cos(a);
    f32 sa = f32_sin(a);
    m4 result = {};
    result.m[0][0] = ca;
    result.m[0][1] = -sa;
    result.m[1][0] = sa;
    result.m[1][1] = ca;
    result.m[2][2] = 1.0f;
    result.m[3][3] = 1.0f;
    return result;
}

inline m4 m4_perspective(f32 fov, f32 aspect, f32 f, f32 n)
{
    f32 c = 1.0f / f32_tan(fov/2);
    f32 fmn = f - n;
    f32 fpn = f + n;
    m4 result = {};
    result.m[0][0] = c / aspect;
    result.m[1][1] = c;
    result.m[2][2] = -(fpn / fmn);
    result.m[2][3] = -((2*n*f) / fmn);
    result.m[3][2] = -1;
    return result;
}

inline m4 m4_perspective(f32 r, f32 l, f32 t, f32 b, f32 f, f32 n)
{
    f32 rml = r - l;
    f32 tmb = t - b;
    f32 fmn = f - n;
    f32 rpl = r + l;
    f32 tpb = t + b;
    f32 fpn = f + n;
    m4 result = {};
    result.m[0][0] = (2*n) / rml;
    result.m[0][2] = rpl / rml;
    result.m[1][1] = (2*n) / tmb;
    result.m[1][2] = tpb / tmb;
    result.m[2][2] = fpn / fmn;
    result.m[2][3] = (2*f*n) / fmn;
    result.m[3][2] = -1;
    return result;
}

inline m4 m4_ortho(f32 l, f32 r, f32 t, f32 b, f32 n, f32 f)
{
    f32 rml = r - l;
    f32 tmb = t - b;
    f32 fmn = f - n;
    f32 rpl = r + l;
    f32 tpb = t + b;
    f32 fpn = f + n;
    m4 result = {};
    result.m[0][0] = 2 / rml;
    result.m[0][3] = -(rpl / rml);
    result.m[1][1] = 2 / tmb;
    result.m[1][3] = -(tpb / tmb);
    result.m[2][2] = 2 / fmn;
    result.m[2][3] = -(fpn / fmn);
    result.m[3][3] = 1.0f;
    return result;
}

inline m4 m4_look_at(v3 r, v3 u, v3 f, v3 t)
{
    m4 result = {};
    result.m[0][0] = r.x;
    result.m[0][1] = r.y;
    result.m[0][2] = r.z;
    result.m[1][0] = u.x;
    result.m[1][1] = u.y;
    result.m[1][2] = u.z;
    result.m[2][0] = f.x;
    result.m[2][1] = f.y;
    result.m[2][2] = f.z;

    result.m[0][3] = v3_dot(-t, r);
    result.m[1][3] = v3_dot(-t, u);
    result.m[2][3] = v3_dot(-t, f);
    
    result.m[3][3] = 1.0f;

    return result;
}

//
// rect 2d functions
// rect2d dont include max x, and max y

inline rect2d rect2d_min_max(v2 min, v2 max)
{
    rect2d result = {};
    result.min = min;
    result.max = max;
    return result;
}

inline rect2d rect2d_min_dim(v2 min, v2 dim)
{
    rect2d result = {};
    result.min = min;
    result.max = min + dim;
    return result;
}

inline rect2d rect2d_intersection(rect2d r0, rect2d r1)
{
    rect2d result = {};
    result.min.x = r0.min.x > r1.min.x ? r0.min.x : r1.min.x;
    result.min.y = r0.min.y > r1.min.y ? r0.min.y : r1.min.y;
    result.max.x = r0.max.x < r1.max.x ? r0.max.x : r1.max.x;
    result.max.y = r0.max.y < r1.max.y ? r0.max.y : r1.max.y;
    return result;
}

inline rect2d rect2d_union(rect2d r0, rect2d r1)
{
    rect2d result = {};
    result.min.x = r0.min.x < r1.min.x ? r0.min.x : r1.min.x;
    result.min.y = r0.min.y < r1.min.y ? r0.min.y : r1.min.y;
    result.max.x = r0.max.x > r1.max.x ? r0.max.x : r1.max.x;
    result.max.y = r0.max.y > r1.max.y ? r0.max.y : r1.max.y;
    return result;
}

#endif // TC_MATH_H
