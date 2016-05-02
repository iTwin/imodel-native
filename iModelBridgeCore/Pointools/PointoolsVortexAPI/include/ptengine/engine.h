#ifndef POINTOOLS_POINTSENGINE_SINGLETONS
#define POINTOOLS_POINTSENGINE_SINGLETONS

#include <Loki/Singleton.h>

#include <ptengine/VisibilityEngine.h>
#include <ptengine/ptengine_api.h>

namespace pointsengine
{
	class PointsScene;
	class RenderEngine;
	class VisibilityEngine;
	class PointsPager;
	class PointsExchanger;
	class PointsFilteringState;
	class PointLayersState;

	PTENGINE_API PointsScene		&thePointsScene();
#ifdef HAVE_OPENGL
    PTENGINE_API VisibilityEngine	&theVisibilityEngine();
#endif
	PTENGINE_API PointsPager		&thePointsPager();
	PTENGINE_API PointLayersState	&thePointLayersState();

	PTENGINE_API PointsFilteringState	&pointsFilteringState();

	PTENGINE_API void initializeEngine();

	PTENGINE_API void pauseEngine();
	PTENGINE_API void unpauseEngine();

	PTENGINE_API void destroy();
}

#endif