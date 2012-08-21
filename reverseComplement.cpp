using namespace std;

#include <iostream>
#include <fstream>

//this makes computing the reverse complement faster and simpler
//I think I stole this from Bowtie - thanks!
static char reverse_char[] = {
  /*   0 */   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
  /*  16 */  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  /*  32 */  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
  /*  48 */  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
  /*  64 */  64,  84,  66,  71,  68,  69,  70,  67,  72,  73,  74,  75,  76,  77,  78,  79,
  //               A         C                   G                                  N
  /*  80 */  80,  81,  82,  83,  65,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
  //                              T
  /*  96 */  96,  84,  98,  71, 100, 101, 102,  67, 104, 105, 106, 107, 108, 109, 110, 111,
  //               a         c                   g                                  n
  /* 112 */ 112, 113, 114, 115,  65, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
  //                              t
  /* 128 */ 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
  /* 144 */ 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
  /* 160 */ 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
  /* 176 */ 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
  /* 192 */ 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
  /* 208 */ 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
  /* 224 */ 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
  /* 240 */ 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, };

void reverseComplement(char* source, char* dest, int length) //XXX shouldn't be reverse order, its stupid
{
  for(int i = length - 1; i > -1; i--)
  {
    dest[length - i - 1] = reverse_char[(short int)source[i]];
  }
  dest[length] = 0;
}

void reverseOrder(char* input_string, int length)
{
  char temp;
  int k = length / 2;
  for(int i = 0; i < k; i++)
  {
    temp = input_string[i];
    input_string[i] = input_string[length - i - 1];
    input_string[length - i - 1] = temp;
  }
}

void oneWayRC(char* source, char* dest, int length, bool& reversed)
{
  reversed = false;
  int i;
  int halfway = (int)length / 2; //rounds down

  for(i = 0; i < halfway; i++)
  {
    dest[i] = reverse_char[(short int)source[length - i - 1]];
    dest[length - i - 1] = reverse_char[(short int)source[i]];

    if(source[i] < dest[i])
    //source char is lexicographically smaller than the RC char, quitting
    {
      return;
    }
    else if(source[i] > dest[i])
    //source char > RC char, so we do want to reverse.  Breaks out and enters loop without checking
    {
      reversed = true;
      i++;
      break;
    }
    //else - so far we don't know which is smaller.  continue.
    //note that if it successfully converts the whole sequence in this loop it was palindromic,
    // and by default this returns that it wasn't necessary to RC it.
  }

  for( ; i < halfway; i++)
  //this is exactly the same as the last loop, only it checks nothing.  time saving feature
  {
    dest[i] = reverse_char[(short int)source[length - i - 1]];
    dest[length - i - 1] = reverse_char[(short int)source[i]];
  }

  if(length % 2 == 1) // input was odd length
  {
    dest[halfway] = reverse_char[(short int)source[halfway]];
  }

  dest[length] = 0; //null term
}
