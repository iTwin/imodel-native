/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

enum class TrackUsageStatus
	{
	BadParam = -2,
	NotEntitled = -1,
	EntitledButErrorUsageTracking = 1,
	Success = 2
	};
