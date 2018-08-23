/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/DateHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <ctime>
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
	static time_t GetCurrentTime()
		{
		time_t ct = time(0);
		time_t currentTime = mktime(gmtime(&ct));
		return currentTime;
		}

	static time_t AddDaysToTime(const time_t& time, int daysToAdd)
		{
		struct tm* tm = localtime(&time);
		tm->tm_mday += daysToAdd;
		time_t newTime = mktime(tm);
		return newTime;
		}

	static time_t AddDaysToCurrentTime(int daysToAdd)
		{
		return AddDaysToTime(GetCurrentTime(),daysToAdd);
		}

	static int64_t GetDaysLeftUntilTime(const time_t& time)
		{
		double secondsLeft = difftime(time, GetCurrentTime());
		int64_t daysLeft = (int)ceil(secondsLeft/(24*60*60));
		return daysLeft;
		}

	static time_t StringToTime(const std::string& string)
		{
		std::tm tm;
		std::istringstream ss(string);
		ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
		return std::mktime(&tm);
		}

	static std::string TimeToString(const time_t& time)
		{
		std::tm* tmtime = (time < 0) ? gmtime(&time) : localtime(&time);
		char buffer[64];
		strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", tmtime);
		return buffer;
		}
};

END_BENTLEY_LICENSING_NAMESPACE
