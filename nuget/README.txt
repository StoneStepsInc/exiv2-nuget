This package contains a static Exiv2 library and header files
for the x64 platform and Debug/Release configurations built with
Visual C++ 2022.

The Exiv2 static library appropriate for the platform and
configuration selected in a Visual Studio solution is explicitly
referenced within this package and will appear within the solution
folder tree after the package is installed. The solution may need
to be reloaded to make the library file visible. This library may
be moved into any solution folder after the installation.

Note that the Exiv2 library path in this package is valid only
for build configurations named Debug and Release and will
not work for any other configuration names. Do not install this
package for projects with configurations other than Debug and
Release.

Exiv2 library calls functions in these Windows libraries, which
need to be added as linker input to the Visual Studio project,
in order to link against the library.

 * psapi.lib
 * ws2_32.lib
 * shell32.lib

See README.md in Exiv2-Nuget project for more details.

https://github.com/StoneStepsInc/exiv2-nuget
