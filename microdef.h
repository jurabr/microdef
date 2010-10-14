/*
   File name: microdef.h
   Date:      2005/01/03 16:31
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

	 MicroDefor
*/

#ifndef __MICRODEF_H__
#define __MICRODEF_H__

#define OK  0
#define ERR 1
#define ERR_EMP 2
#define ERR_IO  3
#define ERR_MEM 4
#define ERR_VAL 5
#define ERR_SIZ 6
#define ERR_ZER 7

#define MDTERM_NONE 0
#define MDTERM_GTK  1
#define MDTERM_GD   2
#define MDTERM_PS   3

#define MDOUT_TEXT  0
#define MDOUT_HTML  1
#define MDOUT_LATEX 2

#define MDGFX_LEN   4
#define MDGFX_SLEN  3
#define MDGFX_BLEN  12
#define MDGFX_BLEN2 6
#define MDGFX_BLENB 14

extern int    md_max_nodes ; /* max. number of nodes   */
extern int    md_max_elems ; /* man number of eleemnts */
extern double gridReal ;  /* real-world size of grid space */

extern int num_div ;

extern int *md_alloc_int(int length);
extern void md_free_int(int *fld);
extern double *md_alloc_double(int length);
extern void md_free_double(double *fld);

/* data operations (preprocessing) */
extern int md_add_node(double x, double y);
extern int md_add_elem(int type, int n1, int n2, double E, double A, double I);
extern int md_add_disp(int node_pos, int type, double posx, double posy, double rotz);
extern int md_add_force(int node_pos, double fx, double fy, double mz);
extern int md_add_n_eload(int elem_pos, double na, double nb);
extern int md_add_v_eload(int elem_pos, double va, double vb);

extern int md_del_node(int pos);
extern int md_del_elem(int pos);
extern int md_del_disp(int node_pos);
extern int md_del_force(int node_pos);
extern int md_del_n_eload(int elem_pos);
extern int md_del_v_eload(int elem_pos);

extern void md_reduce_coords(void);

#ifdef POSIX

#include <math.h>
#include <stdlib.h>

#define GET_NODE_ID(pos) n_id[pos]
#define GET_NODE_X(pos) n_x[pos]
#define GET_NODE_Y(pos) n_y[pos]
#define GET_NODE_TYPE(pos) n_dtype[pos]
#define GET_NODE_FX(pos) n_fx[pos]
#define GET_NODE_FY(pos) n_fy[pos]
#define GET_NODE_MZ(pos) n_mz[pos]
#define GET_NODE_POSX(pos) n_posx[pos]
#define GET_NODE_POSY(pos) n_posy[pos]
#define GET_NODE_ROTZ(pos) n_rotz[pos]

#define SET_NODE_X(pos, val) n_x[pos] = val
#define SET_NODE_Y(pos, val) n_y[pos] = val

#define GET_ELEM_ID(pos) e_id[pos]
#define GET_ELEM_TYPE(pos) e_type[pos]
#define GET_ELEM_E(pos) e_E[pos]
#define GET_ELEM_A(pos) e_A[pos]
#define GET_ELEM_I(pos) e_I[pos]
#define GET_ELEM_N1(pos) e_n1[pos]
#define GET_ELEM_N2(pos) e_n2[pos]
#define GET_ELEM_NA(pos) e_na[pos]
#define GET_ELEM_NB(pos) e_nb[pos]
#define GET_ELEM_VA(pos) e_va[pos]
#define GET_ELEM_VB(pos) e_vb[pos]

#define SET_ELEM_TYPE(pos,type) e_type[pos]=type
#define SET_ELEM_E(pos,val) e_E[pos] = val
#define SET_ELEM_A(pos,val) e_A[pos] = val
#define SET_ELEM_I(pos,val) e_I[pos] = val

#define GET_NRES_FX(pos) nr_fx[pos]
#define GET_NRES_FY(pos) nr_fy[pos]
#define GET_NRES_MZ(pos) nr_mz[pos]
#define GET_NRES_POSX(pos) nr_posx[pos]
#define GET_NRES_POSY(pos) nr_posy[pos]
#define GET_NRES_ROTZ(pos) nr_rotz[pos]

#define PUT_NRES_FX(pos,val) nr_fx[pos]=val
#define PUT_NRES_FY(pos,val) nr_fy[pos]=val
#define PUT_NRES_MZ(pos,val) nr_mz[pos]=val
#define PUT_NRES_POSX(pos,val) nr_posx[pos]=val
#define PUT_NRES_POSY(pos,val) nr_posy[pos]=val
#define PUT_NRES_ROTZ(pos,val) nr_rotz[pos]=val

#define ADD_NRES_FX(pos,val) nr_fx[pos]+=val
#define ADD_NRES_FY(pos,val) nr_fy[pos]+=val
#define ADD_NRES_MZ(pos,val) nr_mz[pos]+=val

#define GET_ERES_A_FX(pos) er_a_fx[pos]
#define GET_ERES_A_FY(pos) er_a_fy[pos]
#define GET_ERES_A_MZ(pos) er_a_mz[pos]
#define GET_ERES_B_FX(pos) er_b_fx[pos]
#define GET_ERES_B_FY(pos) er_b_fy[pos]
#define GET_ERES_B_MZ(pos) er_b_mz[pos]

#define PUT_ERES_A_FX(pos,val) er_a_fx[pos]=val
#define PUT_ERES_A_FY(pos,val) er_a_fy[pos]=val
#define PUT_ERES_A_MZ(pos,val) er_a_mz[pos]=val
#define PUT_ERES_B_FX(pos,val) er_b_fx[pos]=val
#define PUT_ERES_B_FY(pos,val) er_b_fy[pos]=val
#define PUT_ERES_B_MZ(pos,val) er_b_mz[pos]=val

#define ADD_ERES_A_FX(pos,val) er_a_fx[pos]+=val
#define ADD_ERES_A_FY(pos,val) er_a_fy[pos]+=val
#define ADD_ERES_A_MZ(pos,val) er_a_mz[pos]+=val
#define ADD_ERES_B_FX(pos,val) er_b_fx[pos]+=val
#define ADD_ERES_B_FY(pos,val) er_b_fy[pos]+=val
#define ADD_ERES_B_MZ(pos,val) er_b_mz[pos]+=val

#define DELETE_NODE(pos) md_del_node(pos)
#define DELETE_ELEM(pos) md_del_elem(pos)

extern int   n_len  ;
extern int   e_len  ;

/* NODES: */
extern int   *n_id    ;
extern double *n_x     ;
extern double *n_y     ;
extern int   *n_dtype ;
extern double *n_fx    ;
extern double *n_fy    ;
extern double *n_mz    ;
extern double *n_posx  ;
extern double *n_posy  ;
extern double *n_rotz  ;

/* results on nodes */
extern double *nr_posx  ;
extern double *nr_posy  ;
extern double *nr_rotz  ;

extern double *nr_fx  ;
extern double *nr_fy  ;
extern double *nr_mz  ;

/* ELEMENTS: */
extern int   *e_id    ;
extern int   *e_type  ;
extern int   *e_n1    ;
extern int   *e_n2    ;
extern double *e_E     ;
extern double *e_A     ;
extern double *e_I     ;
extern double *e_na    ;
extern double *e_nb    ;
extern double *e_va    ;
extern double *e_vb    ;

/* element results: */
extern double *er_a_fx  ;
extern double *er_a_fy  ;
extern double *er_a_mz  ;
extern double *er_b_fx  ;
extern double *er_b_fy  ;
extern double *er_b_mz  ;

extern int read_test(char *ifname);
extern int read_data(char *ifname);
extern int write_data(char *ofname);
extern int md_alloc_empty(void);

extern void md_make_empty(void);

extern double compute_L(int pos) ;
extern double compute_e_res(int type, int epos, int div, int ppos);
extern double get_max_NVM_res(int type, int epos, int div, int *ppos);
extern double get_min_NVM_res(int type, int epos, int div, int *ppos);

extern char *md_set_file_ext(char *str, char *lab, char *ext);
#endif /* POSIX */

#ifdef GTKGUI
extern void mdgtk_draw_point(int x, int y);
extern void mdgtk_draw_big_point(int x, int y);
extern void mdgtk_draw_line(int x1, int y1, int x2, int y2, int width);
extern void get_draw_size(int *x0, int *y0, int *width, int *height);
extern void mdgtk_draw_string(int x, int y, char *str);
#endif

#ifdef GDGUI /* GD library*/
extern void get_draw_size_gd(int *x0,int *y0,int *width,int *height);
extern void mdgd_draw_point(int x,int  y);
extern void mdgd_draw_line(int x1,int  y1,int  x2,int  y2,int  width);
extern void mdgd_draw_string(int x,int  y, char *str);
extern int mdgd_draw(int width, int height, char *fname);

extern int mdgd_draw_struct(int width, int height, char *fname);
extern int mdgd_draw_def(int width, int height, char *fname);
extern int mdgd_draw_N(int width, int height, char *fname);
extern int mdgd_draw_V(int width, int height, char *fname);
extern int mdgd_draw_M(int width, int height, char *fname);
#endif /* GD */

#ifdef PSGUI /* raw PS output */
extern void get_draw_size_ps(int *x0,int *y0,int *width,int *height);
extern void mdps_draw_point(int x,int  y);
extern void mdps_draw_line(int x1,int  y1,int  x2,int  y2,int  width);
extern void mdps_draw_string(int x,int  y, char *str);
extern int mdps_draw(int width, int height, char *fname);

extern int mdps_draw_struct(int width, int height, char *fname);
extern int mdps_draw_def(int width, int height, char *fname);
extern int mdps_draw_N(int width, int height, char *fname);
extern int mdps_draw_V(int width, int height, char *fname);
extern int mdps_draw_M(int width, int height, char *fname);
#endif /* PS */

/* catastrophic scenario (http://www.ibiblio.org/apollo/WinGtkHowto.html): */
#ifdef WIN32 
#include <windows.h> 
#endif

#endif

/* end of microdef.h */
