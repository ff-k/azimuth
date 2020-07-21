typedef enum {
    EntityType_Sphere,
    EntityType_Triangle,
    EntityType_MeshInstance,
    EntityType_BVHNode
} entity_type;

typedef enum {
    ShadingMode_Flat,
    ShadingMode_Smooth
} shading_mode;

typedef struct entity entity;

typedef struct {
    vec3 P0;
    vec3 P1;
    vec3 P2;
    vec3 N;
} triangle;

typedef struct {
    mat4 InvTransformationMatrix;
    mat4 TransformationMatrix;
    vec3 MotionVector;
    vec3 Radiance;
    entity *BVH;
    triangle *Triangles;
    f32 TotalArea;
    s32 UVOffset;
    b32 IsEmitter;
    u32 TriangleCount;
    u32 TextureIndex;
    u32 MaterialIndex;
    u32 ID;
    shading_mode ShadingMode;
} mesh;

inline mesh
DefaultMesh(){
    mesh Result = {0};

    ZeroStruct(&Result);

    Result.InvTransformationMatrix = Result.TransformationMatrix = 
                                     Mat4Identity();

    return Result;
}

struct entity {
    union {
        struct { // NOTE(furkan): Sphere
            vec3 Center;
            f32 Radius;
        };
        struct { // NOTE(furkan): Triangle
            vec3 P0;
            vec3 P1;
            vec3 P2;

            vec3 N0;
            vec3 N1;
            vec3 N2;

            vec3 FaceNormal;

            vec2 UV0;
            vec2 UV1;
            vec2 UV2;

            shading_mode ShadingMode;
        };
        struct { // NOTE(furkan): MeshInstance
            aabb AABB;
            mesh *Mesh;

            u32 BaseMeshID;
            b32 ResetTransform;
        };
        struct { // NOTE(furkan): BVHNode
            aabb AABB;
            struct entity *Right;
            struct entity *Left;
        };
    };

    mat4 InvTransformationMatrix;
    mat4 TransformationMatrix;
    vec3 MotionVector;
    vec3 Radiance;
    b32 IsEmitter;
    // NOTE(furkan): If TextureIndex is 0, then entity 
    // has no texture on it.
    u32 TextureIndex;
    // NOTE(furkan): If MaterialIndex is 0, then entity 
    // uses the material with an ID of 1.
    u32 MaterialIndex;
    u32 ID;
    u32 UniqueEntityID;
    entity_type Type;
};

static entity
DefaultEntity(){
    entity Result;

    ZeroStruct(&Result);

    Result.Type = EntityType_Sphere;
    Result.InvTransformationMatrix = Result.TransformationMatrix = 
                                     Mat4Identity();

    return Result;
}
