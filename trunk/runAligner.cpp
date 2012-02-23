#include "runAligner.h"

//This code will run either BWA or Bowtie (and potentially others in the future)
// and accepts user arguments.

//generic fork & exec function.
// I disabled the pipe redirection for now.
// A future option might be redirecting aligner stderr to
// dev/null, since it seems to report a lot of garbage XXX
void runThis(pid_t& pid, char** args)
{
  pid = fork();
  if(pid==0)
    //this is the child process
    {
      //turn this program into that command
      execv(args[0],args);
      
      //should never reach this line
      fprintf(stderr,"Error:Execution failed.\n");
      exit(1);
    }
  else if(pid < 0)
    {
      cerr << "Error:Execution failed.\n";
      exit(1);
    }
}

void runBowtie(pid_t& pid,
	       char* bowtie, char* database,
	       char* cfile1, char* cfile2,
	       char* alnfile,
	       bool SE_mode, bool fQ_mode,
	       char* arguments)
{
  //command to execute, args must be separate
  // I used vectors in these functions for convenience
  vector<char*> argv;
  
  argv.push_back(bowtie);
  argv.push_back(database);

  //add the user-specified arguments
  char* argpointer = strtok(arguments," ");
  while(argpointer != NULL)
    {
      argv.push_back(argpointer);
      argpointer = strtok(NULL," ");
    }
  
  if(!fQ_mode)
    {
      argv.push_back((char*)"-f"); //fasta - casting added
    }
  if(SE_mode)
    {
      argv.push_back(cfile1); //input file, don't need -1
    }
  else //PE_mode. input files
    {
      argv.push_back((char*)"-1");
      argv.push_back(cfile1);
      argv.push_back((char*)"-2");
      argv.push_back(cfile2);
    }
  argv.push_back((char*)"--sam"); //sam output
  argv.push_back(alnfile);
  argv.push_back((char*)0); //null terminator
  
  char *args[256];
  int i;
  for(i=0;i < (signed int)argv.size();i++)
  {
    args[i] = argv[i];
  }
  runThis(pid,args);
}

void runBWA(pid_t& pid,
	    char* BWA, char* database,
	    char* cfile1, char* cfile2,
	    char* sai1, char* sai2,
	    char* alnfile,
	    bool SE_mode, bool fQ_mode,
	    char* user_args1, char* user_args2, char* user_args3)
{
  //This is largely similar to running bowtie, except I'm using arrays here to reduce code
  //BWA needs to align and pair reads separately, so multiple invocations are needed
  // -f = fasta
  char* args1[100] = {BWA,(char*)"aln",database,cfile1,(char*)"-f",sai1};
  int i = 6;
  char* argpointer = strtok(user_args1," ");
  while(argpointer != NULL)
    {
      args1[i] = argpointer;
      argpointer = strtok(NULL," ");
      i++;
    }
  args1[i] = (char*)0;
  
  runThis(pid,args1);
  waitpid(pid,NULL,0);

  if(SE_mode) //single end
    {
      char* args2[100] = {BWA, (char*)"samse", database, sai1, cfile1, (char*)"-f", alnfile};
      i = 7;
      argpointer = strtok(user_args2," ");
      while(argpointer != NULL)
	{
	  args2[i] = argpointer;
	  argpointer = strtok(NULL," ");
	  i++;
	}
      args2[i] = (char*)0;
	
      runThis(pid,args2);
    }
  else //paired end
    {
      char* args2[100] = {BWA, (char*)"aln", database, cfile2, (char*)"-f", sai2};
      i = 6;
      argpointer = strtok(user_args2," ");
      while(argpointer != NULL)
	{
	  args2[i] = argpointer;
	  argpointer = strtok(NULL," ");
	  i++;
	}
      args2[i] = (char*)0;

      runThis(pid,args2);
      waitpid(pid,NULL,0);

      char* args3[100] = {BWA, (char*)"sampe", database, sai1, sai2, cfile1, cfile2, (char*)"-f", alnfile};
      i = 9;
      argpointer = strtok(user_args3," ");
      while(argpointer != NULL)
	{
	  args3[i] = argpointer;
	  argpointer = strtok(NULL," ");
	  i++;
	}
      args3[i] = (char*)0;
      
      runThis(pid,args3);
    }
}
