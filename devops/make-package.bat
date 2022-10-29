@echo off

setlocal

if "%~1" == "" (
  echo Package revision must be provided as the first argument
  goto :EOF
)

set PKG_VER=0.27.5
set PKG_REV=%~1

set EXIV2_FNAME=exiv2-%PKG_VER%-Source.tar.gz
set EXIV2_DNAME=exiv2-%PKG_VER%-Source
set EXIV2_SHA256=35a58618ab236a901ca4928b0ad8b31007ebdc0386d904409d825024e45ea6e2

set PATCH=c:\Program Files\Git\usr\bin\patch.exe
set SEVENZIP_EXE=c:\Program Files\7-Zip\7z.exe
set VCVARSALL=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall

curl --location --output %EXIV2_FNAME% https://github.com/Exiv2/exiv2/releases/download/v%PKG_VER%/%EXIV2_FNAME%

"%SEVENZIP_EXE%" h -scrcSHA256 %EXIV2_FNAME% | findstr /C:"SHA256 for data" | call devops\check-sha256 "%EXIV2_SHA256%"

if ERRORLEVEL 1 (
  echo SHA-256 signature for %EXIV2_FNAME% does not match
  goto :EOF
)

tar xzf %EXIV2_FNAME%

cd %EXIV2_DNAME%

rem
rem Patch the source to work around build problems
rem
"%PATCH%" -p1 --unified --input ..\patches\cmake.patch
"%PATCH%" -p1 --unified --input ..\patches\auto-ptr.patch

call "%VCVARSALL%" x64

rem
rem Generate VS projects in case we need to set up Nuget dependencies later
rem
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

rem
rem Build x64 Debug/Release
rem 
cmake --build build --config Debug
cmake --build build --config Release

rem
rem Collect artifacts
rem
mkdir ..\nuget\licenses
copy COPYING ..\nuget\licenses\

mkdir ..\nuget\build\native\lib\x64\Debug
xcopy /Y build\lib\Debug\* ..\nuget\build\native\lib\x64\Debug\

mkdir ..\nuget\build\native\lib\x64\Release
xcopy /Y build\lib\Release\* ..\nuget\build\native\lib\x64\Release\

mkdir ..\nuget\build\native\include\exiv2
xcopy /Y /S include\exiv2\* ..\nuget\build\native\include\exiv2\
copy /Y build\exv_conf.h ..\nuget\build\native\include\exiv2\
copy /Y build\exiv2lib_export.h ..\nuget\build\native\include\exiv2\

cd ..

rem
rem Create a package
rem
nuget pack nuget\StoneSteps.Exiv2.VS2022.Static.nuspec -Version %PKG_VER%.%PKG_REV%
