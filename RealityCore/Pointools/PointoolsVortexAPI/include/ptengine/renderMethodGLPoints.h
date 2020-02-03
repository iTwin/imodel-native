/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ptengine/renderPointsMethod.h>

namespace pointsengine
{
	class RenderMethod_GLPoints : public RenderMethodI
	{
	public:
		void renderPoints( PointsBufferI *buffer, const RenderSettings *settings );
	};
}
