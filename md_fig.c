/*
   File name: md_fig.c
   Date:      2016/06/14 20:38
   Author:    Jiri Brozovsky

   Copyright (C) 2016 Jiri Brozovsky

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

   MicroDefor - (X)Fig 3.2 output code
*/

#include "microdef.h"
#ifdef PSGUI
#include <stdio.h>
#include <stdlib.h>

extern int md_draw(void);
extern void set_mdterm(int term);
extern int get_mdterm(void);

extern int gfxAction ;

int        mdfig_x0 = 0 ;
int        mdfig_y0 = 0 ;
int        mdfig_width  = 1200 ;
int        mdfig_height = 1200 ;

FILE      *mdfigFile = NULL ;

/* Universal drawing functions: */

void get_draw_size_fig(int *x0,int *y0,int *width,int *height)
{
  *x0     = mdfig_x0 ;
  *y0     = mdfig_y0 ;
  *width  = mdfig_width ;
  *height = mdfig_height ;
}

void mdfig_draw_point(int x,int  y)
{
  /* not needed for output */ 
}

void mdfig_draw_line(int x1,int  y1,int  x2,int  y2,int  width)
{
  fprintf(mdfigFile,
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n    %i %i %i %i\n",
      x1,y1,x2,y2);
}

void mdfig_draw_line_red(int x1,int  y1,int  x2,int  y2,int  width)
{
  fprintf(mdfigFile,
      "2 1 0 1 4 7 50 -1 -1 0.000 0 0 -1 0 0 2\n    %i %i %i %i\n",
      x1,y1,x2,y2);
}

void mdfig_draw_line_blue(int x1,int  y1,int  x2,int  y2,int  width)
{
  fprintf(mdfigFile,
      "2 1 0 1 1 7 50 -1 -1 0.000 0 0 -1 0 0 2\n    %i %i %i %i\n",
      x1,y1,x2,y2);
}




void mdfig_draw_string(int x,int  y, char *str)
{
  fprintf(mdfigFile,"4 0 0 50 -1 0 10 0.0000 4 135 360 %i %i %s\\001\n",
      x,y,str);
}

void mdfig_draw_string_red(int x,int  y, char *str)
{
  fprintf(mdfigFile,"4 0 4 50 -1 0 10 0.0000 4 135 360 %i %i %s\\001\n",
      x,y,str);
}

void mdfig_draw_string_blue(int x,int  y, char *str)
{
  fprintf(mdfigFile,"4 0 1 50 -1 0 10 0.0000 4 135 360 %i %i %s\\001\n",
      x,y,str);
}





/* end of "universal drawing functions" */

/* ------------------------------------------ */

int mdfig_draw(int width, int height, char *fname)
{
  int mdterm_old = 0 ;
  mdfig_x0 = 0 ;
  mdfig_y0 = 0 ;
  mdfig_width  = width ;
  mdfig_height = height ;

  if ((mdfigFile = fopen(fname, "wb")) == NULL)
  {
    return(ERR_IO) ;
  }

  fprintf(mdfigFile,"#FIG 3.2\n");
  fprintf(mdfigFile,"Landscape\nCenter\nInches\nLetter\n100.00\nSingle\n");
  fprintf(mdfigFile,"-2\n1200 2\n");

  mdterm_old  = get_mdterm(); ;
  set_mdterm(MDTERM_FIG) ;

  md_draw();

  set_mdterm(mdterm_old) ;

  fclose(mdfigFile);

  return(OK);
}

int mdfig_draw_struct(int width, int height, char *fname)
{
  gfxAction = 0 ;
  return( mdfig_draw(width, height, md_set_file_ext(fname,"S","fig")) );
}

int mdfig_draw_def(int width, int height, char *fname)
{
  gfxAction = 60 ;
  return( mdfig_draw(width, height, md_set_file_ext(fname,"RD","fig")) );
}

int mdfig_draw_N(int width, int height, char *fname)
{
  gfxAction = 61 ;
  return( mdfig_draw(width, height, md_set_file_ext(fname,"N","fig")) );
}

int mdfig_draw_V(int width, int height, char *fname)
{
  gfxAction = 62 ;
  return( mdfig_draw(width, height, md_set_file_ext(fname,"V","fig")) );
}

int mdfig_draw_M(int width, int height, char *fname)
{
  gfxAction = 63 ;
  return( mdfig_draw(width, height, md_set_file_ext(fname,"M","fig")) );
}

#endif

/* end of md_fig.c */
