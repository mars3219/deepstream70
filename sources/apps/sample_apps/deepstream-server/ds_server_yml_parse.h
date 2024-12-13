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
#ifndef _DS_SERVER_YAML_PARSER_H_
#define _DS_SERVER_YAML_PARSER_H_

#include <gst/gst.h>

G_BEGIN_DECLS
guint ds_parse_udp_sink(GstElement * element, gchar * cfg_file_path,
    const char *group, guint *rtsp_port, guint *udp_port);
G_END_DECLS

#endif
