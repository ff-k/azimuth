@echo off

pushd bin
pushd x86

if "%Platform%" == "X64" (
    popd
    pushd x64
)

set AzimuthParams=%*
set AzimuthScene=%1
call set AzimuthOptions=%%AzimuthParams:*%1=%%

call azimuth.exe -s ..\..\assets\scenes\%AzimuthScene%.xml %AzimuthOptions%

popd
popd