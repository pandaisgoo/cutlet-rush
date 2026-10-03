#!/bin/bash
set -e

# Remove existing Allegro installation if it exists
if [ -d "/usr/local/include/allegro5" ] || [ -d "/usr/local/lib/pkgconfig" ] || [ -f "/usr/local/lib/liballegro*" ]; then
  echo "Removing existing Allegro installation from /usr/local..."
  sudo rm -rf /usr/local/include/allegro5
  sudo rm -f /usr/local/lib/liballegro*
  sudo rm -f /usr/local/lib/pkgconfig/allegro*
fi

# Install Homebrew if not present
if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew not found. Installing..."
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# Install dependencies
brew install cmake pkg-config git
brew install freetype libpng jpeg freeimage
brew install dumb flac libogg libvorbis opusfile theora webp
brew install physfs sdl2 gtk+3

# Download minimp3 headers
wget https://raw.githubusercontent.com/lieff/minimp3/refs/heads/master/minimp3.h
wget https://raw.githubusercontent.com/lieff/minimp3/refs/heads/master/minimp3_ex.h
sudo mv minimp3.h /usr/local/include
sudo mv minimp3_ex.h /usr/local/include

# Download and build Allegro
wget https://github.com/liballeg/allegro5/releases/download/5.2.7.0/allegro-5.2.7.0.tar.gz
tar -xzf allegro-5.2.7.0.tar.gz
cd allegro-5.2.7.0
mkdir build
cd build
cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DWANT_PKG_CONFIG=ON -DWANT_SHARED=ON ..
make -j8
sudo make install
cd ../../
rm -rf allegro-5.2.7.0
rm -f allegro-5.2.7.0.tar.gz
rm -f minimp3.h minimp3_ex.h

echo "Allegro installation complete."
