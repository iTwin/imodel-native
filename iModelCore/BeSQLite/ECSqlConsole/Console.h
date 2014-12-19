#pragma once
#include <bentley/BeFilename.h>
#include <Logging/bentleylogging.h>


struct Console
    {
    enum Encoding
        {
        ENCODING_Text,
        ENCODING_Binary,
        ENCODING_Utf8,
        ENCODING_Utf16
        };
private:
  enum Stream 
    {
    STREAM_OUT,
    STREAM_ERROR,
    STREAM_IN
    }; 
    static FILE* m_in ;
    static FILE* m_out ;
    static FILE* m_err ;
    static Utf8Char EOL[];
    static void _Write (Stream stream, Utf8CP format,  va_list args );
    static void _WriteLine (Stream stream, Utf8CP format,  va_list args );

public:
    static void SetEncoding(Encoding encoding);
    static FILE* GetOut() { return m_out; }
    static FILE* GetError() { return m_err; }
    static FILE* GetIn() { return m_in; }

    static void SetOut(FILE* out) {  m_out = out; }
    static void SetError(FILE* err) {  m_err = err; }
    static void SetIn(FILE* in) {  m_in = in; }

    static void Write (Utf8CP format, ... );
    static void WriteLine (Utf8CP format, ... );
    static void WriteLine ();
    static void WriteError (Utf8CP format, ... );
    static void WriteErrorLine (Utf8CP format, ... );
    };