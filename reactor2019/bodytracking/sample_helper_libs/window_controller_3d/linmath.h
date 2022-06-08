#ifndef LINMATH_H
#define LINMATH_H

#include <math.h>

#ifdef _MSC_VER
//#define inline __inline
#endif

namespace linmath
{

typedef int ivec2[2];

#define LINMATH_H_DEFINE_VEC(n) \
typedef float vec##n[n]; \
static inline void vec##n##_copy(vec##n r, vec##n const v) \
{ \
    int i; \
    for(i=0; i<n; ++i) \
        r[i] = v[i]; \
} \
static inline void vec##n##_add(vec##n r, vec##n const a, vec##n const b) \
{ \
    int i; \
    for(i=0; i<n; ++i) \
        r[i] = a[i] + b[i]; \
} \
static inline void vec##n##_sub(vec##n r, vec##n const a, vec##n const b) \
{ \
    int i; \
    for(i=0; i<n; ++i) \
        r[i] = a[i] - b[i]; \
} \
static inline void vec##n##_scale(vec##n r, vec##n const v, float const s) \
{ \
    int i; \
    for(i=0; i<n; ++i) \
        r[i] = v[i] * s; \
} \
static inline float vec##n##_mul_inner(vec##n const a, vec##n const b) \
{ \
    float p = 0.; \
    int i; \
    for(i=0; i<n; ++i) \
        p += b[i]*a[i]; \
    return p; \
} \
static inline float vec##n##_len(vec##n const v) \
{ \
    return (float) sqrt(vec##n##_mul_inner(v,v)); \
} \
static inline void vec##n##_norm(vec##n r, vec##n const v) \
{ \
    float k = 1.f / vec##n##_len(v); \
    vec##n##_scale(r, v, k); \
}

LINMATH_H_DEFINE_VEC(2)
LINMATH_H_DEFINE_VEC(3)
LINMATH_H_DEFINE_VEC(4)

static inline void vec3_set(vec3 r, float v0, float v1, float v2)
{
    r[0] = v0;
    r[1] = v1;
    r[2] = v2;
}

static inline void vec3_set(vec3 r0, vec3 const r1)
{
    r0[0] = r1[0];
    r0[1] = r1[1];
    r0[2] = r1[2];
}

static inline void vec3_mul_cross(vec3 r, vec3 const a, vec3 const b)
{
    r[0] = a[1]*b[2] - a[2]*b[1];
    r[1] = a[2]*b[0] - a[0]*b[2];
    r[2] = a[0]*b[1] - a[1]*b[0];
}

static inline void vec3_reflect(vec3 r, vec3 const v, vec3 const n)
{
    float p  = 2.f*vec3_mul_inner(v, n);
    int i;
    for(i=0;i<3;++i)
        r[i] = v[i] - p*n[i];
}

static inline void vec4_set(vec4 r, float v0, float v1, float v2, float v3)
{
    r[0] = v0;
    r[1] = v1;
    r[2] = v2;
    r[3] = v3;
}
static inline void vec4_mul_cross(vec4 r, vec4 a, vec4 b)
{
    r[0] = a[1]*b[2] - a[2]*b[1];
    r[1] = a[2]*b[0] - a[0]*b[2];
    r[2] = a[0]*b[1] - a[1]*b[0];
    r[3] = 1.f;
}

static inline void vec4_reflect(vec4 r, vec4 v, vec4 n)
{
    float p  = 2.f*vec4_mul_inner(v, n);
    int i;
    for(i=0;i<4;++i)
        r[i] = v[i] - p*n[i];
}

typedef vec4 mat4x4[4];
static inline void mat4x4_identity(mat4x4 M)
{
    int i, j;
    for(i=0; i<4; ++i)
        for(j=0; j<4; ++j)
            M[i][j] = i==j ? 1.f : 0.f;
}
static inline void mat4x4_dup(mat4x4 M, mat4x4 N)
{
    int i, j;
    for(i=0; i<4; ++i)
        for(j=0; j<4; ++j)
            M[i][j] = N[i][j];
}
static inline void mat4x4_row(vec4 r, mat4x4 M, int i)
{
    int k;
    for(k=0; k<4; ++k)
        r[k] = M[k][i];
}
static inline void mat4x4_col(vec4 r, mat4x4 M, int i)
{
    int k;
    for(k=0; k<4; ++k)
        r[k] = M[i][k];
}
static inline void mat4x4_transpose(mat4x4 M, mat4x4 N)
{
    int i, j;
    for(j=0; j<4; ++j)
        for(i=0; i<4; ++i)
            M[i][j] = N[j][i];
}
static inline void mat4x4_add(mat4x4 M, mat4x4 a, mat4x4 b)
{
    int i;
    for(i=0; i<4; ++i)
        vec4_add(M[i], a[i], b[i]);
}
static inline void mat4x4_sub(mat4x4 M, mat4x4 a, mat4x4 b)
{
    int i;
    for(i=0; i<4; ++i)
        vec4_sub(M[i], a[i], b[i]);
}
static inline void mat4x4_scale(mat4x4 M, mat4x4 a, float k)
{
    int i;
    for(i=0; i<4; ++i)
        vec4_scale(M[i], a[i], k);
}
static inline void mat4x4_scale_aniso(mat4x4 M, mat4x4 a, float x, float y, float z)
{
    int i;
    vec4_scale(M[0], a[0], x);
    vec4_scale(M[1], a[1], y);
    vec4_scale(M[2], a[2], z);
    for(i = 0; i < 4; ++i) {
        M[3][i] = a[3][i];
    }
}
static inline void mat4x4_mul(mat4x4 M, mat4x4 a, mat4x4 b)
{
    mat4x4 temp;
    int k, r, c;
    for(c=0; c<4; ++c) for(r=0; r<4; ++r) {
        temp[c][r] = 0.f;
        for(k=0; k<4; ++k)
            temp[c][r] += a[k][r] * b[c][k];
    }
    mat4x4_dup(M, temp);
}
static inline void mat4x4_mul_vec4(vec4 r, mat4x4 M, vec4 v)
{
    int i, j;
    for(j=0; j<4; ++j) {
        r[j] = 0.f;
        for(i=0; i<4; ++i)
            r[j] += M[i][j] * v[i];
    }
}
static inline void mat4x4_translate(mat4x4 T, float x, float y, float z)
{
    mat4x4_identity(T);
    T[3][0] = x;
    T[3][1] = y;
    T[3][2] = z;
}
static inline void mat4x4_translate_in_place(mat4x4 M, float x, float y, float z)
{
    vec4 t = {x, y, z, 0};
    vec4 r;
    int i;
    for (i = 0; i < 4; ++i) {
        mat4x4_row(r, M, i);
        M[3][i] += vec4_mul_inner(r, t);
    }
}
static inline void mat4x4_from_vec3_mul_outer(mat4x4 M, vec3 a, vec3 b)
{
    int i, j;
    for(i=0; i<4; ++i) for(j=0; j<4; ++j)
        M[i][j] = i<3 && j<3 ? a[i] * b[j] : 0.f;
}
static inline void mat4x4_rotate(mat4x4 R, mat4x4 M, float x, float y, float z, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);
    vec3 u = {x, y, z};

    if(vec3_len(u) > 1e-4) {
        mat4x4 T, C, S = {{0}};

        vec3_norm(u, u);
        mat4x4_from_vec3_mul_outer(T, u, u);

        S[1][2] =  u[0];
        S[2][1] = -u[0];
        S[2][0] =  u[1];
        S[0][2] = -u[1];
        S[0][1] =  u[2];
        S[1][0] = -u[2];

        mat4x4_scale(S, S, s);

        mat4x4_identity(C);
        mat4x4_sub(C, C, T);

        mat4x4_scale(C, C, c);

        mat4x4_add(T, T, C);
        mat4x4_add(T, T, S);

        T[3][3] = 1.;
        mat4x4_mul(R, M, T);
    } else {
        mat4x4_dup(R, M);
    }
}
static inline void mat4x4_rotate_X(mat4x4 Q, mat4x4 M, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);
    mat4x4 R = {
        {1.f, 0.f, 0.f, 0.f},
        {0.f,   c,   s, 0.f},
        {0.f,  -s,   c, 0.f},
        {0.f, 0.f, 0.f, 1.f}
    };
    mat4x4_mul(Q, M, R);
}
static inline void mat4x4_rotate_Y(mat4x4 Q, mat4x4 M, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);
    mat4x4 R = {
        {   c, 0.f,   s, 0.f},
        { 0.f, 1.f, 0.f, 0.f},
        {  -s, 0.f,   c, 0.f},
        { 0.f, 0.f, 0.f, 1.f}
    };
    mat4x4_mul(Q, M, R);
}
static inline void mat4x4_rotate_Z(mat4x4 Q, mat4x4 M, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);
    mat4x4 R = {
        {   c,   s, 0.f, 0.f},
        {  -s,   c, 0.f, 0.f},
        { 0.f, 0.f, 1.f, 0.f},
        { 0.f, 0.f, 0.f, 1.f}
    };
    mat4x4_mul(Q, M, R);
}
static inline void mat4x4_invert(mat4x4 T, mat4x4 M)
{
    float idet;
    float s[6];
    float c[6];
    s[0] = M[0][0]*M[1][1] - M[1][0]*M[0][1];
    s[1] = M[0][0]*M[1][2] - M[1][0]*M[0][2];
    s[2] = M[0][0]*M[1][3] - M[1][0]*M[0][3];
    s[3] = M[0][1]*M[1][2] - M[1][1]*M[0][2];
    s[4] = M[0][1]*M[1][3] - M[1][1]*M[0][3];
    s[5] = M[0][2]*M[1][3] - M[1][2]*M[0][3];

    c[0] = M[2][0]*M[3][1] - M[3][0]*M[2][1];
    c[1] = M[2][0]*M[3][2] - M[3][0]*M[2][2];
    c[2] = M[2][0]*M[3][3] - M[3][0]*M[2][3];
    c[3] = M[2][1]*M[3][2] - M[3][1]*M[2][2];
    c[4] = M[2][1]*M[3][3] - M[3][1]*M[2][3];
    c[5] = M[2][2]*M[3][3] - M[3][2]*M[2][3];

    /* Assumes it is invertible */
    idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );

    T[0][0] = ( M[1][1] * c[5] - M[1][2] * c[4] + M[1][3] * c[3]) * idet;
    T[0][1] = (-M[0][1] * c[5] + M[0][2] * c[4] - M[0][3] * c[3]) * idet;
    T[0][2] = ( M[3][1] * s[5] - M[3][2] * s[4] + M[3][3] * s[3]) * idet;
    T[0][3] = (-M[2][1] * s[5] + M[2][2] * s[4] - M[2][3] * s[3]) * idet;

    T[1][0] = (-M[1][0] * c[5] + M[1][2] * c[2] - M[1][3] * c[1]) * idet;
    T[1][1] = ( M[0][0] * c[5] - M[0][2] * c[2] + M[0][3] * c[1]) * idet;
    T[1][2] = (-M[3][0] * s[5] + M[3][2] * s[2] - M[3][3] * s[1]) * idet;
    T[1][3] = ( M[2][0] * s[5] - M[2][2] * s[2] + M[2][3] * s[1]) * idet;

    T[2][0] = ( M[1][0] * c[4] - M[1][1] * c[2] + M[1][3] * c[0]) * idet;
    T[2][1] = (-M[0][0] * c[4] + M[0][1] * c[2] - M[0][3] * c[0]) * idet;
    T[2][2] = ( M[3][0] * s[4] - M[3][1] * s[2] + M[3][3] * s[0]) * idet;
    T[2][3] = (-M[2][0] * s[4] + M[2][1] * s[2] - M[2][3] * s[0]) * idet;

    T[3][0] = (-M[1][0] * c[3] + M[1][1] * c[1] - M[1][2] * c[0]) * idet;
    T[3][1] = ( M[0][0] * c[3] - M[0][1] * c[1] + M[0][2] * c[0]) * idet;
    T[3][2] = (-M[3][0] * s[3] + M[3][1] * s[1] - M[3][2] * s[0]) * idet;
    T[3][3] = ( M[2][0] * s[3] - M[2][1] * s[1] + M[2][2] * s[0]) * idet;
}
static inline void mat4x4_orthonormalize(mat4x4 R, mat4x4 M)
{
    float s = 1.;
    vec3 h;

    mat4x4_dup(R, M);
    vec3_norm(R[2], R[2]);

    s = vec3_mul_inner(R[1], R[2]);
    vec3_scale(h, R[2], s);
    vec3_sub(R[1], R[1], h);
    vec3_norm(R[2], R[2]);

    s = vec3_mul_inner(R[1], R[2]);
    vec3_scale(h, R[2], s);
    vec3_sub(R[1], R[1], h);
    vec3_norm(R[1], R[1]);

    s = vec3_mul_inner(R[0], R[1]);
    vec3_scale(h, R[1], s);
    vec3_sub(R[0], R[0], h);
    vec3_norm(R[0], R[0]);
}

static inline void mat4x4_frustum(mat4x4 M, float l, float r, float b, float t, float n, float f)
{
    M[0][0] = 2.f*n/(r-l);
    M[0][1] = M[0][2] = M[0][3] = 0.f;

    M[1][1] = 2.f*n/(t-b);
    M[1][0] = M[1][2] = M[1][3] = 0.f;

    M[2][0] = (r+l)/(r-l);
    M[2][1] = (t+b)/(t-b);
    M[2][2] = -(f+n)/(f-n);
    M[2][3] = -1.f;

    M[3][2] = -2.f*(f*n)/(f-n);
    M[3][0] = M[3][1] = M[3][3] = 0.f;
}
static inline void mat4x4_ortho(mat4x4 M, float l, float r, float b, float t, float n, float f)
{
    M[0][0] = 2.f/(r-l);
    M[0][1] = M[0][2] = M[0][3] = 0.f;

    M[1][1] = 2.f/(t-b);
    M[1][0] = M[1][2] = M[1][3] = 0.f;

    M[2][2] = -2.f/(f-n);
    M[2][0] = M[2][1] = M[2][3] = 0.f;

    M[3][0] = -(r+l)/(r-l);
    M[3][1] = -(t+b)/(t-b);
    M[3][2] = -(f+n)/(f-n);
    M[3][3] = 1.f;
}
static inline void mat4x4_perspective(mat4x4 m, float y_fov, float aspect, float n, float f)
{
    /* NOTE: Degrees are an unhandy unit to work with.
     * linmath.h uses radians for everything! */
    float const a = 1.f / (float) tan(y_fov / 2.f);

    m[0][0] = a / aspect;
    m[0][1] = 0.f;
    m[0][2] = 0.f;
    m[0][3] = 0.f;

    m[1][0] = 0.f;
    m[1][1] = a;
    m[1][2] = 0.f;
    m[1][3] = 0.f;

    m[2][0] = 0.f;
    m[2][1] = 0.f;
    m[2][2] = -((f + n) / (f - n));
    m[2][3] = -1.f;

    m[3][0] = 0.f;
    m[3][1] = 0.f;
    m[3][2] = -((2.f * f * n) / (f - n));
    m[3][3] = 0.f;
}
static inline void mat4x4_look_at(mat4x4 m, vec3 eye, vec3 center, vec3 up)
{
    /* Adapted from Android's OpenGL Matrix.java.                        */
    /* See the OpenGL GLUT documentation for gluLookAt for a description */
    /* of the algorithm. We implement it in a straightforward way:       */

    /* TODO: The negation of of can be spared by swapping the order of
     *       operands in the following cross products in the right way. */
    vec3 f;
    vec3 s;
    vec3 t;

    vec3_sub(f, center, eye);
    vec3_norm(f, f);

    vec3_mul_cross(s, f, up);
    vec3_norm(s, s);

    vec3_mul_cross(t, s, f);

    m[0][0] =  s[0];
    m[0][1] =  t[0];
    m[0][2] = -f[0];
    m[0][3] =   0.f;

    m[1][0] =  s[1];
    m[1][1] =  t[1];
    m[1][2] = -f[1];
    m[1][3] =   0.f;

    m[2][0] =  s[2];
    m[2][1] =  t[2];
    m[2][2] = -f[2];
    m[2][3] =   0.f;

    m[3][0] =  0.f;
    m[3][1] =  0.f;
    m[3][2] =  0.f;
    m[3][3] =  1.f;

    mat4x4_translate_in_place(m, -eye[0], -eye[1], -eye[2]);
}

typedef union
{
    struct _wxyz
    {
        float w;
        float x;
        float y;
        float z;
    } wxyz;
    vec4 v;
} quaternion;

static inline void quaternion_to_mat4x4(mat4x4 rotation, const quaternion q)
{
    float sqw = q.wxyz.w * q.wxyz.w;
    float sqx = q.wxyz.x * q.wxyz.x;
    float sqy = q.wxyz.y * q.wxyz.y;
    float sqz = q.wxyz.z * q.wxyz.z;

    // Assume quaternion is always normalized, no extra normalize is needed.
    float m00 = sqx - sqy - sqz + sqw;
    float m11 = -sqx + sqy - sqz + sqw;
    float m22 = -sqx - sqy + sqz + sqw;

    float qxqy = q.wxyz.x * q.wxyz.y;
    float qzqw = q.wxyz.z * q.wxyz.w;
    float m10 = 2.f * (qxqy + qzqw);
    float m01 = 2.f * (qxqy - qzqw);

    float qxqz = q.wxyz.x * q.wxyz.z;
    float qyqw = q.wxyz.y * q.wxyz.w;
    float m20 = 2.f * (qxqz - qyqw);
    float m02 = 2.f * (qxqz + qyqw);

    float qyqz = q.wxyz.y * q.wxyz.z;
    float qxqw = q.wxyz.x * q.wxyz.w;
    float m21 = 2.f * (qyqz + qxqw);
    float m12 = 2.f * (qyqz - qxqw);

    // Notice The OpenGL is using row vector. The transformation matrix should be transposed
    mat4x4 temp =
    {
        { m00, m10, m20, 0  },
        { m01, m11, m21, 0  },
        { m02, m12, m22, 0  },
        {0   , 0  , 0  , 1.f}
    };

    mat4x4_dup(rotation, temp);
}
}
#endif
