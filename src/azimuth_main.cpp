#include "azimuth_config.h"
#include "azimuth_common.h"
#include "azimuth_util.h"
#include "azimuth_profiler.h"
#include "azimuth_math.h"
#include "azimuth_string.h"
#include "azimuth_io.h"
#include "azimuth_platform.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <stb_image_write.h>

#pragma warning(push, 0)
#include <OpenEXR/ImfRgba.h>
#include <OpenEXR/ImfRgbaFile.h>
#pragma warning(pop)

#include "azimuth_brdf.h"
#include "azimuth_texture.h"
#include "azimuth_material.h"
#include "azimuth_entity.h"
#include "azimuth_light.h"
#include "azimuth_camera.h"
#include "azimuth_scene.h"
#include "azimuth_ray.h"
#include "azimuth_xml.h"

#include "azimuth_io.cpp"
#include "azimuth_memory.cpp"
#include "azimuth_math.cpp"
#include "azimuth_bvh.cpp"
#include "azimuth_xml.cpp"

typedef struct {
    vec2u Max;
    vec2u Min;
} scissor;

typedef enum {
    FilterType_Box,
    FilterType_Gaussian
} filter_type;

typedef struct {
    vec3 Color;
    f32 TotalWeight;
} pixel;

static vec3 TraceRay(scene *, ray3, u32, b32, b32,  
                     const epsilon_values *, f32 *);

static b32
RayEntityIntersection(ray_hit *Hit, ray3 Ray, 
                      entity *Entity, texture *Textures, 
                      b32 Cull, const f32 HitEpsilon){
    b32 Result = false;

    texture *Texture = 0;
    vec3 Pu = { 0 };
    vec3 Pv = { 0 };
    b32 DeferredNormalTransformation = false;
    switch(Entity->Type){
        case EntityType_Sphere: {
            vec3 Motion = Ray.T*Entity->MotionVector;
            Ray.Origin -= Motion;
            ray3 LocalRay = GetLocalRay(Ray,
                                        Entity->InvTransformationMatrix);

            f32 Radius = Entity->Radius;
            vec3 SphereCenter = Entity->Center;
            vec3 Distance = LocalRay.Origin - SphereCenter;

            f32 A = Dot(LocalRay.Direction, LocalRay.Direction);
            f32 B = Dot(Distance, LocalRay.Direction);
            f32 C = Dot(Distance, Distance) - Square(Radius);

            f32 Discriminant = Square(B) - (A * C);
            if(Discriminant > 0.0f){
                f32 SqrtDisc = Sqrt(Discriminant);
                f32 T1 = (-B + SqrtDisc)/(A);
                f32 T2 = (-B - SqrtDisc)/(A);

                f32 t = f32_Max;
                if(T1 > HitEpsilon && T2 > HitEpsilon){
                    t = MinOf(T1, T2);
                } else if(T1 > HitEpsilon){
                    t = T1;
                } else if(T2 > HitEpsilon){
                    t = T2;
                }

                if(t > HitEpsilon && t < Hit->t){
                    vec3 HitP = LocalRay.Origin + (LocalRay.Direction * t);
                    vec3 LocalHitP = HitP-SphereCenter;
                    vec3 HitN = Normalize(LocalHitP);

                    vec3 WorldPosition = PointToWorld(HitP,
                                                 Entity->TransformationMatrix);
                    WorldPosition += Motion;

                    f32 WorldT = Length(WorldPosition-Ray.Origin-Motion) / 
                                 Length(Ray.Direction);

                    if(WorldT > HitEpsilon && WorldT < Hit->t){
                        f32 Theta = Acos(HitN.y);
                        f32 Phi   = Atan(HitN.z, HitN.x);

                        f32 TwoPi = Pi_f32*2.0f;

                        Hit->P = WorldPosition;

                        if(Entity->TextureIndex){
                            Texture = Textures + (Entity->TextureIndex-1);
                            if(Texture->BumpMap){
                                f32 PyPi = Pi_f32*LocalHitP.y;
                                f32 RPi  = Pi_f32*Radius;

                                Pv = Vec3( Cos(Phi)  * PyPi,
                                          -Sin(Theta)* RPi,
                                           Sin(Phi)  * PyPi);

                                Pu = Vec3( TwoPi*LocalHitP.z,
                                           0.0f,
                                          -TwoPi*LocalHitP.x);
                            } else {
                                Texture = 0;
                            }

                            Hit->Normal = HitN;
                            DeferredNormalTransformation = true;
                        } else {
                            Hit->Normal = NormalToWorld(HitN, 
                                             Entity->InvTransformationMatrix);
                        }

                        Hit->UV = Vec2((-Phi+Pi_f32)/TwoPi, 
                                       Theta/Pi_f32);
                        Hit->Entity = Entity;
                        Hit->t = WorldT;

                        Result = true;
                    }
                }
            }
        } break;
        case EntityType_Triangle: {
            vec3 P0 = Entity->P0;
            vec3 P1 = Entity->P1;
            vec3 P2 = Entity->P2;

            vec3 FaceNormal = Entity->FaceNormal;
            if(!Cull || Dot(Ray.Direction, FaceNormal) < 0.0f){
                f32 A = (P0.x - P1.x);
                f32 B = (P0.x - P2.x);
                f32 C = Ray.Direction.x;

                f32 D = (P0.y - P1.y);
                f32 E = (P0.y - P2.y);
                f32 F = Ray.Direction.y;

                f32 G = (P0.z - P1.z);
                f32 H = (P0.z - P2.z);
                f32 I = Ray.Direction.z;
                
                f32 DetM       = (A*E*I) + (D*H*C) + (G*B*F) - 
                                 (C*E*G) - (F*H*A) - (I*B*D);

                f32 K = (P0.x - Ray.Origin.x);
                f32 M = (P0.y - Ray.Origin.y);
                f32 N = (P0.z - Ray.Origin.z);

                f32 DetM_Beta  = (K*E*I) + (M*H*C) + (N*B*F) - 
                                 (C*E*N) - (F*H*K) - (I*B*M);
                // TODO(furkan): Set LowerLimit to 0
                f32 LowerLimit = -0.000001f;
                f32 Beta = DetM_Beta / DetM;
                if(Beta > LowerLimit && Beta <= 1.0f){
                    f32 DetM_Gamma = (A*M*I) + (D*N*C) + (G*K*F) - 
                                     (C*M*G) - (F*N*A) - (I*K*D);
 
                    f32 Gamma = DetM_Gamma / DetM;
                    if(Gamma > LowerLimit && Gamma <= 1.0f){
                        f32 DetM_t = (A*E*N) + (D*H*K) + (G*B*M) - 
                                     (K*E*G) - (M*H*A) - (N*B*D);
                        f32 Alpha = 1.0f - Beta - Gamma;
                        if(Alpha > LowerLimit){

                            f32 t = DetM_t / DetM;
                            if(t > HitEpsilon && t < Hit->t){
                                Hit->P = Ray.Origin + (Ray.Direction*t);
                                switch(Entity->ShadingMode){
                                    case ShadingMode_Flat:{
                                        Hit->Normal = FaceNormal;
                                    } break;
                                    case ShadingMode_Smooth:{
                                        vec3 N0 = Entity->N0;
                                        vec3 N1 = Entity->N1;
                                        vec3 N2 = Entity->N2;
                                        Hit->Normal = Alpha*N0 + 
                                                       Beta*N1 + 
                                                      Gamma*N2;
                                    } break;
                                }
                                vec2 UV0 = Entity->UV0;
                                vec2 UV1 = Entity->UV1;
                                vec2 UV2 = Entity->UV2;

                                Hit->UV = (Alpha*UV0) +
                                          ( Beta*UV1) +
                                          (Gamma*UV2);
                                Hit->Entity = Entity;
                                Hit->t = t;

                                if(Entity->TextureIndex){
                                    Texture = Textures + 
                                              (Entity->TextureIndex-1);
                                    if(Texture->BumpMap){
                                        f32 Divisor = ((UV1.u-UV0.u) * 
                                                       (UV2.v-UV0.v)) - 
                                                      ((UV1.v-UV0.v) * 
                                                       (UV2.u-UV0.u));
                                        Divisor = 1.0f/Divisor;
                                
                                        Pu = (UV2.v-UV0.v)*(P1-P0) + 
                                             (UV0.v-UV1.v)*(P2-P0);
                                        Pu *= Divisor;

                                        Pv = (UV0.u-UV2.u)*(P1-P0) + 
                                             (UV1.u-UV0.u)*(P2-P0);
                                        Pv *= Divisor;
                                    } else {
                                        Texture = 0;
                                    }
                                }

                                Result = true;
                            }
                        }
                    }
                }
            }
        } break;
        case EntityType_MeshInstance: {
            if(RayAABBIntersection(Entity->AABB, Ray)){
                vec3 Motion = Ray.T*Entity->MotionVector;
                Ray.Origin -= Motion;
                ray3 LocalRay = GetLocalRay(Ray, 
                                            Entity->InvTransformationMatrix);

                ray_hit LocalHit = { 0 };
                LocalHit.t = f32_Max;
                if(RayEntityIntersection(&LocalHit, LocalRay, 
                                         Entity->Mesh->BVH, Textures, 
                                         Cull, HitEpsilon)){

                    vec3 WorldPosition = PointToWorld(LocalHit.P, 
                                                 Entity->TransformationMatrix);
                    WorldPosition += Motion;

                    f32 WorldT = Length(WorldPosition-Ray.Origin-Motion) / 
                                 Length(Ray.Direction);

                    if(WorldT > HitEpsilon && WorldT < Hit->t){
                        *Hit = LocalHit;

                        vec3 WorldNormal = NormalToWorld(LocalHit.Normal, 
                                        Entity->InvTransformationMatrix);

                        Hit->P = WorldPosition;
                        Hit->Normal = WorldNormal;
                        Hit->Entity = Entity;
                        Hit->t = WorldT;

                        Result = true;
                    } else {
                        Result = false;
                    }
                }
            }
        } break;
        case EntityType_BVHNode: {
            if(RayAABBIntersection(Entity->AABB, Ray)){
                ray_hit LeftHit = { 0 };
                LeftHit.t = f32_Max;
                b32 L = RayEntityIntersection(&LeftHit, Ray, 
                                              Entity->Left, 
                                              Textures, 
                                              Cull, HitEpsilon);
                
                ray_hit RightHit = { 0 };
                RightHit.t = f32_Max;
                b32 R = RayEntityIntersection(&RightHit, Ray, 
                                              Entity->Right, 
                                              Textures, 
                                              Cull, HitEpsilon);
                
                if(L){
                    Result = true;

                    if(R){
                         // NOTE(furkan) : Select min
                        if(LeftHit.t < RightHit.t){
                            *Hit = LeftHit;
                        } else {
                            *Hit = RightHit;
                        }
                    } else {
                        // NOTE(furkan) : Select L
                        *Hit = LeftHit;
                    }
                } else if(R){
                    Result = true;

                    // NOTE(furkan) : Select R
                    *Hit = RightHit;
                }
            }
        } break;
        UnexpectedDefaultCase;
    }

    if(Texture){
        Assert(Result);
        Assert(Texture->BumpMap);

        vec2 UV = Hit->UV;
        switch(Texture->Type){
            case TextureType_2D: {
                vec3 A = SampleTexture2D(Texture, UV, false);
                vec3 B = SampleTexture2D(Texture, 
                                         {UV.u + Texture->InvWidth, UV.v}, 
                                         false);
                vec3 C = SampleTexture2D(Texture, 
                                         {UV.u, UV.v + Texture->InvHeight},
                                         false);

                vec3 DuRGB = B-A;
                vec3 DvRGB = C-A;

                f32 Du = ((DuRGB.x + DuRGB.y + DuRGB.z)/3.0f) * 
                         Texture->BumpMapMultiplier;
                f32 Dv = ((DvRGB.x + DvRGB.y + DvRGB.z)/3.0f) * 
                         Texture->BumpMapMultiplier;

                vec3 dPu = Pu + Du*Hit->Normal;
                vec3 dPv = Pv + Dv*Hit->Normal;

                Hit->Normal = Normalize(Cross(dPv, dPu));
            } break;
            case TextureType_PerlinNoise: {
                const f32 PerlinBumpEps = 0.001f;
                vec3 P = Hit->P;
                f32 D  = SamplePerlinNoise(Texture, P);
                f32 Dx = SamplePerlinNoise(Texture, 
                                           Vec3(P.x+PerlinBumpEps, 
                                                P.y, 
                                                P.z));
                f32 Dy = SamplePerlinNoise(Texture, 
                                           Vec3(P.x, 
                                                P.y+PerlinBumpEps, 
                                                P.z));
                f32 Dz = SamplePerlinNoise(Texture, 
                                           Vec3(P.x, 
                                                P.y, 
                                                P.z+PerlinBumpEps));

                vec3 G = Vec3((Dx - D)/PerlinBumpEps, 
                              (Dy - D)/PerlinBumpEps, 
                              (Dz - D)/PerlinBumpEps);

                Hit->Normal -= G*Texture->BumpMapMultiplier;
                Hit->Normal = Normalize(Hit->Normal);
            } break;
            UnexpectedDefaultCase;
        }
    }

    if(DeferredNormalTransformation){
        Hit->Normal = NormalToWorld(Hit->Normal, 
                                    Entity->InvTransformationMatrix);
    }

    return Result;
}

typedef struct {
    vec3 LightDir;
    f32 InvPDF;
} entity_light_sample;

static entity_light_sample
SampleEntityLight(entity *LE, vec3 HitP){
    entity_light_sample Result = {0};

    Assert(LE->IsEmitter);

    vec3 HitPLocal = PointToWorld(HitP, 
                                  LE->InvTransformationMatrix);
    
    vec3 LightDirLocal = {0};
    switch(LE->Type){
        case EntityType_Sphere: {

            vec3 D = LE->Center - HitPLocal;
            f32 ROverD = LE->Radius / Length(D);
            f32 CosThetaMax = Sqrt(1.0f - Square(ROverD));

            f32 CosTheta = 1.0f + Random01()*(CosThetaMax - 
                                              1.0f);
            f32 SinTheta = Sqrt(1.0f - Square(CosTheta));
            f32 Phi = 2.0f * Pi_f32 * Random01();
            f32 CosPhi = Cos(Phi);
            f32 SinPhi = Sin(Phi);
            
            vec3 W = Normalize(D);
            vec3 U, V;
            OrthonormalBasis(W, &U, &V);

            LightDirLocal = Normalize(U*SinTheta*CosPhi + 
                                      V*SinTheta*SinPhi + 
                                      W*CosTheta);

            Result.InvPDF = 2.0f * Pi_f32 * (1.0f - CosThetaMax);
        } break;
        case EntityType_MeshInstance: {
            mesh *M = LE->Mesh;
            u32 Idx = (u32)(Random01()*(M->TriangleCount));
            Idx = MinOf(Idx, M->TriangleCount-1);

            triangle *T = M->Triangles+Idx;

            f32 Rand1Sqrt = Sqrt(Random01());
            f32 Rand2 = Random01();

            vec3 Q = (1.0f-Rand2    )*T->P0 + Rand2*T->P1;
            vec3 P = (1.0f-Rand1Sqrt)*T->P2 + Rand1Sqrt*Q;

            vec3 Wi = P - HitPLocal;

            f32 A = M->TotalArea;
            LightDirLocal = Normalize(Wi);

            f32 CosAngle = Dot(LightDirLocal, T->N);
            if(CosAngle < 0.0f){
                CosAngle *= -1.0f;
            }

            Result.InvPDF = (A*CosAngle) / 
                            Square(Length(Wi));
        } break;
        UnexpectedDefaultCase;
    }

    Result.LightDir = VectorToWorld(LightDirLocal, 
                                    LE->TransformationMatrix);

    return Result;
}

static void
EvaluateBRDF(f32 *Diffuse, f32 *Specular, 
             brdf *BRDF, vec3 LightDir, vec3 HitN, 
             vec3 ViewDir, f32 CosThetaIncident){
    // NOTE(furkan): LightDir must be normalized
    switch(BRDF->Type){
        case BRDF_Phong:{
            vec3 Reflected = Reflect(LightDir, HitN);
            f32 CosAngle = Dot(-ViewDir, Reflected);
    
            if(CosAngle > 0.0f){
                f32 Exp = BRDF->Exponent;
                f32 CosTerm = Power(CosAngle, Exp);
                *Specular = CosTerm/CosThetaIncident;
            } else {
                *Specular = 0.0f;
            }
        } break;
        case BRDF_ModifiedPhong:{
            vec3 Reflected = Reflect(LightDir, HitN);
            f32 CosAngle = Dot(-ViewDir, Reflected);
    
            if(CosAngle > 0.0f){
                f32 Exp = BRDF->Exponent;
                f32 CosTerm = Power(CosAngle, Exp);
                *Specular = CosTerm;
            } else {
                *Specular = 0.0f;
            }
        } break;
        case BRDF_NormalizedModifiedPhong:{
            *Diffuse = 1.0f/Pi;
    
            vec3 Reflected = Reflect(LightDir, HitN);
            f32 CosAngle = Dot(-ViewDir, Reflected);
    
            if(CosAngle > 0.0f){
                f32 Exp = BRDF->Exponent;
                f32 CosTerm = Power(CosAngle, Exp);
                *Specular = ((Exp+2.0f)/(2.0f * Pi_f32)) * CosTerm;
            } else {
                *Specular = 0.0f;
            }
        } break;
        case BRDF_BlinnPhong:{
            vec3 HalfV = Normalize(-ViewDir + LightDir);
            f32 CosAngle = Dot(HitN, HalfV);
            if(CosAngle > 0.0f){
                f32 Exp = BRDF->Exponent;
                f32 CosTerm = Power(CosAngle, Exp);
                *Specular = CosTerm/CosThetaIncident;
            } else {
                *Specular = 0.0f;
            }
        } break;
        case BRDF_ModifiedBlinnPhong:{
            vec3 HalfV = Normalize(-ViewDir + LightDir);
            f32 CosAngle = Dot(HitN, HalfV);
            if(CosAngle > 0.0f){
                f32 Exp = BRDF->Exponent;
                f32 CosTerm = Power(CosAngle, Exp);
                *Specular = CosTerm;
            } else {
                *Specular = 0.0f;
            }
        } break;
        case BRDF_NormalizedModifiedBlinnPhong:{
            *Diffuse = 1.0f/Pi;
    
            vec3 HalfV = Normalize(-ViewDir + LightDir);
            f32 CosAngle = Dot(HitN, HalfV);
            if(CosAngle > 0.0f){
                f32 Exp = BRDF->Exponent;
                f32 CosTerm = Power(CosAngle, Exp);
                *Specular = ((Exp+8.0f)/(8.0f * Pi_f32)) * CosTerm;
            } else {
                *Specular = 0.0f;
            }
        } break;
        case BRDF_TorranceSparrow:{
            *Diffuse = 1.0f/Pi;
    
            vec3 HalfV = Normalize(-ViewDir + 
                                   LightDir);
            f32 CosAngle = Dot(HitN, HalfV);
             if(CosAngle > 0.0f){
                f32 Exp = BRDF->Exponent;
                f32 CosTerm = Power(CosAngle, Exp);
    
                f32 D = ((Exp+2.0f)/(2.0f*Pi_f32)) * CosTerm;
    
                f32 NW0 = Dot(HitN, -ViewDir);
                f32 W0Wh = Dot(-ViewDir, HalfV);
                f32 G = MinOf(NW0, CosThetaIncident);
                G = (2.0f*G*CosAngle)/W0Wh;
                G = MinOf(1.0f, G);
    
                f32 RefIdx = BRDF->RefractiveIndex;
                f32 R0 = Square(RefIdx-1.0f)/Square(RefIdx+1.0f);
                f32 F = R0 + (1.0f-R0) * Power(1.0f-W0Wh, 5.0f);
    
                *Specular = (D*F*G)/(4.0f*CosThetaIncident*NW0);
            } else {
                *Specular = 0.0f;
            }                               
        } break;
        UnexpectedDefaultCase;
    }
}

static void
CalculateLightContribution(vec3 *DiffuseContribution, 
                           vec3 *SpecularContribution, 
                           brdf *BRDF, vec3 LightIntensity, 
                           vec3 LightDir, vec3 ViewDir, vec3 HitN, 
                           f32 LightDistance, f32 FallOff, f32 InvPDF){

    f32 LDistSqr = Square(LightDistance);
    vec3 LightDirNorm = Normalize(LightDir);
    
    vec3 LIncident = Vec3(LightIntensity.x/LDistSqr,
                          LightIntensity.y/LDistSqr,
                          LightIntensity.z/LDistSqr) * FallOff;
    
    f32 CosThetaIncident = Dot(HitN, 
                               LightDirNorm);
    if(CosThetaIncident > 0.0f){
        f32 Diffuse = 1.0f;
        f32 Specular = 1.0f;
    
        EvaluateBRDF(&Diffuse, &Specular, BRDF, 
                     LightDirNorm, HitN, ViewDir, 
                     CosThetaIncident);

        vec3 CommonMultiplier = LIncident * 
                                CosThetaIncident * 
                                InvPDF;
        *DiffuseContribution  += CommonMultiplier * 
                                 Diffuse;
        *SpecularContribution += CommonMultiplier * 
                                 Specular;
    }
}

inline void
CalculateLighting(vec3 *DiffuseContribution, vec3 *SpecularContribution, 
                  scene *Scene, ray3 Ray, brdf *MaterialBRDF,
                  vec3 HitPosition, vec3 HitNormal, f32 ShadowRayEpsilon, b32 InsideDielectric){
    for(u32 LightIndex=0;
            LightIndex < Scene->LightCount;
            LightIndex++){
        light *Light = Scene->Lights + LightIndex;
    
        // TODO(furkan): Find a name other than LightDir, since
        // it does not represent "direction" at all
        vec3 LightDir = Vec3(0.0f, 0.0f, 0.0f);
        vec3 LightIntensity = Light->Intensity;
        b32 DiscardOnAnyHit = false;
        f32 FallOff = 1.0f;
        f32 LightDistance = 1.0f;
        f32 InvPDF = 1.0f;
        switch(Light->Type){
            case LightType_PointLight:{
                vec3 LightP = Light->Position;
                LightDir = LightP - HitPosition;
                LightDistance = Length(LightDir);
            } break;
            case LightType_AreaLight:{
                vec3 LightP = Light->Position + 
                              Light->EdgeVectors[0]*Random01() + 
                              Light->EdgeVectors[1]*Random01();
                FallOff = Dot(Normalize(LightP-HitPosition), 
                              Light->Normal);
                if(FallOff < 0.0f){
                    // NOTE(furkan): Area lights 
                    // are two-sided emitters
                    FallOff *= -1.0f;
                }
    
                LightDir = LightP - HitPosition;
                LightDistance = Length(LightDir);
            } break;
            case LightType_DirectionalLight:{
                LightDir = Normalize(-Light->Direction);
                DiscardOnAnyHit = true;
            } break;
            case LightType_SpotLight:{
                vec3 LightP = Light->Position;
                LightDir = LightP - HitPosition;
                LightDistance = Length(LightDir);
    
                f32 CosHalfCoverage = Light->CosHalfCoverage;
                f32 CosHalfFallOff  = Light->CosHalfFallOff;
    
                f32 CosTheta = Dot(Normalize(-LightDir),
                                   Normalize(Light->SpotDirection));
    
                if(CosTheta < CosHalfCoverage){
                    FallOff = 0.0f;
                } else if(CosTheta > CosHalfFallOff){
                    FallOff = 1.0f;
                } else {
                    FallOff =       (CosTheta-CosHalfCoverage) / 
                              (CosHalfFallOff-CosHalfCoverage);
                    FallOff = Power(FallOff, 4.0f);
    
                    if(FallOff < 0.0f){
                        FallOff = 0.0f;
                    } else if(FallOff > 1.0f){
                        FallOff = 1.0f;
                    }
                }
            } break;
            case LightType_EntityLight:{
                entity *LE = &Light->Entity;
    
                entity_light_sample S = SampleEntityLight(LE, 
                                                          HitPosition);
    
                LightDir = S.LightDir;
                InvPDF = S.InvPDF;
                LightIntensity = LE->Radiance;
            } break;
            UnexpectedDefaultCase;
        }
    
        if(LightDistance == 0.0f){
            LightDistance = 0.001f;
        }
    
        vec3 LightRayOrigin = HitPosition + (LightDir *
                                             ShadowRayEpsilon);
    
        b32 ReachesLight = true;
        ray_hit LightHit = { 0 };
        LightHit.t = f32_Max;
        if(RayEntityIntersection(&LightHit, 
                                 Ray3(LightRayOrigin, 
                                      LightDir, Ray.T), 
                                 Scene->BVHRoot, 
                                 Scene->Textures,
                                 false, 0.0f)){
            if(Light->Type == LightType_EntityLight){
                if(LightHit.Entity->UniqueEntityID != 
                   Light->Entity.UniqueEntityID){
                    ReachesLight = false;
                }
            } else if(DiscardOnAnyHit || 
               LightHit.t < (1.0f-ShadowRayEpsilon)){
                ReachesLight = false;
            } 
        }
    
        if(ReachesLight && !InsideDielectric){
            CalculateLightContribution(DiffuseContribution, 
                                       SpecularContribution, 
                                       MaterialBRDF, LightIntensity, 
                                       LightDir, Ray.Direction, 
                                       HitNormal, LightDistance, 
                                       FallOff, InvPDF);
        }
    }
    
    if(Scene->UseEnvironmentMap){
        f32 Phi = 2.0f * Pi_f32 * Random01();
        f32 CosPhi = Cos(Phi);
        f32 SinPhi = Sin(Phi);
        
#define EnvMapSampleCosine 1
#if EnvMapSampleUniform
        f32 Theta = Acos(Random01());
#elif EnvMapSampleCosine
        f32 Theta = Asin(Sqrt(Random01()));
#endif
    
        f32 CosTheta = Cos(Theta);
        f32 SinTheta = Sin(Theta);
   
        vec3 U, V;
        OrthonormalBasis(HitNormal, &U, &V);

        vec3 L = Normalize(U*SinTheta*CosPhi + 
                           V*SinTheta*SinPhi + 
                           HitNormal*CosTheta);
    
        ray3 EnvMapRay = Ray3(HitPosition + 
                              (L * ShadowRayEpsilon), L, Ray.T);
    
        ray_hit RayHit = { 0 };
        RayHit.t = f32_Max;
        if(!RayEntityIntersection(&RayHit, EnvMapRay, 
                                  Scene->BVHRoot, 
                                  Scene->Textures, false, 0.0f)){
    
            vec2 UV = Vec2((-Atan(L.z, L.x)+Pi_f32)/(2.0f*Pi_f32),
                           Acos(L.y)/Pi_f32);
            vec3 Intensity = SampleTexture(&Scene->EnvironmentMap,
                                           Vec3(UV, 1.0f), UV);

            f32 CosAngle = Dot(L, HitNormal);
            if(CosAngle  == 0.0f){
                CosAngle  = 0.000001f;
            }

#if EnvMapSampleUniform
            f32 InvPDF = Pi_f32*2.0f;
#elif EnvMapSampleCosine
            f32 InvPDF = Pi_f32/CosAngle;
#endif

            CalculateLightContribution(DiffuseContribution, 
                                       SpecularContribution, 
                                       MaterialBRDF, Intensity, 
                                       L, Ray.Direction, 
                                       HitNormal, 1.0f, 1.0f, 
                                       InvPDF);
        }
    }
}

inline vec3
CalculateMirrorContribution(scene *Scene, u32 RecursionDepth, 
                            material *Material, ray3 Ray, 
                            vec3 HitPosition, vec3 HitNormal, 
                            b32 BackfaceCulling, b32 InsideDielectric, 
                            const epsilon_values *Epsilons, 
                            f32 *RayTravelDistance){
    vec3 Result;
    
    vec3 ReflectionDir = Reflect(-Ray.Direction, HitNormal);
    
    f32 Glossiness = Material->Glossiness;
    if(Glossiness != 0.0f){
        vec3 ReflectionPrime;
        ReflectionPrime.x = Abs(ReflectionDir.x);
        ReflectionPrime.y = Abs(ReflectionDir.y);
        ReflectionPrime.z = Abs(ReflectionDir.z);
    
        if(ReflectionPrime.x < ReflectionPrime.y){
            if(ReflectionPrime.x < ReflectionPrime.z){
                ReflectionPrime = ReflectionDir;
                if(ReflectionPrime.x < 0.0f) 
                    ReflectionPrime.x = -1.0f;
                else
                    ReflectionPrime.x = 1.0f;
            } else {
                ReflectionPrime = ReflectionDir;
                if(ReflectionPrime.z < 0.0f) 
                    ReflectionPrime.z = -1.0f;
                else
                    ReflectionPrime.z = 1.0f;
            }
        } else {
            if(ReflectionPrime.y < ReflectionPrime.z){
                ReflectionPrime = ReflectionDir;
                if(ReflectionPrime.y < 0.0f)
                    ReflectionPrime.y = -1.0f;
                else                    
                    ReflectionPrime.y = 1.0f;
            } else {
                ReflectionPrime = ReflectionDir;
                if(ReflectionPrime.z < 0.0f) 
                    ReflectionPrime.z = -1.0f;
                else
                    ReflectionPrime.z = 1.0f;
            }
        }
        ReflectionPrime = Normalize(ReflectionPrime);
    
        vec3 u = Cross(ReflectionPrime, ReflectionDir);
        vec3 v = Cross(u, ReflectionDir);
    
        ReflectionDir += 2.0f*Glossiness*(u*(Random01() - 0.5f) + 
                                          v*(Random01() - 0.5f));
        ReflectionDir = Normalize(ReflectionDir);
    }
    
    const f32 MirrorEpsilon = 0.000825f;
    vec3 ReflectionOrigin = HitPosition + (ReflectionDir*
                                           MirrorEpsilon);
    ray3 ReflectionRay = Ray3(ReflectionOrigin,
                              ReflectionDir, Ray.T);
    
    Result = Hadamard(Material->MirrorReflectance, 
                      TraceRay(Scene, 
                               ReflectionRay, 
                               RecursionDepth+1, 
                               BackfaceCulling, 
                               InsideDielectric, 
                               Epsilons, 
                               RayTravelDistance));
                               
    return Result;
}

inline vec3
CalculateTransparencyContribution(scene *Scene, material *Material, 
                                  ray3 Ray, vec3 HitPosition, vec3 HitNormal, 
                                  u32 RecursionDepth, b32 BackfaceCulling,  
                                  b32 InsideDielectric, 
                                  const epsilon_values *Epsilons){
    vec3 Result = Vec3(0.0f, 0.0f, 0.0f);
    
    vec3 Transparency = Material->Transparency;
    
    f32 RefractionIndexTo = Material->RefractionIndex;
    f32 RefractionIndexFrom = 1.0f;
    if(InsideDielectric){
        // TODO(furkan): Support nested dielectrics
        RefractionIndexFrom = Material->RefractionIndex;
        RefractionIndexTo = 1.0f;
    }
    
    vec3 N = HitNormal;
    f32 CosTheta = -Dot(Ray.Direction, N);
    if(CosTheta < 0.0f){
        N = -HitNormal;
        CosTheta = -Dot(Ray.Direction, N);
    }
    
    f32 RefRatio = RefractionIndexFrom / RefractionIndexTo;
    f32 Delta = 1.0f - Square(RefRatio) * (1.0f-Square(CosTheta));
    
    f32 Reflectivity;
    if(Delta <= 0.0f){
        Reflectivity = 1.0f;
    } else {
        f32 CosPhi = Sqrt(Delta);
        f32 CosAir = (InsideDielectric) ? CosPhi : 
                                          CosTheta;
        Assert(CosAir >= 0.0f);
        f32 InverseCosAir = 1.0f - CosAir;
        f32 RZero = Square((RefractionIndexFrom-RefractionIndexTo) /
                           (RefractionIndexFrom+RefractionIndexTo));
        Reflectivity = RZero + 
                       (1.0f-RZero)*Power(InverseCosAir, 5);
        Assert(Reflectivity >= -0.0001f && Reflectivity <= 1.0001f);
    
        vec3 T = (Ray.Direction + N*CosTheta)*RefRatio - N*CosPhi;
        T = Normalize(T);
    
        const f32 RefractionEpsilon = 0.000100f;
        vec3 RefractionOrigin = HitPosition + 
                                (T*RefractionEpsilon);
        ray3 RefractionRay = Ray3(RefractionOrigin, T, Ray.T);
        
        f32  TravelDistance = 0;
        vec3 RefractionContribution = TraceRay(Scene, 
                                               RefractionRay, 
                                               RecursionDepth+1,
                                               BackfaceCulling,
                                              !InsideDielectric,
                                               Epsilons, 
                                               &TravelDistance);
        vec3 Attenuation;
        if(!InsideDielectric){
            Attenuation = Vec3(
                           Exp((Transparency.r-1.0f)*
                               TravelDistance),
                           Exp((Transparency.g-1.0f)*
                               TravelDistance),
                           Exp((Transparency.b-1.0f)*
                               TravelDistance));
        } else {
            // NOTE(furkan): Do not apply attenuation if ray
            // is leaving the surrounding transparent object
            Attenuation = Vec3(1.0f, 1.0f, 1.0f);
        }
    
        Result += (1.0f-Reflectivity) * 
                  Hadamard(Attenuation, 
                           RefractionContribution);
    }
    
    const f32 ReflectionEpsilon = 0.000100f;
    vec3 ReflectionDir = Reflect(-Ray.Direction, N);
    vec3 ReflectionOrigin = HitPosition + (ReflectionDir *
                           ReflectionEpsilon);
    ray3 ReflectionRay = Ray3(ReflectionOrigin,
                              ReflectionDir, Ray.T);
    f32 TravelDistance = 0;
    vec3 ReflectionContribution = TraceRay(Scene, 
                                           ReflectionRay, 
                                           RecursionDepth+1, 
                                           BackfaceCulling, 
                                           InsideDielectric, 
                                           Epsilons, 
                                           &TravelDistance);
    
    vec3 Attenuation = Vec3(1.0f, 1.0f, 1.0f);
    if(InsideDielectric){
        Attenuation = Vec3(Exp((Transparency.r-1.0f)*
                                    TravelDistance),
                                Exp((Transparency.g-1.0f)*
                                    TravelDistance),
                                Exp((Transparency.b-1.0f)*
                                    TravelDistance));
    }
    
    Result += Reflectivity * Hadamard(Attenuation,
                                      ReflectionContribution);
                                      
    return Result;
}

static vec3
TraceRay(scene *Scene, ray3 Ray, u32 RecursionDepth, 
         b32 BackfaceCulling, b32 InsideDielectric, 
         const epsilon_values *Epsilons, f32 *RayTravelDistance){

    vec3 Result = Vec3(0.0f, 0.0f, 0.0f);
    if(RecursionDepth <= Scene->MaxRecursionDepth){
        ray_hit Hit = { 0 };
        Hit.t = f32_Max;
        b32 RayIntersected = RayEntityIntersection(&Hit, Ray, 
                                                   Scene->BVHRoot, 
                                                   Scene->Textures, 
                                                   BackfaceCulling,
                                                   Epsilons->Intersection);
        if(RayIntersected){
            *RayTravelDistance += Hit.t;

            entity *Entity = Hit.Entity;
            if(Entity->IsEmitter){
                Result = Entity->Radiance;
            } else {
                material *Material = Scene->Materials + Entity->MaterialIndex;

                brdf *MaterialBRDF = &Material->BRDF;                
                vec3 DiffuseColor = Material->DiffuseReflectance;
                vec3 SpecularColor = Material->SpecularReflectance;
                
                vec3 DiffuseContribution = Vec3(0.0f, 0.0f, 0.0f);
                vec3 SpecularContribution = Vec3(0.0f, 0.0f, 0.0f);
                
                vec3 HitPosition = Hit.P;
                vec3 HitNormal = Hit.Normal;
                
                b32 IgnoreAmbient = false;
                if(Entity->TextureIndex){
                    texture *Texture = Scene->Textures + (Entity->TextureIndex-1);
                    vec3 TexSample = SampleTexture(Texture, 
                                                   HitPosition, 
                                                   Hit.UV);
                    switch(Texture->BlendMode){
                        case TextureBlendMode_ReplaceAll:{
                            DiffuseColor = TexSample;
                            DiffuseContribution = Vec3(1.0f, 1.0, 1.0f);
                            SpecularContribution = Vec3(0.0f, 0.0f, 0.0f);
                            IgnoreAmbient = true;
                        } break;
                        case TextureBlendMode_ReplaceDiffuse:{
                            DiffuseColor = TexSample;
                        } break;
                        case TextureBlendMode_BlendDiffuse:{
                            DiffuseColor += TexSample;
                            DiffuseColor *= 0.5f;
                        } break;
                    }
                }

                CalculateLighting(&DiffuseContribution, 
                                  &SpecularContribution, 
                                  Scene, Ray, MaterialBRDF,
                                  HitPosition, HitNormal, 
                                  Epsilons->ShadowRay, 
                                  InsideDielectric);

                if(Scene->PathTrace){
                    f32 Theta;
                    if(Scene->ImportanceSampling){
                        Theta = Asin(Sqrt(Random01()));
                    } else {
                        Theta = Acos(Random01());
                    }
                    f32 CosTheta = Cos(Theta);
                    f32 SinTheta = Sin(Theta);

                    f32 Phi = 2.0f * Pi_f32 * Random01();
                    f32 CosPhi = Cos(Phi);
                    f32 SinPhi = Sin(Phi);

                    vec3 U, V;
                    OrthonormalBasis(HitNormal, &U, &V);

                    vec3 S = Normalize(U*SinTheta*CosPhi + 
                                       V*SinTheta*SinPhi + 
                                       HitNormal*CosTheta);
            
                    ray3 ScatterRay = Ray3(HitPosition + 
                                           (S * Epsilons->ShadowRay), 
                                           S, Ray.T);
                    
                    f32 ScatterTravel = 0.0f;
                    vec3 ScatterColor = TraceRay(Scene, ScatterRay, 
                                                 RecursionDepth+1, 
                                                 BackfaceCulling, 
                                                 InsideDielectric, 
                                                 Epsilons, 
                                                 &ScatterTravel);

                    f32 InvPDF;
                    if(Scene->ImportanceSampling){
                        f32 CosScatter = Dot(S, HitNormal);
                        if(CosScatter == 0.0f){
                            CosScatter = 0.000001f;
                        }

                        InvPDF = Pi_f32/CosScatter;
                    } else {
                        InvPDF = Pi_f32*2.0f;
                    }

                    CalculateLightContribution(&DiffuseContribution, 
                                               &SpecularContribution, 
                                                MaterialBRDF, 
                                                ScatterColor, 
                                                ScatterRay.Direction, 
                                                Ray.Direction, 
                                                HitNormal, 
                                                1.0f, 1.0f, 
                                                InvPDF);
                }

                Result += Hadamard(DiffuseColor, 
                                   DiffuseContribution);
                Result += Hadamard(SpecularColor, 
                                   SpecularContribution);

                vec3 Transparency = Material->Transparency;
                b32 IsTransparent = (*((u32 *)&Transparency.x)) | 
                                    (*((u32 *)&Transparency.y)) | 
                                    (*((u32 *)&Transparency.z));

                if((!InsideDielectric || !IsTransparent) && !IgnoreAmbient){
                    vec3 Ambient = Hadamard(Material->AmbientReflectance, 
                                            Scene->AmbientLight);
                    Result += Ambient;
                }

                vec3 MirrorRef = Material->MirrorReflectance;
                b32 IsMirror = (*((u32 *)&MirrorRef.x)) | 
                               (*((u32 *)&MirrorRef.y)) | 
                               (*((u32 *)&MirrorRef.z));
                if(IsMirror){
                    Result += CalculateMirrorContribution(Scene, 
                                                          RecursionDepth, 
                                                          Material, Ray, 
                                                          HitPosition, 
                                                          HitNormal, 
                                                          BackfaceCulling, 
                                                          InsideDielectric, 
                                                          Epsilons, 
                                                          RayTravelDistance);
                }

                if(IsTransparent){
                    Result += CalculateTransparencyContribution(Scene, 
                                                                Material, 
                                                                Ray, 
                                                                HitPosition, 
                                                                HitNormal, 
                                                                RecursionDepth, 
                                                                BackfaceCulling,  
                                                                InsideDielectric, 
                                                                Epsilons);
                }
            }
        }
    }
    
    return Result;
}

inline vec2
GetSampleCoord(u32 SampleX, u32 SampleY, u32 Row, u32 Col, 
               f32 OneOverSampleCountSqrt){
    vec2 Result;

#if 0
    vec2 SampleOffset = Vec2(Random01(), Random01());
#else
    vec2 SampleOffset = Vec2(0.5f, 0.5f);
#endif

    SampleOffset *= OneOverSampleCountSqrt;
    SampleOffset += Vec2(SampleX*OneOverSampleCountSqrt, 
                         SampleY*OneOverSampleCountSqrt);

    Result = Vec2(Col+SampleOffset.x, Row+SampleOffset.y);

    return Result;
}

inline ray3
GetRayForPixel(vec3 BaseRayDirection, vec3 CameraP, 
               vec3 CamAxisX, vec3 CamAxisY, 
               vec3 PixelStepX, vec3 PixelStepY, 
               vec2 SampleCoord, f32 ApertureSize){
    ray3 Result;
    
    vec2 LensP = Vec2(Random01() - 0.5f, 
                      Random01() - 0.5f) * ApertureSize;
    
    vec3 LensOffsetX = CamAxisX * LensP.x;
    vec3 LensOffsetY = CamAxisY * LensP.y;
    
    vec3 RayDirection = Normalize(BaseRayDirection + 
                                  SampleCoord.x*PixelStepX -
                                  SampleCoord.y*PixelStepY -
                                  LensOffsetX - LensOffsetY);
    Result = Ray3(CameraP + 
                  LensOffsetX + LensOffsetY, 
                  RayDirection, 
                  Random01());

    return Result;
}

inline vec3
SampleBackgroundColor(vec3 RayDirection, vec3 BackgroundColor, 
                      texture *BackgroundTexture, texture *EnvironmentMap, 
                      b32 UseBackgroundTexture, b32 UseEnvironmentMap, 
                      s32 OneMinusWidth, s32 OneMinusHeight, 
                      u32 Row, u32 Col){
    vec3 Result;

    if(UseEnvironmentMap){
        f32 Theta =  Acos(RayDirection.y);
        f32 Phi   =  Atan(RayDirection.z, 
                          RayDirection.x);
    
        vec2 UV = Vec2((-Phi+Pi_f32)/(2.0f*Pi_f32),
                       Theta/Pi_f32);

        Result = SampleTexture(EnvironmentMap, Vec3(UV, 1.0f), UV);
    } else if(UseBackgroundTexture){
        vec2 UV = Vec2((f32)Col/(f32)OneMinusWidth, 
                       (f32)Row/(f32)OneMinusHeight);

        Result = SampleTexture(BackgroundTexture, Vec3(UV, 1.0f), UV);
    } else {
        Result = BackgroundColor;
    }

    return Result;
}

inline void
ComputeSampleWeight(vec3 Color, vec2 SampleCoord, pixel *Pixels, 
                    u32 PixelIndex, u32 Width, u32 Row, u32 Col, 
                    s32 OneMinusWidth, s32 OneMinusHeight, 
                    s32 GaussianKernelSize, f32 OneOverTwoPiSigmaSqr, 
                    f32 NegativeOneOverTwoSigmaSqr, filter_type FilterType){
    switch(FilterType){
        case FilterType_Box: {
            pixel *Pixel = Pixels + PixelIndex;
            PlatformAtomicAddF32(&Pixel->Color.r    , Color.r);
            PlatformAtomicAddF32(&Pixel->Color.g    , Color.g);
            PlatformAtomicAddF32(&Pixel->Color.b    , Color.b);
            PlatformAtomicAddF32(&Pixel->TotalWeight, 1.0f);
        } break;
        case FilterType_Gaussian: {
            s32 GxMin = MaxOf((s32)Col-GaussianKernelSize, 0);
            s32 GxMax = MinOf((s32)Col+GaussianKernelSize, OneMinusWidth);
    
            s32 GyMin = MaxOf((s32)Row-GaussianKernelSize, 0);
            s32 GyMax = MinOf((s32)Row+GaussianKernelSize, OneMinusHeight);
    
            for(s32 Gy = GyMin; Gy <= GyMax; Gy++){
                for(s32 Gx = GxMin; Gx <= GxMax; Gx++){
                    vec2 PixelCoord = Vec2(Gx+0.5f, Gy+0.5f);
                    vec2 DistanceFromPixelCenter = SampleCoord - PixelCoord;

                    f32 DistanceSqr = Square(DistanceFromPixelCenter.x) + 
                                      Square(DistanceFromPixelCenter.y);
                    f32 GaussTerm = OneOverTwoPiSigmaSqr * 
                                    Exp(NegativeOneOverTwoSigmaSqr * 
                                        DistanceSqr);
    
                    u32 GaussianIndex = Gy*Width + Gx;
                    pixel *GaussPixel = Pixels + GaussianIndex;
                    vec3 GaussColor   = Color * GaussTerm;

                    PlatformAtomicAddF32(&GaussPixel->Color.r, GaussColor.r);
                    PlatformAtomicAddF32(&GaussPixel->Color.g, GaussColor.g);
                    PlatformAtomicAddF32(&GaussPixel->Color.b, GaussColor.b);
                    PlatformAtomicAddF32(&GaussPixel->TotalWeight, GaussTerm);
                }
            }
        } break;
        UnexpectedDefaultCase;
    }
}

static void
ProduceImageInLinearSpace(pixel *Pixels, 
                          scene *Scene, 
                          camera *Camera, 
                          epsilon_values *Epsilons, 
                          scissor Scissor, 
                          filter_type FilterType, 
                          b32 BackfaceCulling){
            
    vec3 CamGaze = Camera->Gaze;
    if(Camera->GazeIsPoint){
        CamGaze = Camera->Gaze - Camera->Position;
    }

    vec3 CamGazeNormalized = Normalize(CamGaze);

    vec3 CamAxisZ = -CamGazeNormalized;
    vec3 CamAxisX, CamAxisY;
    if(Camera->IsLeftHanded){
        CamAxisX = Normalize(Cross(Normalize(Camera->Up), 
                                   CamGazeNormalized));
        CamAxisY = Cross(CamAxisX, CamAxisZ);   
    } else {
        CamAxisX = Normalize(Cross(CamGazeNormalized, 
                                   Normalize(Camera->Up)));
        CamAxisY = Cross(CamAxisZ, CamAxisX);   
    }

    f32 FDistOverNDist = 1.0f;
    if(Camera->FocusDistance != 0.0f){
        FDistOverNDist = Camera->FocusDistance/Camera->NearDistance;
    }

    f32 ApertureSize = Camera->ApertureSize;

    u32 ImageWidth = Camera->Resolution.x;
    u32 ImageHeight = Camera->Resolution.y;

    vec4 NearPlane = Camera->NearPlane;
    if(Camera->Type == CameraType_Simple){
        f32 NearPlaneHeightHalf = Camera->NearDistance * 
                                  Tan(DegreeToRadian(Camera->FovY/2.0f));
        f32 AspectRatio = (f32)ImageWidth/(f32)ImageHeight;
        f32 NearPlaneWidthHalf = AspectRatio*NearPlaneHeightHalf;

        vec3 NearPlaneCenter = Camera->NearDistance * CamGazeNormalized;

        NearPlane.t = +NearPlaneHeightHalf;
        NearPlane.b = -NearPlaneHeightHalf;
        NearPlane.r = +NearPlaneWidthHalf;
        NearPlane.l = -NearPlaneWidthHalf;
    }



    vec2 ImagePlaneSize = Vec2(NearPlane.r - NearPlane.l,
                               NearPlane.t - NearPlane.b);
    vec2 PixelSize      = Vec2(ImagePlaneSize.x/(f32)ImageWidth, 
                               ImagePlaneSize.y/(f32)ImageHeight);
    vec3 PixelStepX = CamAxisX*PixelSize.x*FDistOverNDist; 
    vec3 PixelStepY = CamAxisY*PixelSize.y*FDistOverNDist;

    vec3 ImagePlaneTopLeft = Camera->Position + 
                             Camera->NearDistance * (CamGazeNormalized) + 
                             NearPlane.l  * CamAxisX + 
                             NearPlane.t  * CamAxisY;
    vec3 BaseRayDirection = (ImagePlaneTopLeft-Camera->Position) *
                            FDistOverNDist;

    u32 RowMin = Scissor.Min.y;
    u32 RowMax = MinOf(Scissor.Max.y, ImageHeight);

    u32 ColMin = Scissor.Min.x;
    u32 ColMax = MinOf(Scissor.Max.x, ImageWidth);

    u32 SampleCount = Camera->SampleCount;
    u32 SampleCountSqrt = (u32)Sqrt((f32)SampleCount);
    if(Square(SampleCountSqrt) != SampleCount){
        Warning("SampleCount is not a perfect square. This may cause errors");
    }
    f32 OneOverSampleCountSqrt = 1.0f/SampleCountSqrt;

    // NOTE(furkan): You can use GaussianRadius to change filter size
    // TODO(furkan): Take it as a program argument
    f32 GaussianRadius = 1.5f;

    f32 Sigma = GaussianRadius/3.0f;
    f32 TwoSigmaSqr = 2.0f * Square(Sigma);
    f32 NegativeOneOverTwoSigmaSqr = -1.0f / TwoSigmaSqr;
    f32 OneOverTwoPiSigmaSqr = 1.0f/(TwoSigmaSqr * (f32)Pi);

    s32 GaussianKernelSize = CeilAsS32(GaussianRadius-0.5f);
    
    s32 OneMinusWidth = ImageWidth-1;
    s32 OneMinusHeight = ImageHeight-1;

    for(u32 Row=RowMin; Row < RowMax; Row++){
        for(u32 Col=ColMin; Col < ColMax; Col++){

            u32 PixelIndex = Row * ImageWidth + Col;
            for(u32 SampleY=0; SampleY < SampleCountSqrt; SampleY++){
                for(u32 SampleX=0; SampleX < SampleCountSqrt; SampleX++){
                    vec2 SampleCoord = GetSampleCoord(SampleX, SampleY, 
                                                      Row, Col, 
                                                      OneOverSampleCountSqrt);
                
                    ray3 Ray = GetRayForPixel(BaseRayDirection, 
                                              Camera->Position, 
                                              CamAxisX, CamAxisY, 
                                              PixelStepX, PixelStepY, 
                                              SampleCoord, ApertureSize);

                    f32 RayTravelDistance = 0.0f;
                    vec3 SampleColor = TraceRay(Scene, Ray, 0, 
                                                BackfaceCulling, 
                                                false, Epsilons, 
                                                &RayTravelDistance);

                    if(RayTravelDistance == 0.0f){
                        SampleColor = SampleBackgroundColor(
                                                 Ray.Direction, 
                                                 Scene->BackgroundColor, 
                                                 &Scene->BackgroundTexture, 
                                                 &Scene->EnvironmentMap, 
                                                 Scene->UseBackgroundTexture, 
                                                 Scene->UseEnvironmentMap, 
                                                 OneMinusWidth, OneMinusHeight,
                                                 Row, Col);
                    }

                    ComputeSampleWeight(SampleColor, SampleCoord, Pixels, 
                                        PixelIndex, ImageWidth, Row, Col, 
                                        OneMinusWidth, OneMinusHeight, 
                                        GaussianKernelSize, 
                                        OneOverTwoPiSigmaSqr, 
                                        NegativeOneOverTwoSigmaSqr, 
                                        FilterType);
                }
            }
        }
    }
}

typedef struct {
    pixel *Pixels;
    scissor Scissor;
    scene *Scene;
    camera *Camera;
    epsilon_values *Epsilons;
    filter_type FilterType;
    b32 BackfaceCulling;
} produce_image_params;

// NOTE(furkan): Thread procedures. They just call
// corresponding functions with their thread-specific
// parameters.

// TODO(furkan): Instead of using this kind of thread
// procedures, consider implementing a task queue and 
// create a set of worker threads. That might be a little
// bit more flexible and/or efficient.
PlatformThreadProc(ProduceImageInLinearSpaceMT){
    produce_image_params *Params = (produce_image_params *)Parameters;
    ProduceImageInLinearSpace(Params->Pixels, Params->Scene, 
                              Params->Camera, Params->Epsilons, 
                              Params->Scissor, Params->FilterType, 
                              Params->BackfaceCulling);
    return 0;
}

static b32
OutputPixelsToEXR(char *FilePath, pixel *Pixels, vec2u Size){
    b32 Success = true;

    Imf::Rgba *EXRPixels = 0;
    u32 PixelCount = Size.x * Size.y;
    if(MemoryAllocate((void **)&EXRPixels, 
                      sizeof(Imf::Rgba) * PixelCount)){
        for(u32 PIndex = 0;
                PIndex < PixelCount;
                PIndex++){
            vec3 C = Pixels[PIndex].Color;

            f32 R = C.r*255.0f;
            f32 G = C.g*255.0f;
            f32 B = C.b*255.0f;

            EXRPixels[PIndex] = {R, G, B, 1.0f};
        }

        Imf::RgbaOutputFile EXRFile(FilePath, Size.x, Size.y, Imf::WRITE_RGB);
        EXRFile.setFrameBuffer (EXRPixels, 1, Size.x);
        EXRFile.writePixels(Size.y);

        if(!MemoryFree((void **)&EXRPixels)){
            Error("MemoryFree failed");
        }
    } else {
        Error("MemoryAllocate failed");
    }

    return Success;
}

static b32
OutputPixelsToPNG(char *FilePath, pixel *Pixels, vec2u Size){
    b32 Success = true;

    u8 *Pixels255 = 0;
    u32 PixelCount = Size.x * Size.y;
    if(MemoryAllocate((void **)&Pixels255, 
                      sizeof(u8) * 3 * PixelCount)){
        for(u32 PIndex = 0;
                PIndex < PixelCount;
                PIndex++){
            vec3 C = Pixels[PIndex].Color;

            u8 R = (u8)(Clamp01(C.r)*255.0f);
            u8 G = (u8)(Clamp01(C.g)*255.0f);
            u8 B = (u8)(Clamp01(C.b)*255.0f);

            Pixels255[PIndex*3 + 0] = R;
            Pixels255[PIndex*3 + 1] = G;
            Pixels255[PIndex*3 + 2] = B;
        }

        if(!stbi_write_png(FilePath, 
                           Size.x, Size.y, 3, 
                           Pixels255, 
                           Size.x*3)){
            Success = false;
            Error("stbi_write_png failed");
        }

        if(!MemoryFree((void **)&Pixels255)){
            Error("MemoryFree failed");
        }
    } else {
        Error("MemoryAllocate failed");
    }

    return Success;
}

static b32
OutputPixelsToBMP(char *FilePath, pixel *Pixels, vec2u Size){
    b32 Success = true;

    u8 *Pixels255 = 0;
    u32 PixelCount = Size.x * Size.y;
    if(MemoryAllocate((void **)&Pixels255, 
                      sizeof(u8) * 3 * PixelCount)){
        for(u32 PIndex = 0;
                PIndex < PixelCount;
                PIndex++){
            vec3 C = Pixels[PIndex].Color;

            u8 R = (u8)(Clamp01(C.r)*255.0f);
            u8 G = (u8)(Clamp01(C.g)*255.0f);
            u8 B = (u8)(Clamp01(C.b)*255.0f);

            Pixels255[PIndex*3 + 0] = R;
            Pixels255[PIndex*3 + 1] = G;
            Pixels255[PIndex*3 + 2] = B;
        }

        if(!stbi_write_bmp(FilePath, 
                           Size.x, Size.y, 3, 
                           Pixels255)){
            Success = false;
            Error("stbi_write_bmp failed");
        }

        if(!MemoryFree((void **)&Pixels255)){
            Error("MemoryFree failed");
        }
    } else {
        Error("MemoryAllocate failed");
    }

    return Success;
}

static b32
OutputPixelsToTGA(char *FilePath, pixel *Pixels, vec2u Size){
    b32 Success = true;

    u8 *Pixels255 = 0;
    u32 PixelCount = Size.x * Size.y;
    if(MemoryAllocate((void **)&Pixels255, 
                      sizeof(u8) * 3 * PixelCount)){
        for(u32 PIndex = 0;
                PIndex < PixelCount;
                PIndex++){
            vec3 C = Pixels[PIndex].Color;

            u8 R = (u8)(Clamp01(C.r)*255.0f);
            u8 G = (u8)(Clamp01(C.g)*255.0f);
            u8 B = (u8)(Clamp01(C.b)*255.0f);

            Pixels255[PIndex*3 + 0] = R;
            Pixels255[PIndex*3 + 1] = G;
            Pixels255[PIndex*3 + 2] = B;
        }

        if(!stbi_write_tga(FilePath, 
                           Size.x, Size.y, 3, 
                           Pixels255)){
            Success = false;
            Error("stbi_write_tga failed");
        }

        if(!MemoryFree((void **)&Pixels255)){
            Error("MemoryFree failed");
        }
    } else {
        Error("MemoryAllocate failed");
    }

    return Success;
}

static b32
OutputPixelsToJPG(char *FilePath, pixel *Pixels, vec2u Size){
    b32 Success = true;

    u8 *Pixels255 = 0;
    u32 PixelCount = Size.x * Size.y;
    if(MemoryAllocate((void **)&Pixels255, 
                      sizeof(u8) * 3 * PixelCount)){
        for(u32 PIndex = 0;
                PIndex < PixelCount;
                PIndex++){
            vec3 C = Pixels[PIndex].Color;

            u8 R = (u8)(Clamp01(C.r)*255.0f);
            u8 G = (u8)(Clamp01(C.g)*255.0f);
            u8 B = (u8)(Clamp01(C.b)*255.0f);

            Pixels255[PIndex*3 + 0] = R;
            Pixels255[PIndex*3 + 1] = G;
            Pixels255[PIndex*3 + 2] = B;
        }

        // NOTE(furkan): Quality must be between 1 and 100.
        // 100 means maximum quality.
        s32 Quality = 100;
        if(!stbi_write_jpg(FilePath, 
                           Size.x, Size.y, 3, 
                           Pixels255, 
                           Quality)){
            Success = false;
            Error("stbi_write_jpg failed");
        }

        if(!MemoryFree((void **)&Pixels255)){
            Error("MemoryFree failed");
        }
    } else {
        Error("MemoryAllocate failed");
    }

    return Success;
}
static b32
OutputPixelsToFile(char *FilePath, pixel *Pixels, vec2u Size, 
                   tonemap *Tonemap, f32 InvGamma, b32 sRGB, 
                   file_format SaveFormat, b32 SaveEXR){

    b32 Success = false;

    u32 PixelCount = Size.x * Size.y;

    f32 *Luminances = 0;
    b32 Tonemapped = (Tonemap->Operator != TonemapOperator_None) &&
                     (SaveFormat != FileFormat_EXR);
    if(Tonemapped){
        Assert(Tonemap->Operator == TonemapOperator_Photographic);

        if(!MemoryAllocate((void **)&Luminances, 
                           sizeof(f32) * PixelCount)){
            Error("Luminances could not be allocated");
        }
    }

    // NOTE(furkan): Filtering
    f32 LogAverageLuminance = 0.0f;
    for(u32 PIndex = 0;
            PIndex < PixelCount;
            PIndex++){

        vec3 C = Pixels[PIndex].Color;
        C.r   /= Pixels[PIndex].TotalWeight;
        C.g   /= Pixels[PIndex].TotalWeight;
        C.b   /= Pixels[PIndex].TotalWeight;

        Pixels[PIndex].TotalWeight = 1.0f;
        Pixels[PIndex].Color = C;

        if(Tonemapped){
            Assert(Luminances);

            f32 Leps = 0.00001f;
            f32 L = Luminance(C);
            Luminances[PIndex] = L;

            LogAverageLuminance += LogBaseE(L + Leps);
        }
    }

    if(SaveFormat != FileFormat_EXR && SaveEXR){
        u32 FilePathLen = (u32)StringLength(FilePath);
        char EXRFilePath[256] = {0};
        if(MemoryCopy(EXRFilePath, sizeof(EXRFilePath), 
                      FilePath   , sizeof(char)*FilePathLen)){
            u32 DotIndex;
            for(DotIndex=FilePathLen-1; true; DotIndex--){
                if(EXRFilePath[DotIndex] == '.'){
                    break;
                }

                if(DotIndex == 0){
                    break;
                }
            }

            if(MemoryCopy(EXRFilePath+DotIndex+1, FilePathLen-DotIndex-1,
                          "exr", 3)){
                OutputPixelsToEXR(EXRFilePath, Pixels, Size);
            }
        }
    }

    // NOTE(furkan): Tonemapping
    if(Tonemapped){
        LogAverageLuminance = Exp(LogAverageLuminance/(f32)PixelCount);

        f32 Factor = Tonemap->KeyValue / LogAverageLuminance;

        Assert(Luminances);

        qsort(Luminances, PixelCount, sizeof(f32), CompareF32);

        u32 LIndex = RoundAsU32((1.0f - 
                                 Tonemap->BurnoutPercentage*0.01f) *
                                (PixelCount-1));
        f32 LwhiteSqr = Luminances[LIndex] * Factor;
        LwhiteSqr = Square(LwhiteSqr);

        f32 Saturation = Tonemap->Saturation;

        for(u32 PIndex = 0;
                PIndex < PixelCount;
                PIndex++){

            vec3 C = Pixels[PIndex].Color;

            f32 L = Luminance(C);
            if(L != 0.0f){
                f32 Ls = L*Factor;
                f32 Ld = ( Ls*( 1.0f+( Ls/LwhiteSqr ) ) )/(1.0f+Ls);

                f32 PowR = Power(C.r/L, Saturation);
                f32 PowG = Power(C.g/L, Saturation);
                f32 PowB = Power(C.b/L, Saturation);
                
                Pixels[PIndex].Color = Vec3(Clamp01(PowR*Ld), 
                                            Clamp01(PowG*Ld),
                                            Clamp01(PowB*Ld));
            }
        }
    }

    // NOTE(furkan): Gamma-correction
    if(SaveFormat != FileFormat_EXR){
        if(sRGB){
            for(u32 PIndex = 0;
                    PIndex < PixelCount;
                    PIndex++){

                vec3 C = Pixels[PIndex].Color;

                f32 R = C.r;
                f32 G = C.g;
                f32 B = C.b;

                if(R > 0.0031308f){
                    R = 1.055f*Power(R, InvGamma) - 0.055f;
                } else {
                    R *= 12.92f;
                }

                if(G > 0.0031308f){
                    G = 1.055f*Power(G, InvGamma) - 0.055f;
                } else {
                    G *= 12.92f;
                }

                if(B > 0.0031308f){
                    B = 1.055f*Power(B, InvGamma) - 0.055f;
                } else {
                    B *= 12.92f;
                }
                
                Pixels[PIndex].Color = Vec3(R, G, B);
            }           
        } else {
            for(u32 PIndex = 0;
                    PIndex < PixelCount;
                    PIndex++){
                vec3 C = Pixels[PIndex].Color;

                f32 R = Power(C.r, InvGamma);
                f32 G = Power(C.g, InvGamma);
                f32 B = Power(C.b, InvGamma);

                Pixels[PIndex].Color = Vec3(R, G, B);
            }
        }
    }

    switch(SaveFormat){
        case FileFormat_EXR:{
            Success = OutputPixelsToEXR(FilePath, Pixels, Size);
        } break;
        case FileFormat_PNG:{
            Success = OutputPixelsToPNG(FilePath, Pixels, Size);
        } break;
        case FileFormat_BMP:{
            Success = OutputPixelsToBMP(FilePath, Pixels, Size);
        } break;
        case FileFormat_TGA:{
            Success = OutputPixelsToTGA(FilePath, Pixels, Size);
        } break;
        case FileFormat_JPG:{
            Success = OutputPixelsToJPG(FilePath, Pixels, Size);
        } break;
        UnexpectedDefaultCase;
    }

    return Success;
}

static b32
RenderScene(scene *Scene, camera *Camera, 
            epsilon_values *Epsilons, 
            u32 ThreadCount, b32 BackfaceCulling){
    b32 Success = true;

    u32 ImageWidth  = Camera->Resolution.x;
    u32 ImageHeight = Camera->Resolution.y;
    u32 PixelCount = ImageWidth * ImageHeight;
    
    pixel *Pixels = 0;
    if(MemoryAllocate((void **)&Pixels, 
                      sizeof(pixel) * PixelCount)){
        ZeroMemory(Pixels, sizeof(pixel) * PixelCount);
    
        thread_handle *ThreadHandles;
        produce_image_params *ThreadParams;
        if(PlatformMemoryAllocate((void **)&ThreadHandles, 
                                  sizeof(thread_handle) *
                                  ThreadCount) &&
           PlatformMemoryAllocate((void **)&ThreadParams, 
                                  sizeof(produce_image_params) *
                                  ThreadCount)){
    
            u32 MinScanlinePerThread = ImageHeight/ThreadCount;
            u32 ResidualScanline = ImageHeight - 
                                   MinScanlinePerThread*ThreadCount;
    
            u32 ConsumedScanline = 0;
            for(u32 ThreadIndex=0; 
                    ThreadIndex<ThreadCount; 
                    ThreadIndex++){
                u32 Scanline = MinScanlinePerThread;
                if(ResidualScanline){
                    Scanline++;
                    ResidualScanline--;
                }
    
                produce_image_params *Params = ThreadParams + 
                                               ThreadIndex;
                Params->Scissor = { Vec2u(ImageWidth, 
                                          ConsumedScanline+Scanline), 
                                    Vec2u(0, 
                                          ConsumedScanline)};
                Params->Pixels = Pixels;
                Params->Scene = Scene;
                Params->Camera = Camera;
                Params->Epsilons = Epsilons;
                Params->FilterType = FilterType_Box;
                Params->BackfaceCulling = BackfaceCulling;
    
                u32 ThreadID;
                if(PlatformCreateThread(&ThreadHandles[ThreadIndex], 
                                        ProduceImageInLinearSpaceMT, 
                                        Params, 
                                        &ThreadID, 
                                        0)){
                } else {
                    Error("An error occured while creating threads");
                }
    
                ConsumedScanline += Scanline;
            }
    
            if(PlatformWaitForThreads(ThreadCount, ThreadHandles, 
                                      true, true, 0)){
            } else {
                Error("An error occured while waiting threads");
            }
    
            for(u32 ThreadIndex=0; 
                    ThreadIndex<ThreadCount; 
                    ThreadIndex++){
                PlatformCloseThreadHandle(ThreadHandles[ThreadIndex]);
            }
        } else {
            Error("Failed to allocate space for thread handles.");
            Success = false;
        }
    
        PlatformMemoryFree((void **)&ThreadHandles);
        PlatformMemoryFree((void **)&ThreadParams);

        if(Success){
            if(OutputPixelsToFile(Camera->ImageName, Pixels, 
                                  Vec2u(ImageWidth, ImageHeight), 
                                  &Camera->Tonemap, 
                                  Camera->InvGamma, 
                                  Camera->GammaCorrection_sRGB, 
                                  Camera->SaveFormat, 
                                  Camera->SaveEXR)){
            } else {
                Success = false;
            }       
        }           
    } else {
        Success = false;
    }
    
    PlatformMemoryFree((void **)&Pixels);

    return Success;
}

typedef struct {
    char *SceneFilePath;
    u32 ThreadCount;
    b32 BackfaceCulling;
    b32 Multithreaded;
} preferences;

static b32
ParseArguments(preferences *Prefs, char **Args, u32 ArgCount){
    b32 Result = false;

    u32 ArgIndex;
    for(ArgIndex=1; ArgIndex<ArgCount; ArgIndex++){
        if(!strncmp(Args[ArgIndex], "-s", 2) || 
           !strncmp(Args[ArgIndex], "--scene", 7)){
            if((ArgIndex+1) < ArgCount){
                ArgIndex++;
                Prefs->SceneFilePath = Args[ArgIndex];
                Result = true;
            } else {
                Error("Scene file is not specified.");
            }
        } else if (!strncmp(Args[ArgIndex], "-c", 2) || 
                   !strncmp(Args[ArgIndex], "--cull", 6)){
            Prefs->BackfaceCulling = true;
        } else if (!strncmp(Args[ArgIndex], "-m", 2) || 
                   !strncmp(Args[ArgIndex], "--multithread", 13)){
            Prefs->Multithreaded = true;
            if(!PlatformGetProcessorCoreCount(&Prefs->ThreadCount)){
                Error("GetProcessorCoreCount failed.");
                Prefs->ThreadCount = 2;
            }
        } else if (!strncmp(Args[ArgIndex], "-t", 2) || 
                   !strncmp(Args[ArgIndex], "--threads", 9)){
            if((ArgIndex+1) < ArgCount){
                ArgIndex++;
                Prefs->ThreadCount = StringToU32(Args[ArgIndex]);
                if(Prefs->ThreadCount > 1){
                    Prefs->Multithreaded = true;
                } else if (Prefs->ThreadCount == 0){
                    Result = false;
                }
            } else {
                Error("Thread count is not specified.");
            }
        } else {
            Error("Argument could not be recognized : %s", Args[ArgIndex]);
        }
    }

    return Result;
}

static void
ShowUsage(){
    printf("Usage:\n");
    printf("    azimuth [-s | --scene] <filepath> [Options]\n");
    printf("    Options:\n");
    printf("            [-c | --cull ]                 : Apply backface culling\n");
    printf("            [-m | --multithread]           : Run multithreaded");
    printf("            [-t | --threads] <thread_count>: Thread count");
    printf("    Example:\n");
    printf("        azimuth -s bunny.xml --cull\n");
}

#if AzimuthDevelopmentBuild

#pragma warning(push)
#pragma warning(disable : 4127)

inline b32 
ValidateDataTypeSize(){
    b32 Valid = true;

    if(sizeof(s64) != 8){ Valid = false; Error("s64 is not 64 bits length!"); }
    if(sizeof(s32) != 4){ Valid = false; Error("s32 is not 32 bits length!"); }
    if(sizeof(s16) != 2){ Valid = false; Error("s16 is not 16 bits length!"); }
    if(sizeof(s8)  != 1){ Valid = false; Error("s8  is not  8 bits length!"); }
    
    if(sizeof(u64) != 8){ Valid = false; Error("u64 is not 64 bits length!"); }
    if(sizeof(u32) != 4){ Valid = false; Error("u32 is not 32 bits length!"); }
    if(sizeof(u16) != 2){ Valid = false; Error("u16 is not 16 bits length!"); }
    if(sizeof(u8)  != 1){ Valid = false; Error("u8  is not  8 bits length!"); }
    
    if(sizeof(f64) != 8){ Valid = false; Error("f64 is not 64 bits length!"); }
    if(sizeof(f32) != 4){ Valid = false; Error("f32 is not 32 bits length!"); }

    return Valid;
}

#pragma warning(pop) 

int __cdecl main(int ArgCount, char ** Args){
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );  
    if(!ValidateDataTypeSize()){
        Log("Data type size validation failed. Terminating.\n");
        return -1;
    }
#else
int __cdecl main(int ArgCount, char ** Args){
#endif
    // NOTE(furkan): Do not call ProfilerRecord[Start|End] in 
    // main() or it may cause a crash. Because profiler uses 
    // __COUNTER__ macro to keep track of regions (i.e. Start 
    // and End pairs). So, calling one of them after 
    // ProfilerInitialize will cause an out-of-bounds memory 
    // access.  
    ProfilerInitialize();

    // TODO(furkan): Add multithreading support for profiler.
    // Currently, the profiler does not work properly and
    // it is disabled. To enable it, change AzimuthProfilerEnabled
    // in config header to 1.
    timer_freq TimerFreq;
    if(!PlatformQueryTimerFrequency(&TimerFreq)){
        TimerFreq.Value = 1;
    }

    timer TimerStart = { 0 };
    PlatformQueryTimer(&TimerStart);

    b32 Success = false;

    preferences Preferences = { 0 };
    Preferences.ThreadCount = 1;
    epsilon_values Epsilons = { 0 };
    Epsilons.Intersection = 0.0f;
    Epsilons.ShadowRay    = 0.000275f;
    scene Scene = DefaultScene();
    camera *Cameras = 0;
    u32 CameraCount = 0;
    if(ParseArguments(&Preferences, Args, ArgCount)){    
        if(ImportXML(&Scene, &Cameras, &CameraCount, 
                     &Epsilons, 
                     Preferences.SceneFilePath)){
            Success = true;
        } else {
            printf("Failed to import \"%s\", terminating\n", 
                                                   Preferences.SceneFilePath);
        }
    } else {
        Error("Invalid argument list");
        ShowUsage();
    }

    // TODO(furkan): Delete the code below after updating profiler
    timer TimerEnd = { 0 };
    PlatformQueryTimer(&TimerEnd);

    printf("Elapsed time for scene construction: %f\n", 
                        ((f32)(TimerEnd.Counter-TimerStart.Counter)) /
                        ((f32)TimerFreq.Value));

    if(Success){

        printf("Running with %u threads\n", Preferences.ThreadCount);

        for(u32 CameraIndex=0;
                CameraIndex<CameraCount;
                CameraIndex++){
            camera *Camera = Cameras + CameraIndex;

            PlatformQueryTimer(&TimerStart);

            RenderScene(&Scene, Camera, &Epsilons, 
                        Preferences.ThreadCount, 
                        Preferences.BackfaceCulling);

            PlatformQueryTimer(&TimerEnd);

            printf("Render time for camera #%u: %f\n", (CameraIndex+1),
                                ((f32)(TimerEnd.Counter-TimerStart.Counter)) /
                                ((f32)TimerFreq.Value));
        }
    }

    if(Success){
        Log("Completed successfully");
    } else {
        Log("Failure");
    }

    ProfilerReport();
    ProfilerFinalize();

    return 0;
}
