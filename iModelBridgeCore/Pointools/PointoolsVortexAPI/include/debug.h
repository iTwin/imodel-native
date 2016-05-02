


// file debug.h
#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef NEEDS_WORK_VORTEX_DGNDB
#ifdef _DEBUG
void _trace(char *fmt, ...);
#define ASSERT(x) {if(!(x)) _asm{int 0x03}}
#define VERIFY(x) {if(!(x)) _asm{int 0x03}}
#else
#define ASSERT(x)
#define VERIFY(x) x
#endif
#ifdef _DEBUG
#define TRACE _trace
#else
inline void _trace(LPCSTR fmt, ...) { }
#define TRACE  1 ? (void)0 : _trace
#endif

#endif
#endif // __DEBUG_H__

