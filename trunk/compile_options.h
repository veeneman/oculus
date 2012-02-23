#define BOWTIE ""
#define BWA    "fff"

//maximum length of filenames (including path, and suffices)
#define MAX_FILENAME_LENGTH 512
//maximum length of an arg into oculus.  includes white space, and the aligner args fields.
#define MAX_ARG_LEN 1024

//maximum line size in the input file/s
#define MLS 10000
//size of the blocks of memory used for storing compressed reads
// in memory.  tweaking this could improve memory and speed performance.
#define BLOCK_SIZE 10240 //1kb
//#define BLOCK_SIZE 1048576 //1mb

// MAPTYPE - Which type of map to use, google or C standard library (STL).
//  1 = hashmap
//  2 = google sparse_hashmap
#define MAPTYPE 1
