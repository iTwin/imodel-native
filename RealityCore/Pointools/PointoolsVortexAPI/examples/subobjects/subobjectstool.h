/******************************************************************************

Pointools Vortex API Examples

SubObjects.h

Demonstrates the use of layers as point cloud sub objects.

Note that this class is both a Tool and a Renderer since 
per point cloud layers requires a different rendering approach

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_SUBOBJECTS_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_SUBOBJECTS_TOOL_H_

#include "VortexExampleApp.h"
#include <map>
#include <string>

class SubObjectsTool : public Tool, public Renderer
{
public:
	enum
	{
		CmdAddPointClouds	= 1200,
		CmdReportLayers = 1201
	};

	SubObjectsTool( void );	
	
	void	buildUserInterface( GLUI_Node *parent );
	void	command( int cmdId );

	void	drawPostDisplay( void );
	
	void	onSceneUpdate( void );

	// renderer overloads
	void	drawBackground();
	bool	drawPointClouds(bool dynamic, bool clearFrame );

	void	report();

private:

	void	syncScenes( void );

	class LayeredPointCloud
	{
	public:

		LayeredPointCloud( PThandle handle );
		~LayeredPointCloud( void );

		void	draw( bool dynamic ) const;
		void	makeActive( void ) const;		// set editing scope and layer set to this
		void	report();
		
		GLUI_Tree	*m_uibranch;
		int			m_layers[PT_EDIT_MAX_LAYERS];	// int due to UI needs

	private:
		PThandle	m_handle;
	};

	static void		layerCB_stub( int cmdId );
	void			layerCB( int cmdId );

	typedef std::map<PThandle, LayeredPointCloud*>	PCMap;
	PCMap			m_pointClouds;

	GLUI_TreePanel	*m_tree;
};

#endif

