/******************************************************************************

Pointools Vortex API Examples

EditTool.h

Demonstrates layer based point editing capabilities of Vortex

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_EDIT_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_EDIT_TOOL_H_

#include "VortexExampleApp.h"

class EditTool : public Tool
{
public:
	enum
	{
		CmdRectangleSelect	= 300,
		CmdPolygonSelect	= 301,
		CmdBoxSelect		= 302,
		CmdBrushSelect		= 303,
		CmdSelectHide		= 304,
		CmdSelectInvert		= 305,
		CmdVisbilityInvert	= 306,
		CmdResetEdit		= 307,
		CmdRefreshEdit		= 308,
		CmdDeselect			= 309,
		CmdSelectMode		= 310,
		CmdResetSel			= 311,
		
		CmdLayer1			= 321,
		CmdLayer2			= 322,
		CmdLayer3			= 323,
		CmdLayer4			= 324,
		CmdLayer5			= 325,
		CmdLayer6			= 326,
		CmdLayer7			= 327,
		CmdLayer8			= 328,

		CmdLockLayer1		= 331,
		CmdLockLayer2		= 332,
		CmdLockLayer3		= 333,
		CmdLockLayer4		= 334,
		CmdLockLayer5		= 335,
		CmdLockLayer6		= 336,
		CmdLockLayer7		= 337,
		CmdLockLayer8		= 338,

		CmdVisLayer1		= 341,
		CmdVisLayer2		= 342,
		CmdVisLayer3		= 343,
		CmdVisLayer4		= 344,
		CmdVisLayer5		= 345,
		CmdVisLayer6		= 346,
		CmdVisLayer7		= 347,
		CmdVisLayer8		= 348,

		CmdMovePoints		= 350,
		CmdCopyPoints		= 351,

		CmdRotateBox		= 352,
		CmdBrushUpdate		= 353,

		CmdSetScope			= 360,
		CmdSetScopeScene	= 361,
		CmdSetScopeCloud	= 362,

		CmdSelLayer1		= 371,
		CmdSelLayer2		= 372,
		CmdSelLayer3		= 373,
		CmdSelLayer4		= 374,
		CmdSelLayer5		= 375,
		CmdSelLayer6		= 376,
		CmdSelLayer7		= 377,
		CmdSelLayer8		= 378,

		CmdDeselLayer1		= 381,
		CmdDeselLayer2		= 382,
		CmdDeselLayer3		= 383,
		CmdDeselLayer4		= 384,
		CmdDeselLayer5		= 385,
		CmdDeselLayer6		= 386,
		CmdDeselLayer7		= 387,
		CmdDeselLayer8		= 388,
		
		CmdLayerCol1		= 312,
		CmdLayerCol2		= 313,
		CmdLayerCol3		= 314,
		CmdLayerCol4		= 315,
		CmdLayerCol5		= 316,
		CmdLayerCol6		= 317,
		CmdLayerCol7		= 318,

		CmdSelAll			= 390,
		CmdSelCloud			= 391,
		CmdSelScene			= 392,

		CmdEvalMode			 = 399,

		CmdLayersToChannel   = 365,
		CmdChannelToLayers	 = 366,
		CmdSaveLayersFile	 = 367,
		CmdLoadLayersFile	 = 368,
		CmdCheckPointLayers  = 369,
		CmdSaveEditStack	 = 395,
		CmdLoadEditStack	 = 396,
		CmdSaveScopeLayersFile	 = 397,
		CmdLoadScopeLayersFile	 = 398
	};

	EditTool(bool simple);	// simple = true, for no layers or scope 
	
	void	buildUserInterface(GLUI_Node *parent);
	void	command( int cmdId );

	bool	onMouseButtonDown( int button, int x, int y );
	bool	onMouseButtonUp( int button, int x, int y );
	bool	onMouseMove( int x, int y );
	bool	onMouseDrag( int x, int y, int startX, int startY );

	void	drawPostDisplay();

private:
	int					m_mode;
	int					m_selectMode;
	int					m_evalMode;
	int					m_scope;
	bool				m_simple;
	float				m_brushSize;

	Mouse				m_mouse;
	std::vector<int>	m_polygon;

	GLUI_Button			*m_lyrButtons[8];
	GLUI_Button			*m_lyrVis[8];
	GLUI_Button			*m_lyrLock[8];
	GLUI_Button			*m_lyrSel[8];
	GLUI_Button			*m_lyrDesel[8];

	PTdouble			m_boxLower[3];
	PTdouble			m_boxUpper[3];
	PTbool				m_boxValid;

	float				m_boxColor[3];
	PTdouble			m_boxPosition[3];
	PTdouble			m_boxRotation[3];

	PThandle			m_layersChannel;

	void	setCurrentLayer(int layer);
	void	toggleLayerVisibility(int layer);
	void	toggleLayerLock(int layer);
	void	setSceneScope();
	void	setCloudScope();
	void	selectCloudTest();
	void	selectSceneTest();
	void	chooseColor( int layer );

	void	saveLayerChannels();
	void	loadLayerChannels();

	void	layersToChannel();
	void	channelToLayers();

	void	saveEditStack();
	void	loadEditStack();

	void	doesLayerHavePoints();

	void    saveLayerChannelsScope();
	void    loadLayerChannelsScope();

};

#endif

