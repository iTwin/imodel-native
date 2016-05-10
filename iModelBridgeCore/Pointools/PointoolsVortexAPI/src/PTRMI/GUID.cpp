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

	if(isGenerated)
	{
		PTRMI::Array<unsigned char> arrayGUID(sizeof(::GUID), reinterpret_cast<unsigned char *>(&guid));

		buffer >> arrayGUID;
	}

}

void GUID::write(DataBuffer &buffer) const
{
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
}

DataSize GUID::getMaxWriteSize(void)
{
															// Size is generated (bool) + GUID stored as array
	DataBuffer::DataSize maxSize = static_cast<DataBuffer::DataSize>(sizeof(bool) + PTRMI::Array<const unsigned char>::getMaxWriteSize(sizeof(::GUID)));

	return maxSize;
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
															// If not already generated
	if(getGenerated() == false)
	{
															// Generate GUID
		CoCreateGuid(&guid);
															// Flag as generated
		setGenerated(true);
	}
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
	*(reinterpret_cast<PartialValue *>(&guid)) = value;
}


void GUID::setRawSecond64(PartialValue value)
{
	*(reinterpret_cast<PartialValue *>(&((reinterpret_cast<unsigned char *>(&guid))[8]))) = value;
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
	return *(reinterpret_cast<const PartialValue *>(&guid));
}


GUID::PartialValue GUID::getRawSecond64(void) const
{
	return *(reinterpret_cast<const PartialValue *>(&((reinterpret_cast<const unsigned char *>(&guid))[8])));
}


bool GUID::getHexString(std::wstring &string) const
{
	WCHAR	*	wzGuid;
	HRESULT		hr;
	
	if(FAILED(hr = StringFromCLSID(guid, &wzGuid)))
		return false;

	std::wstring gString(wzGuid);
	 
	string = gString.substr(1, gString.length() - 2);

	CoTaskMemFree(wzGuid);

	return true;
}

bool GUID::setHexString(std::wstring &string)
{
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
}



} // End PTRMI namespace