# Setup

First you need a copy of the code:

```powershell
> git clone endless-sky --recursive
```

If you clone the repo another way make sure to download submodules as well when doing so. If you didn't you can download the submodule by executing

```powershell
> git submodule update --init
```

The game's root directory, where your `git clone`d files reside, will be your starting point for compiling the game.

How you build Endless Sky will then depend on your IDE and your operating system.


## Linux

Use your favorite package manager to install the following (version numbers may vary depending on your distribution):

<details>
<summary>DEB-based distros</summary>

```
g++ cmake ninja-build libgl1-mesa-dev libxmu-dev libxi-dev libglu1-mesa-dev
```

</details>

<details>
<summary>RPM-based distros</summary>

```
gcc-c++ cmake ninja mesa-libGL-devel
```

</details>

If you want to use an IDE, scroll down and check the entry for your IDE. If you don't want an IDE, you can build ES as follows:

## Windows

Currently, only the MinGW compiler toolchain is supported on Windows. You can download the [MinGW Winlibs](https://winlibs.com/#download-release) build, which also includes various tools you'll need to build the game as well.

Download the latest GCC MSVCRT 64-bit archive and extract it somewhere (C:\ for example) and add the folder to your PATH (Press the Windows key and type "edit environment variables").

You will also need to install [CMake](https://cmake.org) (if it isn't already installed as part of the Visual Studio toolchain).

## MacOS

Download and install the XCode command line tools. If you want to use XCode as an IDE as well, you'll need to install the IDE itself as well.

Next, install [Homebrew](https://brew.sh). Once it is installed, use it to install the tools you will need:

```
  $ brew install cmake ninja
```

### Building on the command line

If you want to build the game from the command line: from the project root folder, type:

```bash
$ cmake --preset <preset>
$ cmake --build build/<preset> --config Debug
$ ./build/<preset>/Debug/endless-sky
$ ./build/<preset>/Debug/endless-sky-tests
```

(You can also use the `Release` config for a release build).

Replace `<preset>` with one of the following presets:

- Windows: `mingw` (builds with MinGW), `msvc` (builds with the MSVC compiler - no official support)
- MacOS: `macos` (builds with the default compiler), `xcode` (builds using the XCode toolchain)
- Linux: `linux` (builds with the default compiler)

### Building with Visual Studio Code, CLion

Open the root folder and follow the steps from the IDE. Now you can build and run the game immediately.

### Building with Code::Blocks

If you want to use the Code::Blocks IDE, from the root of the project folder execute:

```powershell
> cmake -G"CodeBlocks - Ninja" --preset mingw
```

Now there will be a Code::Blocks project inside `build\mingw`.


### Building with Visual Studio

For Visual Studio, it is recommended to use the CMake project. Make sure you have CMake installed from the Visual Studio Installer, as well as the C++ compiler tools. Then you can open the root folder and hit compile. This is available starting with VS 2022. It is possible to use MinGW to compile the project, but VS is not able to debug the game (you can use Visual Studio Code for debugging) unless you use MSVC to compile.

You can also create a Visual Studio project using the following (from the project root):

```powershell
> mkdir build
> cd build
> cmake -G"Visual Studio 17" ../
```

This will create a VS 2022 solution inside the `build\` directory. If you need a VS 2019 solution, use `16` as the version, and so on.


### Building with XCode

If you want to use the XCode IDE, from the root of the project folder execute:

```bash
$ cmake --preset xcode
```

You will find a XCode project inside `build/xcode`.
