#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include "DaemonApi.h"

static GtkWidget* s_draw_win = NULL;
static GtkWidget* s_text = NULL;

static void start_stream_callback()
{
    GdkScreen* screen = gdk_screen_get_default();
    unsigned int screen_w = gdk_screen_get_width(screen);
    unsigned int screen_h = gdk_screen_get_height(screen);
    gtk_widget_set_size_request(s_draw_win, screen_w, screen_h);
    gtk_window_set_position (GTK_WINDOW (s_draw_win), GTK_WIN_POS_CENTER);
    gtk_widget_show(s_draw_win);
    daemon_show_stream(s_draw_win);
}

static void stop_stream_callback()
{
    printf("stop_stream_callback\n");
    gtk_widget_hide(s_draw_win);
}

static void connect_server(GtkWidget *wid, GtkWidget *win)
{
    GtkTextIter start,end;
    gchar *text_str;

    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(s_text));
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);
    text_str = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),&start, &end, FALSE);
    if (daemon_start(text_str)) {
		daemon_set_start_stream_callback(start_stream_callback);
		daemon_set_stop_stream_callback(stop_stream_callback);
        gtk_widget_set_sensitive(wid, FALSE);
        printf("connect succeed\n");
    }
}

static void start_stream (GtkWidget *wid, GtkWidget *win)
{
	daemon_start_stream();
}

static void stop_stream (GtkWidget *wid, GtkWidget *win)
{
	daemon_stop_stream();
}

int main (int argc, char *argv[])
{
  GtkWidget *button = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *win = NULL;

  XInitThreads();
  /* Initialize GTK+ */
  g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
  gtk_init (&argc, &argv);
  g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

  /* Create the main window */
  win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (win), 8);
  gtk_window_set_title (GTK_WINDOW (win), "GTKClient");
  gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_set_default_size(GTK_WINDOW (win), 150, 200);
  gtk_widget_realize (win);
  g_signal_connect (win, "destroy", gtk_main_quit, NULL);

  /* Create a vertical box with buttons */
  vbox = gtk_vbox_new (TRUE, 6);
  gtk_container_add (GTK_CONTAINER (win), vbox);

  s_text = gtk_text_view_new();
  gtk_box_pack_start (GTK_BOX (vbox), s_text, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_CONNECT);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (connect_server), (gpointer) win);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_DIALOG_INFO);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (start_stream), (gpointer) win);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (stop_stream), (gpointer) win);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  /* Enter the main loop */
  gtk_widget_show_all (win);

  s_draw_win = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_main ();
  return 0;
}
