#!/usr/bin/perl
#
# configuration script for oculus
#

sub usage()
{
  print STDERR "Installer Usage\n\n" . 
               "One of the following is required (supply as many as desired)\n\n" .
  	             " --bwa    <BWA executable>\n" .
        	       " --bowtie <Bowtie executable>\n" .
       	       " --customse <custom aligner string>\n" .
        	       " --custompe <custom aligner string>\n\n" .
        	       "Optional\n\n" .
               " --g      <Google Sparse Hash's include directory>\n\n" .
        	       "Executables and the include directory should be full paths (starting with /).\n\n" .
        	       "Google sparse hash is highly recommended to save both time and memory, but\n" .
        	       "oculus can also use built-in standard library hashes.\n\n" .
        	       "The custom aligner option is designed to support any aligner.  The input string\n" .
        	       "to it should contain \@d, \@1, \@2, and \@o which act as placeholders for oculus.\n" .
        	       "\@d is the location of the database file, \@1 is read file 1, \@2 is read file 2,\n" .
        	       "and \@o is sam-formatted output.  Fields must be separated by single spaces.\n" . 
        	       "--customse is for single-end, and --custompe is for paired-end.\n\n" .
        	       "Here's a single-end example for bowtie:\n\n" .
        	       "./configure.pl --custom \"/mypath/bowtie-0.12.8/bowtie \@d \@1 --sam \@o\"\n\n" .
        	       "Here's a paired-end example for bowtie:\n\n" .
        	       "./configure.pl --custom \"/mypath/bowtie-0.12.8/bowtie \@d -1 \@1 -2 \@2 --sam \@o\"\n\n" .
        	       "Here's a paired-end example for bowtie2:\n\n" .
        	       "./configure.pl --custom \"/mypath/bowtie-2.0.0-beta7/bowtie2 -x \@d -1 \@1 -2 \@2 -S \@o\"\n";
}

if($#ARGV < 1)
{
  usage();
  exit(1);
}

my $gsh = "";
my $bwa = "";
my $bwt = "";
my $customse = "";
my $custompe = "";

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
  elsif($ARGV[$i] eq "--customse")
  {
    	if($i == $#ARGV){ usage(); exit(1); }
    	$i++;
    	$customse = $ARGV[$i];
  }
  elsif($ARGV[$i] eq "--custompe")
  {
    	if($i == $#ARGV){ usage(); exit(1); }
    	$i++;
    	$custompe = $ARGV[$i];
  }
  else
  {
    	usage();
    	exit(1);
  }
}

if(!$bwa && !$bwt && !$customse && !$custompe)
{
  usage();
  exit(1);
}

if($bwa && (!(-e $bwa) || !(-x $bwa) || !(-B $bwa)))
{
  print STDERR "Error - $bwa doesn't look like a valid, executable, binary. Check permissions.\n";
  exit(1);
}

if($bwt && (!(-e $bwt) || !(-x $bwt) || !(-B $bwt)))
{
  print STDERR "Error - $bwt doesn't look like a valid, executable, binary. Check permissions.\n";
  exit(1);
}

if($gsh && (!($gsh =~ m/include\/?$/g || $gsh =~ m/src\/$/g) || !(-d $gsh)))
{
  print STDERR "Error - $gsh doesn't look like the google sparse hash include/src directory.\n";
  exit(1);
}

if($custompe && (!($custompe =~ m/\@d/) ||
                 !($custompe =~ m/\@1/) ||
                 !($custompe =~ m/\@2/) ||
                 !($custompe =~ m/\@o/)))
{
  print STDERR "Error - custompe string is missing \@d, \@1, \@2, or \@o.\n";
  exit(1);
}
if($customse && (!($customse =~ m/\@d/) ||
                 !($customse =~ m/\@1/) ||
                 !($customse =~ m/\@o/)))
{
  print STDERR "Error - customse string is missing \@d, \@1, or \@o.\n";
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

// CUSTOM
// custom aligner string as described in configure.pl
#define CUSTOM_ALIGNER_SE \"$customse\"
#define CUSTOM_ALIGNER_PE \"$custompe\"
";

close COMPILEOPTS;
