#include "PointoolsVortexAPIInternal.h"


#include <ptengine/pointsScene.h>
#include <ptengine/pointspager.h>
#include <ptengine/renderengine.h>
#include <ptengine/visibilityengine.h>
#include <ptengine/pointsfilter.h>
#include <ptengine/pointLayers.h>
#include <ptengine/engine.h>

using namespace pointsengine;

namespace ___pntsengine
{
	PointsScene*			g_pointsScene;
	VisibilityEngine*		g_visibilityEngine;
	PointsPager*			g_pointsPager;
	PointsFilteringState*	g_pointsFilteringState;
	PointLayersState*		g_pointLayers;
};
using namespace ___pntsengine;

void pointsengine::initializeEngine()
{
	g_pointsScene = new PointsScene;
	g_pointsPager = new PointsPager;
	g_pointsFilteringState = new PointsFilteringState;
	g_pointLayers = new PointLayersState;

	g_pointsScene->initialize();
    g_pointsPager->initialize();
    g_pointLayers->initialize();

    g_visibilityEngine = new VisibilityEngine;
    g_visibilityEngine->initialize();
}
	
PointsScene		&pointsengine::thePointsScene()
{
	if (!g_pointsScene) initializeEngine();
	return *g_pointsScene;
}
VisibilityEngine	&pointsengine::theVisibilityEngine() 
{
	if (!g_pointsScene) initializeEngine();	
	return *g_visibilityEngine;
}
PointsPager		&pointsengine::thePointsPager()
{
	if (!g_pointsScene) initializeEngine();
	return *g_pointsPager;
}
PointLayersState	&pointsengine::thePointLayersState()
{
	if (!g_pointLayers) initializeEngine();
	return *g_pointLayers;
}
PointsFilteringState &pointsengine::pointsFilteringState()
{
	if (!g_pointsScene) initializeEngine();
	return *g_pointsFilteringState;
}
void pointsengine::pauseEngine()
{
	if (!g_pointsScene) return;

	g_pointsPager->pause();
    g_visibilityEngine->pause();
}
void pointsengine::unpauseEngine()
{
	if (!g_pointsScene) return;

    g_visibilityEngine->unpause();
	g_pointsPager->unpause();
}
void pointsengine::destroy()
{
	pauseEngine();
	
	g_pointsScene->clear();

	if (!g_pointsScene) return;

	delete g_pointsPager;
	
	g_pointsScene->clear(true);

	delete g_pointsScene;	
	delete g_pointsFilteringState;

	g_pointsPager = 0;
    g_pointsScene = 0;

    delete g_visibilityEngine;
    g_visibilityEngine = 0;
}
