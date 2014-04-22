/*
 * Copyright (C) 2013  Roel Janssen <roel@moefel.org>
 *
 * This file is part of InklingReader
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "mainwindow_sig.h"
#include "../datatypes/configuration.h"
#include <gtk/gtk.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

GtkWidget* document_view = NULL;
GtkWidget* hbox_colors = NULL;
GSList* documents = NULL;

extern dt_configuration settings;

#define MENU_ITEMS_NUM 3
const char* menu_items[] = { "Open file", "Export file", "Quit" };
//const char* menu_items[] = { "Open file", "Open directory", "Export file", "Quit" };

void
gui_init_mainwindow (int argc, char** argv, const char* filename)
{
  /*--------------------------------------------------------------------------.
   | WIDGETS                                                                  |
   '--------------------------------------------------------------------------*/
  GtkWidget* window = NULL;
  GtkWidget* document_viewport = NULL;
  GtkWidget* document_container = NULL;

  GtkWidget* vbox_window = NULL;
  GtkWidget* hbox_menu_top = NULL;

  GtkWidget* menu_bar = NULL;
  GtkWidget* menu_bar_file = NULL;
  GtkWidget* menu_file = NULL;

  GtkWidget* new_color_button = NULL;
  GtkWidget* bg_color_button = NULL;
  GtkWidget* bg_color_label = NULL;
  GtkWidget* fg_color_label = NULL;
  GtkWidget* pressure_label = NULL;
  GtkWidget* pressure_input = NULL;

  /*--------------------------------------------------------------------------.
   | INIT AND CREATION OF WIDGETS                                             |
   '--------------------------------------------------------------------------*/
  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  vbox_window = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  hbox_menu_top = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  hbox_colors = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  document_container = gtk_scrolled_window_new (NULL, NULL);
  document_viewport = gtk_viewport_new (NULL, NULL);
  document_view = gtk_drawing_area_new ();

  menu_bar = gtk_menu_bar_new ();
  menu_bar_file = gtk_menu_item_new_with_label ("File");
  menu_file = gtk_menu_new ();

  new_color_button = gtk_button_new_with_label ("+");

  pressure_label = gtk_label_new ("");
  pressure_input = gtk_spin_button_new_with_range (0, 5, 0.05);

  GdkRGBA bg_doc_color;
  gdk_rgba_parse (&bg_doc_color, "#fff");
  bg_color_button = gtk_color_button_new_with_rgba (&bg_doc_color);
  bg_color_label = gtk_label_new ("");
  fg_color_label = gtk_label_new ("");

  /* Add the (already existing) colors as buttons to the color bar. */
  int a = 0;
  for (; a < settings.num_colors; a++)
    {
      GdkRGBA color;
      gdk_rgba_parse (&color, settings.colors[a]);
      GtkWidget* btn_color = gtk_color_button_new_with_rgba (&color);
      gtk_box_pack_start (GTK_BOX (hbox_colors), btn_color, 0, 0, 0);
    }

  /* Add the menu items to the menu. */
  for (a = 0; a < MENU_ITEMS_NUM; a++)
    {
      GtkWidget* menu_item = gtk_menu_item_new_with_label (menu_items[a]);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu_file), menu_item);

      g_signal_connect (G_OBJECT (menu_item), "activate",
			G_CALLBACK (gui_mainwindow_menu_file_activate), NULL);

    }

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_file), menu_file);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), menu_bar_file);

  /*--------------------------------------------------------------------------.
   | FURTHER CONFIGURATION                                                    |
   '--------------------------------------------------------------------------*/
  gtk_window_set_title (GTK_WINDOW (window), "InklingReader");
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_widget_set_size_request (window, WINDOW_WIDTH, WINDOW_HEIGHT);

  GdkRGBA bg;
  gdk_rgba_parse (&bg, "#101010");
  gtk_widget_override_background_color (document_view, GTK_STATE_NORMAL, &bg);

  gtk_container_add (GTK_CONTAINER (document_viewport), document_view);
  gtk_container_add (GTK_CONTAINER (document_container), document_viewport);

  gtk_label_set_markup (GTK_LABEL (bg_color_label), "<b>Background:</b>");
  gtk_label_set_markup (GTK_LABEL (fg_color_label), "<b>Foreground:</b>");
  gtk_label_set_markup (GTK_LABEL (pressure_label), "<b>Pressure factor:</b>");

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (pressure_input), settings.pressure_factor);

  /*--------------------------------------------------------------------------.
   | CONTAINERS                                                               |
   '--------------------------------------------------------------------------*/
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), menu_bar, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox_window), hbox_menu_top, 0, 0, 0);

  gtk_box_pack_start (GTK_BOX (hbox_menu_top), bg_color_label, 0, 0, 10);
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), bg_color_button, 0, 0, 0);

  gtk_box_pack_start (GTK_BOX (hbox_menu_top), fg_color_label, 0, 0, 10);
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), hbox_colors, 0, 0, 0);
  gtk_box_pack_end (GTK_BOX (hbox_colors), new_color_button, 0, 0, 0);

  gtk_box_pack_start (GTK_BOX (hbox_menu_top), pressure_label, 0, 0, 10);
  gtk_box_pack_start (GTK_BOX (hbox_menu_top), pressure_input, 0, 0, 0);

  gtk_box_pack_start (GTK_BOX (vbox_window), document_container, 1, 1, 0);

  gtk_container_add (GTK_CONTAINER (window), vbox_window);

  /*--------------------------------------------------------------------------.
   | SIGNALS                                                                  |
   '--------------------------------------------------------------------------*/
  g_signal_connect (G_OBJECT (window), "destroy", 
		    G_CALLBACK (gui_mainwindow_quit), NULL);

  g_signal_connect (G_OBJECT (document_view), "draw",
                    G_CALLBACK (gui_mainwindow_document_view_draw), NULL);

  g_signal_connect (G_OBJECT (new_color_button), "clicked",
		    G_CALLBACK (gui_mainwindow_add_color), NULL);

  g_signal_connect (G_OBJECT (bg_color_button), "color-set",
		    G_CALLBACK (gui_mainwindow_set_bg_color), NULL);

  g_signal_connect (G_OBJECT (pressure_input), "value-changed",
		    G_CALLBACK (gui_mainwindow_set_pressure_input), NULL);

  /*--------------------------------------------------------------------------.
   | DISPLAY                                                                  |
   '--------------------------------------------------------------------------*/

  gtk_widget_show_all (window);

  if (filename)
    gui_mainwindow_file_activated (NULL, (char*)filename);

  gtk_main ();
}
