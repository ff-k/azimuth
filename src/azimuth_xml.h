// TODO(furkan): Get rid of std::vector
#include <vector>
#include <string>
#define stdVectorLastElementPtr(Vec) (&(*((Vec).end()-1)))

#include "libxml/xmlreader.h"

#define XMLNodeType_Element                1
#define XMLNodeType_Text                   3
#define XMLNodeType_Comment                8
#define XMLNodeType_SignificantWhitespace 14
#define XMLNodeType_EndElement            15

#define XMLImporterStateStackCapacity 8
#define XMLImporterStateNode(NodeName)                                        \
    { XMLImporterOnEnter_##NodeName, #NodeName, (u32)StringLength(#NodeName), \
       0, XMLImporterState_Parsing##NodeName }

typedef enum {
    XMLImporterState_Root,
    XMLImporterState_ParsingScene,
    XMLImporterState_ParsingShadowRayEpsilon,
    XMLImporterState_ParsingIntersectionTestEpsilon,
    XMLImporterState_ParsingZeroBasedIndexing,
    XMLImporterState_ParsingMaxRecursionDepth,
    XMLImporterState_ParsingBackgroundColor,
    XMLImporterState_ParsingBackgroundTexture,
    XMLImporterState_ParsingIntegrator,
    XMLImporterState_ParsingIntegratorParams,
    XMLImporterState_ParsingCameras,
    XMLImporterState_ParsingCamera,
    XMLImporterState_ParsingPosition,
    XMLImporterState_ParsingGaze,
    XMLImporterState_ParsingUp,
    XMLImporterState_ParsingNearPlane,
    XMLImporterState_ParsingNearDistance,
    XMLImporterState_ParsingFocusDistance,
    XMLImporterState_ParsingApertureSize,
    XMLImporterState_ParsingImageResolution,
    XMLImporterState_ParsingNumSamples,
    XMLImporterState_ParsingImageName,
    XMLImporterState_ParsingGazePoint,
    XMLImporterState_ParsingFovY,
    XMLImporterState_ParsingGammaCorrection,
    XMLImporterState_ParsingTonemap,
    XMLImporterState_ParsingTMO,
    XMLImporterState_ParsingTMOOptions,
    XMLImporterState_ParsingSaturation,
    XMLImporterState_ParsingGamma,
    XMLImporterState_ParsingLights,
    XMLImporterState_ParsingSphericalDirectionalLight,
    XMLImporterState_ParsingEnvMapName,
    XMLImporterState_ParsingAmbientLight,
    XMLImporterState_ParsingPointLight,
    XMLImporterState_ParsingAreaLight,
    XMLImporterState_ParsingDirectionalLight,
    XMLImporterState_ParsingSpotLight,
    XMLImporterState_ParsingIntensity,
    XMLImporterState_ParsingDirection,
    XMLImporterState_ParsingRadiance,
    XMLImporterState_ParsingCoverageAngle,
    XMLImporterState_ParsingFalloffAngle,
    XMLImporterState_ParsingEdgeVector1,
    XMLImporterState_ParsingEdgeVector2,
    XMLImporterState_ParsingBRDFs,
    XMLImporterState_ParsingOriginalPhong,
    XMLImporterState_ParsingModifiedPhong,
    XMLImporterState_ParsingOriginalBlinnPhong,
    XMLImporterState_ParsingModifiedBlinnPhong,
    XMLImporterState_ParsingTorranceSparrow,
    XMLImporterState_ParsingExponent,
    XMLImporterState_ParsingRefractiveIndex,
    XMLImporterState_ParsingMaterials,
    XMLImporterState_ParsingMaterial,
    XMLImporterState_ParsingAmbientReflectance,
    XMLImporterState_ParsingDiffuseReflectance,
    XMLImporterState_ParsingSpecularReflectance,
    XMLImporterState_ParsingPhongExponent,
    XMLImporterState_ParsingMirrorReflectance,
    XMLImporterState_ParsingTransparency,
    XMLImporterState_ParsingRoughness,
    XMLImporterState_ParsingRefractionIndex,
    XMLImporterState_ParsingTransformations,
    XMLImporterState_ParsingTranslation,
    XMLImporterState_ParsingScaling,
    XMLImporterState_ParsingRotation,
    XMLImporterState_ParsingTextures,
    XMLImporterState_ParsingTexture,
    XMLImporterState_ParsingDecalMode,
    XMLImporterState_ParsingInterpolation,
    XMLImporterState_ParsingNormalizer,
    XMLImporterState_ParsingAppearance,
    XMLImporterState_ParsingScalingFactor,
    XMLImporterState_ParsingVertexData,
    XMLImporterState_ParsingTexCoordData,
    XMLImporterState_ParsingObjects,
    XMLImporterState_ParsingMesh,
    XMLImporterState_ParsingFaces,
    XMLImporterState_ParsingMeshInstance,
    XMLImporterState_ParsingSphere,
    XMLImporterState_ParsingCenter,
    XMLImporterState_ParsingRadius,
    XMLImporterState_ParsingMotionBlur,
    XMLImporterState_ParsingLightSphere,
    XMLImporterState_ParsingLightMesh
} xml_importer_state;

typedef struct {
    u32 MeshIndex;
    u32 PlyIndex;
} mesh_ply_pair;

typedef struct{
    std::vector<entity>        Entities;
    std::vector<mesh>          Meshes;
    std::vector<texture>       Textures2D;
    std::vector<brdf>          BRDFs;
    std::vector<material>      Materials;
    std::vector<light>         Lights;
    std::vector<vec4>          Rotations;
    std::vector<vec3>          Scalings;
    std::vector<vec3>          Translations;
    std::vector<vec3>          Vertices;
    std::vector<vec3u>         Faces;
    std::vector<vec2>          UVs;
    std::vector<mesh_ply_pair> MeshPlyPairs;
    std::vector<std::string>   PlyFiles;
    std::vector<u32>           FaceBaseIndices;
    std::vector<u32>           FaceCounts;
    std::vector<u32>           SphereCenters;
    std::vector<u32>           EntityLightIndices;
    std::vector<u32>           LightMeshIndices;
    std::vector<camera>        Cameras;

    scene *Scene;
    epsilon_values *Epsilons;

    b32 ZeroBasedIndexing;
} xml_importer_data;

typedef struct xml_importer_state_node xml_importer_state_node; 

typedef b32 xml_importer_on_enter(xml_importer_data *Data,
                                  xmlTextReaderPtr XMLReader, 
                                  xml_importer_state_node *StateNode, 
                                  xml_importer_state *StateStack, 
                                  u32 StateStackDepth);

struct xml_importer_state_node{
    xml_importer_on_enter *OnEnter;

    char *Name;
    u32 NameLength;

    u32 PastVisitCount;

    xml_importer_state State;
};

typedef enum {
    PlyFormat_ASCII,
    PlyFormat_Binary
} ply_format;
