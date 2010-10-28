/*
 Program to generate the 120 flow shop instances proposed by
 Taillard, E.D.: "Benchmarks for basic scheduling problems",
 EJOR vol. 64, pp. 278-285, 1993
 
 Originaly written by Taillard in PASCAL, re-written in C by
 Dirk C. Mattfeld, University of Bremen <dirk@uni-bremen.de> and
 Rob J.M. Vaessens, Eindhoven University of Technology <robv@win.tue.nl>.
 Last update : 2/7/1996.

 UNIX compile: cc -o fsp_gen fsp_gen.c -lm

 Tested on the machines/systems/compilers listed below.
 DEC 5000    Ultrix 4.2    cc
 SUN 10      Solaris 2     cc
 IBM 6000    AIX 3.x       xlc
 Atari ST    TOS           pure c
 IBM-PC      DOS 6.2       MS-C 
 IBM-PC      Linux         gcc 

 Care should be taken on 64 bit machines (e.g. Cray). There, 
 'long' and 'double' contain 128 bit. In this case replace all 
 'long' and 'double' with the appropriate 64 bit type. 
  
     Verification file 'ta001' (if VERIFY == 1 and FIRMACIND == 1)

     20 5
     1 54 2 79 3 16 4 66 5 58 
     1 83 2 3 3 89 4 58 5 56 
     1 15 2 11 3 49 4 31 5 20 
     1 71 2 99 3 15 4 68 5 85 
     1 77 2 56 3 89 4 78 5 53 
     1 36 2 70 3 45 4 91 5 35 
     1 53 2 99 3 60 4 13 5 53 
     1 38 2 60 3 23 4 59 5 41 
     1 27 2 5 3 57 4 49 5 69 
     1 87 2 56 3 64 4 85 5 13 
     1 76 2 3 3 7 4 85 5 86 
     1 91 2 61 3 1 4 9 5 72 
     1 14 2 73 3 63 4 39 5 8 
     1 29 2 75 3 41 4 41 5 49 
     1 12 2 47 3 63 4 56 5 47 
     1 77 2 14 3 47 4 40 5 87 
     1 32 2 21 3 26 4 54 5 58 
     1 87 2 86 3 75 4 77 5 18 
     1 68 2 5 3 77 4 51 5 68 
     1 94 2 77 3 40 4 31 5 28 
*/ 

#define ANSI_C 0     /* 0:   K&R function style convention */
#define VERIFY 0     /* 1:   produce the verification file */ 
#define FIRMACIND 0  /* 0,1: first machine index           */ 

#include <stdio.h>
#include <math.h>

struct problem {
  long rand_time;      /* random seed for jobs */ 
  short num_jobs;      /* number of jobs */ 
  short num_mach;      /* number of machines */ 
};

#if VERIFY == 1

struct problem S[] = {
  {         0,  0, 0},
  { 873654221, 20, 5},
  {         0,  0, 0}};

#else /* VERIFY */ 
    
struct problem S[] = {
{         0,     0,  0},
                         /* 20 jobs  5 machines */ 
{ 873654221,    20,  5},  
{ 379008056,    20,  5}, 
{ 1866992158,   20,  5}, 
{ 216771124,    20,  5}, 
{ 495070989,    20,  5}, 
{ 402959317,    20,  5}, 
{ 1369363414,   20,  5}, 
{ 2021925980,   20,  5},
{ 573109518,    20,  5}, 
{ 88325120,     20,  5}, 
                          /* 20 jobs  10 machines */ 
{ 587595453,    20, 10},
{ 1401007982,   20, 10},
{ 873136276,    20, 10}, 
{ 268827376,    20, 10}, 
{ 1634173168,   20, 10},
{ 691823909,    20, 10}, 
{ 73807235,     20, 10}, 
{ 1273398721,   20, 10}, 
{ 2065119309,   20, 10}, 
{ 1672900551,   20, 10},
                          /* 20 jobs 20 machines */
{ 479340445,    20, 20},  
{ 268827376,    20, 20},
{ 1958948863,   20, 20},
{ 918272953,    20, 20},
{ 555010963,    20, 20},
{ 2010851491,   20, 20},
{ 1519833303,   20, 20},
{ 1748670931,   20, 20},
{ 1923497586,   20, 20},
{ 1829909967,   20, 20},
                          /* 50 jobs  5 machines */  
{ 1328042058,   50,  5}, 
{ 200382020,    50,  5},
{ 496319842,    50,  5},
{ 1203030903,   50,  5},
{ 1730708564,   50,  5},
{ 450926852,    50,  5},
{ 1303135678,   50,  5},
{ 1273398721,   50,  5},
{ 587288402,    50,  5},
{ 248421594,    50,  5},
                          /* 50 Jobs 10 machines */ 
{ 1958948863,   50, 10},
{ 575633267,    50, 10},
{ 655816003,    50, 10}, 
{ 1977864101,   50, 10},
{ 93805469,     50, 10},
{ 1803345551,   50, 10},  
{ 49612559,     50, 10},
{ 1899802599,   50, 10},
{ 2013025619,   50, 10},
{ 578962478,    50, 10},
                          /* 50 jobs 20 machines */ 
{ 1539989115,   50, 20},
{ 691823909,    50, 20},
{ 655816003,    50, 20}, 
{ 1315102446,   50, 20}, 
{ 1949668355,   50, 20},
{ 1923497586,   50, 20},
{ 1805594913,   50, 20},
{ 1861070898,   50, 20}, 
{ 715643788,    50, 20}, 
{ 464843328,    50, 20}, 
                          /* 100 jobs  5 machines */ 
{ 896678084,   100,  5},
{ 1179439976,  100,  5}, 
{ 1122278347,  100,  5}, 
{ 416756875,   100,  5},
{ 267829958,   100,  5}, 
{ 1835213917,  100,  5}, 
{ 1328833962,  100,  5}, 
{ 1418570761,  100,  5}, 
{ 161033112,   100,  5},
{ 304212574,   100,  5}, 
                          /* 100 jobs 10 machines */ 
{ 1539989115,  100, 10},
{ 655816003,   100, 10}, 
{ 960914243,   100, 10}, 
{ 1915696806,  100, 10},
{ 2013025619,  100, 10}, 
{ 1168140026,  100, 10}, 
{ 1923497586,  100, 10}, 
{ 167698528,   100, 10}, 
{ 1528387973,  100, 10}, 
{ 993794175,   100, 10}, 
                          /* 100 jobs 20 machines */
{ 450926852,   100, 20},
{ 1462772409,  100, 20}, 
{ 1021685265,  100, 20}, 
{ 83696007,    100, 20}, 
{ 508154254,   100, 20}, 
{ 1861070898,  100, 20}, 
{ 26482542,    100, 20}, 
{ 444956424,   100, 20}, 
{ 2115448041,  100, 20}, 
{ 118254244,   100, 20}, 
                          /* 200 jobs 10 machines */ 
{ 471503978,   200, 10},
{ 1215892992,  200, 10}, 
{ 135346136,   200, 10}, 
{ 1602504050,  200, 10}, 
{ 160037322,   200, 10}, 
{ 551454346,   200, 10}, 
{ 519485142,   200, 10}, 
{ 383947510,   200, 10}, 
{ 1968171878,  200, 10}, 
{ 540872513,   200, 10}, 
                          /* 200 jobs 20 machines */
{ 2013025619,  200, 20},
{ 475051709,   200, 20}, 
{ 914834335,   200, 20}, 
{ 810642687,   200, 20},  
{ 1019331795,  200, 20}, 
{ 2056065863,  200, 20}, 
{ 1342855162,  200, 20}, 
{ 1325809384,  200, 20}, 
{ 1988803007,  200, 20}, 
{ 765656702,   200, 20}, 
                          /* 500 jobs 20 machines */
{ 1368624604,  500, 20},
{ 450181436,   500, 20}, 
{ 1927888393,  500, 20}, 
{ 1759567256,  500, 20}, 
{ 606425239,   500, 20}, 
{ 19268348,    500, 20}, 
{ 1298201670,  500, 20}, 
{ 2041736264,  500, 20},
{ 379756761,   500, 20},
{ 28837162,    500, 20},
{          0,    0,  0}};
#endif /* VERIFY */

/* generate a random number uniformly between low and high */

#if ANSI_C == 1
int unif (long *seed, short low, short high)
#else
short unif (seed, low, high)
long *seed; short low, high;
#endif
{
  static long m = 2147483647, a = 16807, b = 127773, c = 2836;
  double  value_0_1;              

  long k = *seed / b;
  *seed = a * (*seed % b) - k * c;
  if(*seed < 0) *seed = *seed + m;
  value_0_1 =  *seed / (double) m;

  return (short) (low + floor(value_0_1 * (high - low + 1)));
}

/* Maximal 500 jobs and 20 machines are provided. */
/* For larger problems extend array sizes.        */ 

short d[21][501];                       /* duration */ 

#if ANSI_C == 1
void generate_flow_shop(short p)          /* Fill d and M according to S[p] */ 
#else
void generate_flow_shop(p)
short p;
#endif
{
  short i, j;
  long time_seed = S[p].rand_time;
  
  for(i = 0; i < S[p].num_mach; ++i)      /* determine a random duration */ 
    for (j = 0; j < S[p].num_jobs; ++j)   /* for all operations */ 
      d[i][j] = unif(&time_seed, 1, 99);  /* 99 = max. duration of op. */
}

#if ANSI_C == 1
void write_problem(short p)  /* write out problem */ 
#else
void write_problem(p)
short p;
#endif
{
  short i, j;
  FILE *f = NULL;
  char name[6];

  sprintf(name,"ta%03d.prb", p);                 /* file name construction */ 
  if(!(f = fopen(name,"w"))) {               /* open file for writing  */ 
    fprintf(stderr,"file %s error\n", name);
    return;
  }
  fprintf(f,"Taillard %d x %d (Problem %d)\n", S[p].num_jobs, S[p].num_mach, p);   /* write header line */
  fprintf(f,"%d %d\n", S[p].num_jobs, S[p].num_mach); /* write header line */ 
  
  for(j = 0; j < S[p].num_jobs; ++j) {
    for(i = 0; i < S[p].num_mach; ++i) {
       fprintf(f,"%2d %2d ", i+FIRMACIND, d[i][j]);   /* write machine and job */ 
    }
    fprintf(f,"\n");                         /* newline == End of job */ 
  }
  fclose(f);                                 /* close file */ 
}


int main()                                    
{
  short i = 1;
  while(S[i].rand_time) {                    /* for i == 1 up to NULL entry */
    generate_flow_shop(i);                   /* generate problem i  */ 
    write_problem(i);                        /* write out problem i */ 
    ++i;                                     /* increment i */ 
  }
  return 0;
}

