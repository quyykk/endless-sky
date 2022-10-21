# Setup

First you need a copy of the code (if you intend on working on the game use your fork URL here):

```powershell
> git clone https://github.com/endless-sky/endless-sky --recursive
```

If you clone the repo another way make sure to download submodules as well when doing so. If you didn't you can download the submodule by executing

```powershell
> git submodule update --init
```

It is highly recommended to update submodules every time you `git pull` automatically, by setting the following config:

```powershell
> git config submodule.recurse true
```

The game's root directory, where your `git clone`d files reside, will be your starting point for compiling the game.

Next you will need to install a couple of dependencies to build the game.

## Windows

On Windows, you have the choice between MinGW and ClangCL. If you want to use Visual Studio, skip these instructions and go directly to the [Visual Studio instructions](#building-with-visual-studio).

You can download the [MinGW Winlibs](https://winlibs.com/#download-release) build, which also includes various tools you'll need to build the game as well. It is possible to use other MinGW builds as well.

You'll need the MSVCRT runtime version, 64-bit. The latest version is currently gcc 12 ([direct download link](https://github.com/brechtsanders/winlibs_mingw/releases/download/12.1.0-14.0.4-10.0.0-msvcrt-r2/winlibs-x86_64-posix-seh-gcc-12.1.0-mingw-w64msvcrt-10.0.0-r2.zip)).

Extract the zip file in a folder whose path doesn't contain a space (C:\ for example) and add the bin\ folder inside to your PATH (Press the Windows key and type "edit environment variables", then click on PATH and add it in the list).

You will also need to install [CMake](https://cmake.org) (if you don't already have it).

## MacOS

Install [Homebrew](https://brew.sh). Once it is installed, use it to install the tools and libraries you will need:

```
$ brew install cmake ninja mad libpng jpeg-turbo sdl2 openal-soft
```

## Linux

If you use a reasonably up-to-date distro (Fedora, latest Ubuntu, Arch, openSUSE Tumbleweed), then you can use your favorite package manager to install the needed dependencies:

<details>
<summary>DEB-based distros</summary>

```
g++ cmake ninja-build libsdl2-dev libpng-dev libjpeg-dev libgl1-mesa-dev libglew-dev libopenal-dev libmad0-dev uuid-dev
```

</details>

<details>
<summary>RPM-based distros</summary>

```
gcc-c++ cmake ninja-build SDL2-devel libpng-devel libjpeg-turbo-devel mesa-libGL-devel glew-devel openal-soft-devel libmad-devel libuuid-devel
```

</details>

It is recommended to use a newish CMake, although CMake 3.16 is the lowest supported. You can get the latest version from the [offical website](https://cmake.org/download/). To follow the instructions written below, you will need at least CMake 3.21.

**Note**: If your distro does not provide up-to-date version of the needed libraries, you will need to tell CMake to build the libraries from source by passing `-DES_USE_SYSTEM_LIBRARIES=OFF` to the first cmake command under the command line build instructions.

<details>
<summary>Packages needed to build the libraries from source</summary>

<details>
<summary>DEB-based distros</summary>

```
g++ cmake ninja-build pkg-config libgl1-mesa-dev libxmu-dev libxi-dev libglu1-mesa-dev tar zip unzip curl
```

</details>
<details>
<summary>RPM-based distros</summary>

```
gcc-c++ cmake ninja-build mesa-libGL-devel autoconf libtool libXext-devel mesa-libGLU-devel
```

</details>

</details>

# Building from the command line

If you want to build the game from the command line: from the project root folder, type:

```bash
$ cmake --preset <preset>
$ cmake --build --preset <preset>-debug
$ ./build/<preset>/Debug/endless-sky
$ ./build/<preset>/Debug/endless-sky-tests
```

(You can also use the `<preset>-release` preset for a release build).

Replace `<preset>` with one of the following presets:

- Windows: `mingw` (builds with MinGW), `vs` (builds with the Visual Studio toolchain)
- MacOS: `macos` or `macos-arm` (builds with the default compiler), `xcode` or `xcode-arm` (builds using the XCode toolchain)
- Linux: `linux` (builds with the default compiler)

# Building with an IDE

## Building with Visual Studio Code

Install the [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) and [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extensions and open the project folder under File -> Open Folder.

You'll be asked to select a preset. Select the one you want (usually `mingw` on Windows, `linux` on Linux and `macos` on MacOS). If you get asked to configure the project, click on Yes. You can use the bar at the very bottom to select between different configurations (Debug/Release) and build and start the game.

## Building with Code::Blocks

If you want to use the Code::Blocks IDE, from the root of the project folder execute:

```powershell
> cmake -G"CodeBlocks - Ninja" --preset <preset>
```

With `<preset>` being one of the available presets (see above for a list). For Windows for example you'd want `mingw`. Now there will be a Code::Blocks project inside `build\mingw`.

## Building with Visual Studio

You will need to install the [Clang component for Visual Studio](https://docs.microsoft.com/en-us/cpp/build/clang-support-msbuild). Next, open a terminal inside the root of the project:

```powershell
> cmake --preset vs
```

This will create a Visual Studio 2022 solution. If you are using an older version of VS, you will need to pass it using `-G`. Now you will find a complete solution in the `build/` folder. Find the solution and open it and you're good to go!

**Note**: Using MSVC to compile Endless Sky is not supported. If you try, you will get a tons of warnings (and maybe even a couple of errors) during compilation.

## Building with XCode

If you want to use the XCode IDE, from the root of the project folder execute:

```bash
$ cmake --preset xcode
```

(or use `xcode-arm` if you are using an ARM-based Mac).

The XCode project is located in the `build/` directory.
