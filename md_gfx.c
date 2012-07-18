/*
   File name: md_gfx.c
   Date:      2005/01/06 20:28
   Author:    Jiri Brozovsky

   Copyright (C) 2005 Jiri Brozovsky

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
  
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.

	 MicroDefor - gfx-independent graphics routines for ANSI C systems
*/

#include "microdef.h"

#define PI 3.141592653589793238462643383279502884

#ifdef GTKGUI
extern int mdgfx_set_input(char *framestr, char *l1, double val1, char *l2, double val2, char *l3, double val3, char *l4, double val4, int butt);
#define GGUI
#endif
extern char *md_intstring(int number);
extern char *md_double2string01(double number);
extern char *md_double2string04(double number);
extern double md_compute_e_def_y(int e_type, double L, double q1, double q2, double x);

int mdSimpleE  =    1 ; /* 1..simpler element look, 0..default */
int mdShowGrid =    1 ; /* 1..show grid, 0..no          */
int mdX0       =    0 ;
int mdY0       =    0 ;
int mdWidth    =    0 ;
int mdHeight   =    0 ;

/* Extra zoom data: */
double gf_zoom =    1.0 ;
int    gf_movx =    0 ;
int    gf_movy =    0 ;

#ifdef _NANONOTE_
int mdText     =    1 ;
#else
int mdText     =    0 ;
#endif

static int mdTerm     =    1 ; /* graphics terminal            */
#ifdef _NANONOTE_
int gridSpace  =   16 ; /* space between grid point     */
#else
int gridSpace  =   32 ; /* space between grid point     */
#endif


double resMult  = 1.0 ; /* N,V,M result multiplier       */

int   pickMode = 0 ; /* 0=nothing, 1=grid, 2=node 3=elem */
int   gfxAction= 0 ; /* 0=nothing, 
                        1=add node 
                        2=add elem |-|
                        3=add elem o-|
                        4=add elem |-o
                        5=add b.c.: ux
                        6=add b.c.: uy
                        7=add b.c.: ux+uy
                        8=add b.c.: ux+rotz
                        9=add b.c.: uy+rotz
                        10=add b.c.: ux+uy+rotz
                        11=add force 
                        12=add nonzero displacement
                        13=add na,nb
                        14=add va,vb
                        15=edit node
                        16=edit elements
                        17=edit force
                        18=edit nonzero displacements
                        19=edit na,nb
                        20=edit va,vb
                        21=delete node
                        22=delete elements
                        23=delete b.c
                        24=delete force
                        25=delete na,nb
                        26=delete va,vb
                        27=set grid real size
                        28=(un)underline elements
                        29=add elem o-o
                        30=change element to |--|
                        31=change element to o--|
                        32=change element to |--o
                        33=change element to o--o
                        41=snow node numbers
                        42=snow element numbers
                        43=keyboard or mouse  control
                        60=plot deformations + reactions
                        61=plot N
                        62=plot M
                        63=plot V
                        64=write report (txt)
                        65=write report (HTML)
                        66=write report (LaTeX)
                        67=write Maxwell-Mohr
                        */
double defaultE    = 10e9 ;
double defaultA    = 0.01 ;
double defaultI    = 8.333e-4 ;
double defaultFx   = 0 ;
double defaultFy   = 0 ;
double defaultMz   = 0 ;
double defaultPosX = 0 ;
double defaultPosY = 0 ;
double defaultRotZ = 0 ;
double defaultNA   = 0 ;
double defaultNB   = 0 ;
double defaultVA   = 0 ;
double defaultVB   = 0 ;

int    defaultET   = 2 ;

int   maxFMi      = 0 ;   /* max. plottted size of load   */
int   maxDRi      = 0 ;   /* max. plottted size of disp.  */

int   resEnds     = 1 ;   /* if N,V,M sizes on ends       */
int   resExtr     = 1 ;   /* if N,V,M extemre sizes       */

int    nodeEdit   = -1 ;   /* number of node to me edited */
int    elemEdit   = -1 ;   /* number of elem to me edited */
int    nodeOK     =  0 ;
int    elemOK     =  0 ;

#ifdef _NANONOTE_
int    nodeNumbers = 1 ;
int    elemNumbers = 1 ;
#else
int    nodeNumbers = 0 ;
int    elemNumbers = 0 ;
#endif

void set_mdterm(int term) { mdTerm = term ; }

int get_mdterm(void) { return(mdTerm); }

/* translates real size to screen coordinates */
int gfx_pos_x(double x)
{
  return( (int) ((gf_zoom * gridSpace * ( x / gridReal )) + 2 +mdX0)+ gf_movx  ) ;
}

int gfx_pos_y(double x)
{
  return( (int) ((mdHeight - 3 - gf_zoom * gridSpace * ( x / gridReal )) + mdY0)+ gf_movy  ) ;
}

/* returns [x,y] of active point of line */
void gfx_elem_act_point(int pos, int *x, int *y)
{
  int n1, n2 ;

  n1 = GET_ELEM_N1(pos) ;
  n2 = GET_ELEM_N2(pos) ;

  *x = gfx_pos_x((0.5*(GET_NODE_X(n1) + GET_NODE_X(n2)))) ;
  *y = gfx_pos_y((0.5*(GET_NODE_Y(n1) + GET_NODE_Y(n2)))) ;
}

/* finds "clicked" node */
int gfx_node_find_clicked(int x, int y)
{
  int   i ;
  int   xn, yn ;
  int   tol ;
  int   pos = -1 ; 
  double dist0 = -1.0 ;
  double dist = 0.0 ;

  tol = (int) (gridSpace) ;

  for (i=0; i<n_len; i++)
  {
    xn = gfx_pos_x( GET_NODE_X(i));
    yn = gfx_pos_y( GET_NODE_Y(i));

    if (((x-tol) <= xn)&&((x+tol) >= xn)&&((y-tol) <= yn)&&((y+tol) >= yn))
    {
      dist = sqrt(pow((double)(xn-x),2)+pow((double)(yn-y),2)) ;

      if (dist0 < 0)
      {
        pos = i ;
      }
      else
      {
        if (dist < dist0)
        {
          pos = i ;
        }
      }
      dist0 = dist ;
    }
  }

  return(pos) ;
}

/* finds "clicked" element */
int gfx_elem_find_clicked(int x, int y)
{
  int   i ;
  int   xn, yn ;
  int   tol ;
  int   pos = -1 ; 
  double dist0 = -1.0 ;
  double dist = 0.0 ;

  tol = (int) (gridSpace) ;

  for (i=0; i<e_len; i++)
  {
    gfx_elem_act_point(i, &xn, &yn) ;

    if (((x-tol) <= xn)&&((x+tol) >= xn)&&((y-tol) <= yn)&&((y+tol) >= yn))
    {
      dist = sqrt(pow((double)(xn-x),2)+pow((double)(yn-y),2)) ;

      if (dist0 < 0)
      {
        pos = i ;
      }
      else
      {
        if (dist < dist0)
        {
          pos = i ;
        }
      }
      dist0 = dist ;
    }
  }

  return(pos) ;
}

/* finds clicked grid coordinates */
void md_grid_find_clicked(int x, int y, double *xg, double *yg)
{
  int   i,j;
  int   xn, yn ;
  int   tol ;
  double dist0 = -1.0 ;
  double dist = 0.0 ;

  tol = (int)(gridSpace / 2) ;
  *xg = 0 ;
  *yg = 0 ;

  for (i=0; i<=(int)((mdWidth-4-mdX0)/gridSpace); i++)
  {
    for (j=0; j<=(int)((mdHeight-4+mdY0)/gridSpace); j++)
    {
      xn = i*gridSpace + 2 + mdX0 ;
      yn = mdHeight - 3 - j*gridSpace + mdY0 ;
    
      if (((x-tol) <= xn)&&((x+tol) >= xn)&&((y-tol) <= yn)&&((y+tol) >= yn))
      {
        dist = sqrt(pow((double)(xn-x),2)+pow((double)(yn-y),2)) ;

        if (dist0 < 0)
        {
          *xg = (double)i*gridReal ;
          *yg = (double)j*gridReal ;
        }
        else
        {
          if (dist < dist0)
          {
            *xg = (double)i*gridReal ;
            *yg = (double)j*gridReal ;
          }
        }
        dist0 = dist ;
      }
    }
  }
}


/* sets grid and limit properties for real data */
void plot_real_limits(void)
{
  int i ;
	int gridSpaceX, gridSpaceY ;
  double max_x = 0 ;
  double max_y = 0 ;

  double min_x = 0 ;
  double min_y = 0 ;

  if (e_len < 1) 
  {
    mdX0 += 4 ;
    mdY0 += -4 ;
    return;
  }

  for (i=0; i<n_len; i++)
  {
    if (GET_NODE_X(i) > max_x) {max_x = GET_NODE_X(i);}
    if (GET_NODE_Y(i) > max_y) {max_y = GET_NODE_Y(i);}
  }

  if (gridReal <= 0.0) { gridReal = 0.005 ; }

  min_x = max_x ;
  min_y = max_y ;

  for (i=0; i<n_len; i++)
  {
    if (GET_NODE_X(i) < min_x) {min_x = GET_NODE_X(i);}
    if (GET_NODE_Y(i) < min_y) {min_y = GET_NODE_Y(i);}
  }

  if ((max_y <= 0) && (max_x <= 0)) { return ; }

  gridSpaceX = 0 ;
  gridSpaceY = 0 ;

  if ((max_y - min_y) > 0)
  {
    if ((max_y-min_y) != 0.0)
    {
      gridSpaceY = (int) ((mdHeight-2.4*maxFMi)/(((max_y-min_y)/gridReal))) ;
    }
    else
    {
      gridSpaceY = 0 ;
    }
  }

  if ((max_x - min_x) > 0)
  {
    if ((max_x-min_x) != 0.0)
    {
      gridSpaceX = (int) ((mdWidth -2.4*maxFMi)/(((max_x-min_x)/gridReal))) ;
    }
    else
    {
      gridSpaceX = 0 ;
    }
  }

/* printf("GridSpace: orig=%i x=%i y=%i\n",gridSpace,gridSpaceX,gridSpaceY); */

		if (gridSpaceX > gridSpaceY)
		{
      if (gridSpaceY > 0)
      {
			  gridSpace = gridSpaceY ;
      }
      else
      {
			  gridSpace = gridSpaceX ;
      }
		}
		else
		{
      if (gridSpaceX > 0)
      {
			  gridSpace = gridSpaceX ;
      }
      else
      {
			  gridSpace = gridSpaceY ;
      }
		}

  /*printf("gridSpace [x, y] = [%i, %i] = %i\n",gridSpaceX, gridSpaceY,gridSpace);*/

  if ((gridReal > (1.3*(max_x-min_x)))||(gridReal > (1.3*(max_y-min_y))))
  {
    if ( (max_y-min_y) > (max_x-min_x) ) 
    {
      if (gridReal > (1.3*(max_y-min_y)))
      {
        gridReal  = (max_y-min_y)/10 ;
        gridSpace = (int)((mdHeight-2.4*maxFMi)/12) ;
      }
    }
    else
    {
      if (gridReal > (1.3*(max_x-min_x)))
      {
        gridReal  = (max_x-min_x)/10 ;
        gridSpace = (int)((mdWidth-2.4*maxFMi)/12) ;
      }
    }
  }

#if 0
  /* non-centered: */
  mdX0 += (int)( 2*gridSpace - (int)(gridSpace*min_x/gridReal) ) ;
  mdY0 += (int)( -2*gridSpace +(int)(gridSpace*min_y/gridReal) ) ;
#else
  /* centered: */
  mdX0 += (int)( (int)(0.4*(mdWidth-gridSpace*(max_x-min_x)/gridReal))
      - (int)(gridSpace*min_x/gridReal) ) ;
  mdY0 += (-(int)(0.44*(mdHeight-gridSpace*(max_y-min_y)/gridReal))
      +(int)(gridSpace*min_y/gridReal) ) ;
#endif
}

/* Universal drawing functions: ################################## */

void md_get_size(int *x0, int *y0, int *width, int *height)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: get_draw_size(x0, y0, width, height); break ;
#endif
#ifdef GDGUI
    case MDTERM_GD:  get_draw_size_gd(x0,y0,width,height); break ;
#endif
#ifdef PSGUI
    case MDTERM_PS:  get_draw_size_ps(x0,y0,width,height); break ;
#endif
#ifdef PLTGUI
    case MDTERM_PLT:  get_draw_size_plt(x0,y0,width,height); break ;
#endif

  }
}

void md_draw_point(int x, int y)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_point(x, y); break ;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_point(x, y); break ;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_point(x, y); break ;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_point(x, y); break ;
#endif
  }
}

void md_draw_big_point(int x, int y)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_big_point(x, y); break ;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_point(x, y); break ;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_point(x, y); break ;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_point(x, y); break ;
#endif
  }
}


void md_draw_line(int x1, int y1, int x2, int y2, int width)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_line(x1, y1, x2, y2, width); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_line(x1, y1, x2, y2, width); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_line(x1, y1, x2, y2, width); break;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_line(x1, y1, x2, y2, width); break;
#endif
  }
}

void md_draw_line_red(int x1, int y1, int x2, int y2, int width)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_line_red(x1, y1, x2, y2, width); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_line_red(x1, y1, x2, y2, width); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_line(x1, y1, x2, y2, width); break;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_line(x1, y1, x2, y2, width); break;
#endif
  }
}

void md_draw_line_blue(int x1, int y1, int x2, int y2, int width)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_line_blue(x1, y1, x2, y2, width); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_line_blue(x1, y1, x2, y2, width); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_line(x1, y1, x2, y2, width); break;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_line(x1, y1, x2, y2, width); break;
#endif
  }
}

void md_draw_line_green(int x1, int y1, int x2, int y2, int width)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_line_green(x1, y1, x2, y2, width); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_line_green(x1, y1, x2, y2, width); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_line(x1, y1, x2, y2, width); break;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_line(x1, y1, x2, y2, width); break;
#endif
  }
}

void md_draw_line_gray(int x1, int y1, int x2, int y2, int width)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK:
				mdgtk_draw_line_gray(x1, y1, x2, y2, width); 
				break;
#endif
#ifdef GDGUI
    case MDTERM_GD:
				mdgd_draw_line_gray(x1, y1, x2, y2, width);
			  break;
#endif
#ifdef PSGUI
    case MDTERM_PS: 
				mdps_draw_line(x1, y1, x2, y2, width); 
				break;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_line(x1, y1, x2, y2, width); break;
#endif
  }
}


void md_draw_string(int x, int y, char *str)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_string(x, y, str); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_string(x, y, str); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_string(x, y, str); break;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_string(x, y, str); break;
#endif
  }
}

void md_draw_string_red(int x, int y, char *str)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_string_red(x, y, str); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_string_red(x, y, str); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_string(x, y, str); break;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_string(x, y, str); break;
#endif
  }
}

void md_draw_string_blue(int x, int y, char *str)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_string_blue(x, y, str); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_string_blue(x, y, str); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_string(x, y, str); break;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_string(x, y, str); break;
#endif
  }
}

void md_draw_string_green(int x, int y, char *str)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_string_green(x, y, str); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_string_green(x, y, str); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_string(x, y, str); break;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_string(x, y, str); break;
#endif
  }
}

void md_draw_string_gray(int x, int y, char *str)
{
  switch (get_mdterm())
  {
#ifdef GTKGUI
    case MDTERM_GTK: mdgtk_draw_string_gray(x, y, str); break;
#endif
#ifdef GDGUI
    case MDTERM_GD: mdgd_draw_string_gray(x, y, str); break;
#endif
#ifdef PSGUI
    case MDTERM_PS: mdps_draw_string(x, y, str); break;
#endif
#ifdef PLTGUI
    case MDTERM_PLT: mdplt_draw_string(x, y, str); break;
#endif
  }
}

/* end of "universal drawing functions" ########################### */

/* ------------------------------------------ */

double md_get_pho(int x0, int y0, int x1, int y1)
{
  if ((x1-x0) == 0)
  {
    if ((y1-y0) > 0) { return(PI/2) ;  }
    else             { return(-PI/2) ; }
  }
  else
  {
    if ( (((x1-x0)<0.0)&&((y1-y0)>=0.0)) || (((x1-x0)<0.0)&&((y1-y0)<0.0)) )
    {
      return(PI+atan((double)(y1-y0)/(double)(x1-x0)));
    }
    else
    {
      return(atan((double)(y1-y0)/(double)(x1-x0)));
    }
  }
}

int md_rot_x(int x0, double pho, int x, int y)
{
  return((int)x0 + (int)( x*cos(pho)+ y*sin(pho) )) ;
}

int md_rot_y(int y0, double pho, int x, int y)
{
  return((int)y0 - (int)( -x*sin(pho)+ y*cos(pho) )) ;
}

int md_prep_draw(void)
{
  int maxSize = 0 ;

  /* size of canvas:*/
  md_get_size(&mdX0, &mdY0, &mdWidth, &mdHeight) ;

  /* compute parameters for size of loads and direct displacements */
  if (mdWidth > mdHeight) { maxSize = mdWidth ; }
  else                    { maxSize = mdHeight ; }
#if 1
  maxFMi = (int) (maxSize / 12) ;
  maxDRi = (int) (maxSize / 12) ;
#else
  maxFMi = gridSpace ;
  maxDRi = gridSpace ;
#endif

  return(OK);
}

#ifdef POSIX
int md_check_odd(int i, int j)
{
  div_t val ;

  val = div(i+1, 2) ;
  if (val.rem == 0)
  {
    val = div(j+1, 2) ;
    if (val.rem == 0)
    {
      return(1);
    }
  }
  
  return(0);
}
#endif

void md_plot_cross(void)
{
  md_draw_line_gray(
			gfx_pos_x(0),
			gfx_pos_y(0),
			gfx_pos_x(2*gridReal), 
			gfx_pos_y(0),
			1);
 md_draw_line_gray(
			gfx_pos_x(0),
			gfx_pos_y(gridReal/3),
			gfx_pos_x(2*gridReal), 
			gfx_pos_y(0),
			1);
    md_draw_string_gray(
			gfx_pos_x(2*gridReal)+MDGFX_LEN, 
			gfx_pos_y(0)-MDGFX_BLEN2,
            "X" ) ;

  md_draw_line_gray(
			gfx_pos_x(0),
			gfx_pos_y(0),
			gfx_pos_x(0),
			gfx_pos_y(2*gridReal), 
			1);
 md_draw_line_gray(
			gfx_pos_x(0),
			gfx_pos_y(2*gridReal),
			gfx_pos_x(gridReal/3), 
			gfx_pos_y(0),
			1);
    md_draw_string_gray(
			gfx_pos_x(0)+MDGFX_LEN, 
			gfx_pos_y(2*gridReal)-MDGFX_BLEN2,
            "Y" ) ;


}

void md_plot_grid(void)
{
  int i,j;

  md_plot_cross();

  for (i=0; i<=(int)((mdWidth-8-mdX0)/gridSpace); i++)
  {
    for (j=0; j<=(int)((mdHeight-8+mdY0)/gridSpace)+1; j++)
    {
#ifdef POSIX
      if (md_check_odd(i,j) == 1)
      {
        md_draw_big_point(
          gfx_pos_x((double)i*gridReal), 
          gfx_pos_y((double)j*gridReal)
          ) ;
      }
      else
      {
#endif
        md_draw_point(
          gfx_pos_x((double)i*gridReal), 
          gfx_pos_y((double)j*gridReal)
          ) ;
#ifdef POSIX
      }
#endif

#if 0
      printf("P %e %e %i %i (%e)\n",
          ((double)i*gridReal), 
          ((double)j*gridReal),
          gfx_pos_x((double)i*gridReal), 
          gfx_pos_y((double)j*gridReal),
          gridReal
          );
#endif
    }
  }
}

/* Plotting functions: #############################################*/


void plot_active_point(int x, int y)
{
#if 0 /* it's too visible */
  md_draw_line_red(x,y, x-MDGFX_LEN, y-MDGFX_LEN,1) ;
  md_draw_line_red(x,y, x-MDGFX_LEN, y+MDGFX_LEN,1) ;
  md_draw_line_red(x,y, x+MDGFX_LEN, y-MDGFX_LEN,1) ;
  md_draw_line_red(x,y, x+MDGFX_LEN, y+MDGFX_LEN,1) ;
#endif

  md_draw_line_red(x-MDGFX_LEN,y-MDGFX_LEN, x-MDGFX_LEN, y+MDGFX_LEN,1) ;
  md_draw_line_red(x-MDGFX_LEN,y+MDGFX_LEN, x+MDGFX_LEN, y+MDGFX_LEN,1) ;
  md_draw_line_red(x+MDGFX_LEN,y+MDGFX_LEN, x+MDGFX_LEN, y-MDGFX_LEN,1) ;
  md_draw_line_red(x+MDGFX_LEN,y-MDGFX_LEN, x-MDGFX_LEN, y-MDGFX_LEN,1) ;
}

void plot_node(int x, int y)
{
  md_draw_line_red(x,y, x-MDGFX_SLEN, y,2) ;
  md_draw_line_red(x,y, x+MDGFX_SLEN, y,2) ;
  md_draw_line_red(x,y, x, y-MDGFX_SLEN,2) ;
  md_draw_line_red(x,y, x, y+MDGFX_SLEN,2) ;
}

/* plots all nodes */
void plot_nodes(int active)
{
  int i ;

  for (i=0; i<n_len; i++)
  {
    if (active == 0)
    {
      plot_node(
        gfx_pos_x(GET_NODE_X(i)),
        gfx_pos_y(GET_NODE_Y(i))) ;

      if (nodeNumbers == 1)
      {
        md_draw_string_green(  
          gfx_pos_x(GET_NODE_X(i)),
          gfx_pos_y(GET_NODE_Y(i)),
          md_intstring(GET_NODE_ID(i)) ) ;
      }
    }
    else
    {
      plot_active_point(gfx_pos_x(GET_NODE_X(i)),
        gfx_pos_y(GET_NODE_Y(i))) ;
    }
  }
}

void plot_elem(int x1,
               int y1,
               int x2,
               int y2,
               int type,
               int active)
{
  double pho, L ;

  pho = md_get_pho(x1, y1, x2, y2);
  L = sqrt ((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1)) ;

  if (mdSimpleE != 1) /* underlined element */
  {
    md_draw_line(
        md_rot_x(x1,pho,0,-MDGFX_LEN),
        md_rot_y(y1, pho,0,-MDGFX_LEN),
        md_rot_x(x1, pho,(int)L,-MDGFX_LEN),
        md_rot_y(y1, pho,(int)L,-MDGFX_LEN),
        1) ;
  }

  if (type == 0)
  {
    md_draw_line(x1,y1, x2, y2, 2) ;
  }
  else
  {
    /* some circle magic should be here */
    if (type == 1)
    {
      /* o---| */
      md_draw_line(
          md_rot_x(x1,pho,+MDGFX_BLEN,0),
          md_rot_y(y1, pho,+MDGFX_BLEN,0),
          x2, y2, 2) ;

      md_draw_line(
          x1,y1,
          md_rot_x(x1,pho,MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_y(y1,pho,MDGFX_BLEN2,MDGFX_BLEN2),
           2) ;
      md_draw_line(
          x1,y1,
          md_rot_x(x1,pho,MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_y(y1,pho,MDGFX_BLEN2,-MDGFX_BLEN2),
           2) ;
      md_draw_line(
          md_rot_x(x1,pho,MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_y(y1,pho,MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_x(x1,pho,MDGFX_BLEN,0),
          md_rot_y(y1,pho,MDGFX_BLEN,0),
           2) ;
      md_draw_line(
          md_rot_x(x1,pho,MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_y(y1,pho,MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_x(x1,pho,MDGFX_BLEN,0),
          md_rot_y(y1,pho,MDGFX_BLEN,0),
           2) ;
    }
    else
    {
      if (type == 2)
      {
      /* |---o */
      md_draw_line(x1,y1,
          md_rot_x(x2,pho,-MDGFX_BLEN,0),
          md_rot_y(y2, pho,-MDGFX_BLEN,0),
          2) ;

      md_draw_line(
          x2,y2,
          md_rot_x(x2,pho,-MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_y(y2,pho,-MDGFX_BLEN2,MDGFX_BLEN2),
           2) ;
      md_draw_line(
          x2,y2,
          md_rot_x(x2,pho,-MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_y(y2,pho,-MDGFX_BLEN2,-MDGFX_BLEN2),
           2) ;
      md_draw_line(
          md_rot_x(x2,pho,-MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_y(y2,pho,-MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_x(x2,pho,-MDGFX_BLEN,0),
          md_rot_y(y2,pho,-MDGFX_BLEN,0),
           2) ;
      md_draw_line(
          md_rot_x(x2,pho,-MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_y(y2,pho,-MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_x(x2,pho,-MDGFX_BLEN,0),
          md_rot_y(y2,pho,-MDGFX_BLEN,0),
           2) ;
      }
      else
      {
        /* o--o */
          md_draw_line(
          md_rot_x(x1,pho,+MDGFX_BLEN,0),
          md_rot_y(y1, pho,+MDGFX_BLEN,0),
          x2, y2, 2) ;

      md_draw_line(
          x1,y1,
          md_rot_x(x1,pho,MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_y(y1,pho,MDGFX_BLEN2,MDGFX_BLEN2),
           2) ;
      md_draw_line(
          x1,y1,
          md_rot_x(x1,pho,MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_y(y1,pho,MDGFX_BLEN2,-MDGFX_BLEN2),
           2) ;
      md_draw_line(
          md_rot_x(x1,pho,MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_y(y1,pho,MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_x(x1,pho,MDGFX_BLEN,0),
          md_rot_y(y1,pho,MDGFX_BLEN,0),
           2) ;
      md_draw_line(
          md_rot_x(x1,pho,MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_y(y1,pho,MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_x(x1,pho,MDGFX_BLEN,0),
          md_rot_y(y1,pho,MDGFX_BLEN,0),
           2) ;

      md_draw_line(x1,y1,
          md_rot_x(x2,pho,-MDGFX_BLEN,0),
          md_rot_y(y2, pho,-MDGFX_BLEN,0),
          2) ;

      md_draw_line(
          x2,y2,
          md_rot_x(x2,pho,-MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_y(y2,pho,-MDGFX_BLEN2,MDGFX_BLEN2),
           2) ;
      md_draw_line(
          x2,y2,
          md_rot_x(x2,pho,-MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_y(y2,pho,-MDGFX_BLEN2,-MDGFX_BLEN2),
           2) ;
      md_draw_line(
          md_rot_x(x2,pho,-MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_y(y2,pho,-MDGFX_BLEN2,-MDGFX_BLEN2),
          md_rot_x(x2,pho,-MDGFX_BLEN,0),
          md_rot_y(y2,pho,-MDGFX_BLEN,0),
           2) ;
      md_draw_line(
          md_rot_x(x2,pho,-MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_y(y2,pho,-MDGFX_BLEN2,MDGFX_BLEN2),
          md_rot_x(x2,pho,-MDGFX_BLEN,0),
          md_rot_y(y2,pho,-MDGFX_BLEN,0),
           2) ;
      }
    }
  }

  if (active == 1)
  {
    plot_active_point((int) ((x1+x2)/2), (int) ((y1+y2)/2)) ;
  }
}


/* plots all elements */
void plot_elems(int active)
{
  int i ;

  for (i=0; i<e_len; i++)
  {
    plot_elem(
        gfx_pos_x(GET_NODE_X(GET_ELEM_N1(i))),
        gfx_pos_y(GET_NODE_Y(GET_ELEM_N1(i))),
        gfx_pos_x(GET_NODE_X(GET_ELEM_N2(i))),
        gfx_pos_y(GET_NODE_Y(GET_ELEM_N2(i))),
               GET_ELEM_TYPE(i),
               active) ;
    if (elemNumbers == 1)
    {
      md_draw_string_gray(
          (int) ((gfx_pos_x(GET_NODE_X(GET_ELEM_N1(i)))+gfx_pos_x(GET_NODE_X(GET_ELEM_N2(i))))/2), 
          (int) ((gfx_pos_y(GET_NODE_Y(GET_ELEM_N1(i)))+gfx_pos_y(GET_NODE_Y(GET_ELEM_N2(i))))/2), 
          md_intstring(GET_ELEM_ID(i)) ) ;
    }
  }
}



void plot_disp_ux(int x, int y)
{
  md_draw_line_blue(x,y, x-MDGFX_BLEN, y+MDGFX_BLEN2, 1) ;
  md_draw_line_blue(x,y, x-MDGFX_BLEN, y-MDGFX_BLEN2, 1) ;
  md_draw_line_blue(x-MDGFX_BLEN,y+MDGFX_BLEN2, x-MDGFX_BLEN, y-MDGFX_BLEN2, 1) ;
  md_draw_line_blue(x-MDGFX_BLENB,y+MDGFX_BLEN2, x-MDGFX_BLENB, y-MDGFX_BLEN2, 1) ;
}

void plot_disp_uy(int x, int y)
{
  md_draw_line_blue(x,y, x-MDGFX_BLEN2, y+MDGFX_BLEN, 1) ;
  md_draw_line_blue(x,y, x+MDGFX_BLEN2, y+MDGFX_BLEN, 1) ;
  md_draw_line_blue(x-MDGFX_BLEN2,y+MDGFX_BLEN, x+MDGFX_BLEN2, y+MDGFX_BLEN, 1) ;
  md_draw_line_blue(x-MDGFX_BLEN2,y+MDGFX_BLENB, x+MDGFX_BLEN2, y+MDGFX_BLENB, 1) ;
}

void plot_disp_uxuy(int x, int y)
{
  md_draw_line_blue(x,y, x-MDGFX_BLEN2, y+MDGFX_BLEN, 1) ;
  md_draw_line_blue(x,y, x+MDGFX_BLEN2, y+MDGFX_BLEN, 1) ;
  md_draw_line_blue(x-MDGFX_BLEN2,y+MDGFX_BLEN, x+MDGFX_BLEN2, y+MDGFX_BLEN, 1) ;
}

void plot_disp_uxuyrotz(int x, int y)
{
  md_draw_line_blue(x-MDGFX_BLEN2,y-MDGFX_BLEN2, x+MDGFX_BLEN2, y-MDGFX_BLEN2, 1) ;
  md_draw_line_blue(x-MDGFX_BLEN2,y+MDGFX_BLEN2, x+MDGFX_BLEN2, y+MDGFX_BLEN2, 1) ;

  md_draw_line_blue(x-MDGFX_BLEN2,y-MDGFX_BLEN2, x-MDGFX_BLEN2, y+MDGFX_BLEN2, 1) ;
  md_draw_line_blue(x+MDGFX_BLEN2,y-MDGFX_BLEN2, x+MDGFX_BLEN2, y+MDGFX_BLEN2, 1) ;
}


void plot_disps(void)
{
	int i ;

	for (i=0; i<n_len; i++)
	{
		switch(GET_NODE_TYPE(i))
		{
			case 1: plot_disp_ux(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i))); break ;
			case 2: plot_disp_uy(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i))) ; break ;
			case 3: plot_disp_uxuy(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i))) ; break ;
			case 6: plot_disp_uxuyrotz(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i))) ; break ;
		}
	}
}

/* force --> (+) or <-- (-) */
void plot_fx(int x, int y, double size)
{
  if (size == 0.0) {return;} /* nothing to do */

  if (size > 0.0)
  {
    /* --> */
    md_draw_line_blue(x,y, x-MDGFX_BLEN2, y-MDGFX_BLEN2, 3) ;
    md_draw_line_blue(x,y, x-MDGFX_BLEN2, y+MDGFX_BLEN2, 3) ;

    md_draw_line_blue(x,y, x-maxFMi, y, 3) ;

    md_draw_string_blue(x-maxFMi,y-MDGFX_LEN, md_double2string01(fabs(size)) ) ;
  }
  else
  {
    /* <-- */
    md_draw_line_blue(x,y, x+MDGFX_BLEN2, y-MDGFX_BLEN2, 3) ;
    md_draw_line_blue(x,y, x+MDGFX_BLEN2, y+MDGFX_BLEN2, 3) ;

    md_draw_line_blue(x,y, x+maxFMi, y, 3) ;

    md_draw_string_blue(x+maxFMi,y-MDGFX_LEN, md_double2string01(fabs(size)) ) ;
  }
}

/* force /|\ (+) or \|/ (-) */
void plot_fy(int x, int y, double size)
{
  if (size == 0.0) {return;} /* nothing to do */

  if (size > 0.0)
  {
    /* /|\ */
    md_draw_line_red(x,y, x-MDGFX_BLEN2, y+MDGFX_BLEN2, 3) ;
    md_draw_line_red(x,y, x+MDGFX_BLEN2, y+MDGFX_BLEN2, 3) ;

    md_draw_line_red(x,y, x, y+maxFMi, 3) ;

    md_draw_string_red(x+MDGFX_LEN,y+maxFMi, md_double2string01(fabs(size)) ) ;
  }
  else
  {
    /* \|/ */
    md_draw_line_red(x,y, x-MDGFX_BLEN2, y-MDGFX_BLEN2, 3) ;
    md_draw_line_red(x,y, x+MDGFX_BLEN2, y-MDGFX_BLEN2, 3) ;

    md_draw_line_red(x,y, x, y-maxFMi, 3) ;

    md_draw_string_red(x+MDGFX_LEN,y-maxFMi, md_double2string01(fabs(size)) ) ;
  }
}

/* moment */
void plot_mz(int x, int y, double size)
{
  int i ;
  int x0, y0, x1, y1 ;

  if (size == 0.0) {return;} /* nothing to do */

  x0 = x-(int)(0.5*maxFMi) ;
  y0 = y ;

  for (i=0; i<=16; i++)
  {
    x1 = x + (int) (-0.5*maxFMi*cos((double)(i*PI)/16)) ;
    y1 = y + (int) (-0.5*maxFMi*sin((double)(i*PI)/16)) ;
    md_draw_line(x0,y0, x1,y1,2) ;
    x0 = x1 ;
    y0 = y1 ;
  }

  if (size > 0.0)
  {
    md_draw_line_red(x+(int)(0.5*maxFMi),y, x+(int)(0.5*maxFMi)+MDGFX_BLEN2, y-MDGFX_BLEN2, 3) ;
    md_draw_line_red(x+(int)(0.5*maxFMi),y, x+(int)(0.5*maxFMi)-MDGFX_BLEN2, y-MDGFX_BLEN2, 3) ;
  }
  else
  {
    md_draw_line_red(x-(int)(0.5*maxFMi),y, x-(int)(0.5*maxFMi)-MDGFX_BLEN2, y-MDGFX_BLEN2, 3) ;
    md_draw_line_red(x-(int)(0.5*maxFMi),y, x-(int)(0.5*maxFMi)+MDGFX_BLEN2, y-MDGFX_BLEN2, 3) ;
  }
    md_draw_string_red(x+MDGFX_LEN,y-(int)(0.5*maxFMi), md_double2string01(fabs(size)) ) ;
}

void plot_forces(void)
{
	int i ;

	for (i=0; i<n_len; i++)
	{
		plot_fx(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NODE_FX(i));
		plot_fy(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NODE_FY(i));
		plot_mz(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NODE_MZ(i));
  }
}



void plot_elem_load_n(int x1,
                      int y1,
                      int x2,
                      int y2,
                      double n1,
                      double n2)
{
  double pho, L ;
  int na = 0 ;
  int nb = 0 ;

#if 1
  if (n1 != 0)
  {
    na = (int)(0.5*maxFMi) ;
    if (n1 < 0.0) {na *= (-1) ;}
  }

  if (n2 != 0)
  {
    nb = (int)(0.5*maxFMi) ;
    if (n2 < 0.0) {nb *= (-1) ;}
  }

  if (fabs(n1) < fabs(n2))
  {
    na = (int)(na / 2) ;
  }

  if (fabs(n1) > fabs(n2))
  {
    nb = (int)(nb / 2) ;
  }


  pho = md_get_pho(x1, y1, x2, y2);
  L = sqrt ((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1)) ;


  md_draw_line_blue(
      md_rot_x(x1,pho,0,0),
      md_rot_y(y1, pho,0,0),
      md_rot_x(x1, pho,0,na),
      md_rot_y(y1, pho,0,na),
      1) ;

  md_draw_line_blue(
      md_rot_x(x1,pho,0,na),
      md_rot_y(y1, pho,0,na),
      md_rot_x(x1, pho,(int)L,nb),
      md_rot_y(y1, pho,(int)L,nb),
      1) ;

  md_draw_line_blue(
      md_rot_x(x1,pho,(int)L,0),
      md_rot_y(y1, pho,(int)L,0),
      md_rot_x(x1, pho,(int)L,nb),
      md_rot_y(y1, pho,(int)L,nb),
      1) ;

    /* some arrows to show direction */
    if (n1 > 0)
    {
    md_draw_line_blue(
          md_rot_x(x1,pho,0,0),
          md_rot_y(y1,pho,0,0),
          md_rot_x(x1,pho,+MDGFX_BLEN,(int)(+ na/2)),
          md_rot_y(y1,pho,+MDGFX_BLEN,(int)(+ na/2)),
          1) ;
    md_draw_line_blue(
          md_rot_x(x1,pho,0,0),
          md_rot_y(y1,pho,0,0),
          md_rot_x(x1,pho,+MDGFX_BLEN,(int)(- na/2)),
          md_rot_y(y1,pho,+MDGFX_BLEN,(int)(- na/2)),
          1) ;
    }
    else
    {
    md_draw_line_blue(
          md_rot_x(x1,pho,MDGFX_BLEN,0),
          md_rot_y(y1,pho,MDGFX_BLEN,0),
          md_rot_x(x1,pho,0,(int)(+ na/2)),
          md_rot_y(y1,pho,0,(int)(+ na/2)),
          1) ;
    md_draw_line_blue(
          md_rot_x(x1,pho,MDGFX_BLEN,0),
          md_rot_y(y1,pho,MDGFX_BLEN,0),
          md_rot_x(x1,pho,0,(int)(- na/2)),
          md_rot_y(y1,pho,0,(int)(- na/2)),
          1) ;
    }

    if (n2 > 0)
    {
    md_draw_line_blue(
          md_rot_x(x1,pho,L-MDGFX_BLEN,0),
          md_rot_y(y1,pho,L-MDGFX_BLEN,0),
          md_rot_x(x1,pho,+L,(int)(+ nb/2)),
          md_rot_y(y1,pho,+L,(int)(+ nb/2)),
          1) ;
    md_draw_line_blue(
          md_rot_x(x1,pho,L-MDGFX_BLEN,0),
          md_rot_y(y1,pho,L-MDGFX_BLEN,0),
          md_rot_x(x1,pho,+L,(int)(- nb/2)),
          md_rot_y(y1,pho,+L,(int)(- nb/2)),
          1) ;
    }
    else
    {
    md_draw_line_blue(
          md_rot_x(x1,pho,L,0),
          md_rot_y(y1,pho,L,0),
          md_rot_x(x1,pho,L-MDGFX_BLEN,(int)(+ nb/2)),
          md_rot_y(y1,pho,L-MDGFX_BLEN,(int)(+ nb/2)),
          1) ;
    md_draw_line_blue(
          md_rot_x(x1,pho,L,0),
          md_rot_y(y1,pho,L,0),
          md_rot_x(x1,pho,L-MDGFX_BLEN,(int)(- nb/2)),
          md_rot_y(y1,pho,L-MDGFX_BLEN,(int)(- nb/2)),
          1) ;
    }

    if (n1 == n2)
    {
      md_draw_string_blue(
          md_rot_x(x1,pho,+(int)(0.5*L),+(int)(1.5*nb)),
          md_rot_y(y1,pho,+(int)(0.5*L),+(int)(1.5*nb)), 
          md_double2string01(fabs(n1)) ) ;
    }
    else
    {
      if (n1 != 0)
      {
      md_draw_string_blue(
          md_rot_x(x1,pho,0,+(int)(1.5*na)),
          md_rot_y(y1,pho,0,+(int)(1.5*na)), 
          md_double2string01(fabs(n1)) ) ;
      }
      if (n2 != 0)
      {
      md_draw_string_blue(
          md_rot_x(x1,pho,L,+(int)(1.5*nb)),
          md_rot_y(y1,pho,L,+(int)(1.5*nb)), 
          md_double2string01(fabs(n2)) ) ;
      }
    }
#endif
}

void plot_elem_load_v(int x1,
                      int y1,
                      int x2,
                      int y2,
                      double n1,
                      double n2)
{
  double pho, L ;
  int na = 0 ;
  int nb = 0 ;

  if (n1 != 0)
  {
    na = (int)(0.5*maxFMi) ;
    if (n1 < 0) {na *= (-1) ;}
  }

  if (n2 != 0)
  {
    nb = (int)(0.5*maxFMi) ;
    if (n2 < 0) {nb *= (-1) ;}
  }

  if (fabs(n1) < fabs(n2))
  {
    na = (int)(na / 2) ;
  }

  if (fabs(n1) > fabs(n2))
  {
    nb = (int)(nb / 2) ;
  }


  pho = md_get_pho(x1, y1, x2, y2);
  L = sqrt ((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1)) ;


  md_draw_line_red(
      md_rot_x(x1,pho,0,0),
      md_rot_y(y1, pho,0,0),
      md_rot_x(x1, pho,0,na),
      md_rot_y(y1, pho,0,na),
      1) ;

  md_draw_line_red(
      md_rot_x(x1,pho,0,na),
      md_rot_y(y1, pho,0,na),
      md_rot_x(x1, pho,(int)L,nb),
      md_rot_y(y1, pho,(int)L,nb),
      1) ;

  md_draw_line_red(
      md_rot_x(x1,pho,(int)L,0),
      md_rot_y(y1, pho,(int)L,0),
      md_rot_x(x1, pho,(int)L,nb),
      md_rot_y(y1, pho,(int)L,nb),
      1) ;

    /* some arrows to show direction */
    md_draw_line_red(
          md_rot_x(x1,pho,0,0),
          md_rot_y(y1,pho,0,0),
          md_rot_x(x1,pho,-MDGFX_LEN,+(int)(0.5*na)), 
          md_rot_y(y1,pho,-MDGFX_LEN,+(int)(0.5*na)), 
          1) ;
    md_draw_line_red(
          md_rot_x(x1,pho,0,0),
          md_rot_y(y1,pho,0,0), 
          md_rot_x(x1,pho,+MDGFX_LEN,+(int)(0.5*na)), 
          md_rot_y(y1,pho,+MDGFX_LEN,+(int)(0.5*na)), 
          1) ;

    md_draw_line_red(
          md_rot_x(x1,pho,L,0),
          md_rot_y(y1,pho,L,0),
          md_rot_x(x1,pho,L-MDGFX_LEN,+(int)(0.5*nb)), 
          md_rot_y(y1,pho,L-MDGFX_LEN,+(int)(0.5*nb)), 
          1) ;
    md_draw_line_red(
          md_rot_x(x1,pho,L,0),
          md_rot_y(y1,pho,L,0), 
          md_rot_x(x1,pho,L+MDGFX_LEN,+(int)(0.5*nb)), 
          md_rot_y(y1,pho,L+MDGFX_LEN,+(int)(0.5*nb)), 
          1) ;


    if (n1 == n2)
    {
      md_draw_string_red(
          md_rot_x(x1,pho,+(int)(0.5*L),+(int)(1.5*nb)),
          md_rot_y(y1,pho,+(int)(0.5*L),+(int)(1.5*nb)), 
          md_double2string01(fabs(n1)) ) ;
    }
    else
    {
      if (n1 != 0)
      {
      md_draw_string_red(
          md_rot_x(x1,pho,0,+(int)(1.5*na)),
          md_rot_y(y1,pho,0,+(int)(1.5*na)), 
          md_double2string01(fabs(n1)) ) ;
      }
      if (n2 != 0)
      {
      md_draw_string_red(
          md_rot_x(x1,pho,L,+(int)(1.5*nb)),
          md_rot_y(y1,pho,L,+(int)(1.5*nb)), 
          md_double2string01(fabs(n2)) ) ;
      }
    }
}

void plot_elem_loads(void)
{
  int i ;

  for (i=0; i<e_len; i++)
  {
    if ((GET_ELEM_NA(i) != 0) || (GET_ELEM_NB(i) != 0) )
    {
      plot_elem_load_n(
        gfx_pos_x(GET_NODE_X(GET_ELEM_N1(i))),
        gfx_pos_y(GET_NODE_Y(GET_ELEM_N1(i))),
        gfx_pos_x(GET_NODE_X(GET_ELEM_N2(i))),
        gfx_pos_y(GET_NODE_Y(GET_ELEM_N2(i))),
        (GET_ELEM_NA(i)),
        (GET_ELEM_NB(i))
               ) ;
    }

    if ((GET_ELEM_VA(i) != 0) || (GET_ELEM_VB(i) != 0) )
    {
      plot_elem_load_v(
        gfx_pos_x(GET_NODE_X(GET_ELEM_N1(i))),
        gfx_pos_y(GET_NODE_Y(GET_ELEM_N1(i))),
        gfx_pos_x(GET_NODE_X(GET_ELEM_N2(i))),
        gfx_pos_y(GET_NODE_Y(GET_ELEM_N2(i))),
        (GET_ELEM_VA(i)),
        (GET_ELEM_VB(i))
               ) ;
    }
  }
}

void plot_elem_def(int e_num, double mult)
{
  int n1,n2,i;
  double y,y0 ;
  double x1,y1,x2,y2, L, dL, x, EI, pho, dx1,dy1,dx2, dy2 ;

  EI = GET_ELEM_E(e_num)*GET_ELEM_I(e_num);

  n1 = GET_ELEM_N1(e_num) ;
  n2 = GET_ELEM_N2(e_num) ;


  x1 = (GET_NODE_X(n1));
  y1 = (GET_NODE_Y(n1));
  x2 = (GET_NODE_X(n2));
  y2 = (GET_NODE_Y(n2));

  L = sqrt ((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)); 

  /* first we have to plot the undeformed structure: */
  md_draw_line_gray(
      gfx_pos_x(x1),
      gfx_pos_y(y1),
      gfx_pos_x(x2),
      gfx_pos_y(y2), 1 );

  dx1 = gfx_pos_x(GET_NODE_X(GET_ELEM_N1(e_num))+mult*GET_NRES_POSX(n1)) ;
  dy1 = gfx_pos_y(GET_NODE_Y(GET_ELEM_N1(e_num))+mult*GET_NRES_POSY(n1)) ;
  dx2 = gfx_pos_x(GET_NODE_X(GET_ELEM_N2(e_num))+mult*GET_NRES_POSX(n2)) ;
  dy2 = gfx_pos_y(GET_NODE_Y(GET_ELEM_N2(e_num))+mult*GET_NRES_POSY(n2)) ;

 /* plot a deformed structure: */
 pho = md_get_pho(dx1, dy1, dx2, dy2);
 dL = sqrt ((dx2-dx1)*(dx2-dx1) + (dy2-dy1)*(dy2-dy1)); 

 for (i=0; i<=num_div; i++)
 {
   x = ((double)i/(double)num_div)*L ;

   y=  -(mult*(md_compute_e_def_y(
      GET_ELEM_TYPE(e_num), 
      L, 
      GET_ELEM_VA(e_num), 
      GET_ELEM_VB(e_num), 
      x) / EI )) ;

   y0 = y ;

    if (i > 0)
    {
      md_draw_line_blue(
        md_rot_x(dx1, pho,dL*(i-1)/num_div,y0),
        md_rot_y(dy1, pho,dL*(i-1)/num_div,y0),
        md_rot_x(dx1, pho,dL*i/num_div,y),
        md_rot_y(dy1, pho,dL*i/num_div,y),
        2) ;
    }

    y0 = y ;
  }

}


/* plots deformed structure */
void plot_elem_defs(void)
{
  int i, j,  n1, n2 ;
  double max_def = 0 ;
  double max_xy = 0 ;
  double min_x = 0 ;
  double min_y = 0 ;
  double mult = 1 ;
  double x1,y1,x2,y2, L, x, EI, y;

  /* get multiplier value: */


  for (i=0; i<e_len; i++)
  {
    n1 = GET_ELEM_N1(i) ;
    n2 = GET_ELEM_N2(i) ;

    if (i == 0) {min_x = GET_NODE_X(n1);}
    if (i == 0) {min_y = GET_NODE_Y(n1);}
    
    if (GET_NODE_X(n1) < min_x) {min_x = GET_NODE_X(n1);}
    if (GET_NODE_Y(n1) < min_y) {min_y = GET_NODE_Y(n1);}
    if (GET_NODE_X(n2) < min_x) {min_x = GET_NODE_X(n2);}
    if (GET_NODE_Y(n2) < min_y) {min_y = GET_NODE_Y(n2);}
  }

  for (i=0; i<e_len; i++)
  {
    n1 = GET_ELEM_N1(i) ;
    n2 = GET_ELEM_N2(i) ;
    
    if ((GET_NODE_X(n1)-min_x) > max_xy) {max_xy = GET_NODE_X(n1)-min_x;}
    if ((GET_NODE_Y(n1)-min_y) > max_xy) {max_xy = GET_NODE_Y(n1)-min_y;}
    if ((GET_NODE_X(n2)-min_x) > max_xy) {max_xy = GET_NODE_X(n2)-min_x;}
    if ((GET_NODE_Y(n2)-min_y) > max_xy) {max_xy = GET_NODE_Y(n2)-min_y;}

    if (fabs(GET_NRES_POSX(n1)) > max_def) {max_def = fabs(GET_NRES_POSX(n1));}
    if (fabs(GET_NRES_POSY(n1)) > max_def) {max_def = fabs(GET_NRES_POSY(n1));}
    if (fabs(GET_NRES_POSX(n2)) > max_def) {max_def = fabs(GET_NRES_POSX(n2));}
    if (fabs(GET_NRES_POSY(n2)) > max_def) {max_def = fabs(GET_NRES_POSY(n2));}

    /* ---------------------- */
#if 1
    EI = GET_ELEM_E(i)*GET_ELEM_I(i);

    n1 = GET_ELEM_N1(i) ;
    n2 = GET_ELEM_N2(i) ;

    x1 = (GET_NODE_X(n1));
    y1 = (GET_NODE_Y(n1));
    x2 = (GET_NODE_X(n2));
    y2 = (GET_NODE_Y(n2));

    L = sqrt ((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)); 

    for (j=0; j<=num_div; j++)
    {
      x = ((double)j/(double)num_div)*L ;
      y =  fabs((md_compute_e_def_y(
          GET_ELEM_TYPE(i), 
          L, 
          GET_ELEM_VA(i), 
          GET_ELEM_VB(i), 
          x) / EI )) ;

      if (y > max_def) {max_def = y ;}
    }
#endif

    /* ---------------------- */
  }

  if (max_def > 0)
  {
#if 1
    mult = (max_xy) / (10 * max_def) ;
#else
    mult = mdWidth/(30*max_def) ;
#endif
  }
  else
  {
    mult = 1; /* 0 should be OK too */
  }

  /*printf("max_xy = %e, max_def = %e, mult = %e\n",max_xy, max_def,mult);*/

  /* plot deformed elements: */
  for (i=0; i<e_len; i++)
  {
    plot_elem_def(i, mult);
  }
}



/* computes multiplier for N, V or M data (brute force way) */
double plot_NVM_mult(int type, int ndiv)
{
  int i,j ;
  double val ;
  double max = 0 ;

  for (i=0; i<e_len; i++)
  {
    for (j=0; j<ndiv; j++)
    {
      val = fabs(compute_e_res(type, i, ndiv, j)) ;
      if (val > max) {max = val ;}
    }
  }

  if (max == 0.0)
  {
    return(0.0);
  }
  else
  {
    return(((double)(1.0*(double)maxFMi))/max);
  }
}

void plot_elem_result(int type, int epos)
{
  double x1,x2,y1,y2, pho, L ;
  double last_y, act_y, last_x, act_x ;
  int    i ;
  int    posmin, posmax ;

  if (resMult == 0.0) {resMult = 1.0 ;}

  /* plot values */
  if (resExtr == 1)
  {
    get_max_NVM_res(type, epos, num_div, &posmax);
    get_min_NVM_res(type, epos, num_div, &posmin);
  }


  x1 = gfx_pos_x(GET_NODE_X(GET_ELEM_N1(epos))) ;
  y1 = gfx_pos_y(GET_NODE_Y(GET_ELEM_N1(epos))) ;
  x2 = gfx_pos_x(GET_NODE_X(GET_ELEM_N2(epos))) ;
  y2 = gfx_pos_y(GET_NODE_Y(GET_ELEM_N2(epos))) ;

  pho = md_get_pho(x1, y1, x2, y2);
  L = sqrt ((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1)) ;

  md_draw_line_blue(
      md_rot_x(x1, pho,0,0),
      md_rot_y(y1, pho,0,0),
      md_rot_x(x1, pho,L,0),
      md_rot_y(y1, pho,L,0),
      1) ;

  last_x = 0 ;
  last_y = resMult*compute_e_res(type, epos, num_div, 0) ;
  
  for (i=0; i<= num_div; i++)
  {
    act_x = ((double)i/(double)num_div)*L ;
    act_y = resMult*compute_e_res(type, epos, num_div, i) ;

    if (i == 0)
    {
      md_draw_line_blue(
        md_rot_x(x1,pho,last_x,0),
        md_rot_y(y1, pho,last_x,0),
        md_rot_x(x1, pho,last_x,last_y),
        md_rot_y(y1, pho,last_x,last_y),
      2) ;
    }
    
    md_draw_line_blue(
      md_rot_x(x1,pho,last_x,last_y),
      md_rot_y(y1, pho,last_x,last_y),
      md_rot_x(x1, pho,act_x,act_y),
      md_rot_y(y1, pho,act_x,act_y),
      2) ;

    if (i == num_div)
    {
      md_draw_line_blue(
        md_rot_x(x1,pho,act_x,0),
        md_rot_y(y1, pho,act_x,0),
        md_rot_x(x1, pho,act_x,act_y),
        md_rot_y(y1, pho,act_x,act_y),
      2) ;
    }
    else
    {
      md_draw_line_blue(
        md_rot_x(x1,pho,act_x,0),
        md_rot_y(y1, pho,act_x,0),
        md_rot_x(x1, pho,act_x,act_y),
        md_rot_y(y1, pho,act_x,act_y),
      1) ;
    }

    /* plot values (if asked) */

    if (resEnds == 1)
    {
      if (i == 0)
      {
        md_draw_string(
            md_rot_x(x1, pho,act_x,act_y)+MDGFX_LEN,
            md_rot_y(y1, pho,act_x,act_y)-MDGFX_LEN,
#if 0
            md_double2string01(fabs(act_y/resMult))
#else
            md_double2string01((act_y/resMult))
#endif
 						) ;
      }
      if (i == num_div)
      {
        md_draw_string(
            md_rot_x(x1, pho,act_x,act_y)+MDGFX_LEN,
            md_rot_y(y1, pho,act_x,act_y)-MDGFX_LEN,
#if 0
            md_double2string01(fabs(act_y/resMult))
#else
            md_double2string01((act_y/resMult))
#endif
 						) ;
      }
    }

    if (resExtr == 1)
    {
      if (posmax == i)
      {
        if ((resEnds == 1) && ((posmax == 0)||(posmax==num_div)))
        {
        }
        else
        {
          md_draw_string(
            md_rot_x(x1, pho,act_x,act_y)+MDGFX_LEN,
            md_rot_y(y1, pho,act_x,act_y)-MDGFX_LEN,
            md_double2string01(fabs(act_y/resMult)) ) ;
        }
      }
      if (posmin == i)
      {
        if ((resEnds == 1) && ((posmin == 0)||(posmin==num_div)))
        {
        }
        else
        {
          md_draw_string(
            md_rot_x(x1, pho,act_x,act_y)+MDGFX_LEN,
            md_rot_y(y1, pho,act_x,act_y)-MDGFX_LEN,
            md_double2string01(fabs(act_y/resMult)) ) ;
        }
      }
    }


    /* for next step: */
    last_x = act_x ;
    last_y = act_y ;
  }

}

/* results */

void plot_reactions(void)
{
	int i ;

	for (i=0; i<n_len; i++)
	{
    switch (n_dtype[i])
    {
      case 1: 
		          plot_fx(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_FX(i));
              break ;
      case 2: 
		          plot_fy(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_FY(i));
              break ;
      case 3: 
		          plot_fx(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_FX(i));
		          plot_fy(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_FY(i));
              break ;
      case 4: 
		          plot_fx(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_FX(i));
		          plot_mz(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_MZ(i));
              break ;
      case 5: 
		          plot_fy(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_FY(i));
		          plot_mz(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_MZ(i));
              break ;
      case 6: 
		          plot_fx(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_FX(i));
		          plot_fy(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_FY(i));
		          plot_mz(gfx_pos_x(GET_NODE_X(i)), gfx_pos_y(GET_NODE_Y(i)), GET_NRES_MZ(i));
              break ;
    }
  }
}

void plot_Ns(void) 
{ 
  int i ; 
  resMult = plot_NVM_mult(1, num_div);

  for (i=0; i<e_len; i++) 
  { 
    plot_elem_result(1, i); 
  } 
}

void plot_Vs(void) 
{ 
  int i ; 
  resMult = plot_NVM_mult(2, num_div);

  for (i=0; i<e_len; i++) 
  { 
    plot_elem_result(2, i); 
  } 
}

void plot_Ms(void) 
{ 
  int i ; 
  resMult = plot_NVM_mult(3, num_div);

  for (i=0; i<e_len; i++) 
  { 
    plot_elem_result(3, i); 
  } 
}

/* main drawing routine */
int md_draw(void)
{
  if (md_prep_draw()!=OK){return(ERR_IO);}
  plot_real_limits();

  if (gfxAction < 60)
  {

    if ((mdShowGrid)||(pickMode == 1)) {md_plot_grid() ;}

    if (pickMode == 2) { plot_nodes(1); }
    else               { plot_nodes(0); }

    if (pickMode == 3) { plot_elems(1); }
    else               { plot_elems(0); }

	  plot_disps();

    plot_forces();
  
    plot_elem_loads();
  }
  else
  {
    switch (gfxAction)
    {
      case 60:
#if 0
        plot_nodes(0);
        plot_elems(0);
#endif
        plot_elem_defs();
        plot_reactions(); 
        break ;
      case 61: plot_Ns(); break ;
      case 62: plot_Vs(); break ;
      case 63: plot_Ms(); break ;
    }
  }
  return(OK);
}



/* action from user (menu etc.) */
void md_from_user_action(void)
{
#ifdef GGUI	
  mdgfx_set_input(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  switch (gfxAction)
  {
    case 0 : pickMode = 0 ;
             mdgfx_set_input(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
             break ;
    case 1 : /* add node */
             if (mdText == 1)
             {
              mdgfx_set_input(" Node coordinates ",
                 " x = ",0.0," y = ",0.0,0,0, 0, 0,2);
             }
             else
             {
              mdgfx_set_input(" Pick a Grid Point ",
                 0, 0, 0, 0, 0, 0, 0, 0, 0);
              pickMode = 1 ;
             }
             break ;
    case 2 : /* add |--| element */
             if (mdText == 0)
             {
               pickMode = 2 ;
               mdgfx_set_input(" Element |--| properties ",
               " E = ", defaultE,
               " A = ", defaultA,
               " I = ", defaultI, 0, 0,
               0);
             }
             else
             {
               pickMode = 2 ;
               mdgfx_set_input(" Element |--| properties ",
               " E = ", defaultE,
               " A = ", defaultA,
               " I = ", defaultI, 0, 0,
               1);
             }
             break ;
    case 3 : /* add o--| element */
             pickMode = 2 ;
             mdgfx_set_input(" Element o--| properties ",
             " E = ", defaultE,
             " A = ", defaultA,
             " I = ", defaultI, 0, 0,
             0);
             break ;
    case 4 : /* add |--o element */
             pickMode = 2 ;
             mdgfx_set_input(" Element |--o properties ",
             " E = ", defaultE,
             " A = ", defaultA,
             " I = ", defaultI, 0, 0,
             0);
             break ;
    case 5 : /* add ux b.c */
             if (mdText == 0)
             {
               pickMode = 2 ;
               mdgfx_set_input(" UX Support ",
                 " UX = ", defaultPosX, 0, 0, 0, 0, 0, 0, 0);
             }
             else
             {
               pickMode = 1 ;
               mdgfx_set_input(" UX Support ",
                 " UX = ", defaultPosX, 0, 0, 0, 0, "Node: ",1, 2);
             }
             break ;
    case 6 : /* add uy b.c */
             if (mdText == 0)
             {
             pickMode = 2 ;
             mdgfx_set_input(" UY Support ",
                 0,0, " UY = ", defaultPosY, 0, 0, 0, 0, 0);
             }
             else
             {
               pickMode = 1 ;
               mdgfx_set_input(" UY Support ",
                 0,0, " UY = ", defaultPosY, 0, 0, "Node: ", 1, 2);
             }
             break ;
    case 7 : /* add ux+uy b.c */
             if (mdText == 0)
             {
             pickMode = 2 ;
             mdgfx_set_input(" UX+UY Support ",
                 " UX = ",defaultPosX," UY = ",defaultPosY,0,0,0, 0, 0);
             }
             else
             {
               pickMode = 1 ;
               mdgfx_set_input(" UX+UY Support ",
                 " UX = ",defaultPosX," UY = ",defaultPosY,0,0,"Node: ", 1, 2);
             }
             break ;
    case 8 : /* add ux+rotz b.c */
             pickMode = 2 ;
             mdgfx_set_input(" UX+ROTZ Support ",
                 " UX = ",defaultPosX,0,0," ROTZ = ",defaultRotZ,0, 0, 0);
             break ;
    case 9 : /* add uy+rotz b.c */
             pickMode = 2 ;
             mdgfx_set_input(" UY+ROTZ Support ",
                 0,0," UY = ",defaultPosY," ROTZ = ",defaultRotZ,0, 0, 0);
             break ;
    case 10 : /* add ux+uy+rotz b.c */
             if (mdText == 0)
             {
             pickMode = 2 ;
             mdgfx_set_input(" UX+UY+ROTZ Support ",
                 " UX = ",defaultPosX," UY = ",defaultPosY," ROTZ = ",defaultRotZ,0, 0, 0);
             }
             else
             {
               pickMode = 1 ;
               mdgfx_set_input(" UX+UY+ROTZ Support ",
                 " UX = ",defaultPosX," UY = ",defaultPosY," ROTZ = ",defaultRotZ,"Node: ", 1, 2);
             }
             break ;
    case 11 : /* add force/moment */
             if (mdText == 0)
             {
             pickMode = 2 ;
             mdgfx_set_input(" Force/Moment ",
                 " FX = ",defaultFx," FY = ",defaultFy," MZ = ",defaultMz,0, 0, 0);
             }
             else
             {
               pickMode = 1 ;
               mdgfx_set_input(" Force/Moment ",
                 "FX=",defaultFx,"FY=",defaultFy,"M=",defaultMz,"Nd:", 1, 2);
             }

             break ;
    case 13 : /* add na,nb  */
             if (mdText == 0)
             {
             pickMode = 3 ;
             mdgfx_set_input(" Load in normal dir. ",
                 " na = ",defaultNA," nb = ",defaultNB,0,0,0, 0, 0);
             }
             else
             {
             pickMode = 3 ;
             mdgfx_set_input(" Load in normal dir. ",
                 " na = ",defaultNA," nb = ",defaultNB,0,0,"E:", 1, 2);
             }
             break ;
    case 14 : /* add na,nb  */
             if (mdText == 0)
             {
             pickMode = 3 ;
             mdgfx_set_input(" Load in transverse dir. ",
                 " va = ",defaultVA," vb = ",defaultVB,0,0,0, 0, 0);
             }
             else
             {
             pickMode = 3 ;
             mdgfx_set_input(" Load in transverse dir. ",
                 " va = ",defaultVA," vb = ",defaultVB,0,0,"E", 1, 2);
             }
             break ;

    case 21 : /* delete node  */
             if (mdText == 0)
             {
              pickMode = 2 ;
              mdgfx_set_input(0, 0,0,0,0,0,0,0, 0, 0);
             }
             else
             {
              pickMode = 1 ;
              mdgfx_set_input("Node to delete", 0,0,0,0,0,0,"Node: ", 1, 2);
             }
             break ;
    case 22 : /* delete element  */
             if (mdText == 0)
             {
              pickMode = 3 ;
              mdgfx_set_input(0, 0,0,0,0,0,0,0, 0, 0);
             }
             else
             {
              pickMode = 3 ;
              mdgfx_set_input("Element to delete", 0,0,0,0,0,0,"Elem: ", 1, 2);
             }
             break ;
    case 23 : /* delete support  */
             if (mdText == 0)
             {
             pickMode = 2 ;
             mdgfx_set_input(0, 0,0,0,0,0,0,0, 0, 0);
             }
             else
             {
             pickMode = 1 ;
             mdgfx_set_input("Support to delete", 0,0,0,0,0,0,"Node: ", 1, 2);
             }
             break ;
    case 24 : /* delete force  */
             if (mdText == 0)
             {
             pickMode = 2 ;
             mdgfx_set_input(0, 0,0,0,0,0,0,0, 0, 0);
             }
             else
             {
             pickMode = 1 ;
             mdgfx_set_input("Force to delete", 0,0,0,0,0,0,"Node: ", 1, 2);
             }
             break ;
    case 25 : /* delete na,nb  */
             if (mdText == 0)
             {
             pickMode = 3 ;
             mdgfx_set_input(0, 0,0,0,0,0,0,0, 0, 0);
             }
             else
             {
              pickMode = 2 ;
              mdgfx_set_input("Axial load to delete", 0,0,0,0,0,0,"Elem: ", 1, 2);
             }
             break ;
    case 26 : /* delete va,vb  */
             if (mdText == 0)
             {
             pickMode = 3 ;
             mdgfx_set_input(0, 0,0,0,0,0,0,0, 0, 0);
             }
             else
             {
              pickMode = 2 ;
              mdgfx_set_input("Transverse load to delete", 0,0,0,0,0,0,"Elem: ", 1, 2);
             }
             break ;

    case 27 : /* grid real size */
             pickMode = 0 ;
             mdgfx_set_input(" Grid size ",
                 " dx = ",gridReal," pixels = ",gridSpace,0,0, 0, 0,1);
             break ;
    case 28 : /* underlined elements */
             if (mdSimpleE == 1) { mdSimpleE = 0 ; }
             else                { mdSimpleE = 1 ; }
             break ;

    case 41 : /* show node numbers */
             if (nodeNumbers == 1) { nodeNumbers = 0 ; }
             else                  { nodeNumbers = 1 ; }
             break ;

    case 42 : /* show element numbers */
             if (elemNumbers == 1) { elemNumbers = 0 ; }
             else                  { elemNumbers = 1 ; }
             break ;

    case 43 : /* show element numbers */
             if (mdText == 1)      { mdText = 0 ; }
             else                  { mdText = 1 ; }
             break ;





    case 29 : /* add |--o element */
             pickMode = 2 ;
             mdgfx_set_input(" Element o--o properties ",
             " E = ", defaultE,
             " A = ", defaultA,
             " I = ", defaultI, 0, 0,
             0);
             break ;
    case 30 : /* change element type */
             if (mdText == 0)
             {
             pickMode = 3 ;
             mdgfx_set_input("Change Element to |--|", 0,0,0,0,0,0,0, 0, 0);
             }
             else
             {
             pickMode = 2 ;
             mdgfx_set_input("Change Element to |--|", 0,0,0,0,0,0,"Elem: ", 1, 2);
             }
             break ;
    case 31 :
             if (mdText == 0)
             {
             pickMode = 3 ;
             mdgfx_set_input("Change Element to o--|", 0,0,0,0,0,0,0, 0, 0);
             }
             else
             {
             pickMode = 2 ;
             mdgfx_set_input("Change Element to o--|", 0,0,0,0,0,0,"Elem: ", 1, 2);
             }
             break ;
    case 32 :
             if (mdText == 0)
             {
             pickMode = 3 ;
             mdgfx_set_input("Change Element to |--o", 0,0,0,0,0,0,0, 0, 0);
             }
             else
             {
             pickMode = 2 ;
             mdgfx_set_input("Change Element to |--o", 0,0,0,0,0,0,"Elem: ", 1, 2);
             }
             break ;
    case 33 :
             if (mdText == 0)
             {
             pickMode = 3 ;
             mdgfx_set_input("Change Element to o--o", 0,0,0,0,0,0,0, 0, 0);
             }
             else
             {
             pickMode = 2 ;
             mdgfx_set_input("Change Element to 0--0", 0,0,0,0,0,0,"Elem: ", 1, 2);
             }
             break ;

    /* Editing functions: ----------------- */
    case 15: /* edit node */
             if (mdText == 0)
             {
             pickMode = 2 ;
             nodeEdit = -1 ;
             mdgfx_set_input(" Pick a Node! ",
             0, 0, 0, 0, 0, 0, 0, 0, 0);
             }
             else
             {
             pickMode = 2 ;
             nodeEdit = -1 ;
             mdgfx_set_input(" Node to edit ",
             0, 0, 0, 0, 0, 0, "Node: ", 1, 2);
             }
             break ;
    case 16: /* edit element */
             if (mdText == 0)
             {
             pickMode = 3 ;
             elemEdit = -1 ;
             mdgfx_set_input(" Pick an Element! ",
             0, 0, 0, 0, 0, 0, 0, 0, 0);
             }
             else
             {
             pickMode = 2 ;
             elemEdit = -1 ;
             mdgfx_set_input(" Element to edit ",
             0, 0, 0, 0, 0, 0, "Elem: ", 1, 2);

             }
             break ;
    /* Text input functions: ----------------- */
    case 51: /* add element */
             pickMode = 1 ;
             mdgfx_set_input(" Element nodes ",
               " N1 = ", 1,
               " N2 = ", 2,
               0, 0, 0, 0,
               2);
             break ;

  }
  nodeEdit = -1 ;
  elemEdit = -1 ;
  nodeOK   =  0 ;
  elemOK   =  0 ;
#endif
}

/* action on input (mouse click, button press,...): */
void md_input_action(int x, int y, double val1, double val2, double val3, double val4)
{
#ifdef GGUI
  static int enode = 0 ;
  static int nnum  = 0 ;
  int   pos ;
  double xg, yg ;

  switch (gfxAction)
  {
    case 0 : 
    case 555:
    case 666:
             pickMode = 0 ;
             mdgfx_set_input(0, 0, 0, 0, 0, 0, 0, 0,0,0);
             break ;
    case 1 : /* add node */
             if (mdText == 1)
             {
               if ((val1 >=0) && (val2 >=0))
               {
                 md_add_node(val1, val2);
               }
             }
             else
             {
               md_grid_find_clicked(x, y, &xg, &yg);
               md_add_node(xg, yg);
             }
             break ;
    case 2 : /* add |--| element */
    case 3 : /* add o--| element */
    case 4 : /* add |--o element */
    case 29: /* add o--o element */
             if (mdText == 0)
             {
             if (n_len < 2) {return;}
             defaultE = val1 ;
             defaultA = val2 ;
             defaultI = val3 ;

             if (enode == 1)
             {
               if ((pos=gfx_node_find_clicked(x,y)) >= 0)
               {
                 if (nnum != pos)
                 {
                   if (gfxAction < 10)
                   {
                     md_add_elem(gfxAction-2,  /* type */
                               nnum,
                               pos,
                               defaultE,
                               defaultA,
                               defaultI);
                   }
                   else
                   {
                      md_add_elem(3,  /* type */
                               nnum,
                               pos,
                               defaultE,
                               defaultA,
                               defaultI);
                   }
                 }
               }
               nnum  = 0 ;
               enode = 0 ;
             }
             else
             {
               nnum = gfx_node_find_clicked(x, y) ;
               if (nnum >= 0)
               {
                enode = 1 ;
               }
             }
             }
             else /* keyboard-only input */
             {
              defaultE = val1 ;
              defaultA = val2 ;
              defaultI = val3 ;
              defaultET= gfxAction ;

              nodeNumbers = 1 ;
              gfxAction = 51 ;
              pickMode = 1 ;
              md_from_user_action();
             }
             break ;
    case 5 : /* add ux b.c */
             if (mdText == 0)
             {
             if ((pos=gfx_node_find_clicked(x,y)) >= 0)
             {
               defaultPosX = val1 ;
               defaultPosY = 0 ;
               defaultRotZ = 0 ;
               md_add_disp(pos, gfxAction-4, defaultPosX, defaultPosY, defaultRotZ);
             }
             }
             else
             {
              if ((pos=md_node_find_by_number((int)val4)) != -1)
              {
                defaultPosX = val1 ;
                defaultPosY = 0 ;
                defaultRotZ = 0 ;
                md_add_disp(pos, gfxAction-4, defaultPosX, defaultPosY, defaultRotZ);
              }
             }
             break ;
    case 6 : /* add uy b.c */
             if (mdText == 0)
             {
             if ((pos=gfx_node_find_clicked(x,y)) >= 0)
             {
               defaultPosX = 0 ;
               defaultPosY = val2 ;
               defaultRotZ = 0 ;
               md_add_disp(pos, gfxAction-4, defaultPosX, defaultPosY, defaultRotZ);
             }
             }
             else
             {
              if ((pos=md_node_find_by_number((int)val4)) != -1)
              {
               defaultPosX = 0 ;
               defaultPosY = val2 ;
               defaultRotZ = 0 ;
               md_add_disp(pos, gfxAction-4, defaultPosX, defaultPosY, defaultRotZ);
              }
             }
             break ;
    case 7 : /* add ux+uy b.c */
             if (mdText == 0)
             {
             if ((pos=gfx_node_find_clicked(x,y)) >= 0)
             {
               defaultPosX = val1 ;
               defaultPosY = val2 ;
               defaultRotZ = 0 ;
               md_add_disp(pos, gfxAction-4, defaultPosX, defaultPosY, defaultRotZ);
             }
             }
             else
             {
              if ((pos=md_node_find_by_number((int)val4)) != -1)
              {
               defaultPosX = val1 ;
               defaultPosY = val2 ;
               defaultRotZ = 0 ;
               md_add_disp(pos, gfxAction-4, defaultPosX, defaultPosY, defaultRotZ);
              }
             }
             break ;
    case 8 : /* add ux+rotz b.c */
             if ((pos=gfx_node_find_clicked(x,y)) >= 0)
             {
               defaultPosX = val1 ;
               defaultPosY = 0 ;
               defaultRotZ = val3 ;
               md_add_disp(pos, gfxAction-4, defaultPosX, defaultPosY, defaultRotZ);
             }
             break ;
    case 9 : /* add uy+rotz b.c */
             if ((pos=gfx_node_find_clicked(x,y)) >= 0)
             {
               defaultPosX = 0 ;
               defaultPosY = val2 ;
               defaultRotZ = val3 ;
               md_add_disp(pos, gfxAction-4, defaultPosX, defaultPosY, defaultRotZ);
             }
             break ;
    case 10 : /* add ux+uy+rotz b.c */
             if (mdText == 0)
             {
             if ((pos=gfx_node_find_clicked(x,y)) >= 0)
             {
               defaultPosX = val1 ;
               defaultPosY = val2 ;
               defaultRotZ = val3 ;
               md_add_disp(pos, gfxAction-4, defaultPosX, defaultPosY, defaultRotZ);
             }
             }
             else
             {
              if ((pos=md_node_find_by_number((int)val4)) != -1)
              {
               defaultPosX = val1 ;
               defaultPosY = val2 ;
               defaultRotZ = val3 ;
               md_add_disp(pos, gfxAction-4, defaultPosX, defaultPosY, defaultRotZ);
              }
             }
             break ;
    case 11 : /* add force/moment */
             if (mdText == 0)
             {
             if ((pos=gfx_node_find_clicked(x,y)) >= 0)
             {
               defaultFx = val1 ;
               defaultFy = val2 ;
               defaultMz = val3 ;
               md_add_force(pos, defaultFx, defaultFy, defaultMz);
             }
             }
             else
             {
              if ((pos=md_node_find_by_number((int)val4)) != -1)
              {
               defaultFx = val1 ;
               defaultFy = val2 ;
               defaultMz = val3 ;
               md_add_force(pos, defaultFx, defaultFy, defaultMz);
              }
             }
             break ;
    case 13 : /* add na,nb  */
             if (mdText == 0)
             {
             if ((pos=gfx_elem_find_clicked(x,y)) >= 0)
             {
               defaultNA = val1 ;
               defaultNB = val2 ;
               md_add_n_eload(pos, defaultNA, defaultNB);
             }
             }
             else
             {
              if ((pos=md_elem_find_by_number((int)val4)) != -1)
              {
               defaultNA = val1 ;
               defaultNB = val2 ;
               md_add_n_eload(pos, defaultNA, defaultNB);
              }
             }
             break ;
    case 14 : /* add na,nb  */
             if (mdText == 0)
             {
             if ((pos=gfx_elem_find_clicked(x,y)) >= 0)
             {
               defaultVA = val1 ;
               defaultVB = val2 ;
               md_add_v_eload(pos, defaultVA, defaultVB);
             }
             }
             else
             {
              if ((pos=md_elem_find_by_number((int)val4)) != -1)
              {
               defaultVA = val1 ;
               defaultVB = val2 ;
               md_add_v_eload(pos, defaultVA, defaultVB);
              }
             }
             break ;

    case 21 : /* delete node */
             if (mdText == 0)
             {
             if ((pos=gfx_node_find_clicked(x,y)) >= 0)
             {
               md_del_node(pos);
             }
             }
             else
             {
              if ((pos=md_node_find_by_number((int)val4)) != -1)
              {
                md_del_node(pos);
              }
             }
             break ;
    case 22 : /* delete element */
             if (mdText == 0)
             {
             if ((pos=gfx_elem_find_clicked(x,y)) >= 0)
             {
               md_del_elem(pos);
             }
             }
             else
             {
              if ((pos=md_elem_find_by_number((int)val4)) != -1)
              {
                md_del_elem(pos);
              }
             }
             break ;
    case 23 : /* delete support */
             if (mdText == 0)
             {
             if ((pos=gfx_node_find_clicked(x,y)) >= 0)
             {
               md_del_disp(pos);
             }
             }
             else
             {
              if ((pos=md_node_find_by_number((int)val4)) != -1)
              {
               md_del_disp(pos);
              }
             }
             break ;
    case 24 : /* delete force */
             if (mdText == 0)
             {
             if ((pos=gfx_node_find_clicked(x,y)) >= 0)
             {
               md_del_force(pos);
             }
             }
             else
             {
              if ((pos=md_node_find_by_number((int)val4)) != -1)
              {
               md_del_force(pos);
              }
             }
             break ;
    case 25 : /* delete na,nb */
             if (mdText == 0)
             {
             if ((pos=gfx_elem_find_clicked(x,y)) >= 0)
             {
               md_del_n_eload(pos);
             }
             }
             else
             {
              if ((pos=md_elem_find_by_number((int)val4)) != -1)
              {
               md_del_n_eload(pos);
              }
             }
             break ;
    case 26 : /* delete va,vb */
             if (mdText == 0)
             {
             if ((pos=gfx_elem_find_clicked(x,y)) >= 0)
             {
               md_del_v_eload(pos);
             }
             }
             else
             {
              if ((pos=md_elem_find_by_number((int)val4)) != -1)
              {
               md_del_v_eload(pos);
              }
             }
             break ;

    case 27 : /* grid real size */
             if ((val1 >= 1e-3)&&((int)val2>=16)&&((int)val2<=256))
             {
               gridReal  = val1 ;
               gridSpace = (int)val2 ;
               if (gridSpace <=0) {gridSpace = 32 ;}
             }
             mdgfx_set_input(0, 0,0,0,0,0,0,0,0,0);
/*printf("gridReal = %e\n", gridReal);*/
             break ;
    /* see "case 2" for "case 29" !*/
    case 30 : /* change element type */
    case 31 :
    case 32 :
    case 33 :
             if (mdText == 0)
             {
             if ((pos=gfx_elem_find_clicked(x,y)) >= 0)
             {
               SET_ELEM_TYPE(pos,gfxAction-30) ;
             }
             }
             else
             {
              if ((pos=md_elem_find_by_number((int)val4)) != -1)
              {
                SET_ELEM_TYPE(pos,gfxAction-30) ;
              }
             }
             break ;


    /* Editing functions: ----------------- */
    case 15: /* edit nodes */
             if (mdText == 0)
             {
             if (nodeEdit == -1)
             {
               if ((nodeEdit=gfx_node_find_clicked(x,y)) >= 0)
               {
                 pickMode = 0 ;
                 mdgfx_set_input(" Node position ",
                 " X = ", GET_NODE_X(nodeEdit),
                 " Y = ", GET_NODE_Y(nodeEdit),
                 0, 0,0,0,
                 1);
                 nodeOK = 1 ;
               }
               else
               {
                 nodeOK = 0 ;
               }
             }
             else
             {
               if (nodeOK == 1)
               {
                SET_NODE_X(nodeEdit, val1) ;
                SET_NODE_Y(nodeEdit, val2) ;
               }

               nodeOK = 0 ;

               /* another node */
               pickMode = 2 ;
               nodeEdit = -1 ;
               mdgfx_set_input(" Pick a Node! ",
               0, 0, 0, 0, 0, 0, 0,0,0);

               nodeEdit = -1 ;
             }
             }
             else /* text-only: */
             {
             if (nodeEdit == -1)
             {
               if ((nodeEdit=md_node_find_by_number((int)val4)) >= 0)
               {
                 pickMode = 0 ;
                 mdgfx_set_input(" Node position ",
                 " X = ", GET_NODE_X(nodeEdit),
                 " Y = ", GET_NODE_Y(nodeEdit),
                 0, 0,0,0,
                 1);
                 nodeOK = 1 ;
               }
               else
               {
                 nodeOK = 0 ;
               }
             }
             else
             {
               if (nodeOK == 1)
               {
                SET_NODE_X(nodeEdit, val1) ;
                SET_NODE_Y(nodeEdit, val2) ;
               }

               nodeOK = 0 ;

               /* another node */
               pickMode = 1 ;
               nodeEdit = -1 ;

               mdgfx_set_input(" Node to edit ",
               0, 0, 0, 0, 0, 0, "Node: ", 1, 2);
             }
             }
             break ;
    case 16: /* edit element */
             if (mdText == 0)
             {
             if (elemEdit == -1)
             {
               if ((elemEdit=gfx_elem_find_clicked(x,y)) >= 0)
               {
                 pickMode = 0 ;
                 mdgfx_set_input(" Element properties ",
                 " E = ", GET_ELEM_E(elemEdit),
                 " A = ", GET_ELEM_A(elemEdit),
                 " I = ", GET_ELEM_I(elemEdit),0,0,
                 1);
                 elemOK = 1 ;
               }
               else
               {
                 elemOK = 0 ;
               }
             }
             else
             {
               if (elemOK == 1)
               {
                 SET_ELEM_E(elemEdit,val1) ;
                 SET_ELEM_A(elemEdit,val2) ;
                 SET_ELEM_I(elemEdit,val3) ;
               }

               elemOK = 0 ;
               
               /* another element */
               pickMode = 3 ;
               elemEdit = -1 ;
               mdgfx_set_input(" Pick an Element! ",
               0, 0, 0, 0, 0, 0, 0,0,0);

               elemEdit = -1 ;
             }
             }
             else
             {
             if (elemEdit == -1)
             {
               if ((elemEdit=md_elem_find_by_number((int)val4)) >= 0)
               {
                 pickMode = 0 ;
                 mdgfx_set_input(" Element properties ",
                 " E = ", GET_ELEM_E(elemEdit),
                 " A = ", GET_ELEM_A(elemEdit),
                 " I = ", GET_ELEM_I(elemEdit),0,0,
                 1);
                 elemOK = 1 ;
               }
               else
               {
                 elemOK = 0 ;
               }
             }
             else
             {
               if (elemOK == 1)
               {
                 SET_ELEM_E(elemEdit,val1) ;
                 SET_ELEM_A(elemEdit,val2) ;
                 SET_ELEM_I(elemEdit,val3) ;
               }

               elemOK = 0 ;
               
               /* another element */
               pickMode = 3 ;
               elemEdit = -1 ;
               mdgfx_set_input(" Element to edit ",
               0, 0, 0, 0, 0, 0, "Elem: ",1,2);

               elemEdit = -1 ;
             }
             }
             break ;
    case 51: /* add element from keyboard */
    if (defaultET < 10)
                   {
                     md_add_elem(defaultET-2,  /* type */
                               (int)val1-1,
                               (int)val2-1,
                               defaultE,
                               defaultA,
                               defaultI);
                   }
                   else
                   {
                      md_add_elem(3,  /* type */
                               (int)val1-1,
                               (int)val2-1,
                               defaultE,
                               defaultA,
                               defaultI);
                   }
                   break;

  }
#endif
}

/* end of md_gfx.c */
