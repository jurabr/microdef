/*
   File name: md_cli.c
   Date:      2012/06/03 21:32
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
*/

#define CNV_SIZE_X 800
#define CNV_SIZE_Y 600
#define MD_ZERO 0.000001

#include "microdef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

extern int solve(void);

extern void set_mdterm(int term);
extern int get_mdterm(void);
extern int md_write_report( char *fname, int term, int print_input, int print_ke, int  print_k, int print_res, int print_rlist, int print_simple);
extern int md_n_alloc(int length);
extern int md_e_alloc(int length);
extern int md_res_alloc(int n_len, int e_len);

char *data_file = NULL ;

/** Reads input from user pseudo-interactively */
int cli_read_data(void)
{
	int i, j, k ;
  int none ;
  double a, b, c, d ;
  FILE *ifile ;

  ifile = stdin ;

  /* Field for NODES */
  fprintf(stdout,"\n\nNumber of nodes: ");
	fscanf(ifile,"%i", &n_len) ;
  if ((n_len < 2) || (n_len > md_max_nodes))
  {
    fprintf(stderr,"\nInvalid number of nodes!\n");
    return(ERR_IO);
  }
  else
  {
    fprintf(stdout,"\n OK, nodes = %i\n", n_len); 
  }

	if (md_n_alloc(n_len) != OK) 
	{
    fprintf(stderr,"\nNot enough memory for nodes!\n");
		return(ERR_MEM);
	}

  fprintf(stdout,"\nEnter %i node coordinates:\n",n_len);
  for (i=0; i<n_len; i++)
	{
    n_id[i] = i+1 ;
    fprintf(stdout," Node [%i] X Y: ",n_id[i]);
		fscanf(ifile,"%lf %lf", &n_x[i], &n_y[i]);
  }
  fprintf(stdout,"\n OK, we have %i nodes\n", n_len); 

  /* ------------------------------------------ */
 
  /* Field for ELEMENTS */
  fprintf(stdout,"\n\nNumber of elements: ");
	fscanf(ifile,"%i", &e_len) ;
  if ((e_len < 1) || (e_len > md_max_elems))
  {
    fprintf(stderr,"\nInvalid number of elements!\n");
    return(ERR_IO);
  }
  else
  {
    fprintf(stdout,"\n OK, elements = %i\n", e_len); 
  }

  if (md_e_alloc(md_max_elems) != OK) 
	{
    fprintf(stderr,"\nNot enough memory for elements!\n");
		return(ERR_MEM);
	}

  fprintf(stdout,"\nEnter %i element(s) [type 0 = |-|]:\n",e_len);
  for (i=0; i<e_len; i++)
	{
    e_id[i] = i+1 ;
    fprintf(stdout," Element [%i] TYPE NODE1 NODE2 E A I: ",e_id[i]);
		fscanf(ifile,"%i %i %i %lf %lf %lf",
				&e_type[i], &e_n1[i], &e_n2[i], &e_E[i], &e_A[i], &e_I[i]);
    /* TODO some tests here */
    if (i == 0)
    {
      if (e_E[i] <= MD_ZERO) {fprintf(stderr,"\nInvalid E!\n"); return(ERR_VAL) ;}
      if (e_A[i] <= MD_ZERO) {fprintf(stderr,"\nInvalid A!\n"); return(ERR_VAL) ;}
      if (e_I[i] <= MD_ZERO) {fprintf(stderr,"\nInvalid I!\n"); return(ERR_VAL) ;}
    }
    else
    {
      if (e_E[i] <= MD_ZERO) e_E[i] = e_E[i-1] ;
      if (e_A[i] <= MD_ZERO) e_A[i] = e_A[i-1] ;
      if (e_I[i] <= MD_ZERO) e_I[i] = e_I[i-1] ;
    }
    if (e_n1[i] == e_n2[i]) 
       {fprintf(stderr,"\nNodes cannot coincide!\n"); return(ERR_VAL) ;}

    none = 1 ;
    for (j=0; j<n_len; j++) {if (e_n1[i] == n_id[j]) {none = 0 ; break;} }
    if (none == 1) {fprintf(stderr,"\nInvalid NODE 1!\n"); return(ERR_VAL) ;}

    none = 1 ;
    for (j=0; j<n_len; j++) {if (e_n2[i] == n_id[j]) {none = 0 ; break;} }
    if (none == 1) {fprintf(stderr,"\nInvalid NODE 2!\n"); return(ERR_VAL) ;}

    e_n1[i]--;
    e_n2[i]--;
  }
  fprintf(stdout,"\n OK, we have %i elements\n", e_len); 

  /* ------------------------------------------ */
 
  /* Fields for RESULTS */
  if (md_res_alloc(md_max_nodes, md_max_elems) != OK) 
	{
    fprintf(stderr,"\nNot enough memory for program data!\n");
		return(ERR_MEM);
	}

  /* ------------------------------------------ */

  /* Data for LOADS in NODES */
  fprintf(stdout,"\n\nNumber of forces in nodes: ");
	fscanf(ifile,"%i", &none) ;
  if ((none > md_max_nodes))
  {
    fprintf(stderr,"\nToo big number of forces!\n");
    return(ERR_IO);
  }
  if (none > 0)
  {
    fprintf(stdout,"\nEnter %i forces(s):\n",none);

    for (i=0; i<none; i++)
	  {
      fprintf(stdout," Force [%i] NODE Fx Fy Mz: ",i+1);
  		fscanf(ifile,"%i %lf %lf %lf",&j, &a, &b, &c);
      j-- ;
      if ((j<0)||(j>=n_len)) {fprintf(stderr,"\nNo such node!\n"); return(ERR_VAL);}
      n_fx[j] = a ;
      n_fy[j] = b ;
      n_mz[j] = c ;
    }
  }

  /* Data for LOADS on ELEMENTS */
  fprintf(stdout,"\n\nNumber of loads on elements: ");
	fscanf(ifile,"%i", &none) ;
  if ((none > md_max_elems))
  {
    fprintf(stderr,"\nToo big number of forces!\n");
    return(ERR_IO);
  }
  if (none > 0)
  {
    fprintf(stdout,"\nEnter %i loads(s):\n",none);

    for (i=0; i<none; i++)
	  {
      fprintf(stdout," Force [%i] element na nb va vb: ",i+1);
  		fscanf(ifile,"%i %lf %lf %lf %lf",&j, &a, &b, &c, &d);
      j-- ;
      if ((j<0)||(j>=e_len)) {fprintf(stderr,"\nNo such element!\n"); return(ERR_VAL);}
      n_fx[j] = a ;
      n_fy[j] = b ;
      n_mz[j] = c ;
			e_na[i] = a ;
			e_nb[i] = b ;
			e_va[i] = c ;
			e_vb[i] = d ;
    }
  }

  /* Data for SUPPORTS */
  fprintf(stdout,"\n\nNumber of supports in nodes: ");
	fscanf(ifile,"%i", &none) ;
  if ((none > md_max_nodes))
  {
    fprintf(stderr,"\nToo big number of forces!\n");
    return(ERR_IO);
  }
  if (none > 0)
  {
    fprintf(stdout,"\nEnter %i supports [1=ux, 3=ux+uy, 6=ux+uy+rot]:\n",none);

    for (i=0; i<none; i++)
	  {
      fprintf(stdout," Force [%i] NODE TYPE: ",i+1);
  		fscanf(ifile,"%i %i",&j, &k);
      j-- ;
      if ((j<0)||(j>=n_len)) {fprintf(stderr,"\nNo such node!\n"); return(ERR_VAL);}
      if ((k<0)||(k>6)) {fprintf(stderr,"\nNo such support type!\n"); return(ERR_VAL);}
      n_dtype[j] = k ;
    }
  }

  /* ------------------------------------------ */

  /* some default values, gridReal is unused, actually: */
  num_div = 10 ; gridReal = 0.5 ;

  /* ------------------------------------------ */

  fprintf(stdout," \n OK, input is complete.\n");

	return(OK) ;
}

/* main program loop */
int main(int argc, char *argv[]) 
{ 
  int rv, i, len ;
  char name[16] ;


  data_file = NULL ;

	fprintf(stdout,"\nMicroDef: statics of 2D frames (batch mode)\n\n");
	setlocale(LC_NUMERIC,"C") ;

	fprintf(stdout,"Data reading.. ");

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
	    fprintf(stdout,"Invalid data file!\n");
		  return(-1);
    }
  }
  else
  {
    if ((rv=cli_read_data())!=OK)
    {
	    fprintf(stdout,"Invalid data, sorry!\n");
		  return(-1);
    }

	  for (i=0; i<16; i++){name[i] = '\0';}
	  fprintf(stderr,"\nName of file to save input (required, must have extension \".dfr\"):\n");
	  if (fscanf(stdin,"%15s", name) > 0)
	  {
		  if (strlen(name) > 0)
		  {
			  if ((name[0]!=' ')&&(name[0]!='\n')&&(name[0] !='0'))
			  {
   		    data_file = name ;	
			  }
        else
        {
	        fprintf(stderr,"Bad data file name!\n");
    		  return(-1);
        }
		  }
	  }
    else
    {
	    fprintf(stderr,"Data file must be specified!\n");
		  return(-1);
    }

    if (write_data(data_file) == OK)
    {
      fprintf(stdout," OK, file is: %s\n",data_file);
    }
    else
    {
	    fprintf(stderr,"Bad file or I/O error!\n");
      return(-1);
    }
  }
	fprintf(stdout,"done!\n");

	setlocale(LC_NUMERIC,"C") ;

	fprintf(stdout,"Solution.. ");
  /* run solver here */
	if (solve()!= OK)
	{
	  fprintf(stderr,"Solver failed!\n");
	}
	else
	{
	  fprintf(stdout,"done!\n");
    if (data_file != NULL) 
    {
	    fprintf(stdout,"Saving of results.. ");
		  write_data(data_file) ;
	    fprintf(stdout,"done!\n");
	    fprintf(stdout,"Writing of report.. ");
      md_write_report( data_file, MDOUT_TEXT, 1, 0, 0, 1, 0, 1);
#ifdef PSGUI
      md_write_report( data_file, MDOUT_LATEX, 1, 0, 0, 1, 0, 0);
      
      set_mdterm( MDTERM_PS) ;
    
      mdps_draw_struct(CNV_SIZE_X,CNV_SIZE_Y,data_file);
      mdps_draw_def(CNV_SIZE_X,CNV_SIZE_Y,data_file);
      mdps_draw_N(CNV_SIZE_X,CNV_SIZE_Y,data_file);
      mdps_draw_V(CNV_SIZE_X,CNV_SIZE_Y,data_file);
      mdps_draw_M(CNV_SIZE_X,CNV_SIZE_Y,data_file);
#endif
#ifdef GDGUI
      md_write_report( data_file, MDOUT_HTML, 1, 0, 1, 1, 0);
      
      set_mdterm( MDTERM_GD) ;
    
      mdgd_draw_struct(CNV_SIZE_X,CNV_SIZE_Y,data_file);
      mdgd_draw_def(CNV_SIZE_X,CNV_SIZE_Y,data_file);
      mdgd_draw_N(CNV_SIZE_X,CNV_SIZE_Y,data_file);
      mdgd_draw_V(CNV_SIZE_X,CNV_SIZE_Y,data_file);
      mdgd_draw_M(CNV_SIZE_X,CNV_SIZE_Y,data_file);
#endif

	    fprintf(stdout,"done!\n");
    }
  }

	fprintf(stdout,"\nAll work done!\n");

  return(OK); 
}

/* end of md_cli.c */
