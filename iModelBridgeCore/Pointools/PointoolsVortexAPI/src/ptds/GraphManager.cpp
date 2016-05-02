#include "PointoolsVortexAPIInternal.h"
#include <Windows.h>

#include <ptds/GraphManager.h>
#include <algorithm>
#include <ptds/DataSourceAnalyzer.h>

#define GRAPH_MUTEX_TIMEOUT		(1000 * 10)


namespace ptds
{

GraphManager	graphManager;

typedef Series<float, 2>	Series2F;


void testGraph(void)
{

	Series2F		*s[10];
	float			a, b;
	Series2F::Point	minimum, maximum;

	s[0] = new Series2F(L"Series A");
	s[1] = new Series2F(L"Series B");

	s[0]->calculateMinMax(minimum, maximum);

	Series2F::Point average1	= s[0]->calculateMean();
	double variance				= s[0]->calculateVariance(Series2F::Point::Y);
	double stdDev				= s[0]->calculateStdDev(Series2F::Point::Y);
	bool ls						= s[0]->calculateLinearLeastSquares2D(a, b, Series2F::Point::X, Series2F::Point::Y);

	Graph	*graph1;
	Graph	*graph2;


	GraphEntityStyle	style;
	style.setColor(GraphEntityStyle::ColorRGB(255, 0, 0));
	s[0]->setStyle(style);

	style.setColor(GraphEntityStyle::ColorRGB(255, 255, 0));
	s[1]->setStyle(style);

	graph1 = graphManager.newGraph(L"Graph 1");
	graph2 = graphManager.newGraph(L"Graph 2");

	s[0]->clear();

	s[0]->generateSquareWave(100, Series2F::Point::X, 1, Series2F::Point(0, 0), Series2F::Point(0, 0), Series2F::Point(0, 5));

//	s1.setIndexRange(10, 20);
//	s1.setIndexRangeEnabled(true);


	s[1]->generateRandom(100, Series2F::Point::X, 1, Series2F::Point(0, 0), Series2F::Point(0, 0), Series2F::Point(0, 15));
	s[1]->setPreserveAspectRatioEnabled(false);

//	graph1->addEntity(*(s[0]));
//	graph1->addEntity(*(s[1]));

//	graph2->addEntity(*(s[0]));

	graph1->createWindow(L"Graph Test", 100, 100, 1000, 400);
	graph2->createWindow(L"Graph Test", 100, 500, 1000, 400);


//	graphManager.update();

}

Graph::Graph(void)
{
	initialize(NULL);
}

Graph::Graph(const wchar_t *graphName)
{
	initialize(graphName);
}


bool Graph::initialize(const wchar_t *graphName)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return false;

//	setEntityOverlayMode(GraphEntityOverlaySeparate);
	setEntityOverlayMode(GraphEntityOverlayCombined);

	setBorderPixelsMin(Vector2i(60, 25));
	setBorderPixelsMax(Vector2i(25, 25));

	setWindowTimer(0);

	setUpdateRandomEnabled(false);

	setHWND(NULL);

	setName(graphName);

	setIncludeOriginX(false);
	setIncludeOriginY(false);

	return true;
}


void Graph::addEntity(GraphEntity &entity)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	entities.push_back(&entity);
}


GraphEntity *Graph::getEntity(Index index)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return false;

	if(index >= getNumEntities())
		return NULL;

//	GraphEntityList::iterator it = std::next(entities.begin(), index);

	GraphEntityList::iterator it = entities.begin();
	std::advance(it, index);

	return *it;
}

void Graph::removeEntity(GraphEntity &entity)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	GraphEntityList::iterator	it;

	if((it = std::find(entities.begin(), entities.end(), &entity)) != entities.end())
	{
		entities.erase(it);
	}
}


unsigned int Graph::removeAllEntities(void)
{
	unsigned int numEntities = getNumEntities();

	entities.clear();

	return numEntities;
}


unsigned int Graph::getNumEntities(void)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return 0;

	return entities.size();
}

LRESULT CALLBACK Graph::windowMessageHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	Graph		*	graph;

	switch(Msg)
	{

	case WM_CREATE:

		SetTimer(hWnd, 0x123456, 1000, (TIMERPROC) NULL);

		break;

	case WM_TIMER:

		if(graph = graphManager.getGraphWithWnd(hWnd))
		{
			if(graph->getUpdateRandomEnabled())
			{
//				graph->updateRandom();
			}

			if(graph->getWindowTimer() == 0)
			{
				graph->setWindowTimer(wParam);
			}

			if(graph->getWindowTimer() == wParam)
			{
				InvalidateRect (hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
			}
		}

		break;

	case WM_ERASEBKGND:

		return TRUE;

	case WM_PAINT:
        {

    #ifdef HAVE_GDIPLUS
        PAINTSTRUCT		ps;
        HDC				hDC;
        hDC = BeginPaint(hWnd, &ps);

        graphManager.draw(hWnd, hDC, ps);

        EndPaint(hWnd, &ps);
    #endif
        }
		break;

	case WM_DESTROY:
		PostQuitMessage(WM_QUIT);
		break;

	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}


	return 0;
}

void Graph::createWindow(const wchar_t *windowName, int posX, int posY, int width, int height)
{
#ifdef HAVE_GDIPLUS
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	WNDCLASSEX		WndCls;

	HINSTANCE hInstance = GetModuleHandle (0);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);


	WndCls.cbSize        = sizeof(WndCls);
	WndCls.style         = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	WndCls.lpfnWndProc   = Graph::windowMessageHandler;
	WndCls.cbClsExtra    = 0;
	WndCls.cbWndExtra    = 0;
	WndCls.hInstance     = hInstance;
	WndCls.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	WndCls.hCursor       = LoadCursor(NULL, IDC_ARROW);
	WndCls.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndCls.lpszMenuName  = NULL;
	WndCls.lpszClassName = windowName;
	WndCls.hIconSm       = LoadIcon(hInstance, IDI_APPLICATION);
	RegisterClassEx(&WndCls);

	hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
		windowName, windowName,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		posX, posY, width, height,
		NULL, NULL, hInstance, NULL);

	setHWND(hWnd);

//	ShowWindow(hWnd, SW_SHOW);
//	UpdateWindow(hWnd);

//	GdiplusShutdown(gdiplusToken);

//	return static_cast<int>(Msg.wParam);
#endif
}


void Graph::calculateMinMax(Vector2d &minimum, Vector2d &maximum)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	GraphEntityList::iterator		it;
	GraphEntity					*	entity;
	Vector2d						entityMin, entityMax;
	bool							set = false;

	for(it = entities.begin(); it != entities.end(); it++)
	{
		entity = *it;

		if(entity)
		{

			entity->calculateMinMax(entityMin, entityMax);

			if(set == false)
			{
				minimum = entityMin;
				maximum = entityMax;

				set = true;
			}

			minimum.insertMin(entityMin);
			maximum.insertMax(entityMax);
		}
	}

}



bool Graph::calculateFrustum(bool preserveAspectRatio, Vector2d &extentsMin, Vector2d &extentsMax, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return false;

	double		frustumAspectRatio;
	double		extentsAspectRatio;

	Vector2d	graphExtentsOffset	= (extentsMax - extentsMin) * 0.5;

	frustumAspectRatio = static_cast<double>(getWindowWidth()) / static_cast<double>(getWindowHeight());
	extentsAspectRatio = graphExtentsOffset[0] / graphExtentsOffset[1];

	if(preserveAspectRatio)
	{
		if(frustumAspectRatio > extentsAspectRatio)
		{
			frustumMin = extentsMin;

															// Fit frustum Y to extents Y and calculate frustum X
			frustumMax[0] = ((extentsMax[1] - extentsMin[1]) * frustumAspectRatio) + extentsMin[0];
			frustumMax[1] = extentsMax[1];
		}
		else
		{
															// Fit frustum X to extents X and calculate frustum Y
			frustumMin = extentsMin;

			frustumMax[0] = extentsMax[0];
			frustumMax[1] = ((extentsMax[0] - extentsMin[0]) * (1 / frustumAspectRatio)) + extentsMin[1];
		}
	}
	else
	{
		frustumMin = extentsMin;
		frustumMax = extentsMax;
	}

	if(getIncludeOriginX())
	{
		frustumMin.insertMin(Vector2d(0, frustumMin[1]));
	}

	if(getIncludeOriginY())
	{
		frustumMin.insertMin(Vector2d(frustumMin[0], 0));
	}

	Vector2d	window(windowWidth, windowHeight);

	Vector2d	frustumPerPixel = (frustumMax - frustumMin) / window;
	Vector2d	frustumBorderMin = frustumPerPixel * (getBorderPixelsMin() * getNumEntities());
	frustumMin -= frustumBorderMin;

	Vector2d	frustumBorderMax = frustumPerPixel * getBorderPixelsMax();
	frustumMax += frustumBorderMax;

	offset		= frustumMin * -1;
	scalar[0]	= 1;
	scalar[1]	= 1;

	scalar[0]	*= getWindowWidth() / (frustumMax[0] - frustumMin[0]);
	scalar[1]	*= getWindowHeight() / (frustumMax[1] - frustumMin[1]);

	return true;
}


bool Graph::calculateGraphFrustum(Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return false;

	Vector2d	graphExtentsMin;
	Vector2d	graphExtentsMax;


	calculateMinMax(graphExtentsMin, graphExtentsMax);

	return calculateFrustum(false, graphExtentsMin, graphExtentsMax, frustumMin, frustumMax, offset, scalar);
}


bool Graph::calculateEntityFrustum(GraphEntity *entity, Vector2d &frustumMin, Vector2d &frustumMax, Vector2d &offset, Vector2d &scalar)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return false;

	if(entity == NULL)
		return false;

	Vector2d	entityExtentsMin;
	Vector2d	entityExtentsMax;

	entity->calculateMinMax(entityExtentsMin, entityExtentsMax);

	return calculateFrustum(entity->getPreserveAspectRatioEnabled(), entityExtentsMin, entityExtentsMax, frustumMin, frustumMax, offset, scalar);

}

#ifdef HAVE_GDIPLUS
void Graph::drawAxisLines(HWND hWnd, HDC hDC, Gdiplus::Graphics &graphics, unsigned int windowWidth, unsigned int windowHeight)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	unsigned int t;

	Gdiplus::Pen pen(Gdiplus::Color(255, 100, 100, 100));

	for(t = (borderPixelsMin[1] * getNumEntities()); t < windowHeight; t += 20)
	{
		unsigned int y = windowHeight - t;
		graphics.DrawLine(&pen, 0, y, windowWidth, y);
	}

}

bool Graph::draw(HWND hWnd, HDC hDC, PAINTSTRUCT &ps)
{

	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return false;

	GraphEntityList::iterator		it;
	GraphEntity					*	entity;
	Vector2d						offset, scalar;
	RECT							clientRect;
	Vector2d						frustumMin;
	Vector2d						frustumMax;
	unsigned int					entityCounter = 0;
	Gdiplus::Bitmap				*	memBitmap;
	Gdiplus::Graphics			*	memGraphics;
	bool							result = true;

	GetClientRect(hWnd,&clientRect);
	setWindowWidth(clientRect.right - clientRect.left + 1);
	setWindowHeight(clientRect.bottom - clientRect.top + 1);



	if((memBitmap = new Gdiplus::Bitmap(getWindowWidth(), getWindowHeight())) == NULL)
	{
		return false;
	}

	if((memGraphics = Gdiplus::Graphics::FromImage(memBitmap)) == NULL)
	{
		if(memBitmap)
			delete memBitmap;

		return false;
	}


	Gdiplus::LinearGradientBrush linGrBrush(Gdiplus::Point(getWindowWidth() - 1, 0),
										    Gdiplus::Point(getWindowWidth() - 1, getWindowHeight() - 1),
										    Gdiplus::Color(255, 180, 180, 180),
										    Gdiplus::Color(255, 0, 0, 0));

	Gdiplus::Pen pen(&linGrBrush);

	memGraphics->FillRectangle(&linGrBrush, 0, 0, getWindowWidth(), getWindowHeight());


	drawAxisLines(hWnd, hDC, *memGraphics, getWindowWidth(), getWindowHeight());

	if(getEntityOverlayMode() == GraphEntityOverlayCombined)
	{
		calculateGraphFrustum(frustumMin, frustumMax, offset, scalar);
	}

	for(it = entities.begin(); it != entities.end(); it++)
	{
		entity = *it;

		entityCounter++;

		if(getEntityOverlayMode() == GraphEntityOverlaySeparate)
		{
			calculateEntityFrustum(entity, frustumMin, frustumMax, offset, scalar);
		}

		if(entity)
		{
			entity->draw(hWnd, hDC, *this, *memGraphics, entityCounter, windowWidth, windowHeight, frustumMin, frustumMax, offset, scalar);
		}
	}


	Gdiplus::Graphics graphics(hDC);

	Gdiplus::Status status;
	if((status = graphics.DrawImage(memBitmap, 0, 0)) != Gdiplus::Ok)
	{
		result = false;
	}

	if(memGraphics)
	{
		delete memGraphics;
	}

	if(memBitmap)
	{
		delete memBitmap;
	}

	return result;
}
#endif // HAVE_GDIPLUS



void Graph::setBorderPixelsMin(Vector2i &pixels)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	borderPixelsMin = pixels;
}


Vector2i Graph::getBorderPixelsMin(void)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return Vector2i();

	return borderPixelsMin;
}


void Graph::setBorderPixelsMax(Vector2i &pixels)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	borderPixelsMax = pixels;
}


Vector2i Graph::getBorderPixelsMax(void)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return Vector2i();

	return borderPixelsMax;
}



void Graph::update(void)
{

}

void Graph::updateRandom(void)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	GraphEntityList::iterator		it;
	GraphEntity					*	entity;

	for(it = entities.begin(); it != entities.end(); it++)
	{
		entity = *it;

		if(entity)
		{
			entity->updateRandom();
		}
	}
}



/*
void Graph::draw(HWND hWnd, HDC hDC, PAINTSTRUCT &ps)
{
	PAINTSTRUCT		Ps;

	GraphEntityList::iterator		it;
	GraphEntity					*	entity;
	GraphEntityStyle			*	entityStyle;
	GraphEntityStyle::ColorRGB		colorRGB;
	Vector2d						graphMin, graphMax;
	Vector2d						offset, scalar;
	RECT clientRect;

	GetClientRect(hWnd,&clientRect);

	setWindowWidth(clientRect.right - clientRect.left);
	setWindowHeight(clientRect.bottom - clientRect.top);

	Vector2d	frustumMin;
	Vector2d	frustumMax;
	double		frustumAspectRatio;
	double		extentsAspectRatio;
	Vector2d	frustumFittedDelta;

	calculateMinMax(graphMin, graphMax);

	Vector2d	graphExtentsCenter	= (graphMax + graphMin) * 0.5;
	Vector2d	graphExtentsOffset	= (graphMax - graphMin) * 0.5;

	frustumAspectRatio = static_cast<double>(getWindowWidth()) / static_cast<double>(getWindowHeight());
	extentsAspectRatio = graphExtentsOffset[0] / graphExtentsOffset[1];

	if(frustumAspectRatio > extentsAspectRatio)
	{
															// Fit frustum Y to extents Y and calculate frustum X
		frustumMin[1]			= graphMin[1];
		frustumMax[1]			= graphMax[1];

		frustumFittedDelta[0]	= graphExtentsOffset[1] * frustumAspectRatio;

		frustumMax[0]			= graphExtentsCenter[0] + frustumFittedDelta[0];
		frustumMin[0]			= graphExtentsCenter[0] - frustumFittedDelta[0];
	}
	else
	{
															// Fit frustum X to extents X and calculate frustum Y
		frustumMin[0]			= graphMin[0];
		frustumMax[0]			= graphMax[0];

		frustumFittedDelta[1]	= graphExtentsOffset[0] * (1 / frustumAspectRatio);

		frustumMax[1]			= graphExtentsCenter[1] + frustumFittedDelta[1];
		frustumMin[1]			= graphExtentsCenter[1] - frustumFittedDelta[1];
	}

	offset		= frustumMin * -1;
	scalar[0]	= 1;
	scalar[1]	= 1;

	scalar[0]	*= getWindowWidth() / (frustumMax[0] - frustumMin[0]);
	scalar[1]	*= getWindowHeight() / (frustumMax[1] - frustumMin[1]);


	for(it = entities.begin(); it != entities.end(); it++)
	{
		entity = *it;

		if(entity)
		{

			entity->draw(hWnd, hDC, windowWidth, windowHeight, frustumMin, frustumMax, offset, scalar);
		}
	}

}
*/

Graph *GraphManager::newGraph(const wchar_t *graphName)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return NULL;

	Graph *graph;

	if((graph = new Graph(graphName)) == NULL)
	{
		return NULL;
	}

	addGraph(graph);

	return graph;
}


Graph *GraphManager::getGraph(Index index)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return NULL;

	if(index >= getNumGraphs())
		return NULL;

	GraphList::iterator it = graphs.begin();
	std::advance(it, index);

	return *it;
}


Graph *GraphManager::getGraphWithWnd(HWND hWnd)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return NULL;

	GraphList::iterator		it;
	Graph				*	graph;
	
	for(it = graphs.begin(); it != graphs.end(); it++)
	{
		if(graph = *it)
		{
			if(graph->getHWND() == hWnd)
			{
				return graph;
			}
		}
	}

	return NULL;
}


Graph *GraphManager::getGraphWithName(const wchar_t *graphName)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return NULL;

	GraphList::iterator		it;
	Graph				*	graph;

	for(it = graphs.begin(); it != graphs.end(); it++)
	{
		if(graph = *it)
		{
			if(graph->getName().compare(graphName) == 0)
			{
				return graph;
			}
		}
	}

	return NULL;
}


bool GraphManager::deleteGraph(Graph *graph)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return false;

	GraphList::iterator	it;

	if(graph == NULL)
	{
		return false;
	}

	if((it = std::find(graphs.begin(), graphs.end(), graph)) != graphs.end())
	{
		Graph *graph = *it;

		if(graph != NULL)
		{
			delete graph;
		}

		graphs.erase(it);

		return true;
	}

	return false;
}


bool GraphManager::addGraph(Graph *graph)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return false;

	if(graph == NULL)
	{
		return false;
	}

	graphs.push_back(graph);

	return true;
}

#ifdef HAVE_GDIPLUS
void GraphManager::draw(HWND hWnd, HDC hDC, PAINTSTRUCT &ps)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	Graph *	graph;

	if(graph = getGraphWithWnd(hWnd))
	{
		graph->draw(hWnd, hDC, ps);
	}

}
#endif


void GraphManager::update(void)
{
	MSG         Msg;

	while(GetMessage(&Msg, NULL, 0, 0) )
	{
		TranslateMessage(&Msg);
		DispatchMessage( &Msg);
	}

}


void GraphManager::updateRandom(void)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	GraphList::iterator		it;
	Graph				*	graph;

	for(it = graphs.begin(); it != graphs.end(); it++)
	{
		graph = *it;

		if(graph)
		{
			graph->updateRandom();
		}
	}

}

unsigned int GraphManager::getNumGraphs()
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return 0;

	return graphs.size();
}


unsigned int GraphManager::removeAllEntities(Graph *graph)
{
	if(graph == NULL)
		return 0;

	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return 0;

	return graph->removeAllEntities();
}


unsigned int GraphManager::removeAllEntities(void)
{
	GraphList::iterator		it;
	Graph				*	graph;
	unsigned int			numEntities = 0;

	PTRMI::MutexScope	mutexScope(mutex, GRAPH_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return 0;

	for(it = graphs.begin(); it != graphs.end(); it++)
	{
		if(graph = *it)
		{
			numEntities += graph->removeAllEntities();
		}
	}

	return numEntities;
}



GraphEntityStyle::GraphEntityStyle(void)
{
	setColor(ColorRGB(0, 0, 0));
}


GraphEntity::GraphEntity(void)
{
	clear();
}

GraphEntity::GraphEntity(const wchar_t *entityName)
{
	clear();

	setName(entityName);
}


void GraphEntity::clear(void)
{
	PTRMI::MutexScope	mutexScope(mutex, GRAPH_ENTITY_MUTEX_TIMEOUT);
	if(mutexScope.isLocked() == false)
		return;

	setPreserveAspectRatioEnabled(false);

	setIndexRangeEnabled(false);

	setIndexRange(0, 0);

	setName(L"");

	setUnitScale(Vector2d(1.0, 1.0));
}


void GraphEntity::beginStyle(HWND hWnd, HDC hDC)
{

}


void GraphEntity::endStyle(void)
{

}

}