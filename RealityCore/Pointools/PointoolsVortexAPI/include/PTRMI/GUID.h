/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <string>

#include <PTRMI/PTRMI.h>
#include <PTRMI/Array.h>

namespace PTRMI
{

class DataBuffer;

class GUID
{

public:

	typedef uint64_t	PartialValue;

protected:

#ifdef NEEDS_WORK_VORTEX_DGNDB
	::GUID		guid;
#endif
	bool		generated;


protected:

	void setGenerated(bool value)
	{
		generated = value;
	}

	bool getGenerated(void) const
	{
		return generated;
	}

	bool isNULL(void) const
	{
		return (getRawFirst64() == 0 && getRawSecond64() == 0);
	}

	void						setRawFirst64		(PartialValue value);
	void						setRawSecond64		(PartialValue value);


public:

								GUID				(void);

	void						clear				(void);

	void						setGUID				(const PTRMI::GUID &initGUID);

	bool						isValidGUID			(void) const;

	void						generate			(void);

	bool						operator ==			(const PTRMI::GUID &otherGUID) const;

	bool						operator<			(const PTRMI::GUID &other) const;

	void						read				(DataBuffer &buffer);
	void						write				(DataBuffer &buffer) const;
	static DataSize				getMaxWriteSize		(void);

	void						readPartial			(DataBuffer &buffer) {}
	void						writePartial		(DataBuffer &buffer) const {}

	void						setRaw				(PartialValue upperValue, PartialValue lowerValue);

	PartialValue				getRawFirst64		(void) const;
	PartialValue				getRawSecond64		(void) const;

	bool						setHexString		(std::wstring &string);
	bool						getHexString		(std::wstring &string) const;
};

} // End PTRMI namespace
