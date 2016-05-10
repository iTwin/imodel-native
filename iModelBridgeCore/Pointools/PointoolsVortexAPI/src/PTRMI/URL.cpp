#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/URL.h>
#include <PTRMI/DataBuffer.h>

#ifndef NO_DATA_SOURCE_SERVER
//#include <PTRMI/Manager.h>
#endif
namespace PTRMI
{

const wchar_t	URL::PT_PTFL[]		= L"PTFL";			// Protocol Local File
const wchar_t	URL::PT_PTRI[]		= L"PTRI";			// Protocol TCP/IP
const wchar_t	URL::PT_PTRE[]		= L"PTRE";			// Protocol External
const wchar_t	URL::PT_PTCI[]		= L"PTCI";			// Protocol Cached TCP/IP
const wchar_t	URL::PT_PTCE[]		= L"PTCE";			// Protocol Cached External (MS/PW)
const wchar_t	URL::PT_PTSS[]		= L"PTSS";			// Protocol Structured Storage (iModel)
const wchar_t	URL::PT_PTCF[]		= L"PTCF";			// Local file (cache)
const wchar_t	URL::PT_PTMY[]		= L"PTMY";			// Protocol for memory based data source

URL::URL(void)
{

}

URL::URL(const std::wstring &str)
{
	set(str);
}

URL::URL(const wchar_t *str)
{
	set(str);
}

// URL::~URL(void)
// {
// 
// }

void URL::set(const std::wstring &str)
{
	url = str;
}

void URL::set(const wchar_t *str)
{
	url = str;
}


void URL::setURL(const URL &initURL)
{
	url = initURL.getString();
}


const std::wstring & URL::getString(void) const
{
	return url;
}


void URL::getString(std::string &result)
{
	char buffer[URL_MAX_LENGTH];

	size_t convertedChars	= 0;
	size_t originalSize		= getLength() + 1;

	wcstombs_s(&convertedChars, buffer, originalSize, url.c_str(), _TRUNCATE);

	result = buffer;	
}

bool URL::isValidURL(void) const
{
	HostAddressIP4	hostAddressIP4;

	if(isProtocol(PT_PTRE) || isProtocol(PT_PTRI) || isProtocol(PT_PTCE) || isProtocol(PT_PTCI) || isProtocol(PT_PTFL) || isProtocol(PT_PTMY))
	{
		if(getHostAddress(hostAddressIP4) == false)
			return false;

		return true;
	}

	return false;
}

unsigned int URL::getLength(void) const
{
	return static_cast<unsigned int>(url.length());
}


bool URL::isEmpty(void) const
{
	return (getLength() == 0);
}


bool URL::getProtocol(URL &protocol, size_t *readPos) const
{
	size_t pos; 

	if((pos = getString().find(L"://")) == std::wstring::npos)
		return false;

	protocol = getString().substr(0, pos);

	if(readPos)
	{
		(*readPos) = pos + 3;
	}

	return protocol.getLength() > 0;
}


bool URL::getHostAddress(HostAddressIP4 &hostAddress, size_t *readPos) const
{
	return hostAddress.getFromURL(getString().c_str(), readPos);
}


bool URL::getObject(URL &object) const
{
	size_t			pos;
	HostAddressIP4	hostAddressIP4;

	std::wstring r;

	if(getHostAddress(hostAddressIP4, &pos))
	{
		r = getString().substr(pos, getLength() - pos);
	}
	else
	{
															// No protocol or address, so remainder is object
		r = getString();
	}

	if(r.length() > 0)
	{
		object = r;
		return true;
	}

	return false;
}


void URL::read(DataBuffer &buffer)
{
	buffer >> url;
}

void URL::write(DataBuffer &buffer) const
{
															// Write url string
	buffer << getString();
}


PTRMI::DataSize URL::getMaxWriteSize(DataSize maxChars)
{
															// Write max of underlying string array
	return PTRMI::Array<const wchar_t>::getMaxWriteSize(maxChars);
}

bool URL::getProtocolHostAddress(URL &protocolHostAddress) const
{
	URL					protocol;
	HostAddressIP4		hostAddress;

	getProtocol(protocol);
															// If protocol is specified
	if(isProtocol(URL::PT_PTFL))
	{
															// If subsequent character is ':' assume it's a full file path
		if(getString()[8] == ':')
		{
																// Get drive letter and colon
			protocolHostAddress = protocol + L"://" + getString().substr(7, 2);
		}
		else
		{
															// Assume it's a relative file path so use .\ to indicate current drive
			protocolHostAddress = L".\\";
		}
	}
	else
	{
															// Get the host address
		getHostAddress(hostAddress);

		protocolHostAddress = protocol + L"://" + hostAddress.getURL();
	}

															// Return OK
	return true;
}

#ifndef NO_DATA_SOURCE_SERVER

bool URL::getRemoteManagerName(URL &remoteManagerName) const
{
#if defined (BENTLEY_WIN32)  // NEEDS_WORK_VORTEX_DGNDB
	URL protocolHostAddress;

	if(getProtocolHostAddress(protocolHostAddress) == false)
		return false;

	remoteManagerName = protocolHostAddress + L"/" + MANAGER_OBJECT_NAME;

	return true;
#else
    return false;
#endif

}
#endif

URL &URL::operator=(const URL &n)
{
	return copy(n);
}

URL &URL::operator+=(const URL &n)
{
	url = url + n.getString();

	return *this;
}

URL &URL::operator+=(const std::wstring &n)
{
	url = url + n;

	return *this;
}

URL &URL::operator+=(const wchar_t *n)
{
	url = url + n;

	return *this;
}

URL URL::operator+(const URL &other) const
{
	URL	result;

	result = url;
	result += other;

	return result;
}

URL URL::operator+(const std::wstring &str) const
{
	URL result;

	result = url;
	result += str;
	
	return result;
}


URL URL::operator+(const wchar_t *str) const
{
	URL result;

	result = url;
	result += str;

	return result;
}


bool URL::operator==(const URL &other) const
{
	return getString() == other.getString();
}

URL &URL::copy(const URL &n)
{
	set(n.getString());

	return *this;
}

bool URL::operator<(const URL &other) const
{
	return getString() < other.getString();
}


unsigned int URL::split(URL &protocol, URL &hostAddress, URL &object, bool *gotProtocol, bool *gotHostAddress, bool *gotObject)
{
	HostAddressIP4	address;
	unsigned int	numItems = 0;

	bool			gProtocol		= false;
	bool			gHostAddress	= false;
	bool			gObject			= false;

	gProtocol		= getProtocol(protocol);
	gHostAddress	= getHostAddress(address);
	gObject			= getObject(object);

	hostAddress		= address.getURL();

	if(gProtocol)
		++numItems;

	if(gHostAddress)
		++numItems;

	if(gObject)
		++numItems;

	if(gotProtocol)
		*gotProtocol = gProtocol;

	if(gotHostAddress)
		*gotHostAddress = gHostAddress;

	if(gotObject)
		*gotObject = gObject;

	return numItems;
}

bool URL::setProtocol(URL &newProtocol)
{
	PTRMI::URL	protocol, hostAddress, object, newURL;

	split(protocol, hostAddress, object);

	newURL = newProtocol + URL(L"://") + hostAddress;

	if(object.isEmpty() == false)
	{
		newURL =  newURL + L"/" + object;
	}

	*this = newURL;

	return true;
}


bool URL::isProtocol(const wchar_t *protocolString) const
{
	URL	protocol;
	URL	testProtocol(protocolString);
															// Get the protocol string
	if(getProtocol(protocol))
	{
		protocol.toUpper();
		testProtocol.toUpper();
															// Compare with string given
		return (protocol == testProtocol);
	}

	return false;
}


bool URL::isProtocolNetworked(void) const
{
															// PTRE (Remote External) is networked
	if(isProtocol(PT_PTRE))
		return true;
															// PTRE (Remote TCP/IP) is networked
	if(isProtocol(PT_PTRI))
		return true;
															// PTCE (Cached External) is networked
	if(isProtocol(PT_PTCE))
		return true;
															// PTCI (Cached TCP/IP) is networked
	if(isProtocol(PT_PTCI))
		return true;
															// Protocol is not a remote networked type
	return false;
}


void URL::toUpper(void)
{
	std::transform(url.begin(), url.end(), url.begin(), ::toupper);
}


void URL::toLower(void)
{
	std::transform(url.begin(), url.end(), url.begin(), ::tolower);
}




} // End PTRMI namespace
