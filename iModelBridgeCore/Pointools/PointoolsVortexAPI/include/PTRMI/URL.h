#pragma once

#include <string>

#include <PTRMI/PTRMI.h>
#include <PTRMI/HostAddress.h>


namespace PTRMI
{
	class DataBuffer;

	const unsigned int URL_MAX_LENGTH = 1024;

	class URL
	{
	public:

		static const wchar_t	PT_PTFL[];
		static const wchar_t	PT_PTRI[];
		static const wchar_t	PT_PTRE[];
		static const wchar_t	PT_PTCE[];
		static const wchar_t	PT_PTCI[];
		static const wchar_t	PT_PTSS[];
		static const wchar_t	PT_PTCF[];
		static const wchar_t	PT_PTMY[];

	protected:

		std::wstring			url;

	public:

								URL						(void);
								URL						(const std::wstring &str);
								URL						(const wchar_t *str);						

		void					set						(const std::wstring &str);
		void					set						(const wchar_t *str);
		void					setURL					(const URL &url);
		const std::wstring	&	getString				(void) const;

		void					getString				(std::string &result);

		bool					isValidURL				(void) const;
		unsigned int			getLength				(void) const;
		bool					isEmpty					(void) const;

		bool					getProtocol				(URL &protocol, size_t *readPos = NULL) const;
		bool					getHostAddress			(HostAddressIP4 &hostAddress, size_t *readPos = NULL) const;
		bool					getObject				(URL &object) const;
		bool					getRemoteManagerName	(URL &remoteManagerName) const;
		bool					getProtocolHostAddress	(URL &protocolHostAddress) const;

		unsigned int			split					(URL &protocol, URL &hostAddress, URL &object, bool *gotProtocol = NULL, bool *gotHostAddress = NULL, bool *gotObject = NULL);

		void					toUpper					(void);
		void					toLower					(void);

		bool					setProtocol				(URL &newProtocol);

		bool					isProtocol				(const wchar_t *protocolString) const;
		bool					isProtocolNetworked		(void) const;

		URL			&			operator=				(const URL &other);
		URL			&			operator+=				(const URL &other);
		URL			&			operator+=				(const std::wstring &str);
		URL			&			operator+=				(const wchar_t *str);
		URL						operator+				(const URL &other) const;
		URL						operator+				(const std::wstring &str) const;
		URL						operator+				(const wchar_t *str) const;

		bool					operator==				(const URL &other) const;
		bool					operator!=				(const URL &other) const;
		bool					operator<				(const URL &other) const;

		URL			&			copy					(const URL &n);

		void					read					(DataBuffer &buffer);
		void					write					(DataBuffer &buffer) const;

		static PTRMI::DataSize	getMaxWriteSize			(DataSize maxURLCharacters);

		void					readPartial				(DataBuffer &buffer) {}
		void					writePartial			(DataBuffer &buffer) {}
	};

} // End PTRMI namespace
