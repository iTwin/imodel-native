/******************************************************************************

Pointools Vortex API Examples

ClassifChannelTest.cpp

Demonstrates basic client server operation

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#include "ClassifChannelTest.h"
#include "EditTool.h"
#include "PodWriterWrapper.h"

#include <math.h>

#define CHANNEL_BUFFER_SIZE			(int)5e6

//-----------------------------------------------------------------------------
ClassifChannelTest::ClassifChannelTest() 
: Tool(CmdLoadChannelsFile, CmdCloneHandle), m_queryBuffer(CHANNEL_BUFFER_SIZE)
//-----------------------------------------------------------------------------
{
	m_isOoc = 0;
	m_classifChannel = 0;
}

//-----------------------------------------------------------------------------
void ClassifChannelTest::buildUserInterface(GLUI_Node *parent)
//-----------------------------------------------------------------------------
{
	/* the test UI */ 
	GLUI_Rollout *rolloutTests = new GLUI_Rollout( parent, "Classification User Channels Test", true );
	rolloutTests->set_w( PANEL_WIDTH );

	GLUI_StaticText* spacer = new GLUI_StaticText( rolloutTests, "" );
	spacer->set_w( PANEL_WIDTH );

	spacer = new GLUI_StaticText( rolloutTests, "" );
	
	GLUI_Panel *edit = new GLUI_Panel( rolloutTests, "Set Sel Pts Class" );

	GLUI_Button *btns[10];

	btns[0] = new GLUI_Button( edit, "1", CmdSetClass1, &ClassifChannelTest::dispatchCmd );
	btns[1] = new GLUI_Button( edit, "4", CmdSetClass4, &ClassifChannelTest::dispatchCmd );
	btns[2] = new GLUI_Button( edit, "7", CmdSetClass7, &ClassifChannelTest::dispatchCmd );
	new GLUI_Column ( edit, false );

	btns[3] = new GLUI_Button( edit, "2", CmdSetClass2, &ClassifChannelTest::dispatchCmd );
	btns[4] = new GLUI_Button( edit, "5", CmdSetClass5, &ClassifChannelTest::dispatchCmd );
	btns[5] = new GLUI_Button( edit, "8", CmdSetClass8, &ClassifChannelTest::dispatchCmd );
	new GLUI_Column ( edit, false );

	btns[6] = new GLUI_Button( edit, "3", CmdSetClass3, &ClassifChannelTest::dispatchCmd );
	btns[7] = new GLUI_Button( edit, "6", CmdSetClass6, &ClassifChannelTest::dispatchCmd );
	btns[8] = new GLUI_Button( edit, "9", CmdSetClass9, &ClassifChannelTest::dispatchCmd );
	new GLUI_Column ( edit, false );

	for (int i=0; i<9; i++) btns[i]->set_w(25);

	new GLUI_Button( rolloutTests, "Save Classif User Channel", CmdSaveChannelsFile, &Tool::dispatchCmd  );
	new GLUI_Button( rolloutTests, "Load Classif User Channel", CmdLoadChannelsFile, &Tool::dispatchCmd  );
        new GLUI_Button( rolloutTests, "Destroy User Channel", CmdDestroyChannel, &Tool::dispatchCmd  );
        new GLUI_Button( rolloutTests, "Clone Handle", CmdCloneHandle, &Tool::dispatchCmd  );

	spacer = new GLUI_StaticText( rolloutTests, "" );

}
//-----------------------------------------------------------------------------
void ClassifChannelTest::command( int cmdId )
//-----------------------------------------------------------------------------
{
	switch (cmdId)
	{

	case CmdLoadChannelsFile:
		loadChannelsFile();
		break;

	case CmdSaveChannelsFile:
		saveChannelsFile();
		break;

	case CmdDestroyChannel:
		reset();
		break;
        case CmdCloneHandle:
            {
            PThandle *handles = new PThandle[ptNumScenes()];
            ptGetSceneHandles( handles );
            PThandle cloneHandle = ptCreateSceneInstance (handles[0]);

            ptRemoveScene (cloneHandle);
//			ptRemoveScene (handles[0]);

			delete [] handles;

            }
                break;

	default:
		if (cmdId >= CmdSetClass1 && cmdId <= CmdSetClass9)
		{
			setSelectedPtsClass(cmdId-CmdSetClass1+1);
		}
	}	
}
//-----------------------------------------------------------------------------
void ClassifChannelTest::reset()
//-----------------------------------------------------------------------------
{
	m_classifChannel = 0;
	m_renderer.setClassifUserChannel( 0 );
	ptDeleteAllChannels();
}
//-----------------------------------------------------------------------------
void ClassifChannelTest::setSelectedPtsClass(int classId)
//-----------------------------------------------------------------------------
{
	// if no channel exists, create one
	if (!m_classifChannel)
	{
		m_classifChannel = ptCreatePointChannel(L"classification", 
			/* byte size */1, /*multiple*/ 1, /* default val*/0, false /*ooc*/);

		VortexExampleApp::instance()->setRenderer( &m_renderer );	// show results using a query based renderer	
		m_renderer.setClassifUserChannel( m_classifChannel );
	}

	// query for points that are selected
	PThandle q = ptCreateSelPointsQuery();
	ptSetQueryDensity(q, PT_QUERY_DENSITY_FULL, 1.0f);

	QueryBufferf qb(1000000);
	qb.initialize();

	// iteratively run query until complete
	while (qb.executeQuery( q, m_classifChannel ))
	{		
		// get hannel buffer
		PTubyte *channelData = reinterpret_cast<PTubyte*>(qb.getChannelBuffer());

		if (channelData)
		{
			// update values
			for (int i=0; i<qb.numPntsInQueryIteration(); i++)
			{
				channelData[i] = (PTubyte)classId;
			}
			// submit changes
			ptSubmitPointChannelUpdate(q, m_classifChannel);
		}
	}

	ptDeleteQuery(q);
}
//-----------------------------------------------------------------------------
void ClassifChannelTest::loadChannelsFile()
//-----------------------------------------------------------------------------
{
	const wchar_t* loadFile = VortexExampleApp::instance()->getUI().
								getLoadFilePath( L"User channel file", L"classif");
	
	if (loadFile)
	{
		reset();
		
		const PThandle *chandles = 0;
		PTint numChannels;
		ptReadChannelsFile( loadFile, numChannels, &chandles );

		if (numChannels == 1)
		{
			m_classifChannel = chandles[0];

			VortexExampleApp::instance()->setRenderer( &m_renderer );	// show results using a query based renderer
			m_renderer.setClassifUserChannel( chandles[0] );
		}
		// in real use we should check the channel info to be sure 
		// these are correctly sized channels with the right name

		Tool::addStatisticMessage("Channels loaded from file");
	}
}
//-----------------------------------------------------------------------------
void ClassifChannelTest::saveChannelsFile()
//-----------------------------------------------------------------------------
{
	if (m_classifChannel)
	{
		const wchar_t* saveFile = VortexExampleApp::instance()->getUI()
			.getSaveFilePath( L"User channel file", L"classif");

		double ms=0;
		if (saveFile)
		{
			pt::SimplePerformanceTimer timer(ms);
			ptWriteChannelsFile( saveFile, 1, &m_classifChannel );
		}
		std::cout << "Write Channel file took " << ms << "ms" << std::endl;
	}
	else
	{
		std::cerr << "No channels to write" << std::endl;
	}
}
//-----------------------------------------------------------------------------
void ClassifChannelTest::destroyChannels()
//-----------------------------------------------------------------------------
{
	reset();	
}
