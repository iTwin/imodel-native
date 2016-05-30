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
