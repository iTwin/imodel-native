/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMDisplayHandlers.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <stdafx.h>
#include <RmgrTools/Tools/RscFileManager.h>
#include "DTMBinaryData.h"
#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include "DTMDataRefXAttribute.h"

USING_NAMESPACE_RASTER


BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

void DTMElementHandlerManagerInitialize();
void Initializations ();
void registerECExtension ();

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/10
//=======================================================================================
bool DTMElementSubDisplayHandler::SetSymbology (DTMElementSubHandler::SymbologyParams& params, DTMDrawingInfo& drawingInfo, ViewContextR context)
    {
    EditElementHandle eeh (drawingInfo.GetSymbologyElement(), true);

    ElemDisplayParamsP  elParams = context.GetCurrentDisplayParams();

    MSElementCR elm = *drawingInfo.GetOriginalElement().GetElementCP ();

    elParams->SetLevel ((params.GetLevelId() != LEVEL_BYCELL) ? params.GetLevelId() : elm.ehdr.level);
    elParams->SetLineColor ((params.GetSymbology().color != COLOR_BYCELL) ? params.GetSymbology().color : elm.hdr.dhdr.symb.color);
    elParams->SetWeight ((params.GetSymbology().weight != WEIGHT_BYCELL) ? params.GetSymbology().weight : elm.hdr.dhdr.symb.weight);
    elParams->SetLineStyle ((params.GetSymbology().style != STYLE_BYCELL) ? params.GetSymbology().style : elm.hdr.dhdr.symb.style);
    elParams->SetTransparency (1 - ((1 - elParams->GetTransparency ()) * (1 - params.GetTransparency())));
    elParams->SetMaterial (NULL);

    params.SetElementMat(eeh, elParams, context);

    DgnModelRefP    model = drawingInfo.GetSymbologyElement().GetModelRef();

    if (context.GetWantMaterials() && NULL == elParams->GetMaterial ())
        elParams->SetMaterial (MaterialManager::GetManagerR ().FindMaterialBySymbology (NULL, elParams->GetLevel (), elParams->GetLineColor (), *model, false, false, &context), true);

    // ReCookDisplayParams...

    if (model != context.GetCurrentModel ())
        {
        DisplayPathP displayPath = const_cast<DisplayPathP>(context.GetCurrDisplayPath ());
        DgnModelRefP oldRoot = displayPath->GetRoot ();
        displayPath->SetRoot (model);
        context.CookDisplayParams (); // Cook m_currDisplayParams into m_elemMatSymb. This will be stored in any cached presentations.
        context.CookDisplayParamsOverrides (); // Apply overrides to m_currDisplayParams and cook into override mat symb.
        context.ActivateOverrideMatSymb ();
        displayPath->SetRoot (oldRoot);
        }
    else
        {
        context.CookDisplayParams (); // Cook m_currDisplayParams into m_elemMatSymb. This will be stored in any cached presentations.
        context.CookDisplayParamsOverrides (); // Apply overrides to m_currDisplayParams and cook into override mat symb.
        context.ActivateOverrideMatSymb ();
        }

    ScanCriteriaCP scanCrit = context.GetScanCriteria();

    if (NULL == scanCrit)
        return true;

    return TestLevelIsVisible (elParams->GetLevel (), drawingInfo, context);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 1/15
//=======================================================================================
bool DTMElementSubDisplayHandler::TestLevelIsVisible (LevelId level, DTMDrawingInfo& drawingInfo, ViewContextR context)
    {
    MSElementCR elm = *drawingInfo.GetOriginalElement ().GetElementCP ();
    DgnModelRefP    model = drawingInfo.GetSymbologyElement ().GetModelRef ();
    int     elemLevel = ((level != LEVEL_BYCELL) ? level : elm.ehdr.level);

    if (drawingInfo.GetOriginalElement().GetModelRef() == drawingInfo.GetSymbologyElement().GetModelRef())
        return context.GetLevelClassMask()->levelBitMaskP->Test(elemLevel - 1);

    ViewportP vp = context.GetViewport();
    ViewInfoCP viewInfo = vp ? vp->GetViewInfoCP() : NULL;
    bool isDisplayed = false;
    if (nullptr != viewInfo)
        viewInfo->GetEffectiveLevelDisplay(isDisplayed, model, elemLevel);

    return isDisplayed;
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 11/10
//=======================================================================================
void DTMElementSubDisplayHandler::SetSymbology(DTMElementSubHandler::SymbologyParams& params, DTMDrawingInfo& drawingInfo, ViewContextR context, UInt32 color, int style, UInt32 weight)
    {
    EditElementHandle eeh (drawingInfo.GetSymbologyElement(), true);

    ElemDisplayParamsP  elParams = context.GetCurrentDisplayParams();

    MSElementCR elm = *drawingInfo.GetOriginalElement().GetElementCP ();

    elParams->SetLevel ((params.GetLevelId() != LEVEL_BYCELL) ? params.GetLevelId() : elm.ehdr.level);
    elParams->SetLineColor ((color != COLOR_BYCELL) ? color : elm.hdr.dhdr.symb.color);
    elParams->SetWeight ((weight != WEIGHT_BYCELL) ? weight : elm.hdr.dhdr.symb.weight);
    elParams->SetLineStyle ((style != STYLE_BYCELL) ? style : elm.hdr.dhdr.symb.style);
    elParams->SetTransparency (1 - ((1 - elParams->GetTransparency ()) * (1 - params.GetTransparency())));
    elParams->SetMaterial (NULL);

    params.SetElementMat(eeh, elParams, context);

    DgnModelRefP    model = drawingInfo.GetSymbologyElement().GetModelRef();

    if (context.GetWantMaterials() && NULL == elParams->GetMaterial ())
        elParams->SetMaterial (MaterialManager::GetManagerR ().FindMaterialBySymbology (NULL, elParams->GetLevel (), elParams->GetLineColor (), *model, false, false, &context), true);

    // ReCookDisplayParams...
    context.CookDisplayParams();
    }

RscFileManager::DllRsc*  g_TMHandlersResources;

WString          TerrainModelElementResources::GetString (UInt stringId)
    {
    return g_TMHandlersResources->GetString (stringId);
    }

static void dummy () {;}

struct DTMElementSubHandlerExtension : public DTMElementSubHandler::IDTMElementSubHandlerExtension
    {
    virtual bool _AddDTMTextStyle (EditElementHandle& elem, UInt32 textStyleId, UInt32 key) override
        {
        return AddDTMTextStyle (elem, textStyleId, key);
        }
    virtual void Bentley::DgnPlatform::DTMElementSubHandler::IDTMElementSubHandlerExtension::DeleteCacheElem (Bentley::ElementHandleR element) override
        {
        DTMDisplayCacheManager::DeleteCacheElem (element);
        }
    };

DTMElementSubHandlerExtension s_DTMElementSubHandlerExtension;

void DTMRegisterDisplayHandlers()
    {
    BeFileName dllFileName;
    BeGetModuleFileName (dllFileName, (void*)&dummy);

    #if defined (CREATE_STATIC_LIBRARIES)
       // We need to manually feed a fake name here so the automatic lookup works.
       // The MUIs are in the same location as you'd expect, but with a different file name base than whatever the statically linked application is.
       WString moduleDir = BeFileName::GetDirectoryName (dllFileName);
       dllFileName.BuildName (NULL, moduleDir.c_str (), DLL_NAME_FOR_RESOURCES, NULL);
    #endif

    g_TMHandlersResources  = new RscFileManager::DllRsc(dllFileName); // InitializeDgnCore has already called RscFileManager::StaticInitialize (<<culturename>>)
    registerECExtension ();

    DTMElementTrianglesDisplayHandler::GetInstance().Register();
    DTMElementContoursDisplayHandler::GetInstance().Register();
    DTMElementHighPointsDisplayHandler::GetInstance().Register();
    DTMElementLowPointsDisplayHandler::GetInstance().Register();
    DTMElementFeaturesDisplayHandler::GetInstance().Register();
    DTMElementFeatureSpotsDisplayHandler::GetInstance().Register();
    DTMElementFlowArrowsDisplayHandler::GetInstance().Register();
    DTMElementRasterDrapingDisplayHandler::GetInstance().Register();
    DTMElementSpotsDisplayHandler::GetInstance().Register();
    DTMElementRegionsDisplayHandler::GetInstance().Register();
    DTMElementSubHandler::SetDTMElementSubHandlerExtension (&s_DTMElementSubHandlerExtension);   
    }


END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
