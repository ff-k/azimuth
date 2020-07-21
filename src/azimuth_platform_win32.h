#include <windows.h>

typedef struct file_handle {
    HANDLE Ptr;
} file_handle;

typedef struct thread_handle {
    HANDLE Ptr;
} thread_handle;

typedef struct timer {
    u64 Counter;
} timer;

typedef struct timer_freq {
    u64 Value;
} timer_freq;

#define PlatformThreadProc(ProcName) \
    DWORD __cdecl ProcName(LPVOID Parameters)
typedef PlatformThreadProc(platform_thread_proc);
