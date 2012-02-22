#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <sstream>
#include <wait.h>
#include <sys/time.h>
#include <vector>
#include "map.h"
#include "compile_options.h"

using namespace std;

//function prototypes
void parseArgs(int,char**,
	       char*,char*,
	       char*,char*,
	       char*,char*,
	       char*,
	       bool&,bool&,bool&,bool&,
	       bool&,bool&,bool&,bool&);
void usage(int);
void runBowtie(pid_t&,char*,char*,char*,char*,char*,bool,bool,char*);
void runBWA(pid_t&,char*,char*,char*,char*,char*,char*,char*,bool,bool,char*,char*,char*);
void compressInput(ifstream&, ifstream&,
		   ofstream&, ofstream&,
		   bool, bool&,
		   bool, bool,
		   bool, bool,
		   SET&, MAP&,
		   vector<unsigned char*>&,
		   int&, int&);
void reconstruct(ifstream&, ofstream&,
		 bool, bool, bool, bool,
		 MAP&);

double getTimeElapsed(timeval start, timeval now)
{
  return (now.tv_sec - start.tv_sec) + ((now.tv_usec - start.tv_usec)/1000000.0);
}
