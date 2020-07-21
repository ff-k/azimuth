static b32
CheckImageFileFormat(file_format *DetectedFormat, 
                     char *Filename, size_t FilenameLength){
    b32 Result = false;

    char *SupportedExtensions[] = {
        ".png" , ".PNG",
        ".bmp" , ".BMP",
        ".tga" , ".TGA",
        ".jpg" , ".JPG",
        ".jpeg", ".JPEG",
        ".exr" , ".EXR",
    };
    
    file_format SupportedFormats[] = {
        FileFormat_PNG, FileFormat_PNG,
        FileFormat_BMP, FileFormat_BMP,
        FileFormat_TGA, FileFormat_TGA,
        FileFormat_JPG, FileFormat_JPG,
        FileFormat_JPG, FileFormat_JPG,
        FileFormat_EXR, FileFormat_EXR
    };
    
#pragma warning(push)
#pragma warning(disable : 4127)
    Assert(StaticArraySize(SupportedFormats) == 
           StaticArraySize(SupportedExtensions));
#pragma warning(pop)
    
    for(u32 FIndex=0; 
            FIndex<StaticArraySize(SupportedFormats); 
            FIndex++){
        char *Ext = SupportedExtensions[FIndex];
        size_t ExtLen = StringLength(Ext);
        if(FilenameLength > ExtLen){
            if(!StringCompare(Filename+(FilenameLength-ExtLen), ExtLen, 
                              Ext, ExtLen)){
                *DetectedFormat = SupportedFormats[FIndex];
                Result = true;
                break;
            }
        }
    }

    return Result;
}

static b32
ReadEntireFile(char *Path, void **Memory, u64 *FileSize){
    // TODO(furkan): Currently, it does not support files larger than
    // 4GBs.

    b32 Success = false;

    // NOTE(furkan): Memory must not be allocated beforehand
    Assert(*Memory == 0);

    file_handle FileHandle;
    if(PlatformOpenFile(&FileHandle, Path, 
                        FileAccessMode_Read,
                        FileShareMode_Read,
                        FileCreationAttrib_OpenIfExist)){
        if(PlatformGetFileSize(FileHandle, FileSize)){
            if(PlatformMemoryAllocate(Memory, *FileSize)){
                Assert(*FileSize < u32_Max)
                if(PlatformReadFile32(FileHandle, *Memory, (u32)*FileSize, 0)){
                    Success = true;
                } else {
                    if(!PlatformMemoryFree(Memory)){
                        Warning("PlatformMemoryFree in ReadEntireFile failed");
                    }
                }
            }
        }

        if(!PlatformCloseFileHandle(FileHandle)){
            Warning("PlatformCloseFileHandle in ReadEntireFile failed");
        }
    }

    return Success;
}

