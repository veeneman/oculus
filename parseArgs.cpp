#include "parseArgs.h"

// This is a really mundane function that reads the input
// arguments and parses them.  I won't bother to explain it or comment it much,
// but a couple interesting points are that it automatically determines
// if the input files are fastA or fastQ, and defaults output to "output" if
// nothing is specified.
void parseArgs(int argc, char** argv,
               char* database,
               char* input1,char* input2,
               char* outputprefix,
               char* aligner_args, char* aligner_args2, char* aligner_args3,
               bool& SE, bool& fQ,
               bool& base4, bool& silent,
               bool& set_mode, int& aligner_mode,
               bool& RC_mode, bool& force_fastq_mode, bool& qual_mode,
               bool& gzip_cfiles, int& compress_mode,
	       long& trim_left, long& trim_length, bool& count_mode)
{
  database[0] = input1[0] = input2[0] = outputprefix[0] = 0;

  if(argc == 1)
  {
    usage(1);
  }

  for(int i = 1; i < argc; i++)
  {

    if(strncmp(argv[i],"-a2",3) == 0 ||
       strncmp(argv[i],"--args2",7) == 0)
    {
      if(i == (argc - 1))
        {
          cerr << "Error: Missing argument to --args.\n";
          exit(1);
        }
      i++;
      aligner_args2[0] = 0;
      strncat(aligner_args2,argv[i],MAX_ARG_LEN);
    }
    else if(strncmp(argv[i],"-a3",3) == 0 ||
            strncmp(argv[i],"--args3",7) == 0)
    {
      if(i == (argc - 1))
      {
        cerr << "Error: Missing argument to --args.\n";
        exit(1);
      }
      i++;
      aligner_args3[0] = 0;
      strncat(aligner_args3,argv[i],MAX_ARG_LEN);
    }
    else if(strncmp(argv[i],"-a",2) == 0 ||
            strncmp(argv[i],"-a1",3) == 0 ||
            strncmp(argv[i],"--args",6) == 0 ||
            strncmp(argv[i],"--args1",7) == 0)
    {
      if(i == (argc - 1))
      {
        cerr << "Error: Missing argument to --args.\n";
        exit(1);
      }
      i++;
      aligner_args[0] = 0;
      strncat(aligner_args,argv[i],MAX_ARG_LEN);
    }
    else if(strncmp(argv[i],"-h",2) == 0 ||
            strncmp(argv[i],"--h",3) == 0 ||
            strncmp(argv[i],"-help",5) == 0 ||
            strncmp(argv[i],"--help",6) == 0)
    {
      usage(0);
    }
    else if(strncmp(argv[i],"-s",2) == 0 ||
            strncmp(argv[i],"--silent",8) == 0)
    {
      silent = true;
    }
    else if(strncmp(argv[i],"-4",2) == 0 ||
            strncmp(argv[i],"--base4",7) == 0)
    {
      base4 = true;
    }
    else if(strncmp(argv[i],"--set",6) == 0)
    {
      set_mode = true;
    }
    else if(strncmp(argv[i],"--rc",5) == 0)
    {
      RC_mode = true;
    }
    else if(strncmp(argv[i],"--bwa",6) == 0)
    {
      aligner_mode = 1;
    }
    else if(strncmp(argv[i],"--custom",9) == 0)
    {
      aligner_mode = 2;
    }
    else if(strncmp(argv[i],"--ffq",6) == 0)
    {
      force_fastq_mode = true;
    }
    else if(strncmp(argv[i],"--gzip",7) == 0)
    {
      gzip_cfiles = true;
    }
    else if(strncmp(argv[i],"--gz_input",11) == 0)
    {
      compress_mode = 1;
    }
    else if(strncmp(argv[i],"--bz_input",11) == 0)
    {
      compress_mode = 2;
    }
    else if(strncmp(argv[i],"--restore_qual",15) == 0)
    {
      qual_mode = true;
    }
    else if(strncmp(argv[i],"--count",8) == 0)
    {
      count_mode = true;
    }
    else if(strncmp(argv[i],"--trim_left",12) == 0)
    {
      trim_left = strtol(argv[i+1],NULL,10);
      if(trim_left == 0)
      {
	cerr << "Error: trim_left must be a positive integer if specified.\n";
	exit(1);
      }
      i++;
    }
    else if(strncmp(argv[i],"--trim_length",14) == 0)
    {
      trim_length = strtol(argv[i+1],NULL,10);
      if(trim_length == 0)
      {
	cerr << "Error: trim_length must be a positive integer if specified.\n";
	exit(1);
      }
      i++;
    }
    else if(strncmp(argv[i],"-d",3) == 0)
    {
      if(i == (argc - 1) || argv[i+1][0] == '-')
      {
        cerr << "Error: Database not specified.\n";
        exit(1);
      }
      i++;
      strncat(database,argv[i],MAX_ARG_LEN);
    }
    else if(strncmp(argv[i],"-o",2) == 0)
    {
      if(i == (argc - 1) || argv[i+1][0] == '-')
      {
        cerr << "Error: Output not specified.\n";
        exit(1);
      }
      i++;
      strncat(outputprefix,argv[i],MAX_ARG_LEN);
    }
    else if(strncmp(argv[i],"-1",2) == 0)
    {
      if(i == (argc - 1) || argv[i+1][0] == '-')
      {
        cerr << "Error: Input file 1 not specified.\n";
        exit(1);
      }
      i++;
      ifstream test_file(argv[i]);
      if(!test_file)
      {
        cerr << "Error: " << argv[i] << " is not an openable file.\n";
        exit(1);
      }
      test_file.close();
      strncat(input1,argv[i],MAX_ARG_LEN);
    }
    else if(strncmp(argv[i],"-2",2) == 0)
    {
      if(i == (argc - 1) || argv[i+1][0] == '-')
      {
        cerr << "Error: Input file 2 not specified.\n";
        exit(1);
      }
      i++;
      ifstream test_file(argv[i]);
      if(!test_file)
      {
        cerr << "Error: " << argv[i] << " is not an openable file.\n";
        exit(1);
      }
      test_file.close();
      strncat(input2,argv[i],MAX_ARG_LEN);
    }
    else
    {
      cerr << "Error: Unrecognized input : " << argv[i] << "\n";
      exit(1);
    }
  }

  if(input2[0] == 0)
  {
    SE = true;
  }
  else
  {
    SE = false;
  }
  if(database[0] == 0)
  {
    cerr << "Error: You must specify a database to search against.\n";
    exit(1);
  }
  if(input1[0] == 0)
  {
    cerr << "Error: You must specify at least 1 fastA or fastQ query.\n";
    exit(1);
  }
  if(outputprefix[0] == 0)
  {
    strncat(outputprefix,"output",7); //default output path, creatively named "output"
  }
  if(set_mode && qual_mode)
  {
    cerr << "Error: sets and quality retrieval are incompatible.\n";
    exit(1);
  }
  if(qual_mode && RC_mode)
  {
    cerr << "Error: reverse-complement storage and quality retrieval are incompatible.\n";
    exit(1);
  }
  if(qual_mode && count_mode)
  {
    cerr << "Error: count-based output and quality retrieval are incompatible.\n";
    exit(1);
  }

  if(SE)
  {
    ifstream testfile1(input1);
    if(testfile1.good())
    {
      char first1 = testfile1.peek();
      if(first1 == '>')
      {
        fQ = false;
      }
      else if(first1 == '@')
      {
        fQ = true;
      }
      else
      {
        cerr << "Error: The file format is not recognized.\n";
        exit(1);
      }
    }
    testfile1.close();
  }
  else
  {
    ifstream testfile1(input1);
    ifstream testfile2(input2);
    if(testfile1.good() && testfile2.good())
    {
      char first1 = testfile1.peek();
      char first2 = testfile2.peek();
      if(first1 == '>' && first2 == '>')
      {
        fQ = false;
      }
      else if(first1 == '@' && first2 == '@')
      {
        fQ = true;
      }
      else
      {
        cerr << "Error: The files are not recognized, or are not the same format.\n";
        exit(1);
      }
    }
    testfile1.close();
    testfile2.close();
  }
}
