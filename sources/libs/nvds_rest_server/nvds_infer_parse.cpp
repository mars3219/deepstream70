/*
 * SPDX-FileCopyrightText: Copyright (c) 2022-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#include "nvds_rest_server.h"
#include "nvds_parse.h"

#define EMPTY_STRING ""

bool
nvds_rest_infer_parse (const Json::Value & in, NvDsServerInferInfo * infer_info)
{
  if (infer_info->uri.find ("/api/v1/") != std::string::npos) {
    for (Json::ValueConstIterator it = in.begin (); it != in.end (); ++it) {

      std::string root_val = it.key ().asString ().c_str ();
      infer_info->root_key = root_val;

      const Json::Value sub_root_val = in[root_val];      //object values of root_key

        infer_info->stream_id =
            sub_root_val.get ("stream_id", EMPTY_STRING).asString ().c_str ();

        if (infer_info->infer_flag == INFER_INTERVAL) {
          try {
            infer_info->interval =
                sub_root_val.get ("interval", 0).asUInt ();
            if (infer_info->interval > 32) {
              infer_info->infer_log = "INFER_INTERVAL_UPDATE_FAIL, interval value not parsed correctly, Unsigned Integer. Range: 0 - 32 ";
              infer_info->status = INFER_INTERVAL_UPDATE_FAIL;
              infer_info->err_info.code = StatusBadRequest;
              return false;
            }
          } catch (const std::exception& e) {
            // Error handling: other exceptions
            infer_info->infer_log = "INFER_INTERVAL_UPDATE_FAIL, error: " + std::string(e.what());
            infer_info->status = INFER_INTERVAL_UPDATE_FAIL;
            infer_info->err_info.code = StatusBadRequest;
            return false;
          }
        }
    }
  } else {
    g_print ("Unsupported REST API version\n");
  }

  return true;
}
