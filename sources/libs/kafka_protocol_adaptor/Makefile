################################################################################
# SPDX-FileCopyrightText: Copyright (c) 2018-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
# property and proprietary rights in and to this material, related
# documentation and any modifications thereto. Any use, reproduction,
# disclosure or distribution of this material and related documentation
# without an express license agreement from NVIDIA CORPORATION or
# its affiliates is strictly prohibited.
################################################################################

# this Makefile is to be used to build the kafka_proto protocol adaptor .so
CXX:=g++

PKGS:= glib-2.0 

DS_LIB:=/opt/nvidia/deepstream/deepstream/lib/

PROTOBUF_BUILD_DIR:= .

SRCS:=  nvds_kafka_proto.cpp kafka_client.cpp json_helper.cpp nvds_utils.cpp $(PROTOBUF_BUILD_DIR)/schema.pb.cc
TARGET_LIB:= libnvds_kafka_proto.so 

CFLAGS:= -fPIC -Wall

CFLAGS+= $(shell pkg-config --cflags $(PKGS))

LIBS:= $(shell pkg-config --libs $(PKGS))
LDFLAGS:= -shared

DS_INC:= ../../includes
RDKAFKA_INC:=/usr/local/include/librdkafka

INC_PATHS:= -I $(DS_INC) -I $(RDKAFKA_INC) -I/opt/tritonclient/include
CFLAGS+= $(INC_PATHS)

LIBS+= -L/$(DS_LIB) -L/opt/tritonclient/lib -lrdkafka -ljansson -lnvds_logger -lpthread -lcrypto -lprotobuf
LDFLAGS+= -Wl,-rpath=$(DS_LIB)

all: $(TARGET_LIB)

$(TARGET_LIB) : $(SRCS)
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBS) $(LDFLAGS)

clean:
	rm -rf $(TARGET_LIB)
SCHEMA_PROTOS:=../nvmsgconv/deepstream_schema/schema.proto
$(PROTOBUF_BUILD_DIR)/%.pb.cc: $(SCHEMA_PROTOS) Makefile
	protoc -I../nvmsgconv/deepstream_schema --cpp_out=$(PROTOBUF_BUILD_DIR)/ $<
