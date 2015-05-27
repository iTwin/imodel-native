/******************************************************************************

Pointools Vortex API Examples

ChannelsTest.cpp

Demonstrates basic client server operation

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "ChannelsTest.h"
#include "EditTool.h"
#include "PodWriterWrapper.h"

#include <math.h>

#define CHANNEL_BUFFER_SIZE			(int)5e6
#define PI 3.14159265

//-----------------------------------------------------------------------------
ChannelsTest::ChannelsTest() 
: Tool(CmdDoChannelsTest, CmdBoxRotate), m_queryBuffer(CHANNEL_BUFFER_SIZE)
//-----------------------------------------------------------------------------
{
	m_isOoc = 0;
	m_isCopyOoc = 0;
	m_effectMixer = 1.0f;
	m_colChannel = 0;
	m_offChannel = 0;
	copyCounter = 0;
}

//-----------------------------------------------------------------------------
void ChannelsTest::buildUserInterface(GLUI_Node *parent)
//-----------------------------------------------------------------------------
{
	/* the test UI */ 
	GLUI_Rollout *rolloutTests = new GLUI_Rollout( parent, "User Channels Test", true );
	rolloutTests->set_w( PANEL_WIDTH );

	GLUI_StaticText* spacer = new GLUI_StaticText( rolloutTests, "" );
	spacer->set_w( PANEL_WIDTH );

	GLUI_Panel *edit = new GLUI_Panel( rolloutTests, 
		"1. Select Points (Use Edit tools)                         " );

	spacer = new GLUI_StaticText( rolloutTests, "" );

	GLUI_Panel *channel = new GLUI_Panel( rolloutTests, 
		"2. Create and update User Channel     " );

	new GLUI_Checkbox( channel, "Create channel Out of Core", &m_isOoc);

	new GLUI_Button( channel, "Create + Apply Values", CmdDoChannelsTest, &Tool::dispatchCmd );

	GLUI_Scrollbar *mixer;
	m_effectMixer = 1.0f;
	mixer = new GLUI_Scrollbar( channel, "Mix", GLUI_SCROLL_HORIZONTAL, &m_effectMixer, CmdUpdateMixer , &Tool::dispatchCmd );
	mixer->set_float_limits(0,2);

	new GLUI_Checkbox(channel, "Copy channel Out of Core", &m_isCopyOoc);
	new GLUI_Button(channel, "Copy User Channel", CmdCopyChannelsFile, &Tool::dispatchCmd  );


	spacer = new GLUI_StaticText( rolloutTests, "" );

	GLUI_Panel *pers = new GLUI_Panel( rolloutTests, 
		"3. Persistence                                " );

	spacer = new GLUI_StaticText( pers, "" );

	new GLUI_Button( pers, "Save User Channel", CmdSaveChannelsFile, &Tool::dispatchCmd  );
	new GLUI_Button( pers, "Load User Channel", CmdLoadChannelsFile, &Tool::dispatchCmd  );
	new GLUI_Button( pers, "Copy User Channel", CmdCopyChannelsFile, &Tool::dispatchCmd  );
	new GLUI_Button( pers, "Write/Read User Channel Buffer", CmdWriteReadChannelsFileBuffer, &Tool::dispatchCmd  );

	new GLUI_Button( pers, "Save POD file", CmdSavePODFile, &Tool::dispatchCmd  );

	new GLUI_Button( rolloutTests, 
		"4. Destroy User Channel                     ", CmdDestroyChannel, &Tool::dispatchCmd  );
	spacer = new GLUI_StaticText( rolloutTests, "" );

	new GLUI_Button( rolloutTests,
		"Test OBB Query/Channels", CmdBoxSelect, &Tool::dispatchCmd );
}

//-----------------------------------------------------------------------------
bool	ChannelsTest::onMouseButtonDown( int button, int x, int y )
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
				m_boxUpper[0] = m_boxLower[0] = (float) pnt[0];
				m_boxUpper[1] = m_boxLower[1] = (float) pnt[1];
				m_boxUpper[2] = m_boxLower[2] = (float) pnt[2];

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
bool ChannelsTest::onMouseDrag(int x, int y, int startX, int startY)
//-----------------------------------------------------------------------------
{
	switch (m_mode)
	{
	case CmdBoxSelect:
		PTdouble pnt[3];
		
		if (ptFindNearestScreenPoint( 0, x, y, pnt ) >= 0)
		{
			m_boxUpper[0] = (float) pnt[0];
			m_boxUpper[1] = (float) pnt[1];
			m_boxUpper[2] = (float) pnt[2];
			m_boxValid = true;

			viewUpdate();
		}
		break;

	case CmdBoxRotate:
		
		Tool::startDynamicView();

		m_boxRotation[0] = (float) (x - m_mouse.eventStartX);
		m_boxRotation[1] = (float) (y - m_mouse.eventStartY);			
			
		viewUpdate();

		break;

	default:
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
void	ChannelsTest::drawPostDisplay()
//-----------------------------------------------------------------------------
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );			

	glDisable(GL_LIGHTING);

	if (m_boxValid)
		//(m_mode == CmdBoxSelect || m_mode == CmdBoxRotate))
	{
		glColor3fv(m_boxColor);
		VortexExampleApp::instance()->getView().drawBox( m_boxLower, m_boxUpper, m_boxPosition, m_boxRotation );
	}
}
//-----------------------------------------------------------------------------
bool ChannelsTest::onMouseButtonUp(int button, int x, int y)
//-----------------------------------------------------------------------------
{
	bool handled = false;

	switch (m_mode)
	{
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
			m_mode = CmdBoxRotate;
														// Switch box render color to red
			m_boxColor[0] = 1;
			m_boxColor[1] = 0;
			m_boxColor[2] = 0;
														// Calculate box center for rotation
			m_boxPosition[0] = (m_boxLower[0] + m_boxUpper[0]) * 0.5f;
			m_boxPosition[1] = (m_boxLower[1] + m_boxUpper[1]) * 0.5f;
			m_boxPosition[2] = (m_boxLower[2] + m_boxUpper[2]) * 0.5f;

			m_boxRotation[0] = 0;
			m_boxRotation[1] = 0;
			m_boxRotation[2] = 0;

			viewUpdate();

			handled = true;
		}
		break;

	case CmdBoxRotate:

		Tool::endDynamicView();

		m_mode = 0;
															// Get box's rotation on two axes
		float rx = m_boxRotation[0];
		float ry = m_boxRotation[1];
														// Convert degrees to radians
		rx = m_boxRotation[0] * static_cast<float>(PI) / 180.0f;
		ry = m_boxRotation[1] * static_cast<float>(PI) / 180.0f;

		float uAxis[3];
		float vAxis[3];
														// Calculate U and V local coordinate axes (X,Y)
														// This is based on rotation on X then Y axis ordering
		uAxis[0] = cosf(ry);
		uAxis[1] = 0;
		uAxis[2] = -sinf(ry);

		vAxis[0] = sinf(rx)*sinf(ry);
		vAxis[1] = cosf(rx);
		vAxis[2] = sinf(rx)*cosf(ry);

														// Calculate lower & upper as offsets from position at center of box
		float lupper[]	= {m_boxUpper[0] - m_boxPosition[0], m_boxUpper[1] - m_boxPosition[1], m_boxUpper[2] - m_boxPosition[2]};
		float llower[]	= {m_boxLower[0] - m_boxPosition[0], m_boxLower[1] - m_boxPosition[1], m_boxLower[2] - m_boxPosition[2]};

		PThandle q = ptCreateOrientedBoundingBoxQuery( llower[0], llower[1], llower[2],
														lupper[0], lupper[1], lupper[2],
														m_boxPosition[0], m_boxPosition[1], m_boxPosition[2],
														uAxis[0], uAxis[1], uAxis[2], 
														vAxis[0], vAxis[1], vAxis[2]);

		doChannelsTest(q);
														// Assign channel values
		
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
void ChannelsTest::command( int cmdId )
//-----------------------------------------------------------------------------
{
	switch (cmdId)
	{

	case CmdBoxSelect:
		m_mode = CmdBoxSelect;
		m_mouse.eventStartX = -1;
		break;

	case CmdDoChannelsTest:
		doChannelsTestOnSelected();
		break;

	case CmdLoadChannelsFile:
		loadChannelsFile();
		updateChannelsRenderer();
		break;

	case CmdCopyChannelsFile:
		copyPointChannels();
		updateChannelsRenderer();
		break;

	case CmdWriteReadChannelsFileBuffer:
		writeReadPointChannelsBuffer();
		updateChannelsRenderer();
		break;

	case CmdSaveChannelsFile:
		saveChannelsFile();
		break;

	case CmdDestroyChannel:
		reset();
		break;

	case CmdUpdateMixer:
		updateChannelsRenderer();
		Tool::viewUpdate();
		break;

	case CmdSavePODFile:
		savePOD();
		break;
	}	
}
//-----------------------------------------------------------------------------
void ChannelsTest::updateChannelsRenderer()
//-----------------------------------------------------------------------------
{
	m_renderer.setMixer(m_effectMixer);
	m_renderer.setChannels(m_offChannel,m_colChannel);
}
//-----------------------------------------------------------------------------
void ChannelsTest::savePOD()
//-----------------------------------------------------------------------------
{
	// save results to Pod
	if (m_offChannel)
	{
		POD_Writer pod;
		if (!pod.initialize(false))
		{
			std::cout << "Failed to load PodWriter.dll" << std::endl;
			return;
		}
		
		PThandle query = ptCreateVisPointsQuery();		// all points that are visible
		QueryBufferf queryBuffer((int) 1e6);				// extract into buffer
		queryBuffer.executeQuery(query, m_offChannel);

		/* scene information */ 
		int numScenes = ptNumScenes();					// need some scene info to get a filename
		if (!numScenes) return;	// nothing to save

		PThandle *sceneHandles = new PThandle[numScenes];
		ptGetSceneHandles(sceneHandles);

		wchar_t scName[64];
		wchar_t scOrigFilename[MAX_PATH];
		wchar_t scNewFilename[MAX_PATH];
		PTint numClouds;
		PTuint numPoints;
		PTuint spec;
		PTbool loaded;
		PTbool visible;
		PTdouble coordBase[3];
		
		ptGetCoordinateBase( coordBase );	//get offset to world coords
		ptSceneInfo( sceneHandles[0], scName, numClouds, numPoints, spec, loaded, visible );
		wcsncpy( scOrigFilename, ptSceneFile( sceneHandles[0] ), MAX_PATH );

		// remove extension 
		scOrigFilename[ wcslen(scOrigFilename) - 4 ] = '\0';

		// add amendment to filename 
		swprintf( scNewFilename, L"%s%s.pod", scOrigFilename, "-modified" );
		pod.startPODFile( scNewFilename );
	
		// iterate points and save
		while(queryBuffer.numPntsInQueryIteration())
		{
			const float *pnts = queryBuffer.getPointsBuffer();
			float *offsets = reinterpret_cast<float *>(queryBuffer.getChannelBuffer(0));

			for (int i=0; i<queryBuffer.numPntsInQueryIteration(); i++)
			{	
				
				double wpnt[] =
				{
					pnts[i*3] + coordBase[0] + offsets[i*3],
					pnts[i*3+1] + coordBase[1] + offsets[i*3+1],
					pnts[i*3+2] + coordBase[2] + offsets[i*3+2] 
				};

				pod.addPoint( wpnt, queryBuffer.getRGB(i), 0/* just a demo*/, 0, 0);
			}
			queryBuffer.executeQuery(query, m_offChannel);
		}
		delete [] sceneHandles;

		pod.endPODFile();
	}
}
//-----------------------------------------------------------------------------
void ChannelsTest::doChannelsTestOnOrientedBox()
//-----------------------------------------------------------------------------
{
	PThandle q = ptCreateSelPointsQuery();
	doChannelsTest( q );
	ptDeleteQuery( q );
}
//-----------------------------------------------------------------------------
void ChannelsTest::doChannelsTestOnSelected()
//-----------------------------------------------------------------------------
{
	PThandle q = ptCreateSelPointsQuery();
	doChannelsTest( q );
	ptDeleteQuery( q );
}
//-----------------------------------------------------------------------------
void ChannelsTest::doChannelsTest(PThandle queryHandle)
//-----------------------------------------------------------------------------
{
	/* allocate the points buffer array to receive the points to modify */ 
	m_queryBuffer.initialize(false, false);

	/* create a user channel that store the offset for each point */ 
	if (!m_offChannel)
	{
		/* per component default value */ 
		float default_value [] = { 0,0,0 };
		unsigned char white [] = { 255,255,255 };

		/* create the user channel with 4 bytes per point (single precision float) and 
		/* 3 components per point for x, y and z ; note that name "offset" is arbitary but must be unique by channel*/  
		m_colChannel = ptCreatePointChannel( L"colour", 1, 3, white, m_isOoc ? PT_CHANNEL_OUT_OF_CORE : 0 ); 
		m_offChannel = ptCreatePointChannel( L"offset", 4, 3, default_value, m_isOoc ? PT_CHANNEL_OUT_OF_CORE : 0 );
		
		/* channel buffers argument, can be used for multiple buffers for multiple user channels */ 
		PThandle channelHandles [] = { m_offChannel, m_colChannel };

		/* create a selected points query */ 
		PThandle q = queryHandle;//

		/* For quick test uncomment the following line, this will perform the query at view density */ 
		//ptSetQueryDensity( q, PT_QUERY_DENSITY_VIEW, 1.0f );
		ptSetQueryDensity( q, PT_QUERY_DENSITY_FULL, 1.0f );

		/* run the query */ 
		int num_points = m_queryBuffer.executeQuery( q, channelHandles, 2 );

		/* extract the query points and update the user offset channel */ 
		/* this is done in iterations with up to 5e6 points extracted per iteration */ 
		do 
		{
			PTfloat no_offset [] = {0,0,0};	
			PTfloat offset[3];
			PTubyte col[3];

			for (int i = 0; i<num_points; i++)
			{
				const float *pnt = m_queryBuffer.getPoint(i);

				// generate a random offset value
				offset[2] = sin((pnt[0] + pnt[1])/ 3.0f); 
				offset[1] = cos((pnt[0] + pnt[1])/ 5.0f)*2.0f; 
				offset[0] = sin(pnt[0] / 7.0f) * 4.0f; 
				
				// a corresponding color
				col[0] = (PTubyte)fmod(offset[0] * 255, 255);
				col[1] = (PTubyte)fmod(offset[1] * 255, 255);
				col[2] = (PTubyte)fmod(offset[2] * 255, 255);

				// update the buffer with new values
				m_queryBuffer.setChannelValue( 0, i, no_offset ); // CHANGE BACK TO 'offset'
				m_queryBuffer.setChannelValue( 1, i, col );

			}
			/* transfer the temp user channel buffer values to the actual channel */ 
			ptSubmitPointChannelUpdate( q, m_offChannel );
			ptSubmitPointChannelUpdate( q, m_colChannel );

			num_points = m_queryBuffer.executeQuery( q, channelHandles, 2 );
		}
		/* num_points = 0 if all the points have been extracted */ 
		while (num_points);
	}
	ptClearEdit();

	Tool::viewRedraw();

	updateChannelsRenderer();
	
	VortexExampleApp::instance()->setRenderer( &m_renderer );	// show results using a query based renderer
}
//-----------------------------------------------------------------------------
void ChannelsTest::reset()
//-----------------------------------------------------------------------------
{
	m_renderer.setChannels(0,0);	// clear existing channels
	m_colChannel = 0;
	m_offChannel = 0;
	ptDeleteAllChannels();
}

//-----------------------------------------------------------------------------
void ChannelsTest::writeReadPointChannelsBuffer(void)
//-----------------------------------------------------------------------------
{
	if(m_colChannel == NULL || m_offChannel == NULL)
	{
		return;
	}

	PThandle			channels[2];
	PTubyte			*	buffer;
	PTuint64			bufferSize;
	const PThandle *	channelsNew = NULL;
	PTint				numChannelsNew = 0;

	channels[0] = m_colChannel;
	channels[1] = m_offChannel;

	PTuint64 bufferHandle = ptWriteChannelsFileToBuffer(2, channels, buffer, bufferSize);

	ptReadChannelsFileFromBuffer(buffer, bufferSize, numChannelsNew, &channelsNew);

	if(channelsNew == NULL || numChannelsNew != 2)
	{
		Tool::addStatisticMessage("Error occured reading channels buffer");
		return;
	}

	ptReleaseChannelsFileBuffer(bufferHandle);

	ptDeletePointChannel(m_colChannel);
	ptDeletePointChannel(m_offChannel);

	m_colChannel = channelsNew[0];
	m_offChannel = channelsNew[1];

	Tool::addStatisticMessage("Channels read/write");

}

//-----------------------------------------------------------------------------
void ChannelsTest::copyPointChannels(void)
//-----------------------------------------------------------------------------
{
	if(m_colChannel == NULL || m_offChannel == NULL)
	{
		return;
	}


	++copyCounter;

	wchar_t newColName[256];
	wchar_t newOffName[256];

	wsprintf(newColName, L"color_%d", copyCounter);
	wsprintf(newOffName, L"offset_%d", copyCounter);

	PThandle copiedChannel[2];


	copiedChannel[0] = ptCopyPointChannel(m_colChannel, newColName, m_isCopyOoc);
	copiedChannel[1] = ptCopyPointChannel(m_offChannel, newOffName, m_isCopyOoc);

	ptDeletePointChannel(m_colChannel);
	ptDeletePointChannel(m_offChannel);

	m_colChannel = copiedChannel[0];
	m_offChannel = copiedChannel[1];

	updateChannelsRenderer();	
	VortexExampleApp::instance()->setRenderer( &m_renderer );	// show results using a query based renderer

	Tool::addStatisticMessage("Channels copied");
}

//-----------------------------------------------------------------------------
void ChannelsTest::loadChannelsFile()
//-----------------------------------------------------------------------------
{
	const wchar_t* loadFile = VortexExampleApp::instance()->getUI().
								getLoadFilePath( L"User channel file", L"uch");
	
	if (loadFile)
	{
		reset();
		
		const PThandle *chandles = 0;
		PTint numChannels;
		ptReadChannelsFile( loadFile, numChannels, &chandles );

		if (numChannels == 2)
		{
			m_colChannel = chandles[0];
			m_offChannel = chandles[1];

			updateChannelsRenderer();	
			VortexExampleApp::instance()->setRenderer( &m_renderer );	// show results using a query based renderer
		}
		// in real use we should check the channel info to be sure 
		// these are correctly sized channels with the right name

		Tool::addStatisticMessage("Channels loaded from file");
	}
}
//-----------------------------------------------------------------------------
void ChannelsTest::saveChannelsFile()
//-----------------------------------------------------------------------------
{
	if (m_offChannel)
	{
		const wchar_t* saveFile = VortexExampleApp::instance()->getUI()
			.getSaveFilePath( L"User channel file", L"uch");

		double ms=0;
		if (saveFile)
		{
			pt::SimplePerformanceTimer timer(ms);
			// note that channel handles are expected to be in an array
			// following is ok since member alignment is as declared in class
			// ** for test code, not production code! compiler could play nasty tricks **		
			ptWriteChannelsFile( saveFile, 2, &m_colChannel );
		}
		std::cout << "Write Channel file took " << ms << "ms" << std::endl;
	}
	else
	{
		std::cerr << "No channels to write" << std::endl;
	}
}
//-----------------------------------------------------------------------------
void ChannelsTest::destroyChannels()
//-----------------------------------------------------------------------------
{
	reset();	
}
