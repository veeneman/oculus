#include <iostream>
#include <fstream>
#include <vector>
#include "map.h"
#include "compile_options.h"
using namespace std;

void compressSequence(char*,unsigned char*);
void compressSequence4(char*, unsigned char*);
void print_cseq(unsigned char*);
void reverseComplement(char*, char*, int);
void oneWayRC(char*, char*, int, bool&);
void check(ifstream&);
