/*--------------------------------------------------------------------------*/ 
/*	String class															*/ 
/*  (C) 2004 Copyright Faraz Ravi - All Rights Reserved						*/ 
/*																			*/ 
/*  Last Updated 11 Jan 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#pragma once
#include <wchar.h>
#include <string.h>

namespace pt
{
	class String
	{
	public:
		String();
		String(const String &s);
		explicit String(const char *);
		explicit String(const wchar_t *);
		~String();

		enum Encoding
		{
			UnknownEncoding = 0,
			AsciiEncoding = 1,
			UTF8Encoding = 2
		};
	
		bool operator >  (const String &) const;
		bool operator >= (const String &) const;
		bool operator <  (const String &) const;
		bool operator <= (const String &) const;
		bool operator == (const String &) const;
		bool operator != (const String &) const;

		void operator += (const String &);
		String operator + (const String &) const;

		inline const char* c_str() const { return const_cast<String*>(this)->encode(AsciiEncoding); }
		inline const char* c_u8str() const { return const_cast<String*>(this)->encode(UTF8Encoding); }
		inline const wchar_t* c_wstr() const { return (_wbuffer ? _wbuffer : L""); }

		inline operator const wchar_t*() const { return c_wstr(); }
		inline operator const char*() const { return c_str(); }
		inline operator bool() const { return c_wstr() ? true : false; }

		const char* getEncoded(Encoding enc, char * buffer, int buffsize) const;
		const wchar_t* getW(wchar_t *buffer, size_t buffsize) const;

		String &operator = (const String &);
		String &operator = (const char *);
		String &operator = (const wchar_t *);

		int length() const;
		
		void format(const char *str, ...);

		int compare(const wchar_t *str)	const { return wcscmp(c_wstr(), str); }  
		int compare(const char *str)	const { return strcmp(c_u8str(), str); }  

	private:		
		void allocBuffer(size_t);
		void freeBuffer();

		const char *encode(Encoding enc);
		
		wchar_t *_wbuffer;
		char	*_abuffer;
		unsigned short _wbuffsize;
		bool _abufferDirty;
		bool _wbufferDirty;
		unsigned char _encoding;
		unsigned char _res;
	};
}
