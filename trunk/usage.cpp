#include "usage.h"

void usage(int ret_code)
{
  FILE* handle;
  (ret_code == 0) ? handle = stdout : handle = stderr;

  fprintf(handle,
	  "Oculus version 0.1.1\n"
	  " Aligner wrapper that improves speed by\n"
	  " aligning only a unique set of input reads.\n\n"
	  "Usage\n\n"
	  " oculus -d <db> -1 <m1> {-2 <m2>} {-o <out>} {options}*\n\n"
	  " db  : bwa or bowtie indexed database\n"
	  " m1  : input sequence (reads) file 1 in fastA or fastQ format\n"
	  " m2  : input sequence (reads) file 2 in fastA or fastQ format (optional)\n"
	  " out : prefix used for intermediate and final results output (default: \"output\")\n\n"
	  "Options\n\n"
	  " Logistical\n\n"
	  "  -h/--h/-help/--help      : print this usage message\n"
	  "  -s/--silent              : don't print anything to stdout\n"
	  "  -a /--args  '<arg1> ...' : arguments to bowtie or custom aligner\n"
	  "  -a1/--args1 '<arg1> ...' : arguments to bwa aln for the first read file\n"
	  "  -a2/--args2 '<arg1> ...' : arguments to bwa aln for the second read file\n"
	  "  -a3/--args3 '<arg1> ...' : arguments to bwa samse/sampe\n\n"
	  " Functional\n\n"
	  "  --bwa      : use BWA instead of Bowtie\n"
      "  --custom   : use custom aligner (see configure.pl) instead of Bowtie\n"
	  "  -4/--base4 : force base 4 (2-bit) encoding for compressed nucleotides\n"
	  "  --set      : use a set in addition to a map when storing reads\n"
	  "  --rc       : store reverse complements (se),\n"
	  "               or reverse read order (pe), together in memory\n"
	  "  --ffq      : force output compressed sequence as fastQ instead of fastA\n\n"
	  " For a more complete description of the functional arguments,\n"
	  " their costs, and benefits, please view the README file.\n\n"
	  "Examples\n\n"
	  " ./oculus -d hg19 -1 reads1.fq -2 reads2.fq -a '-m 1 --quiet' -4 --set --rc\n\n"
	  "  Run a paired-end Bowtie alignment against hg19 with three arguments,\n"
	  "  and enable space saving features in oculus.\n\n"
	  " ./oculus -d hg19 -1 reads1.fq --bwa -a1 '-l 100' -a2 '-l 100' --ffq\n\n"
	  "  Run a single-end BWA alignment against hg19, passing two arguments\n"
	  "  into both alignment steps, and forcing a fastQ intermediate file.\n\n"
	  );
  
  exit(ret_code);
}
