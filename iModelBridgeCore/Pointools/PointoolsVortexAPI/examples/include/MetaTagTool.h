/******************************************************************************

Pointools Vortex API Examples

MetaTagTool.h

Demonstrates reading and setting metatags 

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_METATAG_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_METATAG_TOOL_H_

#include "VortexExampleApp.h"
#include <string>

class MetaTagTool : public Tool
{
public:
	enum
	{
		CmdMetaTagShow = 800,
		CmdMetaTagSet = 801,
		CmdMetaTagSave = 802
	};

	MetaTagTool();
	
	void	buildUserInterface(GLUI_Node *parent);
	void	command( int cmdId );

private:
	void	showMetaTag( void );
	void	setMetaTag( void );
	void	saveMetaTagEdits( void );
	
	std::string		m_currentTag;
	
	PThandle		m_metaDataHandle;

	GLUI_TextBox	*m_editTag;
	GLUI_List		*m_listBox;
};

#endif

