/*
 * SPDX-FileCopyrightText: Copyright (c) 2018-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#include <gst/gst.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


#include "deepstream_app.h"

#define MAX_DISPLAY_LEN 64
static guint demux_batch_num = 0;

GST_DEBUG_CATEGORY_EXTERN (NVDS_APP);

GQuark _dsmeta_quark;

#define CEIL(a,b) ((a + b - 1) / b)

/**
 * @brief  Add the (nvmsgconv->nvmsgbroker) sink-bin to the
 *         overall DS pipeline (if any configured) and link the same to
 *         common_elements.tee (This tee connects
 *         the common analytics path to Tiler/display-sink and
 *         to configured broker sink if any)
 *         NOTE: This API shall return TRUE if there are no
 *         broker sinks to add to pipeline
 *
 * @param  appCtx [IN]
 * @return TRUE if succussful; FALSE otherwise
 */
static gboolean add_and_link_broker_sink (AppCtx * appCtx);

/**
 * @brief  Checks if there are any [sink] groups
 *         configured for source_id=provided source_id
 *         NOTE: source_id key and this API is valid only when we
 *         disable [tiler] and thus use demuxer for individual
 *         stream out
 * @param  config [IN] The DS Pipeline configuration struct
 * @param  source_id [IN] Source ID for which a specific [sink]
 *         group is searched for
 */
static gboolean is_sink_available_for_source_id (NvDsConfig * config,
    guint source_id);

static NvDsSensorInfo* s_sensor_info_create(NvDsSensorInfo* sensor_info);
static void s_sensor_info_destroy(NvDsSensorInfo* sensor_info);

static NvDsSensorInfo* s_sensor_info_create(NvDsSensorInfo* sensor_info) {
  NvDsSensorInfo* sensorInfoToHash = (NvDsSensorInfo*)g_malloc0(sizeof(NvDsSensorInfo));
  *sensorInfoToHash = *sensor_info;
  sensorInfoToHash->sensor_id = (gchar const*)g_strdup(sensor_info->sensor_id);
  sensorInfoToHash->sensor_name = (gchar const*)g_strdup(sensor_info->sensor_name);
  sensorInfoToHash->uri = (gchar const*)g_strdup(sensor_info->uri);
  return sensorInfoToHash;
}

static void s_sensor_info_destroy(NvDsSensorInfo* sensor_info) {
  if(!sensor_info)
    return;
  if(sensor_info->sensor_id) {
    g_free((void*)sensor_info->sensor_id);
  }
  if(sensor_info->sensor_name) {
    g_free((void*)sensor_info->sensor_name);
  }

  g_free(sensor_info);
}

static void s_sensor_info_callback_stream_added (AppCtx *appCtx, NvDsSensorInfo* sensorInfo) {

  NvDsSensorInfo* sensorInfoToHash = s_sensor_info_create(sensorInfo);
  /** save the sensor info into the hash map */
  g_hash_table_insert (appCtx->sensorInfoHash, sensorInfo->source_id + (char *)NULL, sensorInfoToHash);
}

static void s_sensor_info_callback_stream_removed (AppCtx *appCtx, NvDsSensorInfo* sensorInfo) {

  NvDsSensorInfo* sensorInfoFromHash = get_sensor_info(appCtx, sensorInfo->source_id);
  /** remove the sensor info from the hash map */
  if(sensorInfoFromHash) {
    g_hash_table_remove(appCtx->sensorInfoHash, sensorInfo->source_id + (gchar*)NULL);
    s_sensor_info_destroy(sensorInfoFromHash);
  }
}

NvDsSensorInfo* get_sensor_info(AppCtx* appCtx, guint source_id) {
  NvDsSensorInfo* sensorInfo = (NvDsSensorInfo*)g_hash_table_lookup(appCtx->sensorInfoHash,
        source_id + (gchar*)NULL);
  return sensorInfo;
}

/*Note: Below callbacks/functions defined for FPS logging,
 *  when nvmultiurisrcbin is being used*/
static NvDsFPSSensorInfo* s_fps_sensor_info_create(NvDsFPSSensorInfo* sensor_info);
NvDsFPSSensorInfo* get_fps_sensor_info(AppCtx* appCtx, guint source_id);
static void s_fps_sensor_info_destroy(NvDsFPSSensorInfo* sensor_info);

static NvDsFPSSensorInfo* s_fps_sensor_info_create(NvDsFPSSensorInfo* sensor_info) {
  NvDsFPSSensorInfo* fpssensorInfoToHash = (NvDsFPSSensorInfo*)g_malloc0(sizeof(NvDsFPSSensorInfo));
  *fpssensorInfoToHash = *sensor_info;
  fpssensorInfoToHash->uri = (gchar const*)g_strdup(sensor_info->uri);
  fpssensorInfoToHash->source_id = sensor_info->source_id;
  fpssensorInfoToHash->sensor_id = (gchar const*)g_strdup(sensor_info->sensor_id);
  fpssensorInfoToHash->sensor_name = (gchar const*)g_strdup(sensor_info->sensor_name);
  return fpssensorInfoToHash;
}

static void s_fps_sensor_info_destroy(NvDsFPSSensorInfo* sensor_info) {
  if(!sensor_info)
    return;
  if(sensor_info->sensor_id) {
    g_free((void*)sensor_info->sensor_id);
  }
  if(sensor_info->sensor_name) {
    g_free((void*)sensor_info->sensor_name);
  }
  if(sensor_info->uri) {
    g_free((void*)sensor_info->uri);
  }

  g_free(sensor_info);
}

static void s_fps_sensor_info_callback_stream_added (AppCtx *appCtx, NvDsFPSSensorInfo* sensorInfo) {

  NvDsFPSSensorInfo* fpssensorInfoToHash = s_fps_sensor_info_create(sensorInfo);
  /** save the sensor info into the hash map */
  g_hash_table_insert (appCtx->perf_struct.FPSInfoHash, GUINT_TO_POINTER(sensorInfo->source_id), fpssensorInfoToHash);
}

NvDsFPSSensorInfo* get_fps_sensor_info(AppCtx* appCtx, guint source_id) {
  NvDsFPSSensorInfo* sensorInfo = (NvDsFPSSensorInfo*)g_hash_table_lookup(appCtx->perf_struct.FPSInfoHash,
        GUINT_TO_POINTER(source_id));
  return sensorInfo;
}

static void s_fps_sensor_info_callback_stream_removed (AppCtx *appCtx, NvDsFPSSensorInfo* sensorInfo) {

  NvDsFPSSensorInfo* fpsensorInfoFromHash = get_fps_sensor_info(appCtx, sensorInfo->source_id);
  /** remove the sensor info from the hash map */
  if(fpsensorInfoFromHash) {
    g_hash_table_remove(appCtx->perf_struct.FPSInfoHash, GUINT_TO_POINTER(sensorInfo->source_id));
    s_fps_sensor_info_destroy(fpsensorInfoFromHash);
  }
}

static void
process_meta (AppCtx *appCtx, NvDsBatchMeta *batch_meta)
{
  if (!appCtx->config.tiled_display_config.enable ||
      appCtx->config.num_source_sub_bins == 1) {
    appCtx->show_bbox_text = 1;
  }

  for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;
      l_frame = l_frame->next) {
    NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) l_frame->data;
    for (NvDsMetaList * l_obj = frame_meta->obj_meta_list; l_obj != NULL;
        l_obj = l_obj->next) {
      NvDsObjectMeta *obj = (NvDsObjectMeta *) l_obj->data;
      gint class_index = obj->class_id;
      NvDsGieConfig *gie_config = NULL;
      gchar *str_ins_pos = NULL;

      if (obj->unique_component_id ==
          (gint) appCtx->config.primary_gie_config.unique_id) {
        gie_config = &appCtx->config.primary_gie_config;
      } else {
        for (gint i = 0; i < (gint)appCtx->config.num_secondary_gie_sub_bins; i++) {
          gie_config = &appCtx->config.secondary_gie_sub_bin_config[i];
        }
      }

    obj->text_params.display_text = (char *) g_malloc (128);
    obj->text_params.display_text[0] = '\0';
    str_ins_pos = obj->text_params.display_text;

    if (obj->obj_label[0] != '\0')
      sprintf (str_ins_pos, "%s", obj->obj_label);
    str_ins_pos += strlen (str_ins_pos);

    if (obj->object_id != UNTRACKED_OBJECT_ID) {
      if (appCtx->config.tracker_config.display_tracking_id) {
        guint64 const LOW_32_MASK = 0x00000000FFFFFFFF;
        sprintf (str_ins_pos, "%lu", (obj->object_id & LOW_32_MASK));
        str_ins_pos += strlen (str_ins_pos);
      }
    }
    }
  }
}

static GstPadProbeReturn
gie_processing_done_buf_prob (GstPad * pad, GstPadProbeInfo * info,
    gpointer u_data)
{
  GstBuffer *buf = (GstBuffer *) info->data;
  NvDsInstanceBin *bin = (NvDsInstanceBin *) u_data;
  guint index = bin->index;
  AppCtx *appCtx = bin->appCtx;

  if (gst_buffer_is_writable (buf))
    process_buffer (buf, appCtx, index);
  return GST_PAD_PROBE_OK;
}

