sudo apt-get install -y libgbm-dev
sudo apt-get install -y libdrm-dev
sudo apt-get install -y meson
meson build/
ninja -C build/
