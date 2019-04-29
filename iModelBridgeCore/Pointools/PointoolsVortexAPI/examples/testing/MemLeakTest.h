/******************************************************************************

Pointools Vortex API Examples

MemLeakTest.h

Test specific mem leak issue - not for distribution

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_INTERNAL_MEM_LEAK
#define POINTOOLS_EXAMPLE_INTERNAL_MEM_LEAK

#include "VortexExampleApp.h"

class MemLeakTest : public Tool
{
public:
	enum
	{
		CmdTestLeak = 3099
	};

	MemLeakTest() : Tool(CmdTestLeak, CmdTestLeak) {}
	
	void	buildUserInterface(GLUI_Node *parent);
	void	command( int cmdId );

private:
};

#endif

