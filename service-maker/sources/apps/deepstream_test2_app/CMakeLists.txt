####################################################################################################
# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
# property and proprietary rights in and to this material, related
# documentation and any modifications thereto. Any use, reproduction,
# disclosure or distribution of this material and related documentation
# without an express license agreement from NVIDIA CORPORATION or
# its affiliates is strictly prohibited.
####################################################################################################

cmake_minimum_required(VERSION 3.16)

project(Sample)
find_package(nvds_service_maker REQUIRED PATHS /opt/nvidia/deepstream/deepstream/service-maker/cmake)

add_executable(deepstream-test2-app deepstream_test2.cpp)
target_link_libraries(deepstream-test2-app PRIVATE nvds_service_maker)
