Implement Huffman compression algorithm. Two command line utilities:

1. 'encode' -- encodes any input file using Huffman algorithm:

> encode <input-file> <output-file>

2. 'decode' -- decodes input file to the original one:

> decode <input-file> <output-file>

Requirements:

* Pure C or C++ (up to C++14). No any extensions, only standard language
  features. No assembly code.
* Makefile as a build system. Don't use CMake! Executing 'make' should
  build 'encode' and 'decode' binaries.
* Only standard libraries included with the compiler, i.e. C standard library or
  C++ standard library (STL). No Boost! No Qt!
* Encoding and decoding should work as fast as possible.
 Binaries should not crash (e.g. Segmentation Fault) on any* inputs.э