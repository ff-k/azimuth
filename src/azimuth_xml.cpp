template<typename T> static b32
STDVectorToArray(void **Dest, std::vector<T> Vec){
    b32 Result = false;

    size_t DataSize = sizeof(T) * Vec.size();
    if(MemoryAllocate(Dest, DataSize)){
        if(MemoryCopy(*Dest, DataSize,
                      Vec.data(), DataSize)){
            Result = true;
        } else {
             Error("MemoryCopy failed");       
        }
    } else {
        Error("Memory allocation failed");
    }

    return Result;
}

static char *
XMLReadValue(xmlTextReaderPtr XMLReader){
    char *Result = 0;

    if(xmlTextReaderRead(XMLReader) == 1){
        if(xmlTextReaderNodeType(XMLReader) == XMLNodeType_Text){
            Result = (char *)xmlTextReaderConstValue(XMLReader);
        } else {
            Error("Value could not be found");
        }
    } else {
        Error("An error occurred in xmlTextReaderRead");
    }

    return Result;
}

#pragma warning(push)
#pragma warning(disable : 4100)

static b32
XMLImporterOnEnter_Scene(xml_importer_data *Data, 
                         xmlTextReaderPtr XMLReader, 
                         xml_importer_state_node *StateNode, 
                         xml_importer_state *StateStack, 
                         u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_Root){
        Result = false;
    }

    if(StateNode->PastVisitCount){
        Error("Multiple scenes are not supported");
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_ShadowRayEpsilon(xml_importer_data *Data, 
                                    xmlTextReaderPtr XMLReader, 
                                    xml_importer_state_node *StateNode, 
                                    xml_importer_state *StateStack, 
                                    u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 Eps = StringToF32(Value);
            Data->Epsilons->ShadowRay = Eps;

            if(StateNode->PastVisitCount){
                Warning("ShadowRayEpsilon is overwritten");
            }

            Log("ShadowRayEpsilon: %f", Eps);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_IntersectionTestEpsilon(xml_importer_data *Data, 
                                           xmlTextReaderPtr XMLReader, 
                                           xml_importer_state_node *StateNode, 
                                           xml_importer_state *StateStack, 
                                           u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 Eps = StringToF32(Value);
            Data->Epsilons->Intersection = Eps;

            if(StateNode->PastVisitCount){
                Warning("IntersectionTestEpsilon is overwritten");
            }

            Log("IntersectionTestEpsilon: %f", Eps);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_ZeroBasedIndexing(xml_importer_data *Data, 
                                     xmlTextReaderPtr XMLReader, 
                                     xml_importer_state_node *StateNode, 
                                     xml_importer_state *StateStack, 
                                     u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            size_t NameLen = StringLength(Value);
            if(!StringCompare(Value, NameLen, "true", 4)){
                Data->ZeroBasedIndexing = true;
            }
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_MaxRecursionDepth(xml_importer_data *Data, 
                                     xmlTextReaderPtr XMLReader, 
                                     xml_importer_state_node *StateNode, 
                                     xml_importer_state *StateStack, 
                                     u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            u32 MRD = StringToU32(Value);
            Data->Scene->MaxRecursionDepth = MRD;

            if(StateNode->PastVisitCount){
                Warning("MaxRecursionDepth is overwritten");
            }

            Log("MaxRecursionDepth: %u", MRD);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_BackgroundColor(xml_importer_data *Data, 
                                   xmlTextReaderPtr XMLReader, 
                                   xml_importer_state_node *StateNode, 
                                   xml_importer_state *StateStack, 
                                   u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 BackgroundColor = StringToVec3(Value);
            Data->Scene->BackgroundColor = Vec3(BackgroundColor.x/255.0f,
                                          BackgroundColor.y/255.0f,
                                          BackgroundColor.z/255.0f);

            if(StateNode->PastVisitCount){
                Warning("BackgroundColor is overriden");
            }

            Log("BackgroundColor: %f, %f, %f", BackgroundColor.x, 
                                               BackgroundColor.y, 
                                               BackgroundColor.z);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_BackgroundTexture(xml_importer_data *Data, 
                                     xmlTextReaderPtr XMLReader, 
                                     xml_importer_state_node *StateNode, 
                                     xml_importer_state *StateStack, 
                                     u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            texture *T = &Data->Scene->BackgroundTexture;
            size_t NameLenMax = StaticArraySize(T->ImagePath);
            size_t NameLen = StringLength(Value);
            if(MemoryCopy(T->ImagePath, NameLenMax, Value, NameLen)){
                T->ImagePath[MinOf(NameLenMax-1, NameLen)] = '\0';
                Log("ImagePath: %s", Value);

                if(!StringCompare(Value, NameLen, "perlin", 6)){
                    T->Type = TextureType_PerlinNoise;
                    InitPerlinNoise(T);
                } else {
                    T->Type = TextureType_2D;
                    Result = LoadTexture(T);
                    Data->Scene->UseBackgroundTexture = Result;
                }
            } else {
                Error("An error occured while copying ImageName");
            }
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Integrator(xml_importer_data *Data, 
                              xmlTextReaderPtr XMLReader, 
                              xml_importer_state_node *StateNode, 
                              xml_importer_state *StateStack, 
                              u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            size_t NameLen = StringLength(Value);
            if(!StringCompare(Value, NameLen, "PathTracing", 11)){
                Data->Scene->PathTrace = true;
            } else {
                Error("Unexpected integrator: %s", Value);
                Result = false;
            }
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_IntegratorParams(xml_importer_data *Data, 
                                    xmlTextReaderPtr XMLReader, 
                                    xml_importer_state_node *StateNode, 
                                    xml_importer_state *StateStack, 
                                    u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            size_t NameLen = StringLength(Value);
            if(!StringCompare(Value, NameLen, "ImportanceSampling", 18)){
                Data->Scene->ImportanceSampling = true;
            } else {
                Error("Unexpected integrator parameter: %s", Value);
                Result = false;
            }
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Cameras(xml_importer_data *Data, 
                           xmlTextReaderPtr XMLReader, 
                           xml_importer_state_node *StateNode, 
                           xml_importer_state *StateStack, 
                           u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_Camera(xml_importer_data *Data, 
                          xmlTextReaderPtr XMLReader, 
                          xml_importer_state_node *StateNode, 
                          xml_importer_state *StateStack, 
                          u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCameras){
        Result = false;
    } else {
        camera C = DefaultCamera();

        char *CameraID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                           (xmlChar *)"id");
        if(CameraID){
            u32 ID = StringToU32(CameraID);
            xmlFree(CameraID);
            C.ID = ID;

            Log("Parsing a camera with id %u", ID);
        }

        b32 IsLeftHanded = false;
        char *Handedness = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)
                                                             "handedness");
        if(Handedness){
            size_t HandednessLen = StringLength(Handedness);
            if(!StringCompare(Handedness, HandednessLen, "left", 4)){
                IsLeftHanded = true;
            } else if(!StringCompare(Handedness, HandednessLen, "right", 5)){
                IsLeftHanded = false;
            } else {
                Error("Unrecognized handedness value: %s", Handedness);
            }

            xmlFree(Handedness);
        }
        C.IsLeftHanded = IsLeftHanded;

        char *CameraType = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                           (xmlChar *)"type");
        if(CameraType){
            size_t CameraTypeLen = StringLength(CameraType);
            if(!StringCompare(CameraType, CameraTypeLen, "simple", 6)){
                C.Type = CameraType_Simple;
            }

            xmlFree(CameraType);
        }

        char *EXROpt = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                         (xmlChar *)
                                                         "saveEXR");
        if(EXROpt){
            size_t EXROptLen = StringLength(EXROpt);
            if(!StringCompare(EXROpt, EXROptLen, "true", 4)){
                C.SaveEXR = true;
            }

            xmlFree(EXROpt);
        }

        Data->Cameras.push_back(C);
    }

    return Result;
}

static b32
XMLImporterOnEnter_Position(xml_importer_data *Data, 
                            xmlTextReaderPtr XMLReader, 
                            xml_importer_state_node *StateNode, 
                            xml_importer_state *StateStack, 
                            u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingPointLight || 
       StateStack[StateStackDepth] == XMLImporterState_ParsingAreaLight ||
       StateStack[StateStackDepth] == XMLImporterState_ParsingSpotLight){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Position = StringToVec3(Value);
            light *Light = stdVectorLastElementPtr(Data->Lights);
            Light->Position = Position;

            Log("Position: %f %f %f", Position.x, 
                                      Position.y, 
                                      Position.z);
        }
    } else if(StateStack[StateStackDepth] == XMLImporterState_ParsingCamera){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Position = StringToVec3(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->Position = Position;

            Log("Position: %f %f %f", Position.x, 
                                      Position.y, 
                                      Position.z);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_Gaze(xml_importer_data *Data, 
                        xmlTextReaderPtr XMLReader, 
                        xml_importer_state_node *StateNode, 
                        xml_importer_state *StateStack, 
                        u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        camera *Camera = stdVectorLastElementPtr(Data->Cameras);
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Gaze = StringToVec3(Value);
            Camera->Gaze = Gaze;

            Log("Gaze: %f %f %f", Gaze.x, 
                                  Gaze.y, 
                                  Gaze.z);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Up(xml_importer_data *Data, 
                      xmlTextReaderPtr XMLReader, 
                      xml_importer_state_node *StateNode, 
                      xml_importer_state *StateStack, 
                      u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Up = StringToVec3(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->Up = Up;

            Log("Up: %f %f %f", Up.x, 
                                Up.y, 
                                Up.z);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_NearPlane(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        camera *Camera = stdVectorLastElementPtr(Data->Cameras);

        if(Camera->Type == CameraType_Detailed){
            char *Value = XMLReadValue(XMLReader);
            if(Value){
                vec4 NearPlane = StringToVec4(Value);
                Camera->NearPlane = Vec4(NearPlane.y, 
                                         NearPlane.x,
                                         NearPlane.zw);

                Log("NearPlane: %f %f %f %f", NearPlane.x, 
                                              NearPlane.y, 
                                              NearPlane.z,
                                              NearPlane.w);
            }
        } else {
            Error("NearPlane should only be used for detailed cameras");
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_NearDistance(xml_importer_data *Data, 
                                xmlTextReaderPtr XMLReader, 
                                xml_importer_state_node *StateNode, 
                                xml_importer_state *StateStack, 
                                u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 NearDistance = StringToF32(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->NearDistance = NearDistance;

            Log("NearDistance: %f", NearDistance);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_FocusDistance(xml_importer_data *Data, 
                                 xmlTextReaderPtr XMLReader, 
                                 xml_importer_state_node *StateNode, 
                                 xml_importer_state *StateStack, 
                                 u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 FocusDistance = StringToF32(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->FocusDistance = FocusDistance;

            Log("FocusDistance: %f", FocusDistance);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_ApertureSize(xml_importer_data *Data, 
                                xmlTextReaderPtr XMLReader, 
                                xml_importer_state_node *StateNode, 
                                xml_importer_state *StateStack, 
                                u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 ApertureSize = StringToF32(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->ApertureSize = ApertureSize;

            Log("ApertureSize: %f", ApertureSize);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_ImageResolution(xml_importer_data *Data, 
                                   xmlTextReaderPtr XMLReader, 
                                   xml_importer_state_node *StateNode, 
                                   xml_importer_state *StateStack, 
                                   u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec2u ImageResolution = StringToVec2u(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->Resolution = ImageResolution;

            Log("ImageResolution: %u %u", ImageResolution.x,
                                          ImageResolution.y);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_NumSamples(xml_importer_data *Data, 
                              xmlTextReaderPtr XMLReader, 
                              xml_importer_state_node *StateNode, 
                              xml_importer_state *StateStack, 
                              u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            u32 NumSamples = StringToU32(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->SampleCount = NumSamples;

            Log("NumSamples: %u", NumSamples);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_ImageName(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingCamera){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            size_t NameLenMax = StaticArraySize(Camera->ImageName);
            size_t NameLen = StringLength(Value);
            if(MemoryCopy(Camera->ImageName, NameLenMax, 
                          Value, NameLen)){
                Camera->ImageName[MinOf(NameLenMax-1, 
                                                  NameLen)] = '\0';
                Log("ImageName: %s", Value);
            } else {
                Error("An error occured while copying ImageName");
            }

            if(!CheckImageFileFormat(&Camera->SaveFormat, Value, NameLen)){
                Camera->SaveFormat = FileFormat_PNG;
                Warning("Output format could not be detected, saving as PNG");
            }
        }
    } else if (StateStack[StateStackDepth] == XMLImporterState_ParsingTexture){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            texture *T = stdVectorLastElementPtr(Data->Textures2D);
            size_t NameLenMax = StaticArraySize(T->ImagePath);
            size_t NameLen = StringLength(Value);
            if(MemoryCopy(T->ImagePath, NameLenMax, Value, NameLen)){
                T->ImagePath[MinOf(NameLenMax-1, NameLen)] = '\0';
                Log("ImagePath: %s", Value);

                if(!StringCompare(Value, NameLen, "perlin", 6)){
                    T->Type = TextureType_PerlinNoise;
                    InitPerlinNoise(T);
                } else {
                    T->Type = TextureType_2D;
                    Result = LoadTexture(T);
                }
            } else {
                Error("An error occured while copying ImageName");
            }
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_GazePoint(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        camera *Camera = stdVectorLastElementPtr(Data->Cameras);
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 GazePoint  = StringToVec3(Value);
            Camera->Gaze = GazePoint;
            Camera->GazeIsPoint = true;

            Log("GazePoint: %f %f %f", GazePoint.x,
                                       GazePoint.y,
                                       GazePoint.z);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_FovY(xml_importer_data *Data, 
                        xmlTextReaderPtr XMLReader, 
                        xml_importer_state_node *StateNode, 
                        xml_importer_state *StateStack, 
                        u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        camera *Camera = stdVectorLastElementPtr(Data->Cameras);
        if(Camera->Type == CameraType_Simple){
            char *Value = XMLReadValue(XMLReader);
            if(Value){
                f32 FovY = StringToF32(Value);
                Camera->FovY = FovY;

                Log("FovY: %f", FovY);
            }
        } else {
            Error("FovY should only be used for simple cameras");
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_GammaCorrection(xml_importer_data *Data, 
                                   xmlTextReaderPtr XMLReader, 
                                   xml_importer_state_node *StateNode, 
                                   xml_importer_state *StateStack, 
                                   u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    } else {
        camera *Camera = stdVectorLastElementPtr(Data->Cameras);

        char *Value = XMLReadValue(XMLReader);
        if(Value){
            if(!StringCompare(Value, StringLength(Value),
                              "sRGB", 4)){
                Camera->Gamma = 2.4f;
                Camera->InvGamma = 1.0f/2.4f;
                Camera->GammaCorrection_sRGB = true;
            } else {
                Warning("Gamma correction type is not supported: %s", Value);
            }

            Log("GammaCorrection: %s", Value);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Tonemap(xml_importer_data *Data, 
                           xmlTextReaderPtr XMLReader, 
                           xml_importer_state_node *StateNode, 
                           xml_importer_state *StateStack, 
                           u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingCamera){
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_TMO(xml_importer_data *Data, 
                       xmlTextReaderPtr XMLReader, 
                       xml_importer_state_node *StateNode, 
                       xml_importer_state *StateStack, 
                       u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingTonemap){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            Assert(!StringCompare(Value, StringLength(Value),
                                  "Photographic", 12));

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->Tonemap.Operator = TonemapOperator_Photographic;

            Log("Tonemapping operator: %s", Value);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_TMOOptions(xml_importer_data *Data, 
                              xmlTextReaderPtr XMLReader, 
                              xml_importer_state_node *StateNode, 
                              xml_importer_state *StateStack, 
                              u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingTonemap){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec2 Options = StringToVec2(Value);

            f32 Key = Options.x;
            f32 Pct = Options.y;

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->Tonemap.KeyValue = Key;
            Camera->Tonemap.BurnoutPercentage = Pct;

            Log("Tonemap key value: %f, Tonemap burnout percentage: %f", Key, 
                                                                         Pct);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Saturation(xml_importer_data *Data, 
                              xmlTextReaderPtr XMLReader, 
                              xml_importer_state_node *StateNode, 
                              xml_importer_state *StateStack, 
                              u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingTonemap){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 Saturation = StringToF32(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->Tonemap.Saturation = Saturation;

            Log("Tonemap saturation: %f", Saturation);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Gamma(xml_importer_data *Data, 
                         xmlTextReaderPtr XMLReader, 
                         xml_importer_state_node *StateNode, 
                         xml_importer_state *StateStack, 
                         u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingTonemap){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 Gamma = StringToF32(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            Camera->Gamma = Gamma;
            Camera->InvGamma = 1.0f/Gamma;

            Log("Tonemap gamma: %f", Gamma);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Lights(xml_importer_data *Data, 
                          xmlTextReaderPtr XMLReader, 
                          xml_importer_state_node *StateNode, 
                          xml_importer_state *StateStack, 
                          u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {

    }

    return Result;
}

static b32
XMLImporterOnEnter_SphericalDirectionalLight(xml_importer_data *Data, 
                                         xmlTextReaderPtr XMLReader, 
                                         xml_importer_state_node *StateNode, 
                                         xml_importer_state *StateStack, 
                                         u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingLights){
        Result = false;
    } else {
        char *LightID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                          (xmlChar *)"id");
        if(LightID){
            u32 ID = StringToU32(LightID);
            xmlFree(LightID);

            Log("Parsing a spherical directional light with id %u", ID); 
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_EnvMapName(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == 
                  XMLImporterState_ParsingSphericalDirectionalLight){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            texture *EnvMap = &Data->Scene->EnvironmentMap;
            size_t NameLenMax = StaticArraySize(EnvMap->ImagePath);
            size_t NameLen = StringLength(Value);
            if(MemoryCopy(EnvMap->ImagePath, NameLenMax, Value, NameLen)){
                EnvMap->ImagePath[MinOf(NameLenMax-1, NameLen)] = '\0';
                Log("EnvMapPath: %s", Value);

                if(!StringCompare(Value, NameLen, "perlin", 6)){
                    EnvMap->Type = TextureType_PerlinNoise;
                    InitPerlinNoise(EnvMap);
                } else {
                    if(LoadTexture(EnvMap)){
                        Data->Scene->UseEnvironmentMap = true;
                        Result = true;
                    }
                }
            } else {
                Error("An error occured while copying EnvMapName");
            }
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_AmbientLight(xml_importer_data *Data, 
                                xmlTextReaderPtr XMLReader, 
                                xml_importer_state_node *StateNode, 
                                xml_importer_state *StateStack, 
                                u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingLights){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Ambient = StringToVec3(Value);
            Data->Scene->AmbientLight = Vec3(Ambient.x/255.0f, 
                                             Ambient.y/255.0f,
                                             Ambient.z/255.0f);

            Log("Ambient: %f %f %f", Ambient.x, 
                                     Ambient.y, 
                                     Ambient.z);
        }
    }

    if(StateNode->PastVisitCount){
        Warning("AmbientLight is overriden");
    }

    return Result;
}

static b32
XMLImporterOnEnter_PointLight(xml_importer_data *Data, 
                              xmlTextReaderPtr XMLReader, 
                              xml_importer_state_node *StateNode, 
                              xml_importer_state *StateStack, 
                              u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingLights){
        Result = false;
    } else {
        char *PointLightID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)"id");
        if(PointLightID){
            u32 ID = StringToU32(PointLightID);
            xmlFree(PointLightID);

            light L = DefaultLight();
            L.ID = ID;
            L.Type = LightType_PointLight;

            Data->Lights.push_back(L);

            Log("Parsing a point light with id %u", ID); 
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_AreaLight(xml_importer_data *Data, 
                              xmlTextReaderPtr XMLReader, 
                              xml_importer_state_node *StateNode, 
                              xml_importer_state *StateStack, 
                              u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingLights){
        Result = false;
    } else {
        char *AreaLightID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)"id");
        if(AreaLightID){
            u32 ID = StringToU32(AreaLightID);
            xmlFree(AreaLightID);

            light L = DefaultLight();
            L.ID = ID;
            L.Type = LightType_AreaLight;

            Data->Lights.push_back(L);

            Log("Parsing an area light with id %u", ID); 
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_DirectionalLight(xml_importer_data *Data, 
                                    xmlTextReaderPtr XMLReader, 
                                    xml_importer_state_node *StateNode, 
                                    xml_importer_state *StateStack, 
                                    u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingLights){
        Result = false;
    } else {
        char *LightID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                          (xmlChar *)"id");
        if(LightID){
            u32 ID = StringToU32(LightID);
            xmlFree(LightID);

            light L = DefaultLight();
            L.ID = ID;
            L.Type = LightType_DirectionalLight;

            Data->Lights.push_back(L);

            Log("Parsing a directional light with id %u", ID); 
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_SpotLight(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingLights){
        Result = false;
    } else {
        char *LightID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                          (xmlChar *)"id");
        if(LightID){
            u32 ID = StringToU32(LightID);
            xmlFree(LightID);

            light L = DefaultLight();
            L.ID = ID;
            L.Type = LightType_SpotLight;

            Data->Lights.push_back(L);

            Log("Parsing a spot light with id %u", ID); 
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Intensity(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingPointLight || 
       StateStack[StateStackDepth] == XMLImporterState_ParsingAreaLight ||
       StateStack[StateStackDepth] == XMLImporterState_ParsingSpotLight){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Intensity = StringToVec3(Value);
            light *Light = stdVectorLastElementPtr(Data->Lights);
            Light->Intensity = Vec3(Intensity.x/255.0f, 
                                    Intensity.y/255.0f, 
                                    Intensity.z/255.0f);

            Log("Intensity: %f %f %f", Intensity.x, 
                                       Intensity.y, 
                                       Intensity.z);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_Direction(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth]==XMLImporterState_ParsingDirectionalLight){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Direction = StringToVec3(Value);
            light *Light = stdVectorLastElementPtr(Data->Lights);
            Light->Direction = Vec3(Direction.x/255.0f, 
                                    Direction.y/255.0f, 
                                    Direction.z/255.0f);

            Log("Direction: %f %f %f", Direction.x, 
                                       Direction.y, 
                                       Direction.z);
        }
    } else if(StateStack[StateStackDepth]==XMLImporterState_ParsingSpotLight){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Direction = StringToVec3(Value);
            light *Light = stdVectorLastElementPtr(Data->Lights);
            Light->SpotDirection = Vec3(Direction.x/255.0f, 
                                    Direction.y/255.0f, 
                                    Direction.z/255.0f);

            Log("SpotDirection: %f %f %f", Direction.x, 
                                           Direction.y, 
                                           Direction.z);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_Radiance(xml_importer_data *Data, 
                            xmlTextReaderPtr XMLReader, 
                            xml_importer_state_node *StateNode, 
                            xml_importer_state *StateStack, 
                            u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth]==XMLImporterState_ParsingDirectionalLight){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Radiance = StringToVec3(Value);
            light *Light = stdVectorLastElementPtr(Data->Lights);
            Light->Intensity = Vec3(Radiance.x/255.0f, 
                                    Radiance.y/255.0f, 
                                    Radiance.z/255.0f);

            Log("Radiance: %f %f %f", Radiance.x, 
                                      Radiance.y, 
                                      Radiance.z);
        }
    } else if (StateStack[StateStackDepth] == 
                          XMLImporterState_ParsingLightSphere){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Radiance = StringToVec3(Value);

            entity *E = stdVectorLastElementPtr(Data->Entities);
            E->Radiance = Vec3(Radiance.x/255.0f,
                               Radiance.y/255.0f,
                               Radiance.z/255.0f);
            E->IsEmitter = true;

            Log("Radiance: %f %f %f", Radiance.x, 
                                      Radiance.y, 
                                      Radiance.z);
        }
    } else if (StateStack[StateStackDepth] == 
                          XMLImporterState_ParsingLightMesh){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Radiance = StringToVec3(Value);

            mesh *M = stdVectorLastElementPtr(Data->Meshes);
            M->Radiance = Vec3(Radiance.x/255.0f,
                               Radiance.y/255.0f,
                               Radiance.z/255.0f);
            M->IsEmitter = true;

            Log("Radiance: %f %f %f", Radiance.x, 
                                      Radiance.y, 
                                      Radiance.z);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_CoverageAngle(xml_importer_data *Data, 
                                 xmlTextReaderPtr XMLReader, 
                                 xml_importer_state_node *StateNode, 
                                 xml_importer_state *StateStack, 
                                 u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingSpotLight){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 CoverageAngle = StringToF32(Value);
            light *Light = stdVectorLastElementPtr(Data->Lights);
            Light->CoverageAngle = CoverageAngle;
            Light->CosHalfCoverage = Cos(DegreeToRadian(CoverageAngle)/2.0f);

            Log("CoverageAngle: %f", CoverageAngle);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_FalloffAngle(xml_importer_data *Data, 
                                xmlTextReaderPtr XMLReader, 
                                xml_importer_state_node *StateNode, 
                                xml_importer_state *StateStack, 
                                u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingSpotLight){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 FallOffAngle = StringToF32(Value);
            light *Light = stdVectorLastElementPtr(Data->Lights);
            Light->FallOffAngle = FallOffAngle;
            Light->CosHalfFallOff = Cos(DegreeToRadian(FallOffAngle)/2.0f);

            Log("FalloffAngle: %f", FallOffAngle);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_EdgeVector1(xml_importer_data *Data, 
                               xmlTextReaderPtr XMLReader, 
                               xml_importer_state_node *StateNode, 
                               xml_importer_state *StateStack, 
                               u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingAreaLight){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Vec = StringToVec3(Value);

            light *Light = stdVectorLastElementPtr(Data->Lights);
            if(Light->Type == LightType_AreaLight){
                Light->EdgeVectors[0] = Vec;

                Log("EdgeVector1: %f %f %f", Vec.x, 
                                             Vec.y, 
                                             Vec.z);           
            } else {
                Error("EdgeVector1 can be used for AreaLights only");
                Result = false;
            }
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_EdgeVector2(xml_importer_data *Data, 
                               xmlTextReaderPtr XMLReader, 
                               xml_importer_state_node *StateNode, 
                               xml_importer_state *StateStack, 
                               u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingAreaLight){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Vec = StringToVec3(Value);

            light *Light = stdVectorLastElementPtr(Data->Lights);
            if(Light->Type == LightType_AreaLight){
                Light->EdgeVectors[1] = Vec;
                Light->Normal = Normalize(Cross(Light->EdgeVectors[0], 
                                                Light->EdgeVectors[1]));

                Log("EdgeVector2: %f %f %f", Vec.x, 
                                             Vec.y, 
                                             Vec.z);
            } else {
                Error("EdgeVector2 can be used for AreaLights only");
                Result = false;
            }
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_BRDFs(xml_importer_data *Data, 
                         xmlTextReaderPtr XMLReader, 
                         xml_importer_state_node *StateNode, 
                         xml_importer_state *StateStack, 
                         u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {

    }

    return Result;
}

static b32
XMLImporterOnEnter_OriginalPhong(xml_importer_data *Data, 
                                 xmlTextReaderPtr XMLReader, 
                                 xml_importer_state_node *StateNode, 
                                 xml_importer_state *StateStack, 
                                 u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingBRDFs){
        Result = false;
    } else {
        char *BRDFID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                         (xmlChar *)"id");
        if(BRDFID){
            u32 ID = StringToU32(BRDFID);
            xmlFree(BRDFID);

            brdf B = DefaultBRDF();
            B.ID = ID;
            B.Type = BRDF_Phong;
            
            Data->BRDFs.push_back(B);
            
            Log("Parsing a Phong BRDF with id %u", ID);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_ModifiedPhong(xml_importer_data *Data, 
                                 xmlTextReaderPtr XMLReader, 
                                 xml_importer_state_node *StateNode, 
                                 xml_importer_state *StateStack, 
                                 u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingBRDFs){
        Result = false;
    } else {
        char *BRDFID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                         (xmlChar *)"id");
        if(BRDFID){
            u32 ID = StringToU32(BRDFID);
            xmlFree(BRDFID);

            brdf B = DefaultBRDF();
            B.ID = ID;
            B.Type = BRDF_ModifiedPhong;
            
            char *Normalized = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                                (xmlChar *)
                                                                "normalized");
            if(Normalized){
                if(!StringCompare(Normalized, 
                                  StringLength(Normalized),
                                  "true",
                                  4)){
                    B.Type = BRDF_NormalizedModifiedPhong;
                }

                xmlFree(Normalized);
            }

            Data->BRDFs.push_back(B);
            
            Log("Parsing a Modified Phong BRDF with id %u", ID);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_OriginalBlinnPhong(xml_importer_data *Data, 
                                      xmlTextReaderPtr XMLReader, 
                                      xml_importer_state_node *StateNode, 
                                      xml_importer_state *StateStack, 
                                      u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingBRDFs){
        Result = false;
    } else {
        char *BRDFID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                         (xmlChar *)"id");
        if(BRDFID){
            u32 ID = StringToU32(BRDFID);
            xmlFree(BRDFID);

            brdf B = DefaultBRDF();
            B.ID = ID;
            B.Type = BRDF_BlinnPhong;
            
            Data->BRDFs.push_back(B);
            
            Log("Parsing a Blinn-Phong BRDF with id %u", ID);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_ModifiedBlinnPhong(xml_importer_data *Data, 
                                      xmlTextReaderPtr XMLReader, 
                                      xml_importer_state_node *StateNode, 
                                      xml_importer_state *StateStack, 
                                      u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingBRDFs){
        Result = false;
    } else {
        char *BRDFID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                         (xmlChar *)"id");
        if(BRDFID){
            u32 ID = StringToU32(BRDFID);
            xmlFree(BRDFID);

            brdf B = DefaultBRDF();
            B.ID = ID;
            B.Type = BRDF_ModifiedBlinnPhong;
            
            char *Normalized = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                                (xmlChar *)
                                                                "normalized");
            if(Normalized){
                if(!StringCompare(Normalized, 
                                  StringLength(Normalized),
                                  "true",
                                  4)){
                    B.Type = BRDF_NormalizedModifiedBlinnPhong;
                }

                xmlFree(Normalized);
            }

            Data->BRDFs.push_back(B);
            
            Log("Parsing a Modified Blinn-Phong BRDF with id %u", ID);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_TorranceSparrow(xml_importer_data *Data, 
                                   xmlTextReaderPtr XMLReader, 
                                   xml_importer_state_node *StateNode, 
                                   xml_importer_state *StateStack, 
                                   u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingBRDFs){
        Result = false;
    } else {
        char *BRDFID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                         (xmlChar *)"id");
        if(BRDFID){
            u32 ID = StringToU32(BRDFID);
            xmlFree(BRDFID);

            brdf B = DefaultBRDF();
            B.ID = ID;
            B.Type = BRDF_TorranceSparrow;
            
            Data->BRDFs.push_back(B);
            
            Log("Parsing a Torrance-Sparrow BRDF with id %u", ID);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Exponent(xml_importer_data *Data, 
                            xmlTextReaderPtr XMLReader, 
                            xml_importer_state_node *StateNode, 
                            xml_importer_state *StateStack, 
                            u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingOriginalPhong || 
       StateStack[StateStackDepth] == XMLImporterState_ParsingModifiedPhong || 
       StateStack[StateStackDepth] == XMLImporterState_ParsingOriginalBlinnPhong || 
       StateStack[StateStackDepth] == XMLImporterState_ParsingModifiedBlinnPhong || 
       StateStack[StateStackDepth] == XMLImporterState_ParsingTorranceSparrow){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 Exp = StringToF32(Value);

            brdf *B = stdVectorLastElementPtr(Data->BRDFs);
            B->Exponent = Exp;

            Log("Exponent: %f", Exp);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_RefractiveIndex(xml_importer_data *Data, 
                                   xmlTextReaderPtr XMLReader, 
                                   xml_importer_state_node *StateNode, 
                                   xml_importer_state *StateStack, 
                                   u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingTorranceSparrow){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 Idx = StringToF32(Value);

            brdf *B = stdVectorLastElementPtr(Data->BRDFs);
            B->RefractiveIndex = Idx;

            Log("RefractiveIndex: %f", Idx);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_Materials(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {

    }

    return Result;
}

static b32
XMLImporterOnEnter_Material(xml_importer_data *Data, 
                            xmlTextReaderPtr XMLReader, 
                            xml_importer_state_node *StateNode, 
                            xml_importer_state *StateStack, 
                            u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingMaterials){
        char *MaterialID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)"id");
        if(MaterialID){
            u32 ID = StringToU32(MaterialID);
            xmlFree(MaterialID);

            b32 Degamma = false;
            char *DegammaIn = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                                (xmlChar *)
                                                                "degamma");
            if(DegammaIn){
                if(!StringCompare(DegammaIn, 
                                  StringLength(DegammaIn),
                                  "true",
                                  4)){
                    Degamma = true;
                }

                xmlFree(DegammaIn);
            }

            u32 BRDFID = 0;
            
            char *BRDF = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                                (xmlChar *)
                                                                "BRDF");
            if(BRDF){
                BRDFID = StringToU32(BRDF);
                xmlFree(BRDF);
            }
            
            material M = DefaultMaterial();
            M.ID = ID;
            M.Degamma = Degamma;
            M.BRDFID = BRDFID;
            Data->Materials.push_back(M);

            Log("Parsing a material with id %u", ID);
        }
    } else if(StateStack[StateStackDepth] == XMLImporterState_ParsingMesh || 
              StateStack[StateStackDepth] == XMLImporterState_ParsingLightMesh){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            u32 MaterialID = StringToU32(Value);

            mesh *Mesh = stdVectorLastElementPtr(Data->Meshes);
            Mesh->MaterialIndex = MaterialID-1;

            Log("Mesh material id: %u", MaterialID);
        }
    } else if(StateStack[StateStackDepth] == XMLImporterState_ParsingSphere ||
              StateStack[StateStackDepth] == 
                         XMLImporterState_ParsingLightSphere ||
              StateStack[StateStackDepth] == 
                         XMLImporterState_ParsingMeshInstance){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            u32 MaterialID = StringToU32(Value);

            entity *E = stdVectorLastElementPtr(Data->Entities);
            E->MaterialIndex = MaterialID-1;

            Log("Entity material id: %u", MaterialID);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_AmbientReflectance(xml_importer_data *Data, 
                                      xmlTextReaderPtr XMLReader, 
                                      xml_importer_state_node *StateNode, 
                                      xml_importer_state *StateStack, 
                                      u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingMaterial){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Reflectance = StringToVec3(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            material *Material = stdVectorLastElementPtr(Data->Materials);
            if(Material->Degamma){
                Reflectance.x = Power(Reflectance.x, 2.2f);
                Reflectance.y = Power(Reflectance.y, 2.2f);
                Reflectance.z = Power(Reflectance.z, 2.2f);
            }

            Material->AmbientReflectance = Reflectance;

            Log("AmbientReflectance: %f %f %f", Reflectance.x, 
                                                Reflectance.y, 
                                                Reflectance.z);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_DiffuseReflectance(xml_importer_data *Data, 
                                      xmlTextReaderPtr XMLReader, 
                                      xml_importer_state_node *StateNode, 
                                      xml_importer_state *StateStack, 
                                      u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingMaterial){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Reflectance = StringToVec3(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            material *Material = stdVectorLastElementPtr(Data->Materials);
            if(Material->Degamma){
                Reflectance.x = Power(Reflectance.x, 2.2f);
                Reflectance.y = Power(Reflectance.y, 2.2f);
                Reflectance.z = Power(Reflectance.z, 2.2f);
            }

            Material->DiffuseReflectance = Reflectance;

            Log("DiffuseReflectance: %f %f %f", Reflectance.x, 
                                                Reflectance.y, 
                                                Reflectance.z);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_SpecularReflectance(xml_importer_data *Data, 
                                       xmlTextReaderPtr XMLReader, 
                                       xml_importer_state_node *StateNode, 
                                       xml_importer_state *StateStack, 
                                       u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingMaterial){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Reflectance = StringToVec3(Value);

            camera *Camera = stdVectorLastElementPtr(Data->Cameras);
            material *Material = stdVectorLastElementPtr(Data->Materials);
            if(Material->Degamma){
                Reflectance.x = Power(Reflectance.x, 2.2f);
                Reflectance.y = Power(Reflectance.y, 2.2f);
                Reflectance.z = Power(Reflectance.z, 2.2f);
            }

            Material->SpecularReflectance = Reflectance;

            Log("SpecularReflectance: %f %f %f", Reflectance.x, 
                                                 Reflectance.y, 
                                                 Reflectance.z);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_PhongExponent(xml_importer_data *Data, 
                                 xmlTextReaderPtr XMLReader, 
                                 xml_importer_state_node *StateNode, 
                                 xml_importer_state *StateStack, 
                                 u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingMaterial){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 PhongExp = StringToF32(Value);

            material *Material = stdVectorLastElementPtr(Data->Materials);
            Material->BRDF.Exponent = PhongExp;

            Log("PhongExp: %f", PhongExp);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_MirrorReflectance(xml_importer_data *Data, 
                                     xmlTextReaderPtr XMLReader, 
                                     xml_importer_state_node *StateNode, 
                                     xml_importer_state *StateStack, 
                                     u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingMaterial){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Reflectance = StringToVec3(Value);

            material *Material = stdVectorLastElementPtr(Data->Materials);
            Material->MirrorReflectance = Reflectance;

            Log("MirrorReflectance: %f %f %f", Reflectance.x, 
                                               Reflectance.y, 
                                               Reflectance.z);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Transparency(xml_importer_data *Data, 
                                xmlTextReaderPtr XMLReader, 
                                xml_importer_state_node *StateNode, 
                                xml_importer_state *StateStack, 
                                u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingMaterial){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Transparency = StringToVec3(Value);

            material *Material = stdVectorLastElementPtr(Data->Materials);
            Material->Transparency = Transparency;

            Log("Transparency: %f %f %f", Transparency.x, 
                                          Transparency.y, 
                                          Transparency.z);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_RefractionIndex(xml_importer_data *Data, 
                                   xmlTextReaderPtr XMLReader, 
                                   xml_importer_state_node *StateNode, 
                                   xml_importer_state *StateStack, 
                                   u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingMaterial){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 RefractionIndex = StringToF32(Value);

            material *Material = stdVectorLastElementPtr(Data->Materials);
            Material->RefractionIndex = RefractionIndex;

            Log("RefractionIndex: %f", RefractionIndex);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Roughness(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingMaterial){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 Roughness = StringToF32(Value);

            material *Material = stdVectorLastElementPtr(Data->Materials);
            Material->Glossiness = Roughness;

            Log("Roughness: %f", Roughness);
        }
    }

    return Result;
}


static b32
XMLImporterOnEnter_Transformations(xml_importer_data *Data, 
                                   xmlTextReaderPtr XMLReader, 
                                   xml_importer_state_node *StateNode, 
                                   xml_importer_state *StateStack, 
                                   u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingScene){
    } else if(StateStack[StateStackDepth]==XMLImporterState_ParsingMesh ||
              StateStack[StateStackDepth]==XMLImporterState_ParsingSphere ||
              StateStack[StateStackDepth]==XMLImporterState_ParsingLightMesh ||
              StateStack[StateStackDepth] == 
                         XMLImporterState_ParsingLightSphere ||
              StateStack[StateStackDepth] == 
                         XMLImporterState_ParsingMeshInstance){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            
            mat4 TransformMat = Mat4Identity();

            char *Cursor = Value;
            while(Cursor[0] != '\0'){
                if(!IsWhitespace(Cursor[0])){
                    switch(Cursor[0]){
                        case 's':{
                            u32 ID = StringToU32(Cursor+1, &Cursor);
                            ID--;
                            TransformMat = Mat4Scale(TransformMat, 
                                                     Data->Scalings[ID]);
                        } break;
                        case 'r':{
                            u32 ID = StringToU32(Cursor+1, &Cursor);
                            ID--;
                            f32 Angle = Data->Rotations[ID].x;
                            vec3 RotAxis = Data->Rotations[ID].yzw;
                            TransformMat = Mat4RotateAround(TransformMat, 
                                                            Angle, RotAxis);
                        } break;
                        case 't':{
                            u32 ID = StringToU32(Cursor+1, &Cursor);
                            ID--;
                            TransformMat = Mat4Translate(TransformMat, 
                                                   Data->Translations[ID]);
                        } break;
                        default:{
                            Error("Unexpected transformation string %c %s %zd", 
                                                               Cursor[0], 
                                                               Value, 
                                                               (Cursor-Value));
                            Result = false;
                        } break;
                    }
                } else {
                    Cursor++;
                }
            }

            mat4 InvTransformMat;
            if(!Mat4Inverse(TransformMat, &InvTransformMat)){
                InvTransformMat = Mat4Identity();
                Error("Inverse transformation matrix could not be computed");
            }

            if(StateStack[StateStackDepth]==XMLImporterState_ParsingMesh ||
               StateStack[StateStackDepth]==XMLImporterState_ParsingLightMesh){
                mesh *M = stdVectorLastElementPtr(Data->Meshes);
                M->InvTransformationMatrix = InvTransformMat;
                M->TransformationMatrix = TransformMat;
            } else {
                entity *E = stdVectorLastElementPtr(Data->Entities);
                E->InvTransformationMatrix = InvTransformMat;
                E->TransformationMatrix = TransformMat;
            }
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_Translation(xml_importer_data *Data, 
                               xmlTextReaderPtr XMLReader, 
                               xml_importer_state_node *StateNode, 
                               xml_importer_state *StateStack, 
                               u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingTransformations){
        char *TransformID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)"id");
        if(TransformID){
            u32 ID = StringToU32(TransformID);
            xmlFree(TransformID);

            char *Value = XMLReadValue(XMLReader);
            if(Value){
                vec3 Vec = StringToVec3(Value);
                if(Data->Translations.size()+1 == ID){
                    Data->Translations.push_back(Vec);
                } else {
                    Warning("TransformationIDs are expected to be sequential");
                }

                Log("Translation with id %u : (%f %f %f)", ID, 
                                                           Vec.x, 
                                                           Vec.y, 
                                                           Vec.z);
            }
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_Scaling(xml_importer_data *Data, 
                           xmlTextReaderPtr XMLReader, 
                           xml_importer_state_node *StateNode, 
                           xml_importer_state *StateStack, 
                           u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingTransformations){
        char *TransformID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)"id");
        if(TransformID){
            u32 ID = StringToU32(TransformID);
            xmlFree(TransformID);

            char *Value = XMLReadValue(XMLReader);
            if(Value){
                vec3 Vec = StringToVec3(Value);
                if(Data->Scalings.size()+1 == ID){
                    Data->Scalings.push_back(Vec);
                } else {
                    Warning("TransformationIDs are expected to be sequential");
                }

                Log("Scaling with id %u : (%f %f %f)", ID, 
                                                       Vec.x, Vec.y, Vec.z);
            }
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_Rotation(xml_importer_data *Data, 
                            xmlTextReaderPtr XMLReader, 
                            xml_importer_state_node *StateNode, 
                            xml_importer_state *StateStack, 
                            u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingTransformations){
        char *TransformID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)"id");
        if(TransformID){
            u32 ID = StringToU32(TransformID);
            xmlFree(TransformID);

            char *Value = XMLReadValue(XMLReader);
            if(Value){
                vec4 Vec = StringToVec4(Value);
                if(Data->Rotations.size()+1 == ID){
                    Data->Rotations.push_back(Vec);
                } else {
                    Warning("TransformationIDs are expected to be sequential");
                }

                Log("Rotation with id %u : %f degrees around (%f %f %f)", ID, 
                                                Vec.x, Vec.y, Vec.z, Vec.w);
            }


        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_Textures(xml_importer_data *Data, 
                            xmlTextReaderPtr XMLReader, 
                            xml_importer_state_node *StateNode, 
                            xml_importer_state *StateStack, 
                            u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {

    }

    return Result;
}

static b32
XMLImporterOnEnter_Texture(xml_importer_data *Data, 
                           xmlTextReaderPtr XMLReader, 
                           xml_importer_state_node *StateNode, 
                           xml_importer_state *StateStack, 
                           u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingTextures){
        char *TextureID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                            (xmlChar *)"id");
        char *BumpMap = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                          (xmlChar *)
                                                          "bumpmap");
        char *BumpMapMul = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                         (xmlChar *)
                                                         "bumpmapMultiplier");
        if(TextureID){
            u32 ID = StringToU32(TextureID);
            xmlFree(TextureID);

            texture T = DefaultTexture();
            T.ID = ID;
            T.Type = TextureType_2D;

            char *DegammaIn = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                                (xmlChar *)
                                                                "degamma");
            if(DegammaIn){
                if(!StringCompare(DegammaIn, 
                                  StringLength(DegammaIn),
                                  "true",
                                  4)){
                    T.Degamma = true;
                }

                xmlFree(DegammaIn);
            }

            if(BumpMap){
                if(!StringCompare(BumpMap, 
                                  StringLength(BumpMap),
                                  "true",
                                  4)){
                    T.BumpMap = true;
                }

                xmlFree(BumpMap);
            }

            if(BumpMapMul){
                f32 Multiplier = StringToF32(BumpMapMul);
                xmlFree(BumpMapMul);

                T.BumpMapMultiplier = Multiplier;
            }

            Data->Textures2D.push_back(T);

            Log("Parsing a texture with id %u", ID);
        }
    } else if(StateStack[StateStackDepth]==XMLImporterState_ParsingMesh || 
              StateStack[StateStackDepth]==XMLImporterState_ParsingLightMesh){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            u32 TextureID = StringToU32(Value);

            mesh *Mesh = stdVectorLastElementPtr(Data->Meshes);
            Mesh->TextureIndex = TextureID;

            Log("Mesh texture id: %u", TextureID);
        }
    } else if(StateStack[StateStackDepth] == XMLImporterState_ParsingSphere ||
              StateStack[StateStackDepth] == 
                         XMLImporterState_ParsingLightSphere ||
              StateStack[StateStackDepth] == 
                         XMLImporterState_ParsingMeshInstance){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            u32 TextureID = StringToU32(Value);

            entity *E = stdVectorLastElementPtr(Data->Entities);
            E->TextureIndex = TextureID;

            Log("Entity texture id: %u", TextureID);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_DecalMode(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingTexture){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            texture *T = stdVectorLastElementPtr(Data->Textures2D);

            size_t ValueLen = StringLength(Value);
            if(!StringCompare(Value, ValueLen, "replace_all", 11)){
                T->BlendMode = TextureBlendMode_ReplaceAll;
            } else if(!StringCompare(Value, ValueLen, "replace_kd", 10)){
                T->BlendMode = TextureBlendMode_ReplaceDiffuse;
            } else if(!StringCompare(Value, ValueLen, "blend_kd", 8)){
                T->BlendMode = TextureBlendMode_BlendDiffuse;
            } else {
                Result = false;
            }
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Interpolation(xml_importer_data *Data, 
                                 xmlTextReaderPtr XMLReader, 
                                 xml_importer_state_node *StateNode, 
                                 xml_importer_state *StateStack, 
                                 u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingTexture){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            texture *T = stdVectorLastElementPtr(Data->Textures2D);

            size_t ValueLen = StringLength(Value);
            if(!StringCompare(Value, ValueLen, "nearest", 7)){
                T->Filter = TextureFilter_Nearest;
            } else if(!StringCompare(Value, ValueLen, "bilinear", 8)){
                T->Filter = TextureFilter_Bilinear;
            } else {
                Result = false;
            }
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Normalizer(xml_importer_data *Data, 
                              xmlTextReaderPtr XMLReader, 
                              xml_importer_state_node *StateNode, 
                              xml_importer_state *StateStack, 
                              u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingTexture){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 Normalizer = StringToF32(Value);

            texture *T = stdVectorLastElementPtr(Data->Textures2D);
            T->Normalizer = Normalizer;

            Log("Texture normalizer %f", Normalizer);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Appearance(xml_importer_data *Data, 
                              xmlTextReaderPtr XMLReader, 
                              xml_importer_state_node *StateNode, 
                              xml_importer_state *StateStack, 
                              u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingTexture){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            texture *T = stdVectorLastElementPtr(Data->Textures2D);

            size_t ValueLen = StringLength(Value);
            if(!StringCompare(Value, ValueLen, "clamp", 5)){
                T->WrapMode = TextureWrapMode_Clamp;
            } else if(!StringCompare(Value, ValueLen, "repeat", 6)){
                T->WrapMode = TextureWrapMode_Repeat;
            } else if(!StringCompare(Value, ValueLen, "patch", 5)){
                T->PerlinAppearance = TexturePerlinAppearance_Patch;
            } else if(!StringCompare(Value, ValueLen, "vein", 4)){
                T->PerlinAppearance = TexturePerlinAppearance_Vein;
            } else {
                Result = false;
            }
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_ScalingFactor(xml_importer_data *Data, 
                                 xmlTextReaderPtr XMLReader, 
                                 xml_importer_state_node *StateNode, 
                                 xml_importer_state *StateStack, 
                                 u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingTexture){
        Result = false;
    } else {
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 ScalingFactor = StringToF32(Value);

            texture *T = stdVectorLastElementPtr(Data->Textures2D);
            T->ScalingFactor = ScalingFactor;

            Log("Perlin noise scaling factor: %f", ScalingFactor);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_VertexData(xml_importer_data *Data, 
                              xmlTextReaderPtr XMLReader, 
                              xml_importer_state_node *StateNode, 
                              xml_importer_state *StateStack, 
                              u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        char *BinaryPath = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)
                                                             "binaryFile");
        if(BinaryPath){
            char *FileContents = 0;
            u64   FileSize;
            if(ReadEntireFile((char *)BinaryPath,
                              (void **)&FileContents, 
                              &FileSize)){
        
                char *FileCursor = FileContents;
                u32 VertCount = *((u32 *) FileCursor);
                FileCursor += sizeof(u32);
        
                for(u32 Idx=0; Idx<VertCount; Idx++){
                    vec3 Vert;
                    Vert.x = *((f32 *) FileCursor);
                    FileCursor += sizeof(f32);
                    Vert.y = *((f32 *) FileCursor);
                    FileCursor += sizeof(f32);
                    Vert.z = *((f32 *) FileCursor);
                    FileCursor += sizeof(f32);
                    
                    Data->Vertices.push_back(Vert);
                    Log("Vertex: %f %f %f", Vert.x, Vert.y, Vert.z);
                }
        
                PlatformMemoryFree((void **)&FileContents);
            } else {
                Error("Could not read file: %s", BinaryPath);
                Result = false;
            }
        
            xmlFree(BinaryPath);
        } else {
            char *Value = XMLReadValue(XMLReader);
            if(Value){
                char *VertexList = Value;

                while(true){
                    while(IsWhitespace(VertexList[0])){
                        VertexList++;
                    }
                    
                    if(VertexList[0] == '\0'){
                        break;
                    }

                    vec3 Vertex = StringToVec3(VertexList, &VertexList);

                    Data->Vertices.push_back(Vertex);
                    Log("Vertex: %f %f %f", Vertex.x, Vertex.y, Vertex.z);
                }
            }
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_TexCoordData(xml_importer_data *Data, 
                                xmlTextReaderPtr XMLReader, 
                                xml_importer_state_node *StateNode, 
                                xml_importer_state *StateStack, 
                                u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        char *BinaryPath = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)
                                                             "binaryFile");
        if(BinaryPath){
            char *FileContents = 0;
            u64   FileSize;
            if(ReadEntireFile((char *)BinaryPath,
                              (void **)&FileContents, 
                              &FileSize)){
        
                char *FileCursor = FileContents;
                u32 UVCount = *((u32 *) FileCursor);
                FileCursor += sizeof(u32);
        
                for(u32 Idx=0; Idx<UVCount; Idx++){
                    vec2 UV;
                    UV.u = *((f32 *) FileCursor);
                    FileCursor += sizeof(f32);
                    UV.v = *((f32 *) FileCursor);
                    FileCursor += sizeof(f32);
        
                    Data->UVs.push_back(UV);
                    Log("UV: %f %f", UV.x, UV.y);
                }
        
                PlatformMemoryFree((void **)&FileContents);
            } else {
                Error("Could not read file: %s", BinaryPath);
                Result = false;
            }
        
            xmlFree(BinaryPath);
        } else {
            char *Value = XMLReadValue(XMLReader);
            if(Value){
                char *UVList = Value;
    
                while(true){
                    while(IsWhitespace(UVList[0])){
                        UVList++;
                    }
                    
                    if(UVList[0] == '\0'){
                        break;
                    }
    
                    vec2 UV = StringToVec2(UVList, &UVList);
    
                    Data->UVs.push_back(UV);
                    Log("UV: %f %f", UV.x, UV.y);
                }
            }
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Objects(xml_importer_data *Data, 
                           xmlTextReaderPtr XMLReader, 
                           xml_importer_state_node *StateNode, 
                           xml_importer_state *StateStack, 
                           u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingScene){
        Result = false;
    } else {
        
    }

    return Result;
}

static b32
XMLImporterOnEnter_Mesh(xml_importer_data *Data, 
                        xmlTextReaderPtr XMLReader, 
                        xml_importer_state_node *StateNode, 
                        xml_importer_state *StateStack, 
                        u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingObjects){
        Result = false;
    } else {
        char *MeshID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                         (xmlChar *)"id");
        if(MeshID){
            u32 ID = StringToU32(MeshID);
            xmlFree(MeshID);

            mesh M = DefaultMesh();
            M.ID = ID;
            M.ShadingMode = ShadingMode_Flat;

            char *Shading = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                              (xmlChar *)
                                                              "shadingMode");
            if(Shading){
                size_t ShadingLen = StringLength(Shading);
                if(!StringCompare(Shading, ShadingLen, "smooth", 6)){
                    M.ShadingMode = ShadingMode_Smooth;
                } else if(!StringCompare(Shading, ShadingLen, "flat", 4)){
                    M.ShadingMode = ShadingMode_Flat;
                } else {
                    Warning("Unrecognized shading mode. Using flat");
                }
                xmlFree(Shading);
            }

            Data->Meshes.push_back(M);

            Log("Parsing a mesh with id %u", ID);
        } else {
            Result = false;
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Faces(xml_importer_data *Data, 
                         xmlTextReaderPtr XMLReader, 
                         xml_importer_state_node *StateNode, 
                         xml_importer_state *StateStack, 
                         u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingMesh || 
       StateStack[StateStackDepth] == XMLImporterState_ParsingLightMesh){
        s32 UVOffset = 0;
        char *UVOffsetStr = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)
                                                             "textureOffset");
        if(UVOffsetStr){
            UVOffset = StringToS32(UVOffsetStr);
            xmlFree(UVOffsetStr);
        }

        u32 VertOffset = 0;
        char *VertOffsetStr = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                               (xmlChar *)
                                                               "vertexOffset");
        if(VertOffsetStr){
            VertOffset = StringToU32(VertOffsetStr);
            xmlFree(VertOffsetStr);
        }

        mesh *Mesh = stdVectorLastElementPtr(Data->Meshes);
        Mesh->UVOffset = UVOffset - (s32)VertOffset;

        char *PlyPath = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                          (xmlChar *)
                                                          "plyFile");

        char *BinaryPath = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)
                                                             "binaryFile");
        if(PlyPath){
            // NOTE(furkan): Read mesh data from an external file

            u32 MeshIndex = (u32)Data->Meshes.size()-1;
            Assert(MeshIndex >= 0);

            u32 PlyIndex = (u32)Data->PlyFiles.size();
#if 0
            // TODO(furkan): Support this. Do not load same file 
            // again and again
            b32 Duplicate = false;
            for(u32 P=0; P<PlyIndex; P++){
                if(Data->PlyFiles[P].compare(PlyPath) == 0){
                    PlyIndex = P;
                    Duplicate = true;
                }
            }
            if(!Duplicate){
                Data->PlyFiles.push_back(std::string(PlyPath));
            }
#else
            Data->PlyFiles.push_back(std::string(PlyPath)); 
#endif
            xmlFree(PlyPath);

            Data->MeshPlyPairs.push_back({MeshIndex, PlyIndex});

            // NOTE(furkan): FaceBaseIndex and FaceCount for the mesh 
            // will be determined later while importing ply data.
            Data->FaceBaseIndices.push_back(0);
            Data->FaceCounts.push_back(0);
        } else if(BinaryPath){

            char *FileContents = 0;
            u64   FileSize;
            if(ReadEntireFile((char *)BinaryPath,
                              (void **)&FileContents, 
                              &FileSize)){

                char *FileCursor = FileContents;
                u32 FaceCount = *((u32 *) FileCursor);
                FileCursor += sizeof(u32);

                Data->FaceBaseIndices.push_back((u32)Data->Faces.size());

                for(u32 F=0; F<FaceCount; F++){
                    vec3u Face;
                    Face.x = *((u32 *) FileCursor);
                    FileCursor += sizeof(u32);
                    Face.y = *((u32 *) FileCursor);
                    FileCursor += sizeof(u32);
                    Face.z = *((u32 *) FileCursor);
                    FileCursor += sizeof(u32);

                    Face += VertOffset;

                    if(!Data->ZeroBasedIndexing){
                        Face.x--; Face.y--; Face.z--;
                    }


                    Data->Faces.push_back(Face);
                    Log("Face: %u %u %u", Face.x, Face.y, Face.z)
                }

                Data->FaceCounts.push_back(FaceCount);

                PlatformMemoryFree((void **)&FileContents);
            } else {
                Error("Could not read file: %s", BinaryPath);
                Result = false;
            }

            xmlFree(BinaryPath);
        } else {
            // NOTE(furkan): Read mesh data from XML file
            char *Value = XMLReadValue(XMLReader);
            if(Value){
                char *FaceList = Value;

                Data->FaceBaseIndices.push_back((u32)Data->Faces.size());

                u32 FaceCount = 0;
                while(true){
                    while(IsWhitespace(FaceList[0])){
                        FaceList++;
                    }
                    
                    if(FaceList[0] == '\0'){
                        break;
                    }

                    vec3u Face = StringToVec3u(FaceList, &FaceList);
                    Face += VertOffset;

                    if(!Data->ZeroBasedIndexing){
                        Face.x--; Face.y--; Face.z--;
                    }

                    Data->Faces.push_back(Face);
                    FaceCount++;
                    Log("Face: %u %u %u", Face.x, Face.y, Face.z)
                }

                Data->FaceCounts.push_back(FaceCount);
            }
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_MeshInstance(xml_importer_data *Data, 
                                xmlTextReaderPtr XMLReader, 
                                xml_importer_state_node *StateNode, 
                                xml_importer_state *StateStack, 
                                u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingObjects){
        Result = false;
    } else {
        char *InstanceID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)"id");
        char *BaseMeshID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)
                                                             "baseMeshId");
        char *ResetTransform = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                             (xmlChar *)
                                                             "resetTransform");
        if(InstanceID){
            u32 ID = StringToU32(InstanceID);
            xmlFree(InstanceID);

            if(BaseMeshID){
                u32 MeshID = StringToU32(BaseMeshID);
                xmlFree(BaseMeshID);

                entity E = DefaultEntity();
                E.BaseMeshID = MeshID-1;
                if(ResetTransform){
                    if(!StringCompare(ResetTransform, 
                                      StringLength(ResetTransform),
                                      "true",
                                      4)){
                        E.ResetTransform = true;
                    }

                    xmlFree(ResetTransform);
                }
                E.ID = ID;
                E.Type = EntityType_MeshInstance;

                Data->Entities.push_back(E);

                Log("Parsing a mesh instance with id %u", ID);
            } else {
                Result = false;
            }
        } else {
            Result = false;
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Sphere(xml_importer_data *Data, 
                          xmlTextReaderPtr XMLReader, 
                          xml_importer_state_node *StateNode, 
                          xml_importer_state *StateStack, 
                          u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingObjects){
        Result = false;
    } else {
        char *SphereID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                           (xmlChar *)"id");
        if(SphereID){
            u32 ID = StringToU32(SphereID);
            xmlFree(SphereID);

            entity E = DefaultEntity();
            E.ID = ID;
            E.Type = EntityType_Sphere;
            Data->Entities.push_back(E);
            
            Log("Parsing a sphere with id %u", ID);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_Center(xml_importer_data *Data, 
                          xmlTextReaderPtr XMLReader, 
                          xml_importer_state_node *StateNode, 
                          xml_importer_state *StateStack, 
                          u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingSphere || 
       StateStack[StateStackDepth] == XMLImporterState_ParsingLightSphere){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            u32 Center = StringToU32(Value);
    
            Data->SphereCenters.push_back(Center-1);

            Log("Center: %u", Center);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_Radius(xml_importer_data *Data, 
                          xmlTextReaderPtr XMLReader, 
                          xml_importer_state_node *StateNode, 
                          xml_importer_state *StateStack, 
                          u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingSphere || 
       StateStack[StateStackDepth] == XMLImporterState_ParsingLightSphere){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            f32 Radius = StringToF32(Value);

            entity *Entity = stdVectorLastElementPtr(Data->Entities);
            Entity->Radius = Radius;

            Log("Radius: %f", Radius);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_MotionBlur(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] == XMLImporterState_ParsingMesh ||
       StateStack[StateStackDepth] == XMLImporterState_ParsingLightMesh){
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Vec = StringToVec3(Value);

            mesh *Mesh = stdVectorLastElementPtr(Data->Meshes);
            Mesh->MotionVector = Vec;

            Log("Mesh motion: (%f %f %f)", Vec.x, Vec.y, Vec.z);
        }
    } else if(StateStack[StateStackDepth] == XMLImporterState_ParsingSphere ||
              StateStack[StateStackDepth] == 
                         XMLImporterState_ParsingLightSphere ||
              StateStack[StateStackDepth] == 
                         XMLImporterState_ParsingMeshInstance){
        // TODO(furkan): Test a scene with a LightSphere with MotionBlur
        char *Value = XMLReadValue(XMLReader);
        if(Value){
            vec3 Vec = StringToVec3(Value);

            entity *E = stdVectorLastElementPtr(Data->Entities);
            E->MotionVector = Vec;

            Log("Entity motion: (%f %f %f)", Vec.x, Vec.y, Vec.z);
        }
    } else {
        Result = false;
    }

    return Result;
}

static b32
XMLImporterOnEnter_LightSphere(xml_importer_data *Data, 
                               xmlTextReaderPtr XMLReader, 
                               xml_importer_state_node *StateNode, 
                               xml_importer_state *StateStack, 
                               u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingObjects){
        Result = false;
    } else {
        char *SphereID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                           (xmlChar *)"id");
        if(SphereID){
            u32 ID = StringToU32(SphereID);
            xmlFree(SphereID);

            entity E = DefaultEntity();
            E.ID = ID;
            E.Type = EntityType_Sphere;
            Data->Entities.push_back(E);

            u32 LSIndex = (u32)Data->Entities.size()-1;
            Data->EntityLightIndices.push_back(LSIndex);
            
            Log("Parsing a light sphere with id %u", ID);
        }
    }

    return Result;
}

static b32
XMLImporterOnEnter_LightMesh(xml_importer_data *Data, 
                             xmlTextReaderPtr XMLReader, 
                             xml_importer_state_node *StateNode, 
                             xml_importer_state *StateStack, 
                             u32 StateStackDepth){
    b32 Result = true;

    if(StateStack[StateStackDepth] != XMLImporterState_ParsingObjects){
        Result = false;
    } else {
        char *MeshID = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                         (xmlChar *)"id");
        if(MeshID){
            u32 ID = StringToU32(MeshID);
            xmlFree(MeshID);

            mesh M = DefaultMesh();
            M.ID = ID;
            M.ShadingMode = ShadingMode_Flat;

            char *Shading = (char *)xmlTextReaderGetAttribute(XMLReader, 
                                                              (xmlChar *)
                                                              "shadingMode");
            if(Shading){
                size_t ShadingLen = StringLength(Shading);
                if(!StringCompare(Shading, ShadingLen, "smooth", 6)){
                    M.ShadingMode = ShadingMode_Smooth;
                } else if(!StringCompare(Shading, ShadingLen, "flat", 4)){
                    M.ShadingMode = ShadingMode_Flat;
                } else {
                    Warning("Unrecognized shading mode. Using flat");
                }
                xmlFree(Shading);
            }

            Data->Meshes.push_back(M);

            u32 LMIndex = (u32)Data->Meshes.size()-1;
            Data->LightMeshIndices.push_back(LMIndex);

            Log("Parsing a light mesh with id %u", ID);
        } else {
            Result = false;
        }
    }

    return Result;
}

#pragma warning(pop) 

static xml_importer_state_node *
FindStateNodeByName(xml_importer_state_node *Nodes, u32 NodeCount,
                    char *Name, u32 NameLength){
    xml_importer_state_node *Result = 0;

    for(u32 Index=0;
            Index < NodeCount;
            Index++){
        xml_importer_state_node *S = Nodes + Index;
        if(!StringCompare(Name, NameLength, S->Name, S->NameLength)){
            Result = S;
            break;
        }
    }

    return Result;
}

static b32
ImportXML(scene *Scene, camera **Cameras, u32 *CameraCount, 
          epsilon_values *Epsilons, char *XMLFilePath){
    b32 Success = false;

    ProfilerRecordStart(ImportXML);

#ifdef LIBXML_READER_ENABLED
    LIBXML_TEST_VERSION
    
    char *Encoding = 0;
    s32 XMLReaderOptions = XML_PARSE_NONET | XML_PARSE_HUGE;
    xmlTextReaderPtr XMLReader = xmlReaderForFile(XMLFilePath, 
                                                  Encoding, 
                                                  XMLReaderOptions);
    if(XMLReader){
        Success = true;

        xml_importer_state_node StateNodes[] = {
            XMLImporterStateNode(Scene),
            XMLImporterStateNode(ShadowRayEpsilon),
            XMLImporterStateNode(IntersectionTestEpsilon),
            XMLImporterStateNode(ZeroBasedIndexing),
            XMLImporterStateNode(MaxRecursionDepth),
            XMLImporterStateNode(BackgroundColor),
            XMLImporterStateNode(BackgroundTexture),
            XMLImporterStateNode(Integrator),
            XMLImporterStateNode(IntegratorParams),
            XMLImporterStateNode(Cameras),
            XMLImporterStateNode(Camera),
            XMLImporterStateNode(Position),
            XMLImporterStateNode(Gaze),
            XMLImporterStateNode(Up),
            XMLImporterStateNode(NearPlane),
            XMLImporterStateNode(NearDistance),
            XMLImporterStateNode(FocusDistance),
            XMLImporterStateNode(ApertureSize),
            XMLImporterStateNode(ImageResolution),
            XMLImporterStateNode(NumSamples),
            XMLImporterStateNode(ImageName),
            XMLImporterStateNode(GazePoint),
            XMLImporterStateNode(FovY),
            XMLImporterStateNode(GammaCorrection),
            XMLImporterStateNode(Tonemap),
            XMLImporterStateNode(TMO),
            XMLImporterStateNode(TMOOptions),
            XMLImporterStateNode(Saturation),
            XMLImporterStateNode(Gamma),
            XMLImporterStateNode(Lights),
            XMLImporterStateNode(SphericalDirectionalLight),
            XMLImporterStateNode(EnvMapName),
            XMLImporterStateNode(AmbientLight),
            XMLImporterStateNode(PointLight),
            XMLImporterStateNode(AreaLight),
            XMLImporterStateNode(DirectionalLight),
            XMLImporterStateNode(SpotLight),
            XMLImporterStateNode(Intensity),
            XMLImporterStateNode(Direction),
            XMLImporterStateNode(Radiance),
            XMLImporterStateNode(CoverageAngle),
            XMLImporterStateNode(FalloffAngle),
            XMLImporterStateNode(EdgeVector1),
            XMLImporterStateNode(EdgeVector2),
            XMLImporterStateNode(BRDFs),
            XMLImporterStateNode(OriginalPhong),
            XMLImporterStateNode(ModifiedPhong),
            XMLImporterStateNode(OriginalBlinnPhong),
            XMLImporterStateNode(ModifiedBlinnPhong),
            XMLImporterStateNode(TorranceSparrow),
            XMLImporterStateNode(Exponent),
            XMLImporterStateNode(RefractiveIndex),
            XMLImporterStateNode(Materials),
            XMLImporterStateNode(Material),
            XMLImporterStateNode(AmbientReflectance),
            XMLImporterStateNode(DiffuseReflectance),
            XMLImporterStateNode(SpecularReflectance),
            XMLImporterStateNode(PhongExponent),
            XMLImporterStateNode(MirrorReflectance),
            XMLImporterStateNode(Transparency),
            XMLImporterStateNode(RefractionIndex),
            XMLImporterStateNode(Roughness),
            XMLImporterStateNode(Transformations),
            XMLImporterStateNode(Translation),
            XMLImporterStateNode(Scaling),
            XMLImporterStateNode(Rotation),
            XMLImporterStateNode(Textures),
            XMLImporterStateNode(Texture),
            XMLImporterStateNode(DecalMode),
            XMLImporterStateNode(Interpolation),
            XMLImporterStateNode(Normalizer),
            XMLImporterStateNode(Appearance),
            XMLImporterStateNode(ScalingFactor),
            XMLImporterStateNode(VertexData),
            XMLImporterStateNode(TexCoordData),
            XMLImporterStateNode(Objects),
            XMLImporterStateNode(Mesh),
            XMLImporterStateNode(Faces),
            XMLImporterStateNode(MeshInstance),
            XMLImporterStateNode(Sphere),
            XMLImporterStateNode(Center),
            XMLImporterStateNode(Radius),
            XMLImporterStateNode(MotionBlur),
            XMLImporterStateNode(LightSphere),
            XMLImporterStateNode(LightMesh),
        };

        xml_importer_data Data;
        Data.Scene = Scene;
        Data.Epsilons = Epsilons;
        Data.ZeroBasedIndexing = false;

        xml_importer_state StateStack[XMLImporterStateStackCapacity];
        StateStack[0] = XMLImporterState_Root;
        u32 StateStackDepth = 0;

        while(Success){
            s32 ReadResult = xmlTextReaderRead(XMLReader);
            if(ReadResult == -1){
                Success = false;
                Error("An error occurred in xmlTextReaderRead");
            } else if(ReadResult == 0){
                Log("Finished reading");
                break;
            } else {
                Assert(ReadResult == 1);

                const xmlChar *NodeName = xmlTextReaderConstName(XMLReader);
                u32 NodeNameLength = 0;
                if(NodeName){
                    NodeNameLength = (u32)StringLength((char *)NodeName);
                }

                s32 NodeType = xmlTextReaderNodeType(XMLReader);
                switch(NodeType){
                    case XMLNodeType_Element:{
                        // NOTE(furkan): Opening node
                        xml_importer_state_node *S = FindStateNodeByName(
                                                   StateNodes,
                                                   StaticArraySize(StateNodes),
                                                   (char *)NodeName, 
                                                   NodeNameLength);
                        if(S){
                            Success = S->OnEnter(&Data, XMLReader, S, 
                                                 StateStack, StateStackDepth);
                            S->PastVisitCount++;
                            if(StateStackDepth < 
                               XMLImporterStateStackCapacity){
                                StateStackDepth++;
                                StateStack[StateStackDepth] = S->State;
                            } else {
                                Error("XMLImporterStateStack is full");
                                Success = false;
                            }

                            // TODO(furkan): Find a way to detect
                            // <Element attr="val" /> kind of nodes
                            // in libxml
                            if((S->State == 
                                      XMLImporterState_ParsingFaces ||
                                S->State == 
                                      XMLImporterState_ParsingVertexData || 
                                S->State == 
                                      XMLImporterState_ParsingTexCoordData) &&
                               !xmlTextReaderHasValue(XMLReader)){
                                StateStackDepth--;
                            }
                        } else {
                            Error("Unexpected node name: %s", NodeName);
                            Success = false;
                        }
                    } break;
                    case XMLNodeType_EndElement:{
                        // NOTE(furkan): Closing node
                        xml_importer_state_node *S = FindStateNodeByName(
                                                   StateNodes,
                                                   StaticArraySize(StateNodes),
                                                   (char *)NodeName, 
                                                   NodeNameLength);
                        if(S){
                            if(StateStack[StateStackDepth] != S->State){
                                Error("Corrupt XML structure");
                                Success = false;
                            }

                            if(StateStackDepth > 0){
                                StateStackDepth--;
                            } else {
                                Error("StateStackDepth must be >= 0");
                                Success = false;
                            }
                        } else {
                            Error("Unexpected node name: %s", NodeName);
                            Success = false;
                        }
                    } break;
                    case XMLNodeType_Comment:{
                        // NOTE(furkan): Do nothing
                    } break;
                    case XMLNodeType_SignificantWhitespace:{
                        // NOTE(furkan): Do nothing
                    } break;
                    default:{
                        Warning("Unexpected node type: %d", NodeType);
                    } break;
                }
            }
        }
        
        if(Success){
            if(Data.Cameras.size() > 0){
                if(STDVectorToArray((void **)Cameras, Data.Cameras)){
                    *CameraCount = (u32)Data.Cameras.size();
                } else {
                    Success = false;
                }
                Data.Cameras.clear();
            }

            if(Data.Meshes.size() > 0){
                if(STDVectorToArray((void **)&Scene->Meshes, Data.Meshes)){
                    Scene->MeshCount = (u32)Data.Meshes.size();
                } else {
                    Success = false;
                }
                Data.Meshes.clear();
            }

            if(Data.Materials.size() > 0){
                for(u32 MatIdx=0; MatIdx<Data.Materials.size(); MatIdx++){
                    if(Data.Materials[MatIdx].BRDFID){
                        u32 BRDFIdx = Data.Materials[MatIdx].BRDFID-1;
                        if(BRDFIdx < Data.BRDFs.size()){
                            Data.Materials[MatIdx].BRDF = Data.BRDFs[BRDFIdx];
                        } else {
                            Error("Out-of-bounds BRDF index");
                            Success = false;
                            break;
                        }
                    }
                }
                
                if(STDVectorToArray((void **)&Scene->Materials, 
                                    Data.Materials)){
                    Scene->MaterialCount = (u32)Data.Materials.size();
                } else {
                    Success = false;
                }
                Data.Materials.clear();           
            }

            if(Data.Textures2D.size() > 0){
                if(STDVectorToArray((void **)&Scene->Textures, 
                                    Data.Textures2D)){
                    Scene->TextureCount = (u32)Data.Textures2D.size();
                } else {
                    Success = false;
                }
                Data.Textures2D.clear();
            }

            u32 SphereCenterCount = (u32)Data.SphereCenters.size();
            u32 EntityCount = (u32)Data.Entities.size();
            u32 EntityIndex = 0;
            for(u32 CenterIndex = 0;
                    CenterIndex < SphereCenterCount;
                    CenterIndex++){

                while(EntityIndex < EntityCount &&
                      Data.Entities[EntityIndex].Type != EntityType_Sphere){
                    EntityIndex++;
                }
                
                if(EntityIndex < EntityCount){
                    u32 VertexIndex = Data.SphereCenters[CenterIndex];
                    if(VertexIndex < Data.Vertices.size()){
                        Data.Entities[EntityIndex].Center = 
                                                   Data.Vertices[VertexIndex];
                        EntityIndex++;
                    } else {
                        Error("Specified vertex does not exist");
                        Success = false;
                    }
                } else {
                    Error("Unused sphere center data exist");
                    Success = false;
                    break;
                }
            }
            Data.SphereCenters.clear();

            u32 PlyCount = (u32)Data.PlyFiles.size();
            for(u32 Ply=0; Ply<PlyCount; Ply++){
                const char *FilePath = Data.PlyFiles[Ply].c_str();
                char *FileContents = 0;
                u64   FileSize;
                if(ReadEntireFile((char *)FilePath,
                                  (void **)&FileContents, 
                                  &FileSize)){
                    if(FileContents[0] == 'p' &&
                       FileContents[1] == 'l' &&
                       FileContents[2] == 'y'){
                        char *FileCursor = FileContents+3;
                        size_t WordLen;
                        char *Word = StringNextWord(FileCursor, 
                                                    &WordLen, 
                                                    &FileCursor);
                        if(!StringCompare(Word, WordLen, 
                                          "format", 6)){
                            Word = StringNextWord(FileCursor, 
                                                  &WordLen, 
                                                  &FileCursor);

                            ply_format PlyFormat = PlyFormat_ASCII;
                            if(!StringCompare(Word, WordLen, 
                                              "binary_little_endian", 20)){
                                PlyFormat = PlyFormat_Binary;
                            } else if(!StringCompare(Word, WordLen, 
                                                     "ascii", 5)){
                                PlyFormat = PlyFormat_ASCII;
                            } else {
                                Error("Unknown ply format: %s", Word);
                            }

                            f32 Version = StringToF32(FileCursor, &FileCursor);

                            Word = StringNextWord(FileCursor, 
                                                  &WordLen, 
                                                  &FileCursor);
                            while(!StringCompare(Word, WordLen, 
                                                 "comment", 7)){
                                size_t LineLen;
                                char *Line = StringNextLine(FileCursor, 
                                                            &LineLen, 
                                                            &FileCursor);
                                Log("[PLY]: %s", Line);
                                Word = StringNextWord(FileCursor, 
                                                      &WordLen, 
                                                      &FileCursor);
                            }
                            
                            u32 PlyVertCount = 0;
                            u32 PlyFaceCount = 0;
                            u32 ElementSizeVertex = 0;
                            u32 ElementSizeFace = 0;
                            u32 FaceListIndexSize = 0;
                            u32 FaceIndexSize = 0;
                            b32 ParsePerVertexNormals = false;
                            b32 ParsePerTexCoords = false;
                            while(StringCompare(Word, WordLen, 
                                                "end_header", 10)){
                                b32 ParsingVert = false;
                                b32 ParsingFace = false;
                                if(!StringCompare(Word, WordLen, 
                                                  "element", 7)){
                                    Word = StringNextWord(FileCursor, 
                                                          &WordLen, 
                                                          &FileCursor);
                                    if(!StringCompare(Word, WordLen, 
                                                      "vertex", 6)){
                                        PlyVertCount = StringToU32(FileCursor, 
                                                                   &FileCursor);
                                        ParsingVert = true;
                                    } else if (!StringCompare(Word, WordLen, 
                                                              "face", 4)){
                                        PlyFaceCount = StringToU32(FileCursor, 
                                                                   &FileCursor);
                                        ParsingFace = true;
                                    } else {
                                        Error("Unexpected element: %s", Word);
                                    }

                                    Word = StringNextWord(FileCursor, 
                                                          &WordLen, 
                                                          &FileCursor);
                                    while(!StringCompare(Word, WordLen, 
                                                         "property", 8)){
                                        Word = StringNextWord(FileCursor, 
                                                              &WordLen, 
                                                              &FileCursor);
                                        if(!StringCompare(Word, WordLen, 
                                                          "float", 5)){
                                            if(ParsingVert){
                                                ElementSizeVertex += 4;
                                            } else {
                                                Assert(ParsingFace);
                                                ElementSizeFace += 4;
                                            }

                                            // NOTE(furkan): Ignore property 
                                            // name
                                            Word = StringNextWord(FileCursor, 
                                                                  &WordLen, 
                                                                  &FileCursor);

                                            if(!StringCompare(Word, WordLen,
                                                              "nx", 2)){
                                                ParsePerVertexNormals = true;
                                            }

                                            if(!StringCompare(Word, WordLen,
                                                              "u", 1) || 
                                                !StringCompare(Word, WordLen,
                                                              "s", 1)){
                                                ParsePerTexCoords = true;
                                            }
                                        } else if(!StringCompare(Word, WordLen, 
                                                                 "list", 4)){
                                            Word = StringNextWord(FileCursor, 
                                                                  &WordLen, 
                                                                  &FileCursor);
                                            if(!StringCompare(Word, WordLen, 
                                                              "uint8", 5) ||
                                               !StringCompare(Word, WordLen, 
                                                              "uchar", 5)){
                                                FaceListIndexSize = 1;
                                            } else {
                                                Error("Invalid list index");
                                            }

                                            Word = StringNextWord(FileCursor, 
                                                                  &WordLen, 
                                                                  &FileCursor);
                                            if(!StringCompare(Word, WordLen, 
                                                              "int", 3) || 
                                               !StringCompare(Word, WordLen, 
                                                              "uint", 4)){
                                                FaceIndexSize = 4;
                                            } else {
                                                Error("Invalid face index");
                                            }

                                            // NOTE(furkan): Ignore list 
                                            // name
                                            Word = StringNextWord(FileCursor, 
                                                                  &WordLen, 
                                                                  &FileCursor);
                                        } else {
                                            Error("Unknown property type: %s", 
                                                                        Word);
                                        }

                                        Word = StringNextWord(FileCursor, 
                                                              &WordLen, 
                                                              &FileCursor);
                                    }
                                } else {
                                    Error("Expected: 'element', Found: '%s'", 
                                                                       Word);
                                }
                            }

                            // NOTE(furkan): Skip until next line
                            StringNextLine(FileCursor, &WordLen, &FileCursor);

                            u32 VertexOffset = (u32)Data.Vertices.size();
                            u32 FaceOffset = (u32)Data.Faces.size();
                            u32 InsertedFaceCount = 0;
                            if(PlyFormat == PlyFormat_ASCII){
                                for(u32 V=0; V<PlyVertCount; V++){
                                    vec3 Vert = StringToVec3(FileCursor, 
                                                             &FileCursor);
                                    Data.Vertices.push_back(Vert);

                                    // TODO(furkan): Make it optional to use 
                                    // normals from ply files
                                    vec3 N = StringToVec3(FileCursor, 
                                                          &FileCursor);

                                    vec2 UV = StringToVec2(FileCursor, 
                                                          &FileCursor);
                                    Data.UVs.push_back(UV);
                                }

                                for(u32 F=0; F<PlyFaceCount; F++){
                                    u32 VertCount = StringToU32(FileCursor, 
                                                                &FileCursor);

                                    if(VertCount == 3){
                                        vec3u Faces = StringToVec3u(FileCursor,
                                                                    &FileCursor);
                                        Faces += VertexOffset;
                                        Data.Faces.push_back(Faces);
                                        InsertedFaceCount++;
                                    } else if(VertCount == 4){
                                        u32 FaceX = StringToU32(FileCursor, 
                                                                &FileCursor) + 
                                                    VertexOffset;
                                        u32 FaceY = StringToU32(FileCursor, 
                                                                &FileCursor) + 
                                                    VertexOffset;
                                        u32 FaceZ = StringToU32(FileCursor, 
                                                                &FileCursor) + 
                                                    VertexOffset;
                                        u32 FaceW = StringToU32(FileCursor, 
                                                                &FileCursor) + 
                                                    VertexOffset;
                                        Data.Faces.push_back( {FaceX, 
                                                               FaceY, 
                                                               FaceZ} );
                                        Data.Faces.push_back( {FaceX, 
                                                               FaceZ, 
                                                               FaceW} );
                                        InsertedFaceCount += 2;
                                    } else {
                                         Error("Number of edges per face must be 3 or 4");
                                    }

                                    // TODO(furkan): Per face attributes other 
                                    // than vertex indices are not handled
                                }
                            } else if(PlyFormat == PlyFormat_Binary){

                                for(u32 V=0; V<PlyVertCount; V++){
                                    vec3 Vert;
                                    Vert.x = *((f32 *) FileCursor);
                                    FileCursor += sizeof(f32);
                                    Vert.y = *((f32 *) FileCursor);
                                    FileCursor += sizeof(f32);
                                    Vert.z = *((f32 *) FileCursor);
                                    FileCursor += sizeof(f32);

                                    Data.Vertices.push_back(Vert);

                                    vec3 Normal;
                                    Normal.x = *((f32 *) FileCursor);
                                    FileCursor += sizeof(f32);
                                    Normal.y = *((f32 *) FileCursor);
                                    FileCursor += sizeof(f32);
                                    Normal.z = *((f32 *) FileCursor);
                                    FileCursor += sizeof(f32);

                                    vec2 UV;
                                    UV.x = *((f32 *) FileCursor);
                                    FileCursor += sizeof(f32);
                                    UV.y = *((f32 *) FileCursor);
                                    FileCursor += sizeof(f32);

                                    Data.UVs.push_back(UV);
                                }

                                for(u32 F=0; F<PlyFaceCount; F++){
                                    u32 VertCount = (u32)(*((u8 *) FileCursor));
                                    FileCursor += sizeof(u8);

                                    if(VertCount == 3){
                                        vec3u Faces;
                                        Faces.x = *((u32 *) FileCursor);
                                        FileCursor += sizeof(u32);
                                        Faces.y = *((u32 *) FileCursor);
                                        FileCursor += sizeof(u32);
                                        Faces.z = *((u32 *) FileCursor);
                                        FileCursor += sizeof(u32);

                                        Faces += VertexOffset;
                                        Data.Faces.push_back(Faces);
                                        InsertedFaceCount++;
                                    } else if(VertCount == 4){
                                        u32 FaceX = *((u32 *) FileCursor) + 
                                                    VertexOffset;
                                        FileCursor += sizeof(u32);
                                        u32 FaceY = *((u32 *) FileCursor) + 
                                                    VertexOffset;
                                        FileCursor += sizeof(u32);
                                        u32 FaceZ = *((u32 *) FileCursor) + 
                                                    VertexOffset;
                                        FileCursor += sizeof(u32);
                                        u32 FaceW = *((u32 *) FileCursor) + 
                                                    VertexOffset;
                                        FileCursor += sizeof(u32);

                                        Data.Faces.push_back( {FaceX, 
                                                               FaceY, 
                                                               FaceZ} );
                                        Data.Faces.push_back( {FaceX, 
                                                               FaceZ, 
                                                               FaceW} );
                                        InsertedFaceCount += 2;
                                    } else {
                                         Error("Number of edges per face must be 3 or 4");
                                    }

                                    FileCursor += ElementSizeFace;
                                }
                            }

                            u32 PairCount = (u32)Data.MeshPlyPairs.size();
                            for(u32 Pair=0; Pair<PairCount; Pair++){
                                mesh_ply_pair P = Data.MeshPlyPairs[Pair];
                                if(P.PlyIndex == Ply){
                                    u32 MIdx = P.MeshIndex;
                                    mesh *M = Scene->Meshes + MIdx;
                                    M->UVOffset -= VertexOffset;
                                    Data.FaceBaseIndices[MIdx] = FaceOffset;
                                    Data.FaceCounts[MIdx] = InsertedFaceCount;
                                }
                            }
                        } else {
                            Error("Expected: 'format', Found: '%s'", Word);
                        }
                    } else {
                        Error("Ply signature does not match for %s", FilePath);
                        Success = false;
                    }

                    PlatformMemoryFree((void **)&FileContents);
                } else {
                    Error("Could not read file: %s", FilePath);
                    Success = false;
                }
            }

            std::vector<vec3> PerVertexNormals;
            u32 TotalVertexCount = (u32)Data.Vertices.size();
            for(u32 Idx=0;
                    Idx<TotalVertexCount;
                    Idx++){
                PerVertexNormals.push_back(Vec3(0.0f, 0.0f, 0.0f));
            }

            // TODO(furkan): Refactor here
            u32 TotalTriangleCount = (u32)Data.Faces.size();
            for(u32 TriIndex=0;
                    TriIndex<TotalTriangleCount;
                    TriIndex++){
                vec3u Face = Data.Faces[TriIndex];

                vec3 P0 = Data.Vertices[Face.x];
                vec3 P1 = Data.Vertices[Face.y];
                vec3 P2 = Data.Vertices[Face.z];

                vec3 FaceNormal = Cross(P1-P0, P2-P0);
#if 1
                PerVertexNormals[Face.x] += FaceNormal;
                PerVertexNormals[Face.y] += FaceNormal;
                PerVertexNormals[Face.z] += FaceNormal;
#else
                PerVertexNormals[Face.x] += Normalize(FaceNormal);
                PerVertexNormals[Face.y] += Normalize(FaceNormal);
                PerVertexNormals[Face.z] += Normalize(FaceNormal);
#endif
            }

            for(u32 Idx=0;
                    Idx<TotalVertexCount;
                    Idx++){
                PerVertexNormals[Idx] = Normalize(PerVertexNormals[Idx]);
            }

            for(u32 MeshIndex=0;
                    MeshIndex < Scene->MeshCount;
                    MeshIndex++){
                mesh *Mesh = Scene->Meshes + MeshIndex;

                s32 UVOffset = Mesh->UVOffset;
                u32 FaceBaseIndex = Data.FaceBaseIndices[MeshIndex];
                u32 FaceCount = Data.FaceCounts[MeshIndex];
                entity *TriEntities = 0;
                triangle *Triangles = 0;
                if(MemoryAllocate((void **)&TriEntities, 
                                  sizeof(entity) * FaceCount) &&
                   MemoryAllocate((void **)&Triangles, 
                                  sizeof(triangle) * FaceCount)){

                    f32 TotalArea = 0.0f;
                    u32 LightMeshCount = (u32)Data.LightMeshIndices.size();
                    for(u32 FaceIndex = 0;
                            FaceIndex < FaceCount;
                            FaceIndex++){
                        entity *T = TriEntities + FaceIndex;
                        T->ShadingMode = Mesh->ShadingMode;
                        T->Radiance = Mesh->Radiance;
                        T->IsEmitter = Mesh->IsEmitter;
                        T->TextureIndex = Mesh->TextureIndex;
                        T->MaterialIndex = Mesh->MaterialIndex;
                        T->ID = 0;
                        T->Type = EntityType_Triangle;

                        vec3u Face = Data.Faces[FaceBaseIndex + 
                                                FaceIndex];

                        vec3 P0 = Data.Vertices[Face.x];
                        vec3 P1 = Data.Vertices[Face.y];
                        vec3 P2 = Data.Vertices[Face.z];

                        T->P0 = P0;
                        T->P1 = P1;
                        T->P2 = P2;

                        vec3 FaceNormal = Cross(P1-P0, P2-P0);
                        f32 FaceNormalLen = Length(FaceNormal);

                        TotalArea += FaceNormalLen/2.0f;

                        FaceNormal.x /= FaceNormalLen;
                        FaceNormal.y /= FaceNormalLen;
                        FaceNormal.z /= FaceNormalLen;
                        T->FaceNormal = FaceNormal;
 

                        T->N0 = PerVertexNormals[Face.x];
                        T->N1 = PerVertexNormals[Face.y];
                        T->N2 = PerVertexNormals[Face.z];

                        s32 UV0 = (s32)Face.x + UVOffset;
                        s32 UV1 = (s32)Face.y + UVOffset;
                        s32 UV2 = (s32)Face.z + UVOffset;

                        if(UV0 < Data.UVs.size()){
                            T->UV0 = Data.UVs[UV0];
                        }

                        if(UV1 < Data.UVs.size()){
                            T->UV1 = Data.UVs[UV1];
                        }

                        if(UV2 < Data.UVs.size()){
                            T->UV2 = Data.UVs[UV2];
                        }

                        triangle *Tri = Triangles + FaceIndex;
                        Tri->P0 = P0;
                        Tri->P1 = P1;
                        Tri->P2 = P2;
                        Tri->N  = FaceNormal;
                    }

                    Mesh->Triangles = Triangles;
                    Mesh->TotalArea = TotalArea;
                    Mesh->TriangleCount = FaceCount;

                    if(!ConstructBVHTrianglesOnly(&Mesh->BVH,
                                                  TriEntities,
                                                  FaceCount)){
                        Error("BVH construction has failed for a mesh");
                        Success = false;
                    } else {
                        entity Instance = DefaultEntity();
                        Instance.BaseMeshID = MeshIndex;
                        Instance.ResetTransform = false;
                        Instance.MotionVector = Mesh->MotionVector;
                        Instance.Radiance = Mesh->Radiance;
                        Instance.IsEmitter = Mesh->IsEmitter;
                        Instance.TextureIndex = Mesh->TextureIndex;
                        Instance.MaterialIndex = Mesh->MaterialIndex;
                        Instance.Type = EntityType_MeshInstance;

                        Data.Entities.push_back(Instance);

                        if(LightMeshCount){
                            if(Data.LightMeshIndices[0] == MeshIndex){
                                Data.LightMeshIndices.erase(Data.LightMeshIndices.begin());
                                LightMeshCount--;

                                u32 EIndex = (u32)Data.Entities.size()-1;
                                Data.EntityLightIndices.push_back(EIndex);
                            }
                        }
                    }
                } else {
                    Error("Memory allocation failed");
                    Success = false;
                }
            }

            Data.Vertices.clear();
            Data.Faces.clear();
            Data.UVs.clear();
            Data.FaceBaseIndices.clear();
            Data.FaceCounts.clear();

            EntityCount = (u32)Data.Entities.size();
            for(EntityIndex=0;
                EntityIndex<EntityCount;
                EntityIndex++){
                entity E = Data.Entities[EntityIndex];
                if(E.Type == EntityType_MeshInstance){
                    E.Mesh = Scene->Meshes + E.BaseMeshID;
                    E.AABB = E.Mesh->BVH->AABB;
                    if(!E.ResetTransform){
                        E.TransformationMatrix = E.TransformationMatrix * 
                                                 E.Mesh->TransformationMatrix;
                        E.InvTransformationMatrix = E.Mesh->InvTransformationMatrix * 
                                                    E.InvTransformationMatrix;
                    }

                    vec4 Corners[] = {
                        Vec4(E.AABB.Min.x, E.AABB.Min.y, 
                             E.AABB.Min.z, 1.0f),
                        Vec4(E.AABB.Min.x, E.AABB.Min.y, 
                             E.AABB.Max.z, 1.0f),
                        Vec4(E.AABB.Min.x, E.AABB.Max.y, 
                             E.AABB.Min.z, 1.0f),
                        Vec4(E.AABB.Min.x, E.AABB.Max.y, 
                             E.AABB.Max.z, 1.0f),
                        Vec4(E.AABB.Max.x, E.AABB.Min.y, 
                             E.AABB.Min.z, 1.0f),
                        Vec4(E.AABB.Max.x, E.AABB.Min.y, 
                             E.AABB.Max.z, 1.0f),
                        Vec4(E.AABB.Max.x, E.AABB.Max.y, 
                             E.AABB.Min.z, 1.0f),
                        Vec4(E.AABB.Max.x, E.AABB.Max.y, 
                             E.AABB.Max.z, 1.0f)
                    };

                    aabb TAABB = { Vec3(f32_Max, f32_Max, f32_Max), 
                                   Vec3(f32_Min, f32_Min, f32_Min) };

                    for(u32 CornerIdx=0;
                            CornerIdx<8;
                            CornerIdx++){
                        vec4 V = Corners[CornerIdx];
                        vec4 TV = E.TransformationMatrix * V;

                        Assert(TV.w > 0.999f && TV.w < 1.001f);

                        if(TAABB.Min.x > TV.x){ TAABB.Min.x = TV.x; }
                        if(TAABB.Min.y > TV.y){ TAABB.Min.y = TV.y; }
                        if(TAABB.Min.z > TV.z){ TAABB.Min.z = TV.z; }
                        if(TAABB.Max.x < TV.x){ TAABB.Max.x = TV.x; }
                        if(TAABB.Max.y < TV.y){ TAABB.Max.y = TV.y; }
                        if(TAABB.Max.z < TV.z){ TAABB.Max.z = TV.z; }
                    }

                    if(Length(E.MotionVector) > 0.0f){
                        aabb InitialAABB = TAABB;

                        TAABB.Min += E.MotionVector;
                        TAABB.Max += E.MotionVector;

                        if(TAABB.Min.x > InitialAABB.Min.x){
                            TAABB.Min.x = InitialAABB.Min.x;
                        }
                        if(TAABB.Min.y > InitialAABB.Min.y){
                            TAABB.Min.y = InitialAABB.Min.y;
                        }
                        if(TAABB.Min.z > InitialAABB.Min.z){
                            TAABB.Min.z = InitialAABB.Min.z;
                        }
                     
                        if(TAABB.Max.x < InitialAABB.Max.x){
                            TAABB.Max.x = InitialAABB.Max.x;
                        }
                        if(TAABB.Max.y < InitialAABB.Max.y){
                            TAABB.Max.y = InitialAABB.Max.y;
                        }
                        if(TAABB.Max.z < InitialAABB.Max.z){
                            TAABB.Max.z = InitialAABB.Max.z;
                        }
                    }

                    TAABB.Center = TAABB.Min*0.5f + TAABB.Max*0.5f;
                    E.AABB = TAABB;
                    Data.Entities[EntityIndex] = E;
                }
            }

            // NOTE(furkan): Entities are stored in BVH. So, free them 
            // through BVH, if it is needed.
            entity *Entities = 0;
            if(STDVectorToArray((void **)&Entities, Data.Entities)){
                EntityCount = (u32)Data.Entities.size();
                for(u32 EIndex = 0;
                        EIndex < EntityCount;
                        EIndex++){
                    entity *E = Entities+EIndex;
                    E->UniqueEntityID = EIndex;
                }
            } else {
                Success = false;
            }
            Data.Entities.clear();

            u32 EntityLightCount = (u32)Data.EntityLightIndices.size();
            if(EntityLightCount){
                for(u32 LIndex = 0; 
                        LIndex < EntityLightCount;
                        LIndex++){
                    light L;
                    L.Entity = Entities[Data.EntityLightIndices[LIndex]];
                    L.ID = LIndex;
                    L.Type = LightType_EntityLight;
                    Data.Lights.push_back(L);
                }
            }

            if(Data.Lights.size() > 0){
                if(STDVectorToArray((void **)&Scene->Lights, Data.Lights)){
                    Scene->LightCount = (u32)Data.Lights.size();
                } else {
                    Success = false;
                }
                Data.Lights.clear();
            }

            if(!ConstructBVH(&Scene->BVHRoot, Entities, EntityCount)){
                Error("BVH construction has failed for the scene");
                Success = false;
            }
        }

        xmlFreeTextReader(XMLReader);
    } else {
        Error("xmlReaderForFile failed!");
    }
#endif

    ProfilerRecordEnd(ImportXML);
    
    return Success;
}
