/*
   File name: md_plt.c
   Date:      2012/07/18 17:57
   Author:    Jiri Brozovsky

   Copyright (C) 2012 Jiri Brozovsky

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

   MicroDefor - plot(1) compatible output code
*/

#include "microdef.h"
#ifdef PLTGUI
#include <stdio.h>
#include <stdlib.h>
#include <plot.h>

extern int md_draw(void);
extern void set_mdterm(int term);
extern int get_mdterm(void);

extern int gfxAction ;

int        mdplt_x0 = 0 ;
int        mdplt_y0 = 0 ;
int        mdplt_width  = 800 ;
int        mdplt_height = 600 ;

FILE      *mdpltFile = NULL ;

/* Universal drawing functions: */

void get_draw_size_plt(int *x0,int *y0,int *width,int *height)
{
  *x0     = mdplt_x0 ;
  *y0     = mdplt_y0 ;
  *width  = mdplt_width ;
  *height = mdplt_height ;
}

void mdplt_draw_point(int x,int  y)
{
  point(x,mdps_height-y,);
}

void mdplt_draw_line(int x1,int  y1,int  x2,int  y2,int  width)
{
  line(x1,mdplt_height-y1,x2,mdplt_height-y2);
}

void mdplt_draw_string(int x,int  y, char *str)
{
  move(x,mdplt_height-y);
  label(str);
}

/* end of "universal drawing functions" */

/* ------------------------------------------ */

int mdplt_draw(int width, int height, char *fname)
{
  int mdterm_old = 0 ;
  mdplt_x0 = 0 ;
  mdplt_y0 = 0 ;
  mdplt_width  = width ;
  mdplt_height = height ;

  if ((mdpltFile = fopen(fname, "wb")) == NULL)
  {
    return(ERR_IO) ;
  }

  openpl();

#if 0
  fprintf(mdpltFile,"space(%i %i %i %i)\n",
      mdplt_x0,mdplt_y0, mdplt_width,mdplt_height);
#endif

  mdterm_old  = get_mdterm(); ;
  set_mdterm(MDTERM_PS) ;

  md_draw();


  set_mdterm(mdterm_old) ;

  closepl();

  fclose(mdpltFile);

  return(OK);
}

int mdplt_draw_struct(int width, int height, char *fname)
{
  gfxAction = 0 ;
  return( mdplt_draw(width, height, md_set_file_ext(fname,"S","ps")) );
}

int mdplt_draw_def(int width, int height, char *fname)
{
  gfxAction = 60 ;
  return( mdplt_draw(width, height, md_set_file_ext(fname,"RD","ps")) );
}

int mdplt_draw_N(int width, int height, char *fname)
{
  gfxAction = 61 ;
  return( mdplt_draw(width, height, md_set_file_ext(fname,"N","ps")) );
}

int mdplt_draw_V(int width, int height, char *fname)
{
  gfxAction = 62 ;
  return( mdplt_draw(width, height, md_set_file_ext(fname,"V","ps")) );
}

int mdplt_draw_M(int width, int height, char *fname)
{
  gfxAction = 63 ;
  return( mdplt_draw(width, height, md_set_file_ext(fname,"M","ps")) );
}

#endif

/* end of md_plt.c */
