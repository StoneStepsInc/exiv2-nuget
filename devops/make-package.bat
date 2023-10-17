@echo off

setlocal

if "%~1" == "" (
  echo Package revision must be provided as the first argument
  goto :EOF
)

set PKG_VER=0.28.0
set PKG_REV=%~1

set EXIV2_FNAME=exiv2-%PKG_VER%-Source.tar.gz
set EXIV2_DNAME=exiv2-%PKG_VER%-Source
set EXIV2_SHA256=89af3b5ef7277753ef7a7b5374ae017c6b9e304db3b688f1948e73e103491f3d

set PATCH=c:\Program Files\Git\usr\bin\patch.exe
set SEVENZIP_EXE=c:\Program Files\7-Zip\7z.exe
set VCVARSALL=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall

if NOT EXIST %EXIV2_FNAME% (
  curl --location --output %EXIV2_FNAME% https://github.com/Exiv2/exiv2/releases/download/v%PKG_VER%/%EXIV2_FNAME%
)

"%SEVENZIP_EXE%" h -scrcSHA256 %EXIV2_FNAME% | findstr /C:"SHA256 for data" | call devops\check-sha256 "%EXIV2_SHA256%"

if ERRORLEVEL 1 (
  echo SHA-256 signature for %EXIV2_FNAME% does not match
  goto :EOF
)

tar -xzf %EXIV2_FNAME%

cd %EXIV2_DNAME%

rem
rem Patch the source to work around build problems described for
rem each patch in README.md.
rem

"%PATCH%" -p1 --unified --input ..\patches\02-cmake-lists.patch
"%PATCH%" -p1 --unified --input ..\patches\03-cmake-find-zlib-expat.patch

call "%VCVARSALL%" x64

rem
rem Generate VS projects to allow us to set up Nuget dependencies
rem below.
rem
rem     -DCMAKE_BUILD_TYPE=Debug ^
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DEXIV2_ENABLE_WEBREADY=OFF ^
    -DEXIV2_ENABLE_CURL=OFF ^
    -DEXIV2_ENABLE_BROTLI=OFF ^
    -DEXIV2_ENABLE_INIH=OFF ^
    -DEXIV2_ENABLE_VIDEO=ON ^
    -DEXIV2_ENABLE_XMP=ON ^
    -DEXIV2_ENABLE_SSH=OFF ^
    -DEXIV2_BUILD_SAMPLES=OFF ^
    -DEXIV2_BUILD_EXIV2_COMMAND=OFF ^
    -DEXIV2_BUILD_UNIT_TESTS=OFF ^
    -DEXIV2_BUILD_FUZZ_TESTS=OFF ^
    -DEXIV2_ENABLE_PNG=ON ^
    -DEXIV2_ENABLE_WIN_UNICODE=ON

rem
rem See README.md for more information about these patches.
rem
"%PATCH%" -p1 --unified --input ..\patches\20-vs-nuget-exiv2lib.patch
"%PATCH%" -p1 --unified --input ..\patches\21-vs-nuget-exiv2lib_int.patch
"%PATCH%" -p1 --unified --input ..\patches\22-vs-nuget-exiv2-xmp.patch

rem
rem This command downloads and sets up all packages, but won't
rem modify .vcxproj files. Nuget refused to consider this as a
rem deficiency, so it isn't likely that it will be fixed.
rem
nuget install StoneSteps.zLib.VS2022.Static -SolutionDirectory build
nuget install StoneSteps.Expat.VS2022.Static -SolutionDirectory build

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

rem
rem exiv2lib_int.lib and exiv2-xmp.lib are built in such a way that
rem all of their object files are included into exiv2.lib, so there
rem is no need to collect either for packaging.
rem
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
