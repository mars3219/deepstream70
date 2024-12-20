#include "deepstream_app.h"
#include "deepstream_config_file_parser.h"
#include "nvds_version.h"
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <signal.h>

#define MAX_INSTANCES 128
#define APP_TITLE "DeepStream"

#define DEFAULT_X_WINDOW_WIDTH 1280
#define DEFAULT_X_WINDOW_HEIGHT 720

AppCtx *appCtx[MAX_INSTANCES];
static guint cintr = FALSE;
static GMainLoop *main_loop = NULL;
static gchar **cfg_files = NULL;
static gchar **input_uris = NULL;
static gboolean print_version = FALSE;
static gboolean show_bbox_text = FALSE;
static gboolean print_dependencies_version = FALSE;
static gboolean quit = FALSE;
static gint return_value = 0;
static guint num_instances;
static guint num_input_uris;
static GMutex fps_lock;
static gdouble fps[MAX_SOURCE_BINS];
static gdouble fps_avg[MAX_SOURCE_BINS];

static Display *display = NULL;
static Window windows[MAX_INSTANCES] = { 0 };

static GThread *x_event_thread = NULL;
static GMutex disp_lock;

static guint rrow, rcol, rcfg;
static gboolean rrowsel = FALSE, selecting = FALSE;


GST_DEBUG_CATEGORY (NVDS_APP);

GOptionEntry entries[] = {
  {"version", 'v', 0, G_OPTION_ARG_NONE, &print_version,
      "Print DeepStreamSDK version", NULL}
  ,
  {"tiledtext", 't', 0, G_OPTION_ARG_NONE, &show_bbox_text,
      "Display Bounding box labels in tiled mode", NULL}
  ,
  {"version-all", 0, 0, G_OPTION_ARG_NONE, &print_dependencies_version,
      "Print DeepStreamSDK and dependencies version", NULL}
  ,
  {"cfg-file", 'c', 0, G_OPTION_ARG_FILENAME_ARRAY, &cfg_files,
      "Set the config file", NULL}
  ,
  {"input-uri", 'i', 0, G_OPTION_ARG_FILENAME_ARRAY, &input_uris,
      "Set the input uri (file://stream or rtsp://stream)", NULL}
  ,
  {NULL}
  ,
};

static void
all_bbox_generated (AppCtx * appCtx, GstBuffer * buf,
    NvDsBatchMeta * batch_meta, guint index)
{
  guint num_male = 0;
  guint num_female = 0;
  guint num_objects[128];

  memset (num_objects, 0, sizeof (num_objects));

  for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;
      l_frame = l_frame->next) {
    NvDsFrameMeta *frame_meta = l_frame->data;
    for (NvDsMetaList * l_obj = frame_meta->obj_meta_list; l_obj != NULL;
        l_obj = l_obj->next) {
      NvDsObjectMeta *obj = (NvDsObjectMeta *) l_obj->data;
      if (obj->unique_component_id ==
          (gint) appCtx->config.primary_gie_config.unique_id) {
        if (obj->class_id >= 0 && obj->class_id < 128) {
          num_objects[obj->class_id]++;
        }
        if (appCtx->person_class_id > -1
            && obj->class_id == appCtx->person_class_id) {
          if (strstr (obj->text_params.display_text, "Man")) {
            str_replace (obj->text_params.display_text, "Man", "");
            str_replace (obj->text_params.display_text, "Person", "Man");
            num_male++;
          } else if (strstr (obj->text_params.display_text, "Woman")) {
            str_replace (obj->text_params.display_text, "Woman", "");
            str_replace (obj->text_params.display_text, "Person", "Woman");
            num_female++;
          }
        }
      }
    }
  }
}

static void
_intr_handler (int signum)
{
  struct sigaction action;
  NVGSTDS_ERR_MSG_V ("User Interrupted.. \n");

  memset (&action, 0, sizeof (action));
  action.sa_handler = SIG_DFL;

  sigaction (SIGINT, &action, NULL);
  cintr = TRUE;
}