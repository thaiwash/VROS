sudo apt-get install -y libgbm-dev
sudo apt-get install -y libdrm-dev
sudo apt-get install freeglut3-dev freeglut3
sudo apt-get install -y meson
sudo apt-get install -y ninja-build

sudo apt-get install -y cmake \
	build-essential     \
	libboost-program-options-dev     \
	libboost-filesystem-dev     \
	libboost-regex-dev     \
	libboost-system-dev     \
	libboost-test-dev     \
	libeigen3-dev     \
	libsuitesparse-dev     \
	libfreeimage-dev     \
	libgoogle-glog-dev     \
	libgflags-dev     \
	libglew-dev     \
	qtbase5-dev     \
	libqt5opengl5-dev	\

mkdir build
meson build/
ninja -C build/

# meson build/
# ninja -C build/
# install:
# 	meson build
# 
# clean:
# 	rm -rf build
