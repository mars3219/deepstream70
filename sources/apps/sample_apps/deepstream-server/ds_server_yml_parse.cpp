/*
 * SPDX-FileCopyrightText: Copyright (c) 2018-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#include "ds_server_yml_parse.h"
#include <yaml-cpp/yaml.h>
#include <string>
#include <iostream>

guint ds_parse_udp_sink(GstElement * element, gchar * cfg_file_path,
    const char *group, guint *rtsp_port, guint *udp_port) {
  if (!element || !cfg_file_path || !group) {
        std::cerr << "Invalid arguments provided to ds_parse_udp_sink." << std::endl;
        return 1;
    }
  
  try {
        YAML::Node config = YAML::LoadFile(cfg_file_path);
        if (!config[group]) {
            std::cerr << "Group '" << group << "' not found in configuration file." << std::endl;
            return 1;
        }

        int sync = config[group]["sync"].as<int>();
        int qos = config[group]["qos"].as<int>();
        *rtsp_port = config[group]["rtsp-port"].as<int>();
        *udp_port = config[group]["udp-port"].as<int>();

        g_object_set(G_OBJECT (element), "sync", sync, NULL);
        g_object_set(G_OBJECT (element), "qos", qos, NULL);

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error parsing sink configuration: " << e.what() << std::endl;
        return 1;
    }
}
