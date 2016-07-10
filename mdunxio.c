/*
   File name: mdunxio.c
   Date:      2005/01/03 16:07
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

	 MicroDefor - I/O functions for ANSI C systems
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "microdef.h"

int    md_max_nodes = 300 ; /* max. number of nodes   */
int    md_max_elems = 300 ; /* max number of elements */

FILE *ifile = NULL ; /* input file  */
FILE *ofile = NULL ; /* output file */

char *ifname = NULL ; /* input file  */
char *ofname = NULL ; /* output file */


int   n_len  = 0 ; /* number of nodes    */
int   e_len  = 0 ; /* number of elements */

/* NODES: */
int   *n_id    = NULL ;
double *n_x     = NULL ; /* x coordinate   */
double *n_y     = NULL ; /* y coordinate   */
int   *n_dtype = NULL ; /* disp. type 0=none, 1=ux, 2=uy, 3=ux+uy, 4=ux+rotz,
													 5=uy+rotz,6=ux+uy+rotz */
double *n_fx    = NULL ; /* force x        */
double *n_fy    = NULL ; /* force y        */
double *n_mz    = NULL ; /* moment z       */
double *n_posx  = NULL ; /* displacement x */
double *n_posy  = NULL ; /* displacement y */
double *n_rotz  = NULL ; /* rotation       */

/* results on nodes */
double *nr_posx  = NULL ; /* displacement x */
double *nr_posy  = NULL ; /* displacement y */
double *nr_rotz  = NULL ; /* rotation       */
double *nr_fx    = NULL ; /* force x        */
double *nr_fy    = NULL ; /* force y        */
double *nr_mz    = NULL ; /* moment z       */

/* ELEMENTS: */
int   *e_id    = NULL ;
int   *e_type  = NULL ; /* 0..|-| 1..o-| 2..|-o 3 o-o */
int   *e_n1    = NULL ; /* 1st node position */
int   *e_n2    = NULL ; /* 2nd node position */
double *e_E     = NULL ; /* Young's modullus  */
double *e_A     = NULL ; /* area              */
double *e_I     = NULL ; /* moment of inertia */
double *e_na    = NULL ; /* -> load at start  */
double *e_nb    = NULL ; /* -> load at end    */
double *e_va    = NULL ; /* \|/ load at start */
double *e_vb    = NULL ; /* \|/ load at end   */
double *e_ttop  = NULL ; /* temperature at top*/
double *e_tbot  = NULL ; /* temp at bottom    */

/* element results: */
double *er_a_fx  = NULL ; /* N at start       */
double *er_a_fy  = NULL ; /* V at start       */
double *er_a_mz  = NULL ; /* M at start       */
double *er_b_fx  = NULL ; /* N at end         */
double *er_b_fy  = NULL ; /* V at end         */
double *er_b_mz  = NULL ; /* M at end         */

double gridReal = 0.5 ;  /* real-world size of grid space */

int *md_alloc_int(int length)
{
	int *field = NULL;
	int  i;

	if (length < 1) { return(NULL); }

	if ((field = (int *) malloc(length * sizeof(int))) == NULL)
	{ 
		return(NULL);  
	}
	else                                                    
	{ 
		for (i = 0; i < length; i++) { field[i] = 0; }
		return(field); 
	}
}

double *md_alloc_double(int length)
{
	double *field = NULL;
	int    i;

	if (length < 1) { return(NULL); }

	if ((field = (double *) malloc(length * sizeof(double))) == NULL)
	{ 
		return(NULL);  
	}
	else                                                    
	{ 
		for (i = 0; i < length; i++) { field[i] = 0; }
		return(field); 
	}
}

void md_free_int(int *fld)
{
	free(fld);
	fld = NULL;
}

void md_free_double(double *fld)
{
	free(fld);
	fld = NULL;
}

void md_n_free(void)
{
	md_free_int(n_id);
	md_free_double(n_x);
	md_free_double(n_y);
	md_free_int(n_dtype);
	md_free_double(n_fx);
	md_free_double(n_fy);
	md_free_double(n_mz);
	md_free_double(n_posx);
	md_free_double(n_posy);
	md_free_double(n_rotz);
}

int md_n_alloc(int length)
{
	if ((n_id=md_alloc_int(length)) == NULL){goto memFree;}
	if ((n_x=md_alloc_double(length)) == NULL){goto memFree;}
	if ((n_y=md_alloc_double(length)) == NULL){goto memFree;}
	if ((n_dtype=md_alloc_int(length)) == NULL){goto memFree;}
	if ((n_fx=md_alloc_double(length)) == NULL){goto memFree;}
	if ((n_fy=md_alloc_double(length)) == NULL){goto memFree;}
	if ((n_mz=md_alloc_double(length)) == NULL){goto memFree;}
	if ((n_posx=md_alloc_double(length)) == NULL){goto memFree;}
	if ((n_posy=md_alloc_double(length)) == NULL){goto memFree;}
	if ((n_rotz=md_alloc_double(length)) == NULL){goto memFree;}

	return(0);
memFree:
	md_n_free();
	return(ERR_MEM);
}

void md_e_free(void)
{
	md_free_int(e_id);
	md_free_int(e_type);
	md_free_int(e_n1);
	md_free_int(e_n2);
	md_free_double(e_E);
	md_free_double(e_A);
	md_free_double(e_I);
	md_free_double(e_va);
	md_free_double(e_vb);
	md_free_double(e_na);
	md_free_double(e_nb);
}

int md_e_alloc(int length)
{
	if ((e_id=md_alloc_int(length)) == NULL){goto memFree;}
	if ((e_type=md_alloc_int(length)) == NULL){goto memFree;}
	if ((e_n1=md_alloc_int(length)) == NULL){goto memFree;}
	if ((e_n2=md_alloc_int(length)) == NULL){goto memFree;}
	if ((e_E=md_alloc_double(length)) == NULL){goto memFree;}
	if ((e_A=md_alloc_double(length)) == NULL){goto memFree;}
	if ((e_I=md_alloc_double(length)) == NULL){goto memFree;}
	if ((e_va=md_alloc_double(length)) == NULL){goto memFree;}
	if ((e_vb=md_alloc_double(length)) == NULL){goto memFree;}
	if ((e_na=md_alloc_double(length)) == NULL){goto memFree;}
	if ((e_nb=md_alloc_double(length)) == NULL){goto memFree;}

	return(0);
memFree:
	md_e_free();
	return(ERR_MEM);
}

void md_res_free(void)
{
	md_free_double(nr_posx);
	md_free_double(nr_posy);
	md_free_double(nr_rotz);

	md_free_double(nr_fx);
	md_free_double(nr_fy);
	md_free_double(nr_mz);

	md_free_double(er_a_fx);
	md_free_double(er_a_fy);
	md_free_double(er_a_mz);

	md_free_double(er_b_fx);
	md_free_double(er_b_fy);
	md_free_double(er_b_mz);
}

int md_res_alloc(int n_len, int e_len)
{
	if ((nr_posx =md_alloc_double(n_len)) == NULL){goto memFree;}
	if ((nr_posy =md_alloc_double(n_len)) == NULL){goto memFree;}
	if ((nr_rotz =md_alloc_double(n_len)) == NULL){goto memFree;}

	if ((nr_fx =md_alloc_double(n_len)) == NULL){goto memFree;}
	if ((nr_fy =md_alloc_double(n_len)) == NULL){goto memFree;}
	if ((nr_mz =md_alloc_double(n_len)) == NULL){goto memFree;}

	if ((er_a_fx =md_alloc_double(e_len)) == NULL){goto memFree;}
	if ((er_a_fy =md_alloc_double(e_len)) == NULL){goto memFree;}
	if ((er_a_mz =md_alloc_double(e_len)) == NULL){goto memFree;}

	if ((er_b_fx =md_alloc_double(e_len)) == NULL){goto memFree;}
	if ((er_b_fy =md_alloc_double(e_len)) == NULL){goto memFree;}
	if ((er_b_mz =md_alloc_double(e_len)) == NULL){goto memFree;}

	return(0);
memFree:
	md_res_free();
	return(ERR_MEM);
}

/* tests if file exists (OK|ERR_EMP) or is valid (OK|ERR_IO,ERR_VAL) */
int read_test(char *ifname)
{
  long a ;
  int rv = OK ;

  errno = 0 ;
  ifile = NULL ;

	if ((ifile=fopen(ifname,"r"))==NULL){return(ERR_EMP);}

	fscanf(ifile,"%li\n", &a);
  if (a != 666777890) {rv = ERR_VAL; goto memFree;} 

  fscanf(ifile, "%li", &a) ;
  if (errno != 0) {rv = ERR_IO ; goto memFree;}
  if (a <= 0) {rv = ERR_VAL ; goto memFree;}

  fscanf(ifile, "%li", &a) ;
  if (errno != 0) {rv = ERR_IO ; goto memFree;}
  if (a <= 0) {rv = ERR_VAL ; goto memFree;}

  fscanf(ifile, "%li", &a) ;
  if (errno != 0) {rv = ERR_IO ; goto memFree;}
  if (a <= 0) {rv = ERR_VAL ; goto memFree;}

memFree:
  fclose(ifile);
  return(rv);
}

int read_data(char *ifname)
{
	int i ;
  long id ;

  ifile = NULL ;
  errno = 0 ;

	if (ifname == NULL) {return(ERR_EMP);}

	if ((ifile=fopen(ifname,"r"))==NULL){return(ERR_IO);}

	fscanf(ifile,"%li\n", &id);

  if (id != 666777890) { fclose(ifile); return(ERR_VAL);} /* invalid file */

	fscanf(ifile,"%i %i %i %lf", &n_len, &e_len, &num_div, &gridReal) ;

	if (n_len <= 0) { fclose(ifile); return(ERR_EMP); }
	if (e_len <= 0) { fclose(ifile); return(ERR_EMP); }
	if (num_div <= 0) {num_div = 1 ; }
	if (gridReal <= 0.0) {gridReal = 0.5 ; }

	if (md_n_alloc(md_max_nodes) != OK) 
	{
		fclose(ifile);
		return(ERR_MEM);
	}
  
  if (md_e_alloc(md_max_elems) != OK) 
	{
		fclose(ifile);
		return(ERR_MEM);
	}
  
  if (md_res_alloc(md_max_nodes, md_max_elems) != OK) 
	{
		fclose(ifile);
		return(ERR_MEM);
	}

	/* nodes */
	for (i=0; i<n_len; i++)
	{
		fscanf(ifile,"%i %lf %lf %i %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&n_id[i],
				&n_x[i],
				&n_y[i],
				&n_dtype[i],
				&n_fx[i],
				&n_fy[i],
				&n_mz[i],
				&n_posx[i],
				&n_posy[i],
				&n_rotz[i],
        
        &nr_posx[i],
        &nr_posy[i],
        &nr_rotz[i],
        &nr_fx[i],
        &nr_fy[i],
        &nr_mz[i]
				);
    if (errno != 0) {fclose(ifile); goto memFree;}
/*printf("n: %i %e %e %i %e %e %e %e %e %e\n", n_id[i], n_x[i], n_y[i], n_dtype[i], n_fx[i], n_fy[i], n_mz[i], n_posx[i], n_posy[i], n_rotz[i] );*/
	}

	/* elements */
	for (i=0; i<e_len; i++)
	{
		fscanf(ifile,"%i %i %i %i %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&e_id[i],
				&e_type[i],
				&e_n1[i],
				&e_n2[i],
				&e_E[i],
				&e_A[i],
				&e_I[i],
				&e_na[i],
				&e_nb[i],
				&e_va[i],
				&e_vb[i],

        &er_a_fx[i],
        &er_a_fy[i],
        &er_a_mz[i],
        &er_b_fx[i],
        &er_b_fy[i],
        &er_b_mz[i]
				);
    if (errno != 0) {fclose(ifile); goto memFree;}
/*printf("%i %i %i %i %e %e %e %e %e %e %e \n", e_id[i], e_type[i], e_n1[i], e_n2[i], e_E[i], e_A[i], e_I[i], e_na[i], e_nb[i], e_va[i], e_vb[i]);*/
	}
#if 0
#endif
	if ((fclose(ifile))!=0){goto memFree;}

	return(OK) ;

memFree:
  md_make_empty();
  md_alloc_empty();
	return(ERR_IO) ;
}

int write_data(char *ofname)
{
	int i ;

	if ((n_len <= 0)|| (e_len <= 0)){return(ERR_EMP);}

	if (ofname == NULL) {return(ERR_EMP);}

	if ((ofile=fopen(ofname,"w"))==NULL){return(ERR_IO);}

	fprintf(ofile,"666777890\n");

	fprintf(ofile,"%i %i %i %e\n", n_len, e_len, num_div, gridReal) ;
	

	/* nodes */
	for (i=0; i<n_len; i++)
	{
		fprintf(ofile,"%i %e %e %i %e %e %e %e %e %e %e %e %e %e %e %e\n",
				n_id[i],
				n_x[i],
				n_y[i],
				n_dtype[i],
				n_fx[i],
				n_fy[i],
				n_mz[i],
				n_posx[i],
				n_posy[i],
				n_rotz[i],
        
        nr_posx[i],
        nr_posy[i],
        nr_rotz[i],
        nr_fx[i],
        nr_fy[i],
        nr_mz[i]
				);
	}

	/* elements */
	for (i=0; i<e_len; i++)
	{
		fprintf(ofile,"%i %i %i %i %e %e %e %e %e %e %e %e %e %e %e %e %e\n",
				e_id[i],
				e_type[i],
				e_n1[i],
				e_n2[i],
				e_E[i],
				e_A[i],
				e_I[i],
				e_na[i],
				e_nb[i],
				e_va[i],
				e_vb[i],

        er_a_fx[i],
        er_a_fy[i],
        er_a_mz[i],
        er_b_fx[i],
        er_b_fy[i],
        er_b_mz[i]
				);
	}
	if ((fclose(ofile))!=0){return(ERR_IO);}
	return(OK) ;
}

/* Deletes one node (actually only moves things) */
int md_del_node(int pos)
{
  int i ;

  if (pos >= n_len)
  {
    return(ERR_VAL);
  }

	/* tests if node is used in any element */
	for (i=0; i<e_len; i++)
	{
		if (e_n1[i] == pos) {return(ERR);}
		if (e_n2[i] == pos) {return(ERR);}
	}

  for (i=pos; i<n_len; i++)
  {
    if (i == (n_len-1))
    {    
      n_id[i]    = 0 ;
			n_x[i]     = 0 ;
			n_y[i]     = 0 ;
			n_dtype[i] = 0 ;
			n_fx[i]    = 0 ;
			n_fy[i]    = 0 ;
			n_mz[i]    = 0 ;
			n_posx[i]  = 0 ;
			n_posy[i]  = 0 ;
			n_rotz[i]  = 0 ;
        
      nr_posx[i] = 0 ;
      nr_posy[i] = 0 ;
      nr_rotz[i] = 0 ;
      nr_fx[i]   = 0 ;
      nr_fy[i]   = 0 ;
      nr_mz[i]   = 0 ;
    }
    else
    {
      n_id[i]    = n_id[i+1] ;
			n_x[i]     = n_x[i+1] ;
			n_y[i]     = n_y[i+1] ;
			n_dtype[i] = n_dtype[i+1] ;
			n_fx[i]    = n_fx[i+1] ;
			n_fy[i]    = n_mz[i+1] ;
			n_mz[i]    = n_mz[i+1] ;
			n_posx[i]  = n_posx[i+1] ;
			n_posy[i]  = n_posy[i+1] ;
			n_rotz[i]  = n_rotz[i+1] ;
        
      nr_posx[i] = nr_posx[i+1] ;
      nr_posy[i] = nr_posy[i+1] ;
      nr_rotz[i] = nr_rotz[i+1] ;
      nr_fx[i]   = nr_fx[i+1] ;
      nr_fy[i]   = nr_fy[i+1] ;
      nr_mz[i]   = nr_mz[i+1] ;
    }
  }

  n_len-- ;

	/* sets things in elements */
	for (i=0; i<e_len; i++)
	{
		if (e_n1[i] >= pos) {e_n1[i]-- ;}
		if (e_n2[i] >= pos) {e_n2[i]-- ;}
	}

  return(OK);
}

/* Deletes one node (actually only moves things) */
int md_del_elem(int pos)
{
  int i ;

  if (pos >= e_len)
  {
    return(ERR_VAL);
  }

  for (i=pos; i<e_len; i++)
  {
    if (i == (e_len-1))
    {    
      e_id[i]     = 0 ;
			e_type[i]   = 0 ;
			e_n1[i]     = 0 ;
			e_n2[i]     = 0 ;
			e_E[i]      = 0 ;
			e_A[i]      = 0 ;
			e_I[i]      = 0 ;
			e_na[i]     = 0 ;
			e_nb[i]     = 0 ;
			e_va[i]     = 0 ;
			e_vb[i]     = 0 ;

      er_a_fx[i]  = 0 ;
      er_a_fy[i]  = 0 ;
      er_a_mz[i]  = 0 ;
      er_b_fx[i]  = 0 ;
      er_b_fy[i]  = 0 ;
      er_b_mz[i]  = 0 ;
    }
    else
    {
      e_id[i]     = e_id[i+1] ;
			e_type[i]   = e_type[i+1] ;
			e_n1[i]     = e_n1[i+1] ;
			e_n2[i]     = e_n2[i+1] ;
			e_E[i]      = e_E[i+1] ;
			e_A[i]      = e_A[i+1] ;
			e_I[i]      = e_I[i+1] ;
			e_na[i]     = e_na[i+1] ;
			e_nb[i]     = e_nb[i+1] ;
			e_va[i]     = e_va[i+1] ;
			e_vb[i]     = e_vb[i+1] ;

      er_a_fx[i]  = er_a_fx[i+1] ;
      er_a_fy[i]  = er_a_fy[i+1] ;
      er_a_mz[i]  = er_a_mz[i+1] ;
      er_b_fx[i]  = er_b_fx[i+1] ;
      er_b_fy[i]  = er_b_fy[i+1] ;
      er_b_mz[i]  = er_b_mz[i+1] ;
    }
  }

  e_len-- ;

  return(OK);
}


void md_make_empty(void)
{
  n_len = 0 ;
  e_len = 0 ;

  md_n_free();
  md_e_free();
  md_res_free();
}

int md_alloc_empty(void)
{
  if ((md_n_alloc(md_max_nodes)) != OK) {goto memFree;}
  if ((md_e_alloc(md_max_elems)) != OK) {goto memFree;}
  if ((md_res_alloc(md_max_nodes, md_max_elems)) != OK) {goto memFree;}

  n_len = 0 ;
  e_len = 0 ;

  return(OK);
memFree:
  md_n_free();
  md_e_free();
  md_res_free();
  return(ERR_MEM);
}

int md_add_node(double x, double y)
{
  int id ;

  if ((n_len+1) >= md_max_nodes) {return(ERR_MEM);}

  if (n_len < 1)
  {
    id = 1 ;
  }
  else
  {
    id = n_id[n_len-1] + 1 ;
  }

  n_id[n_len] = id ;
  n_x[n_len]  = x ;
  n_y[n_len]  = y ;

  n_len++ ;

#ifdef DEVEL_VERBOSE
  fprintf(stderr,"n[%i]: %e,%e\n",
      id,x,y);
#endif

  return(OK);
}

int md_add_elem(int type, int n1, int n2, double E, double A, double I)
{
  int id ;

  if ((e_len+1) >= md_max_elems) {return(ERR_MEM);}
  if ((n1 <0) || (n2 < 0)) {return(ERR_VAL);}

  if (e_len < 1)
  {
    id = 1 ;
  }
  else
  {
    id = e_id[e_len-1] + 1 ;
  }

  e_id[e_len]   = id ;
  e_type[e_len] = type ;
  e_n1[e_len]   = n1 ;
  e_n2[e_len]   = n2 ;
  e_E[e_len]    = E ;
  e_A[e_len]    = A ;
  e_I[e_len]    = I ;

  e_len++ ;

#ifdef DEVEL_VERBOSE
  fprintf(stderr,"e[%i]: t=%i (%i,%i) E=%e A=%e I=%e\n",
      id,type,n1,n2,E,A,I);
#endif
  return(OK);
}

int md_add_disp(int node_pos, int type, double posx, double posy, double rotz)
{
  if ((n_len) <= node_pos)      {return(ERR_VAL);}
  if ((type < 1) || (type > 6)) {return(ERR_VAL);}

  n_dtype[ node_pos ] = type ;
  n_posx[  node_pos ] = posx ;
  n_posy[  node_pos ] = posy ;
  n_rotz[  node_pos ] = rotz ;

#ifdef DEVEL_VERBOSE
  fprintf(stderr,"d[%i]: t=%i (ux=%e,uy=%e,rotz=%e)\n",
      node_pos,type,posx,posy,rotz);
#endif
  return(OK);
}

int md_del_disp(int node_pos)
{
  if ((n_len) <= node_pos)      {return(ERR_VAL);}

  n_dtype[ node_pos ] = 0 ;
  n_posx[  node_pos ] = 0 ;
  n_posy[  node_pos ] = 0 ;
  n_rotz[  node_pos ] = 0 ;

  return(OK);
}

int md_add_force(int node_pos, double fx, double fy, double mz)
{
  if ((n_len) <= node_pos) {return(ERR_VAL);}

  n_fx[ node_pos ] = fx ;
  n_fy[ node_pos ] = fy ;
  n_mz[ node_pos ] = mz ;

#ifdef DEVEL_VERBOSE
  fprintf(stderr,"f[%i]: (Fx=%e,Fy=%e,Mz=%e)\n",
      node_pos,fx,fy,mz);
#endif
  return(OK);
}

int md_del_force(int node_pos)
{
  if ((n_len) <= node_pos) {return(ERR_VAL);}

  n_fx[ node_pos ] = 0 ;
  n_fy[ node_pos ] = 0 ;
  n_mz[ node_pos ] = 0 ;

  return(OK);
}

int md_add_n_eload(int elem_pos, double na, double nb)
{
  if ((e_len) <= elem_pos) {return(ERR_VAL);}
  if (0 > elem_pos) {return(ERR_VAL);}

  e_na[ elem_pos ] = na ;
  e_nb[ elem_pos ] = nb ;

#ifdef DEVEL_VERBOSE
  fprintf(stderr,"e-n[%i]: (na=%e,nb=%e)\n",
      elem_pos,na,nb);
#endif
  return(OK);
}

int md_del_n_eload(int elem_pos)
{
  if ((e_len) <= elem_pos) {return(ERR_VAL);}

  e_na[ elem_pos ] = 0 ;
  e_nb[ elem_pos ] = 0 ;

  return(OK);
}

int md_add_v_eload(int elem_pos, double va, double vb)
{
  if ((e_len) <= elem_pos) {return(ERR_VAL);}

  e_va[ elem_pos ] = va ;
  e_vb[ elem_pos ] = vb ;

  return(OK);
}

int md_del_v_eload(int elem_pos)
{
  if ((e_len) <= elem_pos) {return(ERR_VAL);}

  e_va[ elem_pos ] = 0 ;
  e_vb[ elem_pos ] = 0 ;

  return(OK);
}

char *md_intstring(int number)
{
  static char str[2048] ;
  int  i ;

  for (i=0; i<2048; i++) { str[i] = '\0' ; }
  sprintf(str,"%i", number);
  return(str);
}

char *md_double2string01(double number)
{
  static char str[2048] ;
  int  i ;

  for (i=0; i<2048; i++) { str[i] = '\0' ; }
  sprintf(str,"%.1f", number);
  return(str);
}

char *md_double2string04(double number)
{
  static char str[2048] ;
  int  i ;

  for (i=0; i<2048; i++) { str[i] = '\0' ; }
  sprintf(str,"%.4f", number);
  return(str);
}

/* output of results */

int md_print_reactions(FILE *fw)
{
  int i ;
  double sFx, sFy, sMz ;

  sFx = 0.0 ;
  sFy = 0.0 ;
  sMz = 0.0 ;

  fprintf(fw, "\n%s:\n\n", "REACTIONS") ;
  for (i=0; i<n_len; i++)
  {
    switch (n_dtype[i])
    {
      case 0: break ;
      case 1: fprintf(fw,"%s: %i\n", "Node", n_id[i]);
              fprintf(fw,"  Fx = %e\n",nr_fx[i]); sFx += nr_fx[i] ;
              break ;
      case 2: fprintf(fw,"%s: %i\n", "Node", n_id[i]);
              fprintf(fw,"  Fy = %e\n",nr_fy[i]); sFy += nr_fy[i] ;
              break ;
      case 3: fprintf(fw,"%s: %i\n", "Node", n_id[i]);
              fprintf(fw,"  Fx = %e\n", nr_fx[i]); sFx += nr_fx[i] ;
              fprintf(fw,"  Fy = %e\n", nr_fy[i]); sFy += nr_fy[i] ;
              break ;
      case 4: fprintf(fw,"%s: %i\n", "Node", n_id[i]);
              fprintf(fw,"  Fx = %e\n", nr_fx[i]); sFx += nr_fx[i] ;
              fprintf(fw,"  Mz = %e\n", nr_mz[i]); sMz += nr_mz[i] ;
              break ;
      case 5: fprintf(fw,"%s: %i\n", "Node", n_id[i]);
              fprintf(fw,"  Fy = %e\n", nr_fy[i]); sFy += nr_fy[i] ;
              fprintf(fw,"  Mz = %e\n", nr_mz[i]); sMz += nr_mz[i] ;
              break ;
      case 6: fprintf(fw,"%s: %i\n", "Node", n_id[i]);
              fprintf(fw,"  Fx = %e\n", nr_fx[i]); sFx += nr_fx[i] ;
              fprintf(fw,"  Fy = %e\n", nr_fy[i]); sFy += nr_fy[i] ;
              fprintf(fw,"  Mz = %e\n", nr_mz[i]); sMz += nr_mz[i] ;
              break ;
    }
  }

  fprintf(fw, "\n%s: Fx = %e, Fy= %e, Mz = %e\n", "Sums", sFx, sFy, sMz);

  return(OK);
}

/*provides filaname with changed extension to "ext" */
char *md_set_file_ext(char *str, char *lab, char *ext)
{
  char  *tmp   = NULL ;
  int   tmplen = 0 ;
  int   dotpos = -1 ;
  int   empty  = 0 ;
  int   i ;

  if (str == NULL) {return(NULL);}

  if (strlen(ext) < 1) {return("ERR-EXT.txt");}
  if (strlen(lab) < 1) {return("ERR-LAB.txt");}

  if (strlen(str) >= 1)
  {
    for (i=(strlen(str)-1); i>=0; i--)
    {
      if (str[i] == '.')
      {
        dotpos = i ; break ;
      }
    }
  }

  if (dotpos == -1)
  {
    if (strlen(str) >= 1) /* name.ext */
    {
      dotpos = strlen(str) ;
      tmplen = strlen(lab) + dotpos + 3 + strlen(ext) ;
    }
    else /* "m.ext" */
    {
      tmplen =  strlen(lab) + 4 + strlen(ext) ;
      dotpos = 1 ;
      empty = 1 ;
    }
  }
  else /* "name.ext" */
  {
    tmplen = dotpos + strlen(lab) + 3 + strlen(ext) ;
  }

  if ((tmp=(char *)malloc(tmplen*sizeof(char))) == NULL)
  {
    return(ext);
  }

  for (i=0; i<tmplen; i++) { tmp[i] = '\0' ; }

  if (empty == 1)
  {
    tmp[0] = 'm' ;
  }
  else
  {
    for (i=0; i<dotpos; i++) { tmp[i] = str[i] ; }
  }

  for (i = dotpos; i< (dotpos + strlen(lab)+1); i++)
  {
    tmp[i] = lab[i-dotpos] ;
  }

  tmp[dotpos+strlen(lab)] = '.' ;

  for (i = (dotpos + strlen(lab)+1); i<tmplen; i++)
  {
    if (i > dotpos+strlen(ext)+strlen(lab)) {break;}
    tmp[i] = ext[i-dotpos-strlen(lab)-1] ;
  }


  if (tmp == NULL)
  {
    return(ext);
  }
  else
  {
    return(tmp);
  }
}

/* *** TEXT OUTPUT: *** */

void md_file_begin(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: break ;
    case MDOUT_HTML: fprintf(fw, "<html>\n");
                     fprintf(fw, "<body>\n");
                     break ;
    case MDOUT_LATEX: fprintf(fw, "\\documentclass{report}\n");
                     fprintf(fw, "\\usepackage{graphicx}\n");

                     fprintf(fw, "\\voffset = -37 mm\n");
                     fprintf(fw, "\\textheight = 264 mm\n");
                     fprintf(fw, "\\hoffset = -18 mm\n");
                     fprintf(fw, "\\textwidth = 172 mm\n");
                     fprintf(fw, "\\footskip = 18 mm\n");

                     fprintf(fw, "\\begin{document}\n");
                     break ;
  }
}

void md_file_end(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: fprintf(fw,"\n");
                     break ;
    case MDOUT_HTML: fprintf(fw, "</body>\n");
                     fprintf(fw, "</html>\n");
                     break ;
    case MDOUT_LATEX: fprintf(fw, "\\end{document}\n");
                     break ;
  }
}

void md_table_header(FILE *fw, int term, int cols)
{
  int i ;
  switch (term)
  {
    case MDOUT_TEXT: break ;
    case MDOUT_HTML: fprintf(fw, "<table border=\"1\">\n"); 
                     break ;
    case MDOUT_LATEX: 
                     fprintf(fw,"\\begin{tabular}{|");
                     for (i=0; i<cols; i++) { fprintf(fw,"r"); }
                     fprintf(fw,"|}\n\\hline\n");
                     break ;
  }
}
void md_table_tr_beg(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: break ;
    case MDOUT_HTML: fprintf(fw,"<tr> "); break ;
    case MDOUT_LATEX: break ;
  }
}
void md_table_tr_end(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: fprintf(fw,"\n"); break ;
    case MDOUT_HTML: fprintf(fw,"</td></tr>\n"); break ;
    case MDOUT_LATEX: fprintf(fw,"\\\\ \n"); break ;
  }
}
void md_table_td_beg(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: fprintf(fw," "); break ;
    case MDOUT_HTML:  fprintf(fw,"<td>"); break ;
    case MDOUT_LATEX:  fprintf(fw," "); break ;
  }
}
void md_table_td_end(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: fprintf(fw," "); break ;
    case MDOUT_HTML:  fprintf(fw,"</td>"); break ;
    case MDOUT_LATEX: fprintf(fw,"& "); break ;
  }
}

void md_table_foot(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: fprintf(fw,"\n"); break ;
    case MDOUT_HTML: fprintf(fw, "</table>\n"); 
                     break ;
    case MDOUT_LATEX: fprintf(fw,"\\hline\n\\end{tabular}\n");
                     break ;
  }
}

void md_par_beg(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: fprintf(fw, "\n"); break ;
    case MDOUT_HTML:  fprintf(fw, "<p>\n"); break ;
    case MDOUT_LATEX: fprintf(fw, "\n"); break ;
  }
}
void md_par_end(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: fprintf(fw, "\n"); break ;
    case MDOUT_HTML:  fprintf(fw, "</p>\n"); break ;
    case MDOUT_LATEX: fprintf(fw, "\n"); break ;
  }
}

void md_header_1(FILE *fw, int term, char *header)
{
  int i ;
  switch (term)
  {
    case MDOUT_TEXT: fprintf(fw, " %s \n ",header);
                     for (i=0; i<strlen(header); i++)
                         { fprintf(fw,"="); }
                     fprintf(fw,"\n\n");
                     break ;
    case MDOUT_HTML: fprintf(fw, "<h1>%s</h1>\n\n",header); break ;
    case MDOUT_LATEX:  fprintf(fw, "\\section*{%s}\n\n",header); break ;
  }
}
void md_header_2(FILE *fw, int term, char *header)
{
  int i ;
  switch (term)
  {
    case MDOUT_TEXT: fprintf(fw, " %s \n ",header);
                     for (i=0; i<strlen(header); i++)
                         { fprintf(fw,"-"); }
                     fprintf(fw,"\n\n");
                     break ;
    case MDOUT_HTML: fprintf(fw, "<h2>%s</h2>\n\n",header); break ;
    case MDOUT_LATEX:  fprintf(fw, "\\subsection*{%s}\n\n",header); break ;
  }
}
void md_header_3(FILE *fw, int term, char *header)
{
  int i ;
  switch (term)
  {
    case MDOUT_TEXT: fprintf(fw, " %s \n ",header);
                     for (i=0; i<strlen(header); i++)
                         { fprintf(fw,"."); }
                     fprintf(fw,"\n\n");
                     break ;
    case MDOUT_HTML: fprintf(fw, "<h3>%s</h3>\n\n",header); break ;
    case MDOUT_LATEX:  fprintf(fw, "\\subsubsection*{%s}\n\n",header); break ;
  }
}

void md_bold_beg(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: break ;
    case MDOUT_HTML:  fprintf(fw,"<b>"); break ;
    case MDOUT_LATEX:  fprintf(fw,"{\\bf "); break ;
  }
}
void md_bold_end(FILE *fw, int term)
{
  switch (term)
  {
    case MDOUT_TEXT: break ;
    case MDOUT_HTML:  fprintf(fw,"</b>"); break ;
    case MDOUT_LATEX: fprintf(fw,"} "); break ;
  }
}


void md_img(FILE *fw, int term, char *img, char *id, char *comment)
{
  switch (term)
  {
    case MDOUT_TEXT: break ;
    case MDOUT_HTML: 
#ifdef GDGUI
                      fprintf(fw,"<img src=\"%s\" width=\"100%%\" alt=\"%s\">\n",
                         md_set_file_ext(img,id,"png"),
                         comment);
#endif
                     break ;
    case MDOUT_LATEX:
#ifdef PSGUI
                     fprintf(fw,"\n\\begin{center}\n\\includegraphics[width=120mm]{%s}\n\\end{center}\n", 
                         md_set_file_ext(img,id,"ps")
                         );
#if 0
                     fprintf(fw,"\\\\{\\bf %s}\n", comment);
#endif
#endif
                     break ;
  }
}

char *md_rep_ext(int term)
{
  switch(term)
  {
    case MDOUT_TEXT: return("txt"); break ;
    case MDOUT_HTML: return("html"); break ;
    case MDOUT_LATEX: return("tex"); break ;
  }
  return("txt");
}

char *md_write_e_type(int type)
{
  switch(type)
  {
    case 0: return("|--|"); break ;
    case 1: return("o--|"); break ;
    case 2: return("o--|"); break ;
    case 3: return("o--o"); break ;
  }
  return("|--|");
}

void md_get_num_loads(int *disps, int *forces, int *eloads)
{
  int i ;

  *disps  = 0 ;
  *forces = 0 ;
  *eloads = 0 ;

  for (i=0; i<n_len; i++)
  {
    if (n_dtype[i] > 0) { *disps += 1; }
    if ((n_fx[i] != 0.0)||(n_fy[i] != 0.0)||(n_mz[i] != 0.0)) { *forces += 1; }
  }
  
  for (i=0; i<e_len; i++)
  {
    if ((e_na[i] != 0.0)||(e_nb[i] != 0.0)||(e_va[i] != 0.0)||(e_vb[i] != 0.0)) { *eloads += 1; }
  }
}

char *md_get_disp_type(int type)
{
  switch (type)
  {
    case 0 : return(" ");
    case 1 : return("Ux");
    case 2 : return("Uy");
    case 3 : return("Ux, Uy ");
    case 4 : return("Ux, ROTz ");
    case 5 : return("Uy, ROTz ");
    case 6 : return("Ux, Uy, ROTz ");
  }
  return(" ");
}

int md_write_report(
                      char *fname, 
                      int term, 
                      int print_input, 
                      int print_ke, 
                      int  print_k, 
                      int print_res, 
                      int print_rlist,
                      int print_simple
                      )
{
  FILE *fw  = NULL ;
  int   i ;
  int disps, forces, eloads ;
  double max, min, xmax, xmin, beg, end, L, defmax, defx ;
  int    ixmax, ixmin, idmaz ;

#ifdef MDDOS
  if ((fw=fopen(md_set_file_ext(fname,"-r",md_rep_ext(term)),"w")) == NULL)
#else
  if ((fw=fopen(md_set_file_ext(fname,"-report",md_rep_ext(term)),"w")) == NULL)
#endif
     { return(ERR_IO) ; }


  md_file_begin(fw, term);

  md_header_1(fw, term, "MicroDef Report Summary");

  md_img(fw, term, fname, "S", "Structure");

  if (print_input == 1)
  {
    md_header_1(fw, term, "Input Data");

    md_header_2(fw, term, "Nodes");
      md_table_header(fw, term, 3);
        md_table_tr_beg(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Number");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"X");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Y");
            md_bold_end(fw,term);

        md_table_tr_end(fw,term);

          for (i=0 ;i<n_len; i++)
          {
            md_table_tr_beg(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", n_id[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", n_x[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", n_y[i]);
            md_table_tr_end(fw,term);
          }

      md_table_foot(fw, term);

    if (print_simple != 1)
    {
    md_header_2(fw, term, "Elements");
      md_table_header(fw, term, 7);
        md_table_tr_beg(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Number");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Node 1");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Node 2");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"A");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"I");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"E");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Type");
            md_bold_end(fw,term);

        md_table_tr_end(fw,term);

          for (i=0 ;i<e_len; i++)
          {
            md_table_tr_beg(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", e_id[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", e_n1[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", e_n2[i]);
              md_table_td_end(fw,term);

              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", e_A[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", e_I[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", e_E[i]);
              md_table_td_end(fw,term);

              md_table_td_beg(fw,term);
              fprintf(fw,"%s", md_write_e_type(e_type[i]));
            md_table_tr_end(fw,term);
          }

      md_table_foot(fw, term);

    md_get_num_loads(&disps, &forces, &eloads);

    if (forces > 0)
    {
      md_header_2(fw, term, "Forces");
  
      
      md_table_header(fw, term, 4);
        md_table_tr_beg(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Node");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Fx");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Fy");
            md_bold_end(fw,term);
 
          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Mz");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

        md_table_tr_end(fw,term);

          for (i=0 ;i<n_len; i++)
          {
            if ((n_fx[i] != 0.0)||(n_fy[i] != 0.0)||(n_mz[i] != 0.0)) 
            {
            md_table_tr_beg(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", n_id[i]);
              md_table_td_end(fw,term);

              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", n_fx[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", n_fy[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", n_mz[i]);

            md_table_tr_end(fw,term);
            }
          }

      md_table_foot(fw, term);
    }

    if (eloads > 0)
    {
      md_header_2(fw, term, "Element Loads");

       
      md_table_header(fw, term, 5);
        md_table_tr_beg(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Element");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"na");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"nb");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"va");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);
 
          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"vb");
            md_bold_end(fw,term);

        md_table_tr_end(fw,term);

          for (i=0 ;i<e_len; i++)
          {
            if ((e_na[i] != 0.0)||(e_nb[i] != 0.0)||(e_va[i] != 0.0)||(e_vb[i] != 0.0))
            {
            md_table_tr_beg(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", e_id[i]);
              md_table_td_end(fw,term);

              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", e_na[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", e_nb[i]);
              md_table_td_end(fw,term);

              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", e_va[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", e_vb[i]);

            md_table_tr_end(fw,term);
            }
          }

      md_table_foot(fw, term);

    }

    if (disps > 0)
    {
      md_header_2(fw, term, "Supports");
      
      md_table_header(fw, term, 5);
        md_table_tr_beg(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Node");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Type");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Ux");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Uy");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);
 
          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"ROTz");
            md_bold_end(fw,term);

        md_table_tr_end(fw,term);

          for (i=0 ;i<n_len; i++)
          {
            if (n_dtype[i] > 0)
            {
            md_table_tr_beg(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", n_id[i]);
              md_table_td_end(fw,term);

              md_table_td_beg(fw,term);
              fprintf(fw,"%s", md_get_disp_type(n_dtype[i]));
              md_table_td_end(fw,term);

              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", n_posx[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", n_posy[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", n_rotz[i]);

            md_table_tr_end(fw,term);
            }
          }

      md_table_foot(fw, term);
    }
  }
  }

  if (print_simple != 1)
  {
  if (print_ke == 1)
  {
    md_header_1(fw, term, "Element Stiffness Matrices And Load Vectors");
  }

  if (print_k == 1)
  {
    md_header_1(fw, term, "Element Stiffness Matrices And Load Vectors");
  }


  if (print_res == 1)
  {
    md_header_1(fw, term, "Results");

    md_header_2(fw, term, "Displacements of nodes");
      md_table_header(fw, term, 4);
        md_table_tr_beg(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Number");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Ux");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Uy");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"ROTz");
            md_bold_end(fw,term);


        md_table_tr_end(fw,term);

          for (i=0 ;i<n_len; i++)
          {
            md_table_tr_beg(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", n_id[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%2.8f", nr_posx[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%2.8f", nr_posy[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%2.8f", nr_rotz[i]);
            md_table_tr_end(fw,term);
          }

      md_table_foot(fw, term);
    }
    }
    md_header_3(fw, term, "Reactions and deformations");
    md_img(fw, term, fname, "RD", "Reactions and deformations");

    if (print_simple != 1)
    {
    md_header_2(fw, term, "Reactions");
      md_table_header(fw, term, 4);
        md_table_tr_beg(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Number");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Fx");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Fy");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Mz");
            md_bold_end(fw,term);


        md_table_tr_end(fw,term);

          for (i=0 ;i<n_len; i++)
          {
            md_table_tr_beg(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", n_id[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", nr_fx[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", nr_fy[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", nr_mz[i]);
            md_table_tr_end(fw,term);
          }

      md_table_foot(fw, term);



    
    md_header_2(fw, term, "Internal Forces");

      md_table_header(fw, term, 8);
        md_table_tr_beg(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Element");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Type");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);


          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"x min.");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"min");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"x max ");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"max");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"begin");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"end");
            md_bold_end(fw,term);

          md_table_tr_end(fw,term);



    for (i=0; i<e_len; i++)
    {
      L = compute_L(i);

      /* N */
      beg = compute_e_res(1, i, 10, 0) ;
      end = compute_e_res(1, i, 10, 10) ;
      max =  get_max_NVM_res(1, i, 100, &ixmax) ;
      min =  get_min_NVM_res(1, i, 100, &ixmin) ;
      xmax = ((double)ixmax / 100.0)* L ;
      xmin = ((double)ixmin / 100.0)* L ;

            md_table_tr_beg(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", e_id[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"N");
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", xmin);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", min);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", xmax);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", max);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", beg);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", end);
            md_table_tr_end(fw,term);


      /* V */
      beg = compute_e_res(2, i, 10, 0) ;
      end = compute_e_res(2, i, 10, 10) ;
      max =  get_max_NVM_res(2, i, 100, &ixmax) ;
      min =  get_min_NVM_res(2, i, 100, &ixmin) ;
      xmax = ((double)ixmax / 100.0)* L ;
      xmin = ((double)ixmin / 100.0)* L ;

            md_table_tr_beg(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", e_id[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"V");
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", xmin);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", min);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", xmax);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", max);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", beg);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", end);
            md_table_tr_end(fw,term);



      /* M */
      beg = compute_e_res(3, i, 10, 0) ;
      end = compute_e_res(3, i, 10, 10) ;
      max =  get_max_NVM_res(3, i, 100, &ixmax) ;
      min =  get_min_NVM_res(3, i, 100, &ixmin) ;
      xmax = ((double)ixmax / 100.0)* L ;
      xmin = ((double)ixmin / 100.0)* L ;

            md_table_tr_beg(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%i", e_id[i]);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"M");
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", xmin);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", min);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", xmax);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", max);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", beg);
              md_table_td_end(fw,term);
              md_table_td_beg(fw,term);
              fprintf(fw,"%8.3f", end);
            md_table_tr_end(fw,term);
    }
    
    md_table_foot(fw, term);

    /* Maximum local deformations */
    md_header_2(fw, term, "Local deformations");

      md_table_header(fw, term, 3);
        md_table_tr_beg(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Element");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);

          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"Deformation");
            md_bold_end(fw,term);
          md_table_td_end(fw,term);


          md_table_td_beg(fw,term);
            md_bold_beg(fw,term);
            fprintf(fw,"x");
            md_bold_end(fw,term);

          md_table_tr_end(fw,term);

    for (i=0; i<e_len; i++)
    {
      defmax =  get_max_def_res(1, i, 100, &idmaz, &defx) ;

      md_table_tr_beg(fw,term);
        md_table_td_beg(fw,term);
        fprintf(fw,"%i", e_id[i]);
        md_table_td_end(fw,term);

        md_table_td_beg(fw,term);
        fprintf(fw,"%8.6f", defmax);
        md_table_td_end(fw,term);

        md_table_td_beg(fw,term);
        fprintf(fw,"%8.3f", defx);

      md_table_tr_end(fw,term);

    }
    md_table_foot(fw, term);
    /* ------------------- */
    }

    md_header_3(fw, term, "Axial Forces");
    md_img(fw, term, fname, "N", "Axial forces");

    md_header_3(fw, term, "Shear Forces");
    md_img(fw, term, fname, "V", "Shear forces");

    md_header_3(fw, term, "Bending Moments");
    md_img(fw, term, fname, "M", "Bending Moments");

  md_file_end(fw, term);
  fclose(fw);
  return(OK);
}


/** Write data for Maxwell-Mohr integration  */
int md_write_maxwell( char *fname )
{
  FILE *fw  = NULL ;
  int   i, j ;
  int   e_div = 100 ;
  double L, dL ;

  if ((fw=fopen(md_set_file_ext(fname,"-mm","out"),"w")) == NULL)
     { return(ERR_IO) ; }

  fprintf(fw, "%i %i\n", e_len, e_div) ;

  for (i=0; i<e_len; i++)
  {
    L = compute_L(i);
    dL = L / ((double)e_div) ;

    for (j=0; j<e_div; j++)
    {
      fprintf(fw, "%4i %4i %e %e %e %e %e %e %e\n",
          i+1, j+1, dL,
          e_E[i], e_I[i], e_A[i],
          compute_e_res(1, i, e_div, j),
          compute_e_res(2, i, e_div, j),
          compute_e_res(3, i, e_div, j)
          ) ;
    }
  }

  fclose(fw);
  return(OK);
}


/** Write input (macro) file for the uFEM */
int md_write_ufem(char *fname)
{
  FILE *fw  = NULL ;
  double L,dx,dy ;
  int i ;

  if ((fw=fopen(md_set_file_ext(fname,"-u","mac"),"w")) == NULL)
     { return(ERR_IO) ; }

  /* nodes */
  for (i=0; i<n_len; i++)
  {
    fprintf(fw,"n,%i,%e,%e,0\n",n_id[i],n_x[i],n_y[i]);
  }

  /* elements with accompanying material and elem. types: */
  for (i=0; i<e_len; i++)
  {
    fprintf(fw,"mat,%i,1\n",i+1); /* material */
    fprintf(fw,"mp,ex,%i,%e\n",i+1,e_E[i]); 
    fprintf(fw,"mp,prxy,%i,0.2\n",i+1); 

    if (e_type[i] == 3)
    {
      fprintf(fw,"et,%i,1\n", i+1); /* o--| */
      fprintf(fw,"rs,%i,1\n", i+1); 
    }
    else
    {
      fprintf(fw,"et,%i,3\n", i+1); /* |--| */
      fprintf(fw,"rs,%i,3\n", i+1);   
      fprintf(fw,"r,iy,%i,%e\n", i+1,e_I[i]); 
      fprintf(fw,"r,hinge_a,%i,1\n", i+1); 
      fprintf(fw,"r,hinge_b,%i,1\n", i+1); 
      if (e_type[i] == 1) fprintf(fw,"r,hinge_a,%i,0\n", i+1); 
      if (e_type[i] == 2) fprintf(fw,"r,hinge_b,%i,0\n", i+1); 
    }

    fprintf(fw,"r,area,%i,%e\n", i+1,e_A[i]); 
    fprintf(fw,"ep, %i, %i,%i,%i\n",e_id[i],i+1,i+1,i+1);
    fprintf(fw,"e,%i,%i,%i\n",e_id[i],e_n1[i]+1,e_n2[i]+1);
  }

  /* supports in nodes: */
  for (i=0; i<n_len; i++)
  {
    if (n_dtype[i] != 0)
    {
      switch(n_dtype[i])
      {
        case 1: fprintf(fw,"d,%i,ux,%e\n",n_id[i],n_posx[i]);
                break;
        case 2: fprintf(fw,"d,%i,uy,%e\n",n_id[i],n_posy[i]);
                break;
        case 3: fprintf(fw,"d,%i,ux,%e\n",n_id[i],n_posx[i]);
                fprintf(fw,"d,%i,uy,%e\n",n_id[i],n_posy[i]);
                break;
        case 4: fprintf(fw,"d,%i,ux,%e\n",n_id[i],n_posx[i]);
                fprintf(fw,"d,%i,uy,%e\n",n_id[i],n_posy[i]);
                fprintf(fw,"d,%i,rotz,%e\n",n_id[i],n_rotz[i]);
                break;
      
      }
    }
  }
  
  /* forces in nodes: */
  for (i=0; i<n_len; i++)
  {
    if (fabs(n_fx[i]) >= 1e-6) fprintf(fw,"f,%i,fx,%e\n",n_id[i],n_fx[i]);
    if (fabs(n_fy[i]) >= 1e-6) fprintf(fw,"f,%i,fy,%e\n",n_id[i],n_fy[i]);
    if (fabs(n_mz[i]) >= 1e-6) fprintf(fw,"f,%i,mz,%e\n",n_id[i],n_mz[i]);
  }

  /* TODO: continuous loads (uFEM doesn't accept it for 1D elements),..
   *  in any case, this code doesn't provide 100% compatible inputs
   *  (MicroDef work with local coordinates but uFEM with global ones)
   * */
  for (i=0; i<e_len; i++)
  {
    dx = n_x[e_n2[i]] -   n_x[e_n1[i]] ;
    dy = n_y[e_n2[i]] -   n_y[e_n1[i]] ;
    L = sqrt(dy*dy + dx*dx) ;

    if ((fabs(e_va[i]) >= 1e-6)||(fabs(e_va[i]) >= 1e-6))
    {
#if 0
      fprintf(fw,"el,%i,fy,%e,%e\n",e_id[i],e_va[i],e_vb[i]);
#else /* a pretty dirty workaround: */
     fprintf(fw,"f,%i,fy,%e,%i\n",e_n1[i]+1,e_va[i]*L/2.0,i+1);
     fprintf(fw,"f,%i,fy,%e,%i\n",e_n2[i]+1,e_vb[i]*L/2.0,i+1);
#endif
    }
    if ((fabs(e_na[i]) >= 1e-6)||(fabs(e_na[i]) >= 1e-6))
    {
#if 0
      fprintf(fw,"el,%i,fy,%e,%e\n",e_id[i],-e_na[i],-e_nb[i]);
#else /* a pretty dirty workaround: */
     fprintf(fw,"f,%i,fy,%e,%i\n",e_n1[i]+1,-e_na[i]*L/2.0,i+1);
     fprintf(fw,"f,%i,fy,%e,%i\n",e_n2[i]+1,-e_vb[i]*L/2.0,i+1);
#endif
    }

  }

  fclose(fw);
  return(OK);
}

/* end of mdunxio.c */
