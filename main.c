#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <cairo.h>
#include <math.h>

//todo:
//make loadavg animation smoother

//create a dynamic array

typedef struct
{
    cairo_t *cr;
} surfaceArray;

typedef struct
{
    char c[100];
} DisposableString;

typedef struct
{
    int c[20];
} disposable_num;

typedef struct
{
    long double c[4];
} DynamicArray;

typedef struct
{
    double c[100];
} dynamic_stored_load;

DynamicArray *dyn_a;
DynamicArray *dyn_b;

dynamic_stored_load *dyn_stored_load;

dynamic_stored_load *dyn_stored_net_load;
dynamic_stored_load *dyn_stored_disk_load;
long double dyn_stored_disk_load_slow_u[2];
DisposableString *dispose_string;

disposable_num *dispose_num;

//drawingArray *dyn_dwm;

char physical_drive[100];

char physical_drive_slow_u[100];

surfaceArray *dyn_surface;

int factorable_cpu_count[2];

int width_cpu_count;
int height_cpu_count;

static cairo_surface_t *surface = NULL;
static cairo_surface_t *surface_core_graph = NULL;
static cairo_surface_t *surface_ram = NULL;
static cairo_surface_t *surface_net = NULL;
static cairo_surface_t *surface_disk = NULL;

double initmemused;
double *loadavg;
double *onceloadavg;
double *fast_loadavg;
double *fast_onceloadavg;
double beat_stored_load[100] = {0};
int cpuid[33] = {0};
int cpuid_b[33] = {0};
long double a[33][4] = {0}, b[33][4] = {0};
int x;
int g_cpu_count;
FILE *fp;
char dump[50];
GtkWidget *g_lbl_count;
GtkWidget *g_id_cpu_name;
GtkWidget *g_id_ram_used;
GtkWidget *g_id_ram_used2;
GtkWidget *g_id_ram_used3;
GtkWidget *g_id_swap_used;
GtkWidget *g_id_swap_used1;
GtkWidget *g_id_network_out;
GtkWidget *g_id_network_in;
GtkWidget *g_id_disk_read;
GtkWidget *g_id_disk_write;

long double ram_u;
long double memtotal;
long double memfree;

long double swap_u;
long double swaptotal;
long double swapfree;

long double initial_inbound_temp;
long double initial_inbound_total = 0;
long double initial_outbound_temp;
long double initial_outbound_total = 0;

long double final_inbound_temp;
long double final_inbound_total = 0;
long double final_outbound_temp;
long double final_outbound_total = 0;

long double initial_inbound_temp_fast_u;
long double initial_inbound_total_fast_u = 0;
long double initial_outbound_temp_fast_u;
long double initial_outbound_total_fast_u = 0;

long double final_inbound_temp_fast_u;
long double final_inbound_total_fast_u = 0;
long double final_outbound_temp_fast_u;
long double final_outbound_total_fast_u = 0;

long double final_disk_read_temp = 0;
long double final_disk_read_total = 0;
long double final_disk_write_temp = 0;
long double final_disk_write_total = 0;

long double initial_disk_read_total = 0;
long double initial_disk_read_temp = 0;
long double initial_disk_write_total = 0;
long double initial_disk_write_temp = 0;


long double final_disk_read_temp_slow_u = 0;
long double final_disk_read_total_slow_u = 0;
long double final_disk_write_temp_slow_u = 0;
long double final_disk_write_total_slow_u = 0;

long double initial_disk_read_total_slow_u = 0;
long double initial_disk_read_temp_slow_u = 0;
long double initial_disk_write_total_slow_u = 0;
long double initial_disk_write_temp_slow_u = 0;

FILE *netinfo_fast_u;
FILE *netinfo;

gfloat f(gfloat y)
{
    return -4 * y; //return x*sin((y*x/M_PI)+M_PI);//y*50*sin ((x/M_PI)+M_PI);
}





static gboolean
on_configure_event_disk(
    GtkWidget *widget,
    GdkEventConfigure *event,
    gpointer data)
{
    static int save_w = 0, save_h = 0;

    if (save_w == event->width && save_h == event->height)
        return TRUE;

    save_w = event->width;
    save_h = event->height;

    if (surface_disk)
        cairo_surface_destroy(surface_disk);

    surface_disk = gdk_window_create_similar_surface(
        gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR,
        save_w, save_h);

    cairo_t *cr = cairo_create(surface_disk);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    cairo_destroy(cr);

    return TRUE;
}

static gboolean
on_draw_disk(
    GtkWidget *widget,
    cairo_t *cr,
    gpointer data)
{
    cairo_set_source_surface(cr, surface_disk, 0, 0);
    cairo_paint(cr);
    return TRUE;
}

gboolean
on_timeout_disk(
    gpointer data)
{
    //needs more data points in network
    GtkWidget *widget = GTK_WIDGET(data);

    cairo_t *cr = cairo_create(surface_disk);
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_paint(cr);

    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_line_width(cr, 1);
    //cairo_translate(cr, (width / 2), (height / 2));
    cairo_save(cr);
    //cairo_set_source_rgb(cr, 0.152941176, 0.545098039, 0.807843137);
    cairo_set_source_rgb(cr, 0.152941176, 0.545098039, 0.807843137);
    cairo_rectangle(cr, 0.5, 0.5, 239, 59);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0.176470588, 0.654901961, 0.97254902);
    cairo_translate(cr, 0.5, 59.5);
    /* Link each data point */
    int i = 0;
    for (i = 0; i < 100; i += 1)
    {
        cairo_line_to(cr, i * 4, f(dyn_stored_disk_load[0].c[i] * (60 - 0.5))); //i/exp(load/100)
    }

    /* Draw the curve */
    cairo_stroke(cr);

    //cairo_set_source_rgb(cr, 0.176470588, 0.654901961, 0.97254902);
    cairo_set_source_rgb(cr, 1, 0.223529412, 0.592156863);
    //cairo_translate(cr, 0.5, 49.5);
    /* Link each data point */
    i = 0;
    for (i = 0; i < 100; i += 1)
    {
        cairo_line_to(cr, i * 4, f(dyn_stored_disk_load[1].c[i] * (60 - 0.5))); //i/exp(load/100)
    }

    /* Draw the curve */
    cairo_stroke(cr);

    cairo_destroy(cr);

    gtk_widget_queue_draw(GTK_WIDGET(data));
    return TRUE;
}

static void on_activate6(GtkWidget *window, GtkBuilder *builder)
{
    GtkWidget *da;
    da = GTK_WIDGET(gtk_builder_get_object(builder, "dwmarea_disk"));
    gtk_widget_set_size_request(da, 240, 60);

    g_signal_connect(da, "draw",
                     G_CALLBACK(on_draw_disk), NULL);
    g_signal_connect(da, "configure-event",
                     G_CALLBACK(on_configure_event_disk), NULL);

    g_timeout_add(50, (GSourceFunc)on_timeout_disk, da);
}





































static gboolean
on_configure_event_net(
    GtkWidget *widget,
    GdkEventConfigure *event,
    gpointer data)
{
    static int save_w = 0, save_h = 0;

    if (save_w == event->width && save_h == event->height)
        return TRUE;

    save_w = event->width;
    save_h = event->height;

    if (surface_net)
        cairo_surface_destroy(surface_net);

    surface_net = gdk_window_create_similar_surface(
        gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR,
        save_w, save_h);

    cairo_t *cr = cairo_create(surface_net);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    cairo_destroy(cr);

    return TRUE;
}

static gboolean
on_draw_net(
    GtkWidget *widget,
    cairo_t *cr,
    gpointer data)
{
    cairo_set_source_surface(cr, surface_net, 0, 0);
    cairo_paint(cr);
    return TRUE;
}

gboolean
on_timeout_net(
    gpointer data)
{
    //needs more data points in network
    GtkWidget *widget = GTK_WIDGET(data);

    cairo_t *cr = cairo_create(surface_net);
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_paint(cr);

    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_line_width(cr, 1);
    //cairo_translate(cr, (width / 2), (height / 2));
    cairo_save(cr);
    //cairo_set_source_rgb(cr, 0.152941176, 0.545098039, 0.807843137);
    cairo_set_source_rgb(cr, 0.152941176, 0.545098039, 0.807843137);
    cairo_rectangle(cr, 0.5, 0.5, 239, 59);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0.176470588, 0.654901961, 0.97254902);
    cairo_translate(cr, 0.5, 59.5);
    /* Link each data point */
    int i = 0;
    for (i = 0; i < 100; i += 1)
    {
        cairo_line_to(cr, i * 4, f(dyn_stored_net_load[0].c[i] * (60 - 0.5))); //i/exp(load/100)
    }

    /* Draw the curve */
    cairo_stroke(cr);

    //cairo_set_source_rgb(cr, 0.176470588, 0.654901961, 0.97254902);
    cairo_set_source_rgb(cr, 1, 0.223529412, 0.592156863);
    //cairo_translate(cr, 0.5, 49.5);
    /* Link each data point */
    i = 0;
    for (i = 0; i < 100; i += 1)
    {
        cairo_line_to(cr, i * 4, f(dyn_stored_net_load[1].c[i] * (60 - 0.5))); //i/exp(load/100)
    }

    /* Draw the curve */
    cairo_stroke(cr);

    cairo_destroy(cr);

    gtk_widget_queue_draw(GTK_WIDGET(data));
    return TRUE;
}

static void on_activate5(GtkWidget *window, GtkBuilder *builder)
{
    GtkWidget *da;
    da = GTK_WIDGET(gtk_builder_get_object(builder, "dwmarea_net"));
    gtk_widget_set_size_request(da, 240, 60);

    g_signal_connect(da, "draw",
                     G_CALLBACK(on_draw_net), NULL);
    g_signal_connect(da, "configure-event",
                     G_CALLBACK(on_configure_event_net), NULL);

    g_timeout_add(50, (GSourceFunc)on_timeout_net, da);
}

static gboolean
on_configure_event_ram(
    GtkWidget *widget,
    GdkEventConfigure *event,
    gpointer data)
{
    static int save_w = 0, save_h = 0;

    if (save_w == event->width && save_h == event->height)
        return TRUE;

    save_w = event->width;
    save_h = event->height;

    if (surface_ram)
        cairo_surface_destroy(surface_ram);

    surface_ram = gdk_window_create_similar_surface(
        gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR,
        save_w, save_h);

    cairo_t *cr = cairo_create(surface_ram);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    cairo_destroy(cr);

    return TRUE;
}

static gboolean
on_draw_ram(
    GtkWidget *widget,
    cairo_t *cr,
    gpointer data)
{
    cairo_set_source_surface(cr, surface_ram, 0, 0);
    cairo_paint(cr);
    return TRUE;
}

gboolean
on_timeout_ram(
    gpointer data)
{

    GtkWidget *widget = GTK_WIDGET(data);

    cairo_t *cr = cairo_create(surface_ram);
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_paint(cr);

    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_line_width(cr, 6);
    cairo_translate(cr, (width / 2), (height / 2));
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    // cairo_save(cr);

    double angle1 = 0 * (M_PI / 180.0);                                // angles are specified
    double angle2 = 270.0 * (M_PI / 180.0);                            // in radians
    double angle3 = (0.000001 + (270 * ram_u / 100)) * (M_PI / 180.0); // in radians
    //printf("a0 %LG, a1 %LG\n", dyn_a[1].c[3], dyn_a[0].c[3]);
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.952941176, 0.952941176, 0.952941176);
    cairo_arc(cr, 0, 0, 55.0, angle1, angle2);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 1, 0.223529412, 0.592156863);
    cairo_arc(cr, 0, 0, 55.0, angle1, angle3);
    cairo_stroke(cr);

    cairo_destroy(cr);

    gtk_widget_queue_draw(GTK_WIDGET(data));
    return TRUE;
}

static void on_activate4(GtkWidget *window, GtkBuilder *builder)
{
    GtkWidget *da;
    da = GTK_WIDGET(gtk_builder_get_object(builder, "dwmarea_ram"));
    gtk_widget_set_size_request(da, 117, 117);

    g_signal_connect(da, "draw",
                     G_CALLBACK(on_draw_ram), NULL);
    g_signal_connect(da, "configure-event",
                     G_CALLBACK(on_configure_event_ram), NULL);

    g_timeout_add(50, (GSourceFunc)on_timeout_ram, da);
}

static gboolean
on_configure_event_core_graph(
    GtkWidget *widget,
    GdkEventConfigure *event,
    gpointer data)
{
    static int save_w = 0, save_h = 0;

    if (save_w == event->width && save_h == event->height)
        return TRUE;

    save_w = event->width;
    save_h = event->height;

    if (surface_core_graph)
        cairo_surface_destroy(surface_core_graph);

    surface_core_graph = gdk_window_create_similar_surface(
        gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR,
        save_w, save_h);

    cairo_t *cr = cairo_create(surface_core_graph);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    cairo_destroy(cr);

    return TRUE;
}

static gboolean
on_draw_core_graph(
    GtkWidget *widget,
    cairo_t *cr,
    gpointer data)
{
    cairo_set_source_surface(cr, surface_core_graph, 0, 0);
    cairo_paint(cr);
    return TRUE;
}

gboolean
on_timeout_core_graph(
    gpointer data)
{
    GtkWidget *widget = GTK_WIDGET(data);

    cairo_t *cr = cairo_create(surface_core_graph);
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_paint(cr);

    float width = gtk_widget_get_allocated_width(widget);
    float height = gtk_widget_get_allocated_height(widget);

    cairo_set_line_width(cr, 1);
    //printf("width: %lf\n",width);
    //cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    cairo_save(cr);

    int j;

    float box_dist = ((width) / 8);
    float box_lenght = ((width - 35) / 8);
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        cairo_set_source_rgb(cr, 0.152941176, 0.545098039, 0.807843137);
        cairo_rectangle(cr, 0.5 + (box_dist * i), 0.5, box_lenght - 0.5, 80);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.376470588, 0.749019608, 1, 0.2);
        for (j = 1; j <= 5; j++)
        {
            cairo_move_to(cr, ((box_lenght / 5) * j) + box_dist * i, 0.5);
            cairo_line_to(cr, ((box_lenght / 5) * j) + box_dist * i, 80.5);
        }
        for (j = 1; j <= 8; j++)
        {
            cairo_move_to(cr, 0.5 + box_dist * i, (10 * j) + 0.5);
            cairo_line_to(cr, box_lenght + 0.5 + box_dist * i, (10 * j) + 0.5);
        }
        cairo_stroke(cr);
    }

    for (i = 0; i < 8; i++)
    {
        cairo_set_source_rgb(cr, 0.152941176, 0.545098039, 0.807843137);
        cairo_rectangle(cr, 0.5 + box_dist * i, 85.5, box_lenght - 0.5, 80);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.376470588, 0.749019608, 1, 0.2);
        for (j = 1; j <= 5; j++)
        {
            cairo_move_to(cr, ((box_lenght / 5) * j) + 0.5 + box_dist * i, 0.5 + 85);
            cairo_line_to(cr, ((box_lenght / 5) * j) + 0.5 + box_dist * i, 80.5 + 85);
        }
        for (j = 1; j <= 8; j++)
        {
            cairo_move_to(cr, 0.5 + box_dist * i, (10 * j) + 0.5 + 85);
            cairo_line_to(cr, box_lenght + 0.5 + box_dist * i, (10 * j) + 0.5 + 85);
        }
        cairo_stroke(cr);
    }

    for (i = 0; i < 8; i++)
    {
        cairo_set_source_rgb(cr, 0.152941176, 0.545098039, 0.807843137);
        cairo_rectangle(cr, 0.5 + box_dist * i, 170.5, box_lenght - 0.5, 80);
        cairo_stroke(cr);
        cairo_set_source_rgba(cr, 0.376470588, 0.749019608, 1, 0.2);
        for (j = 1; j <= 5; j++)
        {
            cairo_move_to(cr, ((box_lenght / 5) * j) + 0.5 + box_dist * i, 0.5 + 170);
            cairo_line_to(cr, ((box_lenght / 5) * j) + 0.5 + box_dist * i, 80.5 + 170);
        }
        for (j = 1; j <= 8; j++)
        {
            cairo_move_to(cr, 0.5 + box_dist * i, (10 * j) + 0.5 + 170);
            cairo_line_to(cr, box_lenght + 0.5 + box_dist * i, (10 * j) + 0.5 + 170);
        }
        cairo_stroke(cr);
    }

    for (i = 0; i < 8; i++)
    {
        cairo_set_source_rgb(cr, 0.152941176, 0.545098039, 0.807843137);
        cairo_rectangle(cr, 0.5 + box_dist * i, 255.5, box_lenght - 0.5, 80);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.376470588, 0.749019608, 1, 0.2);
        for (j = 1; j <= 5; j++)
        {
            cairo_move_to(cr, ((box_lenght / 5) * j) + 0.5 + box_dist * i, 0.5 + 255);
            cairo_line_to(cr, ((box_lenght / 5) * j) + 0.5 + box_dist * i, 80.5 + 255);
        }
        for (j = 1; j <= 8; j++)
        {
            cairo_move_to(cr, 0.5 + box_dist * i, (10 * j) + 0.5 + 255);
            cairo_line_to(cr, box_lenght + 0.5 + box_dist * i, (10 * j) + 0.5 + 255);
        }
        cairo_stroke(cr);
    }

    cairo_set_source_rgb(cr, 0.176470588, 0.654901961, 0.97254902);
    int h = 0;
    cairo_translate(cr, 0.5, 80.5);
    /* Link each data point */
    i = 0;
    for (i = 0; i < 100; i += 1)
    {

        cairo_line_to(cr, i * (box_lenght) / 100, f(dyn_stored_load[1].c[i] * (80 - 0.5) / 400)); //i/exp(load/100)
    }

    /* Draw the curve */
    cairo_stroke(cr);

    //cairo_set_source_rgb(cr, 0.176470588, 0.654901961, 0.97254902);
    for (h = 2; h <= 8; h++)
    {
        cairo_translate(cr, box_dist, 0);
        i = 0;
        for (i = 0; i < 100; i += 1)
        {

            cairo_line_to(cr, i * (box_lenght) / 100, f(dyn_stored_load[h].c[i] * (80 - 0.5) / 400)); //i/exp(load/100)
        }

        /* Draw the curve */
        cairo_stroke(cr);
    }

    cairo_translate(cr, 1, 85);
    i = 0;
    for (i = 0; i < 100; i += 1)
    {

        cairo_line_to(cr, i * (box_lenght) / 100, f(dyn_stored_load[9].c[i] * (80 - 0.5) / 400)); //i/exp(load/100)
    }

    /* Draw the curve */
    cairo_stroke(cr);

    for (h = 10; h <= 16; h++)
    {
        cairo_translate(cr, -box_dist, 0);
        i = 0;
        for (i = 0; i < 100; i += 1)
        {

            cairo_line_to(cr, i * (box_lenght) / 100, f(dyn_stored_load[h].c[i] * (80 - 0.5) / 400)); //i/exp(load/100)
        }

        /* Draw the curve */
        cairo_stroke(cr);
    }

    cairo_translate(cr, 0.5, 85);
    i = 0;
    for (i = 0; i < 100; i += 1)
    {

        cairo_line_to(cr, i * (box_lenght) / 100, f(dyn_stored_load[17].c[i] * (80 - 0.5) / 400)); //i/exp(load/100)
    }

    /* Draw the curve */
    cairo_stroke(cr);

    for (h = 18; h <= 24; h++)
    {
        cairo_translate(cr, box_dist, 0);
        i = 0;
        for (i = 0; i < 100; i += 1)
        {

            cairo_line_to(cr, i * (box_lenght) / 100, f(dyn_stored_load[h].c[i] * (80 - 0.5) / 400)); //i/exp(load/100)
        }

        /* Draw the curve */
        cairo_stroke(cr);
    }

    cairo_translate(cr, 0.5, 85);
    i = 0;
    for (i = 0; i < 100; i += 1)
    {

        cairo_line_to(cr, i * (box_lenght) / 100, f(dyn_stored_load[17].c[i] * (80 - 0.5) / 400)); //i/exp(load/100)
    }

    /* Draw the curve */
    cairo_stroke(cr);

    for (h = 10; h <= 16; h++)
    {
        cairo_translate(cr, -box_dist, 0);
        i = 0;
        for (i = 0; i < 100; i += 1)
        {

            cairo_line_to(cr, i * (box_lenght) / 100, f(dyn_stored_load[h].c[i] * (80 - 0.5) / 400)); //i/exp(load/100)
        }

        /* Draw the curve */
        cairo_stroke(cr);
    }

    cairo_destroy(cr);

    gtk_widget_queue_draw(GTK_WIDGET(data));
    return TRUE;
}

static void on_activate3(GtkWidget *window, GtkBuilder *builder)
{
    GtkWidget *da;
    da = GTK_WIDGET(gtk_builder_get_object(builder, "dwmarea_cpu_cores"));
    //gtk_widget_set_size_request(da, 1040, 500);

    g_signal_connect(da, "draw",
                     G_CALLBACK(on_draw_core_graph), NULL);
    g_signal_connect(da, "configure-event",
                     G_CALLBACK(on_configure_event_core_graph), NULL);

    g_timeout_add(50, (GSourceFunc)on_timeout_core_graph, da);
}

static gboolean
on_configure_event(
    GtkWidget *widget,
    GdkEventConfigure *event,
    gpointer data)
{
    static int save_w = 0, save_h = 0;

    if (save_w == event->width && save_h == event->height)
        return TRUE;

    save_w = event->width;
    save_h = event->height;

    if (surface)
        cairo_surface_destroy(surface);

    surface = gdk_window_create_similar_surface(
        gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR,
        save_w, save_h);

    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    cairo_destroy(cr);

    return TRUE;
}

static gboolean
on_draw3(
    GtkWidget *widget,
    cairo_t *cr,
    gpointer data)
{
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);
    return TRUE;
}

static gboolean stop_timeout = FALSE;

gboolean
on_timeout(gpointer data)
{
    if (stop_timeout)
        return FALSE;

    GtkWidget *widget = GTK_WIDGET(data);
    static int ang = 0;

    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_paint(cr);

    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    int x = (int)(width / 2.0 * (cos(3.1416 / 180 * ang) + 1));
    int y = (int)(height / 2.0 * (sin(3.1416 / 180 * ang) + 1));
    ang = (ang + 2) % 360;

    cairo_set_line_width(cr, 7.5);
    cairo_translate(cr, (width / 2), (height / 2));
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    // cairo_save(cr);

    double angle1 = 120 * (M_PI / 180.0);                                     // angles are specified
    double angle2 = 60.0 * (M_PI / 180.0);                                    // in radians
    double angle3 = (120.000001 + (300 * loadavg[0] / 100)) * (M_PI / 180.0); // in radians
    //printf("a0 %LG, a1 %LG\n", dyn_a[1].c[3], dyn_a[0].c[3]);
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.952941176, 0.952941176, 0.952941176);
    cairo_arc(cr, 0, 0, 82.0, angle1, angle2);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0.176470588, 0.654901961, 0.97254902);
    cairo_arc(cr, 0, 0, 82.0, angle1, angle3);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0.176470588, 0.654901961, 0.97254902);
    cairo_translate(cr, 37 - (width / 2), -10);
    //cairo_scale (cr, ZOOM_X, -ZOOM_Y);

    /* Determine the data points to calculate (ie. those in the clipping zone */
    cairo_set_line_width(cr, 2);

    /* Link each data point */
    int i;

    /*for(i = 52; i<100;i++){
        beat_stored_load[i]= 10;
    }*/

    for (i = 0; i < 100; i += 1)
    {
        //ls = ;
        cairo_line_to(cr, i, f(dyn_stored_load[0].c[i])); //i/exp(load/100)
    }
    /* Draw the curve */

    cairo_stroke(cr);

    //cairo_move_to(cr, width / 2, height / 2);
    //cairo_line_to(cr, x, y);

    //cairo_stroke(cr);
    cairo_destroy(cr);

    gtk_widget_queue_draw(GTK_WIDGET(data));
    return TRUE;
}

static void
on_destroy(
    void)
{
    if (surface)
        cairo_surface_destroy(surface);
}

static void
button_clicked(
    GtkWidget *button,
    gpointer data)
{
    gtk_button_set_label(GTK_BUTTON(button),
                         stop_timeout ? "Stop" : "Start");
    stop_timeout = !stop_timeout;
    //if (!stop_timeout)
    //    g_timeout_add(50, (GSourceFunc)on_timeout, GTK_WINDOW(data));
}

static void on_activate2(GtkWidget *window, GtkBuilder *builder)
{

    //GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    //gtk_container_add(GTK_CONTAINER(window), box);

    //GtkWidget *button;
    // = gtk_button_new_with_label("Stop");
    //button = GTK_WIDGET(gtk_builder_get_object(builder, "ssbut"));

    //g_signal_connect(GTK_BUTTON(button), "clicked",
    //                 G_CALLBACK(button_clicked), window);
    //gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
    //GtkWidget *da;
    GtkWidget *da;
    da = GTK_WIDGET(gtk_builder_get_object(builder, "dwmarea2"));
    gtk_widget_set_size_request(da, 179, 179);

    g_signal_connect(da, "draw",
                     G_CALLBACK(on_draw3), NULL);
    g_signal_connect(da, "configure-event",
                     G_CALLBACK(on_configure_event), NULL);

    g_timeout_add(50, (GSourceFunc)on_timeout, da);
}

#define WIDTH 640
#define HEIGHT 480

#define ZOOM_X 100.0
#define ZOOM_Y 100.0

typedef struct
{
    GtkWidget *w_lbl_time;
} app_widgets;

void initial_fast_update()
{
    fp = fopen("/proc/stat", "r");

    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &dyn_a[0].c[0], &dyn_a[0].c[1], &dyn_a[0].c[2], &dyn_a[0].c[3]);

    int i = 0;
    char line[200] = {0};

    while (fgets(line, 200, fp) != NULL)
    {
        if (i == 0)
        {
        }
        else
        {
            sscanf(line, "cpu%d %LF %LF %LF %LF", &cpuid[i], &dyn_a[i].c[0], &dyn_a[i].c[1], &dyn_a[i].c[2], &dyn_a[i].c[3]);
            //printf("%s",line);
        }
        if (g_cpu_count - 1 == i)
        {
            break;
        }
        i++;
    }
    fclose(fp);
    netinfo_fast_u = fopen("/proc/net/dev", "r");
    //do anything here fix everything fix network icon
    char mem_stats[200];
    initial_inbound_temp_fast_u = 0;
    initial_inbound_total_fast_u = 0;
    initial_outbound_temp_fast_u = 0;
    initial_outbound_total_fast_u = 0;
    i = 0;
    while (fgets(mem_stats, 200, netinfo_fast_u) != NULL)
    {
        if (i == 2)
        {
            sscanf(mem_stats, "%s %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF", &dispose_string[0].c, &initial_inbound_temp_fast_u, &dispose_num[0].c[0], &dispose_num[0].c[1], &dispose_num[0].c[2],
                   &dispose_num[0].c[3], &dispose_num[0].c[4], &dispose_num[0].c[5], &dispose_num[0].c[6], &initial_outbound_temp_fast_u,
                   &dispose_num[0].c[7], &dispose_num[0].c[8], &dispose_num[0].c[9], &dispose_num[0].c[10], &dispose_num[0].c[11], &dispose_num[0].c[12], &dispose_num[0].c[13]);
            initial_inbound_total_fast_u += initial_inbound_temp_fast_u;
            initial_outbound_total_fast_u += initial_outbound_temp_fast_u;
        }
        else
        {
            i++;
        }
    }
    //printf("%LG percent used\n",initial_outbound_total);

    fclose(netinfo_fast_u);

    fp = fopen("/proc/diskstats", "r");
    //int flags;
    mem_stats[200] = 0;
    initial_disk_read_total = 0;
    initial_disk_read_temp = 0;
    initial_disk_write_total = 0;
    initial_disk_write_temp = 0;
    i = 0;
    while (fgets(mem_stats, 200, fp) != NULL)
    {
        physical_drive[100] = 0;
        sscanf(mem_stats, "%Lf %Lf %s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf", &dispose_num[0].c[0], &dispose_num[0].c[2], &physical_drive,
               &dispose_num[0].c[3], &dispose_num[0].c[4], &initial_disk_read_temp, &dispose_num[0].c[5], &dispose_num[0].c[6], &dispose_num[0].c[7], &initial_disk_write_temp,
               &dispose_num[0].c[8], &dispose_num[0].c[9], &dispose_num[0].c[10], &dispose_num[0].c[11]);
        i = 0;
        int flag = 0;

        for (i = 0; physical_drive[i] != '\0'; i++)
        {
            if (physical_drive[i] == '0' || physical_drive[i] == '1' || physical_drive[i] == '2' || physical_drive[i] == '3' || physical_drive[i] == '4' || physical_drive[i] == '5' || physical_drive[i] == '6' || physical_drive[i] == '7' || physical_drive[i] == '8' || physical_drive[i] == '9')
            {
                flag = flag + 1;
            }
        }

        if (flag == 0)
        {

            initial_disk_read_total += initial_disk_read_temp;
            initial_disk_write_total += initial_disk_write_temp;
        }
    }
    //printf("Drive name %LG\n",final_disk_read_total);

    fclose(fp);
}

void final_fast_update()
{
    //static unsigned int count = 0;
    char str_count[30] = {0};

    //count++;
    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &dyn_b[0].c[0], &dyn_b[0].c[1], &dyn_b[0].c[2], &dyn_b[0].c[3]);

    int i = 0;
    char line[200] = {0};
    while (fgets(line, 200, fp) != NULL)
    {
        if (i == 0)
        {
        }
        else
        {
            sscanf(line, "cpu%d %LF %LF %LF %LF", &cpuid_b[i], &dyn_b[i].c[0], &dyn_b[i].c[1], &dyn_b[i].c[2], &dyn_b[i].c[3]);
            //printf("%LG\n",dyn_b[i].c[0]);
        }
        if (g_cpu_count - 1 == i)
        {
            break;
        }
        i++;
    }
    fclose(fp);
    i = 0;
    //fast_onceloadavg[0] = (((dyn_b[0].c[0] + dyn_b[0].c[1] + dyn_b[0].c[2]) - (dyn_a[0].c[0] + dyn_a[0].c[1] + dyn_a[0].c[2])) / ((dyn_b[0].c[0] + dyn_b[0].c[1] + dyn_b[0].c[2] + dyn_b[0].c[3]) - (dyn_a[0].c[0] + dyn_a[0].c[1] + dyn_a[0].c[2] + dyn_a[0].c[3]))) * 100;
    for (i = 0; i <= (g_cpu_count - 1); i++)
    {
        fast_onceloadavg[i] = (((dyn_b[i].c[0] + dyn_b[i].c[1] + dyn_b[i].c[2]) - (dyn_a[i].c[0] + dyn_a[i].c[1] + dyn_a[i].c[2])) / ((dyn_b[i].c[0] + dyn_b[i].c[1] + dyn_b[i].c[2] + dyn_b[i].c[3]) - (dyn_a[i].c[0] + dyn_a[i].c[1] + dyn_a[i].c[2] + dyn_a[i].c[3]))) * 100;
        if (isnan(fast_onceloadavg[i]))
        {
            fast_loadavg[i] = 0;
        }
        else
        {
            fast_loadavg[i] = fast_onceloadavg[i];
        }
        int j = 0;
        for (j = 0; j < 100; j++)
        {
            dyn_stored_load[i].c[j] = dyn_stored_load[i].c[j + 1];
        }
        if (i == 0)
        {
            double dm = ((fast_loadavg[0] * x) / M_PI) + M_PI;
            dyn_stored_load[i].c[99] = 4 * sqrt((fast_loadavg[0]) / 80) * ((sin(2 * dm) + sin(3 * dm)));
        }
        else
        {
            dyn_stored_load[i].c[99] = fast_loadavg[i];
        }
    }

    FILE *meminfo = fopen("/proc/meminfo", "rb");
    //do anything here fix everything
    line[200] = NULL;

    while (fgets(line, 200, meminfo) != NULL)
    {
        if (strstr(line, "MemAvailable") != NULL)
        {
            sscanf(line, "%s %LF %s", &dispose_string[0].c, &memfree, &dispose_string[1].c);
        }
        if (strstr(line, "SwapFree") != NULL)
        {
            sscanf(line, "%s %LF %s", &dispose_string[0].c, &swapfree, &dispose_string[1].c);
            break;
        }
    }
    fclose(meminfo);
    //printf("%LG percent used\n",);

    //printf("%G\n",dyn_stored_load[32].c[99]);
    //update to fast form /slow update overwrite the information before being read
    netinfo_fast_u = fopen("/proc/net/dev", "r");
    //do anything here fix everything fix network icon
    char mem_stats[200];
    final_inbound_temp_fast_u = 0;
    final_inbound_total_fast_u = 0;
    final_outbound_temp_fast_u = 0;
    final_outbound_total_fast_u = 0;
    i = 0;
    while (fgets(mem_stats, 200, netinfo_fast_u) != NULL)
    {
        if (i == 2)
        {
            sscanf(mem_stats, "%s %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF", &dispose_string[0].c, &final_inbound_temp_fast_u, &dispose_num[0].c[0], &dispose_num[0].c[1], &dispose_num[0].c[2],
                   &dispose_num[0].c[3], &dispose_num[0].c[4], &dispose_num[0].c[5], &dispose_num[0].c[6], &final_outbound_temp_fast_u,
                   &dispose_num[0].c[7], &dispose_num[0].c[8], &dispose_num[0].c[9], &dispose_num[0].c[10], &dispose_num[0].c[11], &dispose_num[0].c[12], &dispose_num[0].c[13]);
            final_inbound_total_fast_u += final_inbound_temp_fast_u;
            final_outbound_total_fast_u += final_outbound_temp_fast_u;
        }
        else
        {
            i++;
        }
    }

    fclose(netinfo_fast_u);
    float net_load[2] = {0};
    net_load[0] = (float)((final_inbound_total_fast_u - initial_inbound_total_fast_u) / (1024 * 1024 * 0.05)); //MB/s
    net_load[1] = (float)((final_outbound_total_fast_u - initial_outbound_total_fast_u) / (1024 * 1024 * 0.05));
    if (isnan(net_load[0]))
    {
        net_load[0] = 0;
    }
    else
    {
        net_load[0] = net_load[0];
    }
    if (isnan(net_load[1]))
    {
        net_load[1] = 0;
    }
    else
    {
        net_load[1] = net_load[1];
    }
    int j = 0;
    for (j = 0; j < 99; j++)
    {
        dyn_stored_net_load[0].c[j] = dyn_stored_net_load[0].c[j + 1];
        dyn_stored_net_load[1].c[j] = dyn_stored_net_load[1].c[j + 1];
    }
    dyn_stored_net_load[0].c[99] = net_load[0];
    dyn_stored_net_load[1].c[99] = net_load[1];
    //printf("final %lf, initial %lf\n",(float)final_inbound_total,(float)initial_inbound_total);

    str_count[30] = 0;

    //count++;

    //read disks stats
    fp = fopen("/proc/diskstats", "r");
    //int flags;
    mem_stats[200] = 0;
    final_disk_read_total = 0;
    final_disk_read_temp = 0;
    final_disk_write_total = 0;
    final_disk_write_temp = 0;
    i = 0;
    while (fgets(mem_stats, 200, fp) != NULL)
    {
        physical_drive[100] = 0;
        sscanf(mem_stats, "%Lf %Lf %s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf", &dispose_num[0].c[0], &dispose_num[0].c[2], &physical_drive,
               &dispose_num[0].c[3], &dispose_num[0].c[4], &final_disk_read_temp, &dispose_num[0].c[5], &dispose_num[0].c[6], &dispose_num[0].c[7], &final_disk_write_temp,
               &dispose_num[0].c[8], &dispose_num[0].c[9], &dispose_num[0].c[10], &dispose_num[0].c[11]);
        i = 0;
        int flag = 0;

        for (i = 0; physical_drive[i] != '\0'; i++)
        {
            if (physical_drive[i] == '0' || physical_drive[i] == '1' || physical_drive[i] == '2' || physical_drive[i] == '3' || physical_drive[i] == '4' || physical_drive[i] == '5' || physical_drive[i] == '6' || physical_drive[i] == '7' || physical_drive[i] == '8' || physical_drive[i] == '9')
            {
                flag = flag + 1;
            }
        }

        if (flag == 0)
        {

            final_disk_read_total += final_disk_read_temp;
            final_disk_write_total += final_disk_write_temp;
        }
    }
    float disk_load[2] = {0};
    disk_load[0] = (float)((final_disk_read_total - initial_disk_read_total) / (1024 * 1024 * 2 * 0.05)); // read MB/s
    disk_load[1] = (float)((final_disk_write_total - initial_disk_write_total) / (1024 * 1024 * 2 * 0.05)); // write MB/s
    //printf("Drive name %LG\n",final_disk_read_total);
    if (isnan(disk_load[0]))
    {
        disk_load[0] = 0;
    }
    else
    {
        disk_load[0] = disk_load[0];
    }
    if (isnan(disk_load[1]))
    {
        disk_load[1] = 0;
    }
    else
    {
        disk_load[1] = disk_load[1];
    }
    j = 0;
    for (j = 0; j < 99; j++)
    {
        dyn_stored_disk_load[0].c[j] = dyn_stored_disk_load[0].c[j + 1];
        dyn_stored_disk_load[1].c[j] = dyn_stored_disk_load[1].c[j + 1];
    }
    dyn_stored_disk_load[0].c[99] = disk_load[0];
    dyn_stored_disk_load[1].c[99] = disk_load[1];
    fclose(fp);

    x++;
    if (x == 400)
    {
        x = 0;
    }
    initial_fast_update();
}

gboolean fast_update()
{

    final_fast_update();
    return TRUE;
}

void initial_slow_update()
{
    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0][0], &a[0][1], &a[0][2], &a[0][3]);
    fclose(fp);

    netinfo = fopen("/proc/net/dev", "r");
    //do anything here fix everything fix network icon
    char mem_stats[200];
    initial_inbound_temp = 0;
    initial_inbound_total = 0;
    initial_outbound_temp = 0;
    initial_outbound_total = 0;
    int i = 0;
    while (fgets(mem_stats, 200, netinfo) != NULL)
    {
        if (i == 2)
        {
            sscanf(mem_stats, "%s %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF", &dispose_string[0].c, &initial_inbound_temp, &dispose_num[0].c[0], &dispose_num[0].c[1], &dispose_num[0].c[2],
                   &dispose_num[0].c[3], &dispose_num[0].c[4], &dispose_num[0].c[5], &dispose_num[0].c[6], &initial_outbound_temp,
                   &dispose_num[0].c[7], &dispose_num[0].c[8], &dispose_num[0].c[9], &dispose_num[0].c[10], &dispose_num[0].c[11], &dispose_num[0].c[12], &dispose_num[0].c[13]);
            initial_inbound_total += initial_inbound_temp;
            initial_outbound_total += initial_outbound_temp;
        }
        else
        {
            i++;
        }
    }
    //printf("%LG percent used\n",initial_outbound_total);

    fclose(netinfo);


    fp = fopen("/proc/diskstats", "r");
    //int flags;
    mem_stats[200] = 0;
    initial_disk_read_total_slow_u = 0;
    initial_disk_read_temp_slow_u = 0;
    initial_disk_write_total_slow_u = 0;
    initial_disk_write_temp_slow_u = 0;
    i = 0;
    while (fgets(mem_stats, 200, fp) != NULL)
    {
        physical_drive_slow_u[100] = 0;
        sscanf(mem_stats, "%Lf %Lf %s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf", &dispose_num[0].c[0], &dispose_num[0].c[2], &physical_drive_slow_u,
               &dispose_num[0].c[3], &dispose_num[0].c[4], &initial_disk_read_temp_slow_u, &dispose_num[0].c[5], &dispose_num[0].c[6], &dispose_num[0].c[7], &initial_disk_write_temp_slow_u,
               &dispose_num[0].c[8], &dispose_num[0].c[9], &dispose_num[0].c[10], &dispose_num[0].c[11]);
        i = 0;
        int flag = 0;

        for (i = 0; physical_drive_slow_u[i] != '\0'; i++)
        {
            if (physical_drive_slow_u[i] == '0' || physical_drive_slow_u[i] == '1' || physical_drive_slow_u[i] == '2' || physical_drive_slow_u[i] == '3' || physical_drive_slow_u[i] == '4' || physical_drive_slow_u[i] == '5' || physical_drive_slow_u[i] == '6' || physical_drive_slow_u[i] == '7' || physical_drive_slow_u[i] == '8' || physical_drive_slow_u[i] == '9')
            {
                flag = flag + 1;
            }
        }

        if (flag == 0)
        {

            initial_disk_read_total_slow_u += initial_disk_read_temp_slow_u;
            initial_disk_write_total_slow_u += initial_disk_write_temp_slow_u;
        }
    }
    //printf("Drive name %LG\n",final_disk_read_total);

    fclose(fp);
}

void final_slow_update()
{
    char str_count[30] = {0};
    char str_count2[30] = {0};
    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0][0], &b[0][1], &b[0][2], &b[0][3]);
    fclose(fp);
    onceloadavg[0] = (((b[0][0] + b[0][1] + b[0][2]) - (a[0][0] + a[0][1] + a[0][2])) / ((b[0][0] + b[0][1] + b[0][2] + b[0][3]) - (a[0][0] + a[0][1] + a[0][2] + a[0][3]))) * 100;
    if (isnan(onceloadavg[0]))
    {
        loadavg[0] = 0;
    }
    else
    {
        loadavg[0] = onceloadavg[0];
    }
    sprintf(str_count, "%.1f", loadavg[0]);
    gtk_misc_set_alignment(GTK_MISC(g_lbl_count), 1, 0);
    gtk_label_set_text(GTK_LABEL(g_lbl_count), strcat(str_count, "%"));
    ram_u = (memtotal - memfree) * 100 / memtotal;

    sprintf(str_count, "(%0.1f", (float)ram_u);
    gtk_label_set_text(GTK_LABEL(g_id_ram_used), strcat(str_count, "%)"));

    sprintf(str_count2, "%0.1f", (float)ram_u);
    gtk_label_set_text(GTK_LABEL(g_id_ram_used2), strcat(str_count2, "%"));

    sprintf(str_count2, "%0.1f GB/%0.1f GB", (float)((memtotal - memfree) / (1024 * 1024)), (float)(memtotal / (1024 * 1024)));
    gtk_label_set_text(GTK_LABEL(g_id_ram_used3), str_count2);

    swap_u = (swaptotal - swapfree) * 100 / swaptotal;
    sprintf(str_count, "(%0.1f", (float)swap_u);
    gtk_label_set_text(GTK_LABEL(g_id_swap_used), strcat(str_count, "%)"));

    sprintf(str_count2, "%0.1f GB/%0.1f GB", (float)((swaptotal - swapfree) / (1024 * 1024)), (float)(swaptotal / (1024 * 1024)));
    gtk_label_set_text(GTK_LABEL(g_id_swap_used1), str_count2);
    // to fix for final inbout outbount
    netinfo = fopen("/proc/net/dev", "r");
    //do anything here fix everything fix network icon
    char mem_stats[200];
    final_inbound_temp = 0;
    final_inbound_total = 0;
    final_outbound_temp = 0;
    final_outbound_total = 0;
    int i = 0;
    while (fgets(mem_stats, 200, netinfo) != NULL)
    {
        if (i == 2)
        {
            sscanf(mem_stats, "%s %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF %LF", &dispose_string[0].c, &final_inbound_temp, &dispose_num[0].c[0], &dispose_num[0].c[1], &dispose_num[0].c[2],
                   &dispose_num[0].c[3], &dispose_num[0].c[4], &dispose_num[0].c[5], &dispose_num[0].c[6], &final_outbound_temp,
                   &dispose_num[0].c[7], &dispose_num[0].c[8], &dispose_num[0].c[9], &dispose_num[0].c[10], &dispose_num[0].c[11], &dispose_num[0].c[12], &dispose_num[0].c[13]);
            final_inbound_total += final_inbound_temp;
            final_outbound_total += final_outbound_temp;
        }
        else
        {
            i++;
        }
    }

    fclose(netinfo);
    sprintf(str_count, "%0.2f", (float)((final_inbound_total - initial_inbound_total) / (1024 * 1)));
    gtk_label_set_text(GTK_LABEL(g_id_network_in), strcat(str_count, " kByte/s"));

    sprintf(str_count, "%0.2f", (float)((final_outbound_total - initial_outbound_total) / (1024 * 1)));
    gtk_label_set_text(GTK_LABEL(g_id_network_out), strcat(str_count, " kByte/s"));





    fp = fopen("/proc/diskstats", "r");
    //int flags;
    mem_stats[200] = 0;
    final_disk_read_total_slow_u = 0;
    final_disk_read_temp_slow_u = 0;
    final_disk_write_total_slow_u = 0;
    final_disk_write_temp_slow_u = 0;
    i = 0;
    while (fgets(mem_stats, 200, fp) != NULL)
    {
        physical_drive_slow_u[100] = 0;
        sscanf(mem_stats, "%Lf %Lf %s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf", &dispose_num[0].c[0], &dispose_num[0].c[2], &physical_drive_slow_u,
               &dispose_num[0].c[3], &dispose_num[0].c[4], &final_disk_read_temp_slow_u, &dispose_num[0].c[5], &dispose_num[0].c[6], &dispose_num[0].c[7], &final_disk_write_temp_slow_u,
               &dispose_num[0].c[8], &dispose_num[0].c[9], &dispose_num[0].c[10], &dispose_num[0].c[11]);
        i = 0;
        int flag = 0;

        for (i = 0; physical_drive_slow_u[i] != '\0'; i++)
        {
            if (physical_drive_slow_u[i] == '0' || physical_drive_slow_u[i] == '1' || physical_drive_slow_u[i] == '2' || physical_drive_slow_u[i] == '3' || physical_drive_slow_u[i] == '4' || physical_drive_slow_u[i] == '5' || physical_drive_slow_u[i] == '6' || physical_drive_slow_u[i] == '7' || physical_drive_slow_u[i] == '8' || physical_drive_slow_u[i] == '9')
            {
                flag = flag + 1;
            }
        }

        if (flag == 0)
        {

            final_disk_read_total_slow_u += final_disk_read_temp_slow_u;
            final_disk_write_total_slow_u += final_disk_write_temp_slow_u;
        }
    }
    float disk_load_slow_u[2] = {0};
    disk_load_slow_u[0] = (float)((final_disk_read_total_slow_u - initial_disk_read_total_slow_u) / (1024 * 2)); // read MB/s
    disk_load_slow_u[1] = (float)((final_disk_write_total_slow_u - initial_disk_write_total_slow_u) / (1024 * 2)); // write MB/s
    //printf("Drive name %LG\n",final_disk_read_total);
    if (isnan(disk_load_slow_u[0]))
    {
        disk_load_slow_u[0] = 0;
    }
    else
    {
        disk_load_slow_u[0] = disk_load_slow_u[0];
    }
    if (isnan(disk_load_slow_u[1]))
    {
        disk_load_slow_u[1] = 0;
    }
    else
    {
        disk_load_slow_u[1] = disk_load_slow_u[1];
    }
    fclose(fp);
















    sprintf(str_count, "%0.2f", (float)( disk_load_slow_u[0]));
    gtk_label_set_text(GTK_LABEL(g_id_disk_read), strcat(str_count, " MB/s"));

    sprintf(str_count, "%0.2f", (float)( disk_load_slow_u[1]));
    gtk_label_set_text(GTK_LABEL(g_id_disk_write), strcat(str_count, " MB/s"));


    initial_slow_update();
}

//gboolean timer_handler(app_widgets *widgets);
gboolean slow_update()
{
    final_slow_update();
    return TRUE;
}

//declare the size of arrays after counting the number of cpu
void allocating_array(int cpu_count)
{
    //net load per network adapter next time
    loadavg = (double *)malloc(cpu_count * sizeof(double));
    onceloadavg = (double *)malloc(cpu_count * sizeof(double));
    fast_loadavg = (double *)malloc(cpu_count * sizeof(double));
    fast_onceloadavg = (double *)malloc(cpu_count * sizeof(double));
    dyn_stored_load = (double *)malloc(100 * cpu_count * sizeof(double));
    dyn_stored_net_load = (double *)malloc(2 * 100 * sizeof(double));
    dyn_stored_disk_load = (double *)malloc(2 * 100 * sizeof(double));
    dispose_num = (int *)malloc(30 * sizeof(int));

    dyn_a = (long double *)malloc(4 * cpu_count * sizeof(long double));
    dyn_b = (long double *)malloc(4 * cpu_count * sizeof(long double));
    //dyn_dwm = (GtkWidget *)malloc(10 * sizeof(GtkWidget));

    //allocate values to zero
    int i = 0;
    for (i; i < cpu_count; i++)
    {
        loadavg[i] = 0;
        onceloadavg[i] = 0;
        fast_loadavg[i] = 0;
        fast_onceloadavg[i] = 0;
        int j = 0;
        for (j; j < 100; j++)
        {
            dyn_stored_load[i].c[j] = 0;
            dyn_stored_net_load[0].c[j] = 0;
            dyn_stored_net_load[1].c[j] = 0;
            dyn_stored_disk_load[0].c[j] = 0;
            dyn_stored_disk_load[1].c[j] = 0;
        }
    }
    //printf("%d\n",cpu_count);
}

//get cpu information
void computer_info()
{
    dispose_string = (char *)malloc(10 * 100 * sizeof(char));
    int cpu_count = 0;
    char str_count[30] = {0};
    int i;

    char *arg = 0;
    char *var;
    size_t size = 0;

    char line[200];
    char *args;

    FILE *cpuinfo = fopen("/proc/cpuinfo", "rb");
    while (fgets(line, 200, cpuinfo) != NULL)
    {
        if (strstr(line, "model name") != NULL)
        {
            args = strchr(line, ':');
            if (args != NULL)
            {
                args++; /* we want to look at what's _after_ the '=' */
                //printf("%s", args);
            }
            else
            {
                //printf("cpu name", args);
            }

            break;
        }
    }

    fclose(cpuinfo);
    sprintf(str_count, "%s", args);
    //fix this
    gtk_label_set_text(GTK_LABEL(g_id_cpu_name), str_count);

    fp = fopen("/proc/stat", "rb");
    char line_stats[200];
    while (fgets(line_stats, 200, fp) != NULL)
    {
        if (strstr(line_stats, "cpu") != NULL)
        {
            cpu_count++;
        }
        else
        {
            break;
        }
    }
    g_cpu_count = cpu_count;
    fclose(fp);
    int number;
    double startf;

    //printf("Enter a positive integer: ");
    //scanf("%d",&number);

    //printf("Factors of %d are: ", number);
    number = cpu_count;
    startf = sqrt(number);
    int starti = (int)startf;
    if (startf == starti)
    {
        printf("1 First factor is %d\n", starti);
        return 0;
    }
    for (i = starti; i <= number; i++)
    {
        if (number % i == 0)
        {
            factorable_cpu_count[0] = i;
            factorable_cpu_count[1] = number / i;
            //printf("First factor is %d\n",);
            //printf("Second factor is %d\n",number/i);
            break;
        }
    }

    FILE *meminfo = fopen("/proc/meminfo", "rb");
    //do anything here fix everything fix network icon
    char mem_stats[200];

    while (fgets(mem_stats, 200, meminfo) != NULL)
    {
        if (strstr(mem_stats, "MemTotal") != NULL)
        {
            sscanf(mem_stats, "%s %LF %s", &dispose_string[0].c, &memtotal, &dispose_string[1].c);
        }
        if (strstr(mem_stats, "SwapTotal") != NULL)
        {
            sscanf(mem_stats, "%s %LF %s", &dispose_string[0].c, &swaptotal, &dispose_string[1].c);
            break;
        }
    }
    //printf("%LG percent used\n",);

    fclose(meminfo);
    allocating_array(cpu_count);
}

static void
on_activate()
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *dbb;
    GdkColor color;

    color.red = 0xffff;
    color.green = 0xffff;
    color.blue = 0xffff;
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "window_main.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));

    int i;
    for (i = 0; i < 100; i++)
    {
        //beat_stored_load[i] = 0;
    }
    //dbb = GTK_WIDGET(gtk_builder_get_object(builder, "idheadbox"));
    //gtk_widget_modify_bg(dbb, GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);

    g_lbl_count = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_count"));
    g_id_cpu_name = GTK_WIDGET(gtk_builder_get_object(builder, "id_cpu_name"));
    g_id_ram_used = GTK_WIDGET(gtk_builder_get_object(builder, "id_ram_used"));
    g_id_ram_used2 = GTK_WIDGET(gtk_builder_get_object(builder, "id_ram_used2"));
    g_id_ram_used3 = GTK_WIDGET(gtk_builder_get_object(builder, "id_ram_used3"));
    g_id_swap_used = GTK_WIDGET(gtk_builder_get_object(builder, "id_swap_used"));
    g_id_swap_used1 = GTK_WIDGET(gtk_builder_get_object(builder, "id_swap_used1"));
    g_id_network_out = GTK_WIDGET(gtk_builder_get_object(builder, "id_network_outbound"));
    g_id_network_in = GTK_WIDGET(gtk_builder_get_object(builder, "id_network_inbound"));
    g_id_disk_write = GTK_WIDGET(gtk_builder_get_object(builder, "id_disk_write"));
    g_id_disk_read = GTK_WIDGET(gtk_builder_get_object(builder, "id_disk_read"));
    computer_info();

    GtkWidget *da;

    on_activate2(window, builder);
    on_activate3(window, builder);
    on_activate4(window, builder);
    on_activate5(window, builder);
    on_activate6(window, builder);
    g_timeout_add(50, (GSourceFunc)fast_update, NULL);
    gtk_builder_connect_signals(builder, NULL);

    initial_slow_update();
    final_slow_update();

    g_timeout_add_seconds(1, (GSourceFunc)slow_update, NULL);
    gtk_window_set_default_size(GTK_WINDOW(window), 570, 300);
    //gtk_window_set_resizable(GTK_WINDOW(window),true);
    g_object_unref(builder);

    gtk_widget_show(window);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    on_activate();
    gtk_main();
    return 0;
}

void on_window_main_destroy()
{
    gtk_main_quit();
}
