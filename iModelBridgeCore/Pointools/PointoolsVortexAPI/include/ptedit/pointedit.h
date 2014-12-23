#pragma once

#include <ptcmdppe/eventdefs.h>
#include <pttool/tool.h>
#include <pt/datatree.h>
#include <ptl/block.h>
#include <ptl/branch.h>
#include <pt/typedefs.h>
//#include <ptedit/editDebugDisplay.h>

namespace pt
{

enum SelToolButton
{
	SelRectButton = 0,
	SelFenceButton = 1,
	Sel3DBoxButton = 2,
	Sel3DBallButton = 3,
	SelPlaneButton = 4,
	SelFillButton = 5,
	Sel3DPlaneButton = 6,
	SelCircButton = 7,
	SelSquareButton = 8,
	SelNoneButton = 50
};

/*------------------------------------------------------------------*/ 
/*						Point Editing Tools Class					*/ 
/*------------------------------------------------------------------*/ 
class PointEdit : public  pt::Tool
{
public:
	PointEdit();
	~PointEdit();

	bool initialize();

	static void setSelectModeUI( const char* tool );
	static void setToolUI(SelToolButton b);

	static void makePart( bool selected );
	static void makePartFromSel();
	static void makePartFromVis();
	static void duplicateScene();

	static void showMetaData();
	static void checkForWarnings(void*);

	static void selectMode();
	static void deselectMode();
	static void unhideMode();

	static void openSettings();

	static void deselectAll();
	static void showAll();
	static void clearAll();
	static void clearAllandRedraw();
	static void selectAll();
	static void invertSelection();
	static void invertVisibility();

	static void editMode()		{};
	static void reselectMode()	{};

	static void hideSelPoints();
	
	static void showUI();

	static void refreshEdit();
	static void regenComplete();
	static void regenOOCComplete();
	static void regenLayerComplete();
	static void regenEdit();
	static void stackClear();
	static void stackRemoveSelected();

	static void rectangleSelect();
	static void fenceSelect();
	static bool rectEvents(const EventInfo &e, void*p);
	static bool fenceEvents(const EventInfo &e, void*p);
	static bool planeEvents(const EventInfo &e, void*p);
	static void selPlane();
	static bool paintSelect(const EventInfo &e, void*p);
	static bool paintRGB(const EventInfo &e, void*p);

	static void cloudSelect();

	static void fillSelect();
	static bool _fillSelect( const pt::vector3 &pnt, const pt::vector3i &off, float radius );
	static void _fillSelectRoutine( const pt::vector3 &pnt, const pt::vector3i &off, float radius );

	static void clearPlaneInfoPoints();
	static void editVoxel();
	
	static void fillHistoryTree();

	static void paintSelCube();
	static void paintSelSphere();

	static void clearColourEdits( ubyte layers=0xff );
	static void resetRGBEditing();

	static void paintCube();
	static void paintSphere();
	static void eraseSphere();
	static void updatePaintCol();
	static void updateColBox();
	static void updateLayersWidget();
	static void updatePaintUI();

	static void fillPaint();
	static void erasePaint();
	
	static void activateFillSel();

	static void setConstraint();

	/* layers */ 
	/* current */ 
	static bool setCurrentLayer( ubyte layer, bool maskValue=false );
	static int getCurrentLayer();
	static void setCurrentLayerLeft();
	static void setCurrentLayerRight();
	static void updateLayerVisibility();
	static void updateLayerVisibilityFromEngine();

	/* state */ 
	static bool isLayerLocked( int layer );
	static bool isLayerVisible( int layer );

	static void setVisLayers( ubyte layers );
	static void setLockLayers( ubyte layers );

	static void copyToLayer();
	static void moveToLayer();
	static void syncLayerNames();

	static void updatePaintSettings();
	static void updateEditSettings();
	static bool drawWidgets(const EventInfo &e, void*p);

	static bool readBranch(const datatree::Branch *b);
	static bool writeBranch( datatree::Branch *b);

	template <class F> 
	static bool paintHandler(const EventInfo &e, F &filter);

	static void displayDiagnosticStructure();
	static void displayDiagnosticLayers();
	static void displayDiagnosticNone();

	static void modelCylinder();

	static void reset();

	static void pickIntensity();
	static void pickNormal();

	/* diagnotics */ 
	static void voxelInfo();

	/* rgb filters */ 
	static void hueSatFilter();
	static void brightnessContrastFilter();
	static void updateHueSatPreview();
	static void brightnessConstrastFilter();
	static void updateBConstrastPreview();
	static void applyHLS();
	static void applyBC();
	static void applyWB();
	static void whiteBalanceFilter();
	static void updateWBalancePreview();

	static ParameterMap &params();

	static void extractPointInfo();

	static void evaluateStack();
	static void readEvaluatedStack();
	static void loadEvaluatedStack(); 
	static void saveEvaluatedStack(); 
	static void saveOpStack(); 
	static void loadOpStack(bool merge); 

	static void userStackOption();

	static void generateLayerTexture( const char*rgb128x128 );

	private:
	static void setupImgAdj(const char *, const char*, const char*, const char*);

private:
//	ptgl::DebugDisplayModule *m_debugDisplay;
};
}; /* namespace pt */ 