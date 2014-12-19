#pragma once

#include "Console.h"
#include <fcntl.h> // for _O_U16TEXT
#include <io.h>  // for _setmode()

FILE* Console::m_in = stdin;
FILE* Console::m_out = stdout;
FILE* Console::m_err = stderr;
Utf8Char Console::EOL[] = "\r\n";

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void Console::SetEncoding(Encoding encoding)
    {
    switch(encoding)
        {
        case ENCODING_Text: 
            _setmode(_fileno(GetOut()), _O_TEXT);  
            _setmode(_fileno(GetError()), _O_TEXT);  
            break;
        case ENCODING_Binary: 
            _setmode(_fileno(GetOut()), _O_BINARY);  
            _setmode(_fileno(GetError()), _O_BINARY);  
            break;
        case ENCODING_Utf8: 
            _setmode(_fileno(GetOut()), _O_U8TEXT);  
            _setmode(_fileno(GetError()), _O_U8TEXT);  
            break;
        case ENCODING_Utf16: 
            _setmode(_fileno(GetOut()), _O_U16TEXT);  
            _setmode(_fileno(GetError()), _O_U16TEXT);  
            break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
 void Console::_Write (Stream stream, Utf8CP format, va_list args )
     {
     FILE* out = stream == STREAM_OUT ? m_out : m_err; 
     vfprintf(out, format, args);
     }

 //---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
 void Console::_WriteLine (Stream stream, Utf8CP format,  va_list args )
     {
     Utf8String fmt = format;
     fmt.append(Console::EOL);
     _Write(stream, fmt.c_str(), args);
   
     }
 //---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void Console::Write (Utf8CP format, ... )
    {
    va_list argptr;
    va_start(argptr, format);
    _Write(STREAM_OUT, format, argptr);
    va_end(argptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void Console::WriteLine (Utf8CP format, ... )
    {
    va_list argptr;
    va_start(argptr, format);
    _WriteLine(STREAM_OUT, format, argptr);
    va_end(argptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void Console::WriteLine ()
    {
    WriteLine("");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void Console::WriteError (Utf8CP format, ... )
    {
    va_list argptr;
    va_start(argptr, format);
    _Write(STREAM_ERROR, format, argptr);
    va_end(argptr);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void Console::WriteErrorLine (Utf8CP format, ... )
    {
    va_list argptr;
    va_start(argptr, format);
    _WriteLine(STREAM_ERROR, format, argptr);
    va_end(argptr);
    }
