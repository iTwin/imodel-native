/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/DateHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Bentley/DateTime.h>
#if defined(__APPLE__) || defined(ANDROID)
#import <math.h>
#endif

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct DateHelper
{
private:

	static const int64_t dayMillis = 24 * 60 * 60 * 1000;

public:

	static Utf8String GetCurrentTime()
		{
		return DateTime::GetCurrentTimeUtc().ToString();
		}

	static Utf8String AddDaysToTime(Utf8StringCR string, int daysToAdd)
		{
		DateTime time;
		DateTime::FromString(time, string.c_str());

		int64_t millis;
		time.ToUnixMilliseconds(millis);
		millis += (dayMillis*daysToAdd);
		DateTime::FromUnixMilliseconds(time, millis);
		return time.ToString();
		}

	static Utf8String AddDaysToCurrentTime(int daysToAdd)
		{
		return AddDaysToTime(GetCurrentTime(),daysToAdd);
		}

	static int64_t GetDaysLeftUntilTime(Utf8StringCR time)
		{
		int64_t millisLeft = diffdate(time, GetCurrentTime());
		int64_t daysLeft = (int64_t)ceil(millisLeft/dayMillis);
		return daysLeft;
		}

	static int64_t diffdate(Utf8StringCR timeStringL, Utf8String timeStringR)
		{
		DateTime timeL, timeR;
		DateTime::FromString(timeL, timeStringL.c_str());
		DateTime::FromString(timeR, timeStringR.c_str());
		int64_t l_Millis, r_Millis;
		timeL.ToUnixMilliseconds(l_Millis);
		timeR.ToUnixMilliseconds(r_Millis);
		return l_Millis - r_Millis;
		}

};

END_BENTLEY_LICENSING_NAMESPACE
