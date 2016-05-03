/*--------------------------------------------------------------------------------------+
|
|     $Source: src/pt/debug.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifdef _DEBUG

void _trace(char *fmt, ...)
{
char out[1024];
	va_list body;
	va_start(body, fmt);
	vsprintf(out, fmt, body);
	va_end(body);
	OutputDebugStringA(out);
}
#endif