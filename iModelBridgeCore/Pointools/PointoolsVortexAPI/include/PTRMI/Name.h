#pragma once


#include <PTRMI/URL.h>
#include <PTRMI/GUID.h>
#include <PTRMI/HostAddress.h>

namespace PTRMI
{
	class DataBuffer;
	class URL;

	class Name : public URL, public PTRMI::GUID
	{


	public:

		static const unsigned int MAX_NAME_CHARACTERS = 128;

	public:

		Name(void) {}

		Name(const std::wstring &str)		: URL(str)			{}
		Name(const wchar_t *str)			: URL(str)			{}
		Name(const URL &initURL)			: URL(initURL)		{}
		Name(const PTRMI::GUID &initGUID)	: GUID(initGUID)	{}

		bool operator<(const Name &other) const
		{
			if(getGenerated())
			{
															// If both names are GUIDs
				if(other.getGenerated())
				{
															// Compare GUIDs
					return getGUID() < other.getGUID();
				}
				else
				{
															// This is GUID, other is string so GUIDs always less than URLs
					return true;
				}
			}
			else
			{
				if(other.getGenerated())
				{
															// This is Name, other is GUID, so return false as GUIDs always less than URLs
					return false;
				}
				else
				{
															// Compare URLs
					return url < other.url;
				}
			}
		}

		URL & getURL(void)
		{
			return *this;
		}

		const URL & getURL(void) const
		{
			return *this;
		}

		PTRMI::GUID getGUID(void) const
		{
			return *this;
		}

		void read(DataBuffer &buffer)
		{
			URL::read(buffer);

			GUID::read(buffer);
		}

		void write(DataBuffer &buffer) const
		{
			URL::write(buffer);

			GUID::write(buffer);
		}

		PTRMI::DataSize getMaxWriteSize(PTRMI::DataSize maxChars = MAX_NAME_CHARACTERS)
		{
			return URL::getMaxWriteSize(maxChars) + GUID::getMaxWriteSize();
		}

		void readPartial(DataBuffer &buffer)
		{
		}

		void writePartial(DataBuffer &buffer)
		{
		}


		bool isValid(void) const
		{
															// Return whether URL and GUID are defined
			return (URL::isValidURL() && isValidGUID());
		}

		bool isPartiallyValid(void) const
		{
															// Return whether at least a URL or GUID is defined
			return (URL::isValidURL() || isValidGUID());
		}

		void generateGUIDName(const URL &objectName)
		{
															// Use provisional protocol and host for name
			objectName.getProtocolHostAddress(*this);
															// Generate GUID
			generate();
		}

		void generateGUIDName(const wchar_t *objectName)
		{
			URL url = objectName;

			generateGUIDName(url);
		}


		Name &operator=(const PTRMI::URL &url)
		{
			setURL(url);

			return *this;
		}

		Name &operator=(const PTRMI::GUID &guid)
		{
			setGUID(guid);

			return *this;
		}


		bool operator==(const Name &other) const
		{
															// If GUIDs are both generated
			if(getGenerated() && other.getGenerated())
			{
															// Compare GUIDs
				return GUID::operator==(other);
			}
															// Otherwise, compare URLs
			return URL::operator==(other);
		}


		bool update(const Name &other)
		{
			bool updated = false;
															// If this Name has no URL and given name does, copy it
			if(isValidURL() == false && other.isValidURL())
			{
				*this = other.getURL();

				updated = true;
			}
															// If this Name has no GUID and given name does, copy it
			if(isValidGUID() == false && other.isValidGUID())
			{
				*this = other.getGUID();

				updated = true;
			}

			return updated;
		}
	};

}
