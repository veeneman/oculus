#include "compile_options.h"

//
// This header file is really important!
// It defines:
//  - the map type used for storing reads
//  - the comparison operator used for that map
//  - the hashing function
//
// I had to overload (write) the comparison operator and hashing function,
// because my compressed sequence objects aren't null terminated.  This
// was necessary, because four letter A's equate to 00000000, which is
// a null terminator (in biologist speak, an introduced stop codon).  Hashing
// operate only on the sequence field of the cseq object, not on the size field
// or the representation bit.  See cseq.cpp for a description of the compressed
// sequence objects.  Comparison will use the representation bit and size field
// because its a good way to shortcut comparison in the presence of N's or
// different read lengths.
//
// For the hashing function, I used Murmur hash, as recommended by the Google
// Sparse hash developer, Craig Silverstein, here:
//  http://groups.google.com/group/google-sparsehash/browse_thread/thread/1f54856f2e6bbce6
// Murmur hash has good spreading (low collision rates).  Specifically, I adapted a version of
// Murmur Hash 2 for my non-null-terminated purposes.
//
// A note about google sparse hash - it's really good.  mad props
//

#include <ext/hash_map>
#include <ext/hash_set>

using namespace std;
using __gnu_cxx::hash;
using namespace __gnu_cxx;

typedef pair<unsigned char*,pair<int, int> > MAP_ENTRY;

namespace __gnu_cxx
{
  template<> struct hash<unsigned char*>
  {
    //For this I modified MurmurHash 2 to work with my data type.  Hash is exactly
    // as I found it, except for noted lines.  
    size_t operator()(const unsigned char* key) const
    {
      // 'm' and 'r' are mixing constants generated offline.
      // They're not really 'magic', they just happen to work well.

      const uint32_t m = 0x5bd1e995;
      const int r = 24;

      int len = ((key[0] % 128) * 256) + key[1]; //modified by oculus developer, brendan
      const uint32_t seed = 0x5bd1e995; //ignorantly copied from m, added by oculus developer, brendan

      // Initialize the hash to a 'random' value
      
      uint32_t h = seed ^ len;
      
      // Mix 4 bytes at a time into the hash
      
      key += 2; //added - skip size bytes
      
      while(len >= 4)
	{
	  uint32_t k = *(uint32_t*)key;
	  
	  k *= m;
	  k ^= k >> r;
	  k *= m;
	  
	  h *= m;
	  h ^= k;
	  
	  key += 4;
	  len -= 4;
	}
      
      // Handle the last few bytes of the input array
      
      switch(len) //XXX - this looks suspiciously buggy
	{
	case 3: h ^= key[2] << 16;
	case 2: h ^= key[1] << 8;
	case 1: h ^= key[0];
	  h *= m;
	};

      // Do a few final mixes of the hash to ensure the last few
      // bytes are well-incorporated.
      
      h ^= h >> 13;
      h *= m;
      h ^= h >> 15;
      
      return size_t(h);      
    }
  };
}

struct EQ_comparison //hashmap needs an equals operator for comparison
{
  bool operator()(unsigned char* s1, unsigned char* s2) const
  {
    //I used to have a pointer shortcut comparison,
    // but it never got invoked by my code so it was removed
    
    //this compares both the size and the base bit
    if((s1[0] != s2[0]) || (s1[1] != s2[1]))
      {
	return false;
      }

    //iterate through the sequence fields, and compare.
    // returns false if they're different.  Note also
    // that since the sequence is compressed (probably to 2-bit),
    // this is 4 times faster than comparing ascii characters.
    // we know they're the same length, since we haven't exited yet.
    // by splitting it up we save some math for different cases,
    // especially since most seqs will probably be < 256bp
    short int i = 2;
    short int k = s1[1] + 2;
    for( ; i < k; i++)
      {
	if(s1[i] != s2[i])
	  {
	    return false;
	  }
      }
    
    k = k + ((s1[0] % 128) * 256);
    for( ; i < k; i++)
      {
	if(s1[i] != s2[i])
	  {
	    return false;
	  }
      }
    
    //we made it this far, they aren't different
    return true;
  }
};

#if MAPTYPE == 1
// Regular old STL (standard library) hash
typedef hash_map<unsigned char*, pair<int, int>, hash<unsigned char*>, EQ_comparison> MAP;
typedef hash_set<unsigned char*, hash<unsigned char*>, EQ_comparison> SET;

#else
// Google sparse hash
#include <google/sparse_hash_map>
#include <google/sparse_hash_set>
using google::sparse_hash_map;
using google::sparse_hash_set;
typedef sparse_hash_map<unsigned char*, pair<int, int>, hash<unsigned char*>, EQ_comparison> MAP;
typedef sparse_hash_set<unsigned char*, hash<unsigned char*>, EQ_comparison> SET;

#endif

