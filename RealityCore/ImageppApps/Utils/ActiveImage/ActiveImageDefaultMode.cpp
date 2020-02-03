/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageDefaultMode.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageDefaultMode.cpp,v 1.8 2011/07/18 21:10:36 Donald.Morissette Exp $
//
// Class: ActiveImageDefaultMode
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "ActiveImage.h"
#include "ActiveImageFrame.h"
#include "ActiveImageDoc.h"
#include "ActiveImageView.h"
#include "ActiveImageDefaultMode.h"
#include "HistogramScaling.h"
#include "RotationDialog.h"
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>
#include <Imagepp/all/h/HRPTypeAdaptFilters.h>
#include <Imagepp/all/h/HRPMapFilters8.h>
#include <Imagepp/all/h/HRPMapFilters16.h>
#include <Imagepp/all/h/HRPContrastStretchFilter8.h>
#include <Imagepp/all/h/HRPContrastStretchFilter16.h>
#include <Imagepp/all/h/HRPLigthnessContrastStretch.h>
#include <Imagepp/all/h/HRPLigthnessContrastStretch8.h>
#include <Imagepp/all/h/HRPLigthnessContrastStretch16.h>
#include <Imagepp/all/h/HRPDensitySlicingFilter8.h>
#include <Imagepp/all/h/HRPDensitySlicingFilter16.h>
#include <Imagepp/all/h/HRPLigthnessDensitySlicingFilter8.h>
#include <Imagepp/all/h/HRPLigthnessDensitySlicingFilter16.h>

#include "IntensityDialog.h"
#include "TintDialog.h"

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>

#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>


#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRPCustomConvFilter.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include "ColorReplacerDlg.h"
#include "ColorSpaceTest.h"
#include <Imagepp/all/h/HRAPixelTypeReplacer.h>
#include <Imagepp/all/h/HRPHistogram.h>

#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRAReferenceToStoredRaster.h>
#include <Imagepp/all/h/HRAStoredRaster.h>
#include <ImagePP/all/h/HRADEMRaster.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Messages definition
// use GetCommandID to insure that we have a unique message ID
static const uint32_t ID_EDIT_SELECTMOVE          = CActiveImageFunctionMode::GetCommandID("Select Ordering","Ordering");
static const uint32_t ID_SELECTMOVE_SELECTALL     = CActiveImageFunctionMode::GetCommandID("Select all rasters","Select All");
static const uint32_t ID_SELECTMOVE_UNSELECT      = CActiveImageFunctionMode::GetCommandID("Unselect","Unselect");
static const uint32_t ID_SELECTMOVE_BTF           = CActiveImageFunctionMode::GetCommandID("Bring selected rasters to the front","Bring To Front");
static const uint32_t ID_SELECTMOVE_STB           = CActiveImageFunctionMode::GetCommandID("Send selected rasters to the back","Send To back");
static const uint32_t ID_SELECTMOVE_FLOAT         = CActiveImageFunctionMode::GetCommandID("Move selected rasters up","Move Up");
static const uint32_t ID_SELECTMOVE_SINK          = CActiveImageFunctionMode::GetCommandID("Move selected rasters down","Move Down");
static const uint32_t ID_SELECTMOVE_ROTATE        = CActiveImageFunctionMode::GetCommandID("Rotate the selected raster","Rotate");
static const uint32_t ID_SELECTMOVE_REMOVE        = CActiveImageFunctionMode::GetCommandID("Remove the selected rasters","Remove");
static const uint32_t ID_SELECTMOVE_TOOLBAR       = CActiveImageFunctionMode::GetCommandID("Show/Hide Ordering Toolbar","Ordering ToolBar");

static const uint32_t ID_COLORSPACE_TEST          = CActiveImageFunctionMode::GetCommandID("ColorSpace test","ColorSpace test");

static const uint32_t ID_FILTER_COLORTWIST        = CActiveImageFunctionMode::GetCommandID("Add a colortwist filter","Colortwist");
static const uint32_t ID_FILTER_BRIGHTNESS        = CActiveImageFunctionMode::GetCommandID("Add a brightness filter","Brightness");
static const uint32_t ID_FILTER_CONTRAST          = CActiveImageFunctionMode::GetCommandID("Add a contrast filter","Contrast");

static const uint32_t ID_FILTER_CONTRAST_STRETCH  = CActiveImageFunctionMode::GetCommandID("Add a contrast stretch filter","Contrast Stretch");
static const uint32_t ID_FILTER_LIGHTNESS_STRETCH = CActiveImageFunctionMode::GetCommandID("Add a lightness contrast stretch filter","Lightness Contrast Stretch");
static const uint32_t ID_FILTER_DENSITY_SLICING   = CActiveImageFunctionMode::GetCommandID("Add a density slicing filter","Density Slicing");
static const uint32_t ID_FILTER_LIGHTNESS_SLICING = CActiveImageFunctionMode::GetCommandID("Add a lightness density slicing filter","Lightness Density Slicing");

static const uint32_t ID_FILTER_SMOOTH            = CActiveImageFunctionMode::GetCommandID("Add a smooth filter","Smooth");
static const uint32_t ID_FILTER_SHARPEN           = CActiveImageFunctionMode::GetCommandID("Add a sharpen filter","Sharpen");
static const uint32_t ID_FILTER_DETAIL            = CActiveImageFunctionMode::GetCommandID("Add a detail filter", "Detail");
static const uint32_t ID_FILTER_EDGEENHANCE       = CActiveImageFunctionMode::GetCommandID("Add a edge enhance filter", "Edge Enhance");
static const uint32_t ID_FILTER_FINDEDGES         = CActiveImageFunctionMode::GetCommandID("Add a find edges filter", "Find Edges");
static const uint32_t ID_FILTER_BLUR              = CActiveImageFunctionMode::GetCommandID("Add a blur filter", "Blur");
static const uint32_t ID_FILTER_COLORS_REPLACER   = CActiveImageFunctionMode::GetCommandID("Colors replacing filter", "Color replacer");
static const uint32_t ID_FILTER_ALPHA_REPLACER    = CActiveImageFunctionMode::GetCommandID("Alpha replacing filter", "Alpha replacer");
static const uint32_t ID_FILTER_ALPHA_COMPOSER    = CActiveImageFunctionMode::GetCommandID("Alpha composing filter", "Alpha composer");
static const uint32_t ID_FILTER_GAMMA             = CActiveImageFunctionMode::GetCommandID("Add a gamma filter", "Gamma");
static const uint32_t ID_FILTER_INVERT            = CActiveImageFunctionMode::GetCommandID("Add an invert filter", "Invert");
static const uint32_t ID_FILTER_TINT              = CActiveImageFunctionMode::GetCommandID("Add a tint filter", "Tint");
static const uint32_t ID_FILTER_DEM_ELEVATION         = CActiveImageFunctionMode::GetCommandID("Add DEM Elevation filter", "Elevation Filter");
static const uint32_t ID_FILTER_DEM_ELEVATION_SHADED  = CActiveImageFunctionMode::GetCommandID("Add DEM Elevation filter", "Elevation Filter Shaded");
static const uint32_t ID_FILTER_DEM_SLOPE             = CActiveImageFunctionMode::GetCommandID("Add DEM Slope filter", "Slope Filter");
static const uint32_t ID_FILTER_DEM_SLOPE_SHADED      = CActiveImageFunctionMode::GetCommandID("Add DEM Slope filter", "Slope Filter Shaded");
static const uint32_t ID_FILTER_DEM_ASPECT            = CActiveImageFunctionMode::GetCommandID("Add DEM Aspect filter", "Aspect Filter");
static const uint32_t ID_FILTER_DEM_ASPECT_SHADED     = CActiveImageFunctionMode::GetCommandID("Add DEM Aspect filter", "Aspect Filter Shaded");

static const uint32_t ID_FILTER_3x3_HIGHPASS_EDGE_DETECTION = CActiveImageFunctionMode::GetCommandID("3x3 High pass edge detection", "3x3 Edge Detection");
static const uint32_t ID_FILTER_5x5_HIGHPASS_EDGE_DETECTION = CActiveImageFunctionMode::GetCommandID("5x5 High pass edge detection", "5x5 Edge Detection");
static const uint32_t ID_FILTER_7x7_HIGHPASS_EDGE_DETECTION = CActiveImageFunctionMode::GetCommandID("7x7 High pass edge detection", "7x7 Edge Detection");

static const uint32_t ID_FILTER_3x3_HIGHPASS_EDGE_ENHANCE   = CActiveImageFunctionMode::GetCommandID("3x3 High pass edge detection", "3x3 Edge Enhance");
static const uint32_t ID_FILTER_5x5_HIGHPASS_EDGE_ENHANCE   = CActiveImageFunctionMode::GetCommandID("5x5 High pass edge detection", "5x5 Edge Enhance");
static const uint32_t ID_FILTER_7x7_HIGHPASS_EDGE_ENHANCE   = CActiveImageFunctionMode::GetCommandID("7x7 High pass edge detection", "7x7 Edge Enhance");

static const uint32_t ID_FILTER_3x3_LOWPASS                = CActiveImageFunctionMode::GetCommandID("3x3 Low pass", "3x3 Low pass");
static const uint32_t ID_FILTER_5x5_LOWPASS                = CActiveImageFunctionMode::GetCommandID("5x5 Low pass", "5x5 Low pass");
static const uint32_t ID_FILTER_7x7_LOWPASS                = CActiveImageFunctionMode::GetCommandID("7x7 Low pass", "7x7 Low pass");

static const uint32_t ID_FILTER_3x3_HIGHPASS               = CActiveImageFunctionMode::GetCommandID("3x3 High pass", "3x3 High pass");
static const uint32_t ID_FILTER_5x5_HIGHPASS               = CActiveImageFunctionMode::GetCommandID("5x5 High pass", "5x5 High pass");
static const uint32_t ID_FILTER_7x7_HIGHPASS               = CActiveImageFunctionMode::GetCommandID("7x7 High pass", "7x7 High pass");

static const uint32_t ID_FILTER_3x3_HORIZONTAL             = CActiveImageFunctionMode::GetCommandID("3x3 Horizontal", "3x3 Horizontal");
static const uint32_t ID_FILTER_5x5_HORIZONTAL             = CActiveImageFunctionMode::GetCommandID("5x5 Horizontal", "5x5 Horizontal");
static const uint32_t ID_FILTER_7x7_HORIZONTAL             = CActiveImageFunctionMode::GetCommandID("7x7 Horizontal", "7x7 Horizontal");

static const uint32_t ID_FILTER_3x3_VERTICAL               = CActiveImageFunctionMode::GetCommandID("3x3 Vertical", "3x3 Vertical");
static const uint32_t ID_FILTER_5x5_VERTICAL               = CActiveImageFunctionMode::GetCommandID("5x5 Vertical", "5x5 Vertical");
static const uint32_t ID_FILTER_7x7_VERTICAL               = CActiveImageFunctionMode::GetCommandID("7x7 Vertical", "7x7 Vertical");

static const uint32_t ID_FILTER_3x3_SUMMARY                = CActiveImageFunctionMode::GetCommandID("3x3 Summary", "3x3 Summary");
static const uint32_t ID_FILTER_5x5_SUMMARY                = CActiveImageFunctionMode::GetCommandID("5x5 Summary", "5x5 Summary");
static const uint32_t ID_FILTER_7x7_SUMMARY                = CActiveImageFunctionMode::GetCommandID("7x7 Summary", "7x7 Summary");

static const uint32_t ID_REMOVEFILTERS            = CActiveImageFunctionMode::GetCommandID("Remove the filters associated to the image", "Remove Filters");
static const uint32_t ID_TRANSPARENCE_0           = CActiveImageFunctionMode::GetCommandID("Set the transparence percentage","0%");
static const uint32_t ID_TRANSPARENCE_25          = CActiveImageFunctionMode::GetCommandID("Set the transparence percentage","25%");
static const uint32_t ID_TRANSPARENCE_50          = CActiveImageFunctionMode::GetCommandID("Set the transparence percentage","50%");
static const uint32_t ID_TRANSPARENCE_75          = CActiveImageFunctionMode::GetCommandID("Set the transparence percentage","75%");
static const uint32_t ID_TRANSPARENCE_100         = CActiveImageFunctionMode::GetCommandID("Set the transparence percentage","100%");

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// Global Reference Coord Sys
extern HFCPtr<HGF2DCoordSys> g_pAIRefCoordSys;

// Toolbar definition Array
// toolbar buttons - IDs are command buttons
static UINT BASED_CODE ToolbarButtons[] =
{
//    ID_EDIT_SELECTMOVE,
//        ID_SEPARATOR,

    ID_SELECTMOVE_SELECTALL,
    ID_SELECTMOVE_UNSELECT,
        ID_SEPARATOR,

    ID_SELECTMOVE_BTF,
    ID_SELECTMOVE_STB,
    ID_SELECTMOVE_FLOAT,
    ID_SELECTMOVE_SINK,
        ID_SEPARATOR,

    ID_SELECTMOVE_ROTATE,
    ID_SELECTMOVE_REMOVE,
};


//-----------------------------------------------------------------------------
// Static Variables
//-----------------------------------------------------------------------------
CToolBar CActiveImageDefaultMode::m_Toolbar;



//-----------------------------------------------------------------------------
// Public
// Default Constructor
//-----------------------------------------------------------------------------
CActiveImageDefaultMode::CActiveImageDefaultMode()
{
}


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
CActiveImageDefaultMode::~CActiveImageDefaultMode()
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::RemoveFromSelection(const HFCPtr<HRARaster>& pi_pRaster)
{
    RasterSelection::iterator Itr;
    HPRECONDITION(pi_pRaster != 0);

    Itr = m_Selection.begin();
    while (Itr != m_Selection.end())
    {
        if (*Itr == pi_pRaster)
            break;

        Itr++;
    }

    // delete only if it was found
    if (Itr != m_Selection.end())
        m_Selection.erase(Itr);
}

CMenu CActiveImageDefaultMode::m_Menu;
CMenu CActiveImageDefaultMode::m_FilterMenu;
CMenu CActiveImageDefaultMode::m_FilterSubMenu[CActiveImageDefaultMode_FilteringLevel];
CMenu CActiveImageDefaultMode::m_TransparenceMenu;
Byte CActiveImageDefaultMode::m_Transparence = 0;

//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
bool CActiveImageDefaultMode::SetupMenu(CMenu* pi_pEditMenu, CMenu* pi_pViewMenu)
{
    HPRECONDITION(pi_pEditMenu != 0);
    HPRECONDITION(pi_pViewMenu != 0);

    // Modify the View Menu
    //
    pi_pViewMenu->InsertMenu(-1, MF_BYPOSITION, ID_SELECTMOVE_TOOLBAR, _TEXT("Ordering Toolbar"));

    // Create the ordering sub-menu
    //
    m_Menu.CreateMenu();

    // Create the filter and border menu
    m_FilterMenu.CreateMenu();

    m_TransparenceMenu.CreateMenu();

    // Modify the Edit Menu
    //
    pi_pEditMenu->InsertMenu(0, MF_SEPARATOR|MF_BYPOSITION, 0);
    pi_pEditMenu->InsertMenu(0, MF_BYPOSITION|MF_STRING|MF_POPUP,(UINT_PTR)m_Menu.m_hMenu, _TEXT("Options"));
    pi_pEditMenu->InsertMenu(0, MF_BYPOSITION, ID_EDIT_SELECTMOVE, _TEXT("Ordering"));
    
    pi_pEditMenu->AppendMenu(MF_SEPARATOR|MF_BYPOSITION, 0);
    pi_pEditMenu->AppendMenu(MF_BYPOSITION|MF_STRING|MF_POPUP, (UINT_PTR)m_FilterMenu.m_hMenu, _TEXT("Add Filter..."));
    pi_pEditMenu->AppendMenu(MF_STRING, ID_REMOVEFILTERS, _TEXT("Remove Filters"));
    pi_pEditMenu->AppendMenu(MF_SEPARATOR|MF_BYPOSITION, 0);
    pi_pEditMenu->AppendMenu(MF_BYPOSITION|MF_STRING|MF_POPUP, (UINT_PTR)m_TransparenceMenu.m_hMenu, _TEXT("Transparence..."));    
    pi_pEditMenu->AppendMenu(MF_BYPOSITION, ID_COLORSPACE_TEST, _TEXT("ColorSpace Test"));

    
    // Modify the Ordering Options Sub-Menu
    //
    m_Menu.AppendMenu(MF_STRING, ID_SELECTMOVE_SELECTALL, _TEXT("Select All"));
    m_Menu.AppendMenu(MF_STRING, ID_SELECTMOVE_UNSELECT, _TEXT("Unselect"));
    m_Menu.AppendMenu(MF_SEPARATOR|MF_BYPOSITION, 0);
    m_Menu.AppendMenu(MF_STRING, ID_SELECTMOVE_BTF, _TEXT("Bring To Front"));
    m_Menu.AppendMenu(MF_STRING, ID_SELECTMOVE_STB, _TEXT("Send To Back"));
    m_Menu.AppendMenu(MF_STRING, ID_SELECTMOVE_FLOAT, _TEXT("Move Up"));
    m_Menu.AppendMenu(MF_STRING, ID_SELECTMOVE_SINK, _TEXT("Move Down"));
    m_Menu.AppendMenu(MF_SEPARATOR|MF_BYPOSITION, 0);
    m_Menu.AppendMenu(MF_STRING, ID_SELECTMOVE_ROTATE, _TEXT("Rotate"));
    m_Menu.AppendMenu(MF_STRING, ID_SELECTMOVE_REMOVE, _TEXT("Remove"));
    
    //=======================================================
    // modify the filter menu

    uint32_t Level;
    
    for (Level= 0; Level < CActiveImageDefaultMode_FilteringLevel; Level++)
        m_FilterSubMenu[Level].CreateMenu();
    
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_ALPHA_COMPOSER, _TEXT("Alpha Composer..."));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_ALPHA_REPLACER, _TEXT("Alpha Replacer..."));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_BLUR, _TEXT("Blur..."));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_BRIGHTNESS, _TEXT("Brightness..."));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_COLORS_REPLACER, _TEXT("Colors Replacer..."));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_COLORTWIST, _TEXT("Color Twist..."));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_CONTRAST, _TEXT("Contrast..."));
    m_FilterMenu.AppendMenu(MF_BYPOSITION|MF_STRING|MF_POPUP, (UINT_PTR)m_FilterSubMenu[3].m_hMenu, _TEXT("DEM Filter"));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_DETAIL, _TEXT("Detail"));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_EDGEENHANCE, _TEXT("Edge Enhance"));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_FINDEDGES, _TEXT("Find Edges"));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_GAMMA, _TEXT("Gamma"));    

    m_FilterMenu.AppendMenu(MF_BYPOSITION|MF_STRING|MF_POPUP, (UINT_PTR)m_FilterSubMenu[4].m_hMenu, _TEXT("Histogram Filter"));
    m_FilterSubMenu[4].AppendMenu(MF_STRING, ID_FILTER_CONTRAST_STRETCH,        _TEXT("Contrast Stretch..."));
    m_FilterSubMenu[4].AppendMenu(MF_STRING, ID_FILTER_LIGHTNESS_STRETCH,       _TEXT("Lightness Contrast Stretch..."));
    m_FilterSubMenu[4].AppendMenu(MF_STRING, ID_FILTER_DENSITY_SLICING,         _TEXT("Density Slicing..."));
    m_FilterSubMenu[4].AppendMenu(MF_STRING, ID_FILTER_LIGHTNESS_SLICING,       _TEXT("Lightness Density Slicing..."));

    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_INVERT, _TEXT("Invert"));    
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_SMOOTH, _TEXT("Smooth"));
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_SHARPEN, _TEXT("Sharpen..."));    
    m_FilterMenu.AppendMenu(MF_STRING, ID_FILTER_TINT, _TEXT("Tint"));    
        
    m_FilterMenu.AppendMenu(MF_BYPOSITION|MF_STRING|MF_POPUP, (UINT_PTR)m_FilterSubMenu[0].m_hMenu, _TEXT("Test 3x3CRDVFilter..."));
    m_FilterMenu.AppendMenu(MF_BYPOSITION|MF_STRING|MF_POPUP, (UINT_PTR)m_FilterSubMenu[1].m_hMenu, _TEXT("Test 5x5CRDVFilter..."));
    m_FilterMenu.AppendMenu(MF_BYPOSITION|MF_STRING|MF_POPUP, (UINT_PTR)m_FilterSubMenu[2].m_hMenu, _TEXT("Test 7x7CRDVFilter..."));
    
    m_FilterSubMenu[3].AppendMenu(MF_STRING, ID_FILTER_DEM_ELEVATION,        _TEXT("Elevation"));
    m_FilterSubMenu[3].AppendMenu(MF_STRING, ID_FILTER_DEM_ELEVATION_SHADED, _TEXT("Elevation HillShaded"));
    m_FilterSubMenu[3].AppendMenu(MF_STRING, ID_FILTER_DEM_SLOPE,           _TEXT("Slope"));
    m_FilterSubMenu[3].AppendMenu(MF_STRING, ID_FILTER_DEM_SLOPE_SHADED,    _TEXT("Slope HillShaded"));
    m_FilterSubMenu[3].AppendMenu(MF_STRING, ID_FILTER_DEM_ASPECT,          _TEXT("Aspect"));
    m_FilterSubMenu[3].AppendMenu(MF_STRING, ID_FILTER_DEM_ASPECT_SHADED,   _TEXT("Aspect HillShaded"));

    for (Level= 0; Level < 3; Level++)
    {
        // 3x3 Filter sub menu
        m_FilterSubMenu[Level].AppendMenu(MF_STRING, ID_FILTER_3x3_HIGHPASS_EDGE_DETECTION + Level, _TEXT("High pass edge detection"));
        m_FilterSubMenu[Level].AppendMenu(MF_STRING, ID_FILTER_3x3_HIGHPASS_EDGE_ENHANCE   + Level, _TEXT("High pass edge enhance"));
        m_FilterSubMenu[Level].AppendMenu(MF_STRING, ID_FILTER_3x3_LOWPASS                 + Level, _TEXT("Low pass"));
        m_FilterSubMenu[Level].AppendMenu(MF_STRING, ID_FILTER_3x3_HIGHPASS                + Level, _TEXT("High pass"));
        m_FilterSubMenu[Level].AppendMenu(MF_STRING, ID_FILTER_3x3_HORIZONTAL              + Level, _TEXT("Horizontal filter"));
        m_FilterSubMenu[Level].AppendMenu(MF_STRING, ID_FILTER_3x3_VERTICAL                + Level, _TEXT("Vertical filter"));
        m_FilterSubMenu[Level].AppendMenu(MF_STRING, ID_FILTER_3x3_SUMMARY                 + Level, _TEXT("Summary filter"));
    }
    
    //=======================================================

    m_TransparenceMenu.AppendMenu(MF_STRING|MF_CHECKED,   ID_TRANSPARENCE_0, _TEXT("0%"));
    m_TransparenceMenu.AppendMenu(MF_STRING|MF_UNCHECKED, ID_TRANSPARENCE_25, _TEXT("25%"));
    m_TransparenceMenu.AppendMenu(MF_STRING|MF_UNCHECKED, ID_TRANSPARENCE_50, _TEXT("50%"));
    m_TransparenceMenu.AppendMenu(MF_STRING|MF_UNCHECKED, ID_TRANSPARENCE_75, _TEXT("75%"));
    m_TransparenceMenu.AppendMenu(MF_STRING|MF_UNCHECKED, ID_TRANSPARENCE_100, _TEXT("100%"));

    return (true);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CToolBar* CActiveImageDefaultMode::SetupToolbar(CActiveImageFrame* pi_pFrame,
                                                uint32_t            pi_ID)
{
    CToolBar* pToolbar = 0;
    bool Result;
    HPRECONDITION(pi_pFrame != 0);

    // Create the toolbar
    Result = m_Toolbar.Create(pi_pFrame, 
                              WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY, 
                              pi_ID) &&
             m_Toolbar.LoadBitmap(IDR_SELECTMOVE_TOOLBAR) &&
             m_Toolbar.SetButtons(ToolbarButtons, 
                                   sizeof(ToolbarButtons)/sizeof(UINT));

    // Setup the toolbar
    if (Result)
    {
        m_Toolbar.SetBarStyle(m_Toolbar.GetBarStyle() | CBRS_FLYBY | CBRS_TOOLTIPS);
        m_Toolbar.SetWindowText(_TEXT("Ordering"));
        pToolbar = &m_Toolbar;
    }


    return (pToolbar);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
uint32_t CActiveImageDefaultMode::GetType() const
{
    return (AIFM_SELECTMOVE);
}



//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
bool CActiveImageDefaultMode::OnCommandUpdate(CCmdUI* pi_pCmdUI,
                                               CActiveImageView* pi_pView,
                                               CActiveImageDoc*  pi_pDoc)
{
    HPRECONDITION(pi_pDoc != 0);
    HPRECONDITION(pi_pView != 0);

    m_pDoc = pi_pDoc;
    m_pView = pi_pView;


    bool Result = true;

    if (pi_pCmdUI->m_nID == ID_EDIT_SELECTMOVE)
    {
        const CActiveImageFunctionMode* pFunction = pi_pView->GetFunctionMode();
        
        // Check if the function mode is DefaultMode
        pi_pCmdUI->SetCheck(pFunction || (pFunction->GetType() == AIFM_SELECTMOVE));
    }
    else if (pi_pCmdUI->m_nID == ID_SELECTMOVE_TOOLBAR)
    {
            pi_pCmdUI->SetCheck(m_Toolbar.IsWindowVisible());
    }
    
    else if (pi_pCmdUI->m_nID == ID_SELECTMOVE_SELECTALL)
    {
        HTREEITEM SelObject = m_pDoc->GetSelectedObject();

        pi_pCmdUI->Enable((SelObject != NULL) && (SelObject != m_pDoc->GetDocumentObject()));
    }
    else if (pi_pCmdUI->m_nID == ID_SELECTMOVE_UNSELECT)
    {
        pi_pCmdUI->Enable(!m_Selection.empty() || (pi_pView->GetFunctionMode() != this));
    }
    else if ((pi_pCmdUI->m_nID == ID_SELECTMOVE_BTF) ||
            (pi_pCmdUI->m_nID == ID_SELECTMOVE_STB) ||
            (pi_pCmdUI->m_nID == ID_SELECTMOVE_FLOAT) ||
            (pi_pCmdUI->m_nID == ID_SELECTMOVE_SINK))
    {
        OnUpdateOrder(pi_pCmdUI);
    }
    else if (pi_pCmdUI->m_nID == ID_SELECTMOVE_REMOVE)
        {
        OnUpdateRemove(pi_pCmdUI);
        }
    else if (pi_pCmdUI->m_nID == ID_SELECTMOVE_ROTATE ||
        (pi_pCmdUI->m_nID == ID_SELECTMOVE_REMOVE))
    {
        OnUpdateRotate(pi_pCmdUI);
    }

    else if (((pi_pCmdUI->m_nID >= ID_FILTER_3x3_HIGHPASS_EDGE_DETECTION) && (pi_pCmdUI->m_nID < ID_FILTER_3x3_SUMMARY + 3)) ||
              (pi_pCmdUI->m_nID == ID_FILTER_COLORTWIST)        ||
              (pi_pCmdUI->m_nID == ID_FILTER_CONTRAST)          ||
              (pi_pCmdUI->m_nID == ID_FILTER_CONTRAST_STRETCH)  ||
              (pi_pCmdUI->m_nID == ID_FILTER_LIGHTNESS_STRETCH) ||
              (pi_pCmdUI->m_nID == ID_FILTER_DENSITY_SLICING)   ||
              (pi_pCmdUI->m_nID == ID_FILTER_LIGHTNESS_SLICING) ||
              (pi_pCmdUI->m_nID == ID_FILTER_BRIGHTNESS)        ||
              (pi_pCmdUI->m_nID == ID_FILTER_SMOOTH)            ||
              (pi_pCmdUI->m_nID == ID_FILTER_SHARPEN)           ||
              (pi_pCmdUI->m_nID == ID_FILTER_DETAIL)            ||
              (pi_pCmdUI->m_nID == ID_FILTER_EDGEENHANCE)       ||
              (pi_pCmdUI->m_nID == ID_FILTER_FINDEDGES)         ||
              (pi_pCmdUI->m_nID == ID_FILTER_BLUR)              ||
              (pi_pCmdUI->m_nID == ID_FILTER_GAMMA)             ||
              (pi_pCmdUI->m_nID == ID_FILTER_INVERT)            ||
              (pi_pCmdUI->m_nID == ID_FILTER_TINT)              ||
              (pi_pCmdUI->m_nID == ID_FILTER_COLORS_REPLACER)   ||
              (pi_pCmdUI->m_nID == ID_FILTER_ALPHA_REPLACER)    ||
              (pi_pCmdUI->m_nID == ID_FILTER_ALPHA_COMPOSER)    ||
              (pi_pCmdUI->m_nID == ID_FILTER_DEM_ELEVATION)     ||
              (pi_pCmdUI->m_nID == ID_FILTER_DEM_ELEVATION_SHADED)     ||
              (pi_pCmdUI->m_nID == ID_FILTER_DEM_SLOPE)         ||
              (pi_pCmdUI->m_nID == ID_FILTER_DEM_SLOPE_SHADED)  ||
              (pi_pCmdUI->m_nID == ID_FILTER_DEM_ASPECT)        ||
              (pi_pCmdUI->m_nID == ID_FILTER_DEM_ASPECT_SHADED) ||
              (pi_pCmdUI->m_nID == ID_REMOVEFILTERS)            ||
              (pi_pCmdUI->m_nID == ID_TRANSPARENCE_0)           ||
              (pi_pCmdUI->m_nID == ID_TRANSPARENCE_25)          ||
              (pi_pCmdUI->m_nID == ID_TRANSPARENCE_50)          ||
              (pi_pCmdUI->m_nID == ID_TRANSPARENCE_75)          ||
              (pi_pCmdUI->m_nID == ID_TRANSPARENCE_100))
    {
        Result = true;
    }
    else
        Result = false;

    return (Result);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
bool CActiveImageDefaultMode::OnCommand(uint32_t pi_CommandID,
                                         CActiveImageView* pi_pView,
                                         CActiveImageDoc*  pi_pDoc)
{
    HPRECONDITION(pi_pDoc != 0);
    HPRECONDITION(pi_pView != 0);

    m_pDoc = pi_pDoc;
    m_pView = pi_pView;

    bool Result = true;
    
     const CActiveImageFunctionMode* pFunction = pi_pView->GetFunctionMode();
     
     // verify if the function mode is already a DefaultMode
     if ((pFunction == 0) || (pFunction->GetType() != AIFM_SELECTMOVE))
         pi_pView->SetFunctionMode(this);

    if (pi_CommandID == ID_EDIT_SELECTMOVE)
    {
    }
    else if (pi_CommandID == ID_SELECTMOVE_TOOLBAR)
    {
        CActiveImageFrame* pFrame = (CActiveImageFrame*)AfxGetMainWnd();
        HASSERT(pFrame != 0);

        if (m_Toolbar.IsWindowVisible())
            pFrame->ShowControlBar(&m_Toolbar, false, false);
        else
            pFrame->ShowControlBar(&m_Toolbar, true, false);
    }
    else if (pi_CommandID == ID_SELECTMOVE_SELECTALL)
    {
        OnSelectAll();
    }
    else if (pi_CommandID == ID_SELECTMOVE_UNSELECT)
    {
        OnUnselect();
    }
    else if (pi_CommandID == ID_SELECTMOVE_BTF)
    {
        OnBringToFront();
    }
    else if (pi_CommandID == ID_SELECTMOVE_STB)
    {
        OnSendToBack();
    }
    else if (pi_CommandID == ID_SELECTMOVE_FLOAT)
    {
        OnMoveUp();
    }
    else if (pi_CommandID == ID_SELECTMOVE_SINK)
    {
        OnMoveDown();
    }
    else if (pi_CommandID == ID_SELECTMOVE_REMOVE)
    {
        OnRemove();        
    }
    else if (pi_CommandID == ID_SELECTMOVE_ROTATE)
    {
        OnRotate();
    }
    else if (pi_CommandID == ID_COLORSPACE_TEST)
    {
        ColorSpaceTest Dlg;
        Dlg.DoModal();
    }
    else if (pi_CommandID == ID_FILTER_COLORTWIST)
    {
        // 4 x 4 Colortwist matrix
        double pColortwistMatrix[4][4] = {.3333, .3333, .3333,    0,
                                              0,     1,     0,    0,
                                              0,     0,     1,    0,
                                              0,     0,     0,    1};

        OnFilter(new HRPColortwistFilter(pColortwistMatrix));
    }
    else if (pi_CommandID == ID_FILTER_BRIGHTNESS)
    {
        HTREEITEM SelObject = m_pDoc->GetSelectedObject();
        if(SelObject != NULL && SelObject != m_pDoc->GetDocumentObject())
        {
            CIntensityDialog Dialog(-255, 255, 0);
    
            if(Dialog.DoModal() == IDOK)
            {
                HTREEITEM SelObject = m_pDoc->GetSelectedObject();
                if (IsRasterPixelTypeV16Compatbile(m_pDoc->GetRaster(SelObject)))
                    OnFilter(new HRPColorBalanceFilter16(Dialog.m_Pos << 8));
                else
                    OnFilter(new HRPColorBalanceFilter(Dialog.m_Pos));
            }
        }
    }
    else if (pi_CommandID == ID_FILTER_CONTRAST)
    {
        HTREEITEM SelObject = m_pDoc->GetSelectedObject();
        if(SelObject != NULL && SelObject != m_pDoc->GetDocumentObject())
        {
            CIntensityDialog Dialog(-128, 127, 0);
    
            if(Dialog.DoModal() == IDOK)
            {
                HTREEITEM SelObject = m_pDoc->GetSelectedObject();
                if (IsRasterPixelTypeV16Compatbile(m_pDoc->GetRaster(SelObject)))
                    OnFilter(new HRPContrastFilter16((short)(Dialog.m_Pos << 8)));
                else
                    OnFilter(new HRPContrastFilter((int8_t)Dialog.m_Pos)); 
            }
        }
    }

    else if (pi_CommandID == ID_FILTER_BLUR)
    {
        HTREEITEM SelObject = m_pDoc->GetSelectedObject();
        if(SelObject != NULL && SelObject != m_pDoc->GetDocumentObject())
        {
            CIntensityDialog Dialog(0, 255, 255);
    
            if(Dialog.DoModal() == IDOK)
                OnFilter(new HRPBlurFilter((int8_t)Dialog.m_Pos)); 
        }
    }
    else if (pi_CommandID == ID_FILTER_GAMMA)
    {
        HTREEITEM SelObject = m_pDoc->GetSelectedObject();
        if(SelObject != NULL && SelObject != m_pDoc->GetDocumentObject())
        {
            OnFilter(new HRPGammaFilter(2.2)); 
        }
    }
    else if (pi_CommandID == ID_FILTER_INVERT)   
    {
        HTREEITEM SelObject = m_pDoc->GetSelectedObject();
        if(SelObject != NULL && SelObject != m_pDoc->GetDocumentObject())
        {
            if (IsRasterPixelTypeV16Compatbile(m_pDoc->GetRaster(SelObject)))
                OnFilter(new HRPInvertFilter16());
            else
                OnFilter(new HRPInvertFilter());
        }
    } 
    else if (pi_CommandID == ID_FILTER_TINT)   
    {
        HTREEITEM SelObject = m_pDoc->GetSelectedObject();
        if(SelObject != NULL && SelObject != m_pDoc->GetDocumentObject())
        {
            CTintDialog Dialog(NULL);
            if(Dialog.DoModal() == IDOK)
            {
                COLORREF tintColor = Dialog.GetTintColor();

                if (IsRasterPixelTypeV16Compatbile(m_pDoc->GetRaster(SelObject)))
                {
                    // Normalize values between [0..65555]
                    USHORT col[3];
                    col[0] = GetRValue(tintColor) * 256;
                    col[1] = GetGValue(tintColor) * 256;
                    col[2] = GetBValue(tintColor) * 256;
                    OnFilter(new HRPTintFilter16(col));
                }
                else
                {
                    Byte col[3];
                    col[0] = GetRValue(tintColor);
                    col[1] = GetGValue(tintColor);
                    col[2] = GetBValue(tintColor);
                    OnFilter(new HRPTintFilter(col));
                }
            }
        }
    }
    else if (pi_CommandID == ID_FILTER_COLORS_REPLACER)
    {
        OnPixelReplacerFilter();
    }
    else if (pi_CommandID == ID_FILTER_ALPHA_REPLACER)
    {
        OnAlphaReplacerFilter();
    }
    else if (pi_CommandID == ID_FILTER_ALPHA_COMPOSER)
    {
        OnAlphaComposerFilter();
    }

    else if (pi_CommandID == ID_FILTER_CONTRAST_STRETCH || 
             pi_CommandID == ID_FILTER_LIGHTNESS_STRETCH ||
             pi_CommandID == ID_FILTER_DENSITY_SLICING ||
             pi_CommandID == ID_FILTER_LIGHTNESS_SLICING)
    {
        HTREEITEM SelObject = m_pDoc->GetSelectedObject();
        if(SelObject != NULL && SelObject != m_pDoc->GetDocumentObject())
        {
            HFCPtr<HRARaster> pRaster;
            HTREEITEM         SelObject;

            const HRAHistogramOptions* pHistogramOption = 0;
            SelObject = m_pDoc->GetSelectedObject();

            if(SelObject != m_pDoc->GetDocumentObject())
            {
                pRaster = m_pDoc->GetRaster(SelObject);

                pHistogramOption = pRaster->GetHistogram();
                if (!pHistogramOption)
                {
                    // Ok, here's the problem, HRAHistogramOptions CAN'T be constructed with a pixelType without palette
                    // AND HRPHistogramScalingFilter (HRPMapFilter8 to be more precise.. ) doesn't work with Palette....
                    
                    // HRAHistogramOptions HistogramOptions(0);
                    HRAHistogramOptions HistogramOptions(pRaster->GetPixelType());

                    // Set quality to 100%
			        HRASamplingOptions SamplingOpt;

                    if (AfxMessageBox(_TEXT("Use sub-resolution for histogram sub-sampling ?"), MB_YESNO|MB_ICONSTOP) == IDYES)
                    {
                        SamplingOpt.SetPixelsToScan    (100);
			            SamplingOpt.SetTilesToScan     (100);
			            SamplingOpt.SetPyramidImageSize((Byte)(100.0 / 8));
                    }
                    else
                    {
                        SamplingOpt.SetPixelsToScan    (100);
			            SamplingOpt.SetTilesToScan     (100);
			            SamplingOpt.SetPyramidImageSize(100);
                    }

			        HistogramOptions.SetSamplingOptions(SamplingOpt);

                    // Modify how the histogram will be computed.
                    // HistogramOptions.SetSamplingColorSpace(HRPHistogram::LIGHTNESS);
                    // HistogramOptions.SetSamplingColorSpace(HRPHistogram::GRAYSCALE);
                     HistogramOptions.SetSamplingColorSpace(HRPHistogram::RGB);
                    // HistogramOptions.SetSamplingColorSpace(HRPHistogram::NATIVE);

                    CWaitCursor WaitCursor;

		            // -- Compute histogram --
		            pRaster->ComputeHistogram(&HistogramOptions, true);
                    pHistogramOption = pRaster->GetHistogram();
                }
            }
            
            CHistogramScaling::FilterType filterType;
            if (pi_CommandID == ID_FILTER_CONTRAST_STRETCH) 
                filterType = CHistogramScaling::FILTER_CONTRAST_STRETCH;
            else if (pi_CommandID == ID_FILTER_LIGHTNESS_STRETCH)
                filterType = CHistogramScaling::FILTER_LIGHTNESS_CONTRAST_STRETCH;
            else if (pi_CommandID == ID_FILTER_DENSITY_SLICING)
                filterType = CHistogramScaling::FILTER_DENSITY_SLICING;
            else // pi_CommandID == ID_FILTER_LIGHTNESS_SLICING
                filterType = CHistogramScaling::FILTER_LIGHTNESS_DENSITY_SLICING;

            // Use no more than 3 channels (don't consider alpha here)
            UINT nbChannels = __min(pRaster->GetPixelType()->GetChannelOrg().CountChannels(), 3);
            CHistogramScaling Dialog(pHistogramOption, filterType, nbChannels);
            if(Dialog.DoModal() == IDOK)
            {
                if(SelObject == NULL || SelObject == m_pDoc->GetDocumentObject())
                    Result = false;
                else
                {
                    // get the currently selected object from the document
                    pRaster = m_pDoc->GetRaster(SelObject);
                    HASSERT(pRaster != 0);
                    HFCPtr<HRPPixelType> pPixelType = pRaster->GetPixelType();

                    if (IsRasterPixelTypeV16Compatbile(pRaster))
                    {
                        int StartIndex = Dialog.GetMinValue();
                        int EndIndex = Dialog.GetMaxValue();
                        int MaxOutput = 65535;
                        int OpacityValue = Dialog.GetOpacityValue();
                        int DesaturationValue = Dialog.GetDesaturationValue();
                        COLORREF startColor = Dialog.GetStartColor();
                        COLORREF endColor = Dialog.GetEndColor();

                        if (pi_CommandID == ID_FILTER_CONTRAST_STRETCH)
                        {
                            HFCPtr<HRPContrastStretchFilter16> pFilter(new HRPContrastStretchFilter16(pPixelType));
                            for (unsigned short i = 0; i < nbChannels; i++)
                            {
                                // Normalize values between [MinOutput, MaxOutput]. Dialog values are in [0, 255].
                                int32_t intervalMin = (int32_t)(Dialog.GetIntervalMin(i) * MaxOutput / 255);
                                int32_t intervalMax = (int32_t)(Dialog.GetIntervalMax(i) * MaxOutput / 255);
                                int32_t contrastMin = (int32_t)(Dialog.GetContrastMin(i) * MaxOutput / 255);
                                int32_t contrastMax = (int32_t)(Dialog.GetContrastMax(i) * MaxOutput / 255);
                                double GammaFactor = Dialog.GetGammaFactor(i);

                                pFilter->SetInterval        (i, intervalMin, intervalMax);
                                pFilter->SetGammaFactor     (i, GammaFactor);
                                pFilter->SetContrastInterval(i, contrastMin, contrastMax);
 
                            }
                            OnFilter((HFCPtr<HRPFilter>&)pFilter);
                        }
                        else if (pi_CommandID == ID_FILTER_LIGHTNESS_STRETCH)
                        {
                            HFCPtr<HRPLigthnessContrastStretch16> pFilter(new HRPLigthnessContrastStretch16(pPixelType));

                            // Normalize values between [MinOutput, MaxOutput]. Dialog values are in [0, 255].
                            // ONLY CHANNEL 0 IS CONSIDERED IN THIS CASE
                            int32_t intervalMin = (int32_t)(Dialog.GetIntervalMin(0) * MaxOutput / 255);
                            int32_t intervalMax = (int32_t)(Dialog.GetIntervalMax(0) * MaxOutput / 255);
                            int32_t contrastMin = (int32_t)(Dialog.GetContrastMin(0) * MaxOutput / 255);
                            int32_t contrastMax = (int32_t)(Dialog.GetContrastMax(0) * MaxOutput / 255);
                            double GammaFactor = Dialog.GetGammaFactor(0);

                            for (unsigned short i = 0; i < __min(pRaster->GetPixelType()->GetChannelOrg().CountChannels(), 3); i++)
                            {
                                pFilter->SetInterval        (i, intervalMin, intervalMax);
                                pFilter->SetGammaFactor     (i, GammaFactor);
                                pFilter->SetContrastInterval(i, contrastMin, contrastMax);
                            }
                            OnFilter((HFCPtr<HRPFilter>&)pFilter);
                        }
                        else if (pi_CommandID == ID_FILTER_DENSITY_SLICING)
                        {
                            HFCPtr<HRPDensitySlicingFilter16> pFilter(new HRPDensitySlicingFilter16(pPixelType));
                            pFilter->SetDesaturationFactor(DesaturationValue);
                            pFilter->AddSlice (StartIndex<< 8, EndIndex<< 8, startColor, endColor, OpacityValue);
                            OnFilter((HFCPtr<HRPFilter>&)pFilter);
                        }
                        else if (pi_CommandID == ID_FILTER_LIGHTNESS_SLICING)
                        {
                            HFCPtr<HRPLigthnessDensitySlicingFilter16> pLigthnessFilter(new HRPLigthnessDensitySlicingFilter16(pPixelType));
                            pLigthnessFilter->SetDesaturationFactor(DesaturationValue);
                            pLigthnessFilter->AddSlice (StartIndex<< 8, EndIndex<< 8, startColor, endColor, OpacityValue);
                            OnFilter((HFCPtr<HRPFilter>&)pLigthnessFilter);
                        }
                    }  // IsRasterPixelTypeV16Compatbile

                    else
                    {
                        int StartIndex = Dialog.GetMinValue();
                        int EndIndex = Dialog.GetMaxValue();
                        int OpacityValue = Dialog.GetOpacityValue();
                        int DesaturationValue = Dialog.GetDesaturationValue();
                        COLORREF startColor = Dialog.GetStartColor();
                        COLORREF endColor = Dialog.GetEndColor();

                        if (pi_CommandID == ID_FILTER_CONTRAST_STRETCH)
                        {
                            HFCPtr<HRPContrastStretchFilter8> pFilter(new HRPContrastStretchFilter8(pPixelType));
                            for (unsigned short i = 0; i < pRaster->GetPixelType()->GetChannelOrg().CountChannels(); i++)
                            {
                                pFilter->SetInterval        (i, (int32_t)Dialog.GetIntervalMin(i), (int32_t)Dialog.GetIntervalMax(i));
                                pFilter->SetGammaFactor     (i, Dialog.GetGammaFactor(i));
                                pFilter->SetContrastInterval(i, (int32_t)Dialog.GetContrastMin(i), (int32_t)Dialog.GetContrastMax(i));
                            }
                            OnFilter((HFCPtr<HRPFilter>&)pFilter);
                        }
                        else if (pi_CommandID == ID_FILTER_LIGHTNESS_STRETCH)
                        {
                            HFCPtr<HRPLigthnessContrastStretch8> pFilter(new HRPLigthnessContrastStretch8(pPixelType));
                            for (unsigned short i = 0; i < pRaster->GetPixelType()->GetChannelOrg().CountChannels(); i++)
                                {
                                // ONLY CHANNEL 0 IS CONSIDERED IN THIS CASE
                                pFilter->SetInterval        (i, (int32_t)Dialog.GetIntervalMin(0), (int32_t)Dialog.GetIntervalMax(0));
                                pFilter->SetGammaFactor     (i, Dialog.GetGammaFactor(0));
                                pFilter->SetContrastInterval(i, (int32_t)Dialog.GetContrastMin(0), (int32_t)Dialog.GetContrastMax(0));
                                }
                            OnFilter((HFCPtr<HRPFilter>&)pFilter);
                        }
                        else if (pi_CommandID == ID_FILTER_DENSITY_SLICING)
                        {
                            HFCPtr<HRPDensitySlicingFilter8> pFilter(new HRPDensitySlicingFilter8(pPixelType));
                            pFilter->SetDesaturationFactor(DesaturationValue);
                            pFilter->AddSlice (StartIndex, EndIndex, startColor, endColor, OpacityValue);
                            OnFilter((HFCPtr<HRPFilter>&)pFilter);
                        }
                        else if (pi_CommandID == ID_FILTER_LIGHTNESS_SLICING)
                        {
                            HFCPtr<HRPLigthnessDensitySlicingFilter8> pLigthnessFilter(new HRPLigthnessDensitySlicingFilter8(pPixelType));
                            pLigthnessFilter->SetDesaturationFactor(DesaturationValue);
                            pLigthnessFilter->AddSlice (StartIndex, EndIndex, startColor, endColor, OpacityValue);
                            OnFilter((HFCPtr<HRPFilter>&)pLigthnessFilter);
                        }
                    }
                }
            }
        }
    }

    else if (pi_CommandID == ID_FILTER_SHARPEN)
    {
        HTREEITEM SelObject = m_pDoc->GetSelectedObject();
        if(SelObject != NULL && SelObject != m_pDoc->GetDocumentObject())
        {
            CIntensityDialog Dialog(0, 255, 127);
    
            if(Dialog.DoModal() == IDOK)
                OnFilter(new HRPSharpenFilter((int8_t)Dialog.m_Pos)); 
        }
    }
    else if (pi_CommandID == ID_FILTER_SMOOTH)
    {
       OnFilter(new HRPSmoothFilter());
    }
    else if (pi_CommandID == ID_FILTER_DETAIL)
    {
        OnFilter(new HRPDetailFilter());
    }
    else if (pi_CommandID == ID_FILTER_EDGEENHANCE)
    {
        OnFilter(new HRPEdgeEnhanceFilter());
    }
    else if (pi_CommandID == ID_FILTER_FINDEDGES)
    {
        OnFilter(new HRPFindEdgesFilter());
    }  
    else if (pi_CommandID == ID_FILTER_FINDEDGES)
    {
        OnFilter(new HRPFindEdgesFilter());
    }  
    
    else if ((pi_CommandID >= ID_FILTER_3x3_HIGHPASS_EDGE_DETECTION) && (pi_CommandID < ID_FILTER_3x3_HIGHPASS_EDGE_DETECTION + 3))
    {
        if (pi_CommandID == ID_FILTER_3x3_HIGHPASS_EDGE_DETECTION) 
        {
            // 3 x 3 matrix
            int32_t pWeightMatrix[3 * 3] = {-1, -1, -1,
                                           -1,  8, -1,
                                           -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(3, 3, 1, 1, pWeightMatrix));
        }
        else if (pi_CommandID == ID_FILTER_5x5_HIGHPASS_EDGE_DETECTION)
        {
            // 5 x 5 matrix
            int32_t pWeightMatrix[5 * 5] = {-1, -1, -1, -1, -1,
                                           -1, -2, -2, -2, -1,
                                           -1, -2, 32, -2, -1,
                                           -1, -2, -2, -2, -1,
                                           -1, -1, -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(5, 5, 2, 2, pWeightMatrix));
        }
        
        else
        {
            // 7 x 7 matrix
            int32_t pWeightMatrix[7 * 7] = {-1, -1, -1, -1,  0,  0, -1,
                                            0,  0,  0,  0,  0,  0,  0,
                                            0,  0,  0,  0,  0,  0,  0,
                                           -1, -2, -2, -2,  0,  0, -1,
                                           -1, -2, 70, -2,  0,  0, -1,
                                           -1, -2, -2, -2,  0,  0, -1,
                                           -1, -1, -1, -1,  0,  0, -1,};
            
            OnFilter(new HRPCustomConvFilter(7, 7, 4, 2, pWeightMatrix));
        }
    }
    else if ((pi_CommandID >= ID_FILTER_3x3_HIGHPASS_EDGE_ENHANCE) && (pi_CommandID < ID_FILTER_3x3_HIGHPASS_EDGE_ENHANCE + 3))
    {
        if (pi_CommandID == ID_FILTER_3x3_HIGHPASS_EDGE_ENHANCE) 
        {
            // 3 x 3 matrix
            int32_t pWeightMatrix[3 * 3] = {-1, -1, -1,
                                           -1, 17, -1,
                                           -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(3, 3, 1, 1, pWeightMatrix));
        }
        else if (pi_CommandID == ID_FILTER_5x5_HIGHPASS_EDGE_ENHANCE)
        {
            // 5 x 5 matrix
            int32_t pWeightMatrix[5 * 5] = {-1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1,
                                           -1, -1, 49, -1, -1,
                                           -1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(5, 5, 2, 2, pWeightMatrix));
        }
        
        else
        {
            // 7 x 7 matrix
            int32_t pWeightMatrix[7 * 7] = {-1, -1, -1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1, -1, -1,
                                           -1, -1, -1, 97, -1, -1, -1,
                                           -1, -1, -1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(7, 7, 3, 3, pWeightMatrix));
        }
    }
    else if ((pi_CommandID >= ID_FILTER_3x3_LOWPASS) && (pi_CommandID < ID_FILTER_3x3_LOWPASS + 3))
    {
        if (pi_CommandID == ID_FILTER_3x3_LOWPASS) 
        {
            // 3 x 3 matrix
            int32_t pWeightMatrix[3 * 3] = { 1,  1,  1,
                                            1,  1,  1,
                                            1,  1,  1};
            
            OnFilter(new HRPCustomConvFilter(3, 3, 1, 1, pWeightMatrix));
        }
        else if (pi_CommandID == ID_FILTER_5x5_LOWPASS)
        {
            // 5 x 5 matrix
            int32_t pWeightMatrix[5 * 5] = { 1,  1,  1,  1,  1,
                                            1,  1,  1,  1,  1,
                                            1,  1,  1,  1,  1,
                                            1,  1,  1,  1,  1,
                                            1,  1,  1,  1,  1};
            
            OnFilter(new HRPCustomConvFilter(5, 5, 2, 2, pWeightMatrix));
        }
        
        else
        {
            // 7 x 7 matrix
            int32_t pWeightMatrix[7 * 7] = { 1,  1,  1,  1,  1,  1,  1,
                                            1,  1,  1,  1,  1,  1,  1,
                                            1,  1,  1,  1,  1,  1,  1,
                                            1,  1,  1,  1,  1,  1,  1,
                                            1,  1,  1,  1,  1,  1,  1,
                                            1,  1,  1,  1,  1,  1,  1,
                                            1,  1,  1,  1,  1,  1,  1};
            
            OnFilter(new HRPCustomConvFilter(7, 7, 3, 3, pWeightMatrix));
        }
    }
    else if ((pi_CommandID >= ID_FILTER_3x3_HIGHPASS) && (pi_CommandID < ID_FILTER_3x3_HIGHPASS + 3))
    {
        if (pi_CommandID == ID_FILTER_3x3_HIGHPASS) 
        {
            // 3 x 3 matrix
            int32_t pWeightMatrix[3 * 3] = {-1, -1, -1,
                                           -1,  9, -1,
                                           -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(3, 3, 1, 1, pWeightMatrix));
        }
        else if (pi_CommandID == ID_FILTER_5x5_HIGHPASS)
        {
            // 5 x 5 matrix
            int32_t pWeightMatrix[5 * 5] = {-1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1,
                                           -1, -1, 24, -1, -1,
                                           -1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(5, 5, 2, 2, pWeightMatrix));
        }
        
        else
        {
            // 7 x 7 matrix
            int32_t pWeightMatrix[7 * 7] = {-1, -1, -1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1, -1, -1,
                                           -1, -1, -1, 48, -1, -1, -1,
                                           -1, -1, -1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1, -1, -1,
                                           -1, -1, -1, -1, -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(7, 7, 3, 3, pWeightMatrix));
        }
    }
    else if ((pi_CommandID >= ID_FILTER_3x3_HORIZONTAL) && (pi_CommandID < ID_FILTER_3x3_HORIZONTAL + 3))
    {
        if (pi_CommandID == ID_FILTER_3x3_HORIZONTAL) 
        {
            // 3 x 3 matrix
            int32_t pWeightMatrix[3 * 3] = {-1, -1, -1,
                                            2,  2,  2,
                                           -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(3, 3, 1, 1, pWeightMatrix));
        }
        else if (pi_CommandID == ID_FILTER_5x5_HORIZONTAL)
        {
            // 5 x 5 matrix
            int32_t pWeightMatrix[5 * 5] = {-1, -1, -1, -1, -1,
                                           -2, -2, -2, -2, -2,
                                            6,  6,  6,  6,  6,
                                           -2, -2, -2, -2, -2,
                                           -1, -1, -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(5, 5, 2, 2, pWeightMatrix));
        }
        
        else
        {
            // 7 x 7 matrix
            int32_t pWeightMatrix[7 * 7] = {-1, -1, -1, -1, -1, -1, -1,
                                           -2, -2, -2, -2, -2, -2, -2,
                                           -3, -3, -3, -3, -3, -3, -3,
                                           12, 12, 12, 12, 12, 12, 12,
                                           -3, -3, -3, -3, -3, -3, -3,
                                           -2, -2, -2, -2, -2, -2, -2,
                                           -1, -1, -1, -1, -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(7, 7, 3, 3, pWeightMatrix));
        }
    }
    else if ((pi_CommandID >= ID_FILTER_3x3_VERTICAL) && (pi_CommandID < ID_FILTER_3x3_VERTICAL + 3))
    {
        if (pi_CommandID == ID_FILTER_3x3_VERTICAL) 
        {
            // 3 x 3 matrix
            int32_t pWeightMatrix[3 * 3] = {-1,  2, -1,
                                           -1,  2, -1,
                                           -1,  2, -1};
            
            OnFilter(new HRPCustomConvFilter(3, 3, 1, 1, pWeightMatrix));
        }
        else if (pi_CommandID == ID_FILTER_5x5_VERTICAL)
        {
            // 5 x 5 matrix
            int32_t pWeightMatrix[5 * 5] = {-1, -2,  6, -2, -1,
                                           -1, -2,  6, -2, -1,
                                           -1, -2,  6, -2, -1,
                                           -1, -2,  6, -2, -1,
                                           -1, -2,  6, -2, -1};
            
            OnFilter(new HRPCustomConvFilter(5, 5, 2, 2, pWeightMatrix));
        }
        
        else
        {
            // 7 x 7 matrix
            int32_t pWeightMatrix[7 * 7] = {-1, -2, -3, 12, -3, -2, -1,
                                           -1, -2, -3, 12, -3, -2, -1,
                                           -1, -2, -3, 12, -3, -2, -1,
                                           -1, -2, -3, 12, -3, -2, -1,
                                           -1, -2, -3, 12, -3, -2, -1,
                                           -1, -2, -3, 12, -3, -2, -1,
                                           -1, -2, -3, 12, -3, -2, -1};
            
            OnFilter(new HRPCustomConvFilter(7, 7, 3, 3, pWeightMatrix));
        }
    }
    else if ((pi_CommandID >= ID_FILTER_3x3_SUMMARY) && (pi_CommandID < ID_FILTER_3x3_SUMMARY + 3))
    {
        if (pi_CommandID == ID_FILTER_3x3_SUMMARY) 
        {
            // 3 x 3 matrix
            int32_t pWeightMatrix[3 * 3] = {-1, -1, -1,
                                           -1, 10, -1,
                                           -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(3, 3, 1, 1, pWeightMatrix));
        }
        else if (pi_CommandID == ID_FILTER_5x5_SUMMARY)
        {
            // 5 x 5 matrix
            int32_t pWeightMatrix[5 * 5] = {-1, -1, -1, -1, -1,
                                           -1, -2, -2, -2, -1,
                                           -1, -2, 70, -2, -1,
                                           -1, -2, -2, -2, -1,
                                           -1, -1, -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(5, 5, 2, 2, pWeightMatrix));
        }
        
        else
        {
            // 7 x 7 matrix
            int32_t pWeightMatrix[7 * 7] = {-1, -1, -1, -1, -1, -1, -1,
                                           -1, -2, -2, -2, -2, -2, -1,
                                           -1, -2, -3, -3, -3, -2,  1,
                                           -1, -2, -3, 90, -3, -2,  1,
                                           -1, -2, -3, -3, -3, -2,  1,
                                           -1, -2, -2, -2, -2, -2, -1,
                                           -1, -1, -1, -1, -1, -1, -1};
            
            OnFilter(new HRPCustomConvFilter(7, 7, 3, 3, pWeightMatrix));
        }
    }
    else if(pi_CommandID == ID_FILTER_DEM_ELEVATION || pi_CommandID == ID_FILTER_DEM_ELEVATION_SHADED)
        {
        if(!OnDEMFilter(HRPDEMFilter::Style_Elevation, pi_CommandID == ID_FILTER_DEM_ELEVATION_SHADED))
            AfxMessageBox(L"DEM filter not supported");
        }
    else if(pi_CommandID == ID_FILTER_DEM_SLOPE || pi_CommandID == ID_FILTER_DEM_SLOPE_SHADED)
        {
        if(!OnDEMFilter(HRPDEMFilter::Style_SlopePercent, pi_CommandID == ID_FILTER_DEM_SLOPE_SHADED))
            AfxMessageBox(L"DEM filter not supported");
        }
    else if(pi_CommandID == ID_FILTER_DEM_ASPECT || pi_CommandID == ID_FILTER_DEM_ASPECT_SHADED)
        {
        if(!OnDEMFilter(HRPDEMFilter::Style_Aspect, pi_CommandID == ID_FILTER_DEM_ASPECT_SHADED))
            AfxMessageBox(L"DEM filter not supported");
        }
    else if (pi_CommandID == ID_REMOVEFILTERS)
    {
        OnRemoveFilters();
    }
    else if (pi_CommandID == ID_TRANSPARENCE_0)
    {
        m_Transparence = 0;

        m_TransparenceMenu.CheckMenuItem(0, MF_BYPOSITION|MF_CHECKED);
        m_TransparenceMenu.CheckMenuItem(1, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(2, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(3, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(4, MF_BYPOSITION|MF_UNCHECKED);

        OnTransparent(255);
    }
    else if (pi_CommandID == ID_TRANSPARENCE_25)
    {
        m_Transparence = 1;

        m_TransparenceMenu.CheckMenuItem(0, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(1, MF_BYPOSITION|MF_CHECKED);
        m_TransparenceMenu.CheckMenuItem(2, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(3, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(4, MF_BYPOSITION|MF_UNCHECKED);
    
        OnTransparent(195);
    }
    else if (pi_CommandID == ID_TRANSPARENCE_50)
    {
        m_Transparence = 2;

        m_TransparenceMenu.CheckMenuItem(0, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(1, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(2, MF_BYPOSITION|MF_CHECKED);
        m_TransparenceMenu.CheckMenuItem(3, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(4, MF_BYPOSITION|MF_UNCHECKED);

        OnTransparent(127);
    }
    else if (pi_CommandID == ID_TRANSPARENCE_75)
    {
        m_Transparence = 3;

        m_TransparenceMenu.CheckMenuItem(0, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(1, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(2, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(3, MF_BYPOSITION|MF_CHECKED);
        m_TransparenceMenu.CheckMenuItem(4, MF_BYPOSITION|MF_UNCHECKED);

        OnTransparent(63);
    }
    else if (pi_CommandID == ID_TRANSPARENCE_100)
    {
        m_Transparence = 4;

        m_TransparenceMenu.CheckMenuItem(0, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(1, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(2, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(3, MF_BYPOSITION|MF_UNCHECKED);
        m_TransparenceMenu.CheckMenuItem(4, MF_BYPOSITION|MF_CHECKED);

        OnTransparent(0);    
    }
    else
        Result = false;

    return (Result);
}



//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnDraw(CDC* pDC)
{
    DrawSelection(pDC);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnUndraw(CDC* pDC)
{
    DrawSelection(pDC);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::Setup()
{
    m_Selection.clear();
}

vector<HFCPtr<HRARaster>, allocator<HFCPtr<HRARaster> > > CActiveImageDefaultMode::GetSelection()
    {
    return m_Selection;
    }

//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnLButtonDblClk(uint32_t pi_Flags, 
                                              CPoint& pi_rPoint)
{
}

                                            
//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnLButtonDown(uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
    HTREEITEM         SelObject;
    HFCPtr<HRARaster> pSelectedObject;

    // Verify if the current selection is the document item
    SelObject = m_pDoc->GetSelectedObject();
    if ((SelObject != NULL) &&
        (SelObject != m_pDoc->GetDocumentObject()))
    {
        // get the currently selected object from the document
        pSelectedObject = m_pDoc->GetRaster(SelObject);
        HASSERT(pSelectedObject != 0);

        // verify if the object is a mosaic
        if (pSelectedObject->GetClassID() == HIMMosaic::CLASS_ID)
        {
            // Convert to a mosaic to simplify life
            HFCPtr<HIMMosaic> pMosaic = (HFCPtr<HIMMosaic>&) pSelectedObject;

            // Get the location of the mouse cursor
            HGF2DLocation CurrentLocation(m_pView->GetLocation(pi_rPoint));

            // Set the currently selected raster
            m_pCurrentRaster = pMosaic->GetAt(CurrentLocation);

            // select the raster
            SelectRaster(pi_Flags, m_pCurrentRaster);    

            // start the moving operation if there is a raster and the file
            // is not read only
            // if the database is read-only, disable the controls
            if ((m_pCurrentRaster != 0) && 
                !(m_pDoc->IsReadOnly()))
                MoveRaster(MS_START, pi_rPoint);
            else
                m_pCurrentRaster = 0;
        } // selecting in a mosaic
        else
        {
            // verify if we're in the raster
            HVEShape Shape(*(pSelectedObject->GetEffectiveShape()));
            Shape.ChangeCoordSys(m_pView->GetCoordSys());

            if (Shape.IsPointIn(HGF2DLocation((double)pi_rPoint.x,
                                              (double)pi_rPoint.y, 
                                              m_pView->GetCoordSys())) )
                SelectRaster(pi_Flags, pSelectedObject);
            else
                SelectRaster(pi_Flags, HFCPtr<HRARaster>());

            m_pCurrentRaster = 0;
        } // selecting in a single raster
    }
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnLButtonUp  (uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
    // if there is a selected raster, apply the move
    if (m_pCurrentRaster != 0)
    {
        MoveRaster(MS_END, pi_rPoint);
        m_pCurrentRaster = 0;
    }
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnRButtonDblClk(uint32_t pi_Flags, 
                                              CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnRButtonDown(uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnRButtonUp  (uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnMouseMove  (uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
    // if there is a currently selected raster, move it
    if (m_pCurrentRaster != 0)
    {
        // Get the client area of the view
        CRect rect;
        m_pView->GetClientRect(&rect);
        // verify if the mouse position is in the client area of the view
        // If so, peform the move.  Otherwise abort.
        if (rect.PtInRect(pi_rPoint))
            MoveRaster(MS_MOVE, pi_rPoint);
        else
            MoveRaster(MS_ABORT, pi_rPoint);
    }
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnKeyDown(uint32_t pi_Char, 
                                        uint32_t pi_RepeatCount, 
                                        uint32_t pi_Flags)
{
    // if there is a raster being moved and the character pressed is escape,
	// stop the move
    if ((m_pCurrentRaster != 0) && (pi_Char == VK_ESCAPE))
            MoveRaster(MS_ABORT, CPoint());
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnKeyUp  (uint32_t pi_Char, 
                                        uint32_t pi_RepeatCount, 
                                        uint32_t pi_Flags)
{
}



//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnRemoveFilters()
{
    HFCPtr<HRARaster> pRaster;
    HTREEITEM         SelObject;

    // Verify that the current selection is the document item
    SelObject = m_pDoc->GetSelectedObject();
    if(SelObject == NULL || SelObject == m_pDoc->GetDocumentObject())
        return;

    // get the currently selected object from the document
    pRaster = m_pDoc->GetRaster(SelObject);
    HASSERT(pRaster != 0);

    // is the raster a filtered image?
    //if ((!IsRasterTransparent(pRaster) && pRaster->GetClassID() == HIMFilteredImage::CLASS_ID) || pRaster->GetClassID() == HRADEMRaster::CLASS_ID)
    if (pRaster->GetClassID() == HIMFilteredImage::CLASS_ID || pRaster->GetClassID() == HRADEMRaster::CLASS_ID)
    {
        HFCPtr<HRARaster> pSourceImage = m_pDoc->RemoveImageView((HFCPtr<HRAImageView>&)pRaster); 
    
        m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
    }
}

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnFilter(const HFCPtr<HRPFilter>& pi_rpFilter)
{
    HFCPtr<HRARaster> pRaster;
    HTREEITEM         SelObject;

    // Verify that the current selection is the document item
    SelObject = m_pDoc->GetSelectedObject();
    if(SelObject == NULL || SelObject == m_pDoc->GetDocumentObject())
        return;

    // get the currently selected object from the document
    pRaster = m_pDoc->GetRaster(SelObject);
    HASSERT(pRaster != 0);

    // is the raster already a filtered image?
    if (!IsRasterTransparent(pRaster) && pRaster->GetClassID() == HIMFilteredImage::CLASS_ID)
    {
        HFCPtr<HRPFilter> pNewFilter = ((HFCPtr<HIMFilteredImage>&)pRaster)->GetFilter()->ComposeWith(pi_rpFilter);
        ((HFCPtr<HIMFilteredImage>&)pRaster)->SetFilter(pNewFilter);
    }
    else
    {
	    // create the filtered image
	    HFCPtr<HRARaster> pFilteredImage = new HIMFilteredImage(pRaster, pi_rpFilter);       
        m_pDoc->ReplaceRaster(SelObject, pFilteredImage);
    }

    m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
}


//-----------------------------------------------------------------------------
// OnTransparent 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnTransparent(Byte pi_Transparence)
{
    HFCPtr<HRARaster> pRaster;
    HTREEITEM         SelObject;

    // Verify that the current selection is the document item
    SelObject = m_pDoc->GetSelectedObject();
    if(SelObject == NULL || SelObject == m_pDoc->GetDocumentObject())
        return;

    // get the currently selected object from the document
    pRaster = m_pDoc->GetRaster(SelObject);
    HASSERT(pRaster != 0);

    HFCPtr<HRARaster> pSourceImage;

    // remove the pixel type replacer if there is already one
    if(IsRasterTransparent(pRaster))
        pSourceImage = m_pDoc->RemoveImageView((HFCPtr<HRAImageView>&)pRaster); 
    else
        pSourceImage = pRaster;

    // if the opacity is other than the max, set a pixel type replacer
    if(pi_Transparence != 255) 
    {
        // Ask the document to do the job
        HFCPtr<HRARaster> pTransparentImage = m_pDoc->PlaceInTransparentImage(pSourceImage, pi_Transparence);
    }

   m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::SelectRaster(UINT nFlags, const HFCPtr<HRARaster>& pi_pRaster)
{
    CClientDC DC(m_pView);
    bool     bSelected;

    // if the raster is not valid, clear the selection
    if (pi_pRaster == 0)
    {
        OnUnselect();
    }
    else
    {
        // verify if the raster is already selected
        // If the raster is in the list, do not invalidate
        bSelected = false;
        RasterSelection::iterator itr = m_Selection.begin();
        while (!bSelected && (itr != m_Selection.end()))
        {
            if (*itr == pi_pRaster)
                bSelected = true;
            itr++;
        }

        // verify if multi-selection (Shift)
        if (nFlags & MK_SHIFT)
        {
            // if the raster is not already selected add it
            // to the selection list and draw the contour
            if (!bSelected)
            {
                m_Selection.push_back(pi_pRaster);
                HVEShape Shape(*(pi_pRaster->GetEffectiveShape()));
                DrawContour(&DC, Shape);
            }
        }
        else
        {
            // clear the previous selection
            OnUnselect();

            // clear the list
            m_Selection.clear();

            // Insert the raster in the vector
            m_Selection.push_back(pi_pRaster);

            // If the raster is not already selected, 
            // draw the contour
            HVEShape Shape(*(pi_pRaster->GetEffectiveShape()));
            DrawContour(&DC, Shape);
        }
    }
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::MoveRaster(MoveStep pi_Step, CPoint pi_Point)
{
    static HVEShape      OriginalShape(g_pAIRefCoordSys);
    static HVEShape      MoveShape(g_pAIRefCoordSys);
    static HGF2DLocation PreviousLocation(g_pAIRefCoordSys);
    static HGF2DLocation OriginalLocation(g_pAIRefCoordSys);

    switch(pi_Step)
    {
        case MS_START:
            {
                // capture the mouse
                m_pView->SetCapture();

                // Get the shape of the selected raster
                MoveShape = *(m_pCurrentRaster->GetEffectiveShape());
                OriginalShape = MoveShape;

                // set the origin
                PreviousLocation = HGF2DLocation((double)pi_Point.x,
                                                 (double)pi_Point.y,
                                                 m_pView->GetCoordSys());
                OriginalLocation = PreviousLocation;
            }
            break;


        case MS_MOVE:
            {
                HASSERT(m_pCurrentRaster != 0);

                // Undraw the previous contour
                CClientDC dc(m_pView);
                DrawContour(&dc, MoveShape);

                // create the current location of the cursor
                HGF2DLocation CurrentLocation((double)pi_Point.x,
                                              (double)pi_Point.y,
                                              m_pView->GetCoordSys());

                // Bring the previous location and the current location in the ref coord sys
                PreviousLocation.ChangeCoordSys(g_pAIRefCoordSys);
                CurrentLocation.ChangeCoordSys(g_pAIRefCoordSys);

                // set the shape in ref coord sys
                MoveShape.ChangeCoordSys(g_pAIRefCoordSys);

                // Create translation model
                HGF2DTranslation NewModel (HGF2DDisplacement(CurrentLocation - PreviousLocation));

                // "Move" the shape
                MoveShape.SetCoordSys(new HGF2DCoordSys(NewModel, MoveShape.GetCoordSys()));
            
                // redraw the contour
                DrawContour(&dc, MoveShape);
                PreviousLocation = CurrentLocation;
            }
            break;

        case MS_END:
            {
                HASSERT(m_pCurrentRaster != 0);

                // create the current location of the cursor
                HGF2DLocation CurrentLocation((double)pi_Point.x,
                                              (double)pi_Point.y,
                                              m_pView->GetCoordSys());

                // calculate the displacement in the raster's coord sys
                CurrentLocation.ChangeCoordSys(m_pCurrentRaster->GetCoordSys());
                OriginalLocation.ChangeCoordSys(m_pCurrentRaster->GetCoordSys());
                HGF2DDisplacement FinalDisplacement(CurrentLocation - OriginalLocation);

                // make the operation
                if ((FinalDisplacement.GetDeltaX() != 0.0) ||
                    (FinalDisplacement.GetDeltaY() != 0.0) )
                {
                    // Tell view we're about to do something
                    m_pView->BeginOperation();

                    // Get the extent before the operation
                    HVEShape Before(*(m_pCurrentRaster->GetEffectiveShape()));
                
                    // move the raster
                    HFCPtr<HRARaster> pRaster = DoMove(m_pCurrentRaster, FinalDisplacement);

                    // if the result is 0, the operation was canceled
                    if (pRaster != 0)
                    {
                        // Remove from the selection
                        RemoveFromSelection(pRaster);

                        // update the view
                        HVEShape After(*(pRaster->GetEffectiveShape()));
                        m_pView->RedrawImage(Before, After);
                    }
                    else
                    {
                        // remove the last contour
                        CClientDC dc(m_pView);
                        DrawContour(&dc, MoveShape);

                        // redraw the selection around the raster
                        HVEShape Shape(*(m_pCurrentRaster->GetEffectiveShape()));
                        DrawContour(&dc, Shape);
                    }

                    // Tell the view we're done
                    m_pView->EndOperation();
               }
               m_pCurrentRaster = 0;

                // Realease the mouse
               ::ReleaseCapture();
            }
            break;

        case MS_ABORT:
            {
                HASSERT(m_pCurrentRaster != 0);

                // Undraw the previous contour
                CClientDC dc(m_pView);
                DrawContour(&dc, MoveShape);

                // Remove the raster from the selection
                RemoveFromSelection(m_pCurrentRaster);

                // Stop it
                m_pCurrentRaster = 0;

                // Realease the mouse
                ::ReleaseCapture();
            }
            break;
    }
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnSendToBack()
{
    HFCPtr<HRARaster> pRaster;
    HTREEITEM         SelObject;
    HPRECONDITION(!m_Selection.empty());

    // Verify if the current selection is the document item
    SelObject = m_pDoc->GetSelectedObject();
    HASSERT(SelObject != NULL);
    HASSERT(SelObject != m_pDoc->GetDocumentObject());

    // get the currently selected object from the document
    pRaster = m_pDoc->GetRaster(SelObject);
    HASSERT(pRaster != 0);
    HASSERT(pRaster->GetClassID() == HIMMosaic::CLASS_ID);

    // Tell view we're about to do something
    m_pView->BeginOperation();

    // Get the extent before the operation
    HVEShape Before(GetSelectionExtent());

    // Ask the document to do the job
    DoSendToBack((HFCPtr<HIMMosaic>&)pRaster, SelObject, m_Selection);

    // Tell the view we're done
    m_pView->EndOperation();

    // redraw the view
    HVEShape After(GetSelectionExtent());
    m_pView->RedrawImage(Before, After);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnBringToFront()
{
    HFCPtr<HRARaster> pRaster;
    HTREEITEM         SelObject;
    HPRECONDITION(!m_Selection.empty());

    // Verify if the current selection is the document item
    SelObject = m_pDoc->GetSelectedObject();
    HASSERT(SelObject != NULL);
    HASSERT(SelObject != m_pDoc->GetDocumentObject());

    // get the currently selected object from the document
    pRaster = m_pDoc->GetRaster(SelObject);
    HASSERT(pRaster != 0);
    HASSERT(pRaster->GetClassID() == HIMMosaic::CLASS_ID);

    // Tell view we're about to do something
    m_pView->BeginOperation();

    // Get the extent before the operation
    HVEShape Before(GetSelectionShape());

    // Ask the document to do the job
    DoBringToFront((HFCPtr<HIMMosaic>&)pRaster, SelObject, m_Selection);

    // Tell the view we're done
    m_pView->EndOperation();

    // Ask the document to do the job
    DoBringToFront((HFCPtr<HIMMosaic>&)pRaster, SelObject, m_Selection);

    // redraw the view
    HVEShape After(GetSelectionShape());
    m_pView->RedrawImage(Before, After);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnMoveDown()
{
    HFCPtr<HRARaster> pRaster;
    HTREEITEM         SelObject;
    HPRECONDITION(!m_Selection.empty());

    // Verify if the current selection is the document item
    SelObject = m_pDoc->GetSelectedObject();
    HASSERT(SelObject != NULL);
    HASSERT(SelObject != m_pDoc->GetDocumentObject());

    // get the currently selected object from the document
    pRaster = m_pDoc->GetRaster(SelObject);
    HASSERT(pRaster != 0);
    HASSERT(pRaster->GetClassID() == HIMMosaic::CLASS_ID);

    // Tell view we're about to do something
    m_pView->BeginOperation();

    // Get the extent before the operation
    HVEShape Before(GetSelectionShape());

    // Ask the document to do the job
    DoMoveDown((HFCPtr<HIMMosaic>&)pRaster, SelObject, m_Selection);

    // Tell the view we're done
    m_pView->EndOperation();

    // redraw the view
    HVEShape After(GetSelectionShape());
    m_pView->RedrawImage(Before, After);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnMoveUp()
{
    HFCPtr<HRARaster> pRaster;
    HTREEITEM         SelObject;
    HPRECONDITION(!m_Selection.empty());

    // Verify if the current selection is the document item
    SelObject = m_pDoc->GetSelectedObject();
    HASSERT(SelObject != NULL);
    HASSERT(SelObject != m_pDoc->GetDocumentObject());

    // get the currently selected object from the document
    pRaster = m_pDoc->GetRaster(SelObject);
    HASSERT(pRaster != 0);
    HASSERT(pRaster->GetClassID() == HIMMosaic::CLASS_ID);

    // Tell view we're about to do something
    m_pView->BeginOperation();

    // Get the extent before the operation
    HVEShape Before(GetSelectionShape());

    // Ask the document to do the job
    DoMoveUp((HFCPtr<HIMMosaic>&)pRaster, SelObject, m_Selection);

    // Tell the view we're done
    m_pView->EndOperation();

    // redraw the view
    HVEShape After(GetSelectionShape());
    m_pView->RedrawImage(Before, After);
}


 //-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnRotate()
{
    HPRECONDITION(!m_Selection.empty() && (m_Selection.size() == 1));

    // create the dialog
    CRotationDialog dlg;
    dlg.m_Angle = 0.0;
    dlg.m_Affinity = 0.0;

    // display the dialog
    if ((dlg.DoModal() == IDOK)  && ((dlg.m_Angle != 0) || (dlg.m_Affinity != 0)))
    {
		if (((dlg.m_Affinity < 90) || (dlg.m_Affinity > 270))   // -->> Affinity values are limited here due to technical constraints 
		 && ((dlg.m_Affinity >= 0) && (dlg.m_Affinity <= 360))) // in "SetAnorthogonality" method.
															    // Note: We should have to set affinity by the way of a georef matrix
															    // to avoid this problem.
		{
			// Verify if the current selection is the document item
			HTREEITEM SelObject = m_pDoc->GetSelectedObject();
			HASSERT(SelObject != NULL);
			HASSERT(SelObject != m_pDoc->GetDocumentObject());

			// get the selected in the object
			HFCPtr<HRARaster> pRaster = m_Selection.front();
			HASSERT(pRaster != 0);

			// Tell view we're about to do something
			m_pView->BeginOperation();

			// Get the extent before the operation
			HVEShape Before(*(pRaster->GetEffectiveShape()));

			// Perform the rotation
			HFCPtr<HRARaster> pRotate = DoRotate(SelObject, pRaster, dlg.m_Angle, dlg.m_Affinity);

			// if the result is 0, the operation was canceled
			if (pRotate != 0)
			{
				// redraw the view
				HVEShape After(*(pRotate->GetEffectiveShape()));
				m_pView->RedrawImage(Before, After);
			}

			// Tell the view we're done
			m_pView->EndOperation();

			m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
		}
		else
			AfxMessageBox(_TEXT("Affinity value is invalid!"));
	}
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnRemove()
{

    CTreeCtrl* pTree = m_pDoc->GetTreeCtrl();
    HTREEITEM SelObject = m_pDoc->GetSelectedObject();
    HASSERT(SelObject != NULL);
    HASSERT(SelObject != m_pDoc->GetDocumentObject());

    // get the currently selected object from the document
    HFCPtr<HRARaster> pRaster = m_pDoc->GetRaster(SelObject);
    HASSERT(pRaster != 0);

    HTREEITEM selParent = pTree->GetParentItem(SelObject);
    HFCPtr<HIMMosaic> pMosaic =  static_cast<HIMMosaic*>( m_pDoc->GetRaster(selParent).GetPtr());

    // Tell view we're about to do something
    m_pView->BeginOperation();

    // get the extent of the raster to remove
    HVEShape Before(*((*pRaster).GetEffectiveShape()));

    pMosaic->Remove(pRaster);

    // Ask the document to do the job
    m_pDoc->RemoveTreeItem(SelObject);
    
    m_pDoc->SetModifiedFlag(true);

    // Tell the view we're done
    m_pView->EndOperation();

    // redraw the view
    m_pView->RedrawImage(Before, Before);  // no after, so use the same
    
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnSelectAll()
{
    CClientDC   DC(m_pView);

    // Verify if the current selection is the document item
    HTREEITEM SelObject = m_pDoc->GetSelectedObject();
    HASSERT(SelObject != NULL);
    HASSERT(SelObject != m_pDoc->GetDocumentObject());

    // get the currently selected object from the document
    HFCPtr<HRARaster> pRaster = m_pDoc->GetRaster(SelObject);
    HASSERT(pRaster != 0);

    // if the raster is a mosaic, then select its source otherwise
    // select it
    if (pRaster->GetClassID() == HIMMosaic::CLASS_ID)
    {
        HFCPtr<HIMMosaic>& pMosaic = (HFCPtr<HIMMosaic>&)pRaster;
        HIMMosaic::IteratorHandle Iterator = pMosaic->StartIteration();

        for (uint32_t i = 0; i < pMosaic->CountElements(Iterator); i++)
        {
            // get the current element
            HFCPtr<HRARaster> pRaster = pMosaic->GetElement(Iterator);
            HASSERT(pRaster != 0);

            // Select the raster
            SelectRaster(MK_SHIFT, pRaster);

            // get the next raster
            pMosaic->Iterate(Iterator);
        }

        pMosaic->StopIteration(Iterator);
    }
    else
    {
        // select the raster directly since it is the only one shown
        SelectRaster(MK_SHIFT, pRaster);
    }

}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnUnselect() 
{
    CClientDC DC(m_pView);

    // Draw the selection
    DrawSelection(&DC);

    // clear the selection
    m_Selection.clear();
}



//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::DrawSelection(CDC* pDC)
{
    // Place the selection contours
    if (!m_Selection.empty())
    {
        // parse each selected raster
        RasterSelection::iterator Iterator = m_Selection.begin();
        while(Iterator != m_Selection.end())
        {
            DrawContour(pDC, *((*Iterator)->GetEffectiveShape()));
            Iterator++;
        }
    }
}

                                   
//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::DrawContour(CDC* pDC, const HVEShape& pi_Shape)
{
    HPRECONDITION(pDC != NULL);

    // Set the Raster Operation to XOR
    uint32_t PreviousROP = pDC->SetROP2(R2_NOT);

    // Set the pen & brush
    CPen ContourPen(PS_INSIDEFRAME, 4, RGB(128, 128, 128));
    pDC->SelectObject(&ContourPen);
    pDC->SelectStockObject(HOLLOW_BRUSH);

    // Bring the shape to the view's coord sys. 
    HVEShape Shape(pi_Shape);
    Shape.ChangeCoordSys(m_pView->GetCoordSys());

    // Get the extent of the shape
    HGF2DExtent Extent(Shape.GetExtent());

#if 0
    // get the four corner's of the extent
    HGF2DLocation TopLeft       (pi_Shape.GetExtent().GetXMin(), 
                                 pi_Shape.GetExtent().GetYMin(), 
                                 pi_Shape.GetExtent().GetCoordSys());
    HGF2DLocation TopRight      (pi_Shape.GetExtent().GetXMax(), 
                                 pi_Shape.GetExtent().GetYMin(), 
                                 pi_Shape.GetExtent().GetCoordSys());
    HGF2DLocation BottomLeft    (pi_Shape.GetExtent().GetXMin(), 
                                 pi_Shape.GetExtent().GetYMax(), 
                                 pi_Shape.GetExtent().GetCoordSys());
    HGF2DLocation BottomRight   (pi_Shape.GetExtent().GetXMax(), 
                                 pi_Shape.GetExtent().GetYMax(), 
                                 pi_Shape.GetExtent().GetCoordSys());

    // Bring the corners into the view's coord sys
    TopLeft.ChangeCoordSys(m_pView->GetCoordSys());
    BottomRight.ChangeCoordSys(m_pView->GetCoordSys());
    TopRight.ChangeCoordSys(m_pView->GetCoordSys());
    BottomLeft.ChangeCoordSys(m_pView->GetCoordSys());

    // generate the list of points for the polyline function
#if 0
    POINT Corners[5];
    Corners[0].x = (int32_t )TopLeft.GetX();
    Corners[0].y = (int32_t )TopLeft.GetY();
    Corners[1].x = (int32_t )TopRight.GetX();
    Corners[1].y = (int32_t )TopRight.GetY();
    Corners[2].x = (int32_t )BottomRight.GetX();
    Corners[2].y = (int32_t )BottomRight.GetY();
    Corners[3].x = (int32_t )BottomLeft.GetX();
    Corners[3].y = (int32_t )BottomLeft.GetY();
    Corners[4].x = Corners[0].x;
    Corners[4].y = Corners[0].y;

    // Draw
    pDC->Polygon(Corners, 5);
#else
    pDC->Rectangle(min(TopLeft.GetX(), BottomLeft.GetX()),
                   min(TopLeft.GetY(), TopRight.GetY()),
                   max(TopRight.GetX(), BottomRight.GetX()),
                   max(BottomLeft.GetY(), BottomRight.GetY()));
#endif
#else

    // draw the rectangle
    pDC->Rectangle((int)Extent.GetXMin(), (int)Extent.GetYMin(),
                   (int)Extent.GetXMax(), (int)Extent.GetYMax());
#endif

    // replace the DC properties
    pDC->SetROP2(PreviousROP);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::DoSendToBack(HFCPtr<HIMMosaic>& pi_pMosaic,
                                           HTREEITEM          pi_ParentItem,
                                           RasterSelection&   pi_Selection)
{
    CTreeCtrl* pTree = m_pDoc->GetTreeCtrl();
    HPRECONDITION(pi_pMosaic != 0);
    HPRECONDITION(pi_ParentItem != NULL);
    HPRECONDITION(!pi_Selection.empty());

    // sort the god damn vector
    sort(pi_Selection.begin(), pi_Selection.end(), 
         TreeListOrder(m_pDoc, pTree, pi_ParentItem));

    // parse each item from last to first
    RasterSelection::iterator Iterator = pi_Selection.begin();
    while (Iterator != pi_Selection.end())
    {
        // get the current selected raster's associated tree item
        HTREEITEM CurrentItem = m_pDoc->GetTreeItem(pi_ParentItem, *Iterator);
        HASSERT(CurrentItem != NULL);

        // move the content of the current item to the destination
        m_pDoc->MoveTreeItem(pi_ParentItem, CurrentItem, TVI_LAST);

        // Perform the required operation
        pi_pMosaic->SendToBack(*Iterator);

        // tell the document that it has changed
        m_pDoc->SetModifiedFlag(true);

        // go to the next raster
        Iterator++;
    }
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::DoBringToFront(HFCPtr<HIMMosaic>& pi_pMosaic,
                                             HTREEITEM          pi_ParentItem,
                                             RasterSelection&   pi_Selection)
{
    CTreeCtrl* pTree = m_pDoc->GetTreeCtrl();
    HPRECONDITION(pi_pMosaic != 0);
    HPRECONDITION(pi_ParentItem != NULL);
    HPRECONDITION(!pi_Selection.empty());

    // sort the god damn vector
    sort(pi_Selection.begin(), pi_Selection.end(), 
         TreeListOrder(m_pDoc, pTree, pi_ParentItem));

    // parse each item from first to last
    RasterSelection::reverse_iterator Iterator = pi_Selection.rbegin();
    while (Iterator != pi_Selection.rend())
    {
        // get the current selected raster's associated tree item
        HTREEITEM CurrentItem = m_pDoc->GetTreeItem(pi_ParentItem, *Iterator);
        HASSERT(CurrentItem != NULL);

        // move the content of the current item to the destination
        m_pDoc->MoveTreeItem(pi_ParentItem, CurrentItem, TVI_FIRST);

        // Perform the required operation
        pi_pMosaic->BringToFront(*Iterator);

        // tell the document that it has changed
        m_pDoc->SetModifiedFlag(true);

        // go to the next raster
        Iterator++;
    }
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::DoMoveDown(HFCPtr<HIMMosaic>& pi_pMosaic,
                                         HTREEITEM          pi_ParentItem,
                                         RasterSelection&   pi_Selection)
{
    CTreeCtrl* pTree = m_pDoc->GetTreeCtrl();
    HPRECONDITION(pi_pMosaic != 0);
    HPRECONDITION(pi_ParentItem != NULL);
    HPRECONDITION(!pi_Selection.empty());

    // sort the god damn vector
    sort(pi_Selection.begin(), pi_Selection.end(), 
         TreeListOrder(m_pDoc, pTree, pi_ParentItem));

    // parse each item from last to first
    RasterSelection::iterator Iterator = pi_Selection.begin();
    while (Iterator != pi_Selection.end())
    {
        // get the current selected raster's associated tree item
        HTREEITEM CurrentItem = m_pDoc->GetTreeItem(pi_ParentItem, *Iterator);
        HASSERT(CurrentItem != NULL);

        // get the next item to move after
        HTREEITEM NextItem = pTree->GetNextSiblingItem(CurrentItem);
        if (NextItem != NULL)
        {
            // move the content of the current item to the destination
            m_pDoc->MoveTreeItem(pi_ParentItem, CurrentItem, NextItem);

            // Perform the required operation
            pi_pMosaic->Sink(*Iterator);

            // tell the document that it has changed
            m_pDoc->SetModifiedFlag(true);
        }

        // go to the next raster
        Iterator++;
    }
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::DoMoveUp(HFCPtr<HIMMosaic>& pi_pMosaic,
                                       HTREEITEM          pi_ParentItem,
                                       RasterSelection&   pi_Selection)
{
    CTreeCtrl* pTree = m_pDoc->GetTreeCtrl();
    HPRECONDITION(pi_pMosaic != 0);
    HPRECONDITION(pi_ParentItem != NULL);
    HPRECONDITION(!pi_Selection.empty());

    // sort the god damn vector
    sort(pi_Selection.begin(), pi_Selection.end(), 
         TreeListOrder(m_pDoc, pTree, pi_ParentItem));

    // parse each item from first to last
    RasterSelection::reverse_iterator Iterator = pi_Selection.rbegin();
    while (Iterator != pi_Selection.rend())
    {
        // get the current selected raster's associated tree item
        HTREEITEM CurrentItem = m_pDoc->GetTreeItem(pi_ParentItem, *Iterator);
        HASSERT(CurrentItem != NULL);

        // get the previous item to move before
        HTREEITEM PrevItem = pTree->GetPrevSiblingItem(CurrentItem);
        if (PrevItem != NULL)
        {
            // to insert after the previous item, find the previous
            // item's previous item and insert after it.  If it does
            // not exists, insert after TVI_FIRST
            HTREEITEM InsertAfter = pTree->GetPrevSiblingItem(PrevItem);
            if (InsertAfter == NULL)
                InsertAfter = TVI_FIRST;

            // move the content of the current item to the destination
            m_pDoc->MoveTreeItem(pi_ParentItem, CurrentItem, InsertAfter);

            // Perform the required operation
            pi_pMosaic->Float(*Iterator);

            // tell the document that it has changed
            m_pDoc->SetModifiedFlag(true);
        }

        // go to the next raster
        Iterator++;
    }
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::DoRemove(HFCPtr<HIMMosaic>& pi_pMosaic,
                                       HTREEITEM          pi_ParentItem,
                                       RasterSelection&   pi_Selection)
{
    CTreeCtrl* pTree = m_pDoc->GetTreeCtrl();
    HPRECONDITION(pi_pMosaic != 0);
    HPRECONDITION(pi_ParentItem != NULL);
    HPRECONDITION(!pi_Selection.empty());

    // sort the god damn vector
    sort(pi_Selection.begin(), pi_Selection.end(), 
         TreeListOrder(m_pDoc, pTree, pi_ParentItem));

    // parse each item from first to last
    RasterSelection::iterator Iterator = pi_Selection.begin();
    while (Iterator != pi_Selection.end())
    {
        // get the current selected raster's associated tree item
        HTREEITEM CurrentItem = m_pDoc->GetTreeItem(pi_ParentItem, *Iterator);
        HASSERT(CurrentItem != NULL);

        // Remove the current item
        m_pDoc->RemoveTreeItem(CurrentItem);
        
        // Perform the required operation
        pi_pMosaic->Remove(*Iterator);

        // tell the document that it has changed
        m_pDoc->SetModifiedFlag(true);

        // go to the next raster
        Iterator++;
    }
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
HFCPtr<HRARaster>
CActiveImageDefaultMode::DoMove(HFCPtr<HRARaster>& pi_pRaster,
                                HGF2DDisplacement  pi_Displacement)
{
    HFCPtr<HRARaster> pMove = pi_pRaster;
    HPRECONDITION(pi_pRaster != 0);

    // verify if the raster is a link
    if (m_pDoc->IsRasterLinked(pi_pRaster))
    {
        // verify what the user wants to do
        switch (AfxMessageBox(_TEXT("The raster is a link to an external store\nPlace it in a reference?"), MB_YESNOCANCEL))
        {
            case IDYES:
                // Create a reference on the raster
                pMove = m_pDoc->PlaceInReference(pi_pRaster);
		        HASSERT(pMove != 0);

                // remove the raster from the selection and add the reference
                RemoveFromSelection(pi_pRaster);
                m_Selection.push_back(pMove);
                break;

            case IDCANCEL:
                pMove = 0;
                break;
        }
    }

    if (pMove != 0)
    {
        // move the god damn thing
        pMove->Move(pi_Displacement);

        // tell the document that it has changed
        m_pDoc->SetModifiedFlag(true);
    }

    return (HFCPtr<HRARaster>&)pMove;
}



//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
HFCPtr<HRARaster>
CActiveImageDefaultMode::DoRotate(HTREEITEM          pi_ParentItem,
                                  HFCPtr<HRARaster>& pi_pRaster,
                                  double            pi_Angle,
                                  double            pi_Affinity)
{
    HFCPtr<HRARaster> pRotate = pi_pRaster;
    HPRECONDITION(pi_ParentItem != NULL);
    HPRECONDITION(pi_pRaster != 0);

    // verify if the raster is a link or a propriety object
    // If the raser is a link, ask the user if he/she wants to place the 
    // raster in a ReferenceToRaster object
    if (m_pDoc->IsRasterLinked(pi_pRaster))
    {
        // ask the user what he wants to do
        switch(AfxMessageBox(_TEXT("The raster is a link to an external store\nPlace it in a reference?"), MB_YESNOCANCEL))
        {
            case IDYES:
                pRotate = m_pDoc->PlaceInReference(pi_pRaster);
		        HASSERT(pRotate != 0);

                // remove the raster from the selection and add the reference
                RemoveFromSelection(pi_pRaster);
                m_Selection.push_back(pRotate);
                break;

            case IDCANCEL:
                pRotate = 0;
        }
    }

    if (pRotate != 0)
    {
        // Get base information
        HFCPtr<HGF2DCoordSys> pImageCS = pRotate->GetCoordSys();
        HFCPtr<HGF2DTransfoModel> pBaseToImage = m_pDoc->GetWorldCluster()->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD)->GetTransfoModelTo(pImageCS);

        // Take image's origin for rotation center
        HVEShape ImageShape(*pRotate->GetEffectiveShape());
        ImageShape.ChangeCoordSys(pImageCS);
        double OriginX = ImageShape.GetExtent().GetXMin();
        double OriginY = ImageShape.GetExtent().GetYMin();
        pBaseToImage->ConvertInverse(&OriginX, &OriginY);

        // Create the rotation model
        HFCPtr<HGF2DAffine> pRotation(new HGF2DAffine());
        pRotation->AddRotation(pi_Angle * PI/180.0, OriginX, OriginY);

        pRotation->SetAnorthogonality(pi_Affinity * PI/180.0);

        // Take that rotation and base it on the image's system
        HFCPtr<HGF2DTransfoModel> pRotationForImage = pBaseToImage->ComposeInverseWithDirectOf(*pRotation);
        pRotationForImage = pRotationForImage->ComposeInverseWithInverseOf(*pBaseToImage);
        HFCPtr<HGF2DTransfoModel> pSimplifiedRotationForImage(pRotationForImage->CreateSimplifiedModel());

        // Change the image if it is a StoredRaster...
        if (pRotate->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
        {
            HFCPtr<HGF2DTransfoModel> pModel(((HFCPtr<HRAStoredRaster>&)pRotate)->GetTransfoModel());
            pModel = pModel->ComposeInverseWithDirectOf(pSimplifiedRotationForImage != 0 ?
                                                        *pSimplifiedRotationForImage :
                                                        *pRotationForImage);

            ((HFCPtr<HRAStoredRaster>&)pRotate)->SetTransfoModel(*pModel);
        }
        else
        {
            pRotate->SetCoordSys(new HGF2DCoordSys(pSimplifiedRotationForImage != 0 ? 
                                                        *pSimplifiedRotationForImage :
                                                        *pRotationForImage, 
                                                   pImageCS));
        }

        // tell the document that it has changed
        m_pDoc->SetModifiedFlag(true);
    }

    return (HFCPtr<HRARaster>&)pRotate;
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
HGF2DExtent CActiveImageDefaultMode::GetSelectionExtent() const
{
    HGF2DExtent Extent(g_pAIRefCoordSys);
    HPRECONDITION(!m_Selection.empty());

    // Place the selection contours
    RasterSelection::const_iterator Iterator = m_Selection.begin();
    while(Iterator != m_Selection.end())
    {
        // get the current item's extent
        HGF2DExtent CurrentExtent((*Iterator)->GetEffectiveShape()->GetExtent());

        // if the current extent is the fist copy to our extent otherwise
        // Union the current extent to our extent
        if (Iterator == m_Selection.begin())
            Extent = CurrentExtent;
        else
        {
            Extent.ChangeCoordSys(CurrentExtent.GetCoordSys());
            Extent.Union(CurrentExtent);
        }

        // proceed to the next item
        Iterator++;
    }

    return Extent;
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
HVEShape CActiveImageDefaultMode::GetSelectionShape() const
{
    HVEShape Shape(g_pAIRefCoordSys);
    HPRECONDITION(!m_Selection.empty());

    // Place the selection contours
    RasterSelection::const_iterator Iterator = m_Selection.begin();
    while(Iterator != m_Selection.end())
    {
        // get the current item's extent
        HVEShape CurrentShape(*((*Iterator)->GetEffectiveShape()));

        // if the current extent is the fist copy to our extent otherwise
        // Union the current extent to our extent
        if (Iterator == m_Selection.begin())
            Shape = CurrentShape;
        else
        {
            Shape.ChangeCoordSys(CurrentShape.GetCoordSys());
            Shape.Unify(CurrentShape);
        }

        // proceed to the next item
        Iterator++;
    }

    return Shape;
}

//-----------------------------------------------------------------------------
// Private: OnPixelReplacerFilter
//-----------------------------------------------------------------------------

void CActiveImageDefaultMode::OnPixelReplacerFilter()
{
    ColorReplacerDlg Dlg(this, m_pView, m_pDoc);
    Dlg.DoModal();

    HFCPtr<HRPColorReplacerFilter> pColorReplacer(new HRPColorReplacerFilter);
    pColorReplacer->SetNewColor(0, 255, 0);
    
    pColorReplacer->AddColors   (Dlg.GetSelectedRGBSet());
    pColorReplacer->RemoveColors(Dlg.GetSelectedRemoveRGBSet());

    OnFilter((HFCPtr<HRPFilter>&)pColorReplacer);
}

//-----------------------------------------------------------------------------
// Public: OnPixelReplacerFilter
//-----------------------------------------------------------------------------

HFCPtr<HRABitmap> CActiveImageDefaultMode::ColorSelection()
{
    CRectTracker tracker;
    HFCPtr<HRABitmap> pDibRaster;
    HFCPtr<HRARaster> pSelectedObject;

	CPoint p(10,10);
    if (tracker.TrackRubberBand(m_pView, p, true))
    {
        CRect rc (tracker.m_rect);
        rc.NormalizeRect();
        
        HGF2DLocation MinCorner = m_pView->GetLocation(rc.TopLeft());
        HGF2DLocation MaxCorner = m_pView->GetLocation(rc.BottomRight());
        
        HGF2DExtent Extent(min(MinCorner.GetX(), MaxCorner.GetX()),
                           min(MinCorner.GetY(), MaxCorner.GetY()),
                           max(MinCorner.GetX(), MaxCorner.GetX()),
                           max(MinCorner.GetY(), MaxCorner.GetY()),
                           MinCorner.GetCoordSys());
    
        // Clipborad Disponible
        pDibRaster =  HRABitmap::Create (rc.Width(), 
                                    rc.Height(), 
                                    0, 
                                    m_pView->GetCoordSys(),
                                    new HRPPixelTypeV24B8G8R8(),
                                    32); 

        HVEShape Shape(Extent);
        Shape.ChangeCoordSys(pDibRaster->GetPhysicalCoordSys());
        pDibRaster->Move(HGF2DDisplacement(Shape.GetExtent().GetXMin(), 
                                           Shape.GetExtent().GetYMin()));
    
        pSelectedObject = m_pDoc->GetRaster(m_pDoc->GetSelectedObject());
        pDibRaster->CopyFrom(*pSelectedObject, HRACopyFromOptions());
    }
    return pDibRaster;
}

//-----------------------------------------------------------------------------
// Private: OnAlphaReplacerFilter
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnAlphaReplacerFilter()
{
    ColorReplacerDlg Dlg(this, m_pView, m_pDoc);
    Dlg.DoModal();

    HGFRGBSet rgbSet = Dlg.GetSelectedRGBSet();
    HFCPtr<HGFRGBSet> pRgbSet(new HGFRGBSet(rgbSet));
    ListHRPAlphaRange listAlphaRanges;
    Byte alphaForRange = 100;
    HRPAlphaRange alphaRange ((HFCPtr<HGFColorSet>&)pRgbSet, alphaForRange);
    listAlphaRanges.push_back(alphaRange);
    Byte defaultAlpha = 200;
    HFCPtr<HRPAlphaReplacer> pAlphaReplacer(new HRPAlphaReplacer(defaultAlpha, listAlphaRanges));

    OnFilter((HFCPtr<HRPFilter>&)pAlphaReplacer);
}

//-----------------------------------------------------------------------------
// Private: OnAlphaComposerFilter
//-----------------------------------------------------------------------------
void CActiveImageDefaultMode::OnAlphaComposerFilter()
{
    ColorReplacerDlg Dlg(this, m_pView, m_pDoc);
    Dlg.DoModal();

    HGFRGBSet rgbSet = Dlg.GetSelectedRGBSet();
    HFCPtr<HGFRGBSet> pRgbSet(new HGFRGBSet(rgbSet));
    ListHRPAlphaRange listAlphaRanges;
    Byte alphaForRange = 50;
    HRPAlphaRange alphaRange ((HFCPtr<HGFColorSet>&)pRgbSet, alphaForRange);
    listAlphaRanges.push_back(alphaRange);
    Byte defaultAlpha = 150;
    HFCPtr<HRPAlphaComposer> pAlphaComposer(new HRPAlphaComposer(defaultAlpha, listAlphaRanges));

    OnFilter((HFCPtr<HRPFilter>&)pAlphaComposer);
}

//-----------------------------------------------------------------------------
// Private: IsRasterTransparent 
// 
//-----------------------------------------------------------------------------
bool CActiveImageDefaultMode::IsRasterTransparent(const HFCPtr<HRARaster>& pi_rpRaster) const
{
    if(pi_rpRaster->IsCompatibleWith(HRAPixelTypeReplacer::CLASS_ID))
        return true;

    if(pi_rpRaster->IsCompatibleWith(HIMFilteredImage::CLASS_ID))
    {
        HFCPtr<HRPFilter> pFilter(((HFCPtr<HIMFilteredImage>&)pi_rpRaster)->GetFilter());

        if (pFilter->IsCompatibleWith(HRPAlphaReplacer::CLASS_ID) ||
            pFilter->IsCompatibleWith(HRPAlphaComposer::CLASS_ID))
            return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

BOOL CActiveImageDefaultMode::IsRasterPixelTypeV16Compatbile(const HFCPtr<HRARaster>& pi_rpRaster) const
{
    HPRECONDITION (pi_rpRaster != 0);

    bool  IsCompatible = false;
    HFCPtr<HRPPixelType> pPixelType = pi_rpRaster->GetPixelType();


    if ( pPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID       ) || 
         pPixelType->IsCompatibleWith(HRPPixelTypeV48R16G16B16::CLASS_ID    ) || 
         pPixelType->IsCompatibleWith(HRPPixelTypeV64R16G16B16A16::CLASS_ID ) ||
         pPixelType->IsCompatibleWith(HRPPixelTypeV64R16G16B16X16::CLASS_ID))
         IsCompatible = true;

    return IsCompatible;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CActiveImageDefaultMode::OnDEMFilter(HRPDEMFilter::Style style, bool hillShadingState)
    {
    HTREEITEM SelObject = m_pDoc->GetSelectedObject();

    // Verify that the current selection is the document item
    if(SelObject == NULL || SelObject == m_pDoc->GetDocumentObject())
        return false;

    // get the currently selected object from the document
    HFCPtr<HRARaster> pRaster = m_pDoc->GetRaster(SelObject);
    if (pRaster == NULL)
        return false;
    
    // Remove previous DEM decoration
    if(pRaster->IsCompatibleWith(HRADEMRaster::CLASS_ID))
        pRaster = static_cast<HRADEMRaster*>(pRaster.GetPtr())->GetSource();

    if(!pRaster->IsCompatibleWith(HRAReferenceToStoredRaster::CLASS_ID) && !pRaster->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
        return false;

    if(pRaster->GetPixelType()->GetChannelOrg().CountChannels() != 1)
        return false;

    if(pRaster->GetStore() == NULL || !pRaster->GetStore()->IsCompatibleWith(HRSObjectStore::CLASS_ID))
        return false;

    HFCPtr<HRFRasterFile > pRasterFile = static_cast<HRSObjectStore*>(pRaster->GetStore())->GetRasterFile();

    HRPDEMFilter::HillShadingSettings hillShading;
    hillShading.SetHillShadingState(hillShadingState);

    HFCPtr<HRPDEMFilter> pDemFilter = new HRPDEMFilter(hillShading, style);

    switch(style)
        {
        case HRPDEMFilter::Style_Elevation:
            {
            HRFAttributeMinSampleValue const* pMinValueTag = pRasterFile->GetPageDescriptor(0)->FindTagCP<HRFAttributeMinSampleValue>();
            HRFAttributeMaxSampleValue const* pMaxValueTag = pRasterFile->GetPageDescriptor(0)->FindTagCP<HRFAttributeMaxSampleValue>();
            if(pMinValueTag!=NULL && pMaxValueTag!=NULL)
                {
                pDemFilter->SetClipToEndValues(true);

                double minValue = pMinValueTag->GetData()[0];     // First channel
                double maxValue = pMaxValueTag->GetData()[0];     // First channel

                double step = (maxValue-minValue) / 255;    // First step is minimum.
                HRPDEMFilter::UpperRangeValues upperRangeValues;
                for(uint32_t i=0; i < 256; ++i)
                    upperRangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(minValue+(i*step), HRPDEMFilter::RangeInfo((Byte)i,(Byte)i,(Byte)i,true)));
                pDemFilter->SetUpperRangeValues(upperRangeValues);
                }
            }
            break;
            
        case HRPDEMFilter::Style_SlopePercent:
            {
            HRPDEMFilter::UpperRangeValues rangeValues;  
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(3,   HRPDEMFilter::RangeInfo(   0, 255,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(5,   HRPDEMFilter::RangeInfo(  19, 235,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(7,   HRPDEMFilter::RangeInfo(  39, 215,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(10,  HRPDEMFilter::RangeInfo(  58, 196,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(15,  HRPDEMFilter::RangeInfo(  78, 176,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(20,  HRPDEMFilter::RangeInfo(  98, 156,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(25,  HRPDEMFilter::RangeInfo( 117, 137,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(30,  HRPDEMFilter::RangeInfo( 137, 117,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(35,  HRPDEMFilter::RangeInfo( 156,  98,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(40,  HRPDEMFilter::RangeInfo( 176,  78,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(50,  HRPDEMFilter::RangeInfo( 196,  58,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(60,  HRPDEMFilter::RangeInfo( 215,  39,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(75,  HRPDEMFilter::RangeInfo( 235,  19,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(100, HRPDEMFilter::RangeInfo( 255,   0,   0,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(150, HRPDEMFilter::RangeInfo( 255,   0, 127,true)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(250, HRPDEMFilter::RangeInfo( 255,   0, 255,true)));
            pDemFilter->SetUpperRangeValues(rangeValues); 
            }
            break;

        case HRPDEMFilter::Style_Aspect:
            {
            HRPDEMFilter::UpperRangeValues rangeValues;
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(15,  HRPDEMFilter::RangeInfo(  0,   0, 255,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(45,  HRPDEMFilter::RangeInfo(127,   0, 255,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(75,  HRPDEMFilter::RangeInfo(255,   0, 255,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(105, HRPDEMFilter::RangeInfo(255,   0, 127,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(135, HRPDEMFilter::RangeInfo(255,   0,   0,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(165, HRPDEMFilter::RangeInfo(255, 127,   0,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(195, HRPDEMFilter::RangeInfo(255, 255,   0,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(225, HRPDEMFilter::RangeInfo(127, 255,   0,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(255, HRPDEMFilter::RangeInfo(  0, 255,   0,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(285, HRPDEMFilter::RangeInfo(  0, 255, 127,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(315, HRPDEMFilter::RangeInfo(  0, 255, 255,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(345, HRPDEMFilter::RangeInfo(  0, 127, 255,TRUE)));
            rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(360, HRPDEMFilter::RangeInfo(  0, 0,   255,TRUE)));
            pDemFilter->SetUpperRangeValues(rangeValues); 
            }
            break;

        default:
            pDemFilter = NULL;
            break;
        }

    if(pDemFilter == NULL || pDemFilter->GetUpperRangeValues().empty())
        return false;

    if(pRaster->IsCompatibleWith(HRAReferenceToStoredRaster::CLASS_ID))
        pRaster = new HRADEMRaster(reinterpret_cast<HFCPtr<HRAReferenceToStoredRaster>&> (pRaster), 100.0, 100.0, new HGF2DIdentity(), pDemFilter);
    else if(pRaster->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
        pRaster = new HRADEMRaster(reinterpret_cast<HFCPtr<HRAStoredRaster>&> (pRaster), 100.0, 100.0, new HGF2DIdentity(), pDemFilter);

    m_pDoc->ReplaceRaster(SelObject, pRaster);

    m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0); 

    return true;
    }

     
