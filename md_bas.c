/*
   File name: md_bas.c
   Date:      Fri Mar 10 17:20:51 CET 2017
   Author:    Jiri Brozovsky

   Copyright (C) 2017 Jiri Brozovsky

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
#ifdef BASGUI
#include <stdio.h>
#include <stdlib.h>

extern int md_draw(void);
extern void set_mdterm(int term);
extern int get_mdterm(void);

extern int gfxAction ;

int        mdbas_x0 = 0 ;
int        mdbas_y0 = 0 ;
int        mdbas_width  = 160 ;
int        mdbas_height = 160 ;

FILE      *mdbasFile = NULL ;

static int mdbasCounter = 0 ;

/* Universal drawing functions: */

void get_draw_size_bas(int *x0,int *y0,int *width,int *height)
{
  *x0     = mdbas_x0 ;
  *y0     = mdbas_y0 ;
  *width  = mdbas_width ;
  *height = mdbas_height ;
}

void mdbas_draw_point(int x,int  y)
{
  fprintf(mdbasFile,"%i PSET %i, %i\n", mdbasCounter++, x,y);
}

void mdbas_draw_line(int x1,int  y1,int  x2,int  y2,int  width)
{
  fprintf(mdbasFile,"%i LINE %i, %i, %i, %i\n", mdbasCounter++, x1,y1,x2,y2);
}

void mdbas_draw_string(int x,int  y, char *str)
{
#if 0 /* this is not a good way to write strings!!! */
  fprintf(mdbasFile,"%i PSET %i, %i\n", mdbasCounter++, x,y);
  fprintf(mdbasFile,"%i PRINT %s\n",mdbasCounter++, str);
#endif
}

/* end of "universal drawing functions" */

/* ------------------------------------------ */

int mdbas_draw(int width, int height, char *fname)
{
  int mdterm_old = 0 ;
  mdbas_x0 = 0 ;
  mdbas_y0 = 0 ;
  mdbas_width  = width ;
  mdbas_height = height ;

  if ((mdbasFile = fopen(fname, "wb")) == NULL)
  {
    return(ERR_IO) ;
  }

  mdbasCounter = 10 ;

  mdterm_old  = get_mdterm(); ;
  set_mdterm(MDTERM_BAS) ;

  md_draw();

  set_mdterm(mdterm_old) ;

  fclose(mdbasFile);

  return(OK);
}

int mdbas_draw_struct(int width, int height, char *fname)
{
  gfxAction = 0 ;
  return( mdbas_draw(width, height, md_set_file_ext(fname,"S","bas")) );
}

int mdbas_draw_def(int width, int height, char *fname)
{
  gfxAction = 60 ;
  return( mdbas_draw(width, height, md_set_file_ext(fname,"RD","bas")) );
}

int mdbas_draw_N(int width, int height, char *fname)
{
  gfxAction = 61 ;
  return( mdbas_draw(width, height, md_set_file_ext(fname,"N","bas")) );
}

int mdbas_draw_V(int width, int height, char *fname)
{
  gfxAction = 62 ;
  return( mdbas_draw(width, height, md_set_file_ext(fname,"V","bas")) );
}

int mdbas_draw_M(int width, int height, char *fname)
{
  gfxAction = 63 ;
  return( mdbas_draw(width, height, md_set_file_ext(fname,"M","bas")) );
}

#endif

/* end of md_bas.c */
