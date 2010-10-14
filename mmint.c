/*
   File name: mmint.c
   Date:      2008/05/17 17:15
   Author:    Jiri Brozovsky

   Copyright (C) 2008 Jiri Brozovsky

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

   Provides Maxwell-Mohr integrafion form MicroDefor data

   mmint file0.out file2.out file3.out... (max. 6 files)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 10

static FILE *fr[MAX_SIZE] ;  /* pointer for input files */
FILE *fw ;      /* file for results */
int   fnum = 0 ;
double actlineN[MAX_SIZE] ;
double actlineV[MAX_SIZE] ;
double actlineM[MAX_SIZE] ;
int    e_len, e_div ;
int    e_len_i, e_div_i ;
double L, E, I, A ;
int i ;
double K[MAX_SIZE][MAX_SIZE] ;
double F[MAX_SIZE] ;
double X[MAX_SIZE] ;

int open_ifiles(int argc, char *argv[])
{
  fnum  = 0 ;

  e_len = 0 ;
  e_div = 0 ;
  
  printf("argc = %i\n",argc);
  
  for (i=1; i<(argc); i++)
  {
    if (argv[i] == NULL) {return(fnum);}
    if (strlen(argv[i]) <= 0) {return(fnum);}

    printf("argv[%i] = %s\n",i,argv[i]);

    if ((fr[i]=fopen(argv[i],"r")) == NULL)
    {
      return(fnum);
    }
    else
    {
      if (i == 1)
      {
        fscanf(fr[i], "%i %i", &e_len, &e_div) ;
fprintf(stderr," E=%i N=%i\n", e_len, e_div);
        if ((e_len < 1) || (e_div < 1)) {return(0);}
      }
      else
      {
        fscanf(fr[i], "%i %i", &e_len_i, &e_div_i) ;
        if ((e_len_i < 1) || (e_div_i < 1)) {return(fnum);}

        if ( (e_len_i != e_len) || (e_div_i != e_div) )
        {
          return(fnum);
        }
      }

      fnum++ ;

    }
  }
  return(fnum);
}

void zero_flds(void)
{
  int i, j ;

  for (i=0; i<MAX_SIZE; i++)
  {
    F[i] = 0.0 ;
    X[i] = 0.0 ;

    for (j=0; j<MAX_SIZE; j++)
    {
      K[i][j] = 0.0 ;
    }
  }
}

/* Actual integration - only M is used: */
int do_integrate(void)
{
  int i, j, k, kk ;
  int i_i, i_j ;

  for (i=0; i<e_len; i++)
  {
    for (j=0; j<e_div; j++)
    {
      for (k=0; k<fnum; k++)
      {
        fscanf(fr[k+1], "%i %i %lf %lf %lf %lf %lf %lf %lf", 
            &i_i, &i_j, 
            &L, &E, &I, &A,
            &actlineN[k], &actlineV[k], &actlineM[k]
            ) ;

#if 0
        fprintf(stderr, "%i %i %f %f %f %f %f %f %f\n", 
            i_i, i_j, 
            L, E, I, A,
            actlineN[k], actlineV[k], actlineM[k]
            ) ;
#endif

      }
      for (k=1; k<fnum; k++)
      {
        F[k] -= (actlineM[k]*actlineM[0] * L ) / (E*I) ;

        for (kk=1; kk<fnum; kk++)
        {
          K[k][kk] += (actlineM[k]*actlineM[kk] * L ) / (E*I) ;
          K[kk][k] += (actlineM[k]*actlineM[kk] * L ) / (E*I) ;
        }
      }
    }
  }
  return(0);
}

void print_matrix(FILE *fw)
{
  int i, j ;

  fprintf(fw, "\n");

  for (i=1; i<fnum; i++)
  {
    for (j=1; j<fnum; j++)
    {
      fprintf(fw," %2.6e*X%i  + ", K[i][j], j);
    }
    fprintf(fw," %2.6e = 0\n", (-1)*F[i]);
  }
}


int main(int argc, char *argv[])
{
  if (argc < 3 ) 
  {
    fprintf(stderr,"Usage: %s file0 file1...\n",argv[0]);
    return(0);
  }

  /* recognize data: */
  if ( (fnum=open_ifiles(argc, argv)) <= 0)
  {
    fprintf(stderr,"No input files!\n");
    return(-1);
  }


  if (fnum >= MAX_SIZE) 
  {
    fprintf(stderr,"Max. %i files are allowed!\n", MAX_SIZE-1);
    return(-1);
  }


  /* prepare fields: */
  zero_flds();

  printf("fnum = %i\n", fnum);

  /* integrate: */
  if (do_integrate() != 0) 
  {
    fprintf(stderr, "Integration failed!\n");
    return(-1);
  }

  print_matrix(stdout) ;

  return(0);
}

/* end of mmint.c */
