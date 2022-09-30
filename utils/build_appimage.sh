#!/bin/bash
set -e

if [ -z "$1" ]; then
  echo "Please provide the path to the build directory!"
  exit 1
fi

# Builds Endless Sky and packages it as AppImage
# Control the output filename with the OUTPUT environment variable
# You may have to set the ARCH environment variable to e.g. x86_64.

# We need an icon file with a name matching the executable
cp resources/icons/icon_512x512.png endless-sky.png

# Install
DESTDIR=AppDir cmake --install "$1" --prefix /usr

# Inside an AppImage, the executable is a link called "AppRun" at the root of AppDir/.
# Keeping the data files next to the executable is perfectly valid, so we just move them to AppDir/ to avoid errors.
mv AppDir/usr/share/games/endless-sky/* AppDir/

# Now build the actual AppImage
curl -sSL https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -o linuxdeploy && chmod +x linuxdeploy
./linuxdeploy --appdir AppDir -e build/linux/Release/endless-sky -d resources/endless-sky.desktop -i endless-sky.png --output appimage

# Clean up
rm -rf AppDir linuxdeploy endless-sky.png
