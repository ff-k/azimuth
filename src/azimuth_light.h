typedef enum {
    LightType_PointLight,
    LightType_AreaLight,
    LightType_DirectionalLight,
    LightType_SpotLight,
    LightType_EntityLight
} light_type;

typedef struct {
    union {
        struct { // NOTE(furkan): Point light
            vec3 Position;
            vec3 Intensity;
        };
        struct { // NOTE(furkan): Area light
            vec3 Position;
            vec3 Intensity;
            vec3 EdgeVectors[2];
            vec3 Normal;
        };
        struct { // NOTE(furkan): Directional light
            vec3 Direction;
            vec3 Intensity;
        };
        struct { // NOTE(furkan): Spot light
            vec3 Position;
            vec3 Intensity;
            vec3 SpotDirection;
            f32 CoverageAngle;      // NOTE(furkan): In degrees
            f32 FallOffAngle;       // NOTE(furkan): In degrees
            f32 CosHalfCoverage;
            f32 CosHalfFallOff;
        };
        struct { // NOTE(furkan): Entity light
            // TODO(furkan): Store a reference instead
            entity Entity;
        };
    };

    u32 ID;
    light_type Type;
} light;

static light
DefaultLight(){
    light Result;

    ZeroStruct(&Result);

    Result.Type = LightType_PointLight;

    return Result;
}
