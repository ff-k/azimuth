typedef struct {
    brdf BRDF;

    vec3 AmbientReflectance;
    vec3 DiffuseReflectance;
    vec3 SpecularReflectance;
    vec3 MirrorReflectance;
    vec3 Transparency;

    f32 RefractionIndex;
    f32 Glossiness;

    b32 Degamma;

    u32 BRDFID; // NOTE(furkan): If it is 0, then use material's own
                // BRDF, i.e. do not copy a BRDF from the global BRDF
                // container
    u32 ID;
} material;

static material
DefaultMaterial(){
    material Result;

    ZeroStruct(&Result);

    Result.BRDF = DefaultBRDF();

    return Result;
}
