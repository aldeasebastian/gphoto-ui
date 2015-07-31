#include "ui.h"

int top=0,i;
widgets_list_node *first;
GtkEntry *display;
GtkWidget *window,*grid,*preview;
Camera	*camera;
GPContext *context;

void change_setting(GtkWidget *widget, gpointer data)
{
	widgets_list_node *aux;
	const char *active_text,*widget_name;
	float *active_float;
	active_text = (char*)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
	widget_name = gtk_widget_get_name (widget);
	// printf("active_id=%s\n",active_text);
	// printf("widget_name=%s\n",widget_name);
	CameraWidgetType type;	
	aux=first;
	while(aux!=NULL)
	{
		if(strcmp(widget_name,aux->name)==0)
		{
			break;
		}
		aux=aux->next;
	}
	gp_widget_get_type(aux->widget,&type);
	//printf("type: %i\n",type);
	if((type == GP_WIDGET_MENU) || (type == GP_WIDGET_RADIO) || (type == GP_WIDGET_TEXT))
	{
		set_config_value_string(camera,widget_name,active_text,context);
	}
	if(type == GP_WIDGET_RANGE)
	{
		///de elimant eroarea !!!!!!!!!!!!
		active_float=(float*)malloc(sizeof(float*));
		*active_float=atof(active_text);
		set_config_value_float(camera,widget_name,active_float,context);
	}
	if((type == GP_WIDGET_TOGGLE)||(GP_WIDGET_DATE))
	{
		//de implementat functia!!!!!!!11
		//set_config_value_int(camera,widget_name,    ,context);
	}

	return;
}

void create_label_and_combobox(const char* name,CameraWidget *widget)
{
	int left=1,width=1,height=1,i,no_choices;
	float min,max,increment;
	char temp[1024];
	const char *choices;
	setting_node *set;
	set=(setting_node *)malloc(sizeof(setting_node));
	set->label = gtk_label_new (name);
	gtk_label_set_justify (GTK_LABEL (set->label), GTK_JUSTIFY_CENTER);
	gtk_label_set_line_wrap (GTK_LABEL (set->label), FALSE);
	gtk_grid_attach(GTK_GRID (grid),GTK_WIDGET (set->label),left,top,width,height);
	
	set->combo_box = gtk_combo_box_text_new ();
	no_choices=gp_widget_count_choices(widget);
	//!!!!!!!!!!!!!!
	//de creat choices pentru float
	//if(tipul e float:get min ,increment ....)
	CameraWidgetType type;
	gp_widget_get_type(widget,&type);

	//printf("no_choices= %i\n",no_choices);
	
	if((type == GP_WIDGET_MENU) || (type == GP_WIDGET_RADIO) || (type == GP_WIDGET_TEXT))
	{
		for(i=0;i<no_choices;i++)
		{
			gp_widget_get_choice(widget,i,&choices);
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (set->combo_box), choices);
		}
	}
	if(type == GP_WIDGET_RANGE)
	{
		gp_widget_get_range(widget,&min,&max,&increment);
		//printf("min=%f,max=%f,increment=%f\n",min,max,increment);
		for(i=min;i<max;i=i+increment)
		{
			sprintf(temp,"%i",i);
			//printf("temp=%s\n",temp);
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (set->combo_box),temp);
		}
	}
	if((type == GP_WIDGET_TOGGLE)||(GP_WIDGET_DATE))
	{
		//get choices
	}

	//printf("name=%s\n",name);
	gtk_widget_set_name(GTK_WIDGET (set->combo_box),name);
	gtk_combo_box_set_active (GTK_COMBO_BOX (set->combo_box), 0);
	gtk_grid_attach(GTK_GRID (grid),GTK_WIDGET (set->combo_box),left+1,top,width,height);
	
	g_signal_connect (set->combo_box,"changed",G_CALLBACK (change_setting),NULL);
	gtk_widget_show_all (grid);
	top++;
	free(set);
}
void add_setting(GtkWidget *widget, gpointer data)
{
	const gchar *text;
	widgets_list_node *x;
	int	ret;
	first = (widgets_list_node*) malloc(sizeof(widgets_list_node));
	first->widget=NULL;
	first->name=NULL;
	first->next=NULL;
	CameraWidget *rootconfig;

	ret = gp_camera_get_config (camera, &rootconfig,context);
	if (ret != GP_OK)
	{
		printf("failed to get CameraWidget\n");
		return;
	}
	get_all_widgets(camera,context,rootconfig,"",first);
	text = gtk_entry_get_text(display);

	if(strcmp(text,"")==0)
	{
		printf("display empty\n");
		return;
	}
	else
	{
		x=first;
		while(x!=NULL)
		{
			if(strcmp(text,x->name)==0)
			{
				//printf("name =  %s\n", x->name);
				///create label and combo box
				create_label_and_combobox(x->name,x->widget);

				gtk_entry_set_text(display,"");
				return;
			}
			x=x->next;
		}
	}
	gp_widget_free (rootconfig);
}

int preview_timer(void *p)
{
	static int c;
	gtk_widget_queue_draw (preview);
    return TRUE;
}

void initialize_camera(GtkWidget *widget, gpointer data)
{
	int retval;
	if(camera != NULL)
	{
		gp_camera_exit(camera, context);
	}
	printf("Camera init.  Takes about 10 seconds.\n");
	retval = init_camera(&camera,&context);
	if (retval != GP_OK) 
	{
		printf("  Retval of capture_to_file: %d\n", retval);
		exit (1);
	}
	//init preview
	g_idle_add ((GSourceFunc) preview_timer, NULL);
}

void take_photo(GtkWidget *widget, gpointer data)
{
	char *file;
	file=(char*)malloc(15*sizeof(char));
	sprintf(file,"snapshot%03d.jpg", i);
	printf("%s\n",file);
	i++;
	capture_photo(camera, context, file);
}

int camera_preview_draw (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	//TODO draw preview
}

int main(int argc, char **argv) 
{
	GtkBuilder *builder;
	GtkWidget /**add_button,*/*grid_container;
	gtk_init(&argc, &argv);
	builder = gtk_builder_new();
	if( gtk_builder_add_from_file( builder, "gphoto_ui.glade", NULL ) == 0 )
	{
		printf("Add from file failed\n");
		return 1;
	}
	window = GTK_WIDGET( gtk_builder_get_object( builder, "main_window" ));
	gtk_builder_connect_signals( builder, NULL);
	gtk_window_set_title(GTK_WINDOW(window), "GPHOTO UI");
	
	display = GTK_ENTRY( gtk_builder_get_object( builder, "display" ) );
	//add_button = GTK_WIDGET( gtk_builder_get_object( builder, "add_button" ));
	grid_container = GTK_WIDGET( gtk_builder_get_object( builder, "grid_container" ));
	grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (grid_container), GTK_WIDGET (grid));
	gtk_grid_set_row_homogeneous(GTK_GRID(grid),TRUE);
	gtk_grid_set_column_homogeneous(GTK_GRID(grid),TRUE);

	preview = GTK_WIDGET( gtk_builder_get_object( builder, "preview" ));


	//clear_button = GTK_WIDGET( gtk_builder_get_object( builder, "clear_button" ) );
	g_signal_connect_swapped(G_OBJECT(window), "destroy",G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_show_all(window);

	gtk_main();
	gp_camera_exit(camera, context);
	return 0;
}