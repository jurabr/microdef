/*
   File name: mdkernel.c
   Date:      2005/01/03 16:04
   Author:    Jiri Brozovsky

   Copyright (C) 2005 Jiri Brozovsky

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   in a file called COPYING along with this program; if not, write to
   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
   02139, USA.

	 MicroDefor - numerical kernel
*/

#include "microdef.h"
#include <stdlib.h>

#ifdef DEVEL_VERBOSE
#include <stdio.h>
#endif

#define  MD_ZERO 10e-10
#define  MD_EPS  10e-7

int   num_div = 10 ;

double Ke_loc[7][7] ; /* element stiffness matrix (local) */
double Ke_glb[7][7] ; /* element stiffness matrix (global) */

double T[7][7];       /* transformation matrix      */
double KlocT[7][7];   /* multiplying product matrix */

double Fe_loc[7];     /* load vector (local)  */
double Fe_glb[7];     /* load vector (global) */

double ue_loc[7];     /* displacement vector (local)  */
double ue_glb[7];     /* displacement vector (global) */

int    K_len    = 0 ;
int   *K_sizes  = NULL ; /* lenghts of K's rows */
int   *K_from   = NULL ;
int   *K_cols   = NULL ;
double *K_val    = NULL ;
double *F_val    = NULL ;
double *u_val    = NULL ;

double *M    = NULL ;
double *r    = NULL ;
double *z    = NULL ;
double *p    = NULL ;
double *q    = NULL ;

#ifdef DEVEL_VERBOSE
void md_K_prn(void)
{
  int i,j, k ;
  int found ;

  printf("------------------------------------------\n") ;
  for (i=0; i<3*n_len; i++)
  {
    for (j=0; j<3*n_len; j++)
    {
      found = 0 ;
      for (k=K_from[i]; k<(K_from[i]+K_sizes[i]); k++)
      {
        if (K_cols[k] == j)
        {
          printf(" %e", K_val[k]);
          found = 1;
          break ;
        }
      }
      if (found == 0) { printf(" %e", 0.0); }
    }
    printf("\n") ;
  }
  printf("------------------------------------------\n") ;
}

void md_F_prn(void)
{
  int i ;

  printf("------------------------------------------\n") ;
  for (i=0; i<3*n_len; i++)
  {
    printf(" %e", F_val[i]);
  }
  printf("\n") ;
  printf("------------------------------------------\n") ;
}

void md_U_prn(void)
{
  int i ;

  printf("------------------------------------------\n") ;
  for (i=0; i<3*n_len; i++)
  {
     printf(" %e", u_val[i]);
  }
  printf("\n") ;
  printf("------------------------------------------\n") ;
}
#endif

/* puts data to the right place in K */
void md_K_add(int row, int col, double val)
{
  int i ;

  for (i=K_from[row-1]; i<(K_from[row-1]+K_sizes[row-1]); i++)
  {

    if (K_cols[i] == (col-1))
    {
      K_val[i] += val ; 
      return ;
    }

    if (K_cols[i] < 0)
    {
      K_cols[i] = (col-1) ;
      K_val[i] = val ; 
      return ;
    }
  }
#ifdef DEVEL_VERBOSE
	fprintf(stderr,"Addition of [%i,%i] to K failed (%e)\n",row,col,val);
#endif
  exit(ERR_VAL); /* we should NOT reach this point */
}

int compute_Ke_loc(int pos, double E, double A, double I, double L)
{
						Ke_loc[1][1] = (E*A)/L ;
						Ke_loc[1][4] = (-E*A)/L ;
						Ke_loc[4][1] = (-E*A)/L ;
						Ke_loc[4][4] = (E*A)/L ;

 	switch (GET_ELEM_TYPE(pos))
  {
    case 0: /* |--| */
            Ke_loc[2][2] = (12.0*E*I)/(L*L*L) ;
            Ke_loc[2][5] = (-12.0*E*I)/(L*L*L) ;
            Ke_loc[5][2] = (-12.0*E*I)/(L*L*L) ;
            Ke_loc[5][5] = (12.0*E*I)/(L*L*L) ;

            Ke_loc[2][3] = (-6.0*E*I)/(L*L) ;
            Ke_loc[2][6] = (-6.0*E*I)/(L*L) ;
            Ke_loc[3][2] = (-6.0*E*I)/(L*L) ;
            Ke_loc[3][5] = (6.0*E*I)/(L*L) ;

            Ke_loc[5][3] = (6.0*E*I)/(L*L) ;
            Ke_loc[5][6] = (6.0*E*I)/(L*L) ;
            Ke_loc[6][2] = (-6.0*E*I)/(L*L) ;
            Ke_loc[6][5] = (6.0*E*I)/(L*L) ;

            Ke_loc[3][3] = (4.0*E*I)/(L) ;
            Ke_loc[6][6] = (4.0*E*I)/(L) ;

            Ke_loc[3][6] = (2.0*E*I)/(L) ;
            Ke_loc[6][3] = (2.0*E*I)/(L) ;
            break ;
    case 1: /* o--| */
            Ke_loc[2][2] = (3.0*E*I)/(L*L*L) ;
            Ke_loc[2][5] = (-3.0*E*I)/(L*L*L) ;
            Ke_loc[2][6] = (-3.0*E*I)/(L*L) ;

            Ke_loc[5][2] = (-3.0*E*I)/(L*L*L) ;
            Ke_loc[5][5] = (3.0*E*I)/(L*L*L) ;
            Ke_loc[5][6] = (3.0*E*I)/(L*L) ;

            Ke_loc[6][2] = (-3.0*E*I)/(L*L) ;
            Ke_loc[6][5] = (3.0*E*I)/(L*L) ;
            Ke_loc[6][6] = (3.0*E*I)/(L) ;
            break ;
    case 2: /* |--o */
            Ke_loc[2][2] = (3.0*E*I)/(L*L*L) ;
            Ke_loc[2][3] = (-3.0*E*I)/(L*L) ;
            Ke_loc[2][5] = (-3.0*E*I)/(L*L*L) ;

            Ke_loc[3][2] = (-3.0*E*I)/(L*L) ;
            Ke_loc[3][3] = (3.0*E*I)/(L) ;
            Ke_loc[3][5] = (3.0*E*I)/(L*L) ;

            Ke_loc[5][2] = (-3.0*E*I)/(L*L*L) ;
            Ke_loc[5][3] = (3.0*E*I)/(L*L) ;
            Ke_loc[5][5] = (3.0*E*I)/(L*L*L) ;
            break ;
    case 3: /* o--o - no code necessary here  */
            break ;

    default: return(ERR_VAL); break;
  }

	return(OK);
}

void compute_T(double s, double c)
{
  T[1][1] = c ;
  T[1][2] = s ;
  T[2][1] = -s ;
  T[2][2] = c ;
  T[3][3] = 1 ;

  T[4][4] = c ;
  T[4][5] = s ;
  T[5][4] = -s ;
  T[5][5] = c ;
  T[6][6] = 1 ;
}

int computeFe(int pos, double na, double nb, double va, double vb, double L)
{
	switch (GET_ELEM_TYPE(pos))
  {
    case 0: /* |--| */
            Fe_loc[1]=(-(2.0*na+1.0*nb)*L)/6.0 ;
            Fe_loc[2]=(-(7.0*va+3.0*vb)*L)/20.0 ;
            Fe_loc[3]=((3.0*va+2.0*vb)*L*L)/60.0 ;
            Fe_loc[4]=(-(1.0*na+2.0*nb)*L)/6.0 ;
            Fe_loc[5]=(-(3.0*va+7.0*vb)*L)/20.0 ;
            Fe_loc[6]=(-(2.0*va+3.0*vb)*L*L)/60.0 ;
            break ;
    case 1: /* o--| */
            Fe_loc[1]=(-(2.0*na+1.0*nb)*L)/6.0 ;
            Fe_loc[2]=(-(4.0*va+11.0*vb)*L)/40.0 ;
            Fe_loc[4]=(-(1.0*na+2.0*nb)*L)/6.0 ;
            Fe_loc[5]=(-(16.0*va+9.0*vb)*L)/40.0 ;
            Fe_loc[6]=((8.0*va+7.0*vb)*L*L)/120.0 ;
            break ;
    case 2: /* |--o */
            Fe_loc[1]=(-(2.0*na+1.0*nb)*L)/6.0 ;
            Fe_loc[2]=(-(16.0*va+9.0*vb)*L)/40.0 ;
            Fe_loc[4]=(-(1.0*na+2.0*nb)*L)/6.0 ;
            Fe_loc[3]=((8.0*va+7.0*vb)*L*L)/120.0 ;
            Fe_loc[5]=(-(4.0*va+11.0*vb)*L)/40.0 ;
            break ;
    case 3: /* o--o */
            Fe_loc[1]=(-(2.0*na+1.0*nb)*L)/6.0 ;
            Fe_loc[2]=(-(7.0*va+3.0*vb)*L)/20.0 ; /* FIXME */
            Fe_loc[4]=(-(1.0*na+2.0*nb)*L)/6.0 ;
            Fe_loc[5]=(-(3.0*va+7.0*vb)*L)/20.0 ; /* FIXME */
            break ;
    default: return(ERR_VAL); break;
  }
	return(OK);
}

double compute_L(int pos)
{
  int n1, n2 ;
  double x1,y1,x2,y2;

  n1 = GET_ELEM_N1(pos) ;
  n2 = GET_ELEM_N2(pos) ;

  x1 = GET_NODE_X(n1) ;
  y1 = GET_NODE_Y(n1) ;
  x2 = GET_NODE_X(n2) ;
  y2 = GET_NODE_Y(n2) ;

  return(sqrt( (y2-y1)*(y2-y1) + (x2-x1)*(x2-x1) ) ) ;
}

/* computes Ke, Fe and puts them to K, F */
int compute_KeFe(int pos)
{
  int i,j,k, ii,jj, n1,n2 ;
  double E,A,I,s,c,L,x1,y1,x2,y2;
  double va,vb,na,nb;
  double kval, fval ;

  /* set everything to zero */
  for (i=0; i<7; i++)
  {
    for (j=0; j<7; j++)
    {
      Ke_loc[i][j] = 0.0 ;
      Ke_glb[i][j] = 0.0 ;
      T[i][j] = 0.0 ;
      KlocT[i][j] = 0.0 ;
    }

    Fe_loc[i] = 0.0 ;
    Fe_glb[i] = 0.0 ;
    ue_loc[i] = 0.0 ;
    ue_glb[i] = 0.0 ;
  }

  E = GET_ELEM_E(pos) ;
  A = GET_ELEM_A(pos) ;
  I = GET_ELEM_I(pos) ;

  n1 = GET_ELEM_N1(pos) ;
  n2 = GET_ELEM_N2(pos) ;

  x1 = GET_NODE_X(n1) ;
  y1 = GET_NODE_Y(n1) ;
  x2 = GET_NODE_X(n2) ;
  y2 = GET_NODE_Y(n2) ;

  L = sqrt( (y2-y1)*(y2-y1) + (x2-x1)*(x2-x1) ) ;

  s = (y2-y1) / L ;
  c = (x2-x1) / L ;

  /* ELEMENTS */
	if (compute_Ke_loc(pos, E, A, I, L) != OK) {return(ERR_VAL);}

  na   = GET_ELEM_NA(pos) ;
  nb   = GET_ELEM_NB(pos) ;
  va   = GET_ELEM_VA(pos) ;
  vb   = GET_ELEM_VB(pos) ;

  /* LOADS */
	computeFe(pos, na, nb, va, vb, L);
  

#ifdef DEVEL_VERBOSE
printf("\nn,v (%i)= [%e %e],[%e %e]\n",GET_ELEM_TYPE(pos),na,nb,va,vb);
printf("F = [%e %e %e %e %e %e]\n", Fe_loc[1], Fe_loc[2], Fe_loc[3], Fe_loc[4], Fe_loc[5], Fe_loc[6]);
#endif

	compute_T(s, c);

  for (i=1; i<=6; i++)
  {
    fval = 0.0 ;

    for (j=1; j<=6; j++)
    {
      fval += T[j][i] * Fe_loc[j] ;

      kval = 0.0 ;
      for (k=1; k<=6; k++)
      {
        /* normally: val += T[i][k] * Ke_loc[k][j] ; */
        kval += T[k][i] * Ke_loc[k][j] ;
      }
      KlocT[i][j] = kval ;
    }
    Fe_glb[i] = fval ;
  }

#ifdef DEVEL_VERBOSE
printf("F = [%e %e %e %e %e %e]\n", Fe_glb[1], Fe_glb[2], Fe_glb[3], Fe_glb[4], Fe_glb[5], Fe_glb[6]);
#endif

  for (i=1; i<=6; i++)
  {
    fval = 0.0 ;
    for (j=1; j<=6; j++)
    {
      kval = 0.0 ;
      for (k=1; k<=6; k++)
      {
        kval += KlocT[i][k] * T[k][j] ;
      }
      Ke_glb[i][j] = kval ;
    }
  }

  /* localization of matrix and vector: */
  for (i=1; i<=6; i++)
  {
    if (i <4) { ii = n1*3 + i ; }
    else      { ii = n2*3 + i-3 ; }

    F_val[ii-1] += Fe_glb[i] ;

    for (j=1; j<=6; j++)
    {
      if (j <4) { jj = n1*3 + j ; }
      else      { jj = n2*3 + j-3 ; }

      md_K_add(ii, jj, Ke_glb[i][j]) ;
    }
  }

  return(OK);
}

/* adds boundary condition - row is numbered from 0 */
void md_add_one_disp(int row, double val)
{
  int i,j,n ;

  n = 3*n_len ;

  for (i=0; i<n; i++)
  {
    for (j=K_from[i]; j<(K_from[i]+K_sizes[i]); j++)
    {
      if (K_cols[i] == (row))
      {
        if (K_cols[i] < 0) {break;}
        F_val[K_cols[i]] += K_val[i]*val ;
        K_val[i] = 0.0 ;
        break ;
      }
    }
  }

  for (i=K_from[row]; i<K_from[row]+K_sizes[row]; i++)
  {
    if (K_cols[i] < 0) {break;}
    if (K_cols[i] == row) 
    {
      K_val[i] = 1.0 ;
    }
    else
    {
      K_val[i] = 0.0 ;
    }
  }
  u_val[row] = val ;
  F_val[row] = val ; /* it will destroy any force in this place */
}

int md_add_disps_and_loads(void)
{
  int i ;

  for (i=0; i<n_len; i++)
  {
    F_val[i*3+0] += GET_NODE_FX(i) ;
    F_val[i*3+1] += GET_NODE_FY(i) ;
    F_val[i*3+2] += GET_NODE_MZ(i) ;
  }

  for (i=0; i<n_len; i++)
  {
    switch(GET_NODE_TYPE(i))
    {
      case 1: md_add_one_disp(i*3+0, GET_NODE_POSX(i)) ;
              break; 
      case 2: md_add_one_disp(i*3+1, GET_NODE_POSY(i)) ;
              break;
      case 3: md_add_one_disp(i*3+0, GET_NODE_POSX(i)) ;
              md_add_one_disp(i*3+1, GET_NODE_POSY(i)) ;
              break;
      case 4: md_add_one_disp(i*3+0, GET_NODE_POSX(i)) ;
              md_add_one_disp(i*3+2, GET_NODE_ROTZ(i)) ;
              break;
      case 5: md_add_one_disp(i*3+1, GET_NODE_POSY(i)) ;
              md_add_one_disp(i*3+2, GET_NODE_ROTZ(i)) ;
              break;
      case 6: md_add_one_disp(i*3+0, GET_NODE_POSX(i)) ;
              md_add_one_disp(i*3+1, GET_NODE_POSY(i)) ;
              md_add_one_disp(i*3+2, GET_NODE_ROTZ(i)) ;
              break;
    }
  }

  return(OK);
}


void md_free_eqs(void)
{
  md_free_double(M) ;
  md_free_double(r) ;
  md_free_double(z) ;
  md_free_double(p) ;
  md_free_double(q) ;
}

void md_free_K(void)
{
  md_free_int(K_sizes) ;
  md_free_int(K_from) ;
  md_free_int(K_cols) ;
  md_free_double(K_val) ;
}

void md_free_Fu(void)
{
  md_free_double(F_val) ;
  md_free_double(u_val) ;
}


/* Computes number of nonzero items of K */
int md_alloc_stuff(void)
{
  int i, j, sum ;

  if ((K_sizes = md_alloc_int(3*n_len))   == NULL) { goto memFree ; }
  if ((K_from  = md_alloc_int(3*n_len))   == NULL) { goto memFree ; } 
  if ((F_val   = md_alloc_double(3*n_len)) == NULL) { goto memFree ; } 
  if ((u_val   = md_alloc_double(3*n_len)) == NULL) { goto memFree ; }

  if ((M   = md_alloc_double(3*n_len)) == NULL) { goto memFree ; }
  if ((r   = md_alloc_double(3*n_len)) == NULL) { goto memFree ; }
  if ((z   = md_alloc_double(3*n_len)) == NULL) { goto memFree ; }
  if ((p   = md_alloc_double(3*n_len)) == NULL) { goto memFree ; }
  if ((q   = md_alloc_double(3*n_len)) == NULL) { goto memFree ; }

  K_len = 0 ;

  for (i=0; i<n_len; i++) { K_sizes[i] = 0 ; } 

  for (i=0; i<n_len; i++)
  {
    for (j=0; j<e_len; j++)
    {
      if (GET_ELEM_N1(j) == i) 
			{
				K_sizes[3*i+0]+=6; 
				K_sizes[3*i+1]+=6; 
				K_sizes[3*i+2]+=6; 
			}
      if (GET_ELEM_N2(j) == i) 
			{
				K_sizes[3*i+0]+=6; 
				K_sizes[3*i+1]+=6; 
				K_sizes[3*i+2]+=6; 
			}
    }
  }

  sum = 0 ;

  for (i=0; i<(n_len*3); i++)
  {
    K_from[i] = sum ;
    sum += K_sizes[i] ;
  }

	K_len = sum ;

  if ((K_cols = md_alloc_int(K_len)) == NULL) { goto memFree ; } 
  if ((K_val = md_alloc_double(K_len)) == NULL) { goto memFree ; } 


  for (i=0; i<K_len; i++)
  {
    K_cols[i] = -1 ;
    K_val[i]  = 0.0 ;
  }

  return(OK);
memFree:
  md_free_Fu();
  md_free_K();
  md_free_eqs();
  return(ERR_MEM) ; 
}

void md_free_stuff(void)
{
  md_free_Fu();
  md_free_K();
  md_free_eqs();
}

double md_norm_K(void)
{
  int i,j ;
  double MaxNorm = 0.0 ;
  double Norm    = 0.0 ;
  
  for (i=0; i<(3*n_len); i++)
	{
	 	Norm = 0.0;
		for (j=K_from[i]; j<K_from[i]+K_sizes[i]; j++)
		{
			if (K_cols[j] < 0) {break;}
	  	Norm += (K_val[j]*K_val[j]);
		}
		Norm = sqrt(Norm);
		if (Norm > MaxNorm) {MaxNorm = Norm;}
	}
  return(Norm);
}

double md_vec_norm(double *a, long len)
{
  int i ;
  double Norm    = 0.0 ;
  
  for (i=0; i<len; i++) { Norm += (a[i]*a[i]); }
  return(sqrt(Norm));
}

int md_solve_eqs(void)
{
  double ro, alpha, beta;
	double roro = 0.0 ;
	double normRes, normX, normA, normB;
  double mval ;
	int   converged = 0;
	int   n = 0;
	int   i,j,k;

  n = 3*n_len ;

	normA = md_norm_K();
	normB = md_vec_norm(F_val, n);

  if (normB <= 0.0) /* no loads - nothing to do */
	{
		return(OK);
	}

  /* Jacobi preconditioner: */
	for (i=0; i<n; i++) 
	{ 
		M[i] = 0.0 ;
		for (j=K_from[i]; j<K_from[i]+K_sizes[i]; j++)
		{
			if (K_cols[j] == (i))
			{
				M[i] = K_val[j] ;
				break ;
			}
		}

		if (fabs(M[i]) < MD_ZERO) 
		{ 
#ifdef DEVEL_VERBOSE
	fprintf(stderr,"zero value at [%i,%i]: %e\n",i+1,i+1, M[i]);
#endif
			return( ERR_ZER ); 
		}
	}

	/* r = b - A*x  */
  for (i=0; i<n; i++)
  {
    mval = 0.0 ;

    for (j=0; j<K_sizes[i]; j++)
    {
      if  (K_cols[K_from[i]+j] < 0) {break;}
      mval += K_val[K_from[i]+j] * u_val[K_cols[K_from[i]+j]];
    }
    r[i] = mval ;
  }
  for (i=0; i<n; i++) { r[i] = F_val[i] - r[i] ; }

  /* main loop */
	for (i=1; i<=n; i++) 
  { 
#ifdef DEVEL_VERBOSE
    printf("CG iteration: %i/%i\n",i,n);
#endif
    for (j=0; j<n; j++) { z[j] = (r[j] / M[j]) ; }

    ro = 0.0 ;
    for (j=0; j<n; j++) {ro += r[j]*z[j];}

    if (i == 1)
	  {
	    for (j=0; j<n; j++) { p[j] = z[j]; }
	  }
	  else
	  {
		  beta = ro / roro ;
	    for (j=0; j<n; j++) { p[j] = (z[j] + (beta*p[j])) ; }
	  }


    for (k=0; k<n; k++) /* q = K*p */
    {
      mval = 0.0 ;

      for (j=0; j<K_sizes[k]; j++)
      {
        if  (K_cols[K_from[k]+j] < 0) {break;}
        mval += K_val[K_from[k]+j] * p[K_cols[K_from[k]+j]];
      }
      q[k] = mval ;
    }

    mval = 0.0 ;
    for (j=0; j<n; j++) {mval += p[j]*q[j];}
	  alpha = ro / mval ;

#ifdef DEVEL_VERBOSE
    printf("alpha = %e, beta = %e, ro = %e\n",alpha, beta, ro);
#endif

    for (j=0; j<n; j++) 
	  { 
		  u_val[j] = u_val[j] + (alpha * p[j])  ; 
		  r[j] = r[j] - (alpha * q[j])  ; 
	  } 


		/* Convergence testing */

	  normRes = md_vec_norm(r, n);
	  normX   = md_vec_norm(u_val, n);

    if (normRes  <= (MD_EPS*((normA*normX) + normB)) ) 
		{
			converged = 1;
			break;
		}

		roro = ro;
  
  } /* end of main loop */

  if (converged == 1) { return(OK); }
  else                
	{
#ifdef DEVEL_VERBOSE
		fprintf(stderr,"Unconverged solution!\n");
#endif
		return(ERR_VAL); 
	}
}


/* TODO: 
 * 1. results on nodes and elements + reactions
 * 2. results (M,N,V,def) in element point (for gfx, listing etc.)
 * */

/* computes results in nodes (and ends of elements) */
int compute_res(int pos)
{
  int i,j, ii, n1,n2 ;
  double E,A,I,s,c,L,x1,y1,x2,y2;
  double fval ;

  /* set everything to zero */
  for (i=0; i<7; i++)
  {
    for (j=0; j<7; j++)
    {
      Ke_loc[i][j] = 0.0 ;
      Ke_glb[i][j] = 0.0 ;
      T[i][j] = 0.0 ;
      KlocT[i][j] = 0.0 ;
    }

    Fe_loc[i] = 0.0 ;
    Fe_glb[i] = 0.0 ;
    ue_loc[i] = 0.0 ;
    ue_glb[i] = 0.0 ;
  }

  E = GET_ELEM_E(pos) ;
  A = GET_ELEM_A(pos) ;
  I = GET_ELEM_I(pos) ;

  n1 = GET_ELEM_N1(pos) ;
  n2 = GET_ELEM_N2(pos) ;

  x1 = GET_NODE_X(n1) ;
  y1 = GET_NODE_Y(n1) ;
  x2 = GET_NODE_X(n2) ;
  y2 = GET_NODE_Y(n2) ;

  L = sqrt( (y2-y1)*(y2-y1) + (x2-x1)*(x2-x1) ) ;

  s = (y2-y1) / L ;
  c = (x2-x1) / L ;

  /* ELEMENTS */
	if (compute_Ke_loc(pos, E, A, I, L) != OK) {return(ERR_VAL);}

	compute_T(s, c);

  /* localization of vector ue_glb: */
  for (i=1; i<=6; i++)
  {
    if (i <4) { ii = n1*3 + i ; }
    else      { ii = n2*3 + i-3 ; }

    ue_glb[i] =  u_val[ii-1] ;

#ifdef DEVEL_VERBOSE
    printf("ue[%i] = %e (<- %i..%e) \n",i, ue_glb[i],ii-1,u_val[ii-1]);
#endif
  }

	/* u_e glob -> loc */
  for (i=1; i<=6; i++)
  {
    fval = 0.0 ;

    for (j=1; j<=6; j++)
    {
      fval += T[i][j] * ue_glb[j] ;
    }
    ue_loc[i] = fval ;

#ifdef DEVEL_VERBOSE
    printf("ue_loc[%i] = %e\n",i, ue_loc[i]);
#endif
  }

	/* Fe_loc = Ke_loc * ue_loc */
	for (i=1; i<=6; i++)
  {
    fval = 0.0 ;
    for (j=1; j<=6; j++)
    {
      fval += Ke_loc[i][j] * ue_loc[j] ;
    }
    Fe_loc[i] = fval ;

#ifdef DEVEL_VERBOSE
    printf("Fe_loc[%i] = %e\n",i, Fe_loc[i]);
#endif
  }

	/* Fe_glb = T^T * Fe_loc */ 
	for (i=1; i<=6; i++)
  {
    fval = 0.0 ;
    for (j=1; j<=6; j++)
    {
      fval += T[j][i] * Fe_loc[j] ;
    }
    Fe_glb[i] = fval ;
  }

	/* fe_glb -> reactions */

	switch (GET_NODE_TYPE(n1))
	{
		case 1: /* ux */
						ADD_NRES_FX(n1, Fe_glb[1]);
						break;
		case 2: /* uy */
						ADD_NRES_FY(n1, Fe_glb[2]);
						break;
		case 3: /* ux,uy */
						ADD_NRES_FX(n1, Fe_glb[1]);
						ADD_NRES_FY(n1, Fe_glb[2]);
						break;
		case 4: /* ux, rotz */
						ADD_NRES_FX(n1, Fe_glb[1]);
						ADD_NRES_MZ(n1, Fe_glb[3]);
						break;
		case 5: /* uy, rotz */
						ADD_NRES_FY(n1, Fe_glb[2]);
						ADD_NRES_MZ(n1, Fe_glb[3]);
						break;
		case 6: /* ux, uy, rotz */
						ADD_NRES_FX(n1, Fe_glb[1]);
						ADD_NRES_FY(n1, Fe_glb[2]);
						ADD_NRES_MZ(n1, Fe_glb[3]);
						break;
	}

	switch (GET_NODE_TYPE(n2))
	{
		case 1: /* ux */
						ADD_NRES_FX(n2, Fe_glb[4]);
						break;
		case 2: /* uy */
						ADD_NRES_FY(n2, Fe_glb[5]);
						break;
		case 3: /* ux,uy */
						ADD_NRES_FX(n2, Fe_glb[4]);
						ADD_NRES_FY(n2, Fe_glb[5]);
						break;
		case 4: /* ux, rotz */
						ADD_NRES_FX(n2, Fe_glb[4]);
						ADD_NRES_MZ(n2, Fe_glb[6]);
						break;
		case 5: /* uy, rotz */
						ADD_NRES_FY(n2, Fe_glb[5]);
						ADD_NRES_MZ(n2, Fe_glb[6]);
						break;
		case 6: /* ux, uy, rotz */
						ADD_NRES_FX(n2, Fe_glb[4]);
						ADD_NRES_FY(n2, Fe_glb[5]);
						ADD_NRES_MZ(n2, Fe_glb[6]);
						break;
	}

#if 1
  /* Fe_loc --> element results */
  PUT_ERES_A_FX(pos,Fe_loc[1]);
  PUT_ERES_A_FY(pos,Fe_loc[2]);
  PUT_ERES_A_MZ(pos,Fe_loc[3]);
  PUT_ERES_B_FX(pos,Fe_loc[4]);
  PUT_ERES_B_FY(pos,Fe_loc[5]);
  PUT_ERES_B_MZ(pos,Fe_loc[6]);
#endif

  return(OK);
}

/* compute primary results */
int compute_prim_res(int pos)
{
  int i,j, n1,n2 ;
  double E,A,I,s,c,L,x1,y1,x2,y2;
  double va,vb,na,nb;
  double fval ;

  /* set everything to zero */
  for (i=0; i<7; i++)
  {
    for (j=0; j<7; j++)
    {
      T[i][j] = 0.0 ;
    }
    Fe_loc[i] = 0.0 ;
    Fe_glb[i] = 0.0 ;
  }

  E = GET_ELEM_E(pos) ;
  A = GET_ELEM_A(pos) ;
  I = GET_ELEM_I(pos) ;

  n1 = GET_ELEM_N1(pos) ;
  n2 = GET_ELEM_N2(pos) ;

  x1 = GET_NODE_X(n1) ;
  y1 = GET_NODE_Y(n1) ;
  x2 = GET_NODE_X(n2) ;
  y2 = GET_NODE_Y(n2) ;

  L = sqrt( (y2-y1)*(y2-y1) + (x2-x1)*(x2-x1) ) ;

  s = (y2-y1) / L ;
  c = (x2-x1) / L ;

  /* ELEMENTS */
	if (compute_Ke_loc(pos, E, A, I, L) != OK) {return(ERR_VAL);}

	compute_T(s, c);

  na   = GET_ELEM_NA(pos) ;
  nb   = GET_ELEM_NB(pos) ;
  va   = GET_ELEM_VA(pos) ;
  vb   = GET_ELEM_VB(pos) ;

	computeFe(pos, na, nb, va, vb, L) ;

	/* Fe_glb = T^T * Fe_loc */ 
	for (i=1; i<=6; i++)
  {
    fval = 0.0 ;
    for (j=1; j<=6; j++)
    {
      fval += T[j][i] * Fe_loc[j] ;
    }
    Fe_glb[i] = fval ;
  }

	/* fe_glb -> reactions */

	switch (GET_NODE_TYPE(n1))
	{
		case 1: /* ux */
						ADD_NRES_FX(n1, -Fe_glb[1]);
						break;
		case 2: /* uy */
						ADD_NRES_FY(n1, -Fe_glb[2]);
						break;
		case 3: /* ux,uy */
						ADD_NRES_FX(n1, -Fe_glb[1]);
						ADD_NRES_FY(n1, -Fe_glb[2]);
						break;
		case 4: /* ux, rotz */
						ADD_NRES_FX(n1, -Fe_glb[1]);
						ADD_NRES_MZ(n1, -Fe_glb[3]);
						break;
		case 5: /* uy, rotz */
						ADD_NRES_FY(n1, -Fe_glb[2]);
						ADD_NRES_MZ(n1, -Fe_glb[3]);
						break;
		case 6: /* ux, uy, rotz */
						ADD_NRES_FX(n1, -Fe_glb[1]);
						ADD_NRES_FY(n1, -Fe_glb[2]);
						ADD_NRES_MZ(n1, -Fe_glb[3]);
						break;
	}

	switch (GET_NODE_TYPE(n2))
	{
		case 1: /* ux */
						ADD_NRES_FX(n2, -Fe_glb[4]);
						break;
		case 2: /* uy */
						ADD_NRES_FY(n2, -Fe_glb[5]);
						break;
		case 3: /* ux,uy */
						ADD_NRES_FX(n2, -Fe_glb[4]);
						ADD_NRES_FY(n2, -Fe_glb[5]);
						break;
		case 4: /* ux, rotz */
						ADD_NRES_FX(n2, -Fe_glb[4]);
						ADD_NRES_MZ(n2, -Fe_glb[6]);
						break;
		case 5: /* uy, rotz */
						ADD_NRES_FY(n2, -Fe_glb[5]);
						ADD_NRES_MZ(n2, -Fe_glb[6]);
						break;
		case 6: /* ux, uy, rotz */
						ADD_NRES_FX(n2, -Fe_glb[4]);
						ADD_NRES_FY(n2, -Fe_glb[5]);
						ADD_NRES_MZ(n2, -Fe_glb[6]);
						break;
	}

  /* Fe_loc --> element results */
  ADD_ERES_A_FX(pos,-Fe_loc[1]);
  ADD_ERES_A_FY(pos,-Fe_loc[2]);
  ADD_ERES_A_MZ(pos,-Fe_loc[3]);
  ADD_ERES_B_FX(pos,-Fe_loc[4]);
  ADD_ERES_B_FY(pos,-Fe_loc[5]);
  ADD_ERES_B_MZ(pos,-Fe_loc[6]);

  return(OK);
}

/** Computes N,V or M in point of a element */
double compute_e_res(int type, int epos, int div, int ppos)
{
  int    n1, n2 ;
  double x1,x2,y1,y2 ;
  double Na,Nb, Va,Vb, Ma,Mb, L, lenx, lenxx, na, nb, no, nt ;
  double Xo = 0.0 ;

  Na = GET_ERES_A_FX(epos) ;
  Va = GET_ERES_A_FY(epos) ;
  Ma = GET_ERES_A_MZ(epos) ;
  Nb = GET_ERES_B_FX(epos) ;
  Vb = GET_ERES_B_FY(epos) ;
  Mb = GET_ERES_B_MZ(epos) ;

  n1 = GET_ELEM_N1(epos) ;
  n2 = GET_ELEM_N2(epos) ;

  x1 = GET_NODE_X(n1) ;
  y1 = GET_NODE_Y(n1) ;
  x2 = GET_NODE_X(n2) ;
  y2 = GET_NODE_Y(n2) ;

  L = sqrt( (y2-y1)*(y2-y1) + (x2-x1)*(x2-x1) ) ;
  lenx  = L*((double)((double)ppos/(double)(div))) ;
  lenxx = L - lenx ;

  switch (type)
  {
    case 1 :
             /* Compute X0 here: TODO - UNTESTED! */
             na = GET_ELEM_NA(epos) ;
             nb = GET_ELEM_NB(epos) ;
             no = na ; nt = nb - no ;

             Xo =
							 lenx*no 
							 +
							 0.5*lenx*((nt*lenx/L)) 
							 ;
             return (Xo - (Na) ) ;
             break ;
    case 2 : 
             /* Compute X0 here: */
             na = GET_ELEM_VA(epos) ;
             nb = GET_ELEM_VB(epos) ;
             no = na ; nt = nb - no ;

             Xo = (no*L)/2 - (no*lenx) 
#if 1
							 + ((nt*L)/6 - ((nt*lenx*lenx)/(2*L)) ) /* TODO UNTESTED!*/
#endif
							 ;

             /* result: */
             return (Xo  - ((Mb + Ma)/L)  ) ;
             break ;
    case 3 : 
             /* Compute X0 here: */
             na = GET_ELEM_VA(epos) ;
             nb = GET_ELEM_VB(epos) ;
             no = na ; nt = nb - no ;

             Xo =  (no*L*lenx)/2 - (no*lenx*lenx)/2 
#if 1
                  + ((nt*L*lenx)/6.0 - (nt*lenx*lenx*lenx)/(6*L) )  /* TODO UNTESTED!*/
#endif
									;

             /* result: */
             return ((-1.0)*(Xo + ((Ma*lenxx-Mb*lenx)/L)  ) ) ;
             break ;
  }

  return(0.0);
}

/* */
double get_max_NVM_res(int type, int epos, int div, int *ppos)
{
  int i ;
  double val ;
  double max = 0.0 ;
  int    pos = -1 ;

  for (i=0; i<=div; i++)
  {
    val = compute_e_res(type, epos, div, i) ;
    if (val > max)
    {
      max = val ;
      pos = i ;
    }
  }

  *ppos = pos ;
  return(max);
}


/* compute minimal value on a beam */
double get_min_NVM_res(int type, int epos, int div, int *ppos)
{
  int i ;
  double val ;
  double max = 0.0 ;
  int    pos = -1 ;

  for (i=0; i<=div; i++)
  {
    val = compute_e_res(type, epos, div, i) ;
    if (val < max)
    {
      max = val ;
      pos = i ;
    }
  }

  *ppos = pos ;
  return(max);
}

int compute_results(void)
{
	int i ;

  for (i=0; i<n_len; i++)
	{
		PUT_NRES_FX(i,-GET_NODE_FX(i)); /* reactions to zero */
		PUT_NRES_FY(i,-GET_NODE_FY(i));
		PUT_NRES_MZ(i,-GET_NODE_MZ(i));

		PUT_NRES_POSX(i,u_val[i*3+0]); /* DOF solution */
		PUT_NRES_POSY(i,u_val[i*3+1]);
		PUT_NRES_ROTZ(i,u_val[i*3+2]);
	}

	for (i=0; i<e_len; i++)
	{
		compute_res(i);
		compute_prim_res(i);
	}
	return(OK);
}

/* test for unused nodes - adds supports for these nodes */
void md_idiotproof_nodes(void)
{
  int used = 0 ;
  int i,j ;
  
  for (i=0; i<n_len; i++)
  {
    used = 0 ;

    for (j=0; j<e_len; j++)
    {
      if ((GET_ELEM_N1(j) == i) || (GET_ELEM_N2(j) == i))
      {
        used = 1 ; 
        break ;
      }
    }

    if (used == 0)
    {
      DELETE_NODE(i) ;
      i-- ;
    }
  }
}

/* test for full hinges - adds supports for these nodes */
void md_idiotproof_hinges(void)
{
  int used = 0 ;
  int i,j ;
  
  for (i=0; i<n_len; i++)
  {
    used = 0 ;

    for (j=0; j<e_len; j++)
    {
      if ((GET_ELEM_N1(j) == i) &&
          ((GET_ELEM_TYPE(j) == 0)||(GET_ELEM_TYPE(j) == 2)))
      { used = 1 ; break ; }

      if ((GET_ELEM_N2(j) == i) &&
          ((GET_ELEM_TYPE(j) == 0)||(GET_ELEM_TYPE(j) == 1)))
      { used = 1 ; break ; }
    }

    if (used == 0)
    {
      md_add_one_disp(3*i+2, 0) ;
    }
  }
}

int solve(void)
{
	int i ;

#if 1
  md_idiotproof_nodes();
#endif

#if 1
  md_reduce_coords();
#endif

	if (n_len < 2) { return(ERR); }
	if (e_len < 1) { return(ERR); }

  md_alloc_stuff();
  
  for (i=0; i<e_len; i++) { compute_KeFe(i); }
	
#ifdef DEVEL_VERBOSE
  md_K_prn();
  md_F_prn();
  md_U_prn();
#endif

  md_add_disps_and_loads();

#if 1
  md_idiotproof_hinges();
#endif

  if (md_solve_eqs() != OK)
	{
#ifdef DEVEL_VERBOSE
		fprintf(stderr, "Solution failed!\n");
    md_K_prn();
#endif
    md_free_stuff();
		return(ERR);
	}
#ifdef DEVEL_VERBOSE
	else { fprintf(stderr, "Solution done!\n"); }
#endif

#ifdef DEVEL_VERBOSE
  md_U_prn();
#endif

  compute_results();

  md_free_stuff();

  return(OK);
}

/* element deformation computation:  */
double md_compute_e_def_y(int e_type, double L, double q1, double q2, double x)
{
  double y = 0.0 ;
  double xx;
  double qa, qb ;

  if (q1 == q2)
  {
    qa = q1 ;
    qb = 0.0 ;
  }
  else
  {
    qa = q1 ;
    qb = q2 - q1 ;
  }

  switch(e_type)
  {
    case 0: /* |-| */
            y  = (qa*x*x*x*x)/24 
                 - (qa*L*x*x*x)/12 
                 + (qa*L*L*x*x)/24 ;
            y += ((qb*x*x)/(120*L))*(x*x*x - 3*L*L*x + 2*L*L*L) ;
            break;
    case 1: /* o-| */ 
            y  = (qa*x*x*x*x)/2
              - (3*qa*L*x*x*x)/48
              + (qa*L*L*L*x)/8 ;
            y += (qb*x*x*x*x)/24
                 - (3*qb*L*x*x*x)/48
                 + (qb*L*L*L*x)/48 ;
            break;
    case 2: /* |-o */ 
            y  = (qa*x*x*x*x)/2
              - (3*qa*L*x*x*x)/48
              + (qa*L*L*L*x)/8 ;

            xx = L-x ;
            y += (qb*xx*xx*xx*xx)/24
                 - (3*qb*L*xx*xx*xx)/48
                 + (qb*L*L*L*xx)/48 ;
            break;
    case 3: /* o-o */ 
            y  = (qa*x*x*x*x)/24
                 - (qa*L*x*x)/12
                 + (qa*L*L*L*x)/24 ;
            y += (qb*x*x*x*x*x)/(120*L)
                 - (qb*L*x*x*x)/36 
                 + (7/360)*qb*L*L*L*x ;
            break;
  }

  return(y);
}

/* compute minimal value on a beam */
double get_max_def_res(int type, int epos, int div, int *ppos, double *x)
{
  int i, n1, n2 ;
  double val, EI, L, xx, x1,y1,x2,y2 ;
  double max = 0.0 ;
  int    pos = -1 ;

  n1 = GET_ELEM_N1(epos) ;
  n2 = GET_ELEM_N2(epos) ;


  x1 = (GET_NODE_X(n1));
  y1 = (GET_NODE_Y(n1));
  x2 = (GET_NODE_X(n2));
  y2 = (GET_NODE_Y(n2));

  L = sqrt ((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)); 

  for (i=0; i<=div; i++)
  {
    EI = GET_ELEM_E(epos)*GET_ELEM_I(epos);
    xx = ((double)i/(double)num_div)*L ;
    val = ((md_compute_e_def_y(
      GET_ELEM_TYPE(epos), 
      L, 
      GET_ELEM_VA(epos), 
      GET_ELEM_VB(epos), 
      xx) / EI )) ;

    if (fabs(val) > max)
    {
      max = val ;
      pos = i ;
    }
  }

  *ppos = pos ;
  *x    = xx ;
  return(max);
}



/** Moves lowest left point of structure to (0,0) */
void md_reduce_coords(void)
{
  double min_x = 0 ;
  double min_y = 0 ;
  int i ;

  for (i=0; i<n_len; i++)
  {
    if (i == 0)
    {
      min_x = GET_NODE_X(i) ;
      min_y = GET_NODE_Y(i) ;
    }
    else
    {
      if (GET_NODE_X(i) < min_x) {min_x = GET_NODE_X(i);}
      if (GET_NODE_Y(i) < min_y) {min_y = GET_NODE_Y(i);}
    }
  }

  if (min_x < 0.0) {min_x = fabs(min_x);}
  if (min_y < 0.0) {min_y = fabs(min_y);}

  for (i=0; i<n_len; i++)
  {
    SET_NODE_X(i, GET_NODE_X(i)-min_x) ;
    SET_NODE_Y(i, GET_NODE_Y(i)-min_y) ;
  }
}

/* returnes position of node numbered "id" */
int md_node_find_by_number(int id)
{
  static int pos ;
  int i ;

  for (i=0; i<n_len; i++)
  {
    if (GET_NODE_ID(i) == id)
    {
      pos = i ;
      return(pos);
    }
  }

  return(-1);
}

/* returnes position of node numbered "id" */
int md_elem_find_by_number(int id)
{
  static int pos ;
  int i ;

  for (i=0; i<e_len; i++)
  {
    if (GET_ELEM_ID(i) == id)
    {
      pos = i ;
      return(pos);
    }
  }

  return(-1);
}



/* end of mdkernel.c */
