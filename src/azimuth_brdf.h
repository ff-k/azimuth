typedef enum {
    BRDF_Phong,
    BRDF_ModifiedPhong,
    BRDF_NormalizedModifiedPhong,
    BRDF_BlinnPhong,
    BRDF_ModifiedBlinnPhong,
    BRDF_NormalizedModifiedBlinnPhong,
    BRDF_TorranceSparrow
} brdf_type;

typedef struct {
    f32 Exponent;
    f32 RefractiveIndex;

    brdf_type Type;
    u32 ID;
} brdf;

static brdf
DefaultBRDF(){
    brdf Result;

    ZeroStruct(&Result);

    Result.Type = BRDF_BlinnPhong;

    return Result;
}
