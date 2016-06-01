#include "PointoolsVortexAPIInternal.h"
#include <PTRMI/GUID.h>
#include <PTRMI/DataBuffer.h>


namespace PTRMI
{

void GUID::setGUID(const PTRMI::GUID &initGUID)
{
	*this = initGUID;
}


void GUID::read(DataBuffer &buffer)
{
	bool	isGenerated;

	buffer >> isGenerated;
	setGenerated(isGenerated);

#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
	if(isGenerated)
	{
		PTRMI::Array<unsigned char> arrayGUID(sizeof(::GUID), reinterpret_cast<unsigned char *>(&guid));

		buffer >> arrayGUID;
	}
#endif

}

void GUID::write(DataBuffer &buffer) const
{
#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
	bool isGenerated = getGenerated();

	buffer << isGenerated;
															// If GUID is defined
	if(isGenerated)
	{
															// Wrap GUID as an array
		PTRMI::Array<const unsigned char> arrayGUID(sizeof(::GUID), reinterpret_cast<const unsigned char *>(&guid));
															// Write GUID
		buffer << arrayGUID;
	}
#endif
}

DataSize GUID::getMaxWriteSize(void)
{
#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
															// Size is generated (bool) + GUID stored as array
	DataBuffer::DataSize maxSize = static_cast<DataBuffer::DataSize>(sizeof(bool) + PTRMI::Array<const unsigned char>::getMaxWriteSize(sizeof(::GUID)));

	return maxSize;
#else
    return 0;
#endif
}


bool GUID::operator==(const PTRMI::GUID &otherGUID) const
{
	return (getRawFirst64() == otherGUID.getRawFirst64() && getRawSecond64() == otherGUID.getRawSecond64());
}

bool GUID::operator<(const PTRMI::GUID &other) const
{
	PartialValue upper			= getRawFirst64();
	PartialValue otherUpper		= other.getRawFirst64();

															// If this upper is less than other, return true
	if(upper < otherUpper)
	{
		return true;
	}
															// If other upper is less than this, return false
	if(upper > otherUpper)
	{
		return false;
	}
															// Uppers are equal, so compare lower 64 bits
	PartialValue lower		= getRawSecond64();
	PartialValue otherLower	= other.getRawSecond64();
															// If lower is lower than other, return true
	if(lower < otherLower)
	{
		return true;
	}
															// Must be greater or equal so return false
	return false;
}



void GUID::generate(void)
{

#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
															// If not already generated
	if(getGenerated() == false)
	{
															// Generate GUID
		CoCreateGuid(&guid);
															// Flag as generated
		setGenerated(true);
	}
#endif
}


bool GUID::isValidGUID(void) const
{
	return getGenerated();
}


void GUID::clear(void)
{
	setRaw(0, 0);

	setGenerated(false);
}


GUID::GUID(void)
{
	clear();
}


void GUID::setRawFirst64(PartialValue value)
{
#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
	*(reinterpret_cast<PartialValue *>(&guid)) = value;
#endif
}


void GUID::setRawSecond64(PartialValue value)
{
#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
	*(reinterpret_cast<PartialValue *>(&((reinterpret_cast<unsigned char *>(&guid))[8]))) = value;
#endif
}


void GUID::setRaw(PartialValue firstValue, PartialValue secondValue)
{
	setRawFirst64(firstValue);
	setRawSecond64(secondValue);

	bool isGenerated = (firstValue != 0 && secondValue != 0);

	setGenerated(isGenerated);
}

GUID::PartialValue GUID::getRawFirst64(void) const
{
#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
	return *(reinterpret_cast<const PartialValue *>(&guid));
#else
return 0;
#endif
}


GUID::PartialValue GUID::getRawSecond64(void) const
{
#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
	return *(reinterpret_cast<const PartialValue *>(&((reinterpret_cast<const unsigned char *>(&guid))[8])));
#else
return 0;
#endif
}


bool GUID::getHexString(std::wstring &string) const
{
#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
	WCHAR	*	wzGuid;
	HRESULT		hr;
	
	if(FAILED(hr = StringFromCLSID(guid, &wzGuid)))
		return false;

	std::wstring gString(wzGuid);
	 
	string = gString.substr(1, gString.length() - 2);

	CoTaskMemFree(wzGuid);

#endif

	return true;
}

bool GUID::setHexString(std::wstring &string)
{
#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
	bool			result;
	std::wstring	s;
	HRESULT			r;
															// If not braced, brace temporary string
	if(string[0] != L'{')
	{
		s = L"{";

		s += string;

		s += L'}';
	}
	else
	{
		s = string;
	}

//	r = CLSIDFromString(L"{C53F4E73-C9EB-4E79-B907-EA722B4B6F46}", &guid);
//	r = CLSIDFromString(L"C53F4E73-C9EB-4E79-B907-EA722B4B6F46", &guid);

	r = CLSIDFromString(const_cast<wchar_t *>(s.c_str()), &guid);
	if(r != NOERROR)
	{
		return false;
	}

	if(isNULL())
	{
		result = false;
	}
	else
	{
		result = true;
	}

	setGenerated(result);

	return result;
#else
    return false;
#endif
}



} // End PTRMI namespace