typedef struct {
    f32 Intersection;
    f32 ShadowRay;
} epsilon_values;

typedef struct {
    vec3     AmbientLight;
    
    vec3     BackgroundColor;
    texture  BackgroundTexture;

    entity   *BVHRoot;

    mesh     *Meshes;
    material *Materials;
    light    *Lights;
    texture  *Textures;
    texture  EnvironmentMap;

    u32 MeshCount;
    u32 MaterialCount;
    u32 LightCount;
    u32 TextureCount;

    u32 MaxRecursionDepth;
    b32 UseBackgroundTexture;
    b32 UseEnvironmentMap;
    b32 PathTrace;
    b32 ImportanceSampling;
} scene;

static scene
DefaultScene(){
    scene Result;

    ZeroStruct(&Result);

    Result.BackgroundTexture = DefaultTexture();
    Result.EnvironmentMap = DefaultTexture();

    return Result;
}
