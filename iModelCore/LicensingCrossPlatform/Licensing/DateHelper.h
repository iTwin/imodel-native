/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/DateHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
//#include <ctime>
#include <iomanip>
#include <sstream>

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct DateHelper
{
private:
	/*time_t StringToTimeFormatted(const std::string& string, const std::string format) const
		{
		std::tm tm;
		std::istringstream ss(string);
		ss >> std::get_time(&tm, format);
		
		}*/
public:
	static time_t StringToTime(const std::string& string)
		{
		std::tm tm;
		std::istringstream ss(string);
		ss >> std::get_time(&tm, "%Y-%m-%dT%H%M%S");
		return std::mktime(&tm);
		}

	static std::string TimeToString(const time_t& time)
		{
		return "";
		}
};

END_BENTLEY_LICENSING_NAMESPACE
