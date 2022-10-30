## Exiv2 Nuget Package

This project builds a Exiv2 Nuget package with static Exiv2
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

Libraries in this package are built without Exiv2 dependencies,
which means that only basic EXIF information will be available.
For example, XMP data requires XML dependencies and will not
be available. Similarly, PNG files cannot be interpreted because
zLib is not available and localization is disabled for the same
reason.

The source for this package is patched to disable dependencies
and to compile in C\+\+17. The latter change replaces `std::auto_ptr`,
with `std::unique_ptr` throughout Exiv2 source. See the `patches`
directory for details about these changes.

## Building a Nuget Package

This project can build a Nuget package for Exiv2 either locally
or via a GitHub workflow. In each case, following steps are taken.

  * Exiv2 source archive is downloaded from Exiv2's website and
    its SHA-256 signature is verified.

  * The source is patched to build in Visual C++ 2022.

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
configuration, during the package build process, and is not present
in the package specification file.

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

Verify that the new Exiv2 archive follows the directory name
pattern used in the `EXIV2_DNAME` variable.

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

[nuget.org]: https://www.nuget.org/packages/StoneSteps.Exiv2.VS2022.Static/
