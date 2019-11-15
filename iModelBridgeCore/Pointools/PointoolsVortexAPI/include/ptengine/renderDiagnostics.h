/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ptcloud2/voxel.h>

namespace pointsengine
{
	class RenderVoxelDiagnosticInfo
	{
	public:
		static void beginVoxelEditStateRender();
		static void endVoxelEditStateRender();

		static void renderVoxelEditState( const pcloud::Voxel *vx );
		static void renderVoxelOutline( const pcloud::Voxel *vx );
	private:
		template<typename T> static void renderBox( const pt::BBox<T> *bb );
	};
}
