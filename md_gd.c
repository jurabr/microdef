/*
   File name: md_gd.c
   Date:      2005/04/17 18:09
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

   MicroDefor - libGD output code
*/

#include "microdef.h"
#ifdef GDGUI
#include <gd.h>
#include <gdfonts.h>
#include <stdio.h>
#include <stdlib.h>

extern int md_draw(void);
extern void set_mdterm(int term);
extern int get_mdterm(void);

extern int gfxAction ;

gdImagePtr im;
int        black;
int        white;
int        red;
int        blue;
int        green;

int        mdgd_x0 = 0 ;
int        mdgd_y0 = 0 ;
int        mdgd_width  = 800 ;
int        mdgd_height = 600 ;

FILE      *mdgdFile = NULL ;

/* Universal drawing functions: */

void get_draw_size_gd(int *x0,int *y0,int *width,int *height)
{
  *x0     = mdgd_x0 ;
  *y0     = mdgd_y0 ;
  *width  = mdgd_width ;
  *height = mdgd_height ;
}

void mdgd_draw_point(int x,int  y)
{
  gdImageLine(im, x, y, x, y, black); /* not needed for output */ 
}

void mdgd_draw_line(int x1,int  y1,int  x2,int  y2,int  width)
{
#if 0
  gdImageSetThickness(im,width); /* can be used from GD-2.0 */
#endif
  gdImageLine(im, x1, y1, x2, y2, black);
}

void mdgd_draw_line_red(int x1,int  y1,int  x2,int  y2,int  width)
{
#if 0
  gdImageSetThickness(im,width); /* can be used from GD-2.0 */
#endif
  gdImageLine(im, x1, y1, x2, y2, red);
}
void mdgd_draw_line_blue(int x1,int  y1,int  x2,int  y2,int  width)
{
#if 0
  gdImageSetThickness(im,width); /* can be used from GD-2.0 */
#endif
  gdImageLine(im, x1, y1, x2, y2, blue);
}
void mdgd_draw_line_green(int x1,int  y1,int  x2,int  y2,int  width)
{
#if 0
  gdImageSetThickness(im,width); /* can be used from GD-2.0 */
#endif
  gdImageLine(im, x1, y1, x2, y2, green);
}

void mdgd_draw_string(int x,int  y, char *str)
{
  gdImageString(im, gdFontSmall, x, y, (unsigned char *)str, black) ;
}

void mdgd_draw_string_red(int x,int  y, char *str)
{
  gdImageString(im, gdFontSmall, x, y, (unsigned char *)str, red) ;
}
void mdgd_draw_string_green(int x,int  y, char *str)
{
  gdImageString(im, gdFontSmall, x, y, (unsigned char *)str, green) ;
}
void mdgd_draw_string_blue(int x,int  y, char *str)
{
  gdImageString(im, gdFontSmall, x, y, (unsigned char *)str, blue) ;
}

/* end of "universal drawing functions" */

/* ------------------------------------------ */

int mdgd_draw(int width, int height, char *fname)
{
  int mdterm_old = 0 ;
  mdgd_x0 = 0 ;
  mdgd_y0 = 0 ;
  mdgd_width  = width ;
  mdgd_height = height ;

  if ((im = gdImageCreate(mdgd_width, mdgd_height)) == NULL)
  { 
    return(ERR_MEM) ; 
  }

  mdterm_old  = get_mdterm(); ;
  set_mdterm(MDTERM_GD) ;

  black = gdImageColorAllocate(im, 0, 0, 0);
  white = gdImageColorAllocate(im, 255, 255, 255);
  red   = gdImageColorAllocate(im, 255, 0, 0);
  blue  = gdImageColorAllocate(im, 0, 0, 255);
  green = gdImageColorAllocate(im, 0, 255, 0);

  gdImageFill(im, 0, 0, white) ;

  md_draw();

  set_mdterm(mdterm_old) ;

  if ((mdgdFile = fopen(fname, "wb")) == NULL)
  {
    gdImageDestroy(im);
    return(ERR_IO) ;
  }

  gdImagePng(im, mdgdFile);

  fclose(mdgdFile);
  gdImageDestroy(im);

  return(OK);
}

int mdgd_draw_struct(int width, int height, char *fname)
{
  gfxAction = 0 ;
  return( mdgd_draw(width, height, md_set_file_ext(fname,"S","png")) );
}

int mdgd_draw_def(int width, int height, char *fname)
{
  gfxAction = 60 ;
  return( mdgd_draw(width, height, md_set_file_ext(fname,"RD","png")) );
}

int mdgd_draw_N(int width, int height, char *fname)
{
  gfxAction = 61 ;
  return( mdgd_draw(width, height, md_set_file_ext(fname,"N","png")) );
}

int mdgd_draw_V(int width, int height, char *fname)
{
  gfxAction = 62 ;
  return( mdgd_draw(width, height, md_set_file_ext(fname,"V","png")) );
}

int mdgd_draw_M(int width, int height, char *fname)
{
  gfxAction = 63 ;
  return( mdgd_draw(width, height, md_set_file_ext(fname,"M","png")) );
}

#endif

/* end of md_gd.c */
