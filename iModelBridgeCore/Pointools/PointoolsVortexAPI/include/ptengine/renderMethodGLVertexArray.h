#pragma once

#include <ptengine/renderPointsMethod.h>

namespace pointsengine
{
	class RenderMethod_GLVertexArray : public RenderMethodI
	{
	public:
		void renderPoints( PointsBufferI *buffer, const RenderSettings *settings );
	private:
		void texUnitHack( const PointsBufferI *buffer, const RenderSettings *settings ) const;
	};
}