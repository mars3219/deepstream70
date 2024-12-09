# SPDX-FileCopyrightText: Copyright (c) 2022-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
# property and proprietary rights in and to this material, related
# documentation and any modifications thereto. Any use, reproduction,
# disclosure or distribution of this material and related documentation
# without an express license agreement from NVIDIA CORPORATION or
# its affiliates is strictly prohibited.

CUDA_VER?=
ifeq ($(CUDA_VER),)
  $(error "CUDA_VER is not set")
endif

TARGET_DEVICE = $(shell g++ -dumpmachine | cut -f1 -d -)

CXX:= g++
SRCS:= gstdsnvmultiurisrcbin.cpp \
        gstdsnvmultiurisrcbininit.cpp

INCS:= $(wildcard *.h)

LIB:=libnvdsgst_nvmultiurisrcbin.so
LIB_INSTALL_DIR?=/opt/nvidia/deepstream/deepstream/lib/
GST_INSTALL_DIR?=/opt/nvidia/deepstream/deepstream/lib/gst-plugins/

CFLAGS+= -fPIC -DDS_VERSION=\"7.0.0\" \
     -I../../includes \
     -I../../apps/apps-common/includes/ \
 	 -I/usr/local/cuda-$(CUDA_VER)/include \
	 -I/usr/include/jsoncpp

LIBS := -shared -Wl,-no-undefined \
 	-L/usr/local/cuda-$(CUDA_VER)/lib64/ -lcudart
 
LIBS+= -L$(LIB_INSTALL_DIR) -L/opt/nvidia/deepstream/deepstream/lib/ \
        -lnvdsgst_helper -lnvdsgst_customhelper -lnvds_rest_server -ldl\
        -Wl,-rpath,$(LIB_INSTALL_DIR)

ifeq ($(TARGET_DEVICE),aarch64)
   LIBS+= -L/usr/lib/aarch64-linux-gnu/tegra/ -lgstnvcustomhelper -ljsoncpp
else
   LIBS+= -L$(LIB_INSTALL_DIR) -lgstnvcustomhelper -ljsoncpp
endif

OBJS:= $(SRCS:.cpp=.o)

PKGS:= gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0 gstreamer-audio-1.0
CFLAGS+= $(shell pkg-config --cflags $(PKGS))
LIBS+= $(shell pkg-config --libs $(PKGS))

all: $(LIB)

%.o: %.cpp $(INCS) Makefile
	@echo $(CFLAGS)
	$(CXX) -c -o $@ $(CFLAGS) $<

$(LIB): $(OBJS) $(DEP) Makefile
	$(CXX) -o $@ $(OBJS) $(LIBS)

install: $(LIB)
	cp -rv $(LIB) $(GST_INSTALL_DIR)

clean:
	rm -rf $(OBJS) $(LIB)

