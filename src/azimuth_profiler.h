#if AzimuthProfilerEnabled

// NOTE(furkan): We support recursive calls
// which are not deeper than ProfilerRecordDepth 
#define ProfilerRecordDepth 64

typedef struct {
    u64 TimeStart;
    u64 TimeEnd;
} profiler_record;

typedef struct {
    profiler_record ActiveRecords[ProfilerRecordDepth];
    u64 TimeRecordedTotal;
    char *ID;
    u32 ActiveRecordIndex;
} profiler_entry;

typedef struct {
    profiler_record GlobalRecord;
    profiler_entry *Entries;
    u32 EntryCount;
} profiler_table;

static profiler_table ProfilerTable;

#define ProfilerBuildTagPostfix_(Function, File)                             \
    Function##", "##File
#define ProfilerBuildTagPostfix(Function, File)                              \
    ProfilerBuildTagPostfix_(Function, File)
#define ProfilerBuildTag_(Base, Line, Function, File)                        \
    Base##" ("MacroString(Line)" in "##ProfilerBuildTagPostfix(Function, File)
#define ProfilerBuildTag(TagBase) ProfilerBuildTag_(TagBase,                 \
                                                    __LINE__,                \
                                                    __FUNCTION__,            \
                                                    __FILE__)

#define ProfilerInitialize()                                                 \
    {                                                                        \
        timer ProfilerTimer = { 0 };                                         \
        PlatformQueryTimer(&ProfilerTimer);                                  \
        ProfilerTable.GlobalRecord.TimeStart = ProfilerTimer.Counter;        \
        u32 EntryCount = __COUNTER__;                                        \
        if(EntryCount){                                                      \
            size_t EntryTableSize = sizeof(profiler_entry) * EntryCount;     \
            if(PlatformMemoryAllocate((void **)&ProfilerTable.Entries,       \
                                       EntryTableSize)){                     \
                ZeroMemory(ProfilerTable.Entries, EntryTableSize);           \
                ProfilerTable.EntryCount = EntryCount;                       \
            } else {                                                         \
                Error("Unable to allocate profiler table");                  \
            }                                                                \
        }                                                                    \
    }

#define ProfilerRecordStart(Tag)                                             \
    profiler_entry *Tag##Entry = 0;                                          \
    {                                                                        \
        u32 Index = __COUNTER__;                                             \
        Tag##Entry = ProfilerTable.Entries + Index;                          \
        Tag##Entry->ID = ProfilerBuildTag(MacroString(Tag));                 \
                                                                             \
        u32 RecordIndex = Tag##Entry->ActiveRecordIndex;                     \
        Assert(RecordIndex < ProfilerRecordDepth);                           \
        Tag##Entry->ActiveRecordIndex++;                                     \
                                                                             \
        profiler_record *Record = Tag##Entry->ActiveRecords + RecordIndex;   \
        timer ProfilerTimer = { 0 };                                         \
        if(PlatformQueryTimer(&ProfilerTimer)){                              \
            Record->TimeStart = ProfilerTimer.Counter;                       \
        } else {                                                             \
            Error("Unable to record time");                                  \
        }                                                                    \
    }

#define ProfilerRecordEnd(Tag)                                               \
    {                                                                        \
        Tag##Entry->ActiveRecordIndex--;                                     \
        u32 RecordIndex = Tag##Entry->ActiveRecordIndex;                     \
        Assert(RecordIndex != 0xFFFFFFFF);                                   \
                                                                             \
        profiler_record *Record = Tag##Entry->ActiveRecords + RecordIndex;   \
        timer ProfilerTimer = { 0 };                                         \
        if(PlatformQueryTimer(&ProfilerTimer)){                              \
            Record->TimeEnd = ProfilerTimer.Counter;                         \
            u64 TimeElapsed = Record->TimeEnd - Record->TimeStart;           \
            Tag##Entry->TimeRecordedTotal += TimeElapsed;                    \
        } else {                                                             \
            Error("Unable to record time");                                  \
        }                                                                    \
    }

#define ProfilerReport()                                                     \
    {                                                                        \
        timer_freq ProfilerFreq;                                             \
        if(!PlatformQueryTimerFrequency(&ProfilerFreq)){                     \
            ProfilerFreq.Value = 1;                                          \
            Error("QueryTimerFrequency failed");                             \
        }                                                                    \
                                                                             \
        u32 EntryCount = ProfilerTable.EntryCount;                           \
        u32 EntryIndex;                                                      \
        for(EntryIndex=0; EntryIndex<EntryCount; EntryIndex++){              \
            profiler_entry *Entry = ProfilerTable.Entries + EntryIndex;      \
            if(Entry->ID){                                                   \
                f32 T = (f32)Entry->TimeRecordedTotal/                       \
                        (f32)ProfilerFreq.Value;                             \
            printf("%s: %f\n", Entry->ID, T);                                \
            }                                                                \
        }                                                                    \
                                                                             \
        timer ProfilerTimer;                                                 \
        if(PlatformQueryTimer(&ProfilerTimer)){                              \
            u64 TotalElapsed = ProfilerTimer.Counter -                       \
                               ProfilerTable.GlobalRecord.TimeStart;         \
            f32 T = (f32)TotalElapsed/(f32)ProfilerFreq.Value;               \
            printf("\nTotal: %f\n", T);                                      \
        }                                                                    \
    }

#define ProfilerFinalize()                                                   \
    {                                                                        \
        if(PlatformMemoryFree((void **)&ProfilerTable.Entries)){             \
        }                                                                    \
    }

#else

#define ProfilerInitialize()
#define ProfilerRecordStart(Tag)
#define ProfilerRecordEnd(Tag)
#define ProfilerReport()
#define ProfilerFinalize()

#endif
