#!/bin/bash

# Remove existing GStreamer
echo "Removing: Existing GStreamer..."
apt remove -y --purge gstreamer1.0* libgstreamer*
apt autoremove -y

# Remove GStreamer-related directories
echo "Removing: GStreamer-related directories..."
rm -rf /usr/lib/x86_64-linux-gnu/gstreamer-1.0
rm -rf ~/.cache/gstreamer-1.0
rm -rf ~/.gstreamer-1.0

# Install GStreamer version 1.25
echo "Installing: GStreamer version 1.24..."
apt update
apt install -y build-essential git meson ninja-build pkg-config bison flex

# glib-2.0 version 2.76.6
git clone https://github.com/GNOME/glib.git
cd glib
git checkout 2.76.6  # Select the desired version
meson build --prefix=/usr
ninja -C build
cd build
ninja install
cd ../..
rm -rf glib

# Clone GStreamer source code and install
git clone https://gitlab.freedesktop.org/gstreamer/gstreamer.git
cd gstreamer
git checkout 1.24.0  # Select the desired version

# Build and install GStreamer
pip install --upgrade meson
mkdir -p builddir
meson setup builddir
meson compile -C builddir
meson install -C builddir

# Add GStreamer environment variables
echo "Configuring: Adding GStreamer environment variables..."
echo "export PKG_CONFIG_PATH=/usr/local/lib/x86_64-linux-gnu/pkgconfig:\$PKG_CONFIG_PATH" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=/usr/local/lib/x86_64-linux-gnu:/usr/local/bin:\$LD_LIBRARY_PATH" >> ~/.bashrc
source ~/.bashrc

# Verify GStreamer version
echo "Verifying: Checking GStreamer version..."
gst-launch-1.0 --version
gst-inspect-1.0 --version

# Clean up
cd ..
rm -rf gstreamer

# completion message
echo "GStreamer 1.24 installation completed!"

# Install Rust
echo "Installing: Rust..."
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
source $HOME/.cargo/env
echo "Rust installation completed!"

# Install Node.js
echo "Installing: Node.js..."
curl -fsSL https://deb.nodesource.com/setup_lts.x | bash -
apt-get install -y nodejs
source ~/.bashrc
echo "Node.js installation completed!"

# Completion message
echo "Installation completed: GStreamer 1.24, Rust, Node.js!"

# FFmpeg installation version 4.4
apt install -y \
    build-essential pkg-config yasm cmake git wget libtool autoconf \
    libx264-dev libx265-dev libvpx-dev libfdk-aac-dev libmp3lame-dev libopus-dev
wget https://ffmpeg.org/releases/ffmpeg-4.4.tar.bz2
tar -xjf ffmpeg-4.4.tar.bz2
cd ffmpeg-4.4
./configure --prefix=/usr/local --enable-gpl --enable-nonfree \
  --enable-libx264 --enable-libx265 --enable-libvpx --enable-libfdk-aac \
  --enable-libmp3lame --enable-libopus --enable-shared
make -j$(nproc)
make install
ldconfig
cd ..
rm -rf ffmpeg-4.4 ffmpeg-4.4.tar.bz2

# Remove static libraries for ffmpeg
rm -rf /usr/local/lib/x86_64-linux-gnu/libavcodec.a
rm -rf /usr/local/lib/x86_64-linux-gnu/libswscale.a
rm -rf /usr/local/lib/x86_64-linux-gnu/libavutil.a
ldconfig

# Install OpenCV
echo "Installing: OpenCV..."
pkg-config --modversion opencv
apt-get remove -y --purge  libopencv* python-opencv
apt-get autoremove -y

# Install dependencies
apt-get update
apt-get upgrade -y
apt-get install -y build-essential cmake unzip \
    libjpeg-dev libtiff5-dev libpng-dev libavcodec-dev libavformat-dev \
    libswscale-dev libxvidcore-dev libx264-dev libxine2-dev \
    libv4l-dev v4l-utils \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    libgtk-3-dev \
    mesa-utils libgl1-mesa-dri libgtkgl2.0-dev libgtkglext1-dev \
    libatlas-base-dev gfortran libeigen3-dev \
    python3-dev python3-numpy

# Download OpenCV source code
mkdir -p opencv && cd opencv
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.10.0.zip
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/4.10.0.zip
unzip opencv.zip
unzip opencv_contrib.zip
cd opencv-4.10.0
mkdir -p build && cd build

# Compile OpenCV with GStreamer support / CUDA ARCH 8.6 / CUDNN 8.9.7
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D INSTALL_PYTHON_EXAMPLES=ON -D BUILD_SHARED_LIBS=ON \
    -D INSTALL_C_EXAMPLES=ON -D BUILD_DOCS=OFF -D BUILD_PERF_TESTS=OFF -D BUILD_TESTS=OFF \
    -D BUILD_PACKAGE=OFF -D BUILD_EXAMPLES=OFF -D WITH_TBB=ON -D ENABLE_FAST_MATH=1 -D CUDA_FAST_MATH=1 \
    -D CUDA_TOOLKIT_ROOT_DIR=/usr/local/cuda-12.2 -D WITH_CUDA=ON -D WITH_CUBLAS=ON -D WITH_CUFFT=ON \
    -D WITH_NVCUVID=ON -D WITH_IPP=OFF -D WITH_V4L=OFF -D WITH_LIBV4L=ON -D WITH_1394=OFF -D WITH_GTK=ON \
    -D WITH_QT=OFF -D WITH_OPENGL=OFF -D WITH_EIGEN=ON -D WITH_FFMPEG=ON -D WITH_GSTREAMER=ON -D BUILD_JAVA=OFF \
    -D BUILD_opencv_python3=ON -D BUILD_opencv_python2=OFF -D BUILD_NEW_PYTHON_SUPPORT=ON \
    -D OPENCV_SKIP_PYTHON_LOADER=ON -D OPENCV_GENERATE_PKGCONFIG=ON -D OPENCV_ENABLE_NONFREE=ON \
    -D OPENCV_EXTRA_MODULES_PATH=/workspace/opencv/opencv_contrib-4.10.0/modules -D WITH_CUDNN=ON \
    -D OPENCV_DNN_CUDA=ON -D CUDA_ARCH_BIN=8.6 -D CUDA_ARCH_PTX=8.6 -D CUDNN_LIBRARY=/usr/lib/x86_64-linux-gnu/libcudnn.so.8.9.7 \
    -D CUDNN_INCLUDE_DIR=/usr/include -D PYTHON3_PACKAGES_PATH=/usr/local/lib/python3.10/dist-packages ..

time make -j$(nproc)
make install
ldconfig

# clean up
cd ../../..
rm -rf opencv
echo "Installation completed: OpenCV with GStreamer support!"

# Install webrtc plugin
echo "Installing: WebRTC plugin..."
echo "Extracting webrtc.tar..."
tar -xvf webrtc.tar
cd debug
echo "Copying .so files to /usr/local/lib/x86_64-linux-gnu/gstreamer-1.0/..."
find . -name "*.so" -exec cp {} /usr/local/lib/x86_64-linux-gnu/gstreamer-1.0/ \;

# Verify installation
if [ -f "/usr/local/lib/x86_64-linux-gnu/gstreamer-1.0/libgstrswebrtc.so" ]; then
    echo "libgstrswebrtc.so found, cleaning up..."
    
    # clean up
    cd ..
    rm -rf webrtc.tar
    rm -rf debug
    echo "Installation completed: WebRTC plugin!"
    exit 0
else
    echo "Error: libgstrswebrtc.so not found in /usr/local/lib/x86_64-linux-gnu/gstreamer-1.0/"
    exit 1
fi