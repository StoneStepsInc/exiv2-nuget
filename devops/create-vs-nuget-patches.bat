@echo off

setlocal

if "%~1" == "" goto :usage
if "%~2" == "" goto :usage

set DIFF=%PROGRAMFILES%\Git\usr\bin\diff

"%DIFF%" --unified ^
    %~1\build\src\exiv2lib.vcxproj ^
    %~2\build\src\exiv2lib.vcxproj > patches\20-vs-nuget-exiv2lib.patch

"%DIFF%" --unified ^
    %~1\build\src\exiv2lib_int.vcxproj ^
    %~2\build\src\exiv2lib_int.vcxproj > patches\21-vs-nuget-exiv2lib_int.patch

"%DIFF%" --unified ^
    %~1\build\xmpsdk\exiv2-xmp.vcxproj ^
    %~2\build\xmpsdk\exiv2-xmp.vcxproj > patches\22-vs-nuget-exiv2-xmp.patch

goto :EOF

:usage

echo make-vsnuget-patches base-dir changed-dir
