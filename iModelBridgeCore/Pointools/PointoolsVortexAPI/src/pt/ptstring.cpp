#include "PointoolsVortexAPIInternal.h"

#include <string.h>
#include <pt/utf.h>

#include <assert.h>
#include <iomanip>

#include <pt/ptstring.h>

using namespace pt;

#define MIN_CHUNK		8
#define INITIAL_CHUNK	32
#define INITIALISE_STRING _wbuffer(0), _abuffer(0), _wbuffsize(0), _abufferDirty(true), _encoding(UnknownEncoding) 
#define MAX_STRING_LENGTH 4096

//
// Constructors
//
String::String() : INITIALISE_STRING {}

String::String(const String &s) :INITIALISE_STRING
{
	*this = s;
}
String::String(const char *s) : INITIALISE_STRING
{
	if (s && s[0] != L'\0')
	{
		(*this) = s;
	}
}
String::String(const wchar_t *s) : INITIALISE_STRING
{
	if (s && s[0] != L'\0')
		(*this) = s;
} 
String::~String()
{
	try
	{
	freeBuffer();
	}
	catch(...) {}
}
//
// comparision
//
bool String::operator >  (const String &s) const	{	return lstrcmpW(_wbuffer, s.c_wstr()) > 0;	}
bool String::operator >= (const String &s) const	{	return lstrcmpW(_wbuffer, s.c_wstr()) >= 0;	}
bool String::operator <  (const String &s) const	{	return lstrcmpW(_wbuffer, s.c_wstr()) < 0;	}
bool String::operator <= (const String &s) const	{	return lstrcmpW(_wbuffer, s.c_wstr()) <= 0;	}
bool String::operator == (const String &s) const	{	return lstrcmpW(_wbuffer, s.c_wstr()) == 0;	}
bool String::operator != (const String &s) const	{	return lstrcmpW(_wbuffer, s.c_wstr()) != 0;	}


int String::length() const
{
	if (!_wbuffer) return 0;
	return static_cast<int>(wcsnlen(_wbuffer, _wbuffsize));
}
//
// Assignment operator
//
String &String::operator = (const String &s)
{
	if (this == &s) return *this;

	int len = s.length();
	if (!len)
	{
		if (_wbuffer) _wbuffer[0] = L'\0';
		if (_abuffer)
		{
			delete [] _abuffer;
			_abuffer = 0;
			_abufferDirty = false;
		}
		return *this;
	}
	else
	{		
		len++;
		allocBuffer(len);
		wcscpy(_wbuffer, s.c_wstr());

		_abufferDirty = s._abufferDirty;

		if (_abuffer) delete [] _abuffer;

		if (!s._abufferDirty && s._abuffer)
		{
			size_t size = strlen(s._abuffer) + 1;
			_abuffer = new char[size];
			strcpy_s(_abuffer, size, s._abuffer);
			_encoding = s._encoding;
		}
		else
		{
			_abuffer = 0;
		}
	}
	return *this;
}
//
// Assignment operator
//
String &String::operator = (const char *s)
{
	if (_abuffer)
	{
		delete [] _abuffer;
		_abuffer = 0;
	}
	if (s && s[0] != '\0')
	{
		int len = static_cast<int>(strlen(s)+1);
		allocBuffer(len);
		
		utf8towc(s, (unsigned)len, _wbuffer, _wbuffsize);
		
		_abufferDirty = false;
		_abuffer = new char[len];
		memcpy(_abuffer, s, len);
		_abuffer[len-1] = '\0';
	}
	else 
	{
		if (_wbuffer) _wbuffer[0] = '\0';
		_abufferDirty = false;
	}
	return *this;
}
//
// Assignment operator
//
String &String::operator = (const wchar_t *s)
{
	if (s && s[0] != L'\0')
	{
		size_t len = wcslen(s)+1;	// +1 for terminator
		allocBuffer(len);

		memcpy(_wbuffer, s, sizeof(wchar_t) * len);
	}
	else
	{
		if (_wbuffer)
		{
			_wbuffer[0] = '\0';
		}
		if (_abuffer)
		{
			delete [] _abuffer;
			_abuffer = 0;
			_abufferDirty = false;
		}
	}
	return *this;
}
//
// += operator
//
void String::operator += (const String &s)
{
        int len = s.length();
        int tlen = length();
	
	String st;
	st.allocBuffer(len+tlen);
	memcpy(st._wbuffer, _wbuffer, sizeof(wchar_t)*tlen);
	memcpy(&st._wbuffer[tlen], s._wbuffer, sizeof(wchar_t)*(len+1));

	_abufferDirty = true;

	*this = st;
}
//
// + operator
//
String String::operator + (const String &s) const
{
	int len = s.length();
	int tlen = length();
	
	String st;
	st.allocBuffer(len+tlen);
	memcpy(st._wbuffer, _wbuffer, sizeof(wchar_t)*tlen);
	memcpy(&st._wbuffer[tlen], s._wbuffer, sizeof(wchar_t)*(len+1));

	return st;
}
//
// get the Wchar string
//
const wchar_t *String::getW(wchar_t *buffer, size_t buffsize) const
{
	size_t chars=length();
	if (buffsize < chars) chars = buffsize;
	memcpy(buffer, _wbuffer, (chars+1)*sizeof(wchar_t));
	return buffer;
}
//
// get a utf8 string
//
const char* String::getEncoded(Encoding enc, char * buffer, int buffsize) const
{
	int chars=length();

	memset(buffer, '\0', buffsize);

	/* check for string truncation */ 
	assert(buffsize >= chars);
	
	if (buffsize < chars) chars = buffsize;
	
	if (_abuffer && !_abufferDirty && _encoding == enc)
	{
		assert (strnlen(_abuffer, MAX_STRING_LENGTH) <= (size_t)buffsize);
		strcpy(buffer, _abuffer);
	}
	else
	{
		switch(enc)
		{
		case AsciiEncoding:
			try		{
				BOOL defa;
                                if (!::WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, _wbuffer, 
					static_cast<int>(chars), buffer, buffsize, 0, &defa )) throw;		
			}
			catch( ... ) {
				assert(0);
			}
			break;
		case UTF8Encoding:
			utf8fromwc(buffer, buffsize, _wbuffer, (unsigned)chars);
			break;
		default:
			assert(0);
		}
	}
	return buffer;
} 
//
// buffer allocation
//
void String::allocBuffer(size_t size)
{
	/*round size to chunk boundary*/ 
	size_t newsize = ((size / MIN_CHUNK) + 1) * MIN_CHUNK;
	if (newsize > _wbuffsize)
	{
		freeBuffer();
		try
		{
		_wbuffer = new wchar_t[newsize];
		_wbuffsize = (unsigned short)newsize;
		}
		catch(...) 
		{
			_wbuffsize = 0;
			_wbuffer = 0;
		}
	}
}
//
//free the buffer
//
void String::freeBuffer()
{
	if (_wbuffer) delete [] _wbuffer;
	if (_abuffer) delete [] _abuffer;
	_wbuffsize = 0;
	_abuffer = 0;
	_wbuffer = 0;
}
//
//format string
//
void String::format(const char *str, ...)
{
	va_list	ap;
	char s[4096];

	assert(strlen(str) < 1024);

	va_start(ap, str);
	vsprintf((char*)s, str, ap);
	va_end(ap);
	
	(*this) = s;
}
//
// convert string to ascii
//
const char* String::encode(Encoding enc)
{
	if (!_wbuffer || _wbuffer[0] == L'\0') return "";
	if (!_abufferDirty && _abuffer && _encoding == enc) return _abuffer;

	if (_abuffer) delete [] _abuffer;
	
	int abuffsize = _wbuffsize * sizeof(wchar_t);
	_abuffer = new char[abuffsize];
	getEncoded(enc, _abuffer, abuffsize);
	
	_encoding = (unsigned char)enc;
	_abufferDirty = false;
	return _abuffer;
}