typedef enum {
    TonemapOperator_None,
    TonemapOperator_Photographic,
} tonemap_operator_type;

typedef struct {
    union {
        struct {
            f32 KeyValue;
            f32 BurnoutPercentage;
        };
    };

    f32 Saturation;

    tonemap_operator_type Operator;
} tonemap;

typedef enum {
    CameraType_Detailed, 
    CameraType_Simple
} camera_type;

typedef struct {
    tonemap Tonemap;

    char ImageName[256];
    file_format SaveFormat;

    union {
        struct { // NOTE(furkan): Detailed camera
            vec4 NearPlane;
        };
        struct { // NOTE(furkan): Simple camera
            f32 FovY;
        };
    };

    vec3 Position;
    vec3 Gaze;
    vec3 Up;
    vec3 BackgroundColor;
    vec2u Resolution;
    f32 NearDistance;
    f32 FocusDistance;
    f32 ApertureSize;
    f32 Gamma;
    f32 InvGamma;
    b32 GammaCorrection_sRGB;
    b32 GazeIsPoint;
    b32 IsLeftHanded;
    b32 SaveEXR;
    u32 SampleCount;
    u32 ID;
    camera_type Type;
} camera;

static camera
DefaultCamera(){
    camera Result;

    ZeroStruct(&Result);

    Result.ImageName[0] = 'u';
    Result.ImageName[1] = '.';
    Result.ImageName[2] = 'p';
    Result.ImageName[3] = 'n';
    Result.ImageName[4] = 'g';

    Result.SaveFormat = FileFormat_PNG;
    Result.Tonemap.Operator = TonemapOperator_None;

    Result.NearPlane = Vec4(1.0f, -1.0f, -1.0f, 1.0f);
    Result.Gaze = Vec3(0.0f, 0.0f, -1.0f);
    Result.Up = Vec3(0.0f, 1.0f, 0.0f);
    Result.Resolution = Vec2u(640, 360);
    Result.NearDistance = 1.0f;
    Result.Gamma = Result.InvGamma = 1.0f;
    Result.SampleCount = 1;
    Result.Type = CameraType_Detailed;

    return Result;
}
