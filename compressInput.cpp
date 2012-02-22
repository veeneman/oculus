#include "compress.h"

//function to check input file for readability
void check(ifstream& readfile)
{
  if(!readfile.good())
    {
      cerr << "Error: Input file isn't formatted correctly.\n";
      exit(1);
    } 
}

//This function compresses the input files into the unique set
// of reads.  It stores the sequences it has seen before in memory
// for the later reconstituion step.

void compressInput(ifstream& readfile1, ifstream& readfile2,
		   ofstream& compfile1, ofstream& compfile2,
		   bool SE_mode, bool& fQ_mode,
		   bool base4_mode, bool set_mode,
		   bool RC_mode, bool force_fastq_mode,
		   SET& single_set, MAP& multi_map,
		   vector<unsigned char*>& memory_blocks,
		   int& count, int& compressed_count)
{
  //reusable variables
  char header_line1[MLS];
  char sequence_line1[MLS];
  char useless_line1[MLS];
  char quality_line1[MLS];
  char header_line2[MLS];
  char sequence_line2[MLS];
  char useless_line2[MLS];
  char quality_line2[MLS];
  char lookup_seq[MLS*2];
  short int key_length;
  pair<SET::iterator,bool> key_lookup_sh;
  pair<MAP::iterator,bool> key_lookup_mh;
  int sequence_length;
  bool reversed = false;

  //using block memory allocation greatly improves memory use,
  // since there's significantly less overhead (pointing to blocks)
  // also there is a minor speed benefit
  unsigned char* current_block = (unsigned char*)malloc(BLOCK_SIZE);
  memory_blocks.push_back(current_block);
  int block_iter = 0;
  
  //Iterate through the input files
  while(readfile1.good())
    {
      readfile1.getline(header_line1,MLS);
      if(header_line1[0] == 0) //breaks on an empty line
	{
	  break;
	}

      count++;
      
      check(readfile1); //check is a function to verify the input file stream
      readfile1.getline(sequence_line1,MLS);
      if(fQ_mode) //4 lines
	{
	  check(readfile1);
	  readfile1.getline(useless_line1,MLS);
	  check(readfile1);
	  readfile1.getline(quality_line1,MLS);
	}
      
      if(!SE_mode) //Also read the second file
	{
	  check(readfile2);
	  readfile2.getline(header_line2,MLS);
	  check(readfile2);
	  readfile2.getline(sequence_line2,MLS);
	  if(fQ_mode)
	    {
	      check(readfile2);
	      readfile2.getline(useless_line2,MLS);
	      check(readfile2);
	      readfile2.getline(quality_line2,MLS);
	    }
	}

      //lookup seq will store the sequence of the reads in plain ascii
      lookup_seq[0] = 0;

      //This block loads lookup_seq, either from SE or PE
      if(SE_mode) //single end
	{
	  sequence_length = strnlen(sequence_line1,MLS); //this gets reused a lot

	  if(RC_mode)
	    {
	      //this function computes the reverse complement, IF the reverse complement
	      // is lexicographically smaller than the forward sequence.  This is important
	      // because it ensures consistent selection of the right sequence orientation.
	      oneWayRC(sequence_line1, lookup_seq, sequence_length, reversed);
	    }

	  if(!reversed)
	    {
	      lookup_seq[0] = 0;
	      strncat(lookup_seq,sequence_line1,MLS);
	    }
	}
      else //PE mode
	//Paired end reads are simply concatenated together, in a specific order
	{
	  if(RC_mode && (strncmp(sequence_line1,sequence_line2,MLS) > 0)) //reverse mode, and reverse order
	    {
	      reversed = true;
	      strncat(lookup_seq,sequence_line2,MLS);
	      strncat(lookup_seq,sequence_line1,MLS);
	    }
	  else //forward order
	    {
	      reversed = false;
	      strncat(lookup_seq,sequence_line1,MLS);
	      strncat(lookup_seq,sequence_line2,MLS);
	    }
	  sequence_length = strnlen(lookup_seq,MLS*2);
	}

      if(base4_mode)
	{
	  //its the length of the string divided by four,
	  // rounded up, plus the 2 bits of the size field
	  key_length = (sequence_length + 3)/4 + 2;
	}
      else
	{
	  //length of the string times 3/8ths
	  // rounded up, plus the 2 bits of the size field
	  key_length = ((((sequence_length * 3) + 7)/8) + 2);
	}
	  
      if((block_iter + key_length) > BLOCK_SIZE)
	{
	  //time to allocate a new block of memory,
	  // save it to vector to be freed later
	  current_block = (unsigned char*)malloc(BLOCK_SIZE);
	  memory_blocks.push_back(current_block);
	  block_iter = 0;
	}
      
      //key will store the compressed sequence.  it's basically just
      // a pointer into the memory block that gets written to (and if
      // that key isnt used, it gets overwritten)
      unsigned char* key = current_block + block_iter; //memory addressing

      //convert ascii to compressed sequence
      base4_mode ? compressSequence4(lookup_seq,key) : compressSequence(lookup_seq,key);
      
      if(set_mode) //use only a set, no map
	{
	  key_lookup_sh = single_set.insert(key);

	  if(key_lookup_sh.second) //inserted OK
	    {
	      compressed_count++;
	      block_iter += (((key[0] % 128) * 256 + key[1]) + 2); //roll forward memory

	      //since it's the first, print to file.
	      if(fQ_mode && !force_fastq_mode){ header_line1[0] = '>'; } //convert to fasta
	      compfile1 << header_line1 << "\n" << sequence_line1 << "\n";
	      if(fQ_mode && force_fastq_mode){compfile1 << useless_line1 << "\n" << quality_line1 << "\n";}
	      if(!SE_mode)
		{
		  if(fQ_mode && !force_fastq_mode){ header_line2[0] = '>'; } //convert to fasta
		  compfile2 << header_line2 << "\n" << sequence_line2 << "\n";
		  if(fQ_mode && force_fastq_mode){compfile2 << useless_line2 << "\n" << quality_line2 << "\n";}
		}
	    }
	  else //didn't insert - already present in the single set
	    {
	      key = *key_lookup_sh.first;
	      
	      //the pair of ints indicates the counts in forward and reverse orientation
	      if(reversed)
		{
		  key_lookup_mh = multi_map.insert(MAP_ENTRY(key,pair<int, int>(0,1)));
		}
	      else
		{
		  key_lookup_mh = multi_map.insert(MAP_ENTRY(key,pair<int, int>(1,0)));
		}

	      if(!key_lookup_mh.second) //already present in multi_map
		{
		  reversed ? key_lookup_mh.first->second.second++ : key_lookup_mh.first->second.first++;
		}
	    }
	}
      else //map-only mode
	{
	  key_lookup_mh = multi_map.insert(MAP_ENTRY(key,pair<int, int>(0,0)));
	  
	  if(key_lookup_mh.second) //inserted OK
	    {
	      compressed_count++; //increment count
	      block_iter += (((key[0] % 128) * 256 + key[1]) + 2); //roll forward memory
	      
	      //since it's the first, print to file.
	      if(fQ_mode && !force_fastq_mode){ header_line1[0] = '>'; } //convert to fasta
	      compfile1 << header_line1 << "\n" << sequence_line1 << "\n";
	      if(fQ_mode && force_fastq_mode){compfile1 << useless_line1 << "\n" << quality_line1 << "\n";}
	      if(!SE_mode)
		{
		  if(fQ_mode && !force_fastq_mode){ header_line2[0] = '>'; } //convert to fasta
		  compfile2 << header_line2 << "\n" << sequence_line2 << "\n";
		  if(fQ_mode && force_fastq_mode){compfile2 << useless_line2 << "\n" << quality_line2 << "\n";}
		}
	    }
	  else //didn't insert - it was in there already
	    {
	      reversed ? key_lookup_mh.first->second.second++ : key_lookup_mh.first->second.first++;
	    }
	}
    }

  if(!force_fastq_mode)
    // the input is no longer fastq, it has been converted to fasta
    {
      fQ_mode = false;
    }
}
