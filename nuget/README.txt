This package contains static Exiv2 libraries and header files
for the x64 platform and Debug/Release configurations built with
Visual C++ 2022, against Debug/Release MT/DLL MSVC CRT.

The Exiv2 static libraries appropriate for the platform and
configuration selected in a Visual Studio solution are explicitly
referenced within this package and will appear within the solution
folder tree after the package is installed. The solution may need
to be reloaded to make libraries visible. Library files may be
moved into any solution folder after the installation.

Note that the Exiv2 library path in this package is valid only
for build configurations named Debug and Release and will
not work for any other configuration names. Do not install this
package for projects with configurations other than Debug and
Release.

The Exiv2 libraries call functions in these Windows libraries,
which need to be added as linker input to the Visual Studio
project using this package.

 * psapi.lib
 * ws2_32.lib
 * shell32.lib

You may need to copy PDB files from package dependencies to
the project output directory in order to avoid linker warnings
reporting missing zLib and Expat PDB files.

Due to a bug in Visual Studio's toolsets 14.33/14.34, which
incorrectly optimize std::string construction, leaving internal
string representation without a null terminator, the release
version of this package is compiled with code optimization
disabled (/Od). A new package will be released.

See README.md in Exiv2-Nuget project for more details.

https://github.com/StoneStepsInc/exiv2-nuget
