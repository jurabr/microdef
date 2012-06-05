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


#include "microdef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

extern int solve(void);

extern void set_mdterm(int term);
extern int get_mdterm(void);
extern int md_write_report( char *fname, int term, int print_input, int print_ke, int  print_k, int print_res, int print_rlist);

char *data_file = NULL ;

/* main program loop */
int main(int argc, char *argv[]) 
{ 
  int rv, i, len ;

  data_file       = NULL ;

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
	    fprintf(stderr,"Invalid data file!\n");
		  return(-1);
    }
  }
  else
  {
	  fprintf(stderr,"Data file must be specified!\n");
		return(-1);
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
      md_write_report( data_file, MDOUT_TEXT, 1, 0, 0, 1, 0);
	    fprintf(stdout,"done!\n");
    }
  }

	fprintf(stdout,"\nAll work done!\n");

  return(OK); 
}

/* end of md_cli.c */
