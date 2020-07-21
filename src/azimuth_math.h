#include <math.h>

#define Pi 3.14159265359
#define Pi_f32 ((f32)Pi)

#define MinOf(A, B) (((A) < (B)) ? (A) : (B))
#define MaxOf(A, B) (((A) > (B)) ? (A) : (B))

inline f32 Random01();

inline f32 Clamp(f32 Val, f32 Min, f32 Max);
inline f32 Clamp01(f32 Val);

inline f32 Ceil(f32 Val);
inline s32 CeilAsS32(f32 Val);
inline u32 CeilAsU32(f32 Val);
inline f32 Floor(f32 Val);
inline s32 FloorAsS32(f32 Val);
inline u32 FloorAsU32(f32 Val);
inline f32 Round(f32 Val);
inline s32 RoundAsS32(f32 Val);
inline u32 RoundAsU32(f32 Val);

inline s32 Square(s32 Val);
inline u32 Square(u32 Val);
inline f32 Square(f32 Val);
inline f32 Power(f32 Base, f32 Exp);
inline f32 Exp(f32 Val);
inline f32 Sqrt(f32 Val);
inline f32 Abs(f32 Val);

inline f32 LogBaseE(f32 Val);
inline f32 LogBase10(f32 Val);

inline f32 DegreeToRadian(f32 Degree);
inline f32 RadianToDegree(f32 Radian);

inline f32 Sin(f32 Radians);
inline f32 Asin(f32 Radians);
inline f32 Cos(f32 Radians);
inline f32 Acos(f32 Radians);
inline f32 Tan(f32 Radians);
inline f32 Atan(f32 y, f32 x);

typedef struct {
    union{
        struct{
            f32 x;
            f32 y;
        };
        struct{
            f32 u;
            f32 v;
        };
    };
} vec2;

typedef struct {
    union{
        struct{
            u32 x;
            u32 y;
        };
        struct{
            u32 u;
            u32 v;
        };
    };
} vec2u;

typedef struct {
    union{
        struct{
            s32 x;
            s32 y;
        };
        struct{
            s32 u;
            s32 v;
        };
    };
} vec2i;

inline vec2  Vec2(f32 x, f32 y);
inline vec2u Vec2u(u32 x, u32 y);
inline vec2i Vec2i(s32 x, s32 y);

typedef struct {
    union{
        struct{
            f32 x;
            f32 y;
            f32 z;
        };
        struct{
            vec2 xy;
            f32  z;
        };
        struct{
            f32  x;
            vec2 yz;
        };
        struct{
            f32 r;
            f32 g;
            f32 b;
        };
    };
} vec3;

typedef struct {
    union{
        struct{
            u32 x;
            u32 y;
            u32 z;
        };
        struct{
            vec2u xy;
            u32   z;
        };
        struct{
            u32   x;
            vec2u yz;
        };
        struct{
            u32 r;
            u32 g;
            u32 b;
        };
    };
} vec3u;

typedef struct {
    union{
        struct{
            s32 x;
            s32 y;
            s32 z;
        };
        struct{
            vec2i xy;
            s32   z;
        };
        struct{
            s32   x;
            vec2i yz;
        };
        struct{
            s32 r;
            s32 g;
            s32 b;
        };
    };
} vec3i;

inline vec3 Vec3(f32 x, f32 y, f32 z);
inline vec3 Vec3(vec2 xy, f32 z);
inline vec3 Vec3(f32 x, vec2 yz);
inline vec3u Vec3u(u32 x, u32 y, u32 z);
inline vec3u Vec3u(vec2u xy, u32 z);
inline vec3u Vec3u(u32 x, vec2u yz);
inline vec3i Vec3i(s32 x, s32 y, s32 z);
inline vec3i Vec3i(vec2i xy, s32 z);
inline vec3i Vec3i(s32 x, vec2i yz);

typedef struct {
    union{
        struct{
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
        struct{
            f32 r;
            f32 l;
            f32 b;
            f32 t;
        };
        struct{
            vec2 xy;
            f32 z;
            f32 w;
        };
        struct{
            vec3 xyz;
            f32 w;
        };
        struct {
            f32 x;
            f32 y;
            vec2 zw;
        };
        struct {
            f32 x;
            vec3 yzw;
        };
        struct{
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };
        struct{
            vec3 rgb;
            f32 a;
        };
    };
} vec4;

typedef struct {
    union{
        struct{
            u32 x;
            u32 y;
            u32 z;
            u32 w;
        };
        struct{
            u32 r;
            u32 l;
            u32 b;
            u32 t;
        };
        struct{
            vec2u xy;
            u32 z;
            u32 w;
        };
        struct{
            vec3u xyz;
            u32 w;
        };
        struct {
            u32 x;
            u32 y;
            vec2u zw;
        };
        struct {
            u32 x;
            vec2u yzw;
        };
        struct{
            u32 r;
            u32 g;
            u32 b;
            u32 a;
        };
        struct{
            vec3u rgb;
            u32 a;
        };
    };
} vec4u;

typedef struct {
    union{
        struct{
            s32 x;
            s32 y;
            s32 z;
            s32 w;
        };
        struct{
            s32 r;
            s32 l;
            s32 b;
            s32 t;
        };
        struct{
            vec2i xy;
            s32 z;
            s32 w;
        };
        struct{
            vec3i xyz;
            s32 w;
        };
        struct {
            s32 x;
            s32 y;
            vec2i zw;
        };
        struct {
            s32 x;
            vec2i yzw;
        };
        struct{
            s32 r;
            s32 g;
            s32 b;
            s32 a;
        };
        struct{
            vec3i rgb;
            s32 a;
        };
    };
} vec4i;

inline vec4 Vec4(f32 x, f32 y, f32 z, f32 w);
inline vec4 Vec4(vec2 xy, f32 z, f32 w);
inline vec4 Vec4(f32 x, f32 y, vec2 zw);
inline vec4 Vec4(vec2 xy, vec2 zw);
inline vec4 Vec4(vec3 xyz, f32 w);
inline vec4u Vec4u(u32 x, u32 y, u32 z, u32 w);
inline vec4u Vec4u(vec2u xy, u32 z, u32 w);
inline vec4u Vec4u(u32 x, u32 y, vec2u zw);
inline vec4u Vec4u(vec2u xy, vec2u zw);
inline vec4u Vec4u(vec3u xyz, u32 w);
inline vec4i Vec4i(s32 x, s32 y, s32 z, s32 w);
inline vec4i Vec4i(vec2i xy, s32 z, s32 w);
inline vec4i Vec4i(s32 x, s32 y, vec2i zw);
inline vec4i Vec4i(vec2i xy, vec2i zw);
inline vec4i Vec4i(vec3i xyz, s32 w);

inline vec2 operator+(vec2 A, vec2 B);
inline vec3 operator+(vec3 A, vec3 B);
inline vec4 operator+(vec4 A, vec4 B);
inline vec2 operator+(vec2 A, f32 B);
inline vec3 operator+(vec3 A, f32 B);
inline vec4 operator+(vec4 A, f32 B);
inline vec2 operator+(f32 A, vec2 B);
inline vec3 operator+(f32 A, vec3 B);
inline vec4 operator+(f32 A, vec4 B);
inline vec2 operator+=(vec2& A, vec2 B);
inline vec3 operator+=(vec3& A, vec3 B);
inline vec4 operator+=(vec4& A, vec4 B);
inline vec2 operator+=(vec2& A, f32 B);
inline vec3 operator+=(vec3& A, f32 B);
inline vec4 operator+=(vec4& A, f32 B);
inline vec2 operator-(vec2 A, vec2 B);
inline vec3 operator-(vec3 A, vec3 B);
inline vec4 operator-(vec4 A, vec4 B);
inline vec2 operator-(vec2 A, f32 B);
inline vec3 operator-(vec3 A, f32 B);
inline vec4 operator-(vec4 A, f32 B);
inline vec2 operator-(f32 A, vec2 B);
inline vec3 operator-(f32 A, vec3 B);
inline vec4 operator-(f32 A, vec4 B);
inline vec2 operator-=(vec2& A, vec2 B);
inline vec3 operator-=(vec3& A, vec3 B);
inline vec4 operator-=(vec4& A, vec4 B);
inline vec2 operator-=(vec2& A, f32 B);
inline vec3 operator-=(vec3& A, f32 B);
inline vec4 operator-=(vec4& A, f32 B);
inline vec2 operator*(f32 A, vec2 B);
inline vec3 operator*(f32 A, vec3 B);
inline vec4 operator*(f32 A, vec4 B);
inline vec2 operator*(vec2 A, f32 B);
inline vec3 operator*(vec3 A, f32 B);
inline vec4 operator*(vec4 A, f32 B);
inline vec2 operator*=(vec2& A, f32 B);
inline vec3 operator*=(vec3& A, f32 B);
inline vec4 operator*=(vec4& A, f32 B);
inline vec2 operator/(vec2 A, vec2 B);
inline vec3 operator/(vec3 A, vec3 B);
inline vec4 operator/(vec4 A, vec4 B);
inline vec2 operator-(vec2 &v1);
inline vec3 operator-(vec3 &v1);
inline vec4 operator-(vec4 &v1);

inline vec3u operator+(vec3u A, vec3u B);
inline vec4u operator+(vec4u A, vec4u B);
inline vec2u operator+(vec2u A, u32 B);
inline vec3u operator+(vec3u A, u32 B);
inline vec4u operator+(vec4u A, u32 B);
inline vec2u operator+(u32 A, vec2u B);
inline vec3u operator+(u32 A, vec3u B);
inline vec4u operator+(u32 A, vec4u B);
inline vec2u operator+=(vec2u& A, vec2u B);
inline vec3u operator+=(vec3u& A, vec3u B);
inline vec4u operator+=(vec4u& A, vec4u B);
inline vec2u operator+=(vec2u& A, u32 B);
inline vec3u operator+=(vec3u& A, u32 B);
inline vec4u operator+=(vec4u& A, u32 B);

inline vec2 Hadamard(vec2 A, vec2 B);
inline vec3 Hadamard(vec3 A, vec3 B);
inline vec4 Hadamard(vec4 A, vec4 B);

inline f32 Length(vec2 V);
inline vec2 Normalize(vec2 V);
inline f32 Length(vec3 V);
inline vec3 Normalize(vec3 V);
inline f32 Dot(vec3 A, vec3 B);
inline vec3 Cross(vec3 A, vec3 B);
inline vec3 Reflect(vec3 V, vec3 R);
inline vec3 RotateAround(vec3 V, f32 AngleDegrees, vec3 Axis);

typedef struct {
    union{
        // NOTE(furkan) : 
        //      | E[0][0] E[0][1] E[0][2] E[0][3] |
        //      | E[1][0] E[1][1] E[1][2] E[1][3] |
        //      | E[2][0] E[2][1] E[2][2] E[2][3] |
        //      | E[3][0] E[3][1] E[3][2] E[3][3] |

        f32 E[4][4];
        f32 Ptr[16];
    };
} mat4;

inline mat4 operator*(mat4 A, mat4 B);
inline vec4 operator*(mat4 A, vec4 B);

inline mat4 Mat4Identity();
inline mat4 Mat4Transpose(mat4 Mat);

inline mat4 Mat4Translate(mat4 Mat, vec3 Translation);
inline mat4 Mat4RotateAround(mat4 Mat, f32 Angle, vec3 RotateAxis);
inline mat4 Mat4Scale(mat4 Mat, vec3 Scale);

inline mat4 Mat4Orthographic(f32 Left, f32 Bottom, 
                             f32 Right, f32 Top, 
                             f32 NearZ, f32 FarZ);
inline mat4 Mat4Perspective(f32 FOVAngle, 
                            f32 Width, f32 Height, 
                            f32 NearZ, f32 FarZ);

inline vec3 PointToWorld(vec3 V, mat4 TransformMat);
inline vec3 VectorToWorld(vec3 V, mat4 TransformMat);
inline vec3 NormalToWorld(vec3 V, mat4 InverseTransformMat);

typedef struct {
    vec3 Min;
    vec3 Max;
    vec3 Center;
} aabb;

inline f32 Luminance(f32 R, f32 G, f32 B);
inline f32 Luminance(vec3 Color);
