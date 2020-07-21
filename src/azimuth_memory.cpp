static b32
MemoryAllocate(void **Memory, size_t Size){
    return PlatformMemoryAllocate(Memory, Size);
}

static b32
MemoryCopy(void *Dest, size_t DestSize, void *Src, size_t SrcSize){
    return PlatformMemoryCopy(Dest, DestSize, Src, SrcSize);
}

static b32
MemoryFree(void **Memory){
    return PlatformMemoryFree(Memory);
}
