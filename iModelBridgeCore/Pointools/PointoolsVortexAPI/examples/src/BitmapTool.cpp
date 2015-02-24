/******************************************************************************

Pointools Vortex API Examples

BitmapTool.cpp

Demonstrates creating a Bitmap Viewport and saving a screenshot

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "BitmapTool.h"

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>
#include <shellapi.h>


//-----------------------------------------------------------------------------
BitmapTool::BitmapTool() : Tool(CmdCreateBitmap, CmdCreateBitmap)
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
void BitmapTool::buildUserInterface( GLUI_Node *parent )
//-----------------------------------------------------------------------------
{

	GLUI_Rollout *rollout = new GLUI_Rollout(parent, "Bitmap", true );

	rollout->set_w( PANEL_WIDTH );

	GLUI_StaticText *spacer = new GLUI_StaticText( rollout, "" );
	spacer->set_w( PANEL_WIDTH );

	GLUI_Panel *pan = new GLUI_Panel( rollout, " ", GLUI_PANEL_NONE);

	new GLUI_Button( pan, "Make Bitmap", CmdCreateBitmap, &Tool::dispatchCmd );
}
//-----------------------------------------------------------------------------
void BitmapTool::command( int cmdId )
//-----------------------------------------------------------------------------
{
	if (cmdId==CmdCreateBitmap)
		createBitmap();
}
//-----------------------------------------------------------------------------
void BitmapTool::createBitmap()
//-----------------------------------------------------------------------------
{
	UI &ui = VortexExampleApp::instance()->getUI();

	extern void WriteBMPFile(HBITMAP bitmap, const wchar_t *filename, HDC hDC);

	/* create a bitmap viewport - returns the HBITMAP*/ 
	void *bmpViewport = ptCreateBitmapViewport( 1024, 1024 / ui.xy_aspect, L"Bitmap");

	if (bmpViewport)
	{
		HBITMAP hBmp = reinterpret_cast<HBITMAP>(bmpViewport);

		/* Set this viewport as current */ 
		ptSetViewportByName( L"Bitmap" );

		/* setup the GL view and draw the scene */ 
		VortexExampleApp::instance()->glutDisplay();

		ptSetViewport(0);

		const wchar_t *filename = ui.getSaveFilePath( L"Windows Bitmap File", L"bmp" );

		if (filename)
		{
			/* save the file */ 
			HDC hDC = CreateCompatibleDC( NULL );
			SelectObject( hDC, hBmp ); 
			WriteBMPFile( hBmp, filename, hDC );

			/* clean up */ 
			DeleteDC( hDC );
		}
		ptDestroyBitmapViewport( L"Bitmap" );

		/* take a look at the file */ 
		ShellExecute( NULL, _T("open"), filename, NULL, NULL, SW_SHOW);
	}
}