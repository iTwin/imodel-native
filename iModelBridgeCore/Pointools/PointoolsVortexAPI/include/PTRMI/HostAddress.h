
#pragma once

#include <string>
#include <stdio.h>

#include <PTRMI/URL.h>

#define HOST_ADDRESS_T			template<typename IPSubValue, unsigned int numDigits>
#define	DEFAULT_SUBVALUE		0


namespace PTRMI
{

	template<typename IPSubValue, unsigned int numDigits> class HostAddress
	{
	public:

		typedef unsigned short	Index;

		typedef IPSubValue		SubValue;

		typedef unsigned int	Port;

	protected:

		std::wstring	url;
		IPSubValue		ip[numDigits];

		bool			ipDefined;

		Port			port;

	protected:

		void			setIPDefined			(bool value);

		bool			getStringFromDigits		(std::wstring &string);

		bool			getPortNumberFromString	(const std::wstring & string, size_t* start);

		bool			updateURL				(void);

	public:
						HostAddress				(void);
						HostAddress				(const wchar_t *initURL);
						HostAddress				(const IPSubValue *values);
						HostAddress				(const wchar_t *initURL, const IPSubValue *values);

		void			clear					(void);

		void			setPort					(unsigned int initPort);
		Port			getPort					(void);

		void			setURL					(const wchar_t *initURL);
		void			setURL					(const std::wstring &initURL);
		const wchar_t *	getURL					(void);
		bool			getURL					(std::wstring &URL, bool includePort);

		void			setSubValue				(Index index, IPSubValue value);
		IPSubValue		getSubValue				(Index index) const;

		bool			setSubValues			(const unsigned int values[numDigits]);
		bool			setSubValues			(const IPSubValue values[numDigits]);

#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
		bool			setAddress				(const sockaddr_in &addr);
		bool			getAddress				(sockaddr_in &result) const;
#endif

		bool			isValidIndex			(Index index) const;
		bool			getIPDefined			(void) const;

		bool			getFromURL				(const std::wstring &string, size_t *readPos = NULL);

		bool			getDigitsFromString		(const std::wstring &string, size_t *readPos = NULL);
	};



	typedef HostAddress<unsigned char, 4>	HostAddressIP4;
	typedef HostAddress<unsigned short, 6>	HostAddressIP6;

#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER

	HOST_ADDRESS_T inline
	bool PTRMI::HostAddress<IPSubValue, numDigits>::setAddress(const sockaddr_in &addr)
	{
		setSubValue(0, addr.sin_addr.S_un.S_un_b.s_b1);
		setSubValue(1, addr.sin_addr.S_un.S_un_b.s_b2);
		setSubValue(2, addr.sin_addr.S_un.S_un_b.s_b3);
		setSubValue(3, addr.sin_addr.S_un.S_un_b.s_b4);

		setIPDefined(true);

		updateURL();

		return true;
	}

	HOST_ADDRESS_T inline
	bool PTRMI::HostAddress<IPSubValue, numDigits>::getAddress(sockaddr_in &result) const
	{
		result.sin_addr.S_un.S_un_b.s_b1 = getSubValue(0);
		result.sin_addr.S_un.S_un_b.s_b2 = getSubValue(1);
		result.sin_addr.S_un.S_un_b.s_b3 = getSubValue(2);
		result.sin_addr.S_un.S_un_b.s_b4 = getSubValue(3);

		return true;
	}
#endif

	HOST_ADDRESS_T inline
	void HostAddress<IPSubValue, numDigits>::setPort(Port initPort)
	{
		port = initPort;
	}

	HOST_ADDRESS_T inline
	typename PTRMI::HostAddress<IPSubValue, numDigits>::Port HostAddress<IPSubValue, numDigits>::getPort(void)
	{
		return port;
	}

	HOST_ADDRESS_T inline
	void HostAddress<IPSubValue, numDigits>::setIPDefined(bool value)
	{
		ipDefined = value;
	}

	HOST_ADDRESS_T inline
	bool HostAddress<IPSubValue, numDigits>::getIPDefined(void) const
	{
		return ipDefined;
	}


	HOST_ADDRESS_T inline
	HostAddress<IPSubValue, numDigits>::HostAddress(void)
	{
		clear();
	}

	HOST_ADDRESS_T inline
	HostAddress<IPSubValue, numDigits>::HostAddress(const wchar_t *initURL)
	{
		setURL(initURL);
	}

	HOST_ADDRESS_T inline
	HostAddress<IPSubValue, numDigits>::HostAddress(const IPSubValue *values)
	{
		setSubValues(values);
	}


	HOST_ADDRESS_T inline
	void HostAddress<IPSubValue, numDigits>::setURL(const wchar_t *initURL)
	{
		url = initURL;
	}

	HOST_ADDRESS_T inline
	void HostAddress<IPSubValue, numDigits>::setURL(const std::wstring &initURL)
	{
		url = initURL;
	}


	HOST_ADDRESS_T inline
	const wchar_t *HostAddress<IPSubValue, numDigits>::getURL(void)
	{
		return url.c_str();
	}

	HOST_ADDRESS_T inline
	bool HostAddress<IPSubValue, numDigits>::getURL(std::wstring &resultURL, bool includePort)
	{
		unsigned int p = url.find_first_of(L":");

		if(includePort || p == std::wstring::npos)
		{
			resultURL = getURL();
			return true;
		}

		resultURL = url.substr(0, p);

		return resultURL.length() > 0;
	}


	HOST_ADDRESS_T inline
	void HostAddress<IPSubValue, numDigits>::setSubValue(Index index, IPSubValue value)
	{
		if(isValidIndex(index))
		{
			ip[index] = value;
		}
	}


	HOST_ADDRESS_T inline
	IPSubValue HostAddress<IPSubValue, numDigits>::getSubValue(Index index) const
	{
		if(isValidIndex(index))
		{
			return ip[index];
		}

		return 0;
	}

	HOST_ADDRESS_T inline
	bool PTRMI::HostAddress<IPSubValue, numDigits>::setSubValues(const IPSubValue values[numDigits])
	{
		Index	i;

		if(values)
		{
			for(i = 0; i < numDigits; i++)
			{
				setSubValue(i, values[i]);
			}

			setIPDefined(true);

			updateURL();

			return true;
		}

		setIPDefined(false);

		updateURL();

		return false;
	}

	HOST_ADDRESS_T inline
	bool PTRMI::HostAddress<IPSubValue, numDigits>::setSubValues(const unsigned int values[numDigits])
	{
		Index	i;

		if(values)
		{
			for(i = 0; i < numDigits; i++)
			{
				setSubValue((Index)i, (IPSubValue)values[i]);
			}

			setIPDefined(true);

			return true;
		}

		setIPDefined(false);

		updateURL();

		return false;
	}



	HOST_ADDRESS_T inline
	bool HostAddress<IPSubValue, numDigits>::isValidIndex(Index index) const
	{
		return index < numDigits;
	}


	HOST_ADDRESS_T inline
	void HostAddress<IPSubValue, numDigits>::clear(void)
	{
		setURL(L"");

		setPort(0);

		unsigned int t;

		for(t = 0; t < numDigits; t++)
		{
			setSubValue((Index)t, DEFAULT_SUBVALUE);
		}

		setIPDefined(false);
	}

	HOST_ADDRESS_T inline
	bool PTRMI::HostAddress<IPSubValue, numDigits>::getPortNumberFromString(const std::wstring &string, size_t* readPos)
	{
		Port	initPort = 0;
		size_t	pos;

		if(swscanf_s(&(string[*readPos]), L"%d", &initPort) == 1)
		{
			if(initPort > 0)
			{
				setPort(initPort);

				if((pos = string.find_first_of(L"/", *readPos)) != std::wstring::npos)
				{
					*readPos = pos;
				}
				else
				{
					*readPos = string.length();
				}
			}

			return true;
		}

		return false;
	}


	HOST_ADDRESS_T inline
	bool PTRMI::HostAddress<IPSubValue, numDigits>::getDigitsFromString(const std::wstring &string, size_t *readPos)
	{
		size_t	pos;
		bool	found = false;

		if((pos = string.find_first_of(L"://")) == std::wstring::npos)
			return false;

		pos += 3;

		unsigned int values[numDigits];

		if(numDigits == 4)
		{
			if(swscanf_s(&(string[pos]), L"%d.%d.%d.%d", &(values[0]), &(values[1]), &(values[2]), &(values[3])) == 4)
			{
				found = true;
			}
		}
		else
		if(numDigits == 6)
		{
			if(swscanf_s(&(string[pos]), L"%d.%d.%d.%d.%d.%d", &(values[0]), &(values[1]), &(values[2]), &(values[3]), &(values[4]), &(values[5])) == 4)
			{
				found = true;
			}
		}

		if(found)
		{
			setSubValues(values);

			if(readPos)
			{
				size_t p1;
				size_t p2;

				p1 = string.find_first_of(L":", pos);
				p2 = string.find_first_of(L"/", pos);

				if(p1 != std::wstring::npos && p1 < p2)
				{
					(*readPos) = p1;
				}
				else
				{
					(*readPos) = p2;
				}

				(*readPos)++;
			}

			wchar_t buffer[1024];

			if(numDigits == 4)
			{
            wsprintfW(buffer, L"%d.%d.%d.%d", getSubValue(0), getSubValue(1), getSubValue(2), getSubValue(3));
			}
			else
			if(numDigits == 6)
			{
            wsprintfW(buffer, L"%d.%d.%d.%d.%d.%d", getSubValue(0), getSubValue(1), getSubValue(2), getSubValue(3), getSubValue(4), getSubValue(5), getSubValue(6));
			}

			setURL(buffer);			
		}

		return found;
	}


	HOST_ADDRESS_T inline
	bool PTRMI::HostAddress<IPSubValue, numDigits>::getStringFromDigits(std::wstring &string)
	{
		wchar_t	buffer[64];

		if(numDigits == 4)
		{
			swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"%d.%d.%d.%d", getSubValue(0), getSubValue(1), getSubValue(2), getSubValue(3));
		}
		else
		if(numDigits == 6)
		{
            swprintf(buffer, sizeof(buffer)/ sizeof(buffer[0]), L"%d.%d.%d.%d.%d.%d", getSubValue(0), getSubValue(1), getSubValue(2), getSubValue(3), getSubValue(4), getSubValue(5));
		}

		string = buffer;

		return true;
	}


	HOST_ADDRESS_T inline
	bool PTRMI::HostAddress<IPSubValue, numDigits>::updateURL(void)
	{
		return getStringFromDigits(url);
	}


	HOST_ADDRESS_T inline
	bool PTRMI::HostAddress<IPSubValue, numDigits>::getFromURL(const std::wstring &string, size_t *readPos)
	{
		size_t	        start;
		size_t	        end;
		unsigned int	items;
		unsigned int	p = 0;
		wchar_t			buffer[2048];

		if((start = string.find(L"://"))  == std::wstring::npos)
			return false;

		start += 3;

		if((end = string.find(L"/", start)) == std::wstring::npos)
		{
			end = string.length();
		}


		unsigned int i[4];

		if((items = swscanf(&(string.c_str()[start]), L"%d.%d.%d.%d:%d", &i[0], &i[1], &i[2], &i[3], &p)) >= 4)
		{
			setSubValues(i);
		}
		else
		if(swscanf(&(string.c_str()[start]), L"%[^:/]:%d", buffer, &p) == 0)
		{
			return 0;
		}

		if(p)
		{
			setPort(p);
		}

		url = string.substr(start, end - start);

		if(readPos)
		{
															// Strip off trailing separators
			while(string[end] == L'/')
			{
				++end;
			}

			*readPos = end;
		}

		return true;
/*
		std::wstring t;
		unsigned int r;
		unsigned int i1, i2, i3, i4, port;

		t = L"1.2.3.4";
		t = L"localhost:8091";

		wchar_t n[1024];

		r = swscanf(t.c_str(), L"%d.%d.%d.%d:%d", &i1, &i2, &i3, &i4, &port);

		r = swscanf(t.c_str(), L"%[^:]:%d", n, &port);
*/

/*
		bool gotDigits	= false;
		bool gotPort	= false;

		size_t start = 0;
		size_t end, pos1, pos2;

		size_t digitsPos = 0;
															// Try to parse dotted decimal format first
		if(getDigitsFromString(string, &start))
		{
			gotDigits = true;
			--start;
		}
		else
		{

			if((start = string.find_first_of(L"://"))  == std::wstring::npos)
				return false;

			start += 3;
		}

		pos1 = string.find_first_of(L":", start);
		pos2 = string.find_first_of(L"/", start);

															// May be a port number specified
		if(pos1 != std::wstring::npos && (pos1 < pos2))
		{
			end = pos1;
			++pos1;
			getPortNumberFromString(string, &pos1);
			gotPort = true;
		}
		else
		{
															// If port not specified, try to get end of host name definition
			if((end = pos2) == std::wstring::npos)
				end = string.length();
		}


		if(readPos)
		{
			if(gotPort)
			{
				(*readPos) = pos1 + 1;
			}
			else
			{
				(*readPos) = end + 1;
			}
		}

		if(gotDigits == false)
		{
			std::wstring r = string.substr(start, end - start);

			if(r.length() > 0)
			{
				setURL(r);

				return true;
			}
		}
		else
		{
			return true;
		}

		return false;

*/
	}

} // End PTRMI namespace