/******************************************************************************

Pointools Vortex API Examples

EditTool.cpp

Demonstrates layer based point editing capabilities of Vortex

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

//----------------------------------------------------------------------------

*******************************************************************************/
#include "EditTool.h"
#include "../include/PointoolsVortexAPI_resultCodes.h"
#include <math.h>

static RGBc layerButtonCol(96,96,96);
static RGBc layerButtonSelCol(35,174,183);
static RGBc layerButtonVisCol(255, 86, 91);
static RGBc labelCol(60,210,210);

#define PI 3.14159265

//-----------------------------------------------------------------------------
EditTool::EditTool(bool simple) : Tool(CmdRectangleSelect, CmdEvalMode)
//-----------------------------------------------------------------------------
{
	m_simple = simple;
	m_mode = 0;
	m_boxValid = false;

	m_boxColor[0] = 1;
	m_boxColor[1] = 1;
	m_boxColor[2] = 1;

	m_boxPosition[0] = 0;
	m_boxPosition[1] = 0;
	m_boxPosition[2] = 0;

	m_boxRotation[0] = 0;
	m_boxRotation[1] = 0;
	m_boxRotation[2] = 0;

	m_brushSize = 0.3f;

	m_scope = 0;
	m_evalMode = 0;

	m_layersChannel = 0;

	// set up the layer colours as a test
	float col[] = {0,0,0};
	ptSetLayerColor( 0, col, 0 ); // no colour
	
	float col1[] = { 1.0f, 0, 0 };
	ptSetLayerColor( 1, col1, 0.2f );

	float col2[] = { 1.0f, 1.0f, 0 };
	ptSetLayerColor( 2, col2, 0.4f );

	float col3[] = { 0, 1.0f, 0.5f };
	ptSetLayerColor( 3, col3, 0.6f );

	float col4[] = { 1.0f, 0, 1.0f };
	ptSetLayerColor( 4, col4, 0.8f );

	float col5[] = { 0.75f, 0.5f, 0.0f };
	ptSetLayerColor( 5, col5, 0.5f );

	float col6[] = { 0.5f, 0.25f, 1.0f };
	ptSetLayerColor( 5, col6, 0.5f );
}
//-----------------------------------------------------------------------------
void EditTool::command( int cmdId )
//-----------------------------------------------------------------------------
{
	switch (cmdId)
	{
		case CmdRectangleSelect : 
			m_mode = CmdRectangleSelect;
			m_mouse.eventStartX = -1;
			break;

		case CmdPolygonSelect	: 
			m_mode = CmdPolygonSelect;
			m_polygon.clear();
			break;

		case CmdBrushUpdate :
			break;

		case CmdBoxSelect		: 
			m_mode = CmdBoxSelect;
			m_mouse.eventStartX = -1;
			break;

		case CmdBrushSelect		: 
			m_mode = CmdBrushSelect;
			break;

		case CmdSelectHide		: 
			ptHideSelected();
			VortexExampleApp::instance()->updateBoundingBox();
			viewRedraw();
			break;

		case CmdSelectInvert	:
			ptInvertSelection();
			viewRedraw();
			break;

		case CmdVisbilityInvert	: 
			ptInvertVisibility();
			VortexExampleApp::instance()->updateBoundingBox();
			viewRedraw();
			break;

		case CmdResetEdit		: 
			ptClearEdit();
			setCurrentLayer(0); 
			viewRedraw();
			break;

		case CmdRefreshEdit		: 
			ptRefreshEdit();
			VortexExampleApp::instance()->updateBoundingBox();
			viewRedraw();
			break;

		case CmdDeselect		: 
			ptUnselectAll();
			viewRedraw();
			break;

		case CmdSelectMode		: 
			ptSetSelectPointsMode( m_selectMode+1 );
			break;
		
		case CmdLayer1			: 
			setCurrentLayer(0); 
			viewRedraw();
			break;

		case CmdLayer2			: 
			setCurrentLayer(1); 
			viewRedraw();
			break;

		case CmdLayer3			: 
			setCurrentLayer(2); 
			viewRedraw();
			break;

		case CmdLayer4			: 
			setCurrentLayer(3); 
			viewRedraw();
			break;

		case CmdLayer5			: 
			setCurrentLayer(4); 
			viewRedraw();
			break;

		case CmdLayer6			: 
			setCurrentLayer(5); 
			viewRedraw();
			break;

		case CmdLayer7			: 
			setCurrentLayer(6); 
			viewRedraw();
			break;

		case CmdLayer8			: 
			setCurrentLayer(7); 
			viewRedraw();
			break;

		case CmdLockLayer1		:
			toggleLayerLock(0); 
			break;

		case CmdLockLayer2		:
			toggleLayerLock(1); 
			break;

		case CmdLockLayer3		:
			toggleLayerLock(2); 
			break;

		case CmdLockLayer4		:
			toggleLayerLock(3); 
			break;

		case CmdLockLayer5		:
			toggleLayerLock(4); 
			break;

		case CmdLockLayer6		:
			toggleLayerLock(5); 
			break;

		case CmdLockLayer7		:
			toggleLayerLock(6); 
			break;

		case CmdLockLayer8		:
			toggleLayerLock(7); 
			break;


		case CmdVisLayer1		: 
			toggleLayerVisibility(0); 
			break;

		case CmdVisLayer2		: 
			toggleLayerVisibility(1); 
			break;

		case CmdVisLayer3		: 
			toggleLayerVisibility(2); 
			break;
	
		case CmdVisLayer4		: 
			toggleLayerVisibility(3); 
			break;
	
		case CmdVisLayer5		: 
			toggleLayerVisibility(4); 
			break;
	
		case CmdVisLayer6		: 
			toggleLayerVisibility(5); 
			break;

		case CmdVisLayer7		: 
			toggleLayerVisibility(6); 
			break;

		case CmdVisLayer8		: 
			toggleLayerVisibility(7); 
			break;

		case CmdSelLayer1			: 
			ptSelectPointsInLayer(0); 
			viewRedraw();
			break;

		case CmdSelLayer2			: 
			ptSelectPointsInLayer(1); 
			viewRedraw();
			break;

		case CmdSelLayer3			: 
			ptSelectPointsInLayer(2); 
			viewRedraw();
			break;

		case CmdSelLayer4			: 
			ptSelectPointsInLayer(3); 
			viewRedraw();
			break;

		case CmdSelLayer5			: 
			ptSelectPointsInLayer(4); 
			viewRedraw();
			break;

		case CmdSelLayer6			: 
			ptSelectPointsInLayer(5); 
			viewRedraw();
			break;

		case CmdSelLayer7			: 
			ptSelectPointsInLayer(6); 
			viewRedraw();
			break;

		case CmdSelLayer8			: 
			ptSelectPointsInLayer(7); 
			viewRedraw();
			break;

		case CmdDeselLayer1			: 
			ptUnhideAll();
			//ptDeselectPointsInLayer(0); 
			viewRedraw();
			break;

		case CmdDeselLayer2		: 
			ptDeselectPointsInLayer(1); 
			viewRedraw();
			break;

		case CmdDeselLayer3		: 
			ptDeselectPointsInLayer(2); 
			viewRedraw();
			break;

		case CmdDeselLayer4		: 
			ptDeselectPointsInLayer(3); 
			viewRedraw();
			break;

		case CmdDeselLayer5		: 
			ptDeselectPointsInLayer(4); 
			viewRedraw();
			break;

		case CmdDeselLayer6		: 
			ptDeselectPointsInLayer(5); 
			viewRedraw();
			break;

		case CmdDeselLayer7		: 
			ptDeselectPointsInLayer(6); 
			viewRedraw();
			break;

		case CmdDeselLayer8			: 
			ptDeselectPointsInLayer(7); 
			viewRedraw();
			break;
			
		case CmdLayerCol1:			
			chooseColor(0);
			viewRedraw();
			break;

		case CmdLayerCol2:			
			chooseColor(1);
			viewRedraw();
			break;

		case CmdLayerCol3:		 
			chooseColor(2);
			viewRedraw();
			break;

		case CmdLayerCol4:			
			chooseColor(3);
			viewRedraw();
			break;

		case CmdLayerCol5:			
			chooseColor(4);
			viewRedraw();
			break;

		case CmdLayerCol6:			
			chooseColor(5);
			viewRedraw();
			break;


		case CmdLayerCol7:			
			chooseColor(6);
			viewRedraw();
			break;

		case CmdMovePoints		: 
			ptMoveSelToCurrentLayer( true );
			break;

		case CmdCopyPoints		: 
			ptCopySelToCurrentLayer( true );
			break;

		case CmdSetScope	: 
			switch (m_scope)
			{
			case 0: ptSetSelectionScope(0); break;
			case 1: setSceneScope(); break;
			case 2: setCloudScope(); break;
			}
			break;

		//case CmdSetScopeScene	: 
		//	setSceneScope();
		//	break;
	
		//case CmdSetScopeCloud	:
		//	setCloudScope();
		//	break;

		case CmdSelAll	:
			ptSelectAll();
			viewRedraw();
			break;

		case CmdSelCloud :
			selectCloudTest();
			viewRedraw();
			break;

		case CmdSelScene :
			selectSceneTest();
			viewRedraw();
			break;

		case CmdEvalMode :
			switch (m_evalMode)
			{
			case 0: ptSetEditWorkingMode(PT_EDIT_WORK_ON_VIEW); break;
			case 1: ptSetEditWorkingMode(PT_EDIT_WORK_ON_ALL); break;
			}
			break;

		case CmdLayersToChannel:
			layersToChannel();
			ptClearEdit();
			viewRedraw();
			break;

		case CmdChannelToLayers:
			channelToLayers();
			viewRedraw();
			break;

		case CmdResetSel:
			ptResetSelection();
			viewRedraw();
			break;

		case CmdLoadLayersFile:
			loadLayerChannels();
			break;

		case CmdSaveLayersFile:
			saveLayerChannels();
			break;

		case CmdLoadEditStack:
			loadEditStack();
			break;

		case CmdSaveEditStack:
			saveEditStack();
			break;

		case CmdCheckPointLayers:
			doesLayerHavePoints();
			break;

		case CmdSaveScopeLayersFile:
			saveLayerChannelsScope();
			break;

		case CmdLoadScopeLayersFile:
			loadLayerChannelsScope();
			break;
	}
}
//-----------------------------------------------------------------------------
void	EditTool::chooseColor( int layer )
//-----------------------------------------------------------------------------
{
	COLORREF col, res;
	PTfloat *curr = ptGetLayerColor( layer );

	col = RGB(curr[0]*255,curr[1]*255,curr[2]*255);

	if (VortexExampleApp::instance()->getUI().getColor( col, res ))
	{
		PTfloat new_col[] = { (float)GetRValue(res) / 255, (float)GetGValue(res) / 255, (float)GetBValue(res) / 255 };
		ptSetLayerColor( layer, new_col, 1.0f );
	}
}
//-----------------------------------------------------------------------------
bool	EditTool::onMouseButtonDown( int button, int x, int y )
//-----------------------------------------------------------------------------
{
	if (m_mode && m_mouse.eventStartX == -1)
	{
		m_mouse.eventStartX = x;
		m_mouse.eventStartY = y;
		m_mouse.lastX = x;
		m_mouse.lastY = y;
	
		if (m_mode==CmdBoxSelect)
		{	
			PTdouble pnt[3];
		
			if (ptFindNearestScreenPoint( 0, x, y, pnt ) >= 0)
			{
				m_boxUpper[0] = m_boxLower[0] = pnt[0];
				m_boxUpper[1] = m_boxLower[1] = pnt[1];
				m_boxUpper[2] = m_boxLower[2] = pnt[2];

				m_boxPosition[0] = 0;
				m_boxPosition[1] = 0;
				m_boxPosition[2] = 0;

				m_boxRotation[0] = 0;
				m_boxRotation[1] = 0;
				m_boxRotation[2] = 0;

				m_boxValid = true;
			}
			return true;
		}
	}
	return false;
}
//-----------------------------------------------------------------------------
bool	EditTool::onMouseButtonUp( int button, int x, int y )
//-----------------------------------------------------------------------------
{
	bool handled = false;

	switch (m_mode)
	{
	case CmdBrushSelect:
		if (button == MouseRightButton) m_mode = 0;
		break;

	case CmdRectangleSelect:

		if (m_mouse.eventStartX > 0)
		{
			/* make selection */ 
			double time;
			{
				pt::SimplePerformanceTimer t(time);

				float minX = min(m_mouse.eventStartX, x);
				float minY = min(m_mouse.eventStartY, y);

				float maxX = max(m_mouse.eventStartX, x);
				float maxY = max(m_mouse.eventStartY, y);

				ptSelectPointsByRect( minX, minY, maxX - minX, maxY - minY); 

			}
			char stat[64];
			sprintf(stat, "Rect selection took %0.1f ms", time);			
			addStatisticMessage(stat);

			m_mode = 0;
			m_mouse.eventStartX = -1;
			handled=true;
			
			viewRedraw();
		}
		break;

	case CmdPolygonSelect:

			if (button == MouseLeftButton)	
			{
				m_polygon.push_back( x );
				m_polygon.push_back( y );

				viewUpdate();
			}
			else if (button == MouseRightButton)
			{
				/* make selection */ 
				double time;
				{
					pt::SimplePerformanceTimer t(time);
					ptSelectPointsByFence( (PTint)m_polygon.size()/2, &m_polygon[0]); 
				}
				char stat[64];
				sprintf(stat, "Fence selection took %0.1f ms", time);
				addStatisticMessage(stat);
				m_mode = 0;
				handled=true;
				viewRedraw();
			}	
		break;

	case CmdBoxSelect:

		if (m_boxValid)
		{
			PTfloat lower[3];
			PTfloat upper[3];

			for (int i=0; i<3; i++)
			{
				if (m_boxLower[i] < m_boxUpper[i])
				{
					lower[i] = m_boxLower[i];
					upper[i] = m_boxUpper[i];
				}
				else
				{
					lower[i] = m_boxUpper[i];
					upper[i] = m_boxLower[i];
				}
			}

			m_boxLower[0] = lower[0];
			m_boxLower[1] = lower[1];
			m_boxLower[2] = lower[2];
			m_boxUpper[0] = upper[0];
			m_boxUpper[1] = upper[1];
			m_boxUpper[2] = upper[2];
														// Switch to oriented bounding box rotation mode
			m_mode = CmdRotateBox;
														// Switch box render color to red
			m_boxColor[0] = 1;
			m_boxColor[1] = 0;
			m_boxColor[2] = 0;
														// Calculate box center for rotation
			m_boxPosition[0] = (m_boxLower[0] + m_boxUpper[0]) * 0.5;
			m_boxPosition[1] = (m_boxLower[1] + m_boxUpper[1]) * 0.5;
			m_boxPosition[2] = (m_boxLower[2] + m_boxUpper[2]) * 0.5;

			m_boxRotation[0] = 0;
			m_boxRotation[1] = 0;
			m_boxRotation[2] = 0;

			viewUpdate();

			handled = true;
		}
		break;

	case CmdRotateBox:

		Tool::endDynamicView();

		m_mode = 0;
															// Get box's rotation on two axes
		float rx = m_boxRotation[0];
		float ry = m_boxRotation[1];
														// Convert degrees to radians
		rx = m_boxRotation[0] * static_cast<float>(PI) / 180.0f;
		ry = m_boxRotation[1] * static_cast<float>(PI) / 180.0f;

		PTdouble uAxis[3];
		PTdouble vAxis[3];
														// Calculate U and V local coordinate axes (X,Y)
														// This is based on rotation on X then Y axis ordering
		uAxis[0] = cosf(ry);
		uAxis[1] = 0;
		uAxis[2] = -sinf(ry);

		vAxis[0] = sinf(rx)*sinf(ry);
		vAxis[1] = cosf(rx);
		vAxis[2] = sinf(rx)*cosf(ry);

														// Calculate lower & upper as offsets from position at center of box
		double lupper[]	= {m_boxUpper[0] - m_boxPosition[0], m_boxUpper[1] - m_boxPosition[1], m_boxUpper[2] - m_boxPosition[2]};
		double llower[]	= {m_boxLower[0] - m_boxPosition[0], m_boxLower[1] - m_boxPosition[1], m_boxLower[2] - m_boxPosition[2]};

														// Select points using oriented bounding box
		int n = ptSelectPointsByOrientedBox(llower, lupper, m_boxPosition, uAxis, vAxis);

														// Select points using ptCreateOrientedBoxQuery()
		
														// Set box color back to white
		m_boxColor[0] = 1;
		m_boxColor[1] = 1;
		m_boxColor[2] = 1;

		viewRedraw();

		m_mode = 0;
		m_mouse.eventStartX=-1;

		handled = true;
		break;
	}

	return handled;
}
//-----------------------------------------------------------------------------
bool	EditTool::onMouseMove( int x, int y )
//-----------------------------------------------------------------------------
{
	if (m_mode == CmdPolygonSelect && m_polygon.size())
	{
		m_mouse.lastX = x;
		m_mouse.lastY = y;
		viewUpdate();

		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------
bool	EditTool::onMouseDrag( int x, int y, int startX, int startY )
//-----------------------------------------------------------------------------
{
	switch(m_mode)
	{
	case CmdRectangleSelect:
		if (m_mouse.eventStartX >= 0)
		{
			m_mouse.lastX = x;
			m_mouse.lastY = y;

			viewUpdate();
		}
		break;
	case CmdPolygonSelect:
		if (m_polygon.size() > 0)
		{
			m_mouse.lastX = x;
			m_mouse.lastY = y;
		}				
		break;
	case CmdBrushSelect:
	{
		PTdouble pnt[3];
		PTdouble dist = ptFindNearestScreenPoint(0,x,y,pnt);

		if (dist >= 0)
		{
			PTdouble sel[] = { pnt[0], pnt[1], pnt[2] };
			ptSelectPointsBySphere(sel, m_brushSize );

			viewUpdate();
		}
		break;
	}
	case CmdBoxSelect:
		PTdouble pnt[3];
		
		if (ptFindNearestScreenPoint( 0, x, y, pnt ) >= 0)
		{
			m_boxUpper[0] = pnt[0];
			m_boxUpper[1] = pnt[1];
			m_boxUpper[2] = pnt[2];
			m_boxValid = true;

			viewUpdate();
		}
		break;

	case CmdRotateBox:
		
		Tool::startDynamicView();

		m_boxRotation[0] = (x - m_mouse.eventStartX);
		m_boxRotation[1] = (y - m_mouse.eventStartY);			
			
		viewUpdate();

		break;

	default:
		return false;
	}
	return true;
}
//-----------------------------------------------------------------------------
void EditTool::setCurrentLayer(int layer)
//-----------------------------------------------------------------------------
{
	int lastCurrent = ptGetCurrentLayer();

	if (ptSetCurrentLayer( layer ))
	{
		if (!m_simple)
		{
			m_lyrButtons[lastCurrent]->set_back_col( &layerButtonCol );
			m_lyrButtons[layer]->set_back_col( &layerButtonSelCol );

			m_lyrButtons[lastCurrent]->redraw();
			m_lyrButtons[layer]->redraw();

			/* current layer will always be visible */ 
			m_lyrVis[layer]->set_back_col( &layerButtonSelCol );
		}
		VortexExampleApp::instance()->updateBoundingBox();
	}
}
//-----------------------------------------------------------------------------
void	EditTool::toggleLayerVisibility(int layer)
//-----------------------------------------------------------------------------
{
	bool vis = !ptIsLayerShown( layer );
	
	if (ptShowLayer( layer, vis ))
	{
		m_lyrVis[layer]->set_back_col( vis ? &layerButtonSelCol : &layerButtonCol );
		m_lyrVis[layer]->redraw();

		VortexExampleApp::instance()->updateBoundingBox();

		viewRedraw();
	}	
}
//-----------------------------------------------------------------------------
void	EditTool::toggleLayerLock(int layer)
//-----------------------------------------------------------------------------
{
	bool lock = !ptIsLayerLocked( layer );
	if (ptLockLayer( layer, lock ))
	{
		m_lyrLock[layer]->set_back_col( lock ? &layerButtonSelCol : &layerButtonCol );
		m_lyrLock[layer]->redraw();
	}
}
//-----------------------------------------------------------------------------
void	EditTool::setSceneScope()
//-----------------------------------------------------------------------------
{
	int numScenes = ptNumScenes();		
	if (numScenes)
	{
		PThandle *handles = new PThandle[numScenes];
		ptGetSceneHandles( handles );

		ptSetSelectionScope( handles[0] );
		delete [] handles;

		std::cout << "Selection scope set to Scene 1" << std::endl;
	}
	else
	{
		std::cout << "Can't set scope, nothing loaded" << std::endl;
	}	
}
//-----------------------------------------------------------------------------
void	EditTool::setCloudScope()
//-----------------------------------------------------------------------------
{
	int numScenes = ptNumScenes();		
	if (numScenes)
	{
		PThandle *handles = new PThandle[numScenes];
		ptGetSceneHandles( handles );

		ptSetSelectionScope( ptGetCloudHandleByIndex( handles[0], 0 ) );
		delete [] handles;

		std::cout << "Selection scope set to Scene 1, Cloud 1" << std::endl;
	}
	else
	{
		std::cout << "Can't set scope, nothing loaded" << std::endl;
	}
}
//-----------------------------------------------------------------------------
void	EditTool::selectCloudTest()
//-----------------------------------------------------------------------------
{
	int numScenes = ptNumScenes();		
	if (numScenes)
	{
		PThandle *handles = new PThandle[numScenes];
		ptGetSceneHandles( handles );

		ptSelectCloud( ptGetCloudHandleByIndex( handles[0], 0 ) );
		delete [] handles;

	}
	else
	{
		std::cout << "Can't select cloud, nothing loaded" << std::endl;
	}
}
//-----------------------------------------------------------------------------
void	EditTool::selectSceneTest()
//-----------------------------------------------------------------------------
{
	int numScenes = ptNumScenes();		
	if (numScenes)
	{
		PThandle *handles = new PThandle[numScenes];
		ptGetSceneHandles( handles );

		ptSelectScene( handles[0] );
		delete [] handles;
	}
	else
	{
		std::cout << "Can't select scene, nothing loaded" << std::endl;
	}
}

//-----------------------------------------------------------------------------
void	EditTool::layersToChannel()
//-----------------------------------------------------------------------------
{
	std::cout << "Creating channels from layers...";
	m_layersChannel = ptCreatePointChannelFromLayers( L"layers", 0 );
	std::cout << "Done\n";
}
//-----------------------------------------------------------------------------
void	EditTool::saveLayerChannels()
//-----------------------------------------------------------------------------
{
	UI &ui = VortexExampleApp::instance()->getUI();

	if (!m_layersChannel)
	{
		layersToChannel();
	}

	// save file picker
	const wchar_t *filename = ui.getSaveFilePath( L"Layer channels File", L"layers" );

	if (filename)
	{
		//PTuint64	PTAPI ptWriteChannelsFileToBuffer(PTint numChannels, const PThandle *channels, PTubyte *&buffer, PTuint64 &bufferSize);
		//( const PTstr filename, PTint numChannels, const PThandle *channels );
		if (ptWriteChannelsFile( filename, 1, &m_layersChannel) == PTV_SUCCESS)
		{
			std::cout << "Channels file written\n";
		}
		else 
		{
			std::cout << "Channels file failed to write\n";
		}
	}
}
//-----------------------------------------------------------------------------
void	EditTool::loadLayerChannels()
//-----------------------------------------------------------------------------
{
	UI &ui = VortexExampleApp::instance()->getUI();

	const PThandle *channels;
	PTint	 num_channels;

	//PTres		PTAPI ptReadChannelsFile( const PTstr filename, PTint &numChannels, const PThandle **channels );
	if (m_layersChannel)
	{
		ptDeletePointChannel( m_layersChannel );
	}
	// load file picker
	const wchar_t *filename = ui.getLoadFilePath( L"Layer channels File", L"layers" );

	if (filename)
	{	
		// load the channel file
		if (ptReadChannelsFile( filename, num_channels, &channels ) == PTV_SUCCESS && num_channels ==1 )
		{
			m_layersChannel = channels[0];
			channelToLayers();
			std::cout << "Channels file loaded and applied to layers\n";
		}
		else
		{
			std::cout << "Channels file failed to load\n";
		}	
	}
}
//-----------------------------------------------------------------------------
void	EditTool::loadLayerChannelsScope()
//-----------------------------------------------------------------------------
{
	
}
//-----------------------------------------------------------------------------
void	EditTool::saveLayerChannelsScope()
//-----------------------------------------------------------------------------
{
	UI &ui = VortexExampleApp::instance()->getUI();

	// get the first handle
	int numScenes = ptNumScenes();
	PThandle *handles = new PThandle[numScenes];
	ptGetSceneHandles( handles );

	// crate channel for POD 1 only
	std::cout << "Creating channels from layers for first POD file...";
	m_layersChannel = ptCreatePointChannelFromLayers( L"layers", handles[0] );
	std::cout << "Done\n";

	delete [] handles;

	// save file picker
	const wchar_t *filename = ui.getSaveFilePath( L"Layer channels File", L"layers" );

	if (filename)
	{
		//PTuint64	PTAPI ptWriteChannelsFileToBuffer(PTint numChannels, const PThandle *channels, PTubyte *&buffer, PTuint64 &bufferSize);
		//( const PTstr filename, PTint numChannels, const PThandle *channels );
		if (ptWriteChannelsFile( filename, 1, &m_layersChannel) == PTV_SUCCESS)
		{
			std::cout << "Channels file written\n";
		}
		else 
		{
			std::cout << "Channels file failed to write\n";
		}
	}
}
//-----------------------------------------------------------------------------
void	EditTool::saveEditStack()
//-----------------------------------------------------------------------------
{
	UI &ui = VortexExampleApp::instance()->getUI();

	// load file picker
	const wchar_t *filename = ui.getSaveFilePath( L"Edit stack file", L"edit" );

	if (filename)
	{
		FILE *file = _wfsopen( filename, L"wb", _SH_DENYNO );
		
		size_t stacksize = ptGetEditDataSize(0); 
		unsigned char* stackdata = new unsigned char[ stacksize ];
		ptGetEditData( 0, stackdata );

		if (file!=NULL)
		{
			fwrite( &stacksize, sizeof(stacksize), 1, file );
			fwrite( stackdata, stacksize, 1, file );
			fclose ( file );
		}
		delete [] stackdata;
	}
}

//-----------------------------------------------------------------------------
void	EditTool::loadEditStack()
//-----------------------------------------------------------------------------
{
	UI &ui = VortexExampleApp::instance()->getUI();

	// load file picker
	const wchar_t *filename = ui.getLoadFilePath( L"Edit stack file", L"edit" );

	if (filename)
	{
		size_t stacksize = 0;
		FILE *file = _wfsopen( filename, L"rb", _SH_DENYNO );
		
		if (file!=NULL)
		{
			fread( &stacksize, sizeof(stacksize), 1, file );
			if (stacksize < 5e8)
			{
				unsigned char *stackdata = new unsigned char[ stacksize ];
				fread( stackdata, stacksize, 1, file);
				fclose (file);

				ptCreateEditFromData( stackdata );
				delete [] stackdata;
			}
		}
	}
}
//-----------------------------------------------------------------------------
void	EditTool::channelToLayers()
//-----------------------------------------------------------------------------
{
	if (m_layersChannel) 
	{	
		ptLayersFromPointChannel( m_layersChannel, 0 );
		ptDeletePointChannel( m_layersChannel );

		m_layersChannel = 0;
	}
	viewRedraw();
}
//-----------------------------------------------------------------------------
void	EditTool::drawPostDisplay()
//-----------------------------------------------------------------------------
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );			

	glDisable(GL_LIGHTING);

	if (m_mode == CmdRectangleSelect || m_mode == CmdPolygonSelect)
	{
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();

		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D( tx, tx+tw, ty, ty+th);
		glDisable(GL_DEPTH_TEST);
		glDisable( GL_LIGHTING );

		glLineWidth(1.0f);
		glColor3f(0,1.0f,1.0f);
				
		if (m_mode == CmdRectangleSelect)
		{
			if (m_mouse.lastX > 0 && m_mouse.eventStartX > 0)
			{
				glBegin( GL_LINE_STRIP );
					glVertex2i(m_mouse.eventStartX, th-m_mouse.eventStartY);	
					glVertex2i(m_mouse.lastX, th-m_mouse.eventStartY);	
					glVertex2i(m_mouse.lastX, th-m_mouse.lastY);	
					glVertex2i(m_mouse.eventStartX, th-m_mouse.lastY);	
					glVertex2i(m_mouse.eventStartX, th-m_mouse.eventStartY);	
				glEnd();
			}
		}
		else if ( m_polygon.size() )	
		{
			glBegin( GL_LINE_STRIP );

			for (PTuint i=0;i<m_polygon.size();i+=2)
			{
				glVertex2i(m_polygon[i], th - m_polygon[i+1]);
			}
			if ( m_mouse.lastX > 0 )
				glVertex2i(m_mouse.lastX, th - m_mouse.lastY);

			glVertex2i(m_polygon[0], th - m_polygon[1]);	//close the polygon

			glEnd();
		}
		glEnable(GL_DEPTH_TEST);
		glPopMatrix();
		glMatrixMode( GL_MODELVIEW );
		glPopMatrix();
	}
	else if (m_boxValid)// && 
		//(m_mode == CmdBoxSelect || m_mode == CmdRotateBox))
	{
		glColor3fv(m_boxColor);
		VortexExampleApp::instance()->getView().
			drawBox( m_boxLower, m_boxUpper, m_boxPosition, m_boxRotation );
	}
}
//-----------------------------------------------------------------------------
void	EditTool::doesLayerHavePoints()
//-----------------------------------------------------------------------------
{
	//simple test to list layers with poitns
	for (int i=0; i<PT_EDIT_MAX_LAYERS; i++)
	{
		std::cout << "Layer " << i << ": "<< (ptDoesLayerHavePoints(i) ? "Y" : "-") << std::endl;
	}	
}
//-----------------------------------------------------------------------------
void EditTool::buildUserInterface(GLUI_Node *parent)
//-----------------------------------------------------------------------------
{
	GLUI_Rollout *rolloutSelect = new GLUI_Rollout(parent, "Regions / Editing", true );
	rolloutSelect->set_w( PANEL_WIDTH );

		GLUI_StaticText *spacer = new GLUI_StaticText( rolloutSelect, "Tools" );
		spacer->set_col( labelCol );

		GLUI_Panel *selectTools = new GLUI_Panel( rolloutSelect, " ", GLUI_PANEL_NONE);
		
		spacer = new GLUI_StaticText( rolloutSelect, " " );

		new GLUI_Button( selectTools, "Rectangle", CmdRectangleSelect, &Tool::dispatchCmd );
		new GLUI_Button( selectTools, "Brush", CmdBrushSelect, &Tool::dispatchCmd );
		new GLUI_Button( selectTools, "Deselect", CmdDeselect, &Tool::dispatchCmd );
		new GLUI_Button( selectTools, "Hide Selected", CmdSelectHide, &Tool::dispatchCmd );
		spacer = new GLUI_StaticText( selectTools, "" );
		new GLUI_Button( selectTools, "Reset", CmdResetEdit, &Tool::dispatchCmd );
		spacer = new GLUI_StaticText( selectTools, "" );
		new GLUI_Button( selectTools, "Reset Sel", CmdResetSel, &Tool::dispatchCmd );

		GLUI_Column* col = new GLUI_Column( selectTools, false );
		col->set_w(1);

		new GLUI_Button( selectTools, "Polygon", CmdPolygonSelect, &Tool::dispatchCmd );
		new GLUI_Button( selectTools, "Box", CmdBoxSelect, &Tool::dispatchCmd );
		new GLUI_Button( selectTools, "Invert Selected", CmdSelectInvert, &Tool::dispatchCmd );
		new GLUI_Button( selectTools, "Invert Visible", CmdVisbilityInvert, &Tool::dispatchCmd );
		spacer = new GLUI_StaticText( selectTools, "" );
		new GLUI_Button( selectTools, "Refresh Edit", CmdRefreshEdit, &Tool::dispatchCmd );
		spacer = new GLUI_StaticText( selectTools, "" );


		/* selection modes */ 
		GLUI_StaticText *lblm = new GLUI_StaticText( rolloutSelect, "Selection Mode" );
		lblm->set_col( labelCol );

		GLUI_RadioGroup *selectMode = new GLUI_RadioGroup( rolloutSelect, &m_selectMode, CmdSelectMode, &Tool::dispatchCmd );
		new GLUI_RadioButton( selectMode, "Select  " ); 

		new GLUI_Column( selectMode, false );
		new GLUI_RadioButton( selectMode, "Deselect " ); 

		new GLUI_Column( selectMode, false );
		new GLUI_RadioButton( selectMode, "Unhide  " ); 

		selectMode->set_selected(0);
	
		new GLUI_StaticText( selectTools, "Brush Size" );
		GLUI_Scrollbar* brush = new GLUI_Scrollbar( selectTools, "Brush Size", GLUI_SCROLL_HORIZONTAL, &m_brushSize, CmdBrushUpdate ,&Tool::dispatchCmd);
		brush->set_float_limits(0.01,2.0);

	if (!m_simple)
	{

		/* selection layers */  	
		const char *num [] = { "Layer 1", "Layer 2", "Layer 3", "Layer 4", "Layer 5", "Layer 6" };

		new GLUI_StaticText( rolloutSelect, " " );
		GLUI_StaticText *lbll = new GLUI_StaticText( rolloutSelect, "LAYERS" );
		lbll->set_col( labelCol );

		GLUI_StaticText *lbld = new GLUI_StaticText( rolloutSelect, "V=Visibility, L=Lock, S=Select, DS=Deselect" );
		lbld->set_col( labelCol );
		
		GLUI_Panel *selectLayers = new GLUI_Panel( rolloutSelect, " ", GLUI_PANEL_NONE);

		/* vis */ 
		int lyr;
		for (lyr =0; lyr < PT_EDIT_MAX_LAYERS; lyr++)
		{
			m_lyrVis[lyr] = new GLUI_Button( selectLayers, "V", CmdVisLayer1+lyr, &Tool::dispatchCmd );
			m_lyrVis[lyr]->set_back_col( lyr ? &layerButtonCol : &layerButtonSelCol );
			m_lyrVis[lyr]->set_w(8);
		}
		/* lock */ 
		new GLUI_Column( selectLayers, false );
		for (lyr =0; lyr < PT_EDIT_MAX_LAYERS; lyr++)
		{
			m_lyrLock[lyr] = new GLUI_Button( selectLayers, "L", CmdLockLayer1+lyr, &Tool::dispatchCmd );
			m_lyrLock[lyr]->set_back_col( &layerButtonCol );
			m_lyrLock[lyr]->set_w(8);
		}
		/*current */ 
		new GLUI_Column( selectLayers, false );
		for (int lyr=0;lyr<PT_EDIT_MAX_LAYERS;lyr++)
		{
			m_lyrButtons[lyr] = new GLUI_Button( selectLayers, num[lyr], CmdLayer1+lyr, &Tool::dispatchCmd );
			m_lyrButtons[lyr]->set_back_col( lyr ? &layerButtonCol : &layerButtonSelCol );
			m_lyrButtons[lyr]->set_w(100);
		}
		/*select */ 
		new GLUI_Column( selectLayers, false );
		for (int lyr=0;lyr<PT_EDIT_MAX_LAYERS;lyr++)
		{
			m_lyrSel[lyr] = new GLUI_Button( selectLayers, "S", CmdSelLayer1+lyr, &Tool::dispatchCmd );
			m_lyrSel[lyr]->set_back_col( &layerButtonCol );
			m_lyrSel[lyr]->set_w(8);
		}
		/*deselect */ 
		new GLUI_Column( selectLayers, false );
		for (int lyr=0;lyr<PT_EDIT_MAX_LAYERS;lyr++)
		{
			m_lyrSel[lyr] = new GLUI_Button( selectLayers, "DS", CmdDeselLayer1+lyr, &Tool::dispatchCmd );
			m_lyrSel[lyr]->set_back_col( &layerButtonCol );
			m_lyrSel[lyr]->set_w(8);
		}

		/*color */ 
		new GLUI_Column( selectLayers, false );
		for (int lyr=0;lyr<PT_EDIT_MAX_LAYERS;lyr++)
		{
			const PTfloat *col = ptGetLayerColor( lyr );
			
			m_lyrSel[lyr] = new GLUI_Button( selectLayers, " ", CmdLayerCol1+lyr, &Tool::dispatchCmd );
			if (col)
			{			
				static RGBc button_cols[PT_EDIT_MAX_LAYERS];
				button_cols[lyr] = RGBc(col[0]*255, col[1]*255, col[2]*255);

				m_lyrSel[lyr]->set_back_col( &button_cols[lyr] );
			}
			m_lyrSel[lyr]->set_w(8);
		}

		/* Copy / Move to layer */ 
		GLUI_Panel *selectCopy = new GLUI_Panel( rolloutSelect, " ", GLUI_PANEL_NONE);
		GLUI_Button *btn = new GLUI_Button( selectCopy, "Copy", CmdCopyPoints, &Tool::dispatchCmd );
		btn->set_back_col( &layerButtonCol );
		col = new GLUI_Column( selectCopy, false );
		btn = new GLUI_Button( selectCopy, "Move", CmdMovePoints, &Tool::dispatchCmd );
		btn->set_back_col( &layerButtonCol );

		btn = new GLUI_Button( selectCopy, "Occupancy", CmdCheckPointLayers, &Tool::dispatchCmd );
		btn->set_back_col( &layerButtonCol );

		/* Scope */ 
		GLUI_Panel *selectScope = new GLUI_Panel( rolloutSelect, " ", GLUI_PANEL_NONE);

		GLUI_StaticText *lbl = new GLUI_StaticText( rolloutSelect, "Editing Scope" );
		lbl->set_col( labelCol );

		GLUI_RadioGroup *scope = new GLUI_RadioGroup( rolloutSelect, &m_scope, CmdSetScope, &Tool::dispatchCmd );
		new GLUI_RadioButton( scope, "Global  " ); 

		new GLUI_Column( scope, false );
		new GLUI_RadioButton( scope, "Scene " ); 

		new GLUI_Column( scope, false );
		new GLUI_RadioButton( scope, "Cloud  " ); 

		scope->set_selected(0);

		/* Eval Mode */ 
		GLUI_Panel *evalMode = new GLUI_Panel( rolloutSelect, " ", GLUI_PANEL_NONE);

		GLUI_StaticText *lbl2 = new GLUI_StaticText( rolloutSelect, "Evaluation Mode" );
		lbl2->set_col( labelCol );

		GLUI_RadioGroup *eval = new GLUI_RadioGroup( rolloutSelect, &m_evalMode, CmdEvalMode, &Tool::dispatchCmd );
		new GLUI_RadioButton( eval, "View Based (Default) " ); 

		new GLUI_Column( eval, false );
		new GLUI_RadioButton( eval, "All (Slower)" ); 

		scope->set_selected(0);

		/* Selection tests */ 
		new GLUI_StaticText( rolloutSelect, " " );
		GLUI_StaticText *lbl3 = new GLUI_StaticText( rolloutSelect, "Selection Tests" );
		lbl3->set_col( labelCol );

		GLUI_Panel *selectTests = new GLUI_Panel( rolloutSelect, " ", GLUI_PANEL_NONE);

		btn = new GLUI_Button( selectTests, "ptSelectAll", CmdSelAll, &Tool::dispatchCmd );
		btn->set_back_col( &layerButtonCol );	
		btn->set_w(90);
		col = new GLUI_Column( selectTests, false );

		btn = new GLUI_Button( selectTests, "ptSelectScene", CmdSelScene, &Tool::dispatchCmd );
		btn->set_w(90);
		btn->set_back_col( &layerButtonCol );	

		col = new GLUI_Column( selectTests, false );

		btn = new GLUI_Button( selectTests, "ptSelectCloud", CmdSelCloud, &Tool::dispatchCmd );
		btn->set_w(90);
		btn->set_back_col( &layerButtonCol );

		/* Persitent tests */ 
		new GLUI_StaticText( rolloutSelect, " " );
		GLUI_StaticText *lbl4 = new GLUI_StaticText( rolloutSelect, "Persistence Tests" );
		lbl4->set_col( labelCol );
	
		GLUI_Panel *persistenceTests = new GLUI_Panel( rolloutSelect, " ", GLUI_PANEL_NONE);

		btn = new GLUI_Button( persistenceTests, "to Channel", CmdLayersToChannel, &Tool::dispatchCmd );
		btn->set_back_col( &layerButtonCol );	
		btn->set_w(90);
		col = new GLUI_Column( persistenceTests, false );

		btn = new GLUI_Button( persistenceTests, "from Channel", CmdChannelToLayers, &Tool::dispatchCmd );
		btn->set_w(90);
		btn->set_back_col( &layerButtonCol );	

		btn = new GLUI_Button( persistenceTests, "to Ch File", CmdSaveLayersFile, &Tool::dispatchCmd );
		btn->set_back_col( &layerButtonCol );	
		btn->set_w(90);
		col = new GLUI_Column( persistenceTests, false );

		btn = new GLUI_Button( persistenceTests, "from Ch File (S)", CmdLoadScopeLayersFile, &Tool::dispatchCmd );
		btn->set_w(90);
		btn->set_back_col( &layerButtonCol );

		btn = new GLUI_Button( persistenceTests, "to Ch File (S)", CmdSaveScopeLayersFile, &Tool::dispatchCmd );
		btn->set_back_col( &layerButtonCol );	
		btn->set_w(90);
		col = new GLUI_Column( persistenceTests, false );

		btn = new GLUI_Button( persistenceTests, "from Ch File", CmdLoadLayersFile, &Tool::dispatchCmd );
		btn->set_w(90);
		btn->set_back_col( &layerButtonCol );

		btn = new GLUI_Button( persistenceTests, "to Stack File", CmdSaveEditStack, &Tool::dispatchCmd );
		btn->set_back_col( &layerButtonCol );	
		btn->set_w(90);
		col = new GLUI_Column( persistenceTests, false );

		btn = new GLUI_Button( persistenceTests, "from Stack File", CmdLoadEditStack, &Tool::dispatchCmd );
		btn->set_w(90);
		btn->set_back_col( &layerButtonCol );
	}
}