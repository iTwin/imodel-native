#ifndef POINTOOLS_POINTSENGINE_SINGLETONS
#define POINTOOLS_POINTSENGINE_SINGLETONS

#include <loki/Singleton.h>

#include <ptengine/visibilityengine.h>
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
    PTENGINE_API VisibilityEngine	&theVisibilityEngine();
	PTENGINE_API PointsPager		&thePointsPager();
	PTENGINE_API PointLayersState	&thePointLayersState();

	PTENGINE_API PointsFilteringState	&pointsFilteringState();

	PTENGINE_API void initializeEngine();

	PTENGINE_API void pauseEngine();
	PTENGINE_API void unpauseEngine();

	PTENGINE_API void destroy();
}

#endif
