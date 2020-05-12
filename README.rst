Conversion from UTF16 into UTF8
==================================

* UTF16 - https://tools.ietf.org/html/rfc2781
* UTF8 - https://tools.ietf.org/html/rfc3629


To do
------
* Benchmark against iconv
* Up to C++14, the standard library provided means to do unicode conversion

* UTF-16 decoder should take BOM
* UTF-8 decoder should be able to generate a BOM and support BE/LE
* (optional) Error Handling
