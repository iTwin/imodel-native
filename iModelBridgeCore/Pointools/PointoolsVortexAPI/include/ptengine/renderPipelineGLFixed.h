/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma  once

#include <ptengine/renderPointsMethod.h>
#include <ptengine/renderContext.h>

namespace pointsengine
{
	class RenderPipeline_GLFixed : public RenderPipelineI
	{
	public:
		RenderPipeline_GLFixed( RenderMethodI *method );

		void renderPoints( PointsBufferI *buffer, RenderContext *context);
		void renderSelPoints( PointsBufferI *buffer, RenderContext *context );

		bool isSupportedOnPlatform() const;
		bool useAggregateBuffers( const RenderSettings *settings, const pcloud::Voxel *voxel ) const;
	};
}
