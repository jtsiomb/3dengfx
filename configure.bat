@echo off

REM ****************
REM *** DEFAULTS ***
REM ****************
set gfxlib=NATIVE
set jpeg=yes
set png=yes
set ft=yes

set display_help=no
set cfg_file=src\3dengfx_config.h

REM *************************
REM *** PARSING ARGUMENTS ***
REM *************************
: arg_loop
shift
if "%0"=="" goto end_arg_loop
if "%0"=="--with-gfxlib" (
        if "%1"=="sdl" set gfxlib=SDL
        if "%1"=="glut" set gfxlib=GLUT
        if "%1"=="gtk" set gfxlib=GTK
        if "%1"=="native" set gfxlib=NATIVE
)
if "%0"=="--enable-jpeg" set jpeg=yes
if "%0"=="--disable-jpeg" set jpeg=no
if "%0"=="--enable-png" set png=yes
if "%0"=="--disable-png" set png=no
if "%0"=="--enable-ft" set ft=yes
if "%0"=="--disable-ft" set ft=no

if "%0"=="--help" set display_help=yes
goto arg_loop
: end_arg_loop

echo Configuring 3dengfx ...
echo gfx lib: %gfxlib%
echo png support: %png%
echo jpeg support: %jpeg%
echo ft support: %ft%
echo Creating config header: %cfg_file%

REM *********************
REM *** CONFIG HEADER ***
REM *********************
echo /* this header is created by the 3dengfx configuration script */ > %cfg_file%
echo #ifndef _3DENGFX_CONFIG_H_ >> %cfg_file%
echo #define _3DENGFX_CONFIG_H_ >> %cfg_file%
echo #define USING_3DENGFX >> %cfg_file%
echo #define VER_STR "0.5" >> %cfg_file%
echo #define PREFIX     "/usr/local" >> %cfg_file%
echo #define SDL                        1 >> %cfg_file%
echo #define GLUT                       2 >> %cfg_file%
echo #define GTK                        3 >> %cfg_file%
echo #define GTKMM                      4 >> %cfg_file%
echo #define NATIVE                     5 >> %cfg_file%
echo #define NATIVE_X11                 10 >> %cfg_file%
echo #define NATIVE_WIN32               11 >> %cfg_file%
echo #define GFX_LIBRARY %gfxlib% >> %cfg_file%
echo #define SINGLE_PRECISION_MATH >> %cfg_file%
if "%png%"=="no" echo #define IMGLIB_NO_PNG >> %cfg_file%
if "%jpeg%"=="no" echo #define IMGLIB_NO_JPEG >> %cfg_file%
if "%ft%"=="no" echo #define FWXT_NO_FREETYPE >> %cfg_file%
echo #if GFX_LIBRARY == NATIVE >> %cfg_file%
echo #if defined(__unix__) >> %cfg_file%
echo #define NATIVE_LIB         NATIVE_X11 >> %cfg_file%
echo #elif defined(WIN32) >> %cfg_file%
echo #define NATIVE_LIB         NATIVE_WIN32 >> %cfg_file%
echo #endif     /* unix/win32 */ >> %cfg_file%
echo #endif     /* GFX_LIBRARY == NATIVE */ >> %cfg_file%
echo #endif     /* _3DENGFX_CONFIG_H_ */ >> %cfg_file%


if "%display_help%"=="no" goto end
REM ********************
REM *** HELP MESSAGE ***
REM ********************
echo usage: configure [options]
echo options:
REM echo   --prefix=<path>: installation path (default: /usr/local)
echo   --with-gfxlib=[sdl,glut,native or gtk] (default: native)
echo   --enable-png: enable png support, requires libpng (default)
echo   --disable-png: disable png support
echo   --enable-jpeg: enable jpeg support, requres libjpeg (defaul)
echo   --disable-jpeg: disable jpeg support
echo   --enable-ft: enable freetype support (default)
echo   --disable-ft: disable freetype support
echo   --help: this help screen
echo all invalid options are silently ignored.

: end

echo Done.
@echo on
