/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

enum class TrackUsageStatus
	{
	BadParam = -2,
	NotEntitled = -1,
	Error = 0,
	EntitledButErrorUsageTracking = 1,
	Success = 2
	};
