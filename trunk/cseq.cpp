#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "compile_options.h"

using namespace std;

// In addition to the map header, this code lies at the heart of the
// sequence storage.  The apparently magic numbers all result from
// some serious equation reduction.

// Array for fast ascii - binary compression.  This one uses 2 bits (Nn -> A)
static unsigned char charToDNA4[] = {
  /*   0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  16 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  32 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  48 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  64 */ 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0,
  //           A     C           G                    N
  /*  80 */ 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //                    T
  /*  96 */ 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0,
  //           a     c           g                    n
  /* 112 */ 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //                    t
  /* 128 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 144 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 160 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 176 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 192 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 208 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 224 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 240 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

// Array for fast ascii - binary compression.  This one uses 3 bits (Nn -> N)
//  Ostensibly there are 3 unused letters.  I haven't seen anything other
//  than acgtnACGTN, but I think others could be possible in the future.
static unsigned char charToDNA8[] = {
  /*   0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  16 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  32 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  48 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  64 */ 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 4, 0,
  //           A     C           G                    N
  /*  80 */ 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //                    T
  /*  96 */ 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 4, 0,
  //           a     c           g                    n
  /* 112 */ 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //                    t
  /* 128 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 144 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 160 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 176 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 192 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 208 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 224 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 240 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

//
// This function is more simple than the other so its easier to explain
//  Basically, take a sequence of characters, and convert each one to
//  two bits.  Since AAAA = 0 = null termination of string, you can't
//  rely on null termination functions, and the size of the array becomes
//  necessary.  This version forces base4.
//
void compressSequence4(char* sequence, unsigned char* cseq)
{
  short int i = 8;
  
  //stole this from stl - increment local pointer & compare to null
  for( ; *sequence; ++sequence)
    {
      cseq[i >> 2] <<= 2;
      cseq[i >> 2] ^= charToDNA4[(int)*sequence];
      i++;
    }

  //in memory size, 0 in 128 bit indicates base4
  cseq[0] = (i - 5) >> 10;
  cseq[1] = ((i - 5) >> 2) & 0xFF;

  //shifting last char to the left so comparison still works
  if(i & 3)
    {
      cseq[i >> 2] <<= 8 - 2*(i & 3);
    }
}

//
// This function is more complicated - conceptually its the same as the
//  base 4 function, except using base3.  There's an ugly engineering foible
//  that falls out of that, which is that characters can span char ends. I could
//  have done it with boolean logic, but I chose to use an int buffer (4 chars)
//  instead.  Technically, base 8 could also use 0/null termination, but
//  for the sake of code compatibility I'm not implementing it.
//
void compressSequence8(char* sequence, unsigned char* cseq)
{
  short int i = 0;
  int buffer = 0;

  //same iteration trickery
  for( ; *sequence; ++sequence)
    {
      i++;
      buffer <<= 3;
      buffer ^= charToDNA8[(int)*sequence];
      if(i % 8 == 0) //flush buffer to cseq
	{
	  //the mods and division here are equivalent to bit shifting out 4 blocks
	  // of 8 bits each.  .375 = 3/8 = the 8 to 3 indexing transform
	  cseq[(3 * i >> 3) - 1] = buffer >> 16;
	  cseq[(3 * i) >> 3]     = (buffer >> 8) & 0xFF;
	  cseq[(3 * i >> 3) + 1] = buffer & 0xFF;
	  buffer = 0;
	}
    }

  //since the comparison operators don't know if it's 2 or 3-bit,
  // I encapsulate that information in the first size byte. 1 in 128 bit indicates base8
  //multiplying by 3 first could cause overflow, but id rather do that than floating point algebra

  cseq[0] = ((i * 3) + 262151) >> 11;
  cseq[1] = (((i * 3) + 7) >> 3) & 0xFF;

  //flush the remainder of the buffer

  if((i % 8) > 0)
    {
      buffer = buffer << (24 - 3 * (i % 8)) % 8;
      cseq[((i * 3) + 15) >> 3] = buffer & 0xFF;

      if((i % 8) > 2)
	{
	  cseq[((i * 3) + 7)  >> 3] = (buffer >> 8) & 0xFF;

	  if((i % 8) > 5)
	    {
	      cseq[((i * 3) - 1)  >> 3] = buffer >> 16;
	    }
	}
    }
}

//This is the dynamic sequence compressor.  If it doesn't see any N's,
// it uses 2-bit nucleotide representation.  If it does - it kicks
// off the 3-bit function.  The benefit of this is great - It allows
// N's to be represented correctly, while using ~ 2/3rds less memory for
// the majority of sequences, without N's.
void compressSequence(char* o_sequence, unsigned char* cseq)
{
  char* sequence = o_sequence;
  short int i = 8;
  unsigned char lookup;
  for( ; *sequence; ++sequence)
    {
      lookup = charToDNA8[(int)*sequence];
      if(lookup == 4)  //found an N/n
	{
	  compressSequence8(o_sequence,cseq);
	  return;
	}
      cseq[i >> 2] <<= 2;
      cseq[i >> 2] ^= lookup;
      i++;
    }

  //in memory size, 0 in 128 bit indicates base4
  cseq[0] = (i - 5) >> 10;
  cseq[1] = ((i - 5) >> 2) & 0xFF;
  
  //shifting last char to the left so comparison still works
  if(i & 3)
    {
      cseq[i >> 2] <<= 8 - 2*(i & 3);
    }
}

//printing function for debugging
void print_cseq(unsigned char* cseq)
{
  short int size = (cseq[0] % 128) * 256 + cseq[1] + 2;

  short int i;

  for(i = 0; i < size; i++)
    {
      cout << "[" << (unsigned int)cseq[i] << "]";
    }
  
  cout << "\n";
}
