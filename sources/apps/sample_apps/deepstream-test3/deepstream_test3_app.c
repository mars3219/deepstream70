/*
 * Copyright (c) 2018-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <gst/gst.h>
#include <glib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <cuda_runtime_api.h>

#include "gstnvdsmeta.h"
#include "nvds_yml_parser.h"
#include "gst-nvmessage.h"

#include <gst/rtsp-server/rtsp-server.h>

#define MAX_DISPLAY_LEN 64

#define PGIE_CLASS_ID_VEHICLE 0
#define PGIE_CLASS_ID_PERSON 2

/* By default, OSD process-mode is set to CPU_MODE. To change mode, set as:
 * 1: GPU mode (for Tesla only)
 * 2: HW mode (For Jetson only)
 */
#define OSD_PROCESS_MODE 1

/* By default, OSD will not display text. To display text, change this to 1 */
#define OSD_DISPLAY_TEXT 0

/* The muxer output resolution must be set if the input streams will be of
 * different resolution. The muxer will scale all the input frames to this
 * resolution. */
#define MUXER_OUTPUT_WIDTH 1920
#define MUXER_OUTPUT_HEIGHT 1080

/* Muxer batch formation timeout, for e.g. 40 millisec. Should ideally be set
 * based on the fastest source's framerate. */
#define MUXER_BATCH_TIMEOUT_USEC 40000

#define TILED_OUTPUT_WIDTH 1280
#define TILED_OUTPUT_HEIGHT 720

/* NVIDIA Decoder source pad memory feature. This feature signifies that source
 * pads having this capability will push GstBuffers containing cuda buffers. */
#define GST_CAPS_FEATURES_NVMM "memory:NVMM"

gchar pgie_classes_str[4][32] = { "Vehicle", "TwoWheeler", "Person",
  "RoadSign"
};

static gboolean PERF_MODE = FALSE;

guint udp_port  = 5400;

/* tiler_sink_pad_buffer_probe  will extract metadata received on OSD sink pad
 * and update params for drawing rectangle, object information etc. */

static GstPadProbeReturn
tiler_src_pad_buffer_probe (GstPad * pad, GstPadProbeInfo * info,
    gpointer u_data)
{
    GstBuffer *buf = (GstBuffer *) info->data;
    guint num_rects = 0; 
    NvDsObjectMeta *obj_meta = NULL;
    guint vehicle_count = 0;
    guint person_count = 0;
    NvDsMetaList * l_frame = NULL;
    NvDsMetaList * l_obj = NULL;
    //NvDsDisplayMeta *display_meta = NULL;

    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buf);

    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL;
      l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);
        //int offset = 0;
        for (l_obj = frame_meta->obj_meta_list; l_obj != NULL;
                l_obj = l_obj->next) {
            obj_meta = (NvDsObjectMeta *) (l_obj->data);
            if (obj_meta->class_id == PGIE_CLASS_ID_VEHICLE) {
                vehicle_count++;
                num_rects++;
            }
            if (obj_meta->class_id == PGIE_CLASS_ID_PERSON) {
                person_count++;
                num_rects++;
            }
        }
          g_print ("Frame Number = %d Number of objects = %d "
            "Vehicle Count = %d Person Count = %d\n",
            frame_meta->frame_num, num_rects, vehicle_count, person_count);
#if 0
        display_meta = nvds_acquire_display_meta_from_pool(batch_meta);
        NvOSD_TextParams *txt_params  = &display_meta->text_params;
        txt_params->display_text = g_malloc0 (MAX_DISPLAY_LEN);
        offset = snprintf(txt_params->display_text, MAX_DISPLAY_LEN, "Person = %d ", person_count);
        offset = snprintf(txt_params->display_text + offset , MAX_DISPLAY_LEN, "Vehicle = %d ", vehicle_count);

        /* Now set the offsets where the string should appear */
        txt_params->x_offset = 10;
        txt_params->y_offset = 12;

        /* Font , font-color and font-size */
        txt_params->font_params.font_name = "Serif";
        txt_params->font_params.font_size = 10;
        txt_params->font_params.font_color.red = 1.0;
        txt_params->font_params.font_color.green = 1.0;
        txt_params->font_params.font_color.blue = 1.0;
        txt_params->font_params.font_color.alpha = 1.0;

        /* Text background color */
        txt_params->set_bg_clr = 1;
        txt_params->text_bg_clr.red = 0.0;
        txt_params->text_bg_clr.green = 0.0;
        txt_params->text_bg_clr.blue = 0.0;
        txt_params->text_bg_clr.alpha = 1.0;

        nvds_add_display_meta_to_frame(frame_meta, display_meta);
#endif

    }
    return GST_PAD_PROBE_OK;
}

static gboolean
bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_WARNING:
    {
      gchar *debug;
      GError *error;
      gst_message_parse_warning (msg, &error, &debug);
      g_printerr ("WARNING from element %s: %s\n",
          GST_OBJECT_NAME (msg->src), error->message);
      g_free (debug);
      g_printerr ("Warning: %s\n", error->message);
      g_error_free (error);
      break;
    }
    case GST_MESSAGE_ERROR:
    {
      gchar *debug;
      GError *error;
      gst_message_parse_error (msg, &error, &debug);
      g_printerr ("ERROR from element %s: %s\n",
          GST_OBJECT_NAME (msg->src), error->message);
      if (debug)
        g_printerr ("Error details: %s\n", debug);
      g_free (debug);
      g_error_free (error);
      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_ELEMENT:
    {
      if (gst_nvmessage_is_stream_eos (msg)) {
        guint stream_id;
        if (gst_nvmessage_parse_stream_eos (msg, &stream_id)) {
          g_print ("Got EOS from stream %d\n", stream_id);
        }
      }
      break;
    }
    default:
      break;
  }
  return TRUE;
}

static void
cb_newpad (GstElement * decodebin, GstPad * decoder_src_pad, gpointer data)
{
  GstCaps *caps = gst_pad_get_current_caps (decoder_src_pad);
  if (!caps) {
    caps = gst_pad_query_caps (decoder_src_pad, NULL);
  }
  const GstStructure *str = gst_caps_get_structure (caps, 0);
  const gchar *name = gst_structure_get_name (str);
  GstElement *source_bin = (GstElement *) data;
  GstCapsFeatures *features = gst_caps_get_features (caps, 0);

  /* Need to check if the pad created by the decodebin is for video and not
   * audio. */
  if (!strncmp (name, "video", 5)) {
    /* Link the decodebin pad only if decodebin has picked nvidia
     * decoder plugin nvdec_*. We do this by checking if the pad caps contain
     * NVMM memory features. */
    if (gst_caps_features_contains (features, GST_CAPS_FEATURES_NVMM)) {
      /* Get the source bin ghost pad */
      GstPad *bin_ghost_pad = gst_element_get_static_pad (source_bin, "src");
      if (!gst_ghost_pad_set_target (GST_GHOST_PAD (bin_ghost_pad),
              decoder_src_pad)) {
        g_printerr ("Failed to link decoder src pad to source bin ghost pad\n");
      }
      gst_object_unref (bin_ghost_pad);
    } else {
      g_printerr ("Error: Decodebin did not pick nvidia decoder plugin.\n");
    }
  }
}

static void
decodebin_child_added (GstChildProxy * child_proxy, GObject * object,
    gchar * name, gpointer user_data)
{
  g_print ("Decodebin child added: %s\n", name);
  if (g_strrstr (name, "decodebin") == name) {
    g_signal_connect (G_OBJECT (object), "child-added",
        G_CALLBACK (decodebin_child_added), user_data);
  }
  if (g_strrstr (name, "source") == name) {
        g_object_set(G_OBJECT(object),"drop-on-latency",true,NULL);
  }

}

static GstElement *
create_source_bin (guint index, gchar * uri)
{
  GstElement *bin = NULL, *uri_decode_bin = NULL;
  gchar bin_name[16] = { };

  g_snprintf (bin_name, 15, "source-bin-%02d", index);
  /* Create a source GstBin to abstract this bin's content from the rest of the
   * pipeline */
  bin = gst_bin_new (bin_name);

  /* Source element for reading from the uri.
   * We will use decodebin and let it figure out the container format of the
   * stream and the codec and plug the appropriate demux and decode plugins. */
  if (PERF_MODE) {
    uri_decode_bin = gst_element_factory_make ("nvurisrcbin", "uri-decode-bin");
    g_object_set (G_OBJECT (uri_decode_bin), "file-loop", TRUE, NULL);
  } else {
    uri_decode_bin = gst_element_factory_make ("uridecodebin", "uri-decode-bin");
  }

  if (!bin || !uri_decode_bin) {
    g_printerr ("One element in source bin could not be created.\n");
    return NULL;
  }

  /* We set the input uri to the source element */
  g_object_set (G_OBJECT (uri_decode_bin), "uri", uri, NULL);

  /* Connect to the "pad-added" signal of the decodebin which generates a
   * callback once a new pad for raw data has beed created by the decodebin */
  g_signal_connect (G_OBJECT (uri_decode_bin), "pad-added",
      G_CALLBACK (cb_newpad), bin);
  g_signal_connect (G_OBJECT (uri_decode_bin), "child-added",
      G_CALLBACK (decodebin_child_added), bin);

  gst_bin_add (GST_BIN (bin), uri_decode_bin);

  /* We need to create a ghost pad for the source bin which will act as a proxy
   * for the video decoder src pad. The ghost pad will not have a target right
   * now. Once the decode bin creates the video decoder and generates the
   * cb_newpad callback, we will set the ghost pad target to the video decoder
   * src pad. */
  if (!gst_element_add_pad (bin, gst_ghost_pad_new_no_target ("src",
              GST_PAD_SRC))) {
    g_printerr ("Failed to add ghost pad in source bin\n");
    return NULL;
  }

  return bin;
}

static GstRTSPServer *server;
static gboolean
start_rtsp_streaming (guint rtsp_port_num, guint updsink_port_num,
                      guint64 udp_buffer_size)
{
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    char udpsrc_pipeline[512];

    char port_num_Str[64] = { 0 };
    char *encoder_name;

    if (udp_buffer_size == 0)
        udp_buffer_size = 512 * 1024;

    sprintf (udpsrc_pipeline,
             "( udpsrc name=pay0 port=%d buffer-size=%lu caps=\"application/x-rtp, media=video, "
             "clock-rate=90000, encoding-name=H264, payload=96 \" )",
             updsink_port_num, udp_buffer_size);

    sprintf (port_num_Str, "%d", rtsp_port_num);

    server = gst_rtsp_server_new ();
    g_object_set (server, "service", port_num_Str, NULL);

    mounts = gst_rtsp_server_get_mount_points (server);

    factory = gst_rtsp_media_factory_new ();
    gst_rtsp_media_factory_set_launch (factory, udpsrc_pipeline);

    gst_rtsp_mount_points_add_factory (mounts, "/ds-test", factory);

    g_object_unref (mounts);

    gst_rtsp_server_attach (server, NULL);

    g_print
            ("\n *** DeepStream: Launched RTSP Streaming at rtsp://localhost:%d/ds-test ***\n\n",
             rtsp_port_num);

    return TRUE;
}

static GstRTSPFilterResult
client_filter (GstRTSPServer * server, GstRTSPClient * client,
               gpointer user_data)
{
    return GST_RTSP_FILTER_REMOVE;
}

static void
destroy_sink_bin ()
{
    GstRTSPMountPoints *mounts;
    GstRTSPSessionPool *pool;

    mounts = gst_rtsp_server_get_mount_points (server);
    gst_rtsp_mount_points_remove_factory (mounts, "/ds-test");
    g_object_unref (mounts);
    gst_rtsp_server_client_filter (server, client_filter, NULL);
    pool = gst_rtsp_server_get_session_pool (server);
    gst_rtsp_session_pool_cleanup (pool);
    g_object_unref (pool);
}


int
main (int argc, char *argv[])
{
  GMainLoop *loop = NULL;
  GstElement *pipeline = NULL, *streammux = NULL, *sink = NULL, *pgie = NULL,
      *queue1, *queue2, *queue3, *queue4, *queue5, *nvvidconv = NULL,
      *nvosd = NULL, *tiler = NULL, *nvdslogger = NULL, 
      *nvvidconv1 = NULL, *filter4 = NULL, *parse = NULL, *rtppay = NULL, *encoder = NULL;
  GstElement *transform = NULL;
  GstBus *bus = NULL;
  guint bus_watch_id;
  GstPad *tiler_src_pad = NULL;
  guint i =0, num_sources = 0;
  guint tiler_rows, tiler_columns;
  guint pgie_batch_size;
  GstCaps *caps4 = NULL;
  PERF_MODE = g_getenv("NVDS_TEST3_PERF_MODE") &&
      !g_strcmp0(g_getenv("NVDS_TEST3_PERF_MODE"), "1");

  int current_device = -1;
  cudaGetDevice(&current_device);
  struct cudaDeviceProp prop;
  cudaGetDeviceProperties(&prop, current_device);

  /* Check input arguments */
  if (argc < 2) {
    g_printerr ("Usage: %s <yml file>\n", argv[0]);
    g_printerr ("OR: %s <uri1> [uri2] ... [uriN] \n", argv[0]);
    return -1;
  }

  /* Standard GStreamer initialization */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* Create gstreamer elements */
  /* Create Pipeline element that will form a connection of other elements */
  pipeline = gst_pipeline_new ("dstest3-pipeline");

  /* Create nvstreammux instance to form batches from one or more sources. */
  streammux = gst_element_factory_make ("nvstreammux", "stream-muxer");

  if (!pipeline || !streammux) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }
  gst_bin_add (GST_BIN (pipeline), streammux);

  GList *src_list = NULL ;

  if (g_str_has_suffix (argv[1], ".yml") || g_str_has_suffix (argv[1], ".yaml")) {

    nvds_parse_source_list(&src_list, argv[1], "source-list");

    GList * temp = src_list;
    while(temp) {
      num_sources++;
      temp=temp->next;
    }
    g_list_free(temp);
  } else {
      num_sources = argc - 1;
  }

  for (i = 0; i < num_sources; i++) {
    GstPad *sinkpad, *srcpad;
    gchar pad_name[16] = { };

    GstElement *source_bin= NULL;
    if (g_str_has_suffix (argv[1], ".yml") || g_str_has_suffix (argv[1], ".yaml")) {
      g_print("Now playing : %s\n",(char*)(src_list)->data);
      source_bin = create_source_bin (i, (char*)(src_list)->data);
    } else {
      source_bin = create_source_bin (i, argv[i + 1]);
    }
    if (!source_bin) {
      g_printerr ("Failed to create source bin. Exiting.\n");
      return -1;
    }

    gst_bin_add (GST_BIN (pipeline), source_bin);

    g_snprintf (pad_name, 15, "sink_%u", i);
    sinkpad = gst_element_get_request_pad (streammux, pad_name);
    if (!sinkpad) {
      g_printerr ("Streammux request sink pad failed. Exiting.\n");
      return -1;
    }

    srcpad = gst_element_get_static_pad (source_bin, "src");
    if (!srcpad) {
      g_printerr ("Failed to get src pad of source bin. Exiting.\n");
      return -1;
    }

    if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK) {
      g_printerr ("Failed to link source bin to stream muxer. Exiting.\n");
      return -1;
    }

    gst_object_unref (srcpad);
    gst_object_unref (sinkpad);

    if (g_str_has_suffix (argv[1], ".yml") || g_str_has_suffix (argv[1], ".yaml")) {
      src_list = src_list->next;
    }
  }

  if (g_str_has_suffix (argv[1], ".yml") || g_str_has_suffix (argv[1], ".yaml")) {
    g_list_free(src_list);
  }

  /* Use nvinfer to infer on batched frame. */
  pgie = gst_element_factory_make ("nvinfer", "primary-nvinference-engine");

  /* Add queue elements between every two elements */
  queue1 = gst_element_factory_make ("queue", "queue1");
  queue2 = gst_element_factory_make ("queue", "queue2");
  queue3 = gst_element_factory_make ("queue", "queue3");
  queue4 = gst_element_factory_make ("queue", "queue4");
  queue5 = gst_element_factory_make ("queue", "queue5");

  /* Use nvdslogger for perf measurement. */
  nvdslogger = gst_element_factory_make ("nvdslogger", "nvdslogger");

  /* Use nvtiler to composite the batched frames into a 2D tiled array based
   * on the source of the frames. */
  tiler = gst_element_factory_make ("nvmultistreamtiler", "nvtiler");

  /* Use convertor to convert from NV12 to RGBA as required by nvosd */
  nvvidconv = gst_element_factory_make ("nvvideoconvert", "nvvideo-converter");

  /* Create OSD to draw on the converted RGBA buffer */
  nvosd = gst_element_factory_make ("nvdsosd", "nv-onscreendisplay");

  if (PERF_MODE) {
    sink = gst_element_factory_make ("fakesink", "nvvideo-renderer");
  } else {
    /* Finally render the osd output */
    // if(prop.integrated) {
    //   transform = gst_element_factory_make ("nvegltransform", "nvegl-transform");
    //   if(!transform) {
    //     g_printerr ("One tegra element could not be created. Exiting.\n");
    //     return -1;
    //   }
    // }
    // sink = gst_element_factory_make ("nveglglessink", "nvvideo-renderer");
    
    nvvidconv1 = gst_element_factory_make ("nvvideoconvert", "nvvideo-converter1");

    filter4 = gst_element_factory_make ("capsfilter", "filter4");
    caps4 = gst_caps_from_string ("video/x-raw(memory:NVMM), format=I420");
    g_object_set (G_OBJECT (filter4), "caps", caps4, NULL);
    gst_caps_unref (caps4);

    // x264enc = gst_element_factory_make ("x264enc", "h264 encoder");
    encoder = gst_element_factory_make ("nvv4l2h264enc", "nvv4l2h264encoder"); 
    parse = gst_element_factory_make ("h264parse", "h264-parser2");
    rtppay = gst_element_factory_make ("rtph264pay", "rtp-payer");
    sink = gst_element_factory_make ("udpsink", "udp-sink");
    g_object_set (G_OBJECT (encoder), "bitrate", 4000000, NULL);
    g_object_set (G_OBJECT (encoder), "iframeinterval", 30, NULL);
    g_object_set (G_OBJECT (encoder), "profile", 2, NULL);
    g_object_set (G_OBJECT (sink), "host", "127.0.0.1", "port",
                  udp_port, "async", FALSE, "sync", 1, NULL);
  }

  if (!pgie || !nvdslogger || !tiler || !nvvidconv || !nvosd || !sink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  if (!nvvidconv1 || !filter4 || !encoder || !parse || !rtppay ) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  if (g_str_has_suffix (argv[1], ".yml") || g_str_has_suffix (argv[1], ".yaml")) {

    nvds_parse_streammux(streammux, argv[1],"streammux");

    g_object_set (G_OBJECT (pgie),
      "config-file-path", "dstest3_pgie_config.yml", NULL);

    g_object_get (G_OBJECT (pgie), "batch-size", &pgie_batch_size, NULL);
    if (pgie_batch_size != num_sources) {
      g_printerr
          ("WARNING: Overriding infer-config batch-size (%d) with number of sources (%d)\n",
          pgie_batch_size, num_sources);
      g_object_set (G_OBJECT (pgie), "batch-size", num_sources, NULL);
    }

    nvds_parse_osd(nvosd, argv[1],"osd");

    tiler_rows = (guint) sqrt (num_sources);
    tiler_columns = (guint) ceil (1.0 * num_sources / tiler_rows);
    g_object_set (G_OBJECT (tiler), "rows", tiler_rows, "columns", tiler_columns, NULL);

    nvds_parse_tiler(tiler, argv[1], "tiler");
    // nvds_parse_egl_sink(sink, argv[1], "sink");
  }
  else {

    g_object_set (G_OBJECT (streammux), "batch-size", num_sources, NULL);

    g_object_set (G_OBJECT (streammux), "width", MUXER_OUTPUT_WIDTH, "height",
        MUXER_OUTPUT_HEIGHT,
        "batched-push-timeout", MUXER_BATCH_TIMEOUT_USEC, NULL);

    /* Configure the nvinfer element using the nvinfer config file. */
    g_object_set (G_OBJECT (pgie),
        "config-file-path", "dstest3_pgie_config.txt", NULL);

    /* Override the batch-size set in the config file with the number of sources. */
    g_object_get (G_OBJECT (pgie), "batch-size", &pgie_batch_size, NULL);
    if (pgie_batch_size != num_sources) {
      g_printerr
          ("WARNING: Overriding infer-config batch-size (%d) with number of sources (%d)\n",
          pgie_batch_size, num_sources);
      g_object_set (G_OBJECT (pgie), "batch-size", num_sources, NULL);
    }

    tiler_rows = (guint) sqrt (num_sources);
    tiler_columns = (guint) ceil (1.0 * num_sources / tiler_rows);
    /* we set the tiler properties here */
    g_object_set (G_OBJECT (tiler), "rows", tiler_rows, "columns", tiler_columns,
        "width", TILED_OUTPUT_WIDTH, "height", TILED_OUTPUT_HEIGHT, NULL);

    g_object_set (G_OBJECT (nvosd), "process-mode", OSD_PROCESS_MODE,
        "display-text", OSD_DISPLAY_TEXT, NULL);

    g_object_set (G_OBJECT (sink), "qos", 0, NULL);

  }

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* Set up the pipeline */
  /* we add all elements into the pipeline */
  if(transform) {
    gst_bin_add_many (GST_BIN (pipeline), queue1, pgie, queue2, nvdslogger, tiler,
        queue3, nvvidconv, queue4, nvosd, queue5, transform, 
        nvvidconv1, filter4, encoder, parse, rtppay, sink, NULL);
    /* we link the elements together
    * nvstreammux -> nvinfer -> nvdslogger -> nvtiler -> nvvidconv -> nvosd
    * -> video-renderer */
    if (!gst_element_link_many (streammux, queue1, pgie, queue2, nvdslogger, tiler,
          queue3, nvvidconv, queue4, nvosd, queue5, transform, 
          nvvidconv1, filter4, encoder, parse, rtppay, sink, NULL)) {
      g_printerr ("Elements could not be linked. Exiting.\n");
      return -1;
    }
  }
  else {
  gst_bin_add_many (GST_BIN (pipeline), queue1, pgie, queue2, nvdslogger, tiler,
      queue3, nvvidconv, queue4, nvosd, queue5, 
      nvvidconv1, filter4, encoder, parse, rtppay, sink, NULL);
    /* we link the elements together
    * nvstreammux -> nvinfer -> nvdslogger -> nvtiler -> nvvidconv -> nvosd
    * -> video-renderer */
    if (!gst_element_link_many (streammux, queue1, pgie, queue2, nvdslogger, tiler,
          queue3, nvvidconv, queue4, nvosd, queue5, 
          nvvidconv1, filter4, encoder, parse, rtppay, sink, NULL)) {
      g_printerr ("Elements could not be linked. Exiting.\n");
      return -1;
    }
  }

  
  /* Lets add probe to get informed of the meta data generated, we add probe to
   * the sink pad of the osd element, since by that time, the buffer would have
   * had got all the metadata. */
  tiler_src_pad = gst_element_get_static_pad (pgie, "src");
  if (!tiler_src_pad)
    g_print ("Unable to get src pad\n");
  else
    gst_pad_add_probe (tiler_src_pad, GST_PAD_PROBE_TYPE_BUFFER,
        tiler_src_pad_buffer_probe, NULL, NULL);
  gst_object_unref (tiler_src_pad);

  /* Set the pipeline to "playing" state */
  if (g_str_has_suffix (argv[1], ".yml") || g_str_has_suffix (argv[1], ".yaml")) {
    g_print ("Using file: %s\n", argv[1]);
  }
  else {
    g_print ("Now playing:");
    for (i = 0; i < num_sources; i++) {
      g_print (" %s,", argv[i + 1]);
    }
    g_print ("\n");
  }
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  start_rtsp_streaming (8554/*rtsp_port*/, udp_port, 0);
  /* Wait till pipeline encounters an error or EOS */
  g_print ("Running...\n");
  g_main_loop_run (loop);

  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);
  destroy_sink_bin();
  return 0;
}
