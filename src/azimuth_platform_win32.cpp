#include "azimuth_platform_win32.h"

static void
Win32LogErrorString(DWORD ErrCode){
    char *MessageBuffer = 0;
    DWORD FormatResult = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  0,
                  ErrCode,
                  MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                  MessageBuffer,
                  0, 0);
    if(FormatResult){
        Error("%s", MessageBuffer);
    } else {
        Error("FormatMessage failed!");
    }
    if(MessageBuffer){
        LocalFree(MessageBuffer);
    }
}

static void
Win32LogErrorString(){
    DWORD ErrCode = GetLastError();
    Win32LogErrorString(ErrCode);
}

static b32
Win32MemoryAllocate(void **Ptr, size_t AllocSize){
    b32 Success = false;
    
    // TODO(furkan): Implement separate platform layer functions for 
    // HeapAlloc and VirtualAlloc. Then, use the one which operates 
    // VirtualAlloc to write application's own memory management
    // component

    if(Ptr && AllocSize > 0){
#if 0
        *Ptr = VirtualAlloc(0, AllocSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        if(*Ptr){
            Success = true;
        } else {
            Error("VirtualAlloc error: %d", GetLastError());
            Win32LogErrorString();
        }
#else
        *Ptr = HeapAlloc(GetProcessHeap(), 0, AllocSize);
        if(*Ptr){
            Success = true;
        } else {
            Error("HeapAlloc error");
            // NOTE(furkan): According to MSDN, HeapAlloc does not call 
            // SetLastError() which is pretty weird.
        }
#endif
    }
    
    return Success;
}

static b32
Win32MemoryFree(void **Ptr){
    b32 Success = false;
    
    if(Ptr){
        if(*Ptr){
#if 0
            if(VirtualFree(*Ptr, 0, MEM_RELEASE)){
                Success = true;
                *Ptr = 0;
            } else {
                Error("VirtualFree error: %d", GetLastError());
                Win32LogErrorString();
            }
#else
           if(HeapFree(GetProcessHeap(), 0, *Ptr)){
                Success = true;
                *Ptr = 0;
           } else {
                Error("HeapFree error: %d", GetLastError());
                Win32LogErrorString();
           }
#endif
        }    
    }
    
    return Success;
}

static b32
Win32MemoryCopy(void *Dest, size_t DestSize, void *Src, size_t SrcSize){
    b32 Success = true;

    size_t MinSize = MinOf(DestSize, SrcSize);
    if(MinSize != SrcSize){
        Success = false;
        Warning("Destination memory is not large enough");
    }

    CopyMemory(Dest, Src, MinSize);

    return Success;
}

static b32
Win32OpenFile(file_handle *Handle, char *Path, u32 FileAccessMode, u32 FileShareMode, file_creation_attrib FileCreationAttrib){
    b32 Success = false;
    
    if(Handle){
        if(Path){
            u32 FileAccess = 0;
            if(FileAccessMode & FileAccessMode_Read){
                FileAccess = FileAccess | GENERIC_READ;
            }
            
            if(FileAccessMode & FileAccessMode_Write){
                FileAccess = FileAccess | GENERIC_WRITE;
            }
            
            u32 FileShare = 0;
            if(FileShareMode & FileShareMode_Read){
                FileShare = FileShare | FILE_SHARE_READ;
            }
            
            if(FileShareMode & FileShareMode_Write){
                FileShare = FileShare | FILE_SHARE_WRITE;
            }
            
            if(FileShareMode & FileShareMode_Delete){
                FileShare = FileShare | FILE_SHARE_DELETE;
            }
            
            u32 FileCreation = 0;
            switch(FileCreationAttrib){
                case FileCreationAttrib_CreateAlways:{
                    FileCreation = CREATE_ALWAYS;
                } break;
                case FileCreationAttrib_CreateIfNotExist:{
                    FileCreation = CREATE_NEW;
                } break;
                case FileCreationAttrib_OpenAlways:{
                    FileCreation = OPEN_ALWAYS;
                } break;
                case FileCreationAttrib_OpenIfExist:{
                    FileCreation = OPEN_EXISTING;
                } break;
                case FileCreationAttrib_OpenAndTruncate:{
                    FileCreation = TRUNCATE_EXISTING;
                } break;
                default:{
                    Error("Unexpected file creation attrib! (%u)", FileCreationAttrib);
                } break;
            }
            
            if(FileCreation){
                Handle->Ptr = CreateFileA(Path, FileAccess, FileShare, 0, FileCreation, 0, 0);
                if(Handle->Ptr && Handle->Ptr != INVALID_HANDLE_VALUE){
                    Success = true;
                } else {
                    Error("CreateFile error: %d", GetLastError());
                    Win32LogErrorString();
                }
            }
        }
    }
    
    return Success;
}

static b32
Win32GetFileSize(file_handle FileHandle, u64 *FileSize){
    b32 Success = false;

    if(FileSize){
        LARGE_INTEGER Size;
        if(GetFileSizeEx(FileHandle.Ptr, &Size)){
            *FileSize = Size.QuadPart;
            Success = true;
        } else {
            Error("GetFileSizeEx error: %d", GetLastError());
            Win32LogErrorString();
        }    
    }
    
    return Success;
}

static b32
Win32ReadFile32(file_handle File, void *MemoryPtr, u32 BytesToRead, u32 *BytesRead){
    b32 Success = false;
    
    if(File.Ptr && File.Ptr != INVALID_HANDLE_VALUE){
        if(MemoryPtr){
            if(BytesToRead > 0){
                Success = ReadFile(File.Ptr, MemoryPtr, BytesToRead, (LPDWORD)BytesRead, 0);
                if(!Success){
                    Error("ReadFile error: %d", GetLastError());
                    Win32LogErrorString();
                }
            }
        }
    }
    
    return Success;
}

static b32
Win32SetFilePointer(file_handle File, s64 SeekDistance, file_seek_pivot Pivot){
    b32 Success = false;
    
    u32 MoveMethod = u32_Max;
    switch(Pivot){
        case FileSeekPivot_Begin:{
            MoveMethod = FILE_BEGIN;
        } break;
        case FileSeekPivot_Current:{
            MoveMethod = FILE_CURRENT;
        } break;
        case FileSeekPivot_End:{
            MoveMethod = FILE_END;
        } break;
        default:{
            Error("Unexpected file seek pivot! (%u)", Pivot);
        } break;
    }
    
    if(MoveMethod != u32_Max){
        LARGE_INTEGER Seek;
        Seek.QuadPart = SeekDistance;
        if(SetFilePointerEx(File.Ptr, Seek, 0, MoveMethod)){
            Success = true;
        } else {
            Error("SetFilePointerEx error: %d", GetLastError());
            Win32LogErrorString();
        }
    }
    
    return Success;
}

static b32
Win32CloseFileHandle(file_handle Handle){
    b32 Success = false;
    
    if(CloseHandle(Handle.Ptr)){
        Success = true;
    } else {
        Error("CloseHandle error: %d", GetLastError());
        Win32LogErrorString();
    }
    
    return Success;
}

static b32
Win32GetProcessorCoreCount(u32 *Result){
    b32 Success = false;
    
    if(Result){
        SYSTEM_INFO SysInfo;
        GetNativeSystemInfo(&SysInfo);
        
        *Result = SysInfo.dwNumberOfProcessors;
        Success = true;
    }
    
    return Success;
}

static b32
Win32GetAvailableMemory(u64 *Result){
    b32 Success = false;
    
    if(Result){
        MEMORYSTATUSEX MemoryStatus;
        MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
        if(GlobalMemoryStatusEx(&MemoryStatus)){
            *Result = MemoryStatus.ullAvailPhys;
            Success = true;
        } else {
            Error("GlobalMemoryStatusEx failed! (Error: %d)", GetLastError());
            Win32LogErrorString();
        }
    }
    
    return Success;
}

static b32
Win32QueryTimer(timer *Timer){
    b32 Success = false;
    
    if(Timer){
        LARGE_INTEGER Counter;
        if(QueryPerformanceCounter(&Counter)){
            Timer->Counter = Counter.QuadPart;
            Success = true;
        } else {
            Error("QueryPerformanceCounter failed! (Error: %d)", GetLastError());
            Win32LogErrorString();
        }
    }
    
    return Success;
}

static b32
Win32QueryTimerFrequency(timer_freq *Freq){
    b32 Success = false;
    
    if(Freq){
        LARGE_INTEGER QueryData;
        if(QueryPerformanceFrequency(&QueryData)){
            Freq->Value = QueryData.QuadPart;
            Success = true;
        } else {
            Error("QueryPerformanceFrequency failed! (Error: %d)", GetLastError());
            Win32LogErrorString();
        }
    }
    
    return Success;
}

static b32
Win32CreateThread(thread_handle *Handle, DWORD (*ThreadProc)(void *), void *Params, u32 *ThreadID, size_t StackSize){
    b32 Success = false;
    
    if(Handle){
        Handle->Ptr = CreateThread(0 , StackSize, (LPTHREAD_START_ROUTINE)ThreadProc, Params, 0, (LPDWORD)ThreadID);
        if(Handle->Ptr){
            Success = true;
        } else {
            Error("CreateThread error: %d", GetLastError());
            Win32LogErrorString();
        }
    }
    
    return Success;
}

static b32
Win32WaitForThreads(u32 ThreadCount, thread_handle *ThreadHandles, b32 WaitAll, b32 WaitInfinitely, u32 WaitTimeLimit){
    b32 Success = false;
    
    if(WaitInfinitely){
        WaitTimeLimit = INFINITE;
    }
    
    u32 Result = WaitForMultipleObjects(ThreadCount, (HANDLE *)ThreadHandles, WaitAll, WaitTimeLimit);
    if(Result != WAIT_FAILED){
        Success = true;
    } else {
        Error("WaitForMultipleObjects error: %d", GetLastError());
        Win32LogErrorString();
    }
    
    return Success;
}

static b32
Win32CloseThreadHandle(thread_handle Handle){
    b32 Success = false;
    
    if(CloseHandle(Handle.Ptr)){
        Success = true;
    } else {
        Error("CloseHandle error: %d", GetLastError());
        Win32LogErrorString();
    }
    
    return Success;
}

inline b32 
Win32AtomicAddF32(f32 volatile *Dest, f32 Val){
    b32 Result = true;

    while(true){
        volatile f32 InitialVal = *Dest;
        volatile f32 Sum = InitialVal + Val;

        // TODO(furkan): Try InterlockedCompareExchangeNoFence
        LONG Old = InterlockedCompareExchange(*((LONG volatile **)&Dest), 
                                              *((LONG *)&Sum), 
                                              *((LONG *)&InitialVal));
        if(*((f32 *)&Old) == InitialVal){
            break;
        }
    }

    return Result;
}
