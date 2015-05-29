//----------------------------------------------------------------------------
//
// SubObjectsTool.cpp
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#include "SubObjectsTool.h"
#include <set>

static RGBc LABELCOL(60,210,210);
static SubObjectsTool* s_instance = 0;

//-----------------------------------------------------------------------------
SubObjectsTool::SubObjectsTool( void )
{
	m_tree = 0;
	s_instance = this;
}
//-----------------------------------------------------------------------------
void	SubObjectsTool::buildUserInterface(GLUI_Node *parent)
{
	GLUI_Rollout *rolloutTree = new GLUI_Rollout(parent, "Sub-Objects", true );
	rolloutTree->set_w( PANEL_WIDTH );

	m_tree = new GLUI_TreePanel( rolloutTree, "Objects", true );
	
	m_tree->set_format(GLUI_TREEPANEL_ALTERNATE_COLOR | 
             GLUI_TREEPANEL_CONNECT_CHILDREN_ONLY );
}
//-----------------------------------------------------------------------------
void	SubObjectsTool::command( int cmdId )
{

}
//-----------------------------------------------------------------------------
void	SubObjectsTool::drawPostDisplay( void )
{

}
//-----------------------------------------------------------------------------
SubObjectsTool::LayeredPointCloud::LayeredPointCloud( PThandle handle )
{
	m_handle = handle;
	m_uibranch = 0;

	for (int i=0; i<PT_EDIT_MAX_LAYERS; i++)
	{
		m_layers[i] = 0;
	}
	m_layers[0] = 1;

	ptSetEditWorkingMode(PT_EDIT_WORK_ON_ALL);

	PTdouble		lower[3], upper[3], mid[3];
	PTdouble		bbl[3], bbu[3];

	// perform some arbitrary segmentation into layers
	makeActive();
	ptSceneBoundsd( handle, lower, upper );
	
	mid[0] = (lower[0] + upper[0]) * 0.5;
	mid[1] = (lower[1] + upper[1]) * 0.5;
	mid[2] = (lower[2] + upper[2]) * 0.5;

	// select and move
	bbl[0] = lower[0];	bbl[1] = lower[1];	bbl[2] = lower[2];
	bbu[0] = mid[0];	bbu[1] = mid[1];	bbu[2] = mid[2];


	ptSetCurrentLayer(0);
	ptSelectPointsByBox( bbl, bbu );
	ptSetCurrentLayer(1);
	ptMoveSelToCurrentLayer(true);

	// select and move
	bbl[0] = lower[0];	bbl[1] = mid[1];	bbl[2] = lower[2];
	bbu[0] = mid[0];	bbu[1] = upper[1];	bbu[2] = mid[2];

	ptSetCurrentLayer(0);
	ptSelectPointsByBox( bbl, bbu );
	ptSetCurrentLayer(2);

	ptMoveSelToCurrentLayer(true);

	// select and move
	bbl[0] = mid[0];	bbl[1] = lower[1];	bbl[2] = lower[2];
	bbu[0] = upper[0];	bbu[1] = mid[1];	bbu[2] = mid[2];

	ptSetCurrentLayer(0);
	ptSelectPointsByBox( bbl, bbu );
	ptSetCurrentLayer(3);

	ptMoveSelToCurrentLayer(true);

	// select and move
	bbl[0] = mid[0];	bbl[1] = mid[1];	bbl[2] = lower[2];
	bbu[0] = upper[0];	bbu[1] = upper[1];	bbu[2] = mid[2];

	ptSetCurrentLayer(0);
	ptSelectPointsByBox( bbl, bbu );
	ptSetCurrentLayer(4);

	ptMoveSelToCurrentLayer(true);

	// select and move remaining 
	bbl[0] = lower[0];	bbl[1] = lower[1];	bbl[2] = mid[2];
	bbu[0] = upper[0];	bbu[1] = upper[1];	bbu[2] = upper[2];

	ptSetCurrentLayer(0);
	ptSelectPointsByBox( bbl, bbu );

	ptSetCurrentLayer(5);
	ptMoveSelToCurrentLayer(false);
}
//-----------------------------------------------------------------------------
SubObjectsTool::LayeredPointCloud::~LayeredPointCloud()
{
	ptRemoveScene( m_handle );
}
//-----------------------------------------------------------------------------
void SubObjectsTool::LayeredPointCloud::draw( bool dynamic ) const
{
	makeActive();
	ptDrawSceneGL( m_handle, dynamic );
}
//-----------------------------------------------------------------------------
void SubObjectsTool::LayeredPointCloud::makeActive( void ) const
{
	bool has_vis_layer = false;

	ptSetSelectionScope( m_handle );

	// set up layer state
	for (int i=0; i<PT_EDIT_MAX_LAYERS; i++)
	{
		ptShowLayer( i, m_layers[i] ? true : false );
		
		if (!has_vis_layer && m_layers[i])
		{
			ptSetCurrentLayer(i);	// can't turn off current layer 
			has_vis_layer = true;
		}
	}
	// if all layers off, need to turn off point cloud to get this behavior
	if (!has_vis_layer)
		ptShowScene( m_handle, has_vis_layer );
}
//-----------------------------------------------------------------------------
void	SubObjectsTool::onSceneUpdate( void )
{
	syncScenes();
}
//-----------------------------------------------------------------------------
void	SubObjectsTool::syncScenes( void )
{
	PThandle			handles[256];	/* not safe :)	*/ 
	std::set<PThandle>	loadedScenes;
	
	int numScenes = ptGetSceneHandles(handles);

	for (int i=0; i<numScenes; i++)
	{
		loadedScenes.insert(handles[i]);
	}

	/* add any loaded scenes */ 
	for ( std::set<PThandle>::iterator it = loadedScenes.begin(); it != loadedScenes.end(); it++ )
	{
		if ( m_pointClouds.find(*it) == m_pointClouds.end() )
		{
			/* add this */ 
			LayeredPointCloud *pc = new LayeredPointCloud( *it );
			m_pointClouds.insert( PCMap::value_type( *it, pc ) );
		}
	}
	/* remove any unloaded scenes */ 
	std::vector<PThandle> remove;
	for ( PCMap::iterator pit = m_pointClouds.begin(); pit != m_pointClouds.end(); pit++ )
	{
		if (loadedScenes.find(pit->first) == loadedScenes.end())
		{
			remove.push_back(pit->first);
		}
	}
	/* do the remove */ 
	for (unsigned int i=0;i<remove.size();i++)
	{
		PCMap::iterator it = m_pointClouds.find(remove[i]);
		delete it->second;
		m_pointClouds.erase(it);
	}
	/* update the UI */ 
	
	m_tree->resetToRoot();
	m_tree->db();

	GLUI_Tree *vortex = m_tree->curr_branch;

	for (PCMap::iterator pit = m_pointClouds.begin(); pit != m_pointClouds.end(); pit++)
	{
		LayeredPointCloud *pc = pit->second;
		
		if (pc->m_uibranch==0)
		{
			char buff[256];
			sprintf(buff, "POD : %02i", pit->first);
		
			pc->m_uibranch = m_tree->ab( buff, vortex );

			// layers
			for (int i=0;i<PT_EDIT_MAX_LAYERS;i++)
			{
				sprintf(buff, "Layer %i", i);
				GLUI_Checkbox * cb = new GLUI_Checkbox;
				cb->callback = &SubObjectsTool::layerCB_stub;
				cb->set_id( pit->first + i );
				cb->set_name( buff );
				cb->set_int_val( pc->m_layers[i] );
				pc->m_uibranch->add_control( cb );
			}
		}
	}
}
//-----------------------------------------------------------------------------
void		SubObjectsTool::layerCB_stub( int cmdId )
{
	s_instance->layerCB( cmdId );
}
//-----------------------------------------------------------------------------
void		SubObjectsTool::layerCB( int cmdId )
{
	// turn the layer on or off
	int layer = cmdId % 10;
	
	PThandle handle = cmdId - layer;

	PCMap::iterator it = m_pointClouds.find(handle);

	// toggle vis
	if (it->second->m_layers[layer])
		it->second->m_layers[layer] = 0;
	else it->second->m_layers[layer] = 1;
	
	viewRedraw();
}
//-----------------------------------------------------------------------------
void SubObjectsTool::drawBackground()
{
	glClearColor( .0f, .0f, .0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}
//-----------------------------------------------------------------------------
bool SubObjectsTool::drawPointClouds( bool dynamic, bool clearFrame )
{
	ptSetViewport( 0 );	// important to do this in OpenGL context
	ptReadViewFromGL();

	if ( ptNumScenes() )
	{
		for (PCMap::iterator pit = m_pointClouds.begin(); pit != m_pointClouds.end(); pit++)
		{
			LayeredPointCloud *pc = pit->second;
			pc->draw( dynamic );
		}
		return true;
	}

	return false;
}
//-----------------------------------------------------------------------------