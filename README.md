## Exiv2 Nuget Package

This project builds an Exiv2 Nuget package with static Exiv2
libraries and header files  for the `x64` platform and `Debug`/
`Release` configurations.

Visit Exiv2 website for additional information about the Exiv2
project and library documentation:

https://github.com/Exiv2/exiv2

## Package Configuration

The Exiv2 static library appropriate for the platform and
configuration selected in a Visual Studio solution is explicitly
referenced within this package and will appear within the solution
folder tree after the package is installed. The solution may need
to be reloaded to make the library file visible. This library may
be moved into any solution folder after the installation.

Note that the Exiv2 library path in this package is valid only
for build configurations named `Debug` and `Release` and will
not work for any other configuration names. Do not install this
package for projects with configurations other than `Debug` and
`Release`.

CMake is configured to build Exiv2 with the following feature
options:

  * `BUILD_SHARED_LIBS=OFF`
  * `EXIV2_ENABLE_WEBREADY=OFF`
  * `EXIV2_ENABLE_CURL=OFF`
  * `EXIV2_ENABLE_SSH=OFF`
  * `EXIV2_BUILD_EXIV2_COMMAND=OFF`
  * `EXIV2_ENABLE_XMP=ON`
  * `EXIV2_ENABLE_PNG=ON`
  * `EXIV2_ENABLE_WIN_UNICODE=ON`

You may need to copy PDB files from package dependencies to the
output directory in order to avoid linker warnings reporting for
missing zLib and Expat PDB files. See _Build Events > Pre-Link
Events_ in the sample project for an example.

## Exiv2 Changes

Exiv2 source that was used to create this package contains a few
changes applied in patches described in this section against the
Exiv2 release indicated in the package version.

### `01-auto-ptr.patch`

This patch replaces `std::auto_ptr` with `std::unique_ptr`, so
the source compiles in C\+\+17.

There is similar work done in the trunk of the Exiv2 project,
which will be available at some point and will require projects
consuming Exiv2 updated to use the new pointer type name.

In this patch, however, the type `Exiv2::AutoPtr` was kept
intact to minimize the size of the patch and allow projects
consuming this package to continue using `Exiv2::AutoPtr`.

Note that the underlying type behind `Exiv2::AutoPtr` is now
`std::unique_ptr` and projects expecting `std::auto_ptr` need
to be updated to implement move semantics against pointers
obtained from Exiv2. Do not use this package if your project
requires specifically `std::auto_ptr` to work properly.

### `02-cmake-lists.patch`

This patch adds a definition `SUPPRESS_WARNINGS` to disable
Exiv2 warnings reported to one of the standard streams while
reading EXIF.

### `03-cmake-find-zlib-expat.patch`

This patch disables CMake's `find_package` calls looking for
zLib and Expat on the build machine, so these dependencies can
be injected as Nuget packages during the build process.

Current version of Nuget does not provide support for updating
`.vcxproj` files via command line tools, which limits CMake in
being able to handle Nuget packages gracefully.

### Nuget dependencies patches

Nuget lacks the capability to install packages and update
`.vcxproj` files via command line tools, which makes it
impossible for CMake to integrate Nuget packages into Visual
Studio projects generated by CMake. Patches with the prefix
`2x-vs-nuget-` are used as a work-around to inject Nuget
dependencies into `.vcxproj` files.

These patches are extremely fragile and may not work when the
next version of Nuget or CMake is installed on build machines
or when Visual Studio changes the format of `.vcxproj`. If no
alternative for supplying Nuget dependencies is found when
patching stops working, this package will be updated to remove
dependencies from Exiv2, which will disable XMP and PNG support.

The rest of this section describes how to create these patches.

Add `goto :EOF` in `make-package.bat` after CMake generates
Visual Studio projects. Copy the generated build directory
as the base installation to run `diff` against later.

Run this command to start VS2022 with the generated `exiv2.sln`
file.

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

These commands modify `.vcxproj` files to add all instructions
from installed packages, as well as references to `packages.config`,
which need to be removed because they will cause patches to pick
up lines with absolute file paths CMake outputs when it generates
`.vcxproj` files, and these lines will fail patches when they are
applied on build machines with different build paths.

Open `exiv2lib.vcxproj` and `exiv2lib_int.vcxproj` and delete
the following group of lines in each.

    <ItemGroup>
        <None Include="packages.config" />
    </ItemGroup>

Delete this line in `exiv2-xmp.vcxproj`.

    <None Include="packages.config" />

Generate patches for all three `.vcxproj` files in the same
way using the base installation directory saved earlier.

    diff --unified build_base\src\exiv2lib.vcxproj build\src\exiv2lib.vcxproj

Depending on the version of `diff.exe`, `--strip-trailing-cr`
may be required to generate a patch similar to the current ones.

Edit the generated patch file to make paths at the top look
similar to those in the current patches and save the patch
with LF line endings. Technically, `.vcxproj` files are
expected to use CRLF line endings, but `patch` gets confused
when it finds mixed line endings and having LF only works
out most optimally.

Inspect patches to make sure they don't contain absolute paths
in reference lines around the changes. If they do, these
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
  * devops/make-package.bat (`PKG_VER`, `PKG_REV`, `EXIV2_FNAME`,
    `EXIV2_DNAME`, `EXIV2_SHA256`)
  * .github/workflows/build-nuget-package.yml (`name`, `PKG_VER`,
    `PKG_REV`, `EXIV2_FNAME`, `EXIV2_DNAME`, `EXIV2_SHA256`)

`EXIV2_SHA256` ia a SHA-256 checksum of the Exiv2 package file and
needs to be changed when a new version of Exiv2 is released.

In the GitHub workflow YAML, `PKG_REV` must be reset to `1` (one)
every time Exiv2 version is changed. The workflow file must be
renamed with the new version in the name. This is necessary because
GitHub maintains build numbers per workflow file name.

For local builds package revision is supplied on the command line
and should be specified as `1` (one) for a new version of Exiv2.

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
the package to keep linking against network and Windows shell
libraries visible.

The sample project copies PDB files from package dependencies
to the project output directory in order to avoid linker warnings
reporting missing zLib and Expat PDB files.

[nuget.org]: https://www.nuget.org/packages/StoneSteps.Exiv2.VS2022.Static/
