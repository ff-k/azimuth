inline f32
Random01(){
    f32 Result;

    Result = ((f32)rand())/RAND_MAX;

    return Result;
}

inline f32 
Clamp(f32 Val, f32 Min, f32 Max){
    f32 Result;

    Result = MaxOf(Min, MinOf(Val, Max));

    return Result;
}

inline f32 
Clamp01(f32 Val){
    f32 Result;

    Result = Clamp(Val, 0.0f, 1.0f);

    return Result;
}

inline f32
Ceil(f32 Val){
    f32 Result;

    Result = ceilf(Val);

    return Result;
}

inline s32
CeilAsS32(f32 Val){
    s32 Result;

    Result = (s32)ceilf(Val);

    return Result;
}

inline u32
CeilAsU32(f32 Val){
    u32 Result;

    Result = (u32)ceilf(Val);

    return Result;
}

inline f32
Floor(f32 Val){
    f32 Result;

    Result = floorf(Val);

    return Result;
}

inline s32
FloorAsS32(f32 Val){
    s32 Result;

    Result = (s32)floorf(Val);

    return Result;
}

inline u32
FloorAsU32(f32 Val){
    u32 Result;

    Result = (u32)floorf(Val);

    return Result;
}

inline f32
Round(f32 Val){
    f32 Result;

    Result = floorf(Val+0.5f);

    return Result;
}

inline s32 
RoundAsS32(f32 Val){
    s32 Result;

    Result = (s32)(Val+0.5f);

    return Result;
}

inline u32 
RoundAsU32(f32 Val){
    u32 Result;

    Result = (u32)(Val+0.5f);

    return Result;
}

inline s32
Square(s32 Val){
    s32 Result;
    
    Result = Val*Val;

    return Result;
}

inline u32
Square(u32 Val){
    u32 Result;
    
    Result = Val*Val;

    return Result;
}

inline f32
Square(f32 Val){
    f32 Result;
    
    Result = Val*Val;

    return Result;
}

inline f32
Power(f32 Base, f32 Exp){
    f32 Result;

    Result = powf(Base, Exp);
    
    return Result;
}

inline f32 
Exp(f32 Val){
    f32 Result;

    Result = exp(Val);

    return Result;
}

inline f32
Sqrt(f32 Val){
    f32 Result;

    Result = sqrtf(Val);

    return Result;
}

inline 
f32 Abs(f32 Val){
    f32 Result;

    Result = fabs(Val);

    return Result;
}

inline f32 
LogBaseE(f32 Val){
    f32 Result;

    Result = log(Val);

    return Result;
}

inline f32 
LogBase10(f32 Val){
    f32 Result;

    Result = log10(Val);

    return Result;
}

inline f32
DegreeToRadian(f32 Degree){
    f32 Result;

    Result = (Degree/180.0f)*((f32)Pi);

    return Result;
}

inline f32 
RadianToDegree(f32 Radian){
    f32 Result;

    Result = (Radian*180.0f)/((f32)Pi);

    return Result;
}

inline f32
Sin(f32 Radians){
    f32 Result;

    Result = sinf(Radians);

    return Result;
}

inline f32 
Asin(f32 Radians){
    f32 Result;

    Result = asin(Radians);

    return Result;
}

inline f32
Cos(f32 Radians){
    f32 Result;

    Result = cosf(Radians);

    return Result;
}

inline f32 
Acos(f32 Radians){
    f32 Result;

    Result = acos(Radians);

    return Result;
}

inline f32
Tan(f32 Radians){
    f32 Result;

    Result = tanf(Radians);

    return Result;
}

inline f32
Atan(f32 y, f32 x){
    f32 Result;

    Result = atan2f(y, x);

    return Result;
}

inline vec2
Vec2(f32 x, f32 y){
    vec2 Result;
    
    Result.x = x;
    Result.y = y;
    
    return Result;
}

inline vec2u
Vec2u(u32 x, u32 y){
    vec2u Result;
    
    Result.x = x;
    Result.y = y;
    
    return Result;
}

inline vec2i
Vec2i(s32 x, s32 y){
    vec2i Result;
    
    Result.x = x;
    Result.y = y;
    
    return Result;
}

inline vec3 
Vec3(f32 x, f32 y, f32 z){
    vec3 Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    
    return Result;
}

inline vec3 
Vec3(vec2 xy, f32 z){
    vec3 Result;
    
    Result.x = xy.x;
    Result.y = xy.y;
    Result.z = z;
    
    return Result;
}

inline vec3 
Vec3(f32 x, vec2 yz){
    vec3 Result;
    
    Result.x = x;
    Result.y = yz.x;
    Result.z = yz.y;
    
    return Result;
}

inline vec3u 
Vec3u(u32 x, u32 y, u32 z){
    vec3u Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    
    return Result;
}

inline vec3u 
Vec3u(vec2u xy, u32 z){
    vec3u Result;
    
    Result.x = xy.x;
    Result.y = xy.y;
    Result.z = z;
    
    return Result;
}

inline vec3u 
Vec3u(u32 x, vec2u yz){
    vec3u Result;
    
    Result.x = x;
    Result.y = yz.x;
    Result.z = yz.y;
    
    return Result;
}

inline vec3i 
Vec3i(s32 x, s32 y, s32 z){
    vec3i Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    
    return Result;
}

inline vec3i 
Vec3i(vec2i xy, s32 z){
    vec3i Result;
    
    Result.x = xy.x;
    Result.y = xy.y;
    Result.z = z;
    
    return Result;
}

inline vec3i 
Vec3i(s32 x, vec2i yz){
    vec3i Result;
    
    Result.x = x;
    Result.y = yz.x;
    Result.z = yz.y;
    
    return Result;
}

inline vec4 
Vec4(f32 x, f32 y, f32 z, f32 w){
    vec4 Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    
    return Result;
}

inline vec4 
Vec4(vec2 xy, f32 z, f32 w){
    vec4 Result;
    
    Result.x = xy.x;
    Result.y = xy.y;
    Result.z = z;
    Result.w = w;
    
    return Result;
}

inline vec4 
Vec4(f32 x, f32 y, vec2 zw){
    vec4 Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = zw.x;
    Result.w = zw.y;
    
    return Result;
}

inline vec4 
Vec4(vec2 xy, vec2 zw){
    vec4 Result;
    
    Result.x = xy.x;
    Result.y = xy.y;
    Result.z = zw.x;
    Result.w = zw.y;
    
    return Result;
}

inline vec4 
Vec4(vec3 xyz, f32 w){
    vec4 Result;
    
    Result.x = xyz.x;
    Result.y = xyz.y;
    Result.z = xyz.z;
    Result.w = w;
    
    return Result;
}

inline vec4u 
Vec4u(u32 x, u32 y, u32 z, u32 w){
    vec4u Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    
    return Result;
}

inline vec4u 
Vec4u(vec2u xy, u32 z, u32 w){
    vec4u Result;
    
    Result.x = xy.x;
    Result.y = xy.y;
    Result.z = z;
    Result.w = w;
    
    return Result;
}

inline vec4u
Vec4u(u32 x, u32 y, vec2u zw){
    vec4u Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = zw.x;
    Result.w = zw.y;
    
    return Result;
}

inline vec4u 
Vec4u(vec2u xy, vec2u zw){
    vec4u Result;
    
    Result.x = xy.x;
    Result.y = xy.y;
    Result.z = zw.x;
    Result.w = zw.y;
    
    return Result;
}

inline vec4u 
Vec4u(vec3u xyz, u32 w){
    vec4u Result;
    
    Result.x = xyz.x;
    Result.y = xyz.y;
    Result.z = xyz.z;
    Result.w = w;
    
    return Result;
}

inline vec4i 
Vec4i(s32 x, s32 y, s32 z, s32 w){
    vec4i Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    
    return Result;
}

inline vec4i 
Vec4i(vec2i xy, s32 z, s32 w){
    vec4i Result;
    
    Result.x = xy.x;
    Result.y = xy.y;
    Result.z = z;
    Result.w = w;
    
    return Result;
}

inline vec4i
Vec4i(s32 x, s32 y, vec2i zw){
    vec4i Result;
    
    Result.x = x;
    Result.y = y;
    Result.z = zw.x;
    Result.w = zw.y;
    
    return Result;
}

inline vec4i 
Vec4i(vec2i xy, vec2i zw){
    vec4i Result;
    
    Result.x = xy.x;
    Result.y = xy.y;
    Result.z = zw.x;
    Result.w = zw.y;
    
    return Result;
}

inline vec4i 
Vec4i(vec3i xyz, s32 w){
    vec4i Result;
    
    Result.x = xyz.x;
    Result.y = xyz.y;
    Result.z = xyz.z;
    Result.w = w;
    
    return Result;
}

inline vec2 
operator+(vec2 A, vec2 B) {
    vec2 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return Result;
}

inline vec3 
operator+(vec3 A, vec3 B) {
    vec3 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

    return Result;
}

inline vec4
operator+(vec4 A, vec4 B) {
    vec4 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;

    return Result;
}

inline vec2 
operator+(vec2 A, f32 B) {
    vec2 Result;

    Result.x = A.x + B;
    Result.y = A.y + B;

    return Result;
}

inline vec3 
operator+(vec3 A, f32 B) {
    vec3 Result;

    Result.x = A.x + B;
    Result.y = A.y + B;
    Result.z = A.z + B;

    return Result;
}

inline vec4
operator+(vec4 A, f32 B) {
    vec4 Result;

    Result.x = A.x + B;
    Result.y = A.y + B;
    Result.z = A.z + B;
    Result.w = A.w + B;

    return Result;
}

inline vec2 
operator+(f32 A, vec2 B) {
    vec2 Result;

    Result.x = A + B.x;
    Result.y = A + B.y;

    return Result;
}

inline vec3 
operator+(f32 A, vec3 B) {
    vec3 Result;

    Result.x = A + B.x;
    Result.y = A + B.y;
    Result.z = A + B.z;

    return Result;
}

inline vec4
operator+(f32 A, vec4 B) {
    vec4 Result;

    Result.x = A + B.x;
    Result.y = A + B.y;
    Result.z = A + B.z;
    Result.w = A + B.w;

    return Result;
}

inline vec2
operator+=(vec2& A, vec2 B) {
    vec2 Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec3
operator+=(vec3& A, vec3 B) {
    vec3 Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec4
operator+=(vec4& A, vec4 B) {
    vec4 Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec2
operator+=(vec2& A, f32 B) {
    vec2 Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec3
operator+=(vec3& A, f32 B) {
    vec3 Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec4
operator+=(vec4& A, f32 B) {
    vec4 Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec2 
operator-(vec2 A, vec2 B) {
    vec2 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return Result;
}

inline vec3 
operator-(vec3 A, vec3 B) {
    vec3 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;

    return Result;
}

inline vec4 
operator-(vec4 A, vec4 B) {
    vec4 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;
   
    return Result;
}

inline vec2 
operator-(vec2 A, f32 B) {
    vec2 Result;

    Result.x = A.x - B;
    Result.y = A.y - B;

    return Result;
}

inline vec3 
operator-(vec3 A, f32 B) {
    vec3 Result;

    Result.x = A.x - B;
    Result.y = A.y - B;
    Result.z = A.z - B;

    return Result;
}

inline vec4
operator-(vec4 A, f32 B) {
    vec4 Result;

    Result.x = A.x - B;
    Result.y = A.y - B;
    Result.z = A.z - B;
    Result.w = A.w - B;

    return Result;
}

inline vec2 
operator-(f32 A, vec2 B) {
    vec2 Result;

    Result.x = A - B.x;
    Result.y = A - B.y;

    return Result;
}

inline vec3 
operator-(f32 A, vec3 B) {
    vec3 Result;

    Result.x = A - B.x;
    Result.y = A - B.y;
    Result.z = A - B.z;

    return Result;
}

inline vec4
operator-(f32 A, vec4 B) {
    vec4 Result;

    Result.x = A - B.x;
    Result.y = A - B.y;
    Result.z = A - B.z;
    Result.w = A - B.w;

    return Result;
}

inline vec2
operator-=(vec2& A, vec2 B) {
    vec2 Result;

    Result = A - B;

    A = Result;
    
    return Result;
}

inline vec3
operator-=(vec3& A, vec3 B) {
    vec3 Result;

    Result = A - B;

    A = Result;
    
    return Result;
}

inline vec4
operator-=(vec4& A, vec4 B) {
    vec4 Result;

    Result = A - B;

    A = Result;
    
    return Result;
}

inline vec2
operator-=(vec2& A, f32 B) {
    vec2 Result;

    Result = A - B;

    A = Result;
    
    return Result;
}

inline vec3
operator-=(vec3& A, f32 B) {
    vec3 Result;

    Result = A - B;

    A = Result;
    
    return Result;
}

inline vec4
operator-=(vec4& A, f32 B) {
    vec4 Result;

    Result = A - B;

    A = Result;
    
    return Result;
}

inline vec2
operator*(f32 A, vec2 B) {
    vec2 Result;

    Result.x = A * B.x;
    Result.y = A * B.y;

    return Result;
}

inline vec3 
operator*(f32 A, vec3 B) {
    vec3 Result;

    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;

    return Result;
}

inline vec4
operator*(f32 A, vec4 B) {
    vec4 Result;

    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    Result.w = A * B.w;

    return Result;
}

inline vec2
operator*(vec2 A, f32 B) {
    vec2 Result;

    Result.x = A.x * B;
    Result.y = A.y * B;

    return Result;
}

inline vec3 
operator*(vec3 A, f32 B) {
    vec3 Result;

    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;

    return Result;
}

inline vec4
operator*(vec4 A, f32 B) {
    vec4 Result;

    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    Result.w = A.w * B;

    return Result;
}

inline vec2
operator*=(vec2& A, f32 B) {
    vec2 Result;

    Result = A * B;

    A = Result;
    
    return Result;
}

inline vec3
operator*=(vec3& A, f32 B) {
    vec3 Result;

    Result = A * B;

    A = Result;
    
    return Result;
}

inline vec4
operator*=(vec4& A, f32 B) {
    vec4 Result;

    Result = A * B;

    A = Result;
    
    return Result;
}

inline vec2 
operator/(vec2 A, vec2 B) {
    vec2 Result;

    Result.x = A.x / B.x;
    Result.y = A.y / B.y;

    if(B.x == 0.0f && 
       B.y == 0.0f){
        Warning("Divide by zero!");
    }
 
    return Result;
}

inline vec3 
operator/(vec3 A, vec3 B) {
    vec3 Result;

    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    Result.z = A.z / B.z;

    if(B.x == 0.0f && 
       B.y == 0.0f &&
       B.z == 0.0f){
        Warning("Divide by zero!");
    }

    return Result;
}

inline vec4 
operator/(vec4 A, vec4 B) {
    vec4 Result;

    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    Result.z = A.z / B.z;
    Result.w = A.w / B.w;

    if(B.x == 0.0f && 
       B.y == 0.0f &&
       B.z == 0.0f &&
       B.w == 0.0f){
        Warning("Divide by zero!");
    }

    return Result;
}

inline vec2
operator-(vec2 &v1){
    return Vec2(-v1.x, -v1.y);
}

inline vec3
operator-(vec3 &v1){
    return Vec3(-v1.x, -v1.y, -v1.z);
}

inline vec4
operator-(vec4 &v1){
    return Vec4(-v1.x, -v1.y, -v1.z, -v1.w);
}

inline vec2u 
operator+(vec2u A, vec2u B) {
    vec2u Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return Result;
}

inline vec3u 
operator+(vec3u A, vec3u B) {
    vec3u Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

    return Result;
}

inline vec4u
operator+(vec4u A, vec4u B) {
    vec4u Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;

    return Result;
}

inline vec2u 
operator+(vec2u A, u32 B) {
    vec2u Result;

    Result.x = A.x + B;
    Result.y = A.y + B;

    return Result;
}

inline vec3u 
operator+(vec3u A, u32 B) {
    vec3u Result;

    Result.x = A.x + B;
    Result.y = A.y + B;
    Result.z = A.z + B;

    return Result;
}

inline vec4u
operator+(vec4u A, u32 B) {
    vec4u Result;

    Result.x = A.x + B;
    Result.y = A.y + B;
    Result.z = A.z + B;
    Result.w = A.w + B;

    return Result;
}

inline vec2u 
operator+(u32 A, vec2u B) {
    vec2u Result;

    Result.x = A + B.x;
    Result.y = A + B.y;

    return Result;
}

inline vec3u 
operator+(u32 A, vec3u B) {
    vec3u Result;

    Result.x = A + B.x;
    Result.y = A + B.y;
    Result.z = A + B.z;

    return Result;
}

inline vec4u
operator+(u32 A, vec4u B) {
    vec4u Result;

    Result.x = A + B.x;
    Result.y = A + B.y;
    Result.z = A + B.z;
    Result.w = A + B.w;

    return Result;
}

inline vec2u
operator+=(vec2u& A, vec2u B) {
    vec2u Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec3u
operator+=(vec3u& A, vec3u B) {
    vec3u Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec4u
operator+=(vec4u& A, vec4u B) {
    vec4u Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec2u
operator+=(vec2u& A, u32 B) {
    vec2u Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec3u
operator+=(vec3u& A, u32 B) {
    vec3u Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec4u
operator+=(vec4u& A, u32 B) {
    vec4u Result;

    Result = A + B;

    A = Result;
    
    return Result;
}

inline vec2
Hadamard(vec2 A, vec2 B){
    vec2 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    
    return Result;
}

inline vec3
Hadamard(vec3 A, vec3 B){
    vec3 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    Result.z = A.z * B.z;
    
    return Result;
}

inline vec4
Hadamard(vec4 A, vec4 B){
    vec4 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    Result.z = A.z * B.z;
    Result.w = A.w * B.w;
    
    return Result;
}

inline f32
Length(vec2 V){
    f32 Result;

    Result = Sqrt(V.x*V.x + V.y*V.y);

    return Result;
}

inline vec2
Normalize(vec2 V){
    vec2 Result;

    f32 L = Length(V);

    if(L != 0.0f){
        Result.x = V.x/L;
        Result.y = V.y/L;
    } else {
        Result = Vec2(0.0f, 0.0f);
    }

    return Result;
}

inline f32
Length(vec3 V){
    f32 Result;

    Result = Sqrt(V.x*V.x + V.y*V.y + V.z*V.z);

    return Result;
}

inline vec3
Normalize(vec3 V){
    vec3 Result;

    f32 L = Length(V);

    if(L != 0.0f){
        Result.x = V.x/L;
        Result.y = V.y/L;
        Result.z = V.z/L;
    } else {
        Result = Vec3(0.0f, 0.0f, 0.0f);
    }

    return Result;
}

inline f32
Dot(vec3 A, vec3 B){
    f32 Result;

    Result = A.x*B.x + A.y*B.y + A.z*B.z;

    return Result;
}

inline vec3
Cross(vec3 A, vec3 B){
    vec3 Result;

    Result = Vec3(A.y*B.z - A.z*B.y,
                  A.z*B.x - A.x*B.z,
                  A.x*B.y - A.y*B.x);

    return Result;
}

inline vec3
Reflect(vec3 V, vec3 R){
    // NOTE(furkan):
    // \     |     /
    //  V    R    Result
    //   \   |   /
    // _____________

    vec3 Result;

    Result = ((2.0f * Dot(V, R)) * R) - V;

    return Result;
}

inline vec3 
RotateAround(vec3 V, f32 AngleDegrees, vec3 Axis){

    vec3 Result = V;

    if(AngleDegrees != 0.0f){

        vec3 Projected = Dot(V, Axis) * Normalize(Axis);
        vec3 RotationAxisX = V - Projected;
        vec3 RotationAxisY = Cross(Axis, RotationAxisX);

        f32 RotationAxisXLength = Length(RotationAxisX);
        f32 RotationAxisYLength = Length(RotationAxisY);
 
        f32 AngleRadians = DegreeToRadian(AngleDegrees);
   
        vec3 RotatedOnAxisX = (Cos(AngleRadians) / RotationAxisXLength) * 
                              RotationAxisX;
        vec3 RotatedOnAxisY = (Sin(AngleRadians) / RotationAxisYLength) *
                              RotationAxisY;
    
        Result = RotationAxisXLength * (RotatedOnAxisX + 
                                        RotatedOnAxisY);
        
        Result += Projected;
    }
    
    return Result;
}

inline mat4
operator*(mat4 A, mat4 B) {
    mat4 Result;

    for(u32 i=0; i<4; i++){
        for(u32 j=0; j<4; j++){
            f32 Val = 0.0f;
            
            Val += A.E[i][0] * B.E[0][j];
            Val += A.E[i][1] * B.E[1][j];
            Val += A.E[i][2] * B.E[2][j];
            Val += A.E[i][3] * B.E[3][j];
            
            Result.E[i][j] = Val;
        }
    }

    return Result;
}

inline vec4 
operator*(mat4 A, vec4 B){
    vec4 Result;

    Result.x = A.E[0][0]*B.x + 
               A.E[0][1]*B.y + 
               A.E[0][2]*B.z + 
               A.E[0][3]*B.w;

    Result.y = A.E[1][0]*B.x + 
               A.E[1][1]*B.y + 
               A.E[1][2]*B.z + 
               A.E[1][3]*B.w;

    Result.z = A.E[2][0]*B.x + 
               A.E[2][1]*B.y + 
               A.E[2][2]*B.z + 
               A.E[2][3]*B.w;

    Result.w = A.E[3][0]*B.x + 
               A.E[3][1]*B.y + 
               A.E[3][2]*B.z + 
               A.E[3][3]*B.w;

    return Result;
}

inline mat4 
Mat4Identity(){
    mat4 Result;
    ZeroStruct(&Result);

    Result.E[0][0] = 1.0f;
    Result.E[1][1] = 1.0f;
    Result.E[2][2] = 1.0f;
    Result.E[3][3] = 1.0f;

    return Result;
}

static b32
Mat4Inverse(mat4 M, mat4 *Inverse){
    // NOTE(furkan): Adapted from the code written for
    // "The Mesa 3D Graphics Library". __gluInvertMatrixd function at
    // https://cgit.freedesktop.org/mesa/glu/tree/src/libutil/project.c
    b32 Result = false;

    mat4 Inv;

    Inv.Ptr[0] =   M.Ptr[5]  * M.Ptr[10] * M.Ptr[15] - 
                   M.Ptr[5]  * M.Ptr[11] * M.Ptr[14] - 
                   M.Ptr[9]  * M.Ptr[6]  * M.Ptr[15] + 
                   M.Ptr[9]  * M.Ptr[7]  * M.Ptr[14] +
                   M.Ptr[13] * M.Ptr[6]  * M.Ptr[11] - 
                   M.Ptr[13] * M.Ptr[7]  * M.Ptr[10];

    Inv.Ptr[4] = - M.Ptr[4]  * M.Ptr[10] * M.Ptr[15] + 
                   M.Ptr[4]  * M.Ptr[11] * M.Ptr[14] + 
                   M.Ptr[8]  * M.Ptr[6]  * M.Ptr[15] - 
                   M.Ptr[8]  * M.Ptr[7]  * M.Ptr[14] - 
                   M.Ptr[12] * M.Ptr[6]  * M.Ptr[11] + 
                   M.Ptr[12] * M.Ptr[7]  * M.Ptr[10];

    Inv.Ptr[8] =   M.Ptr[4]  * M.Ptr[9]  * M.Ptr[15] - 
                   M.Ptr[4]  * M.Ptr[11] * M.Ptr[13] - 
                   M.Ptr[8]  * M.Ptr[5]  * M.Ptr[15] + 
                   M.Ptr[8]  * M.Ptr[7]  * M.Ptr[13] + 
                   M.Ptr[12] * M.Ptr[5]  * M.Ptr[11] - 
                   M.Ptr[12] * M.Ptr[7]  * M.Ptr[9];

    Inv.Ptr[12] = -M.Ptr[4]  * M.Ptr[9]  * M.Ptr[14] + 
                   M.Ptr[4]  * M.Ptr[10] * M.Ptr[13] +
                   M.Ptr[8]  * M.Ptr[5]  * M.Ptr[14] - 
                   M.Ptr[8]  * M.Ptr[6]  * M.Ptr[13] - 
                   M.Ptr[12] * M.Ptr[5]  * M.Ptr[10] + 
                   M.Ptr[12] * M.Ptr[6]  * M.Ptr[9];

    Inv.Ptr[1] =  -M.Ptr[1]  * M.Ptr[10] * M.Ptr[15] + 
                   M.Ptr[1]  * M.Ptr[11] * M.Ptr[14] + 
                   M.Ptr[9]  * M.Ptr[2]  * M.Ptr[15] - 
                   M.Ptr[9]  * M.Ptr[3]  * M.Ptr[14] - 
                   M.Ptr[13] * M.Ptr[2]  * M.Ptr[11] + 
                   M.Ptr[13] * M.Ptr[3]  * M.Ptr[10];

    Inv.Ptr[5] =   M.Ptr[0]  * M.Ptr[10] * M.Ptr[15] - 
                   M.Ptr[0]  * M.Ptr[11] * M.Ptr[14] - 
                   M.Ptr[8]  * M.Ptr[2]  * M.Ptr[15] + 
                   M.Ptr[8]  * M.Ptr[3]  * M.Ptr[14] + 
                   M.Ptr[12] * M.Ptr[2]  * M.Ptr[11] - 
                   M.Ptr[12] * M.Ptr[3]  * M.Ptr[10];

    Inv.Ptr[9] =  -M.Ptr[0]  * M.Ptr[9]  * M.Ptr[15] + 
                   M.Ptr[0]  * M.Ptr[11] * M.Ptr[13] + 
                   M.Ptr[8]  * M.Ptr[1]  * M.Ptr[15] - 
                   M.Ptr[8]  * M.Ptr[3]  * M.Ptr[13] - 
                   M.Ptr[12] * M.Ptr[1]  * M.Ptr[11] + 
                   M.Ptr[12] * M.Ptr[3]  * M.Ptr[9];

    Inv.Ptr[13] =  M.Ptr[0]  * M.Ptr[9]  * M.Ptr[14] - 
                   M.Ptr[0]  * M.Ptr[10] * M.Ptr[13] - 
                   M.Ptr[8]  * M.Ptr[1]  * M.Ptr[14] + 
                   M.Ptr[8]  * M.Ptr[2]  * M.Ptr[13] + 
                   M.Ptr[12] * M.Ptr[1]  * M.Ptr[10] - 
                   M.Ptr[12] * M.Ptr[2]  * M.Ptr[9];

    Inv.Ptr[2] =   M.Ptr[1]  * M.Ptr[6] * M.Ptr[15] - 
                   M.Ptr[1]  * M.Ptr[7] * M.Ptr[14] - 
                   M.Ptr[5]  * M.Ptr[2] * M.Ptr[15] + 
                   M.Ptr[5]  * M.Ptr[3] * M.Ptr[14] + 
                   M.Ptr[13] * M.Ptr[2] * M.Ptr[7]  - 
                   M.Ptr[13] * M.Ptr[3] * M.Ptr[6];

    Inv.Ptr[6] =  -M.Ptr[0]  * M.Ptr[6] * M.Ptr[15] + 
                   M.Ptr[0]  * M.Ptr[7] * M.Ptr[14] + 
                   M.Ptr[4]  * M.Ptr[2] * M.Ptr[15] - 
                   M.Ptr[4]  * M.Ptr[3] * M.Ptr[14] - 
                   M.Ptr[12] * M.Ptr[2] * M.Ptr[7]  + 
                   M.Ptr[12] * M.Ptr[3] * M.Ptr[6];

    Inv.Ptr[10] =  M.Ptr[0]  * M.Ptr[5] * M.Ptr[15] - 
                   M.Ptr[0]  * M.Ptr[7] * M.Ptr[13] - 
                   M.Ptr[4]  * M.Ptr[1] * M.Ptr[15] + 
                   M.Ptr[4]  * M.Ptr[3] * M.Ptr[13] + 
                   M.Ptr[12] * M.Ptr[1] * M.Ptr[7]  - 
                   M.Ptr[12] * M.Ptr[3] * M.Ptr[5];

    Inv.Ptr[14] = -M.Ptr[0]  * M.Ptr[5] * M.Ptr[14] + 
                   M.Ptr[0]  * M.Ptr[6] * M.Ptr[13] + 
                   M.Ptr[4]  * M.Ptr[1] * M.Ptr[14] - 
                   M.Ptr[4]  * M.Ptr[2] * M.Ptr[13] - 
                   M.Ptr[12] * M.Ptr[1] * M.Ptr[6]  + 
                   M.Ptr[12] * M.Ptr[2] * M.Ptr[5];

    Inv.Ptr[3] =  -M.Ptr[1] * M.Ptr[6] * M.Ptr[11] + 
                   M.Ptr[1] * M.Ptr[7] * M.Ptr[10] + 
                   M.Ptr[5] * M.Ptr[2] * M.Ptr[11] - 
                   M.Ptr[5] * M.Ptr[3] * M.Ptr[10] - 
                   M.Ptr[9] * M.Ptr[2] * M.Ptr[7]  + 
                   M.Ptr[9] * M.Ptr[3] * M.Ptr[6];

    Inv.Ptr[7] =   M.Ptr[0] * M.Ptr[6] * M.Ptr[11] - 
                   M.Ptr[0] * M.Ptr[7] * M.Ptr[10] - 
                   M.Ptr[4] * M.Ptr[2] * M.Ptr[11] + 
                   M.Ptr[4] * M.Ptr[3] * M.Ptr[10] + 
                   M.Ptr[8] * M.Ptr[2] * M.Ptr[7]  - 
                   M.Ptr[8] * M.Ptr[3] * M.Ptr[6];

    Inv.Ptr[11] = -M.Ptr[0] * M.Ptr[5] * M.Ptr[11] + 
                   M.Ptr[0] * M.Ptr[7] * M.Ptr[9]  + 
                   M.Ptr[4] * M.Ptr[1] * M.Ptr[11] - 
                   M.Ptr[4] * M.Ptr[3] * M.Ptr[9]  - 
                   M.Ptr[8] * M.Ptr[1] * M.Ptr[7]  + 
                   M.Ptr[8] * M.Ptr[3] * M.Ptr[5];

    Inv.Ptr[15] =  M.Ptr[0] * M.Ptr[5] * M.Ptr[10] - 
                   M.Ptr[0] * M.Ptr[6] * M.Ptr[9]  - 
                   M.Ptr[4] * M.Ptr[1] * M.Ptr[10] + 
                   M.Ptr[4] * M.Ptr[2] * M.Ptr[9]  + 
                   M.Ptr[8] * M.Ptr[1] * M.Ptr[6]  - 
                   M.Ptr[8] * M.Ptr[2] * M.Ptr[5];

    f32 Det = M.Ptr[0] * Inv.Ptr[0] + 
              M.Ptr[1] * Inv.Ptr[4] + 
              M.Ptr[2] * Inv.Ptr[8] + 
              M.Ptr[3] * Inv.Ptr[12];

    if(Det != 0.0f){
        Result = true;

        f32 InvDet = 1.0f / Det;

        for(u32 E = 0; E<16; E++){
            Inverse->Ptr[E] = Inv.Ptr[E] * InvDet;
        }
    }
    
    return Result;
}

inline mat4 
Mat4Transpose(mat4 Mat){
    mat4 Result;

    Result.E[0][0] = Mat.E[0][0];
    Result.E[0][1] = Mat.E[1][0];
    Result.E[0][2] = Mat.E[2][0];
    Result.E[0][3] = Mat.E[3][0];

    Result.E[1][0] = Mat.E[0][1];
    Result.E[1][1] = Mat.E[1][1];
    Result.E[1][2] = Mat.E[2][1];
    Result.E[1][3] = Mat.E[3][1];

    Result.E[2][0] = Mat.E[0][2];
    Result.E[2][1] = Mat.E[1][2];
    Result.E[2][2] = Mat.E[2][2];
    Result.E[2][3] = Mat.E[3][2];

    Result.E[3][0] = Mat.E[0][3];
    Result.E[3][1] = Mat.E[1][3];
    Result.E[3][2] = Mat.E[2][3];
    Result.E[3][3] = Mat.E[3][3];

    return Result;
}

inline mat4 
Mat4Translate(mat4 Mat, vec3 Translation){
    mat4 Result;

    if(Translation.x == 0.0f &&
       Translation.y == 0.0f &&
       Translation.z == 0.0f){

       Result = Mat;
    } else {

        mat4 TranslationMatrix = Mat4Identity();
        TranslationMatrix.E[0][3] = Translation.x;
        TranslationMatrix.E[1][3] = Translation.y;
        TranslationMatrix.E[2][3] = Translation.z;

        Result = TranslationMatrix * Mat;
    }

    return Result;
}

inline mat4 
Mat4RotateAround(mat4 Mat, f32 Angle, vec3 RotateAxis){
    mat4 Result;

    if(Angle == 0.0f){

        Result = Mat;
    } else {

        // NOTE(furkan): Theta = RotationX, 
        //               Beta  = RotationY, 
        //               Alpha = RotationZ

        f32 Theta = Atan(RotateAxis.y, Sqrt(Square(RotateAxis.x) + 
                                            Square(RotateAxis.z)));
        f32 Beta = Atan(RotateAxis.x, RotateAxis.z);

        f32 Alpha = DegreeToRadian(Angle);

        Theta *= -1.0f;
        Beta  *= -1.0f;
        Alpha *= -1.0f;

        f32 CosTheta = Cos(Theta);
        f32 SinTheta = Sin(Theta);

        f32 CosBeta = Cos(Beta);
        f32 SinBeta = Sin(Beta);

        f32 CosAlpha = Cos(Alpha);
        f32 SinAlpha = Sin(Alpha);

        mat4 RotationMatrix = Mat4Identity();

        RotationMatrix.E[0][0] = Square(CosBeta)*CosAlpha + Square(SinBeta);
        RotationMatrix.E[0][1] = CosBeta*(SinBeta*SinTheta - 
                                          SinBeta*CosAlpha*SinTheta +
                                          SinAlpha*CosTheta);
        RotationMatrix.E[0][2] = CosBeta*(SinBeta*CosTheta*CosAlpha + 
                                          SinAlpha*SinTheta - 
                                          SinBeta*CosTheta);

        RotationMatrix.E[1][0] = (SinTheta*SinBeta -
                                  SinTheta*SinBeta*CosAlpha -
                                  CosTheta*SinAlpha) * CosBeta;
        RotationMatrix.E[1][1] = (SinBeta*SinTheta*CosAlpha - 
                                  SinAlpha*CosTheta) * SinBeta*SinTheta + 
                                 (CosAlpha*CosTheta -
                                  SinBeta*SinTheta*SinAlpha) * CosTheta + 
                                 Square(CosBeta)*Square(SinTheta);
        RotationMatrix.E[1][2] = (SinBeta*(-SinTheta)*CosAlpha - 
                                  SinAlpha*CosTheta) * SinBeta*CosTheta + 
                                 (CosAlpha*CosTheta - 
                                  SinBeta*SinTheta*SinAlpha) * SinTheta - 
                                 Square(CosBeta)*SinTheta*CosTheta;


        RotationMatrix.E[2][0] = (SinBeta*CosTheta*CosAlpha - 
                                  SinBeta*CosTheta -
                                  SinAlpha*SinTheta) * CosBeta;
        RotationMatrix.E[2][1] = (SinAlpha*SinTheta - 
                                  SinBeta*CosTheta*CosAlpha)*SinBeta*SinTheta + 
                                 (SinBeta*CosTheta*SinAlpha + 
                                  CosAlpha*SinTheta) * CosTheta - 
                                 Square(CosBeta)*CosTheta*SinTheta;
        RotationMatrix.E[2][2] = (SinBeta*CosTheta*CosAlpha - 
                                  SinAlpha*SinTheta) * SinBeta*CosTheta +
                                 (SinBeta*CosTheta*SinAlpha + 
                                  CosAlpha*SinTheta) * SinTheta +
                                 Square(CosBeta)*Square(CosTheta);

        Result = RotationMatrix * Mat;
    }

    return Result;
}

inline mat4 
Mat4Scale(mat4 Mat, vec3 Scale){
    mat4 Result;

    if(Scale.x == 1.0f &&
       Scale.y == 1.0f &&
       Scale.z == 1.0f){

       Result = Mat;
    } else {

        mat4 ScaleMatrix = Mat4Identity();
        ScaleMatrix.E[0][0] = Scale.x;
        ScaleMatrix.E[1][1] = Scale.y;
        ScaleMatrix.E[2][2] = Scale.z;

        Result = ScaleMatrix * Mat;
    }

    return Result;
}

inline mat4 
Mat4Orthographic(f32 Left, f32 Bottom, 
                 f32 Right, f32 Top, 
                 f32 NearZ, f32 FarZ){
    mat4 Result;
    ZeroStruct(&Result);

    f32 Width  = Right-Left;
    f32 Height =   Top-Bottom;
    f32 Depth  =  FarZ-NearZ;

    Result.E[0][0] =  2.0f/Width;
    Result.E[1][1] =  2.0f/Height;
    Result.E[2][2] = -2.0f/Depth;

    Result.E[0][3] = -(Right+Left  )/Width;
    Result.E[1][3] = -(  Top+Bottom)/Height;
    Result.E[2][3] = -( FarZ+NearZ )/Depth;

    Result.E[3][3] = 1.0f;


    return Result;
}

inline mat4 
Mat4Perspective(f32 FOVAngle, f32 Width, f32 Height, f32 NearZ, f32 FarZ){
    mat4 Result;
    ZeroStruct(&Result);

    f32 FOVRadian = DegreeToRadian(FOVAngle);
    f32 TanHalfFOV = Tan(FOVRadian / 2.0f);

    f32 Aspect = Width/Height;

    f32 Depth = NearZ - FarZ;

    Result.E[0][0] =           1.0f / (TanHalfFOV * Aspect);
    Result.E[1][1] =           1.0f /  TanHalfFOV;
    Result.E[2][2] = (NearZ + FarZ) / Depth;

    Result.E[2][3] = (2.0f * NearZ * FarZ) / Depth;
    Result.E[3][2] = -1.0f;

    return Result;
}

inline vec3
PointToWorld(vec3 V, mat4 TransformMat){
    vec3 Result = V;

#if AzimuthDevelopmentBuild
    Assert(Abs(TransformMat.E[3][0]) < 0.001f);
    Assert(Abs(TransformMat.E[3][1]) < 0.001f);
    Assert(Abs(TransformMat.E[3][2]) < 0.001f);
    Assert(Abs(TransformMat.E[3][3]) > 0.999f &&
           Abs(TransformMat.E[3][3]) < 1.001f);
#endif

    Result.x = TransformMat.E[0][0]*V.x + 
               TransformMat.E[0][1]*V.y + 
               TransformMat.E[0][2]*V.z +
               TransformMat.E[0][3];
    Result.y = TransformMat.E[1][0]*V.x + 
               TransformMat.E[1][1]*V.y + 
               TransformMat.E[1][2]*V.z +
               TransformMat.E[1][3];
    Result.z = TransformMat.E[2][0]*V.x + 
               TransformMat.E[2][1]*V.y + 
               TransformMat.E[2][2]*V.z +
               TransformMat.E[2][3];

    return Result;
}

inline vec3
VectorToWorld(vec3 V, mat4 TransformMat){
    vec3 Result = V;

#if AzimuthDevelopmentBuild
    Assert(Abs(TransformMat.E[3][0]) < 0.001f);
    Assert(Abs(TransformMat.E[3][1]) < 0.001f);
    Assert(Abs(TransformMat.E[3][2]) < 0.001f);
    Assert(Abs(TransformMat.E[3][3]) > 0.999f &&
           Abs(TransformMat.E[3][3]) < 1.001f);
#endif

    Result.x = TransformMat.E[0][0]*V.x + 
               TransformMat.E[0][1]*V.y + 
               TransformMat.E[0][2]*V.z;
    Result.y = TransformMat.E[1][0]*V.x + 
               TransformMat.E[1][1]*V.y + 
               TransformMat.E[1][2]*V.z;
    Result.z = TransformMat.E[2][0]*V.x + 
               TransformMat.E[2][1]*V.y + 
               TransformMat.E[2][2]*V.z;

    return Result;
}

inline vec3
NormalToWorld(vec3 V, mat4 InverseTransformMat){
    vec3 Result = V;

    Result.x = InverseTransformMat.E[0][0]*V.x + 
               InverseTransformMat.E[1][0]*V.y + 
               InverseTransformMat.E[2][0]*V.z;
    Result.y = InverseTransformMat.E[0][1]*V.x + 
               InverseTransformMat.E[1][1]*V.y + 
               InverseTransformMat.E[2][1]*V.z;
    Result.z = InverseTransformMat.E[0][2]*V.x + 
               InverseTransformMat.E[1][2]*V.y + 
               InverseTransformMat.E[2][2]*V.z;
    Result = Normalize(Result);

    return Result;
}

inline void
OrthonormalBasis(vec3 W, vec3 *U, vec3 *V) {
    // NOTE(furkan): Taken from:
    // "Building an Orthonormal Basis, Revisited", Pixar, 2017
    //     https://graphics.pixar.com/library/OrthonormalB/paper.pdf
    if(W.z < 0.0){
        const f32 a = 1.0f / (1.0f - W.z);
        const f32 b = W.x * W.y * a;

        *U = Vec3(1.0f - W.x * W.x * a, -b, W.x);
        *V = Vec3(b, W.y * W.y * a - 1.0f, -W.y);
    } else {
        const f32 a = 1.0f / (1.0f + W.z);
        const f32 b = -W.x * W.y * a;

        *U = Vec3(1.0f - W.x * W.x * a, b, -W.x);
        *V = Vec3(b, 1.0f - W.y * W.y * a, -W.y);
    }
}

inline f32 
Luminance(f32 R, f32 G, f32 B){
    f32 Result;

#if 1
    Result = R*0.27f + G*0.67f + B*0.06f;
#elif 0
    Result = R*0.2126f + G*0.7152f + B*0.0722f;
#else
    Result = R*0.265068f + G*0.67023428f + B*0.06409157f;
#endif

    return Result;
}

inline f32
Luminance(vec3 Color){
    f32 Result;

    Result = Luminance(Color.r, Color.g, Color.b);

    return Result;
}
