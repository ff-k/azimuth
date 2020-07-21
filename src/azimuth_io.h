typedef enum {
    FileFormat_PNG,
    FileFormat_BMP,
    FileFormat_TGA,
    FileFormat_JPG,
    FileFormat_EXR
} file_format;

typedef enum {
    FileAccessMode_Read  = 0x1,
    FileAccessMode_Write = 0x2,
} file_access_mode;

typedef enum {
    FileShareMode_Read   = 0x1,
    FileShareMode_Write  = 0x2,
    FileShareMode_Delete = 0x4,
} file_share_mode;

typedef enum {
    FileCreationAttrib_CreateAlways      = 0x01,
    FileCreationAttrib_CreateIfNotExist  = 0x02,
    FileCreationAttrib_OpenAlways        = 0x04,
    FileCreationAttrib_OpenIfExist       = 0x08,
    FileCreationAttrib_OpenAndTruncate   = 0x10,
} file_creation_attrib;

typedef enum {
    FileSeekPivot_Begin,
    FileSeekPivot_Current,
    FileSeekPivot_End
} file_seek_pivot;

static b32 CheckImageFileFormat(file_format *DetectedFormat, 
                                char *Filename, size_t FilenameLength);

static b32 ReadEntireFile(char *Path, void **Memory, u64 *FileSize);
