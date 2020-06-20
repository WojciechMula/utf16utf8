Conversion from UTF16 into UTF8
==================================

* UTF16 - https://tools.ietf.org/html/rfc2781
* UTF8 - https://tools.ietf.org/html/rfc3629


To do
------
* Review the paper Cameron, Robert D. "A case study in SIMD text processing with parallel bit streams: UTF-8 to UTF-16 transcoding." Proceedings of the 13th ACM SIGPLAN Symposium on Principles and practice of parallel programming. 2008.
* We should be concerned with the branch predictor outsmarting us: we need to test on sizeable inputs and see if results hold up.
* Compare with https://woboq.com/blog/utf-8-processing-using-simd.html
* [Accelerating UTF-8 Decoding Using SIMD Instructions](https://researcher.watson.ibm.com/researcher/files/jp-INOUEHRS/IPSJPRO2008_SIMDdecoding.pdf) by Hiroshi Inoue
* [utf8lut: Vectorized UTF-8 converter](https://dirtyhandscoding.github.io/posts/utf8lut-vectorized-utf-8-converter-introduction.html) (does UTF-8 <=> UTF-16) 
* [CppCon 2018: Bob Steagall “Fast Conversion From UTF-8 with C++, DFAs, and SSE Intrinsics”](https://www.youtube.com/watch?v=5FQ87-Ecb-A) with [GitHub](https://github.com/CppCon/CppCon2018/tree/master/Presentations/fast_conversion_from_utf8_with_cpp_dfas_and_sse_intrinsics)
* Benchmark against iconv
* Up to C++14, the standard library provided means to do unicode conversion

* UTF-16 decoder should take BOM
* UTF-8 decoder should be able to generate a BOM and support BE/LE
* (optional) Error Handling
* Port to NEON, AVX2, AVX-512


Link
----

* SQL Server team says that UTF16<==>UTF8 is costly https://cloudblogs.microsoft.com/sqlserver/2018/12/18/introducing-utf-8-support-in-sql-server-2019-preview/
