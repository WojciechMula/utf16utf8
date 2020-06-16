Conversion from UTF16 into UTF8
==================================

* UTF16 - https://tools.ietf.org/html/rfc2781
* UTF8 - https://tools.ietf.org/html/rfc3629


To do
------
* Review the paper Cameron, Robert D. "A case study in SIMD text processing with parallel bit streams: UTF-8 to UTF-16 transcoding." Proceedings of the 13th ACM SIGPLAN Symposium on Principles and practice of parallel programming. 2008.
* We should be concerned with the branch predictor outsmarting us: we need to test on sizeable inputs and see if results hold up.
* Compare with https://woboq.com/blog/utf-8-processing-using-simd.html
* Benchmark against iconv
* Up to C++14, the standard library provided means to do unicode conversion

* UTF-16 decoder should take BOM
* UTF-8 decoder should be able to generate a BOM and support BE/LE
* (optional) Error Handling
* Port to NEON, AVX2, AVX-512
