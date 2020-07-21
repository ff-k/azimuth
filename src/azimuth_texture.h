#if 0
#define PerlinGradientSize 16

vec3 PerlinGradient[PerlinGradientSize] = {
    Vec3( 1.0f, 1.0f, 0.0f), Vec3(-1.0f, 1.0f, 0.0f), 
    Vec3( 1.0f,-1.0f, 0.0f), Vec3(-1.0f,-1.0f, 0.0f), 
    Vec3( 1.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 1.0f), 
    Vec3( 1.0f, 0.0f,-1.0f), Vec3(-1.0f, 0.0f,-1.0f), 
    Vec3( 0.0f, 1.0f, 1.0f), Vec3( 0.0f,-1.0f, 1.0f), 
    Vec3( 0.0f, 1.0f,-1.0f), Vec3( 0.0f,-1.0f,-1.0f), 
    Vec3( 1.0f, 1.0f, 0.0f), Vec3(-1.0f, 1.0f, 0.0f), 
    Vec3( 0.0f,-1.0f, 1.0f), Vec3( 0.0f,-1.0f,-1.0f)
};
b32 PerlinGradientFilled = true;
#else
#define PerlinGradientSize 256
vec3 PerlinGradient[PerlinGradientSize];
b32 PerlinGradientFilled = false;
#endif

typedef enum {
    TexturePerlinAppearance_Patch,
    TexturePerlinAppearance_Vein
} texture_perlin_appearance;

typedef enum {
    TextureWrapMode_Clamp,
    TextureWrapMode_Repeat
} texture_wrap_mode;

typedef enum {
    TextureBlendMode_ReplaceAll,
    TextureBlendMode_ReplaceDiffuse,
    TextureBlendMode_BlendDiffuse
} texture_blend_mode;

typedef enum {
    TextureFilter_Nearest,
    TextureFilter_Bilinear
} texture_filter;

typedef enum {
    TextureType_2D,
    TextureType_PerlinNoise
} texture_type;

typedef enum {
    PixelFormat_RGB_U8,
    PixelFormat_RGB_F32
} pixel_format;

typedef struct {
    char ImagePath[256];

    union {
        struct {
            void *Pixels;
            pixel_format PixelFormat;

            u32 Width;
            u32 Height;
            f32 InvWidth;
            f32 InvHeight;
            f32 Normalizer;

            b32 Loaded;
            file_format ImageFileFormat;
        };
        struct {
            u32 Perm[PerlinGradientSize];
            f32 ScalingFactor;
            texture_perlin_appearance PerlinAppearance;
        };
    };

    f32 BumpMapMultiplier;
    b32 BumpMap;
    b32 Degamma;

    u32 ID;

    texture_wrap_mode WrapMode;
    texture_blend_mode BlendMode;
    texture_filter Filter;
    texture_type Type;
} texture;

static texture
DefaultTexture(){
    texture Result;

    ZeroStruct(&Result);
    
    Result.PixelFormat = PixelFormat_RGB_U8;
    Result.Normalizer = 255.0f;
    Result.BumpMapMultiplier = 1.0f;
    Result.BumpMap = false;
    Result.WrapMode = TextureWrapMode_Repeat;
    Result.BlendMode = TextureBlendMode_BlendDiffuse;
    Result.Filter = TextureFilter_Nearest;
    Result.Type = TextureType_2D;

    return Result;
}

static void
InitPerlinNoise(texture *Texture){
    
    Assert(Texture->Type == TextureType_PerlinNoise);

    if(!PerlinGradientFilled){
        for(u32 Idx=0; Idx<PerlinGradientSize; Idx++){
            vec3 V = (Vec3(Random01(), Random01(), Random01())-0.5f)*2.0f;
            PerlinGradient[Idx] = Normalize(V);
        }
        PerlinGradientFilled = true;
    }

    for(u32 Idx=0; Idx<PerlinGradientSize; Idx++){
        Texture->Perm[Idx] = Idx;
    }

    u32 *Perm = Texture->Perm;

    u32 Idx = PerlinGradientSize;
    do {
        u32 SwapIdx = (u32)(Random01()*Idx);
        Perm[Idx]     = Perm[Idx] ^ Perm[SwapIdx];
        Perm[SwapIdx] = Perm[Idx] ^ Perm[SwapIdx];
        Perm[Idx]     = Perm[Idx] ^ Perm[SwapIdx];
    } while(Idx--);
}

inline u32
PerlinHash(u32 P[PerlinGradientSize], s32 I, s32 J, s32 K){
    u32 Result;

    Result = P[(I + P[(J + P[(K) % 
                             PerlinGradientSize]) % 
                      PerlinGradientSize]) % 
               PerlinGradientSize];

    return Result;
}

inline f32
PerlinWeight(f32 Val){
    f32 Result;

    Val = Abs(Val);
    Result = 1.0f - ((((6.0f*Val)-15.0f)*Val+10.0f)*Val*Val*Val);

    return Result;
}

static f32
SamplePerlinNoise(texture *Texture, vec3 P){

    f32 Result = 0.0f;

    Assert(Texture->Type == TextureType_PerlinNoise);

    P *= Texture->ScalingFactor;

    s32 I = FloorAsS32(P.x);
    s32 J = FloorAsS32(P.y);
    s32 K = FloorAsS32(P.z);

    for(s32 Z=0; Z<2; Z++){
        for(s32 Y=0; Y<2; Y++){
            for(s32 X=0; X<2; X++){
                u32 H = PerlinHash(Texture->Perm, I+X, J+Y, K+Z);
                Assert(H < PerlinGradientSize);
                vec3 E = PerlinGradient[H];
                vec3 V = Vec3(P.x-(f32)(I+X), P.y-(f32)(J+Y), P.z-(f32)(K+Z));
                Result +=         Dot(E, V) * PerlinWeight(V.x) * 
                          PerlinWeight(V.y) * PerlinWeight(V.z);
            }
        }
    }

    switch(Texture->PerlinAppearance){
        case TexturePerlinAppearance_Patch: {
            Result = (Result+1.0f)/2.0f;
        } break;
        case TexturePerlinAppearance_Vein: {
            Result = Abs(Result);
        } break;
        UnexpectedDefaultCase;
    }

    Assert(Result > -0.001f && Result < 1.001f);

    return Result;
}

static vec3
FetchTexture2D_RGB_U8(texture *Texture, u32 FetchX, u32 FetchY){
    vec3 Result;

    Assert(FetchX >= 0 && FetchX < Texture->Width);
    Assert(FetchY >= 0 && FetchY < Texture->Height);
   
    const u32 ChannelCount = 3;
    u32 TexelIndex = (FetchY*Texture->Width + FetchX)*ChannelCount;
    Assert(TexelIndex < (Texture->Width*Texture->Height*ChannelCount));

    Assert(Texture->PixelFormat == PixelFormat_RGB_U8);
    u8 *Pixels = (u8 *)Texture->Pixels;

    Result.r = (f32)Pixels[TexelIndex + 0];
    Result.g = (f32)Pixels[TexelIndex + 1];
    Result.b = (f32)Pixels[TexelIndex + 2];

    return Result;
}

static vec3
FetchTexture2D_RGB_F32(texture *Texture, u32 FetchX, u32 FetchY){
    vec3 Result;

    Assert(FetchX >= 0 && FetchX < Texture->Width);
    Assert(FetchY >= 0 && FetchY < Texture->Height);
   
    const u32 ChannelCount = 3;
    u32 TexelIndex = (FetchY*Texture->Width + FetchX)*ChannelCount;
    Assert(TexelIndex < (Texture->Width*Texture->Height*ChannelCount));

    Assert(Texture->PixelFormat == PixelFormat_RGB_F32);
    f32 *Pixels = (f32 *)Texture->Pixels;

    Result.r = (f32)Pixels[TexelIndex + 0];
    Result.g = (f32)Pixels[TexelIndex + 1];
    Result.b = (f32)Pixels[TexelIndex + 2];

    return Result;
}

static vec3
SampleTexture2D(texture *Texture, vec2 UV, b32 Normalize){
    vec3 Result = Vec3(0.0f, 0.0f, 0.0f);

    Assert(Texture->Type == TextureType_2D);
    Assert(Texture->Loaded);

    switch(Texture->WrapMode){
        case TextureWrapMode_Clamp:{
            UV.u = MaxOf(0.0f, MinOf(UV.u, 1.0f));
            UV.v = MaxOf(0.0f, MinOf(UV.v, 1.0f));
        } break;
        case TextureWrapMode_Repeat:{
            UV.u = UV.u - Floor(UV.u);
            UV.v = UV.v - Floor(UV.v);
        } break;
        UnexpectedDefaultCase;
    }

    Assert(UV.u >= 0.0f && UV.u <= 1.0f);
    Assert(UV.v >= 0.0f && UV.v <= 1.0f);

    vec3 (*FetchTexture2D)(texture *, u32, u32) = 0;
    switch(Texture->PixelFormat){
        case PixelFormat_RGB_U8:{
            FetchTexture2D = &FetchTexture2D_RGB_U8;
        } break;
        case PixelFormat_RGB_F32:{
            FetchTexture2D = &FetchTexture2D_RGB_F32;
        } break;
        UnexpectedDefaultCase;
    }

    switch(Texture->Filter){
        case TextureFilter_Nearest:{
            u32 FetchX = FloorAsU32(UV.u*((f32)Texture->Width-1.0f)  + 0.5f);
            u32 FetchY = FloorAsU32(UV.v*((f32)Texture->Height-1.0f) + 0.5f);
            Result = FetchTexture2D(Texture, FetchX, FetchY);
        } break;
        case TextureFilter_Bilinear:{
            // NOTE(furkan):
            //     (0, 0)
            //          ___________________________
            //         |___|___|___|___|___|___|___|
            //         |___|_C_|_D_|___|___|___|___|
            //         |___|_B_|_A_|___|___|___|___|
            //         |___|___|___|___|___|___|___|
            //                               (1, 1)

            vec2 ST = Vec2(UV.u*(f32)Texture->Width, 
                           UV.v*(f32)Texture->Height);

            u32 AX = RoundAsU32(ST.u);
            AX = AX % Texture->Width;
            u32 AY = RoundAsU32(ST.v);
            AY = AY % Texture->Height;

            u32 BX = (AX-1)%Texture->Width;
            u32 BY = (AY  )%Texture->Height;

            u32 CX = (AX-1)%Texture->Width;
            u32 CY = (AY-1)%Texture->Height;

            u32 DX = (AX  )%Texture->Width;
            u32 DY = (AY-1)%Texture->Height;

            vec3 A = FetchTexture2D(Texture, AX, AY);
            vec3 B = FetchTexture2D(Texture, BX, BY);
            vec3 C = FetchTexture2D(Texture, CX, CY);
            vec3 D = FetchTexture2D(Texture, DX, DY);

            ST.u = Round(ST.u);
            ST.v = Round(ST.v);

            if(ST.u >= Texture->Width){
                ST.u -= Texture->Width;
            }

            if(ST.v >= Texture->Height){
                ST.v -= Texture->Height;
            }

            f32 FactorX = ST.u - (f32)AX;
            f32 FactorY = ST.v - (f32)AY;

            Result = A*(1.0f-FactorX)*(1.0f-FactorY) +
                     B*(     FactorX)*(1.0f-FactorY) +
                     C*(     FactorX)*(     FactorY) +
                     D*(1.0f-FactorX)*(     FactorY);
        } break;
        UnexpectedDefaultCase;
    }

    if(Normalize){
        Result.r /= Texture->Normalizer;
        Result.g /= Texture->Normalizer;
        Result.b /= Texture->Normalizer;
    }

    return Result;
}

static vec3
SampleTexture(texture *Texture, vec3 P, vec2 UV){
    vec3 Result = Vec3(0.0f, 0.0f, 0.0f);

    switch(Texture->Type){
        case TextureType_2D: {
            Result = SampleTexture2D(Texture, UV, true);
        } break;
        case TextureType_PerlinNoise: {
            f32 Noise = SamplePerlinNoise(Texture, P);
            Result = Vec3(Noise, Noise, Noise);
        } break;
        UnexpectedDefaultCase;
    }

    return Result;
}

#pragma warning(push)
#pragma warning(disable : 4100)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning(pop)
static b32
LoadTexture(texture *Texture){
    b32 Result = false;

    Assert(Texture->ImagePath[0]);
    Assert(!Texture->Pixels);
    Assert(Texture->Type == TextureType_2D);

    if(Texture->Loaded){
        Error("Texture is already loaded");
    } else {
        if(CheckImageFileFormat(&Texture->ImageFileFormat, 
                                Texture->ImagePath, 
                                StringLength(Texture->ImagePath))){
            switch(Texture->ImageFileFormat){
                case FileFormat_PNG: 
                case FileFormat_BMP: 
                case FileFormat_TGA: 
                case FileFormat_JPG:{
                    s32 Component;
                    Texture->Pixels = stbi_load(Texture->ImagePath, 
                                                (s32 *)&Texture->Width, 
                                                (s32 *)&Texture->Height, 
                                                &Component, 3);
                    if(Texture->Pixels){
                        Texture->Loaded = true;

                        if(Texture->Degamma){
                            u32 W = Texture->Width;
                            u32 H = Texture->Height;
                            u8 *Pixels = (u8 *)Texture->Pixels;
                            for(u32 Row=0; Row<H; Row++){
                                for(u32 Col=0; Col<W; Col++){
                                    f32 R = Power((( (f32)*Pixels)/255.0f), 
                                                  2.2f) * 255.001f;
                                    *Pixels = (u8)(R);
                                    Pixels++;

                                    f32 G = Power((( (f32)*Pixels)/255.0f), 
                                                  2.2f) * 255.001f;
                                    *Pixels = (u8)(G);
                                    Pixels++;

                                    f32 B = Power((( (f32)*Pixels)/255.0f), 
                                                  2.2f) * 255.001f;
                                    *Pixels = (u8)(B);
                                    Pixels++;
                                }
                            }
                        }
                    }

                    Texture->PixelFormat = PixelFormat_RGB_U8;
                } break;
                case FileFormat_EXR:{
                    Imf::RgbaInputFile EXRFile(Texture->ImagePath);
                    Imath::Box2i DataWindow = EXRFile.dataWindow();
                    Texture->Width  = DataWindow.max.x - 
                                      DataWindow.min.x + 1;
                    Texture->Height = DataWindow.max.y - 
                                      DataWindow.min.y + 1;
                    
                    Imf::Rgba *EXRPixels = 0;
                    if(PlatformMemoryAllocate((void **)&EXRPixels, 
                                              sizeof(Imf::Rgba) * 
                                              Texture->Width * 
                                              Texture->Height) &&
                       PlatformMemoryAllocate((void **)&Texture->Pixels, 
                                              sizeof(f32) * 3 *
                                              Texture->Width * 
                                              Texture->Height)){
                        EXRFile.setFrameBuffer(&EXRPixels[0] - 
                                               DataWindow.min.x - 
                                               DataWindow.min.y * 
                                               Texture->Width, 1, 
                                               Texture->Width);
                        EXRFile.readPixels(DataWindow.min.y, 
                                           DataWindow.max.y);
                        
                        u32 W = Texture->Width;
                        u32 H = Texture->Height;
                        for(u32 Row=0; Row<H; Row++){
                            for(u32 Col=0; Col<W; Col++){
                                Imf::Rgba EXRPixel = EXRPixels[Row*W + Col];
                                f32 R = EXRPixel.r;
                                f32 G = EXRPixel.g;
                                f32 B = EXRPixel.b;

                                if(Texture->Degamma){
                                    R = Power(R, 2.2f);
                                    G = Power(G, 2.2f);
                                    B = Power(B, 2.2f);
                                }
                                
                                f32 *Pixel = ((f32 *)Texture->Pixels) + 
                                             (3 * (Row*W+Col));
                                Pixel[0] = R;
                                Pixel[1] = G;
                                Pixel[2] = B;
                            }
                        }

                        Texture->Loaded = true;

                        PlatformMemoryFree((void **)&EXRPixels);
                    } else {
                        Error("Memory could not be allocated");
                    }

                    Texture->PixelFormat = PixelFormat_RGB_F32;
                } break;
                UnexpectedDefaultCase;
            }

            if(Texture->Loaded){
                Texture->InvWidth = 1.0f/(f32)Texture->Width;
                Texture->InvHeight = 1.0f/(f32)Texture->Height;

                Result = true;
            } else {
                Error("Image could not be loaded from %s", 
                      Texture->ImagePath);
            }
        } else {
            Error("Only EXR and PNG formats are supported for textures");
        }
    }

    return Result;
}

