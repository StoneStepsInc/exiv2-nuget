## Exiv2 Nuget Package

This project builds an Exiv2 Nuget package with static Exiv2
libraries and header files  for the `x64` platform.

Visit Exiv2 website for additional information about the Exiv2
project and library documentation:

https://github.com/Exiv2/exiv2

## Package Configuration

The Exiv2 static libraries appropriate for the platform and
configuration selected in a Visual Studio solution are explicitly
referenced within this package and will appear within the solution
folder tree after the package is installed. The solution may need
to be reloaded to make the library file visible. These libraries
may be moved into any solution folder after the installation.

Note that the Exiv2 library path in this package will be selected
as `Debug` or `Release` based on whether the active configuration
is designated as a development or as a release configuration in
the underlying `.vcxproj` file.

Specifically, the initial project configurations have a property
called `UseDebugLibraries` in the underlying `.vcxproj` file,
which reflects whether the configuration is intended for building
release or development artifacts. Additional configurations copied
from these initial ones inherit this property. Manually created
configurations should have this property defined in the `.vcxproj`
file.

Do not install this package if your projects use debug configurations
without `UseDebugLibraries`. Note that CMake-generated Visual Studio
projects will not emit this property.

CMake is configured to build Exiv2 with the following feature
options:

  * `BUILD_SHARED_LIBS=OFF`
  * `EXIV2_ENABLE_WEBREADY=OFF`
  * `EXIV2_ENABLE_CURL=OFF`
  * `EXIV2_ENABLE_BROTLI=OFF`
  * `EXIV2_ENABLE_INIH=OFF`
  * `EXIV2_ENABLE_VIDEO=ON`
  * `EXIV2_ENABLE_XMP=ON`
  * `EXIV2_ENABLE_SSH=OFF`
  * `EXIV2_BUILD_SAMPLES=OFF`
  * `EXIV2_BUILD_EXIV2_COMMAND=OFF`
  * `EXIV2_BUILD_UNIT_TESTS=OFF`
  * `EXIV2_BUILD_FUZZ_TESTS=OFF`
  * `EXIV2_ENABLE_PNG=ON`

You may need to copy PDB files from package dependencies to the
output directory in order to avoid linker warnings reporting for
missing zLib and Expat PDB files. See _Build Events > Pre-Link
Events_ in the sample project for an example.

## Exiv2 Changes

Exiv2 source that was used to create this package contains a few
changes applied in patches described in this section against the
Exiv2 release indicated in the package version.

### `01-wide-char-paths.patch`

Exiv2 dropped support for wide-character paths in v0.28.0 and
did not provide any alternative ways to open file paths with
characters outside of the currently selected Windows character
set, such as Win-1252. Their intent appears to rely on Windows
replacing various locale code pages with the UTF-8 code page,
but support for this solution is limited at this point.

The Exiv2 source in this package is patched to restore limited
file-only support for opening images with names comprised of
valid Unicode characters (i.e. will not work with URLs).

Use methods that take `std::filesystem:path` to open images
via the wide-character file interfaces.

Note that `std::filesystem:path` requires C\+\+17 and will
not compile in previous versions of C\+\+.

### `02-cmake-lists.patch`

This patch adds a definition `SUPPRESS_WARNINGS` to disable
Exiv2 warnings reported to one of the standard streams while
reading EXIF.

All optimizations in debug builds have been disabled (Exiv2
enables `/Ox` and `/Zo).

Exiv2 forces statically linked MSVC CRT (`/MTd`) for static
library builds. This is changed to use dynamic MSVC CRT at
all times to make sure memory managers are never mixed up
in applications built against Exiv2 static libraries.

Exiv2 disables all forms of inlining by removing `/Ob2` and
`/Ob1` from CMake-generated projects. This behavior is
disabled because inlining provides good performance benefits
and does not cause problems when includes and libraries are
properly set up.

Release configurations have been changed to generate PDB files
to aid debugging of released builds (e.g. crash dumps).

### `03-cmake-find-zlib-expat.patch`

This patch disables CMake's `find_package` calls looking for
zLib and Expat on the build machine, so these dependencies can
be injected as Nuget packages during the build process.

Current version of Nuget does not provide support for updating
`.vcxproj` files via command line tools, which limits CMake in
being able to handle Nuget packages gracefully.

## Migrating from 0.27.6

There are many breaking changes in Exiv2 v0.28.0, compared to
v0.27.6. Some of the most notable ones are listed below.

  * `Exiv2::Image::AutoPtr` has been replaced with
    `Exiv2::Image::UniquePtr`.
  * The ubiquitous `toLong()` methods has been replaced with
    `toInt64()` for signed integers and with `toUint32()` for
    unsigned integers.
  * `Exiv2::AnyError` has been removed.
  * `Exiv2::IfdId` was changed to a scoped `enum` and will
    not compare against integer values.
  * Native support for wide-character paths has been removed.
    The patch `01-wide-char-paths.patch` described above
    restores limited support for wide-character paths, but
    requires C\+\+17.

### Nuget dependencies patches

Nuget lacks the capability to install packages and update
`.vcxproj` files via command line tools, which makes it
impossible for CMake to integrate Nuget packages into Visual
Studio projects generated by CMake. Patches with the pattern
`2x-vs-nuget-*.patch` are used as a work-around to inject
Nuget dependencies into `.vcxproj` files.

These patches are extremely fragile and may not work when the
next version of Nuget or CMake is installed on build machines
or when Visual Studio changes the format of `.vcxproj`. If no
alternative for supplying Nuget dependencies is found when
patching stops working, this package will be updated to remove
dependencies from Exiv2, which will disable XMP and PNG support.

The rest of this section describes how to create these patches,
with the assumption that source code patches have already been
applied.

Add `goto :EOF` in `make-package.bat` after CMake generates
Visual Studio projects. Run this batch file from the project
root to generate Visual Studio projects.

Copy the generated `exiv2-0.28.0-Source` directory as the base
installation to run `diff` against later (e.g. `exiv2-0.28.0-Source_base`).

Change to the Exiv2 source directory and run this command to
start VS2022 with the generated `exiv2.sln` file.

    devenv /Command View.PackageManagerConsole build\exiv2.sln

Run these commands to install dependencies into the first
project.

    Install-Package StoneSteps.zLib.VS2022.Static -ProjectName exiv2lib
    Install-Package StoneSteps.Expat.VS2022.Static -ProjectName exiv2lib

There is another bug in Nuget that prevents installing packages
into projects located in the same directory. In order to work
around that bug, rename `build\src\packages.config` to
`packages.exiv2lib.config`. Run this command to install the
remaining dependencies.

    Install-Package StoneSteps.zLib.VS2022.Static -ProjectName exiv2lib_int
    Install-Package StoneSteps.Expat.VS2022.Static -ProjectName exiv2-xmp

These commands modify `.vcxproj` files to add build instructions
from installed packages, as well as references to `packages.config`.

Generate patches for all three `.vcxproj` files in the same
way using the base installation directory saved earlier.

    devops\create-vs-nuget-patches ^
        exiv2-0.28.0-Source_base ^
        exiv2-0.28.0-Source

Note that `.vcxproj` files are expected to use CRLF line endings,
but `patch` gets confused when it finds mixed line endings and
having LF only works out most optimally. Visual Studio 2022 and
build tools that come with it will accept these files with LF
line endings.

Edit the generated patch file to make paths at the top to use
forward slashes for path segment separators, remove quotes and
add the top-level placeholder directories `a` and `b`, like this.

    ---- a/build/src/exiv2lib.vcxproj	2023-02-20 16:10:23.489175900 -0500
    -+++ b/build/src/exiv2lib.vcxproj	2023-02-20 16:13:29.173423900 -0500
    +--- "build_base\\src\\exiv2lib.vcxproj"	2023-02-25 13:04:41.665092600 -0500
    ++++ "build\\src\\exiv2lib.vcxproj"	2023-02-25 13:16:07.929154700 -0500

Inspect patches to make sure they don't contain absolute paths
in referenced lines around the changes. If they do, these
patches will not work on other build machines.

## Building a Nuget Package

This project can build a Nuget package for Exiv2 either locally
or via a GitHub workflow. In each case, following steps are taken.

  * Exiv2 source archive is downloaded from Exiv2's website and
    its SHA-256 signature is verified.

  * The source is patched to build in Visual C++ 2022.

  * CMake is used to generate Visual Studio project files.

  * Project files generated in the previous step are patched
    up to inject Nuget dependencies, as if they were configured
    in Visual Studio via Nuget functionality form.

  * VS2022 Community Edition is used to build Exiv2 libraries
    locally and Enterprise Edition to build libraries on GitHub.

  * Build artifacts for all platforms and configurations are
    collected in staging directories under `nuget/build/native`.

  * `nuget.exe` is used to package staged files with the first
    three version components used as a Exiv2 version and the last
    version component used as a package revision. See _Package
    Version_ section for more details.

  * The Nuget package built on GitHub is uploaded to [nuget.org][].
    The package built locally is saved in the root project
    directory.

## Package Version

### Package Revision

Nuget packages lack package revision and in order to repackage
the same upstream software version, such as Exiv2 v0.27.5, the
4th component of the Nuget version is used to track the Nuget
package revision.

Nuget package revision is injected outside of the Nuget package
configuration, during the package build process, and is not
present in the package specification file.

Specifically, `nuget.exe` is invoked with `-Version=0.27.5.123`
to build a package with the revision `123`.

### Version Locations

Exiv2 version is located in a few places in this repository and
needs to be changed in all of them for a new version of Exiv2.

  * nuget/StoneSteps.Exiv2.VS2022.Static.nuspec (`version`)
  * devops/make-package.bat (`PKG_VER`, `PKG_REV`, `EXIV2_SHA256`)
  * .github/workflows/build-nuget-package.yml (`name`, `PKG_VER`,
    `PKG_REV`, `EXIV2_FNAME`, `EXIV2_DNAME`, `EXIV2_SHA256`)

`EXIV2_SHA256` ia a SHA-256 checksum of the Exiv2 package file and
needs to be updated when a new version of Exiv2 is released.

In the GitHub workflow YAML, `PKG_REV` must be reset to `1` (one)
every time Exiv2 version is changed. The workflow file must be
renamed with the new version in the name. This is necessary because
GitHub maintains build numbers per workflow file name.

For local builds package revision is supplied on the command line
and should be specified as `1` (one) for a new version of Exiv2
and incremented with every package release for the same upstream
version.

### GitHub Build Number

Build number within the GitHub workflow YAML is maintained in an
unconventional way because of the lack of build maturity management
between GitHub and Nuget.

For example, using build management systems, such as Artifactory,
every build would generate a Nuget package with the same version
and package revision for the upcoming release and build numbers
would be tracked within the build management system. A build that
was successfully tested would be promoted to the production Nuget
repository without generating a new build.

Without a build management system, the GitHub workflow in this
repository uses the pre-release version as a surrogate build
number for builds that do not publish packages to nuget.org,
so these builds can be downloaded and tested before the final
build is made and published to nuget.org. This approach is not
recommended for robust production environments because even
though the final published package is built from the exact
same source, the build process may still potentially introduce 
some unknowns into the final package (e.g. build VM was updated).

## Building Package Locally

You can build a Nuget package locally with `make-package.bat`
located in `devops`. This script expects VS2022 Community Edition
installed in the default location. If you have other edition of
Visual Studio, edit the file to use the correct path to the
`vcvarsall.bat` file.

Run `make-package.bat` from the repository root directory with a
package revision as the first argument. There is no provision to
manage build numbers from the command line and other tools should
be used for this.

## Sample Application

A Visual Studio project is included in this repository under
`sample-exiv2` to test the Nuget package built by this project.
This application dumps all EXIF tags found in a supplied image
file.

In order to build `sample-exiv2.exe`, open Nuget Package manager
in the solution and install either the locally-built Nuget package
or the one from [nuget.org][].

Binaries linking against this package need to include these Win32
libraries as linker input, similar to how this sample project does
it. This is because of some functionality in Exiv2 that cannot be
conditionally compiled, such as the `Exiv2::http` function.

 * psapi.lib
 * ws2_32.lib
 * shell32.lib

These library references are not automatically included within
the package to keep having to link against network and Windows
shell libraries visible.

[nuget.org]: https://www.nuget.org/packages/StoneSteps.Exiv2.VS2022.Static/
