#ifndef GPHOTO_API
#define GPHOTO_API

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <gphoto2/gphoto2.h>

typedef struct _widgets_list_node
{
	CameraWidget *widget;
	const char *name;
	struct _widgets_list_node *next;
}widgets_list_node;


void capture_photo(Camera *camera, GPContext *context, char *fn);
void capture_preview(Camera *camera, GPContext *context, const char **ptr, unsigned long int *size);
int init_camera(Camera **camera,GPContext **context);
int _lookup_widget(CameraWidget *widget, const char *key, CameraWidget **child);
int _lookup_widget_copy(CameraWidget *widget, const char *key, CameraWidget **child);
int get_config_value_string (Camera *camera, const char *key, char **str, GPContext *context);
int set_config_value_string (Camera *camera, const char *key, char *val, GPContext *context);
int get_config_value_float (Camera *camera, const char *key, float *value, GPContext *context);
int set_config_value_float (Camera *camera, const char *key, float *value, GPContext *context);
void get_all_widgets(Camera	*camera,GPContext *context,CameraWidget *widget, char *prefix,widgets_list_node *first);
void add_widget(widgets_list_node **first,CameraWidget *node);
void free_all_widgets(widgets_list_node *first);
void gp_api_widget_count_choices(CameraWidget *widget,const char *name);
#endif