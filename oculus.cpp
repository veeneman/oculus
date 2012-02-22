#include "oculus.h"

//Main function of oculus, a program designed to
// improve linear aligner speeds, especially in the
// case of transcriptomics, where redundancy is 
// both common and desirable (quantitativeness!)
//
// Oculus was written by Brendan Veeneman in 2011
// contact: veeneman at umich dot edu
//

int main(int argc, char** argv)
{
  //
  // Execution time measurement
  //
  timeval times[8];
  gettimeofday(&times[0],NULL);
  //
  // Variables
  //
  bool SE_mode;
  bool fQ_mode;
  bool bowtie_mode      = true;
  bool silent_mode      = false;
  bool base4_mode       = false;
  bool set_mode         = false;
  bool RC_mode          = false;
  bool force_fastq_mode = false;
  SET single_set;
  MAP multi_map;
  int total_lines = 0;
  int compressed_lines = 0;
  vector<unsigned char*> memory_blocks;
  
  //
  // Strings
  //
  char         database[MAX_FILENAME_LENGTH];
  char        inputfp_1[MAX_FILENAME_LENGTH];
  char        inputfp_2[MAX_FILENAME_LENGTH];
  char     outputprefix[MAX_FILENAME_LENGTH];
  char   compressedfp_1[MAX_FILENAME_LENGTH];
  char   compressedfp_2[MAX_FILENAME_LENGTH];
  char            sai_1[MAX_FILENAME_LENGTH];
  char            sai_2[MAX_FILENAME_LENGTH];
  char       alignments[MAX_FILENAME_LENGTH];
  char alignments_recon[MAX_FILENAME_LENGTH];
  char     aligner_args[MAX_ARG_LEN];
  char     aligner_args2[MAX_ARG_LEN];
  char     aligner_args3[MAX_ARG_LEN];
  //blank them
  compressedfp_1[0] = 0;
  compressedfp_2[0] = 0;
  sai_1[0] = 0;
  sai_2[0] = 0;
  alignments[0] = 0;
  alignments_recon[0] = 0;
  
  //
  // Parse Arguments
  //
  parseArgs(argc,argv,
	    database,inputfp_1,inputfp_2,
	    outputprefix,
	    aligner_args,aligner_args2,aligner_args3,
	    SE_mode,fQ_mode,
	    base4_mode,silent_mode,
	    set_mode,bowtie_mode,
	    RC_mode, force_fastq_mode);

  //generate output file paths based on user-specified output prefix
  strncat(strncat(compressedfp_1  ,outputprefix,MAX_FILENAME_LENGTH - 5),".cf1",5);
  strncat(strncat(compressedfp_2  ,outputprefix,MAX_FILENAME_LENGTH - 5),".cf2",5);
  strncat(strncat(sai_1           ,outputprefix,MAX_FILENAME_LENGTH - 6),".sai1",6);
  strncat(strncat(sai_2           ,outputprefix,MAX_FILENAME_LENGTH - 6),".sai2",6);
  strncat(strncat(alignments      ,outputprefix,MAX_FILENAME_LENGTH - 5),".cmp",5);
  strncat(strncat(alignments_recon,outputprefix,MAX_FILENAME_LENGTH - 5),".sam",5);

  if(!silent_mode)
    //silent mode basically means to shut up oculus's output
    //  - by default it'll print run time, and stats about how its working
    {
      cout << "Input:\n";
      SE_mode ? cout << "  Single end " : cout << "  Paired end ";
      fQ_mode ? cout << "fastQ\n" : cout << "fastA\n";
      cout << "Options:\n";
      base4_mode ? cout << "  Forcing 2 bit nucleotides\n" : cout << "";
      if(force_fastq_mode){ cout << "  Forcing fastQ intermediate file\n"; }
      if(RC_mode){ cout << "  Storing reverse complements together\n"; }
      bowtie_mode ? cout << "Aligner:\n  Bowtie\n" : cout << "Aligner:\n  BWA\n";
      
      //
      // Print the type of map that has been compiled in.  Uses compiler flags
      //
#if MAPTYPE == 1
      cout << "Storage:\n  Hashmap";
#elif MAPTYPE == 2
      cout << "Storage:\n  Google Sparse Hashmap";
#else
      cerr << "Error: Maptype isn't correctly defined, exiting...\n";
      exit(1);
#endif
      set_mode ? cout << " + Hashset\n" : cout << "\n";
      
      gettimeofday(&times[1],NULL);
      cout << "Compressing Input...";
      cout.flush();
    }

  //
  // Compress input file/s and populate map
  //
  ifstream readfile1(inputfp_1); // the files are opened out here to facilitate measuring file write time
  ofstream compfile1(compressedfp_1);
  ifstream readfile2;
  ofstream compfile2;
  if(!SE_mode)
    {
      readfile2.open(inputfp_2);
      compfile2.open(compressedfp_2);
    }

  compressInput(readfile1,readfile2,
		compfile1,compfile2,
		SE_mode,fQ_mode,
		base4_mode, set_mode,
		RC_mode,force_fastq_mode,
		single_set,multi_map,
		memory_blocks,
		total_lines,compressed_lines);
  
  if(!silent_mode)
    {
      gettimeofday(&times[2],NULL);
      cout << "done.\nWriting to file...";
      cout.flush();
    }

  //
  // Flush compresssed file buffers
  //
  readfile1.close();
  readfile2.close();
  compfile1.close();
  compfile2.close();

  if(!silent_mode)
    {
      gettimeofday(&times[3],NULL);
      bowtie_mode ? cout << "done.\nRunning Bowtie...\n" : cout << "done.\nRunning BWA...\n";
      cout.flush();
    }

  //
  // Run the Aligner
  //
  pid_t pid;
  int exit_status;

  if(bowtie_mode)
    {
      runBowtie(pid,BOWTIE,database,compressedfp_1,compressedfp_2,alignments,SE_mode,fQ_mode, aligner_args);
    }
  else
    {
      runBWA(pid,BWA,database,
	     compressedfp_1,compressedfp_2,sai_1,sai_2,alignments,
	     SE_mode,fQ_mode,
	     aligner_args, aligner_args2, aligner_args3);
    }
  cout.flush();
  waitpid(pid,&exit_status,0);
  if(!WIFEXITED(exit_status) || WEXITSTATUS(exit_status) != 0)
    {
      cerr << "Error: Aligner crashed.\n";
      exit(1);
    }

  if(!silent_mode)
    {
      gettimeofday(&times[4],NULL);
      cout << "done.\nReconstituing Compressed Hits...";
      cout.flush();
    }

  //
  // Reconstruct the hits that we removed before.  last functional step
  //
  ifstream alignfile(alignments);
  ofstream reconfile(alignments_recon);

  reconstruct(alignfile, reconfile,
	      SE_mode, fQ_mode,base4_mode, RC_mode,
	      multi_map);
  
  if(!silent_mode)
    {
      gettimeofday(&times[5],NULL);
      cout << "done.\nWriting to file...";
      cout.flush();
    }
      
  alignfile.close();
  reconfile.close();

  if(!silent_mode)
    {
      gettimeofday(&times[6],NULL);
      cout << "done.\nFreeing memory...";
      cout.flush();
    }
  
  //collecting garbage
  vector<unsigned char*>::iterator memory_it;
  for(memory_it=memory_blocks.begin();memory_it != memory_blocks.end();memory_it++)
    {
      free((void*)*memory_it);
    }
  
  if(!silent_mode)
    {
      gettimeofday(&times[7],NULL);
      cout << "done.\nTotal entries:          " << total_lines
	   << "\nCompressed entries:     " << compressed_lines
	   << "\nBowtie runtime:         "
	   << getTimeElapsed(times[3],times[4])
	   << "s\nExplicit file i/o wait: "
	   << getTimeElapsed(times[2],times[3]) +
              getTimeElapsed(times[5],times[6])
	   << "s\nTime in Oculus:         "
	   << getTimeElapsed(times[0],times[2]) +
	      getTimeElapsed(times[4],times[5]) +
              getTimeElapsed(times[6],times[7])
	   << "s\nTotal runtime:          "
	   << getTimeElapsed(times[0],times[7])
	   << "s\nEstimated time saved:   "
	   << (((double)total_lines / (double)compressed_lines) *
	       getTimeElapsed(times[3],times[4])) - getTimeElapsed(times[0],times[7]) << "\n";
    }

  return 0;
}
