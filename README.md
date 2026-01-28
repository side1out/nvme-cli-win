# nvme-cli

## Windows Fork

**Maintainer:** James Huey <side1out@yahoo.com>  
**Repository:** https://github.com/side1out/nvme-cli-win (planned)  
**Based on:** nvme-cli v2.15 (upstream: https://github.com/linux-nvme/nvme-cli)  
**License:** GPL-2.0-or-later

This is a Windows port of the official Linux nvme-cli tool, enabling NVMe device management on Windows systems through native Windows Storage APIs.

**Purpose:** This project provides a Windows-based version of nvme-cli for users who are comfortable and familiar with the nvme-cli command-line interface and output format. While it is well understood that Windows imposes significant limitations on NVMe device access compared to Linux (many admin commands are blocked for system stability), there is still sufficient benefit in providing a familiar tool for read-only operations, diagnostics, and firmware management on Windows platforms.

---

![Coverity Scan Build Status](https://scan.coverity.com/projects/24883/badge.svg)
![MesonBuild](https://github.com/linux-nvme/nvme-cli/actions/workflows/build.yml/badge.svg)
![GitHub](https://img.shields.io/github/license/linux-nvme/nvme-cli)
![GitHub](https://img.shields.io/github/license/linux-nvme/libnvme)
![PyBuild](https://github.com/linux-nvme/nvme-cli/actions/workflows/libnvme-release-python.yml/badge.svg)
[![PyPI](https://img.shields.io/pypi/v/libnvme)](https://pypi.org/project/libnvme/)
[![PyPI - Wheel](https://img.shields.io/pypi/wheel/libnvme)](https://pypi.org/project/libnvme/)
[![codecov](https://codecov.io/gh/linux-nvme/nvme-cli/branch/master/graph/badge.svg)](https://codecov.io/gh/linux-nvme/nvme-cli)
[![Read the libnvme Docs](https://img.shields.io/readthedocs/libnvme)](https://libnvme.readthedocs.io/en/latest/)

NVM-Express user space tooling for Linux.

## Windows Port Overview

### Architecture

The Windows port is based on **nvme-cli v2.15** and **libnvme v1.15**, with Windows-specific compatibility patches:

- **Build System**: Meson + Ninja with MSYS2/MinGW-w64 toolchain
- **Compiler**: GCC 15.2.0 (MSYS2)
- **Dependencies**: 
  - json-c (0.18) for JSON output support
  - Windows SDK for native IOCTL interfaces
  - shlwapi for path manipulation

**Directory Structure:**
- `windows/` - Windows-specific compatibility layer (types, compat functions)
- `subprojects/libnvme` - Directory junction to separate libnvme-win repository
- Windows compatibility implemented via `#ifdef WINDOWS_GCC` preprocessor guards

### Current Status (January 2026)

✅ **Working:**
- Successfully compiles on Windows with MSYS2/MinGW
- nvme.exe builds and runs (2.75 MB executable)
- libnvme and libnvme-mi DLLs compile
- Basic nvme-cli command structure operational
- JSON output support enabled

⚠️ **Limitations:**
- Based on v1.15 codebase (upstream is at v1.16.1)
- Some features disabled: nbft.c, fabrics.c, nvme-rpmb.c
- NBFT and fabrics support excluded from Windows build
- Full device compatibility testing pending

🔧 **Known Issues:**
- Need to merge upstream v1.16.1 improvements
- May require Windows-specific IOCTL implementation refinement
- Documentation for Windows-specific features incomplete

### Building on Windows

**Prerequisites:**
1. Install MSYS2 from https://www.msys2.org/
2. Install required packages:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-meson \
             mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-json-c
   ```

**Setup libnvme subproject:**
```bash
# Clone libnvme-win separately
git clone https://github.com/side1out/libnvme-win.git ../libnvme-win

# Create directory junction (Windows) or symlink (MSYS2)
cd nvme-cli-win
cmd /c "mklink /J subprojects\libnvme ..\libnvme-win"
```

**VS Code Project:**

This repository includes a complete VS Code workspace configuration with:
- Build tasks (Meson configure, build, clean)
- C/C++ IntelliSense configuration
- Debugging launch configurations
- Recommended extensions

Simply open the folder in VS Code to use the pre-configured development environment.

**Build:**
```bash
# Configure (debug build)
meson setup .vscbuild --buildtype=debug

# Or configure (release build)
meson setup .vscbuild --buildtype=release

# Compile
meson compile -C .vscbuild

# Binary location: .vscbuild/nvme.exe
```

**Testing:**
```bash
.vscbuild/nvme.exe --version
```

### Windows Usage - Syntax Differences

**Important:** On Windows, you specify NVMe devices using the physical drive number instead of Linux-style `/dev/nvme*` paths.

**Device Specification:**
- **Linux:** `/dev/nvme0`, `/dev/nvme0n1`, etc.
- **Windows:** `0`, `1`, `2`, etc. (PhysicalDrive number)

**Example Commands:**

```bash
# List all NVMe devices
nvme list

# Identify controller on PhysicalDrive0
nvme id-ctrl 0

# Get SMART/health information
nvme smart-log 0

# Identify namespace 1
nvme id-ns 0 -n 1

# Get firmware log
nvme fw-log 0

# Download firmware (uses Windows Storage API)
nvme fw-download 0 --fw=firmware.bin

# Activate firmware slot 1
nvme fw-activate 0 -s 1 -a 3
```

**Finding Your Drive Number:**
Use Device Manager → Disk Drives, or run `nvme list` to see all NVMe devices with their PhysicalDrive numbers.

### Windows NVMe Passthrough Behavior

**⚠️ Important:** The Windows port attempts NVMe passthrough operations at all times, even when the operation may be expected to fail due to Windows security policies or driver restrictions. This is intentional behavior to match the Linux implementation's approach.

**Common Error Messages:**

When Windows blocks NVMe passthrough commands (typically due to security policies or driver limitations), you will see error messages from the `print_last_error()` function. The most common error is **Error 1: Incorrect function**, which indicates the Windows NVMe driver does not support the requested passthrough operation.

For example, attempting to create a namespace (which Windows typically blocks):

```
C:\> nvme create-ns /dev/nvme0 --nsze=0x10000 --ncap=0x10000

NVMEPassthrough DeviceIoControl failed: (1) Incorrect function.
```

Other error codes you may encounter:

```
NVMEPassthrough DeviceIoControl failed: (5) Access is denied.
NVMEPassthrough DeviceIoControl failed: (50) The request is not supported.
NVMEPassthrough DeviceIoControl failed: (87) The parameter is incorrect.
```

**What This Means:**

These errors are expected in many Windows environments and indicate:
- Windows security policies are blocking direct NVMe command access
- The NVMe driver does not support the requested passthrough operation
- Administrative privileges may be required (though not always sufficient)
- Some operations are restricted by the Windows Storage Stack

**This is normal behavior** - the library will attempt the operation and report the error. Some commands may work while others are blocked, depending on your specific Windows configuration, NVMe controller, and security policies.

**Note:** Unlike Linux where most NVMe commands can be issued directly with appropriate permissions, Windows implements stricter controls on storage device access for system stability and security reasons.

### Windows Port Development Notes

The port maintains compatibility with upstream by:
- Using conditional compilation (`#ifdef WINDOWS_GCC`)
- Separate meson.build sections for Windows vs Linux
- Windows compatibility layer in `windows/` directory
- Preserving ability to merge upstream improvements

For Windows-specific development, modifications to libnvme should be made in the separate libnvme-win repository accessed via the junction.

---

## Build from source (Linux)

nvme-cli uses meson as its build system. There is more than one way to configure and
build the project in order to mitigate meson dependency on the build environment.

If you build on a relative modern system, either use meson directly or the
Makefile wrapper.

Older distros might ship a too old version of meson, in this case it's possible
to build the project using [samurai](https://github.com/michaelforney/samurai)
and [muon](https://github.com/annacrombie/muon). Both build tools have only a
minimal dependency on the build environment. Too easy this step there is a build
script which helps to setup a build environment.

### nvme-cli dependencies (3.x and later):

Starting with nvme-cli 3.x, the libnvme library is fully integrated into the nvme-cli source tree. There is no longer any dependency on an external libnvme repository or package. All required libnvme and libnvme-mi code is included and built as part of nvme-cli.

| Library | Dependency | Notes |
|---------|------------|-------|
| libnvme, libnvme-mi | integrated | No external dependency, included in nvme-cli |
| json-c | optional | Recommended; without it, all plugins are disabled and json-c output format is disabled |


### Build with meson

#### Configuring

#### Configuring

No special configuration is required for libnvme, as it is now part of the
nvme-cli source tree. Simply run:

	$ meson setup .build

With meson's --wrap-mode argument it's possible to control if additional
dependencies should be resolved. The options are:

	--wrap-mode {default,nofallback,nodownload,forcefallback,nopromote}

Note for nvme-cli the 'default' is set to nofallback.

#### Building

	$ meson compile -C .build

#### Installing

	# meson install -C .build
	
### Build with build.sh wrapper

The `scripts/build.sh` is used for the CI build but can also be used for
configuring and building the project.

Running `scripts/build.sh` without any argument builds the project in the
default configuration (meson, gcc and defaults)

It's possible to change the compiler to clang

`scripts/builds.sh -c clang`

or enabling all the fallbacks

`scripts/build.sh fallback`

### Minimal static build with muon

`scripts/build.sh -m muon` will download and build `samurai` and `muon` instead
using `meson` to build the project. This reduces the dependency on the build
environment to:
- gcc
- make
- git

Furthermore, this configuration will produce a static binary.

### Build with Makefile wrapper

There is a Makefile wrapper for meson for backwards compatibility

	$ make
	# make install

Note: In previous versions, libnvme needed to be installed by hand.
This is no longer required in nvme-cli 3.x and later.

RPM build support via Makefile that uses meson

	$ make rpm

Static binary(no dependency) build support via Makefile that uses meson   

	$ make static

If not sure how to use, find the top-level documentation with:

	$ man nvme

Or find a short summary with:

	$ nvme help
	
## Distro Support

Many popular distributions (Alpine, Arch, Debian, Fedora, FreeBSD, Gentoo,
Ubuntu, Nix(OS), openSUSE, ...) and the usual package name is nvme-cli.

#### OpenEmbedded/Yocto

An [nvme-cli recipe](https://layers.openembedded.org/layerindex/recipe/88631/)
is available as part of the `meta-openembeded` layer collection.

#### Buildroot

`nvme-cli` is available as [buildroot](https://buildroot.org) package. The
package is named `nvme`.

## Developers

You may wish to add a new command or possibly an entirely new plug-in
for some special extension outside the spec.

This project provides macros that help generate the code for you. If
you're interested in how that works, it is very similar to how trace
events are created by Linux kernel's 'ftrace' component.

### Add command to existing built-in

The first thing to do is define a new command entry in the command
list. This is declared in nvme-builtin.h. Simply append a new "ENTRY" into
the list. The ENTRY normally takes three arguments: the "name" of the 
subcommand (this is what the user will type at the command line to invoke
your command), a short help description of what your command does, and the
name of the function callback that you're going to write. Additionally,
You can declare an alias name of subcommand with fourth argument, if needed.

After the ENTRY is defined, you need to implement the callback. It takes
four arguments: argc, argv, the command structure associated with the
callback, and the plug-in structure that contains that command. The
prototype looks like this:

  ```c
  int f(int argc, char **argv, struct command *command, struct plugin *plugin);
  ```

The argc and argv are adjusted from the command line arguments to start
after the sub-command. So if the command line is "nvme foo --option=bar",
the argc is 1 and argv starts at "--option".

You can then define argument parsing for your sub-command's specific
options then do some command specific action in your callback.

### Add a new plugin

The nvme-cli provides macros to make define a new plug-in simpler. You
can certainly do all this by hand if you want, but it should be easier
to get going using the macros. To start, first create a header file
to define your plugin. This is where you will give your plugin a name,
description, and define all the sub-commands your plugin implements.

There is a very important order on how to define the plugin. The following
is a basic example on how to start this:

File: foo-plugin.h
```c
#undef CMD_INC_FILE
#define CMD_INC_FILE plugins/foo/foo-plugin

#if !defined(FOO) || defined(CMD_HEADER_MULTI_READ)
#define FOO

#include "cmd.h"

PLUGIN(NAME("foo", "Foo plugin"),
	COMMAND_LIST(
		ENTRY("bar", "foo bar", bar)
		ENTRY("baz", "foo baz", baz)
		ENTRY("qux", "foo quz", qux)
	)
);

#endif

#include "define_cmd.h"
```

In order to have the compiler generate the plugin through the xmacro
expansion, you need to include this header in your source file, with
pre-defining macro directive to create the commands.

To get started from the above example, we just need to define "CREATE_CMD"
and include the header:

File: foo-plugin.c
```c
#include "nvme.h"

#define CREATE_CMD
#include "foo-plugin.h"
```

After that, you just need to implement the functions you defined in each
ENTRY, then append the object file name to the meson.build "sources".


## Dependency

libnvme depends on the /sys/class/nvme-subsystem interface which was
introduced in the Linux kernel release v4.15. Hence nvme-cli 2.x is
only working on kernels >= v4.15. For older kernels nvme-cli 1.x is
recommended to be used.

## How to contribute

There are two ways to send code changes to the project. The first one
is by sending the changes to linux-nvme@lists.infradead.org. The
second one is by posting a pull request on github. In both cases
please follow the Linux contributions guidelines as documented in

https://docs.kernel.org/process/submitting-patches.html#

That means the changes should be a clean series (no merges should be
present in a github PR for example) and every commit should build.

See also https://opensource.com/article/19/7/create-pull-request-github

### How to cleanup your series before creating PR

This example here assumes, the changes are in a branch called
fix-something, which branched away from master in the past. In the
meantime the upstream project has changed, hence the fix-something
branch is not based on the current HEAD. Before posting the PR, the
branch should be rebased on the current HEAD and retest everything.

For example rebasing can be done by following steps

```shell
# Update master branch
#   upstream == https://github.com/linux-nvme/nvme-cli.git
$ git switch master
$ git fetch --all
$ git reset --hard upstream/master

# Make sure all dependencies are up to date and make a sanity build
$ meson subprojects update
$ ninja -C .build

# Go back to the fix-something branch
$ git switch fix-something

# Rebase it to the current HEAD
$ git rebase master
[fixup all merge conflicts]
[retest]

# Push your changes to github and trigger a PR
$ git push -u origin fix-something
```

## Persistent, volatile configuration

Persistent configurations can be stored in two different locations: either in
the file `/etc/nvme/discovery.conf` using the old style, or in the file
`/etc/nvme/config.json` using the new style.

On the other hand, volatile configurations, such as those obtained from
third-party tools like `nvme-stats` or `blktests'` can be stored in the
`/run/nvme` directory. When using the `nvme-cli` tool, all these configurations
are combined into a single configuration that is used as input.

The volatile configuration is particularly useful for coordinating access to the
global resources among various components. For example, when executing
`blktests` for the FC transport, the `nvme-cli` udev rules can be triggered. To
prevent interference with a test, `blktests` can create a JSON configuration
file in `/run/nvme` to inform `nvme-cli` that it should not perform any actions
triggered from the udev context. This behavior can be controlled using the
`--context` argument.

For example a `blktests` volatile configuration could look like:

```json
[
  {
    "hostnqn": "nqn.2014-08.org.nvmexpress:uuid:242d4a24-2484-4a80-8234-d0169409c5e8",
    "hostid": "242d4a24-2484-4a80-8234-d0169409c5e8",
    "subsystems": [
      {
	"application": "blktests",
        "nqn": "blktests-subsystem-1",
        "ports": [
          {
            "transport": "fc",
	    "traddr": "nn-0x10001100aa000001:pn-0x20001100aa000001",
	    "host_traddr": "nn-0x10001100aa000002:pn-0x20001100aa000002"
          }
        ]
      }
    ]
  }
]
```

Note when updating the volatile configuration during runtime, it should done in
a an atomic way. For example create a temporary file without the `.json` file
extension in `/run/nvme` and write the contents to this file. When finished use
`rename` to add the `'.json'` file name extension. This ensures nvme-cli only
sees the complete file.

## Testing

For testing purposes a x86_64 static build from the current HEAD and is
available here:

https://monom.org/linux-nvme/upload/nvme-cli-latest-x86_64

## Container-Based Debugging and CI Build Reproduction

The nvme-cli project provides prebuilt CI containers that allow you to locally
reproduce GitHub Actions builds for debugging and development. These containers
mirror the environments used in the official CI workflows.

CI Containers Repository:
https://github.com/linux-nvme/ci-containers

CI Build Workflow Reference:
https://github.com/linux-nvme/nvme-cli/blob/master/.github/workflows/libnvme-build.yml

### 1. Pull a CI Container

All CI containers are published as OCI/Docker images.

Example: Ubuntu latest CI image:

```bash
docker pull ghcr.io/linux-nvme/debian.python:latest
```

Or with Podman:

```bash
podman pull ghcr.io/linux-nvme/debian.python:latest
```

### 2. Start the Container and Log In

Start an interactive shell inside the container:

```bash
docker run --rm -it \
  --name nvme-cli-debug \
  ghcr.io/linux-nvme/debian.python:latest \
  bash
```

Or with Podman:

```bash
podman run --rm -it \
  --name nvme-cli-debug \
  ghcr.io/linux-nvme/debian.python:latest \
  bash
```

You are now logged into the same environment used by CI.

### 3. Clone the nvme-cli Repository

Inside the running container:

```bash
git clone https://github.com/linux-nvme/nvme-cli.git
cd nvme-cli
```

(Optional) Checkout a specific branch or pull request:

```bash
git checkout <branch-or-commit>
```

### 4. Run the CI Build Script

The GitHub Actions workflow uses `scripts/build.sh`. To reproduce the CI build locally:

```bash
./scripts/build.sh
```

Build artifacts remain inside the container unless a host volume is mounted.

### 5. Cross-Build Example

The CI supports cross compilation using a dedicated cross-build container.

#### 5.1 Pull the Cross-Build Container

```bash
docker pull ghcr.io/linux-nvme/ubuntu-cross-s390x:latest
```

Or with Podman:

```bash
podman pull ghcr.io/linux-nvme/ubuntu-cross-s390x:latest
```

#### 5.2 Start the Cross-Build Container

```bash
docker run --rm -it \
  --name nvme-cli-cross \
  ghcr.io/linux-nvme/ubuntu-cross-s390x:latest \
  bash
```

Or with Podman:

```bash
podman run --rm -it \
  --name nvme-cli-cross \
  ghcr.io/linux-nvme/ubuntu-cross-s390x:latest \
  bash
```

#### 5.3 Clone the Repository

```bash
git clone https://github.com/linux-nvme/nvme-cli.git
cd nvme-cli
```

#### 5.4 Run a Cross Build

Example: Cross-build for `s390x`:

```bash
./scripts/build.sh -b release -c gcc -t s390x cross
```

The exact supported targets depend on the toolchains installed in the container.
