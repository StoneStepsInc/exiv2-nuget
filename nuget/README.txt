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

Due to a bug in VC++ toolsets 14.33/14.34 that yields malformed
std::string instances lacking null termination in release builds,
this package was built outside of GitHub using toolset 14.34.
Projects using this package must use a toolset with the same or
greater version.

See README.md in Exiv2-Nuget project for more details.

https://github.com/StoneStepsInc/exiv2-nuget
