#if _WIN32
    #include "azimuth_platform_win32.cpp"
#elif
    #error "Platform layer is not implemented for this platform!"
#endif

#define SetPlatformAPI(PlatformPrefix);                                                                                               \
    platform_get_processor_core_count        *PlatformGetProcessorCoreCount   = PlatformPrefix##GetProcessorCoreCount;                \
    platform_get_available_memory            *PlatformGetAvailableMemory      = PlatformPrefix##GetAvailableMemory;                   \
    platform_query_timer                     *PlatformQueryTimer              = PlatformPrefix##QueryTimer;                           \
    platform_query_timer_frequency           *PlatformQueryTimerFrequency     = PlatformPrefix##QueryTimerFrequency;                  \
    platform_memory_allocate                 *PlatformMemoryAllocate          = PlatformPrefix##MemoryAllocate;                       \
    platform_memory_free                     *PlatformMemoryFree              = PlatformPrefix##MemoryFree;                           \
    platform_memory_copy                     *PlatformMemoryCopy              = PlatformPrefix##MemoryCopy;                           \
    platform_open_file                       *PlatformOpenFile                = PlatformPrefix##OpenFile;                             \
    platform_get_file_size                   *PlatformGetFileSize             = PlatformPrefix##GetFileSize;                          \
    platform_read_file_32                    *PlatformReadFile32              = PlatformPrefix##ReadFile32;                           \
    platform_set_file_pointer                *PlatformSetFilePointer          = PlatformPrefix##SetFilePointer;                       \
    platform_close_file_handle               *PlatformCloseFileHandle         = PlatformPrefix##CloseFileHandle;                      \
    platform_create_thread                   *PlatformCreateThread            = PlatformPrefix##CreateThread;                         \
    platform_wait_for_threads                *PlatformWaitForThreads          = PlatformPrefix##WaitForThreads;                       \
    platform_close_thread_handle             *PlatformCloseThreadHandle       = PlatformPrefix##CloseThreadHandle;                    \
    platform_atomic_add_f32                  *PlatformAtomicAddF32            = PlatformPrefix##AtomicAddF32                          
    

typedef b32 platform_get_processor_core_count(u32 *Result);
typedef b32 platform_get_available_memory(u64 *Result);
typedef b32 platform_query_timer(timer *Timer);
typedef b32 platform_query_timer_frequency(timer_freq *Freq);
typedef b32 platform_memory_allocate(void **Ptr, size_t AllocSize);
typedef b32 platform_memory_free(void **Ptr);
typedef b32 platform_memory_copy(void *Dest, size_t DestSize, 
                                 void *Src, size_t SrcSize);
typedef b32 platform_open_file(file_handle *Handle, char *Path, 
                               u32 FileAccessMode, u32 FileShareMode, 
                               file_creation_attrib FileCreationAttrib);
typedef b32 platform_get_file_size(file_handle FileHandle, u64 *FileSize);
typedef b32 platform_read_file_32(file_handle File, void *MemoryPtr, 
                                  u32 BytesToRead, u32 *BytesRead);
typedef b32 platform_set_file_pointer(file_handle File, s64 SeekDistance, 
                                      file_seek_pivot Pivot);
typedef b32 platform_close_file_handle(file_handle Handle);
typedef b32 platform_create_thread(thread_handle *Handle, 
                                   platform_thread_proc *ThreadProc, 
                                   void *Params, u32 *ThreadID, 
                                   size_t StackSize);
typedef b32 platform_wait_for_threads(u32 ThreadCount, 
                                      thread_handle *ThreadHandles, 
                                      b32 WaitAll, b32 WaitInfinitely, 
                                      u32 WaitTimeLimit);
typedef b32 platform_close_thread_handle(thread_handle Handle);
typedef b32 platform_atomic_add_f32(f32 volatile *Dest, f32 Val);

#if _WIN32
    SetPlatformAPI(Win32);
#elif

#endif
