/*
   File name: md_ps.c
   Date:      2005/04/17 23:45
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

   MicroDefor - PostScript output code
*/

#include "microdef.h"
#ifdef PSGUI
#include <stdio.h>
#include <stdlib.h>

extern int md_draw(void);
extern void set_mdterm(int term);
extern int get_mdterm(void);

extern int gfxAction ;

int        mdps_x0 = 0 ;
int        mdps_y0 = 0 ;
int        mdps_width  = 800 ;
int        mdps_height = 600 ;

FILE      *mdpsFile = NULL ;

/* Universal drawing functions: */

void get_draw_size_ps(int *x0,int *y0,int *width,int *height)
{
  *x0     = mdps_x0 ;
  *y0     = mdps_y0 ;
  *width  = mdps_width ;
  *height = mdps_height ;
}

void mdps_draw_point(int x,int  y)
{
  /* not needed for output */ 
#if 0
  fprintf(mdpsFile,"%i %i moveto %i %i lineto\nclosepath\n",x-1,mdps_height-y,x+1,mdps_height-y+1);
  fprintf(mdpsFile,"1 stroke\n");
#endif
}

void mdps_draw_line(int x1,int  y1,int  x2,int  y2,int  width)
{
  fprintf(mdpsFile,"%i %i moveto %i %i lineto\nclosepath\n",x1,mdps_height-y1,x2,mdps_height-y2);
  fprintf(mdpsFile,"%i stroke\n", width);
}

void mdps_draw_string(int x,int  y, char *str)
{
  fprintf(mdpsFile,"%i %i moveto\n (%s)show\n",x,mdps_height-y,str);
}

/* end of "universal drawing functions" */

/* ------------------------------------------ */

int mdps_draw(int width, int height, char *fname)
{
  int mdterm_old = 0 ;
  mdps_x0 = 0 ;
  mdps_y0 = 0 ;
  mdps_width  = width ;
  mdps_height = height ;

  if ((mdpsFile = fopen(fname, "wb")) == NULL)
  {
    return(ERR_IO) ;
  }

  fprintf(mdpsFile,"%%!PS-Adobe-2.0\n");
  fprintf(mdpsFile,"%%%%Creator: MicroDef \n");
  fprintf(mdpsFile,"%%%%BoundingBox: %i %i %i %i\n",
      mdps_x0,mdps_y0, mdps_width,mdps_height);
  fprintf(mdpsFile,"/Helvetica\nfindfont\n");
  fprintf(mdpsFile,"12 scalefont setfont\n");
  fprintf(mdpsFile,"0 setgray\n");

  mdterm_old  = get_mdterm(); ;
  set_mdterm(MDTERM_PS) ;

  md_draw();

  fprintf(mdpsFile,"showpage\n");

  set_mdterm(mdterm_old) ;

  fclose(mdpsFile);

  return(OK);
}

int mdps_draw_struct(int width, int height, char *fname)
{
  gfxAction = 0 ;
  return( mdps_draw(width, height, md_set_file_ext(fname,"S","ps")) );
}

int mdps_draw_def(int width, int height, char *fname)
{
  gfxAction = 60 ;
  return( mdps_draw(width, height, md_set_file_ext(fname,"RD","ps")) );
}

int mdps_draw_N(int width, int height, char *fname)
{
  gfxAction = 61 ;
  return( mdps_draw(width, height, md_set_file_ext(fname,"N","ps")) );
}

int mdps_draw_V(int width, int height, char *fname)
{
  gfxAction = 62 ;
  return( mdps_draw(width, height, md_set_file_ext(fname,"V","ps")) );
}

int mdps_draw_M(int width, int height, char *fname)
{
  gfxAction = 63 ;
  return( mdps_draw(width, height, md_set_file_ext(fname,"M","ps")) );
}

#endif

/* end of md_ps.c */
