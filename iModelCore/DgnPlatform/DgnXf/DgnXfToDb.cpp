/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnXf/DgnXfToDb.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnXf/DgnProjectXf.h>
#include <DgnPlatform/DgnCore/DgnProject.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnCore/LineStyle.h>
#include <DgnPlatform/DgnCore/ViewController.h>
#include <DgnPlatform/DgnCore/XGraphics.h>
#include <DgnPlatform/DgnCore/DisplayAttribute.h>
#include <DgnPlatform/DgnCore/DgnMarkupProject.h>
#include <DgnPlatform/DgnCore/XGraphics.h>
#include <DgnPlatform/DgnHandlers/DgnECPersistence.h>
#include <DgnPlatform/DgnHandlers/ExtendedElementHandler.h>
#include <DgnPlatform/DgnCore/ElementProperties.h>
#include <DgnPlatform/DgnHandlers/IAreaFillProperties.h>
#include <BeSQLite/ECDb/ECDbApi.h>
#include <ECObjects/ECObjectsAPI.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_FOREIGNFORMAT
using namespace ProtoBuf;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbRemapper::RecordLevelId (UInt32, LevelId) {;} // The importer currently preserves LevelIds. When that changes, we'll have to start using a remap dictionary.
LevelId DgnXfToDbRemapper::RemapLevelId (UInt32 oldId) {return (LevelId)oldId;} 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbRemapper::RecordFontId (UInt32 oldId, UInt32 newId)               {m_fontRemap[oldId] = newId;}
UInt32 DgnXfToDbRemapper::RemapFontId (UInt32 oldId)                     {return m_fontRemap[oldId];}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbRemapper::RecordDisplayStyleId (UInt32 oldId, DgnStyleId newId)       {m_displayStyleRemap[oldId] = newId;}
DgnStyleId DgnXfToDbRemapper::RemapDisplayStyleId (UInt32 oldId)             {return m_displayStyleRemap[oldId];}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbRemapper::RecordExtendedColor (UInt32 oldId, DgnTrueColorId newId) {m_extendedColorRemap[oldId] = newId;}
DgnTrueColorId DgnXfToDbRemapper::RemapExtendedColor (UInt32 oldId)       {return m_extendedColorRemap[oldId];}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 DgnXfToDbRemapper::RemapElementColor (UInt32 oldColor)
    {
    if (COLOR_BYCELL == oldColor || COLOR_BYLEVEL == oldColor)
        return oldColor;

    //  *** TRICKY: Even though oldColor pertains to a different file in a different format, we still expect to be
    //              able to analyze the value itself in order to a) detect if it's a true color ID and b) extract
    //              the (old) true color index from it. This is possible because the encoding is based only on ranges
    //              of values in the 32-bit integer, not on any external tables.
    IntColorDef unusedColorDef;
    if (m_importer.m_project.Colors().IsTrueColor (unusedColorDef, oldColor))
        {
        //  If it is a true color ID, then remap the old *index* to the index in the DgnDb extended color table that we 
        //  assigned when we imported the extended colors from the DgnXf stream.
        return RemapExtendedColor (ColorUtil::GetExtendedIndexFromRawColor (oldColor)).GetValue();
        }
    
    //  If it's *not* a true color ID, then it's either a special fixed value such as bylevel, or it's a color map index.
    //  In either case, just return it.
    //  Special fixed values mean the same thing in DgnDb as in DgnXf.
    //  Color map indices are valid, because we preserved indices when we imported the color map.
    return oldColor;     
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbRemapper::RecordSymbolId (UInt64 oldId, ElementId newId)         {m_xgraphicsSymbolRemap[oldId] = newId;}
ElementId DgnXfToDbRemapper::RemapSymbolId (UInt64 oldId)               {return m_xgraphicsSymbolRemap[oldId];}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbRemapper::RecordModelId (UInt64 oldId, DgnModelId newId)            {m_modelIdRemap[oldId] = newId;}
DgnModelId DgnXfToDbRemapper::RemapModelId (UInt64 oldId)                  {return m_modelIdRemap[oldId];}
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbRemapper::RecordModelSelectorId (UInt64 oldId, DgnModelSelectorId newId) {m_modelSelectorIdRemap[oldId] = newId;}
DgnModelSelectorId DgnXfToDbRemapper::RemapModelSelectorId (UInt64 oldId)       {return m_modelSelectorIdRemap[oldId];}
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ElementProperties DgnXfToDbRemapper::_GetEditPropertiesMask () {return (ElementProperties) (ELEMENT_PROPERTY_Level | ELEMENT_PROPERTY_Color | ELEMENT_PROPERTY_Linestyle | ELEMENT_PROPERTY_Font | ELEMENT_PROPERTY_TextStyle | ELEMENT_PROPERTY_DimStyle | ELEMENT_PROPERTY_MLineStyle | ELEMENT_PROPERTY_Material);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
EditPropertyPurpose DgnXfToDbRemapper::_GetEditPropertiesPurpose () {return EditPropertyPurpose::Remap;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDgnModelP DgnXfToDbRemapper::_GetDestinationDgnModel () {BeAssert(false); return nullptr;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbRemapper::_EachColorCallback (EachColorArg& arg)             {arg.SetStoredValue (RemapElementColor      (arg.GetStoredValue()));}
void DgnXfToDbRemapper::_EachLevelCallback (EachLevelArg& arg)             {arg.SetStoredValue (RemapLevelId           (arg.GetStoredValue().GetValue()));}
void DgnXfToDbRemapper::_EachFontCallback (EachFontArg& arg)               {arg.SetStoredValue (RemapFontId            (arg.GetStoredValue()));}
void DgnXfToDbRemapper::_EachLineStyleCallback (EachLineStyleArg& arg)     {ReportTBD(L"LineStyles"); /* *** WIP: arg.SetStoredValue (RemapLineStyleId   (arg.GetStoredValue()));*/}
void DgnXfToDbRemapper::_EachMaterialCallback (EachMaterialArg& arg)       {ReportTBD(L"Materials");  /* *** WIP: arg.SetStoredValue (RemapMaterialId    (arg.GetStoredValue()));*/}
void DgnXfToDbRemapper::_EachTextStyleCallback (EachTextStyleArg& arg)     {BeAssert(false&&"This kind of style is not supporte by DgnXf"); /* *** WIP: arg.SetStoredValue (RemapTextStyle     (arg.GetStoredValue()));*/}
void DgnXfToDbRemapper::_EachDimStyleCallback (EachDimStyleArg& arg)       {BeAssert(false&&"This kind of style is not supporte by DgnXf"); /* *** WIP: arg.SetStoredValue (RemapDimStyle      (arg.GetStoredValue()));*/}
void DgnXfToDbRemapper::_EachMLineStyleCallback (EachMLineStyleArg& arg)   {BeAssert(false&&"This kind of style is not supporte by DgnXf"); /* *** WIP: arg.SetStoredValue (RemapMLineStyle    (arg.GetStoredValue()));*/}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnXfToDbRemapper::ReportTBD (WCharCP item)
    {
    if (m_reportedMissing[item])
        return;
    m_importer.ReportIssue(DgnProjectFromDgnXfMessageProcessor::IMPORT_ISSUE_TBDStyles, item);
    m_reportedMissing[item] = true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::_ReportIssue (ImportIssue issueId, WCharCP item)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::ReportIssue (ImportIssue error, WCharCP item)
    {
    _ReportIssue (error, item);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::_ReportImportPhase (ImportPhase phase)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::ReportImportPhase (ImportPhase phase)
    {
    _ReportImportPhase (phase);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::_ReportImportProgress()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::ReportImportProgress()
    {
    _ReportImportProgress();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectFromDgnXfMessageProcessor::DgnProjectFromDgnXfMessageProcessor (DgnProjectR p) : m_project(p)
    {
    m_inSection = IN_SECTION_Neutral;
    m_context = new DgnXfToDbContextSection (*this);
    m_remapper = new DgnXfToDbRemapper (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectFromDgnXfMessageProcessor::~DgnProjectFromDgnXfMessageProcessor()
    {
    delete m_context;
    delete m_remapper;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::ImportSymbolDefinition (DgnXfLib::GraphicSymbol const& symb)
    {
    DgnModelP v8model = m_project.Models().FindModelById (m_remapper->RemapModelId(symb.modelId()));
    if (v8model == NULL)
        {
        ReportIssue (IMPORT_ISSUE_ImportFailure, L"Symbol");
        return;
        }

#if defined (WIP_STAMP_SYMBOL)
    Transform transform;
    memcpy (&transform, symb.transform().data(), symb.transform().size());
    
    XGraphicsContainer xgContainer (symb.graphics().data(), symb.graphics().size());

    EditElementHandle eeh;
    if (XGraphicsSymbol::DefineElement (eeh, *v8model, xgContainer, &transform, /*XGraphicsSymbolIdCP*/NULL) != SUCCESS) // WIP_XGRAPHICS_MERGE symbolid?
        {
        ReportIssue (IMPORT_ISSUE_ImportFailure, L"Symbol");
        return;
        }

    eeh.AddToModel();

    m_remapper->RecordSymbolId (symb.symbolId(), eeh.GetElementId());
#endif
    
//    BeAssert (XGraphicsContainer::IsXGraphicsSymbol (eeh.GetElementRef()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::ImportElement (DgnXfLib::GraphicElement const& el)
    {
    BeAssert (m_imported.context);

    DgnModelP v8model = m_project.Models().GetModelById (m_remapper->RemapModelId(el.modelId()));
    if (v8model == nullptr)
        {
        ReportIssue (IMPORT_ISSUE_ImportFailure, L"Element");
        return;
        }

    // Note: We ignore el.elementId(); - we let DgnV8 issue the ElementId. Currently, we have no ElementId-based dependencies, so there's no remapping to worry about.

    EditElementHandle eeh;
    ExtendedElementHandler::InitializeElement (eeh, nullptr, v8model, v8model->Is3d());

    ElementPropertiesSetter setter;
    setter.SetLevel (m_remapper->RemapLevelId (el.levelId()));
    setter.SetColor (m_remapper->RemapElementColor (el.color()));
    setter.SetWeight (el.weight());
    // *** WIP: setter.SetLinestyle (m_remapper->RemapLineStyleId(el.style())); 
    setter.SetElementClass ((DgnElementClass)el.msClass());  // NB: DgnElementClass must match
    if (el.has_transparency())
        setter.SetTransparency (el.transparency());
    setter.Apply (eeh);

    if (!v8model->Is3d())
        {
        if (el.has_priority())
            eeh.GetElementP()->SetDisplayPriority(el.priority());
        }

    if (el.has_invisible())
        eeh.GetElementP()->SetInvisible(el.invisible());

    if (el.has_fillColor())
        {
        UInt32 fillColor = m_remapper->RemapElementColor (el.fillColor());
        bool   alwaysFilled = el.alwaysFilled();
        IAreaFillPropertiesEdit areaFillProps;
        areaFillProps.AddSolidFill (eeh, &fillColor, &alwaysFilled);
        }

        /* *** TBD: basicRange/Transform
    if (el.has_basisRange())
        BasisXAttributesUtil::SetRange (GetDRange3d(el.basisRange()), eeh);

    if (el.has_basisTransform())
        BasisXAttributesUtil::SetTransform (GetTransform(el.basisTransform()), eeh);
        */

    Utf8StringCR graphics = el.graphics();    // TRICKY: we set up protobuf to present a BLob of binary data as a Utf8String!
    XGraphicsContainer xgContainer (graphics.data(), graphics.size());

    PropertyContext remapIds (m_remapper);
    xgContainer.ProcessProperties (remapIds);

    for (int i=0; i<el.symbolIds().size(); ++i)
        xgContainer.AddSymbol (m_remapper->RemapSymbolId (el.symbolIds(i)));

    xgContainer.AddToElement (eeh);

    if (!el.has_xmin())
        eeh.GetDisplayHandler ()->ValidateElementRange (eeh, true);
    else
        {
        DRange3dR  range = eeh.GetElementP()->GetRangeR();
        if (v8model->Is3d())
            {
            range.low.x = (double) el.xmin();
            range.low.y = (double) el.ymin();
            range.low.z = (double) el.zmin();
            range.high.x = (double) el.xmax();
            range.high.y = (double) el.ymax();
            range.high.z = (double) el.zmax();
            }
        else
            {
            range.low.x = (double) el.xmin();
            range.low.y = (double) el.ymin();
            range.high.x = (double) el.xmax();
            range.high.y = (double) el.ymax();
            }
        }

    eeh.AddToModel();
    
    // Note: Don't restore XAttributes in the general case. We are really only presenting the element's graphics in this import. 

    // *** m_provenance: not currently exported. Once we start getting it, should we try to get the original ElementId from it and try to apply it??

    // ***************************************
    // *** WIP: m_relationClassId?? Is this something that applies only to assembly leaders?
    // ***************************************

    /* *** WIP_ECINSTANCES
    if (el.has_primaryECInstanceKey())
        m_elementByECInstanceId.Add (eeh.GetElementRef(), true, el.primaryECInstanceKey());
    for (int ii=0; ii<el.secondaryECInstanceKey().size(); ii++)
        m_elementByECInstanceId.Add (eeh.GetElementRef(), false, el.secondaryECInstanceKey(ii));
    */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::ProcessElementsSectionMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg)
    {
    BeAssert (m_inSection == IN_SECTION_Elements);
    BeAssert (m_imported.context);

    switch (messageType)
        {
        default:
            ReportIssue (IMPORT_ISSUE_UnrecognizedMessage, L"");
/*<==*/     return;

        case DgnXfLib::MESSAGE_ELEMENT_GraphicElement:
            ImportElement (*(DgnXfLib::GraphicElement*)&msg);
            break;

        case DgnXfLib::MESSAGE_ELEMENT_GraphicSymbol:
            ImportSymbolDefinition (*(DgnXfLib::GraphicSymbol*)&msg);
            break;

        case DgnXfLib::MESSAGE_SectionEnd:
            m_inSection = IN_SECTION_Neutral;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::ProcessECSchemaSectionMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg)
    {
    BeAssert (m_inSection == IN_SECTION_ECSchema);
    BeAssert (m_imported.context);
    BeAssert (!m_imported.ecschema);

    switch (messageType)
        {
        default:
            ReportIssue (IMPORT_ISSUE_UnrecognizedMessage, L"");
/*<==*/     return;

        case DgnXfLib::MESSAGE_EC_Schema:
#ifdef WIP_ECSCHEMAS
            ImportECSchema (*(DgnXfLib::ECSchema*)&msg);
#endif
            break;

        case DgnXfLib::MESSAGE_SectionEnd:
            m_imported.ecschema = true;
            m_inSection = IN_SECTION_Neutral;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Ramanujam.Raman                    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::ProcessECInstancesSectionMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg)
    {
    BeAssert (m_inSection == IN_SECTION_ECInstances);
    BeAssert (m_imported.context);
    BeAssert (m_imported.ecschema);

    switch (messageType)
        {
        default:
            ReportIssue (IMPORT_ISSUE_UnrecognizedMessage, L"");
/*<==*/     return;
        
        case DgnXfLib::MESSAGE_EC_Instance:
#ifdef WIP_ECINSTANCES
            ImportECInstance (*(DgnXfLib::ECInstance*)&msg);
#endif
            break;

        case DgnXfLib::MESSAGE_EC_RelationshipInstance:
#ifdef WIP_ECINSTANCES
            ImportECRelationshipInstance (*(DgnXfLib::ECRelationshipInstance*)&msg);
#endif
            break;

        case DgnXfLib::MESSAGE_SectionEnd:
            m_inSection = IN_SECTION_Neutral;
            break;
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::OnStartSection (InSection s, ImportPhase p)
    {
    m_inSection = s;
    ReportImportPhase (p);
    }

/*---------------------------------------------------------------------------------**//**
* This function transitions from "neutral" to some state.
* It expects a SectionState message.
* @bsimethod                                    Sam.Wilson                      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::ProcessSectionStartMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg)
    {
    if (DgnXfLib::MESSAGE_SectionStart != messageType)
        {
        ReportIssue (IMPORT_ISSUE_UnrecognizedMessage, WPrintfString(L"%d",messageType));
/*<==*/ return;
        }

    DgnXfLib::SectionStart* sectionStartMsg = (DgnXfLib::SectionStart*)&msg;

    switch (sectionStartMsg->type())
        {
        default:
            ReportIssue (IMPORT_ISSUE_UnrecognizedMessage, WPrintfString(L"%d",messageType));
/*<==*/     return;

        case DgnXfLib::SECTION_Context:     OnStartSection (IN_SECTION_Context, IMPORT_PHASE_Context);          break;
        case DgnXfLib::SECTION_ECSchema:    OnStartSection (IN_SECTION_ECSchema, IMPORT_PHASE_ECSchemas);       break;
        case DgnXfLib::SECTION_Elements:    OnStartSection (IN_SECTION_Elements, IMPORT_PHASE_Elements);        break;
        case DgnXfLib::SECTION_ECInstances: OnStartSection (IN_SECTION_ECInstances, IMPORT_PHASE_ECInstances);  break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::_ProcessMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg)
    {
    if (IN_SECTION_Neutral != m_inSection)
        ReportImportProgress();

    switch (m_inSection)
        {
        default:
            BeAssert (false && "invalid state");
/*<==*/     return;

        case IN_SECTION_Neutral:  ProcessSectionStartMessage (messageType, msg);    break;
        case IN_SECTION_Context:  m_context->ProcessContextSectionMessage (messageType, msg);  break;
        case IN_SECTION_ECSchema: ProcessECSchemaSectionMessage (messageType, msg); break;
        case IN_SECTION_Elements: ProcessElementsSectionMessage (messageType, msg); break;
        case IN_SECTION_ECInstances: ProcessECInstancesSectionMessage (messageType, msg); break;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_THUMBNAILS
void DgnProjectFromDgnXfMessageProcessor::GenerateThumbnails ()
    {
    ThumbnailConfig thumbnailConfig (m_config);
    if (0 != thumbnailConfig.GetViewTypes())
        {
        // Initalize the graphics subsystem, to produce thumbnails.
        DgnViewLib::GetHost().GetViewManager().InitForGraphics();
        DgnViews::Iterator viewIterator = m_project.Views().MakeIterator (thumbnailConfig.GetViewTypes(), nullptr);
        for (auto const& entry: viewIterator)
            {
            SetTaskName (ImporterMessage::TASK_CREATING_THUMBNAIL, entry.GetName());
            m_project.Views().RenderAndSaveThumbnail(entry.GetDgnViewId(), thumbnailConfig.GetResolution(), thumbnailConfig.GetRenderModeOverride());
            if (ShowProgress())
                break;
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectFromDgnXfMessageProcessor::OnImportComplete ()
    {
#ifdef WIP_THUMBNAILS
    ensureAUserView(*this);
    GenerateThumbnails();
#endif
    m_project.SaveSettings();
    m_project.SaveChanges();
    }
