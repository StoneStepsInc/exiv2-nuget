This package contains static Exiv2 libraries and header files
for the x64 platform built with Visual C++ 2022, against
Debug/Release MT/DLL MSVC CRT.

The Exiv2 static libraries appropriate for the platform and
configuration selected in a Visual Studio solution are explicitly
referenced within this package and will appear within the solution
folder tree after the package is installed. The solution may need
to be reloaded to make the library file visible. These libraries
may be moved into any solution folder after the installation.

Note that the Exiv2 library path in this package will be selected
as Debug or Release based on whether the active configuration
is designated as a development or as a release configuration in
the underlying .vcxproj file.

Specifically, the initial project configurations have a property
called UseDebugLibraries in the underlying .vcxproj file,
which reflects whether the configuration is intended for building
release or development artifacts. Additional configurations copied
from these initial ones inherit this property. Manually created
configurations should have this property defined in the .vcxproj
file.

Do not install this package if your projects use debug configurations
without UseDebugLibraries. Note that CMake-generated Visual Studio
projects will not emit this property.

The Exiv2 libraries call functions in these Windows libraries,
which need to be added as linker input to the Visual Studio
project using this package.

 * psapi.lib
 * ws2_32.lib
 * shell32.lib

See README.md in Exiv2-Nuget project for more details.

https://github.com/StoneStepsInc/exiv2-nuget
