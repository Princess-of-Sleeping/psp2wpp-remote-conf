
if [ ! -d libusb ]; then
  git clone https://github.com/libusb/libusb.git libusb
  cd libusb
  ./bootstrap.sh
  ./configure --host=x86_64-w64-mingw32 --enable-static --disable-shared
  make
  cd ..
fi

LIBUSB1_BUILD_DIR="./libusb"

if [ ! -d libusb-compat ]; then
  git clone https://github.com/libusb/libusb-compat-0.1.git libusb-compat
  cd libusb-compat
  ./autogen.sh
  LIBUSB_1_0_CFLAGS="-I../../libusb/libusb" LIBUSB_1_0_LIBS="-L../../libusb/libusb/.libs -lusb-1.0" PKG_CONFIG_PATH="../../libusb" ./configure --host=x86_64-w64-mingw32 --enable-static --disable-shared
  make
  cd ..
fi

if [ ! -d build ]; then
  mkdir build
fi

cd build

cmake ../ -DCMAKE_TOOLCHAIN_FILE=toolchain-x86_64-w64-mingw32.cmake
make
cd ..
