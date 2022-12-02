@echo off
set PATH=%PATH%;C:\software\llvm-project\bin
set COMMON_COMPILER_FLAGS=/W4 /wd4100 /Oi /nologo /Feprogram.exe

set DEBUG_FLAGS=/Zi /MTd
set RELEASE_FLAGS=/O3

set COMPILER_FLAGS=%COMMON_COMPILER_FLAGS% %DEBUG_FLAGS%
REM set COMPILER_FLAGS=%COMMON_COMPILER_FLAGS% %RELEASE_FLAGS%

set LINKER_FLAGS=/INCREMENTAL:NO /OPT:REF
set LIBS=user32.lib gdi32.lib

clang-cl %COMPILER_FLAGS% main.cpp /link %LINKER_FLAGS% %LIBS%