################################################################################ 
# SPDX-FileCopyrightText: Copyright (c) 2022-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
# property and proprietary rights in and to this material, related
# documentation and any modifications thereto. Any use, reproduction,
# disclosure or distribution of this material and related documentation
# without an express license agreement from NVIDIA CORPORATION or
# its affiliates is strictly prohibited.
################################################################################ 

CUDA_VER?=
ifeq ($(CUDA_VER),)
  $(error "CUDA_VER is not set")
endif

CXX:= g++
SRCS:= gst-nvdscustommessage.c gst-nvdscustomevent.c \
        gst-nvmultiurisrcbincreator.cpp

INCS:= $(wildcard *.h) \

LIB:=libnvdsgst_customhelper.so
LIB_INSTALL_DIR?=/opt/nvidia/deepstream/deepstream-$(NVDS_VERSION)/lib/
NVDS_VERSION:=7.0

CFLAGS+= -fPIC -DDS_VERSION=\"7.0.0\" \
 	 -I../../includes \
	 -I../../apps/apps-common/includes/ \
 	 -I/usr/local/cuda-$(CUDA_VER)/include \
	 -I/usr/include/jsoncpp -g

LIBS := -shared -Wl,-no-undefined \
 	-L/usr/local/cuda-$(CUDA_VER)/lib64/ -lcudart \
 	-pthread

LIBS+= -L$(LIB_INSTALL_DIR) -L/opt/nvidia/deepstream/deepstream/lib/ \
       -lnvdsgst_helper -lnvds_rest_server -ldl\
        -Wl,-rpath,$(LIB_INSTALL_DIR) \

ifeq ($(TARGET_DEVICE),aarch64)
   LIBS+= -L/usr/lib/aarch64-linux-gnu/tegra/ -ljsoncpp
else
   LIBS+= -L$(LIB_INSTALL_DIR) -ljsoncpp
endif

OBJS:= $(SRCS:.c=.o)
OBJS:= $(OBJS:.cpp=.o)

PKGS:= gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0 
CFLAGS+= $(shell pkg-config --cflags $(PKGS))
CFLAGS+= -DPLATFORM_TEGRA
LIBS+= $(shell pkg-config --libs $(PKGS))
LIBS+= -lnvdsgst_meta -lnvds_meta


all: $(LIB)

%.o: %.c $(INCS) Makefile
	$(CXX) -c -o $@ $(CFLAGS) $<

%.o: %.cpp $(INCS) Makefile
	$(CXX) -c -o $@ $(CFLAGS) $<

$(LIB): $(OBJS) $(DEP) Makefile
	$(CXX) -o $@ $(OBJS) $(LIBS)

install: $(LIB)
	cp -rv $(LIB) $(LIB_INSTALL_DIR)

clean:
	rm -rf $(OBJS) $(LIB)
