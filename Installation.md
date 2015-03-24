### 1. Download and Install an Aligner ###
Oculus is currently configured to support BWA and Bowtie, which are available here:

  * http://bio-bwa.sourceforge.net
  * http://bowtie-bio.sourceforge.net

Oculus 0.1.1 was tested with bowtie version 0.12.7, and BWA version 0.5.9-`r16`.

### 2. Download and Install Google sparsehash (Optional) ###
Oculus can compile with either standard library maps, or Google sparsehash maps, which are faster and use significantly less memory.

  * http://code.google.com/p/google-sparsehash

Oculus 0.1.1 was tested with `google-sparsehash` version 1.10.

### 3. Download and Unzip Oculus ###
  1. Download Oculus [here](http://code.google.com/p/oculus-bio/downloads/detail?name=oculus.tar.gz)
  1. unzip Oculus: `tar -xzf oculus.tar.gz`
  1. enter the Oculus directory: `cd oculus`

Alternatively, you can type `"svn checkout https://oculus-bio.googlecode.com/svn/trunk/ oculus"` to get the most recent version.

### 4. Configure Oculus ###
Run configure.pl, and supply **full** filepaths (i.e., starting with /) to
  * BWA and/or Bowtie's executables
  * Google Sparse Hash's `include` directory (in 2.0.2, this is instead `[sparsehash home]/src`)

```
perl configure.pl --bwa [bwa executable] --bowtie [bowtie executable] --g [Google Sparse Hash's include directory]
```

configure.pl will generate a makefile and a C header file, indicating paths to your installed libraries.  perl is available [here](http://www.perl.org/get.html)

### 5. Run 'make' ###

```
make
```