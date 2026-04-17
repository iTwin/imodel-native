When updating zlib package please note that you will need to update the 2 .h files in this dir with the new ones located in compress/zlib.
Also you will need to update iModelCore/libsrc/png/vendor/pnglibconf.h to use indicate the correct zlib version

example:
- #define PNG_ZLIB_VERNUM 0x1310
+ #define PNG_ZLIB_VERNUM 0x1320