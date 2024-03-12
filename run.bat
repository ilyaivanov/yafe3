@echo off
set arg1=%1

if exist build rmdir /s /q build
mkdir build
pushd build

set CommonCompilerOptions=/nologo /GR- /FC /GS- /Gs9999999

@REM DEVELOPMENT
set CompilerOptions=/Zi /Od

@REM PRODUCTION
@REM set CompilerOptions=/O2 

set LinkerOptions=/nodefaultlib /subsystem:windows /STACK:0x100000,0x100000 /incremental:no
set Libs=user32.lib gdi32.lib kernel32.lib dwmapi.lib

cl %CommonCompilerOptions% %CompilerOptions% ../main.c /link %LinkerOptions% %Libs% 

IF NOT "%arg1%" == "b" (
    call .\main.exe
)

popd