typedef struct {
    vec3 Origin;
    vec3 Direction;
    f32 T;
} ray3;

typedef struct {
    vec3 P;
    vec3 Normal;
    vec2 UV;
    entity *Entity;
    f32 t;
} ray_hit;

inline ray3
Ray3(vec3 O, vec3 D, f32 T){
    ray3 Result;

    Result = {O, D, T};

    return Result;
}

static ray3
GetLocalRay(ray3 Ray, mat4 InverseTransformMat){
    ray3 Result = Ray;

    Result.Origin.x    = InverseTransformMat.E[0][0]*Ray.Origin.x + 
                         InverseTransformMat.E[0][1]*Ray.Origin.y + 
                         InverseTransformMat.E[0][2]*Ray.Origin.z + 
                         InverseTransformMat.E[0][3];
    Result.Origin.y    = InverseTransformMat.E[1][0]*Ray.Origin.x + 
                         InverseTransformMat.E[1][1]*Ray.Origin.y + 
                         InverseTransformMat.E[1][2]*Ray.Origin.z + 
                         InverseTransformMat.E[1][3];
    Result.Origin.z    = InverseTransformMat.E[2][0]*Ray.Origin.x + 
                         InverseTransformMat.E[2][1]*Ray.Origin.y + 
                         InverseTransformMat.E[2][2]*Ray.Origin.z + 
                         InverseTransformMat.E[2][3];
    
    Result.Direction.x = InverseTransformMat.E[0][0]*Ray.Direction.x + 
                         InverseTransformMat.E[0][1]*Ray.Direction.y + 
                         InverseTransformMat.E[0][2]*Ray.Direction.z;
    Result.Direction.y = InverseTransformMat.E[1][0]*Ray.Direction.x + 
                         InverseTransformMat.E[1][1]*Ray.Direction.y + 
                         InverseTransformMat.E[1][2]*Ray.Direction.z;
    Result.Direction.z = InverseTransformMat.E[2][0]*Ray.Direction.x + 
                         InverseTransformMat.E[2][1]*Ray.Direction.y + 
                         InverseTransformMat.E[2][2]*Ray.Direction.z;
    Result.Direction = Normalize(Result.Direction);
    
#if AzimuthDevelopmentBuild
    f32 LocalOriginW    = InverseTransformMat.E[3][0]*Ray.Origin.x + 
                          InverseTransformMat.E[3][1]*Ray.Origin.y + 
                          InverseTransformMat.E[3][2]*Ray.Origin.z + 
                          InverseTransformMat.E[3][3];
    f32 LocalDirectionW = InverseTransformMat.E[3][0]*Ray.Direction.x + 
                          InverseTransformMat.E[3][1]*Ray.Direction.y + 
                          InverseTransformMat.E[3][2]*Ray.Direction.z;
    
    Assert(LocalOriginW    < 1.001f && LocalOriginW    > 0.999f);
    Assert(LocalDirectionW < 0.001f && LocalDirectionW > -0.001f);
#endif
    
    return Result;
}

static b32 
RayAABBIntersection(aabb AABB, ray3 Ray){
    b32 Result = false;

    vec3 InverseRayDir;
    InverseRayDir.x = 1.0f/Ray.Direction.x;
    InverseRayDir.y = 1.0f/Ray.Direction.y;
    InverseRayDir.z = 1.0f/Ray.Direction.z;

    vec3 MinOriginDist = AABB.Min - Ray.Origin;
    vec3 MaxOriginDist = AABB.Max - Ray.Origin;

    vec3 TEnter;
    vec3 TExit;
    if(InverseRayDir.x > 0.0f){
        TEnter.x = MinOriginDist.x*InverseRayDir.x;
        TExit.x  = MaxOriginDist.x*InverseRayDir.x;
    } else {
        TEnter.x = MaxOriginDist.x*InverseRayDir.x;
        TExit.x  = MinOriginDist.x*InverseRayDir.x;
    }
    
    if(InverseRayDir.y > 0.0f){
        TEnter.y = MinOriginDist.y*InverseRayDir.y;
        TExit.y  = MaxOriginDist.y*InverseRayDir.y;
    } else {
        TEnter.y = MaxOriginDist.y*InverseRayDir.y;
        TExit.y  = MinOriginDist.y*InverseRayDir.y;
    }
    
    if(InverseRayDir.z > 0.0f){
        TEnter.z = MinOriginDist.z*InverseRayDir.z;
        TExit.z  = MaxOriginDist.z*InverseRayDir.z;
    } else {
        TEnter.z = MaxOriginDist.z*InverseRayDir.z;
        TExit.z  = MinOriginDist.z*InverseRayDir.z;
    }
    
    f32 TEnterMax = (TEnter.x > TEnter.y)  ? TEnter.x : TEnter.y;
    TEnterMax     = (TEnter.z > TEnterMax) ? TEnter.z : TEnterMax;

    f32 TExitMin = (TExit.x < TExit.y)  ? TExit.x : TExit.y;
    TExitMin     = (TExit.z < TExitMin) ? TExit.z : TExitMin;

    f32 Epsilon = 0.0f;
    if(TEnterMax <= TExitMin && TExitMin > Epsilon){
        f32 THit;
        if(TEnterMax > Epsilon){
            THit = TEnterMax;
        } else {
            THit = TExitMin;
        }
        
        Result = true;
    }
    
    return Result;
}
