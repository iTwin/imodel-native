#ifndef POINTOOLS_TYPEDEFS_HEADER
#define POINTOOLS_TYPEDEFS_HEADER

typedef unsigned char	ubyte;
typedef unsigned int	uint;
typedef unsigned short	ushort;

#ifdef _MSC_EXTENSIONS
    typedef int64_t			   int64;
    typedef uint64_t   uint64;
#else
    typedef long long		   int64;
    typedef unsigned long long uint64;
#endif

#endif
