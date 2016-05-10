#include "stdafx.h"

/*  Make the class name into a global variable  */ 
char szClassName[ ] = "Pointools Vortex Monitor";

#include <ptappdll/ptapp.h>
#include "appFramework.h"

#include <ptui/ptScrollWin.h>

#include <pt/unicodeconversion.h>
#include <ptfs/filepath.h>

#include <iostream>
#include <ctype.h>
#include <string.h>
#include <conio.h>
#include <time.h>

#include <diagnostics/diagnostics.h>


// Interprocess support for PCG licenses
#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/ipc/message_queue.hpp>

#include <pt/utf.h> 
#include <ptui/ptgrid.h>

#include <fstream>

pt::Timer::TimeMs		g_startTime = 0;
int				g_lastQueryRow = 1;
int				g_lastLoadRow = 1;
int				g_lastVisRow = 1;

/* query */ 
#define PT_QUERY_DENSITY_FULL			0x01
#define	PT_QUERY_DENSITY_VIEW			0X02
#define PT_QUERY_DENSITY_LIMIT			0X03
#define	PT_QUERY_DENSITY_VIEW_COMPLETE	0X04
#define PT_QUERY_DENSITY_SPATIAL		0x07

//-----------------------------------------------------------------------------
// Fill grids with stats
//-----------------------------------------------------------------------------
void addQueryRow( ptdg::QueryData1 &query )
{
	ptui::Grid *grid = App::instance()->ui().queriesGrid();
	ptui::ScrollWindow *scroll = static_cast<ptui::ScrollWindow*>(grid->parent());

	if (g_lastQueryRow > 499)
	{
		g_lastQueryRow = 1;
		grid->clearGrid();
	}

	if (query.m_numPoints == 0) return;

	int timestamp = pt::Timer::instance()->delta_m( g_startTime, query.m_timeStamp );

    pt::Timer::TimeMs currTimestamp = pt::Timer::tick();
	int cts = pt::Timer::delta_m( g_startTime, currTimestamp );

	grid->insertItem(0, g_lastQueryRow, timestamp );
	grid->insertItem(1, g_lastQueryRow, (int)query.m_queryID );

	pt::String den;

	switch (query.m_density)
	{
	case PT_QUERY_DENSITY_FULL: den = "PT_QUERY_DENSITY_FULL"; break;
	case PT_QUERY_DENSITY_VIEW: den = "PT_QUERY_DENSITY_VIEW"; break;
	case PT_QUERY_DENSITY_LIMIT: den = "PT_QUERY_DENSITY_LIMIT"; break;
	case PT_QUERY_DENSITY_VIEW_COMPLETE: den = "PT_QUERY_DENSITY_VIEW_COMPLETE"; break;
	case PT_QUERY_DENSITY_SPATIAL: den = "PT_QUERY_DENSITY_SPATIAL"; break;
	}

	grid->insertItem(4, g_lastQueryRow, den );
	grid->insertItem(5, g_lastQueryRow, query.m_densityVal);
	grid->insertItem(6, g_lastQueryRow, query.m_bufferSize);

	ptui::OutputGraph *graph = App::instance()->ui().queryGraph();
	graph->addValue( 0, timestamp, query.m_numPoints );

	grid->insertItem(3, g_lastQueryRow, (int)query.m_numPoints);
	grid->insertItem(2, g_lastQueryRow, pt::String(query.m_queryType));

	fltk::Color bcol = 0;
	fltk::Color fcol = 0;

	// shading
	if (query.m_numPoints > 2e7 )
	{
		bcol = fltk::color(255,96,54);
		fcol = fltk::color(255,255,255);
	}
	else if (query.m_numPoints > 1e7)
	{
		bcol = fltk::color(255,136,56);
		fcol = fltk::color(255,255,255);
	}
	else if (query.m_numPoints > 5e6)
	{
		bcol = fltk::color(235,200,92);
		fcol = 0;
	}
	else if (query.m_numPoints > 1e6)
	{
		bcol = fltk::color(201,197,133);
		fcol = 0;
	}
	else
	{
		bcol = fltk::color(153,204,153);
		fcol = 0;
	}
	for (int clm=0;clm<8; clm++)
	{
		grid->cellBackColor(clm, g_lastQueryRow, bcol);
		grid->cellForeColor(clm, g_lastQueryRow, fcol);
	}
	++g_lastQueryRow;

	scroll->scrollToBottom();

	pt::String ts;
	ts.format("Timestamp: %d", cts);
	App::instance()->pmap().set("currTimestamp", ts);
	App::instance()->ui().update();
}
//-----------------------------------------------------------------------------
void addVisibilityRow( ptdg::VisibilityData1 &vis )
{
	ptui::Grid *grid = App::instance()->ui().visGrid();
	ptui::ScrollWindow *scroll = static_cast<ptui::ScrollWindow*>(grid->parent());

	if (g_lastVisRow > 499)
	{
		g_lastVisRow = 1;
		grid->clearGrid();
	}	
	int timestamp = pt::Timer::instance()->delta_m( g_startTime, vis.m_timeStamp );
	
	static int lastTimestamp = 0;

	pt::String str;
	if (timestamp > lastTimestamp)
	{
		grid->insertItem(0, g_lastVisRow, timestamp );
		grid->insertItem(1, g_lastVisRow, vis.m_numVoxelsInView);

		str.format( "%.1fm", (double)vis.m_numFullPtsInView / 1e6 );
		grid->insertItem(2, g_lastVisRow, str);
		
		str.format( "%.1fm", (double)vis.m_numLODPtsInView1 / 1e6 );
		grid->insertItem(3, g_lastVisRow, str);
		
		grid->insertItem(4, g_lastVisRow, vis.m_maxLOD);
		grid->insertItem(5, g_lastVisRow, vis.m_minLOD);
		grid->insertItem(6, g_lastVisRow, vis.m_averageLOD);

		str.format( "%.1fmpx", (double)vis.m_totalPxArea / 1e6 );
		grid->insertItem(7, g_lastVisRow, str);
		
		float overplot = vis.m_totalPxArea > 0 ? (int)vis.m_numLODPtsInView1 / vis.m_totalPxArea : 0;

		grid->insertItem(8, g_lastVisRow, overplot );

		ptui::OutputGraph *graph = App::instance()->ui().visGraph();
		graph->addValue( 0, timestamp, vis.m_numLODPtsInView1 );
		graph->addValue( 1, timestamp, overplot * 100000 );

		lastTimestamp = timestamp;
		scroll->scrollToBottom();
		++g_lastVisRow;
	}
}
//-----------------------------------------------------------------------------
void addLoadRow( ptdg::LoadData1 &load )
{
	ptui::Grid *grid = App::instance()->ui().loadGrid();
	ptui::ScrollWindow *scroll = static_cast<ptui::ScrollWindow*>(grid->parent());

	if (g_lastLoadRow > 499)
	{
		g_lastLoadRow = 1;
		grid->clearGrid();
	}

	int timestamp = pt::Timer::instance()->delta_m( g_startTime, load.m_timeStamp );

	static int lastTimestamp = 0;

	if (timestamp > lastTimestamp)
	{
		grid->insertItem(0, g_lastLoadRow, timestamp );
		grid->insertItem(1, g_lastLoadRow, (int)load.m_ptsShortfall);
		grid->insertItem(2, g_lastLoadRow, (int)load.m_ptsLoadedSinceDraw);

		ptui::OutputGraph *graph = App::instance()->ui().loadGraph();
		graph->addValue( 0, timestamp, load.m_ptsShortfall );
		graph->addValue( 1, timestamp, load.m_ptsLoadedSinceDraw );
		
		scroll->scrollToBottom();

		lastTimestamp = timestamp;
		++g_lastLoadRow;
	}
}
//-----------------------------------------------------------------------------
// Error message box
//-----------------------------------------------------------------------------
static void errorMessage(const char *title, const char *message)
{
	ptui::alertBox(title, message);
}


//-----------------------------------------------------------------------------
// func that handles feedback from UI system
//-----------------------------------------------------------------------------
ptapp::CommandPipe::Result MonitorInteractor::call(const char*cmd)
{
	if ( strcmp(cmd, "clearQueries") == 0 )
	{
		g_lastQueryRow = 1;
		App::instance()->ui().queriesGrid()->clearGrid();
		

	}
	else if ( strcmp(cmd, "clearVis") == 0 )
	{
		g_lastVisRow = 1;
		App::instance()->ui().visGrid()->clearGrid();

	}
	else if ( strcmp(cmd, "bookmarkVis") == 0 )
	{
		static int bm = 1;
		App::instance()->ui().visGrid()->insertItem(0,g_lastVisRow, bm++);
		App::instance()->ui().visGrid()->cellForeColor(0,g_lastVisRow, fltk::color(255,255,255));
		App::instance()->ui().visGrid()->cellBackColor(0,g_lastVisRow++, fltk::color(192,0,0));

	}
	else if ( strcmp(cmd, "clearShortfall") == 0 )
	{
		g_lastLoadRow = 1;
		App::instance()->ui().loadGrid()->clearGrid();

	}
	return ptapp::CommandPipe::CP_CMD_FOUND;	
}
static void checkMessageQueue( void * )
{
	using namespace boost::interprocess;

	static message_queue *mq = 0;
	
	if (!mq)
	{

	  try
	  {	
		  mq = new message_queue(open_only, "ptvortex" );
		
		  g_startTime = pt::Timer::instance()->tick();
	  }
	   catch(interprocess_exception &ex)
	   {
		  delete mq;
		  mq = 0;
		  message_queue::remove("ptvortex");
		  std::cout << ex.what() << std::endl;

		  // try again
		  fltk::add_timeout( 0.3f, checkMessageQueue );
		  return;	   
	   }
	}
   try
   {
		unsigned int priority;
		std::size_t recvd_size;

		ubyte block[ 128 ];

		for (int i=0; i<20; i++)
		{
			mq->try_receive( block, sizeof(block), recvd_size, priority);

			if (recvd_size != 0)
			{
				int type;
				memcpy( &type, block, sizeof(int) );

				switch( type )
				{
					case ptdg::eQueryMetric1:
					{
						ptdg::QueryData1 qd;
						memcpy( &qd, &block[sizeof(int)], sizeof(ptdg::QueryData1) );

						addQueryRow( qd );
						break;
					}
					case ptdg::eLoadMetric1:
					{
						ptdg::LoadData1 ld;
						memcpy( &ld, &block[sizeof(int)], sizeof(ptdg::LoadData1) );
						addLoadRow( ld );
						break;
					}
					case ptdg::eVisibilityMetric1:
					{
						ptdg::VisibilityData1 vd;
						memcpy( &vd, &block[sizeof(int)], sizeof(ptdg::VisibilityData1) );

						addVisibilityRow( vd );
					}
				}
			}
			else break;
		}
		
	}
   catch(interprocess_exception &ex)
   {
	  std::cout << ex.what() << std::endl;
   }

   fltk::add_timeout( 0.3f, checkMessageQueue );
}

//=============================================================================
//	MAIN ENTRY POINT
//=============================================================================
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	
	bool res = App::instance()->initialiseApp();

	if (!res)
	{
		fltk::alert("Initialisation failure");		
		exit(0);
	}

	fltk::add_timeout( 1.0f, checkMessageQueue );

	//App::instance()->ui().reqGrid()->selectCallback( onSelectLicReqCust );
	App::instance()->run();
 
	using namespace boost::interprocess;
 
	message_queue::remove("ptvortex");
    return 0; 
}
