/******************************************************************************

Pointools Vortex API Examples

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/

#include "MemLeakTest.h"

//-----------------------------------------------------------------------------
void MemLeakTest::command( int cmdId )
//-----------------------------------------------------------------------------
{
#define TEST_ARRAY_SIZE 500000

	double *pts = new double[TEST_ARRAY_SIZE*3];

	// run the short test
	int nbOfLoops (5000); // one million
    
	for (int i = 0; i < nbOfLoops; i++)
	{
        PThandle query = ptCreateVisPointsQuery ();
        //ptSetQueryScope(query, podHandle);
        //ptSetQueryDensity (query, 0x01, .1f);

        // will leak
        int pointsRead2 = ptGetDetailedQueryPointsd( query, TEST_ARRAY_SIZE, pts, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL);

        // will not leak
        //int pointsRead2 = ptGetQueryPointsd( query, vectorSize, &pointVector[0], NULL, NULL, NULL, NULL);
        ptDeleteQuery(query);
	}

	delete [] pts;
}
//-----------------------------------------------------------------------------
void MemLeakTest::buildUserInterface(GLUI_Node *parent)
//-----------------------------------------------------------------------------
{
	GLUI_Rollout *rolloutTest = new GLUI_Rollout( parent, "Tests", true );

	rolloutTest->set_w( PANEL_WIDTH );

	GLUI_StaticText *spacer = new GLUI_StaticText( rolloutTest, "" );
	spacer->set_w( PANEL_WIDTH );

	GLUI_Panel *pan = new GLUI_Panel( rolloutTest, " ", GLUI_PANEL_NONE);

	new GLUI_Button( pan, "Test HF Queries", CmdTestLeak, &Tool::dispatchCmd  );
	
	new GLUI_StaticText( rolloutTest, "" );
}