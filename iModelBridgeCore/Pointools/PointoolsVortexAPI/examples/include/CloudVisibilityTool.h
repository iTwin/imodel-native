/******************************************************************************

Pointools Vortex API Examples

CloudVisibilityTool.h

(c) Copyright 2014 Bentley 

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_CLOUD_VISIBILITY_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_CLOUD_VISIBILITY_TOOL_H_

#include "VortexExampleApp.h"

/** Simple tool that shows a list of currently loaded clouds IDs and lets you
 show/hide them using ptShowScene()
 */
class CloudVisibilityTool : public Tool
{
public:
	enum
	{
		CmdShowCloud = 4500,
		CmdHideCloud = 4501
	};

	CloudVisibilityTool();

	void buildUserInterface(GLUI_Node *parent);
	void command( int cmdId );
	
private:
	PThandle getSelectedCloudHandle();
	void updateCloudList();

	std::string		m_cloudInfo;
	GLUI_List*		m_cloudTextBox;
	
	typedef std::vector<PThandle> CloudList;
	CloudList m_cloudList;

};

#endif // POINTOOLS_EXAMPLE_APP_CLOUD_VISIBILITY_TOOL_H_

