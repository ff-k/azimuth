@echo off

pushd bin
pushd x86
set AzimuthLibPath="..\..\lib\x86"

if "%Platform%" == "X64" (
    popd
    pushd x64
    set AzimuthLibPath="..\..\lib\x64"
)

REM /nologo      : Suppress startup banner
REM /fp          : Floating-point model
REM /Gm-         : Disable minimal rebuild
REM /GR-         : Disable C++ RTTI
REM /EHa-        : Disable C++ exception handling with SEH exceptions
REM /WX          : Treat warnings as errors
REM /Wx          : Warning level=x
REM /wdxxxx      : Disable warning xxxx
REM /FC          : Use full pathnames
REM /Z7          : Enable old-style debug info
REM /GS          : Enable security checks
REM /sdl         : Enable additional security features and warnings
set DisabledWarnings=/wd4189 /wd4201 /wd4505 /wd4530 /wd4577
set CommonCompilerFlags=/nologo /fp:except- /DOPENEXR_DLL /fp:fast /Gm- /GR- /EHa- /WX /W4 %DisabledWarnings% /FC /Z7 -I "..\..\include"
REM /incremental : Incremental linking [:no disables it]
set CommonLinkerFlags=user32.lib libxml2.lib Half.lib IlmImf-2_2.lib /subsystem:console /entry:mainCRTStartup /incremental:no /NODEFAULTLIB:libc.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:msvcrtd.lib /LIBPATH:%AzimuthLibPath% /OUT:"azimuth.exe"

REM /Od          : Disable optimizations 
REM /MTd         : Link with libcmtd.lib
set DebugCompilerFlag=%CommonCompilerFlags% /D_CRT_SECURE_NO_WARNINGS /GS /sdl /Od /MTd /wd4100 /wd4189
REM /opt:noref   : Do not remove functions that are never referenced
set DebugLinkerFlag=%CommonLinkerFlags% /opt:noref /NODEFAULTLIB:libcmt.lib

REM /O1          : Minimize space
REM /O2          : Maximize speed
REM /MT          : Link with libcmt.lib
REM /Zo          : Generate richer debugging information for optimized code
set ReleaseCompilerFlag=%CommonCompilerFlags% /Qpar /Gw- /GS- /GL /Gs0 /O2 /Ob2 /Oi /MT /Zo
REM /opt:ref     : Remove functions that are never referenced
set ReleaseLinkerFlag=%CommonLinkerFlags% /opt:ref /NODEFAULTLIB:libcmtd.lib 



if "%1%" == "debug" (
    REM Debug build
    printf "Debug build\n"
    cl %DebugCompilerFlag% ..\..\src\azimuth_main.cpp /link %DebugLinkerFlag%
) else (
    REM Release build
    cl %ReleaseCompilerFlag% ..\..\src\azimuth_main.cpp /link %ReleaseLinkerFlag%
)

popd
popd