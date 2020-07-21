#if AzimuthDevelopmentBuild

    #include <stdio.h>
    
    #if defined(_MSC_VER) && AzimuthUseVisualStudioConsole
        #include <windows.h>
        #include <stdarg.h>
        
        #define DebugMessage(Label, ...) \
            VisualStudioDebugMessage_("%s ", Label); \
            VisualStudioDebugMessage_(__VA_ARGS__); \
            VisualStudioDebugMessage_(" (%s:%d, %s)\n", __FUNCTION__, __LINE__, __FILE__);
        #if AzimuthVerboseLog
            #define Log(...) DebugMessage("[ DEBUG ]", __VA_ARGS__);
        #else
            #define Log(...)
        #endif
        #define Warning(...) DebugMessage("[WARNING]", __VA_ARGS__);
        #define Error(...) DebugMessage("[ ERROR ]", __VA_ARGS__);
        
        static void VisualStudioDebugMessage_(char * Format, ...){
            char MessageBuffer[1024];
            int MessageLength;
            
            va_list VA_Args;
            va_start(VA_Args, Format);
            
            MessageLength = _vsnprintf_s(MessageBuffer, 1024, 1024, Format, 
                                                                      VA_Args);
            
            if (MessageLength > -1){
                OutputDebugStringA(MessageBuffer);
            } else {
                OutputDebugStringA("[ ERROR ] : MessageBuffer is not large enough to print a message! Check VisualStudioDebugMessage_ function.");
            }
            
            va_end(VA_Args);
        }
    #else
        #define DebugMessage(Label, ...) \
            printf("%s ", Label); \
            printf(__VA_ARGS__); \
            printf(" (%s:%d, %s)\n", __FUNCTION__, __LINE__, __FILE__);
        #if AzimuthVerboseLog
            #define Log(...) DebugMessage("[ DEBUG ]", __VA_ARGS__);
        #else
            #define Log(...)
        #endif
        #define Warning(...) DebugMessage("[WARNING]", __VA_ARGS__);
        #define Error(...) DebugMessage("[ ERROR ]", __VA_ARGS__);
    #endif

#define UnexpectedDefaultCase \
    default:{ \
        Error("Encountered an unexpected default case"); \
    } break;
#else
    #define Log(...)
    #define Warning(...)
    #define Error(...)
    #define UnexpectedDefaultCase
#endif

#define MacroString(x) #x

#define ZeroStruct(Str) memset((Str), 0, sizeof(*Str))
#define Assert(Expression) if(!(Expression)) { *((int *)0) = 0; }
#define StaticArraySize(Array) (sizeof((Array)) / sizeof((Array)[0]))
#define OffsetOf(Type, Member) offsetof(Type, Member)

int CompareF32(const void *A, const void *B){
    int Result = 0;

    f32 Diff = *((f32 *)A) - *((f32 *)B);

    if(Diff > 0.0f){
        Result =  1;
    } else if(Diff < 0.0f){
        Result = -1;
    }

    return Result;
}
