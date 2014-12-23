
/*! \file
  Functions to manipulate UTF-8 strings and convert from/to legacy
  encodings.
*/

#ifndef pt_uft_funcs
#define pt_uft_funcs

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int			utf8bytes(unsigned ucs);
unsigned	utf8decode(const char*, const char* end, int* len);
int			utf8encode(unsigned, char*);
const char* utf8fwd(const char*, const char* start, const char* end);
const char* utf8back(const char*, const char* start, const char* end);
unsigned	utf8towc(const char*, unsigned, wchar_t*, unsigned);
unsigned	utf8tomb(const char*, unsigned, char*, unsigned);
unsigned	utf8toa (const char*, unsigned, char*, unsigned);
unsigned	utf8fromwc(char*, unsigned, const wchar_t*, unsigned);
unsigned	utf8frommb(char*, unsigned, const char*, unsigned);
unsigned	utf8froma (char*, unsigned, const char*, unsigned);
int			utf8locale();
int			utf8test(const char*, unsigned);

#ifdef __cplusplus
}
#endif

#endif
