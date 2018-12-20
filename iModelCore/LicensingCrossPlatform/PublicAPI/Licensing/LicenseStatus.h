/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/PublicAPI/Licensing/LicenseStatus.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

enum class LicenseStatus
	{
	Error = -1,
	Ok = 101,
	Offline = 102,
	Expired = 105,
	AccessDenied = 106,
	DisabledByLogSend = 107,
	DisabledByPolicy = 108,
	Trial = 109,
	NotEntitled = 110
	};
