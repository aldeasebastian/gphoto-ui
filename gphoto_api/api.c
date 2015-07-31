#include "api.h"

void add_widget(widgets_list_node **first,CameraWidget *node)
{
	widgets_list_node *x;
	if((*first)->widget==NULL)
	{
		(*first)->widget=node;
		gp_widget_get_name (node, &(*first)->name);
		
	}
	else
	{
		widgets_list_node *aux;
		aux = (widgets_list_node*) malloc(sizeof(widgets_list_node));
		aux->widget=node;
		gp_widget_get_name (node, &aux->name);	
		aux->next=NULL;
		x=*first;
		while(x->next!=NULL)
		{
			x=x->next;
		}
		x->next=aux;
	}
}

void free_all_widgets(widgets_list_node *first)
{
	widgets_list_node *aux;
	while(first->next!=NULL)
	{
		aux=first;
		first=first->next;
		free(aux);
	}
	free(first);
}

void get_all_widgets(Camera	*camera,GPContext *context,CameraWidget *widget, char *prefix,widgets_list_node *first)
{
	int ret, n, i;
	char *newprefix;
	const char *label, *name, *uselabel;
	CameraWidgetType type;

	gp_widget_get_label (widget, &label);
	/* fprintf(stderr,"label is %s\n", label); */
	ret = gp_widget_get_name (widget, &name);
	/* fprintf(stderr,"name is %s\n", name); */
	gp_widget_get_type (widget, &type);

	if (strlen(name))
		uselabel = name;
	else
		uselabel = label;

	n = gp_widget_count_children (widget);

	newprefix = malloc(strlen(prefix)+1+strlen(uselabel)+1);
	if (!newprefix)
	{
		abort();
	}
	sprintf(newprefix,"%s/%s",prefix,uselabel);

	if ((type != GP_WIDGET_WINDOW) && (type != GP_WIDGET_SECTION)) 
	{
		// int readonly;
		// gp_widget_get_readonly(widget,&readonly);
		// printf("name: %s  readonly: %i \n",name,readonly);
		add_widget(&first,widget);
	}
	for (i=0; i<n; i++) 
	{
		CameraWidget *child;
	
		ret = gp_widget_get_child (widget, i, &child);
		if (ret != GP_OK)
			continue;

		get_all_widgets(camera,context,child,newprefix,first);
	}
	free(newprefix);
}

int init_camera(Camera **camera,GPContext **context)
{
	int retval;
	*context = gp_context_new();
	gp_camera_new(camera);
	retval = gp_camera_init(*camera, *context);
	return retval;
}

void capture_photo(Camera *camera, GPContext *context, char *fn) 
{
	int fd, retval;
	CameraFile *file;
	CameraFilePath camera_file_path;

	printf("Capturing.\n");
	int i=0;
	/* NOP: This gets overridden in the library to /capt0000.jpg */
	strcpy(camera_file_path.folder, "/");
	strcpy(camera_file_path.name, "foo.jpg");

	retval = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, context);
	printf("  Retval: %d\n", retval);

	printf("Pathname on the camera: %s/%s\n", camera_file_path.folder, camera_file_path.name);

	fd = open(fn, O_CREAT | O_WRONLY, 0644);
	retval = gp_file_new_from_fd(&file, fd);
	printf("  Retval: %d\n", retval);
	retval = gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name,GP_FILE_TYPE_NORMAL, file, context);
	printf("  Retval: %d\n", retval);

	printf("Deleting.\n");
	retval = gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name,
			context);
	printf("  Retval: %d\n", retval);

	gp_file_free(file);
}

void capture_preview(Camera *camera, GPContext *context, const char **ptr, unsigned long int *size) 
{
	int retval;
	CameraFile *file;
	CameraFilePath camera_file_path;

	printf("Capturing.\n");

	/* NOP: This gets overridden in the library to /capt0000.jpg */
	strcpy(camera_file_path.folder, "/");
	strcpy(camera_file_path.name, "foo.jpg");

	retval = gp_camera_capture_preview(camera, &camera_file_path, context);
	printf("  Retval: %d\n", retval);

	printf("Pathname on the camera: %s/%s\n", camera_file_path.folder, camera_file_path.name);

	retval = gp_file_new(&file);
	printf("  Retval: %d\n", retval);
	retval = gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name,
		     GP_FILE_TYPE_NORMAL, file, context);
	printf("  Retval: %d\n", retval);

	gp_file_get_data_and_size (file, ptr, size);

	printf("Deleting.\n");
	retval = gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name,
			context);
	printf("  Retval: %d\n", retval);
	/*gp_file_free(file); */
}

int _lookup_widget(CameraWidget*widget, const char *key, CameraWidget **child)
{
	int ret;
	ret = gp_widget_get_child_by_name (widget, key, child);
	return ret;
}
int get_config_value_string (Camera *camera, const char *key, char **str, GPContext *context)
{
	CameraWidget *widget = NULL, *child = NULL;
	CameraWidgetType type;
	int	ret;
	char *val;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	
	ret = _lookup_widget (widget, key, &child);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "lookup widget failed: %d\n", ret);
		goto out;
	}
	/* This type check is optional, if you know what type the label
	 * has already. If you are not sure, better check. */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out;
	}
	switch (type) 
	{
		case GP_WIDGET_MENU:
		case GP_WIDGET_RADIO:
		case GP_WIDGET_TEXT:
		break;
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out;
	}
	/* This is the actual query call. Note that we just
	 * a pointer reference to the string, not a copy... */
	ret = gp_widget_get_value (child, &val);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "could not query widget value: %d\n", ret);
		goto out;
	}
	/* Create a new copy for our caller. */
	*str = strdup (val);
out:
	gp_widget_free (widget);
	return ret;
}
/* Sets a string configuration value.
 * This can set for:
 *  - A Text widget
 *  - The current selection of a Radio Button choice
 *  - The current selection of a Menu choice
 
 * Sample (for Canons eg):
 *   get_config_value_string (camera, "owner", &ownerstr, context);
 */

int set_config_value_string (Camera *camera, const char *key, char *val, GPContext *context)
{
	CameraWidget *widget = NULL, *child = NULL;
	CameraWidgetType type;
	int ret;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK)
	{
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = _lookup_widget (widget, key, &child);
	if (ret < GP_OK)
	{
		fprintf (stderr, "lookup widget failed: %d\n", ret);
		goto out;
	}
	/* This type check is optional, if you know what type the label
	 * has already. If you are not sure, better check. */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out;
	}
	switch (type)
	{
        case GP_WIDGET_MENU:
        case GP_WIDGET_RADIO:
        case GP_WIDGET_TEXT:
		break;
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out;
	}
	/* This is the actual set call. Note that we keep
	 * ownership of the string and have to free it if necessary.
	 */
	ret = gp_widget_set_value (child, val);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "could not set widget value: %d\n", ret);
		goto out;
	}
	/* This stores it on the camera again */
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "camera_set_config failed: %d\n", ret);
		return ret;
	}
out:
	gp_widget_free (widget);
	return ret;
}


int get_config_value_float (Camera *camera, const char *key, float *value, GPContext *context)
{
	CameraWidget *widget = NULL, *child = NULL;
	CameraWidgetType type;
	int	ret;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	
	ret = _lookup_widget (widget, key, &child);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "lookup widget failed: %d\n", ret);
		goto out;
	}
	/* This type check is optional, if you know what type the label
	 * has already. If you are not sure, better check. */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out;
	}
	switch (type) 
	{
        case GP_WIDGET_RANGE:
		break;
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out;
	}
	/* This is the actual query call. Note that we just
	 * a pointer reference to the string, not a copy... */
	ret = gp_widget_get_value (child, value);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "could not query widget value: %d\n", ret);
		goto out;
	}
out:
	gp_widget_free (widget);
	return ret;
}


int set_config_value_float (Camera *camera, const char *key, float *value, GPContext *context)
{
	CameraWidget *widget = NULL, *child = NULL;
	CameraWidgetType type;
	int ret;

	printf("Hai sa vedem ce e busit dintre camera si context %p %p\n",camera,context);
	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK)
	{
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = _lookup_widget (widget, key, &child);
	if (ret < GP_OK)
	{
		printf("name: %s",key);
		fprintf (stderr, "lookup widget failed: %d\n", ret);
		goto out;
	}
	/* This type check is optional, if you know what type the label
	 * has already. If you are not sure, better check. */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out;
	}
	switch (type) 
	{
        case GP_WIDGET_RANGE:
		break;
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out;
	}
	ret = gp_widget_set_value (child, value);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "could not set widget value: %d\n", ret);
		goto out;
	}

	/* This stores it on the camera again */
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) 
	{
		fprintf (stderr, "camera_set_config failed: %d\n", ret);
		return ret;
	}
out:
	gp_widget_free (widget);
	return ret;
}
