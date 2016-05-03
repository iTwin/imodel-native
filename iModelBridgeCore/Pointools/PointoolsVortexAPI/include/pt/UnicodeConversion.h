/*  SmartWin a template based Windows API GUI Library
    Copyright (C) 2003  Thomas Hansen
    I can be reached at polterguy@gmail.com
    Project website is at http://smartwin.sourceforge.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    As a special exception, if other files call macros or inline functions
    or instantiate templates from this file, and you compile this file and
    link it with other files to produce an executable, this file does not by
    itself cause the resulting executable to be covered by the GNU General
    Public License.  This exception does not however invalidate any other
    reasons why the executable file might be covered by the GNU General
    Public License.  However, if you distribute such executables, you must
    make available the source code to the version of SmartWin which your
    executable instantiates templates from, under the terms of your choice
    of subsections (a), (b), or (c) of the GPL.
*/
#ifndef UnicodeConversion_h
#define UnicodeConversion_h


#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS      0x00000400  // do not use best fit chars
#endif

namespace pt
{
	// Generic version is NEVER implemented...
	template<class From, class To>
	class UnicodeConverter
	{
	private:
		UnicodeConverter(); // Never implemented
	};

	/// Specialized version of UnicodeConverter class for converting FROM UNICODE or wchar_t TO char (std::string)
	/** It can in many scenarios be useful to be able to convert from wchar_t to char, one good example is when you have a
	  * a filepath and you want to construct a std::fstream object.<br>
	  * Often you would for instance use a WidgetLoadFile or a WidgetSaveFile widget to retrieve a path from the user.<br>
	  * These classes returns their file paths in UNICODE format ifi UNICODE is defined.<br>
	  * But the std::fstream constructors can not take any other string types then char * types for a file path.<br>
	  * then you can use this class to convert from wchar_t string to char strings.
	  */
	template<>
	class UnicodeConverter<wchar_t, char>
	{
	public:
		/// Converts from UNICODE format to a "normal" char string...
		/** The input will be converted to a std::string object which will be returned to the caller.
		  */
		static std::string convert( const std::wstring & input )
		{
			return convert( input.c_str() );
		}

		/// Converts from UNICODE format to a "normal" char string...
		/** The input will be converted to a std::string object which will be returned to the caller.
		  */
		static std::string convert( const wchar_t * input )
		{
			std::string retVal;
			size_t strLength = wcslen(input)+sizeof(wchar_t);
			char * buffer = new char[ strLength ];
			try
			{
				BOOL defa;
				::WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, input, -1, buffer, static_cast<int>(strLength), 0, &defa );
				retVal = buffer;
				delete [] buffer;
			}
			catch( ... )
			{
				delete [] buffer;
				throw;
			}
			return retVal;
		}
	};

	/// Specialized version of UnicodeConverter class for converting FROM "normal" char strings TO UNICODE string or wchar_t string (std::wstring)
	/** Use one of the static members to convert from char strings to wchar_t strings.
	  * the return value of both is std::wstring
	  */
	template<>
	class UnicodeConverter<char, wchar_t>
	{
	public:
		/// Converts from UNICODE format to a "normal" char string...
		/** The input will be converted to a std::string object which will be returned to the caller.
		  */
		static std::wstring convert( const std::string & input )
		{
			return convert( input.c_str() );
		}

		/// Converts from UNICODE format to a "normal" char string...
		/** The input will be converted to a std::string object which will be returned to the caller.
		  */
		static std::wstring convert( const char * input )
		{
			if( 0 == input || '\0' == input[0] )
				return std::wstring();

			std::wstring retVal;
			const size_t bufferSize = strlen(input) + 1; // buffer size, need to allocate space for the '\0' char
			wchar_t * buffer = new wchar_t[ bufferSize ];
														// Null terminate the destination buffer
			buffer[bufferSize - 1] = L'\0';

			try
			{
				::MultiByteToWideChar( CP_ACP, MB_COMPOSITE , input, -1, buffer, static_cast<int>(bufferSize - 1) );
				retVal = buffer;
				delete [] buffer;
			}
			catch( ... )
			{
				delete [] buffer;
				throw;
			}
			return retVal;
		}
	};

	/// Specialized version of UnicodeConverter class for NOT converting AT ALL
	/** This probably looks stupid at first sight, but if you wish to TRULY make UNICODE/ANSII support be COMPLETELY transparent you should
	  * in fact have a version that does NOTHING.<br>
	  * The reason is to avoid having the application do custom "#ifdef" on UNICODE checks to see "if we're supposed to convert or not".<br>
	  * Fact is if you think about iy you will probably quickly see that this is one of the only ways to completely let UNICODE support be 100% transparent.
	  */
	template<>
	class UnicodeConverter<char, char>
	{
	public:
		/// Does not convert AT ALL
		/** Read the class info to understand why this is here.<br>
		  * Mark the signature is slightly different then the "converting" classes, this is to avoid unnecesary copying of the input string.
		  */
		static const std::string & convert( const std::string & input )
		{
			return input;
		}

		/// Does not convert AT ALL
		/** Read the class info to understand why this is here.<br>
		  */
		static std::string convert( const char * input )
		{
			// This one DOES carry a bit overhead, but I think it's worth it to make the UNICODE support completely transparent without bringing in ugly macros...
			return input;
		}
	};

	/// Specialized version of UnicodeConverter class for NOT converting AT ALL
	/** This probably looks stupid at first sight, but if you wish to TRULY make UNICODE/ANSII support be COMPLETELY transparent you should
	  * in fact have a version that does NOTHING.<br>
	  * The reason is to avoid having the application do custom "#ifdef" on UNICODE checks to see "if we're supposed to convert or not".<br>
	  * Fact is if you think about iy you will probably quickly see that this is one of the only ways to completely let UNICODE support be 100% transparent.
	  */
	template<>
	class UnicodeConverter<wchar_t, wchar_t>
	{
	public:
		/// Does not convert AT ALL
		/** Read the class info to understand why this is here.<br>
		  * Mark the signature is slightly different then the "converting" classes, this is to avoid unnecesary copying of the input string.
		  */
		static const std::wstring & convert( const std::wstring & input )
		{
			return input;
		}

		/// Does not convert AT ALL
		/** Read the class info to understand why this is here.<br>
		  */
		static std::wstring convert( const wchar_t * input )
		{
			// This one DOES carry a bit overhead, but I think it's worth it to make the UNICODE support completely transparent without bringing in ugly macros...
			return input;
		}
	};

	/// \ingroup GlobalStuff
	/// Typdef for easy conversion from char to wchar_t strings
	/** Be careful with using these ones since it is not generic, you shouldn't use this one unless you ALWAYS expect no matter what build you're
	  * in to get the "from" part in a char string and the to part in a wchar_t string.
	  */
	typedef UnicodeConverter<char, wchar_t> Ascii2Unicode;

	/// \ingroup GlobalStuff
	// Typdef for easy conversion from wchar_t to char strings
	/** Be careful with using these ones since it is not generic, you shouldn't use this one unless you ALWAYS expect no matter what build you're
	  * in to get the "from" part in a wchar_t string and the to part in a char string.
	  */
	typedef UnicodeConverter<wchar_t, char> Unicode2Ascii;

	/// \ingroup GlobalStuff
	// Typdef for easy conversion from wchar_t to char strings
	/** This is the more "safe" version of the typedefs if you wish to GUARANTEE that the OUTPUT is of the current "build version".<br>
	  * If UNICODE is defined you will NOT get any conversion at all, if UNICODE is NOT defined you WILL get a conversion to a char string.<br>
	  * This and the Ascii2UnicodeOrMaybeNot is the most "safe" ones if you wish to make UNICODE support transparent.
	  */
	typedef UnicodeConverter<wchar_t, TCHAR> Unicode2CurrentBuild;

	/// \ingroup GlobalStuff
	// Typdef for easy conversion from char to wchar_t strings
	/** This is the more "safe" version of the typedefs if you wish to GUARANTEE that the OUTPUT is of the current "build version".<br>
	  * If UNICODE is defined you WILL get a conversion to a wchar_t string, if UNICODE is NOT defined you WILL NOT get a conversion at all.
	  * This and the Unicode2AsciiOrMaybeNot is the most "safe" ones if you wish to make UNICODE support transparent.
	  */
	typedef UnicodeConverter<char, TCHAR> Ascii2CurrentBuild;

	/// \ingroup GlobalStuff
	// Typdef for easy conversion from wchar_t to char strings
	/** This is the more "safe" version of the typedefs if you wish to GUARANTEE that the OUTPUT is of the current "build version".<br>
	  * If UNICODE is defined you will NOT get any conversion at all, if UNICODE is NOT defined you WILL get a conversion to a char string.<br>
	  * This and the Ascii2UnicodeOrMaybeNot is the most "safe" ones if you wish to make UNICODE support transparent.
	  */
	typedef UnicodeConverter<TCHAR, char> AsciiGuaranteed;

	/// \ingroup GlobalStuff
	// Typdef for easy conversion from char to wchar_t strings
	/** This is the more "safe" version of the typedefs if you wish to GUARANTEE that the OUTPUT is of the current "build version".<br>
	  * If UNICODE is defined you WILL get a conversion to a wchar_t string, if UNICODE is NOT defined you WILL NOT get a conversion at all.
	  * This and the Unicode2AsciiOrMaybeNot is the most "safe" ones if you wish to make UNICODE support transparent.
	  */
	typedef UnicodeConverter<TCHAR, wchar_t> UnicodeGuaranteed;
}

#endif


