/*
   File name: md_gtk.c
   Date:      2005/01/06 20:32
   Author:    Jiri Brozovsky

   Copyright (C) 2005 Jiri Brozovsky

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   in a file called COPYING along with this program; if not, write to
   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
   02139, USA.
*/

#ifdef _NANONOTE_
#define CNV_SIZE_X 320
#define CNV_SIZE_Y 220
#else
#define CNV_SIZE_X 800
#define CNV_SIZE_Y 600
#endif


#include "microdef.h"
#include "gtk/gtk.h"
#include "gdk/gdk.h"
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkcolor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

extern int pickMode ;
extern int gfxAction;

extern int md_draw(void);
extern void md_input_action(int x, int y, double val1, double val2, double val3);
extern void md_from_user_action(void);
extern int solve(void);

extern void set_mdterm(int term);
extern int get_mdterm(void);
extern int md_write_report( char *fname, int term, int print_input, int print_ke, int  print_k, int print_res, int print_rlist);
extern int md_write_maxwell( char *fname ) ;

/* prototypes: */
int mdgfx_set_input(char *framestr, char *l1, double val1, char *l2, double val2, char *l3, double val3, int butt);
void mdgui_scan_data(double *val1, double *val2, double *val3);
void mdgui_select_file(char *title);
void mdgui_select_file_save(char *title);

char *mdgui_set_title(char *fname);

gint screen_state     = 0 ;
char *data_file = NULL ;

GtkWidget *windowMain = NULL ;
GtkWidget *vbox       = NULL ;
GtkWidget *hbox       = NULL ;
GtkWidget *label1     = NULL ;
GtkWidget *label2     = NULL ;
GtkWidget *label3     = NULL ;
GtkWidget *text1      = NULL ;
GtkWidget *text2      = NULL ;
GtkWidget *text3      = NULL ;
GtkWidget *frame      = NULL ;
GtkWidget *button     = NULL ;
GtkWidget *button2    = NULL ;
GtkWidget *area       = NULL ;

GtkWidget *menubar    = NULL ;
GtkItemFactory *menu  = NULL ;
GtkAccelGroup *accelg = NULL ;

GdkPixmap *pixmap     = NULL ; /* off-screen pixmap */
GdkGC     *gc         = NULL ;
GdkPoint   point ;
GdkFont   *font = NULL ;
GdkColor     color_white;
GdkColor     color_black;
GdkColor     color_red;
GdkColor     color_blue;
GdkColormap *cmap = NULL ;

int label1_active = 0 ;
int label2_active = 0 ;
int label3_active = 0 ;

/* GFX cooperation --------------------------- */

void mdgtk_draw_point(int x, int y)
{
#if 0
  point.x = x ;
  point.y = y ;

  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);
#else
  point.x = x-1 ; point.y = y ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);
  point.x = x+1 ; point.y = y ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);

  point.x = x ; point.y = y-1 ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);
  point.x = x ; point.y = y+1 ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);
#endif
}

void mdgtk_draw_big_point(int x, int y)
{
  point.x = x ; point.y = y ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);

  point.x = x-1 ; point.y = y ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);
  point.x = x+1 ; point.y = y ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);

  point.x = x-2 ; point.y = y ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);
  point.x = x+2 ; point.y = y ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);

  point.x = x ; point.y = y-1 ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);
  point.x = x ; point.y = y+1 ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);

  point.x = x ; point.y = y-2 ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);
  point.x = x ; point.y = y+2 ;
  gdk_draw_points(pixmap, area->style->black_gc, &point, 1);
}

void mdgtk_draw_line(int x1, int y1, int x2, int y2, int width)
{
  gdk_gc_set_line_attributes(area->style->black_gc, width, GDK_LINE_SOLID,GDK_CAP_BUTT,GDK_JOIN_MITER);
  gdk_draw_line(pixmap, area->style->black_gc,x1,y1,x2,y2);
}


void mdgtk_draw_line_red(int x1, int y1, int x2, int y2, int width)
{
  gdk_gc_set_foreground(area->style->black_gc, &color_red);
  gdk_gc_set_line_attributes(area->style->black_gc, width, GDK_LINE_SOLID,GDK_CAP_BUTT,GDK_JOIN_MITER);
  gdk_draw_line(pixmap, area->style->black_gc,x1,y1,x2,y2);
  gdk_gc_set_foreground(area->style->black_gc, &color_black);
}

void mdgtk_draw_line_blue(int x1, int y1, int x2, int y2, int width)
{
  gdk_gc_set_foreground(area->style->black_gc, &color_blue);
  gdk_gc_set_line_attributes(area->style->black_gc, width, GDK_LINE_SOLID,GDK_CAP_BUTT,GDK_JOIN_MITER);
  gdk_draw_line(pixmap, area->style->black_gc,x1,y1,x2,y2);
  gdk_gc_set_foreground(area->style->black_gc, &color_black);
}


void mdgtk_draw_string(int x, int y, char *str)
{
  if (font == NULL) { font = gdk_font_load ("fixed"); }
  if (font == NULL) { return; }
  gdk_draw_string(pixmap, font, area->style->black_gc, x,y, str);
}

void mdgtk_draw_string_red(int x, int y, char *str)
{
  if (font == NULL) { font = gdk_font_load ("fixed"); }
  if (font == NULL) { return; }
  gdk_gc_set_foreground(area->style->black_gc, &color_red);
  gdk_draw_string(pixmap, font, area->style->black_gc, x,y, str);
  gdk_gc_set_foreground(area->style->black_gc, &color_black);
}
void mdgtk_draw_string_blue(int x, int y, char *str)
{
  if (font == NULL) { font = gdk_font_load ("fixed"); }
  if (font == NULL) { return; }
  gdk_gc_set_foreground(area->style->black_gc, &color_blue);
  gdk_draw_string(pixmap, font, area->style->black_gc, x,y, str);
  gdk_gc_set_foreground(area->style->black_gc, &color_black);
}



void get_draw_size(int *x0, int *y0, int *width, int *height)
{
  *x0 = 0 ;
  *y0 = 0 ;
  *width  = area->allocation.width  ;
  *height = area->allocation.height ;
}

/* sets properties of the input line */
int mdgfx_set_input(char *framestr, char *l1, double val1, char *l2, double val2, char *l3, double val3, int butt)
{
  char str[2049] ;
  int  i ;

  if (framestr == NULL)
  {
    gtk_widget_hide(frame);
    return(OK);
  }
  else
  {
    gtk_widget_show(frame);
  }

  gtk_frame_set_label(GTK_FRAME(frame), framestr);

  switch (butt)
  {
    case 0 : 
      gtk_widget_hide(button) ; 
      gtk_widget_hide(button2) ; 
      break ;
    case 1 : 
      gtk_widget_show(button) ; 
      gtk_widget_hide(button2) ; 
      break ;
    case 2 : 
      gtk_widget_show(button) ; 
      gtk_widget_show(button2) ; 
      break ;
    default:
      gtk_widget_hide(button) ; 
      gtk_widget_hide(button2) ; 
      break ;
  }

  if (l1 != NULL)
  {
    label1_active = 1 ;
    gtk_widget_show(label1) ;
    gtk_widget_show(text1) ;

    gtk_label_set_text(GTK_LABEL(label1), l1);
    for (i=0; i<2048; i++) {str[i] = '\0' ;}
		if (val1 == 0) { sprintf(str,"0"); }
		else { if (fabs(val1) < 1.0) { sprintf(str,"%f", val1); }
			else { sprintf(str,"%2.0f", val1); } }
    gtk_entry_set_text(GTK_ENTRY(text1), str) ;
  }
  else
  {
    label1_active = 0 ;
    gtk_widget_hide(label1) ;
    gtk_widget_hide(text1) ;
  }

  if (l2 != NULL)
  {
    label2_active = 1 ;
    gtk_widget_show(label2) ;
    gtk_widget_show(text2) ;

    gtk_label_set_text(GTK_LABEL(label2), l2);
    for (i=0; i<2048; i++) {str[i] = '\0' ;}
		if (val2 == 0) { sprintf(str,"0"); }
		else { if (fabs(val2) < 1.0) { sprintf(str,"%f", val2); }
			else { sprintf(str,"%2.0f", val2); } }
    gtk_entry_set_text(GTK_ENTRY(text2), str) ;
  }
  else
  {
    label2_active = 0 ;
    gtk_widget_hide(label2) ;
    gtk_widget_hide(text2) ;
  }

  if (l3 != NULL)
  {
    label3_active = 1 ;
    gtk_widget_show(label3) ;
    gtk_widget_show(text3) ;

    gtk_label_set_text(GTK_LABEL(label3), l3);
    for (i=0; i<2048; i++) {str[i] = '\0' ;}
		if (val3 == 0) { sprintf(str,"0"); }
		else { if (fabs(val3) < 1.0) { sprintf(str,"%f", val3); }
			else { sprintf(str,"%2.0f", val3); } }
    gtk_entry_set_text(GTK_ENTRY(text3), str) ;
  }
  else
  {
    label3_active = 0 ;
    gtk_widget_hide(label3) ;
    gtk_widget_hide(text3) ;
  }

  gtk_widget_show(frame) ;
  return(OK);
}

void mdgui_scan_data(double *val1, double *val2, double *val3)
{
  *val1 = 0 ;
  *val2 = 0 ;
  *val3 = 0 ;

  if (label1_active == 1) { *val1 = atof(gtk_entry_get_text(GTK_ENTRY(text1))); }
  if (label2_active == 1) { *val2 = atof(gtk_entry_get_text(GTK_ENTRY(text2))); }
  if (label3_active == 1) { *val3 = atof(gtk_entry_get_text(GTK_ENTRY(text3))); }
}
/* end of GFX cooperation --------------------------- */

/* dialog for messages type: 1=OK 2=error */
void mdgui_msg(char *text, int type)
{
  GtkWidget *dialog = NULL ;

  if (type == 2) /* error */
  {
    dialog = gtk_message_dialog_new (
      GTK_WINDOW(windowMain),
      GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_ERROR,
      GTK_BUTTONS_CLOSE,
      text);
  }
  else /* ok */
  {
    dialog = gtk_message_dialog_new (
      GTK_WINDOW(windowMain),
      GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_INFO,
      GTK_BUTTONS_OK,
      text);
  }

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

/* make a backing pixmap */
static gint draw_event(GtkWidget *widget, GdkEventConfigure *event)
{
  if (pixmap) { gdk_pixmap_unref(pixmap); }
  pixmap = gdk_pixmap_new(widget->window,
                          widget->allocation.width,
                          widget->allocation.height,
                          -1);   
  
  gdk_draw_rectangle (pixmap,
                      widget->style->white_gc,
                      TRUE,
                      0, 0,
                      widget->allocation.width,
                      widget->allocation.height);

  md_draw();

  return(TRUE);
} 



/* redraw the screen from the backing pixmap */ 
static gint expose_event (GtkWidget *widget, GdkEventExpose *event)
{   
  draw_event(widget, NULL);

  gdk_draw_pixmap(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                  pixmap,
                  event->area.x,
                  event->area.y,
                  event->area.x,
                  event->area.y,
                  event->area.width,
                  event->area.height);
  return(FALSE); 
}

gint delete_event( GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data )
{
	gtk_main_quit(); 
	return(TRUE); 
}

void destroy( GtkWidget *widget,
              gpointer   data )
{ 
	gtk_main_quit(); 
}

/** Pressing of button */
gint area_button_press (GtkWidget* widget, GdkEventButton *event, gpointer data) 
{
  int x = event->x;
  int y = event->y;
  double val1, val2, val3 ;

  if (event->button == 1)
  {
    if (pickMode > 0)
    {
      mdgui_scan_data(&val1, &val2, &val3);
      md_input_action(x, y, val1, val2, val3);
    
      md_draw();
      gtk_widget_draw (area, NULL);
    }
  }
  
  return (OK);
}

/* the "OK" button in bottom right corner was pressed */
gint button_click_action (GtkWidget* widget, GdkEventButton *event, gpointer data)
{
  double val1, val2, val3 ;

  mdgui_scan_data(&val1, &val2, &val3);
  md_input_action(0, 0, val1, val2, val3);

  md_draw();
  gtk_widget_draw (area, NULL);
  return(OK);
}

gint button2_click_action (GtkWidget* widget, GdkEventButton *event, gpointer data)
{
  gfxAction = 0 ;
  md_input_action(0, 0, 0, 0, 0);

  md_draw();
  gtk_widget_draw (area, NULL);
  return(OK);
}




/* MENU SYSTEM ----------------------------------------------- */

/* Runs the solver */
int execute_solver(void)
{
	/* run solver here */
	if (solve()!= OK)
	{
    mdgui_msg("Solution failed!", 2);
#ifdef DEVEL_VERBOSE
	  fprintf(stderr,"Solver failed!\n");
#endif
	}
	else
	{
    if (data_file != NULL) 
    {
		  write_data(data_file) ;
    }
  }
  return(OK);
}

void menu_ops(gpointer    data,
              guint       action,
              GtkWidget  *widget)
{
  gfxAction = (int) action ;

  if ((gfxAction >=58) && (gfxAction <= 68))
  {
    /* always run solver before results! */
    if  (execute_solver() != OK)
    {
      gfxAction = 0 ;
      return;
    }
  }

  if (gfxAction == 555) 
	{ 
    pickMode = 0 ;
    mdgfx_set_input(0, 0,0,0,0,0,0,0);

		/* run solver here */
		if (solve()!= OK)
		{
      gfxAction = 0 ;
      mdgui_msg("Solution failed!", 2);
#ifdef DEVEL_VERBOSE
			fprintf(stderr,"Solver failed!\n");
#endif
		}
		else
		{
      if (data_file != NULL) 
      {
			  write_data(data_file) ;
      }
      gfxAction = 60 ;
		}
	}

  if (gfxAction == 666) { gtk_main_quit() ; }

  if (gfxAction == 600) 
  { 
    md_make_empty();
    mdgui_select_file("MicroDef: please select a file or input name of new one");
    gfxAction = 0 ;
    return ;
  }

  if (gfxAction == 601) 
  { 
    gfxAction = 0 ;
    if ((n_len < 2) || (e_len < 1))
    {
      mdgui_msg("Cannot write empty data to file!\n",2);
      return ;
    }

    if (data_file == NULL) /* if trying to save unnamed file */
    {
      mdgui_select_file_save("MicroDef: please select a file or input name of new one");
      return;
    }

    if (write_data(data_file) != OK)
    {
      mdgui_msg("Unable to write data!\n",2);
    }
    return ;
  }

  if (gfxAction == 602) 
  { 
    gfxAction = 0 ;
    if ((n_len < 2) || (e_len < 1))
    {
      mdgui_msg("Cannot write empty data to file!\n",2);
      return ;
    }

    mdgui_select_file_save("MicroDef: please select a file or input name of new one");
    return ;
  }

  if (action == 58)
  {
#ifdef GDGUI
    if (data_file == NULL)
    {
      mdgui_msg("Please save your data first!\n",2);
      gfxAction = 602 ;
      return ;
    }
    
    set_mdterm( MDTERM_GD) ;
   
    mdgd_draw_struct(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdgd_draw_def(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdgd_draw_N(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdgd_draw_V(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdgd_draw_M(CNV_SIZE_X,CNV_SIZE_Y,data_file);

    set_mdterm( MDTERM_GTK) ;
#endif
    gfxAction = 0 ;
  }

  if (action == 59)
  {
#ifdef PSGUI
    if (data_file == NULL)
    {
      mdgui_msg("Please save your data first!\n",2);
      gfxAction = 602 ;
      return ;
    }

    set_mdterm( MDTERM_PS) ;
    
    mdps_draw_struct(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdps_draw_def(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdps_draw_N(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdps_draw_V(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdps_draw_M(CNV_SIZE_X,CNV_SIZE_Y,data_file);

    set_mdterm( MDTERM_GTK) ;
#endif
    gfxAction = 0 ;
  }

  if (action == 64)
  {
    md_write_report( data_file, MDOUT_TEXT, 1, 0, 0, 1, 0);
    gfxAction = 0 ;
  }

  if (action == 65)
  {
#ifdef GDGUI
    if (data_file == NULL)
    {
      mdgui_msg("Please save your data first!\n",2);
      gfxAction = 602 ;
      return ;
    }

    set_mdterm( MDTERM_GD) ;
   
    mdgd_draw_struct(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdgd_draw_def(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdgd_draw_N(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdgd_draw_V(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdgd_draw_M(CNV_SIZE_X,CNV_SIZE_Y,data_file);

    set_mdterm( MDTERM_GTK) ;
#endif

    md_write_report( data_file, MDOUT_HTML, 1, 0, 0, 1, 0);
    gfxAction = 0 ;
  }

  if (action == 66)
  {
#ifdef PSGUI
    if (data_file == NULL)
    {
      mdgui_msg("Please save your data first!\n",2);
      gfxAction = 602 ;
      return ;
    }

    set_mdterm( MDTERM_PS) ;
    
    mdps_draw_struct(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdps_draw_def(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdps_draw_N(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdps_draw_V(CNV_SIZE_X,CNV_SIZE_Y,data_file);
    mdps_draw_M(CNV_SIZE_X,CNV_SIZE_Y,data_file);

    set_mdterm( MDTERM_GTK) ;
#endif

    md_write_report( data_file, MDOUT_LATEX, 1, 0, 0, 1, 0);
    gfxAction = 0 ;
  }

  if (action == 67)
  {
    if (data_file == NULL)
    {
      mdgui_msg("Please save your data first!\n",2);
      gfxAction = 602 ;
      return ;
    }

    md_write_maxwell( data_file );
    gfxAction = 0 ;
  }


  md_from_user_action() ;
  gtk_widget_draw (area, NULL);
}

void make_menus(void)
{
  static GtkItemFactoryEntry menu_items[] = {
    { "/_File",         NULL,      NULL,         0, "<Branch>" },
#ifndef _OMAKO_
    { "/File/tear1",    NULL,      NULL,         0, "<Tearoff>" },
#endif
    { "/File/_Open..", "<CTRL>O", menu_ops,    600, "<Item>" },
    { "/File/_Save", "<CTRL>S", menu_ops,    601, "<Item>" },
    { "/File/_Save as..", "<CTRL><SHIFT>S", menu_ops,    602, "<Item>" },
    { "/File/sep1",     NULL,      NULL,         0, "<Separator>" },
    { "/File/_Quit",    "<CTRL>Q", menu_ops, 666, "<Item>"}, 


    { "/_Create",         NULL,      NULL,         0, "<Branch>" },
#ifndef _OMAKO_
    { "/Create/tear1",    NULL,      NULL,         0, "<Tearoff>" },
#endif
    { "/Create/_Node",     "<CTRL>N", menu_ops,     1, "<Item>" },
    { "/Create/sep1",     NULL,      NULL,         0, "<Separator>" },
    { "/Create/_Element: |--|", "<CTRL>E", menu_ops,    2, "<Item>" },
#if 0 /* refuses to work for  some reason: */
    { "/Create/Element: o--|", NULL, menu_ops,    3, "<Item>" },
#endif
    { "/Create/Element: |--o", NULL, menu_ops,    4, "<Item>" },
    { "/Create/Element: o--o", NULL, menu_ops,    29, "<Item>" },
    { "/Create/sep2",     NULL,      NULL,         0, "<Separator>" },
    { "/Create/Support: UX", "<Ctrl>X", menu_ops, 5, "<Item>" },
    { "/Create/Support: UY", "<Ctrl>Y", menu_ops, 6, "<Item>" },
    { "/Create/Support: UX+UY", "<Ctrl>Z", menu_ops, 7, "<Item>" },
#ifndef _OMAKO_
    { "/Create/Support: UX+ROTZ", NULL, menu_ops, 8, "<Item>" },
    { "/Create/Support: UY+ROTZ", NULL, menu_ops, 9, "<Item>" },
#endif
    { "/Create/_Support: UX+UY+ROTZ", "<CTRL>D", menu_ops, 10, "<Item>" },
    { "/Create/sep3",     NULL,      NULL,         0, "<Separator>" },
    { "/Create/Force or _Moment on Node", "<CTRL>F", menu_ops, 11, "<Item>" },
#ifndef _OMAKO_
    { "/Create/sep4",     NULL,      NULL,         0, "<Separator>" },
#endif
    { "/Create/Axial Beam Load", "<CTRL>L", menu_ops, 13, "<Item>" },
    { "/Create/Transverse Beam Load", "<CTRL>T", menu_ops, 14, "<Item>" },


    { "/_Edit",         NULL,      NULL,         0, "<Branch>" },
#ifndef _OMAKO_
    { "/Edit/tear6",    NULL,      NULL,         0, "<Tearoff>" },
#endif
    { "/Edit/Node",     NULL, menu_ops,     15, "<Item>" },
    { "/Edit/Element", NULL, menu_ops,    16, "<Item>" },

    { "/Edit/sep1",     NULL,      NULL,         0, "<Separator>" },
    { "/Edit/Set element to |--|",     NULL, menu_ops,     30, "<Item>" },
#if 0 /* refuses to work for  some reason: */
    { "/Edit/Set element to o--|",     NULL, menu_ops,     31, "<Item>" },
#endif
    { "/Edit/Set element to |--o",     NULL, menu_ops,     32, "<Item>" },
    { "/Edit/Set element to o--o",     NULL, menu_ops,     33, "<Item>" },


    { "/_Delete",         NULL,      NULL,         0, "<Branch>" },
#ifndef _OMAKO_
    { "/Delete/tear7",    NULL,      NULL,         0, "<Tearoff>" },
#endif
    { "/Delete/Node",     NULL, menu_ops,     21, "<Item>" },
    { "/Delete/Element", NULL, menu_ops,    22, "<Item>" },
    { "/Delete/Support", NULL, menu_ops, 23, "<Item>" },
    { "/Delete/Force or Moment", NULL, menu_ops, 24, "<Item>" },
    { "/Delete/Normal Beam Load", NULL, menu_ops,    25, "<Item>" },
    { "/Delete/Transverse Beam Load", NULL, menu_ops,    26, "<Item>" },

#if 0
    { "/_Solve",         NULL,      NULL,         0, "<Branch>" },
#ifndef _OMAKO_
    { "/Solve/tear8",    NULL,      NULL,         0, "<Tearoff>" },
#endif
    { "/Solve/_Run Solver",    "<CTRL>R", menu_ops, 555, "<Item>"}, 
#endif


    { "/_Results",         NULL,      NULL,         0, "<Branch>" },
#ifndef _OMAKO_
    { "/Results/tear9",    NULL,      NULL,         0, "<Tearoff>" },
#endif
    { "/Results/Reactions", "<CTRL>R", menu_ops,     60, "<Item>" },
    { "/Results/N", "<CTRL>A", menu_ops,    61, "<Item>" },
    { "/Results/V", "<CTRL>V", menu_ops,    62, "<Item>" },
    { "/Results/M", "<CTRL>M", menu_ops, 63, "<Item>"}, 
    { "/Results/sep3",     NULL,      NULL,         0, "<Separator>" },
    { "/Results/Model",    "<Ctrl>I", menu_ops, 0, "<Item>"}, 
    { "/Results/sep1",     NULL,      NULL,         0, "<Separator>" },
#ifdef GDGUI
    { "/Results/Write PNG Files", NULL, menu_ops, 58, "<Item>"},
#endif
#ifdef PSGUI
    { "/Results/Write PS Files", NULL, menu_ops, 59, "<Item>"},
#endif
    { "/Results/Write Report", NULL, menu_ops, 64, "<Item>"},
    { "/Results/Write Report (HTML)", NULL, menu_ops, 65, "<Item>"},
    { "/Results/Write Report (LaTeX)", NULL, menu_ops, 66, "<Item>"},
    { "/Results/sep3a",     NULL,      NULL,         0, "<Separator>" },
#ifndef _OMAKO_
    { "/Results/Write data for Maxwell-Mohr", NULL, menu_ops, 67, "<Item>"},
#endif


    { "/_Properties",         NULL,      NULL,         0, "<Branch>" },
#ifndef _OMAKO_
    { "/Properties/tear10",    NULL,      NULL,         0, "<Tearoff>" },
#endif
    { "/Properties/_Grid Size", "<CTRL>G", menu_ops, 27, "<Item>"},
    { "/Properties/_Underlined Elements", "<CTRL>U", menu_ops, 28, "<Item>"}, 
    { "/Properties/_Node Numbers", NULL, menu_ops, 41, "<Item>"}, 
    { "/Properties/_Elements Numbers", NULL, menu_ops, 42, "<Item>"} 
  };
  static gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

  accelg = gtk_accel_group_new ();
  menu   = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",accelg);
  gtk_item_factory_create_items (menu, nmenu_items, menu_items, NULL);
  gtk_window_add_accel_group (GTK_WINDOW (windowMain), accelg);
  menubar = gtk_item_factory_get_widget (menu, "<main>");
}

/* end of MENU SYSTEM ---------------------------------------- */

/* Key presses */
gboolean key_press_cb (GtkWidget * widget, GdkEventKey * event, GtkWindow * window)
{
  switch (event->keyval)
  {
    case GDK_Up:
      return TRUE;

    case GDK_Down:
      return TRUE;

    case GDK_Left:
      return TRUE;

    case GDK_Right:
      return TRUE;

    case GDK_Return:
      return TRUE ;

    case GDK_Escape: 
      return TRUE;
  }

  return FALSE;
}
			
gboolean window_state_cb (GtkWidget * widget, GdkEventWindowState * event,
		gpointer user_data)
{
  return FALSE;
}


void error_dialog(char *string)
{
  GtkWidget *dialog = NULL ;

  dialog = gtk_message_dialog_new (
      GTK_WINDOW(windowMain),
      GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_ERROR,
      GTK_BUTTONS_OK,
			string
      );

  	gtk_dialog_run (GTK_DIALOG (dialog));
  	gtk_widget_destroy (dialog);
}


/** GUI initialization*/
int gui_main(int argc, char *argv[])
{
  int rv = OK ;
	gboolean   homogenous = FALSE;
	gboolean   expand = FALSE;
	gint       spacing = 3 ;
	gint       padding = 0 ;

	/* main window: */
	windowMain = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(windowMain),
      "MicroDef 0.0.12");
  gtk_container_set_border_width (GTK_CONTAINER (windowMain), 1);

	vbox = gtk_vbox_new(homogenous, spacing);
	gtk_container_add(GTK_CONTAINER(windowMain),GTK_WIDGET(vbox));
	gtk_widget_show(vbox);

	g_signal_connect (G_OBJECT (windowMain), "delete_event",
                      G_CALLBACK (delete_event), NULL);
    
  g_signal_connect (G_OBJECT (windowMain), "destroy",
                      G_CALLBACK (destroy), NULL);

  make_menus();
	gtk_box_pack_start(GTK_BOX(vbox),menubar,FALSE, TRUE, padding);
	gtk_widget_show(menubar);


	area = gtk_drawing_area_new();
	gtk_box_pack_start(GTK_BOX(vbox),area,TRUE, TRUE, padding);
	gtk_widget_show(area);

	gtk_widget_set_size_request (area, CNV_SIZE_X, CNV_SIZE_Y);

	frame = gtk_frame_new("Input");
	gtk_box_pack_end(GTK_BOX(vbox),frame, expand, TRUE, padding);
	gtk_widget_show(frame);

	hbox = gtk_hbox_new(homogenous, spacing);
	gtk_container_add(GTK_CONTAINER(frame),hbox);
	gtk_widget_show(hbox);

	label1 = gtk_label_new("  ----: ") ;
	gtk_box_pack_start(GTK_BOX(hbox),label1,expand, TRUE, padding);
	gtk_widget_show(label1);

	text1 = gtk_entry_new() ;
	gtk_box_pack_start(GTK_BOX(hbox),text1, TRUE, TRUE, padding);
	gtk_widget_set_size_request (text1, 8, -1);
	gtk_widget_show(text1);

	label2 = gtk_label_new("  ----: ") ;
	gtk_box_pack_start(GTK_BOX(hbox),label2,expand, TRUE, padding);
	gtk_widget_show(label2);

	text2 = gtk_entry_new() ;
	gtk_box_pack_start(GTK_BOX(hbox),text2, TRUE, TRUE, padding);
	gtk_widget_set_size_request (text2, 8, -1);
	gtk_widget_show(text2);
	
	label3 = gtk_label_new("  ----: ") ;
	gtk_box_pack_start(GTK_BOX(hbox),label3,expand, TRUE, padding);
	gtk_widget_show(label3);

	text3 = gtk_entry_new() ;
	gtk_box_pack_start(GTK_BOX(hbox),text3, TRUE, TRUE, padding);
	gtk_widget_set_size_request (text3, 8, -1);
	gtk_widget_show(text3);

	button = gtk_button_new_with_label("OK") ;
	gtk_box_pack_start(GTK_BOX(hbox),button, TRUE, TRUE, padding);
	gtk_widget_show(button);

	button2 = gtk_button_new_with_label("Stop") ;
	gtk_box_pack_start(GTK_BOX(hbox),button2, TRUE, TRUE, padding);
	gtk_widget_show(button2);

	g_signal_connect (G_OBJECT (area), "expose_event", G_CALLBACK (expose_event), NULL);
  g_signal_connect(G_OBJECT(area), "button_press_event", G_CALLBACK(area_button_press), NULL);

	gtk_widget_set_events (area, GDK_EXPOSURE_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK
			| GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK); 

  g_signal_connect(G_OBJECT(button), "clicked",
      G_CALLBACK(button_click_action), NULL);

  g_signal_connect(G_OBJECT(button2), "clicked",
      G_CALLBACK(button2_click_action), NULL);

	/* key presses: */
	g_signal_connect(G_OBJECT(windowMain), 
			        "key_press_event", G_CALLBACK(key_press_cb), windowMain);
	
  gtk_widget_show (windowMain);
	gtk_widget_hide(frame);

  /* disable bottom frame by default: */
  gtk_widget_hide(frame) ;

  md_from_user_action() ; /* probably should be removed later */

#ifndef _OMAKO_
  if (data_file == NULL)
  {
    mdgui_select_file( "MicroDef: please select a file or input name of new one");
  }

  if (data_file != NULL)
  {
    if (strlen(data_file) > 1)
    {
	    gtk_window_set_title(GTK_WINDOW(windowMain),
      mdgui_set_title(data_file));
    }
  }
#else
  mdgui_set_title("untitled");
#endif /* ndef _OMAKO_ */

  gtk_main();

  return(rv);
}

/* File selection dialogs: *** *** */
char *mdgui_set_title(char *fname)
{
  int i ;
  static char str[2049];

  for (i=0; i<2049; i++) { str[i] = '\0' ; }
  snprintf(str,2048,"MicroDef 0.0.12: %s", fname);

  return(str);
}

#ifdef FEM_NEW_FILE_DLG
void fem_dlg_file_react(char *outfile, gpointer data, int open)
{
  int rv  = OK ;
  int i ;

  if ( (outfile != NULL) && (data != NULL) )
  {
    free(data_file) ; data_file = NULL ;
    if ((data_file = (char *)malloc(strlen(outfile)+1)) != NULL)
    {
      for (i=0; i<=(strlen(outfile)+1); i++) {data_file[i] = '\0';}
      strcat(data_file, outfile);
  }

    if (open == 1)
    {
      /* open file */
      if ((rv = read_test(outfile)) != OK)
      {
        if (rv == ERR_EMP)
        {
          md_alloc_empty() ;
        }
        else
        {
#if 1
          free(data_file) ; data_file = NULL ;
#else
          return;
#endif
        }
      }
      else
      {
        if ((rv = read_data(outfile)) != OK)
        {
          md_alloc_empty() ;
        }
      }
    }
    else
    {
      /* write to file */
      if (outfile != NULL) 
      {
        if ((rv = write_data(outfile)) != OK)
        {
#if 1
          free(data_file) ; data_file = NULL ;
#else
          return ;
#endif
        }
      }
    }

	  gtk_window_set_title(GTK_WINDOW(windowMain), mdgui_set_title(outfile));
  }
  else
  {
	  gtk_window_set_title(GTK_WINDOW(windowMain),
      mdgui_set_title(("no file!")));
  }
}

void mdgui_select_file_io(char *title, int open)
{
  GtkWidget *dialog = NULL ;
  GtkFileFilter* lakmus = NULL ;
  char      *filename = NULL;
  char       tiitle[2049] ;
  int        i;

  for (i=0; i<=2049; i++) 
  {
    tiitle  [i]='\0';
  }

  /* TODO: determine action type and filename */

  lakmus = gtk_file_filter_new() ;
  gtk_file_filter_add_pattern(lakmus,"*.dfr") ;
  gtk_file_filter_set_name(lakmus,("MicroDef data file")) ;



  if (open == 1)
  {
    sprintf(tiitle,"%s: ", "Open data file");
    dialog = gtk_file_chooser_dialog_new (tiitle,
				      (GtkWindow *)windowMain,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
  }
  else
  {
    sprintf(tiitle,"%s: ", "Save data to file");
    dialog = gtk_file_chooser_dialog_new (tiitle,
				      (GtkWindow *)windowMain,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);

    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
  }

  if (lakmus != NULL)
  {
    gtk_file_chooser_add_filter((GtkFileChooser *)dialog, lakmus) ;
    gtk_file_chooser_set_filter((GtkFileChooser *)dialog, lakmus) ;
  }

#if 0
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), femDataDir);
#endif

  if (data_file != NULL) 
  {
    if (data_file[0] != '\0') 
    { 
      gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), data_file); 
    }
  }

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    /* TODO: do something here: */
    fem_dlg_file_react(filename, tiitle, open); /* run command */

    g_free (filename);
    filename = NULL ; /* for sure */
  }

  gtk_object_destroy((GtkObject *)lakmus);

  gtk_widget_destroy (dialog);
}

void mdgui_select_file(char *title)
{
  mdgui_select_file_io(title, 1) ;
  while (data_file == NULL)
  {
    mdgui_select_file_io(title, 1) ;
  }
  if (strlen(data_file) > 1)
    { gtk_window_set_title(GTK_WINDOW(windowMain),
      mdgui_set_title(data_file)); }
}

void mdgui_select_file_save(char *title)
{
  mdgui_select_file_io(title, 0) ;
  while (data_file == NULL)
  {
    mdgui_select_file_io(title, 1) ;
  }
  if (strlen(data_file) > 1)
    { gtk_window_set_title(GTK_WINDOW(windowMain),
      mdgui_set_title(data_file)); }
}
#else
/* dialog */
void mdgui_get_fname (GtkFileSelection *file_selector0, gpointer file_selector)
{    
  int rv ;
  int len,i ;

  gtk_widget_show(windowMain) ;

  free(data_file) ; data_file = NULL ;

  if (gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector))!=NULL)
  {
    len = strlen((char *) gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector)));

    if ((data_file = (char *)malloc(sizeof(char)*(len+1))) != NULL)
    {
      for (i=0; i<=len; i++) {data_file[i] = '\0';}

      strcat(data_file,
        (char *)gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector))
          );
    }
  }
  
  if (data_file != NULL) 
  {

  if ((rv = read_test(data_file)) != OK)
  {
    if (rv == ERR_EMP)
    {
      md_alloc_empty() ;
    }
    else
    {
      free(data_file) ; data_file = NULL ;
    }
  }
  else
  {
    if ((rv = read_data(data_file)) != OK)
    {
      md_alloc_empty() ;
    }
  }

#if 1
  if (data_file != NULL)
  {
    if (strlen(data_file) > 1)
    {
	    gtk_window_set_title(GTK_WINDOW(windowMain),
      mdgui_set_title(data_file));
    }
  }
#endif
  }

  if (data_file == NULL)
  {
    mdgui_select_file( "MicroDef: please select a file or input name of new one") ;;
  }

}

void mdgui_cancel_fname (GtkFileSelection *file_selector0, gpointer file_selector)
{    

  gtk_widget_show(windowMain) ;

  if (data_file == NULL)
  {
    mdgui_select_file( "MicroDef: please select a file or input name of new one") ;;
  }
}

void mdgui_select_file(char *title)
{
  GtkWidget *file_selector;

  gtk_widget_hide(frame);

  gtk_widget_hide(GTK_WIDGET(windowMain)) ;

  file_selector = gtk_file_selection_new (title);

  g_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
    "clicked", G_CALLBACK (mdgui_get_fname), file_selector);

  g_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
    "clicked", G_CALLBACK (mdgui_cancel_fname), file_selector);
  
  g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
      "clicked",
      G_CALLBACK (gtk_widget_destroy),
      (gpointer) file_selector);
  g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
      "clicked",
      G_CALLBACK (gtk_widget_destroy),
      (gpointer) file_selector);

  gtk_file_selection_complete(GTK_FILE_SELECTION(file_selector), "./*.dfr");

  gtk_widget_show (file_selector);
}


/* save dialog */
void mdgui_get_fname_save (GtkFileSelection *file_selector0, gpointer file_selector)
{    
  int rv ;
  int len,i ;

  free(data_file) ; data_file = NULL ;

  if (gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector))!=NULL)
  {

  len = strlen((char *) gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector)));

  if ((data_file = (char *)malloc(sizeof(char)*(len+1))) != NULL)
  {
    for (i=0; i<=len; i++) {data_file[i] = '\0';}

    strcat(data_file,
      (char *)gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector))
        );
  }
  }

  if (data_file != NULL) 
  {
    if ((rv = write_data(data_file)) != OK)
    {
      free(data_file) ; data_file = NULL ;
    }
  }

  if (data_file == NULL)
  {
    mdgui_select_file_save( "MicroDef: please select a file or input name of new one") ;;
  }
}


void mdgui_cancel_fname_save (GtkFileSelection *file_selector0, gpointer file_selector)
{    
  if (data_file == NULL)
  {
    mdgui_select_file_save( "MicroDef: please select a file or input name of new one") ;;
  }
}

void mdgui_select_file_save(char *title)
{
  GtkWidget *fslelector_save;

  fslelector_save = gtk_file_selection_new (title);

  g_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fslelector_save)->ok_button),
    "clicked", G_CALLBACK (mdgui_get_fname_save), fslelector_save);

  g_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fslelector_save)->cancel_button),
    "clicked", G_CALLBACK (mdgui_cancel_fname_save), fslelector_save);
  
  g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (fslelector_save)->ok_button),
      "clicked",
      G_CALLBACK (gtk_widget_destroy),
      (gpointer) fslelector_save);
  g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (fslelector_save)->cancel_button),
      "clicked",
      G_CALLBACK (gtk_widget_destroy),
      (gpointer) fslelector_save);

  gtk_file_selection_complete(GTK_FILE_SELECTION(fslelector_save), data_file);

  gtk_widget_show (fslelector_save);
}
#endif

/* main program loop */
int main(int argc, char *argv[]) 
{ 
#ifndef _OMAKO_
  int rv, i, len ;
#endif

  data_file       = NULL ;

	setlocale(LC_NUMERIC,"C") ;

#ifndef _OMAKO_
  if (argc >= 2)
  {
    if ((rv=read_test(argv[1])) == OK)
    {
      len = strlen(argv[1]) ;
      if((data_file=(char *)malloc((len+1)*sizeof(char)))!=NULL) 
      {
        for (i=0; i<=len; i++) {data_file[i]='\0';}
        strcat(data_file,argv[1]);
        rv = read_data(data_file);
      }
    }
    else
    {
      if (rv == ERR_EMP)
      {
        len = strlen(argv[1]) ;
        if((data_file=(char *)malloc((len+1)*sizeof(char)))!=NULL) 
        {
          for (i=0; i<=len; i++) {data_file[i]='\0';}
          strcat(data_file,argv[1]);
          md_alloc_empty() ;
        }
      }
      else
      {
        data_file = NULL ;
        md_alloc_empty() ; /* newly added - is this necessary? */
      }
    }
  }
#else
  data_file = NULL ;
  md_alloc_empty() ;
#endif

	setlocale(LC_NUMERIC,"C") ;
	gtk_init (&argc, &argv);
	setlocale(LC_NUMERIC,"C") ;


  cmap = gdk_colormap_get_system(  );

  gdk_color_parse("White", &color_white);
  gdk_colormap_alloc_color(cmap, &color_white, FALSE, TRUE);

  gdk_color_parse("Black", &color_black);
  gdk_colormap_alloc_color(cmap, &color_black, FALSE, TRUE);

  gdk_color_parse("Red", &color_red);
  gdk_colormap_alloc_color(cmap, &color_red, FALSE, TRUE);

  gdk_color_parse("Blue", &color_blue);
  gdk_colormap_alloc_color(cmap, &color_blue, FALSE, TRUE);
	
	gui_main(argc, argv);

  return(OK); 
}

/* end of md_gtk.c */
