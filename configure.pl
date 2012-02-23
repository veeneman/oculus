#!/usr/bin/perl
#
# configuration script for oculus
#

sub usage()
{
  print STDERR "Installer Usage\n\n" . 
               " --g      <path>   Where <path> = Google Sparse Hash's include directory.\n" .
	       " --bwa    <path>   Where <path> = The BWA executable.\n" .
	       " --bowtie <path>   Where <path> = The Bowtie executable.\n\n" .
	       "Only Bowtie or BWA is required, but google sparse hash is highly recommended\n" .
	       "to save both time and memory.\n\n";
}

if($#ARGV < 1)
  {
    usage();
    exit(1);
  }

my $gsh = "";
my $bwa = "";
my $bwt = "";

my $i;
for($i = 0; $i <= $#ARGV; $i++)
  {
    if($ARGV[$i] eq "--g")
      {
	if($i == $#ARGV){ usage(); exit(1); }
	$i++;
	$gsh = $ARGV[$i];
      }
    elsif($ARGV[$i] eq "--bwa")
      {
	if($i == $#ARGV){ usage(); exit(1); }
	$i++;
	$bwa = $ARGV[$i];
      }
    elsif($ARGV[$i] eq "--bowtie")
      {
	if($i == $#ARGV){ usage(); exit(1); }
	$i++;
	$bwt = $ARGV[$i];
      }
    else
      {
	usage();
	exit(1);
      }
  }

if(!$bwa && !$bwt)
  {
    usage();
    exit(1);
  }

if($bwa && (!(-e $bwa) || !(-x $bwa) || !(-B $bwa)))
  {
    print STDERR "Error - $bwa doesn't look like a valid, executable, binary. Check permissions.\n";
    usage();
    exit(1);
  }

if($bwt && (!(-e $bwt) || !(-x $bwt) || !(-B $bwt)))
  {
    print STDERR "Error - $bwt doesn't look like a valid, executable, binary. Check permissions.\n";
    usage();
    exit(1);
  }

if($gsh && (!($gsh =~ m/include\/$/g) || !(-d $gsh)))
  {
    print STDERR "Error - $gsh doesn't look like the google sparse hash include directory.\n";
    usage();
    exit(1);
  }

my $map_type = 1;
if($gsh)
  {
    $gsh = "-I " . $gsh;
    $map_type = 2;
  }

#this is ironically the best way of editing the makefile.  it guarantees it never gets obliterated.

open(MAKEFILE,">makefile") or die "Couldn't open makefile.\n";

print MAKEFILE
"# oculus makefile
# brendan veeneman 3/2011

CC= g++
OPTS= -Wall
GOOG= $gsh
OBJS= usage.o parseArgs.o runAligner.o compressInput.o reconstruct.o cseq.o reverseComplement.o

all:oculus

oculus:oculus.cpp oculus.h map.h compile_options.h \$(OBJS)
	\$(CC) \$(OPTS) -Wno-deprecated \$(GOOG) oculus.cpp \$(OBJS) -o oculus

runAligner.o:runAligner.cpp runAligner.h compile_options.h
	\$(CC) \$(OPTS) -c runAligner.cpp

parseArgs.o:parseArgs.cpp parseArgs.h compile_options.h
	\$(CC) \$(OPTS) -c parseArgs.cpp

compressInput.o:compressInput.cpp compress.h map.h compile_options.h cseq.o reverseComplement.o
	\$(CC) \$(OPTS) -Wno-deprecated \$(GOOG) -c compressInput.cpp

reconstruct.o:reconstruct.cpp compress.h map.h compile_options.h cseq.o reverseComplement.o
	\$(CC) \$(OPTS) -Wno-deprecated \$(GOOG) -c reconstruct.cpp

reverseComplement.o:reverseComplement.cpp
	\$(CC) \$(OPTS) -c reverseComplement.cpp

cseq.o:cseq.cpp compile_options.h
	\$(CC) \$(OPTS) -c cseq.cpp

usage.o:usage.cpp usage.h compile_options.h
	\$(CC) \$(OPTS) -c usage.cpp

clean:
	rm -f oculus \$(OBJS)
";

close MAKEFILE;

open(COMPILEOPTS,">compile_options.h") or die "Couldn't open compile_options.h.\n";

print COMPILEOPTS
"#define BOWTIE \"$bwt\"
#define BWA    \"$bwa\"

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
#define MAPTYPE $map_type
";

close COMPILEOPTS;
