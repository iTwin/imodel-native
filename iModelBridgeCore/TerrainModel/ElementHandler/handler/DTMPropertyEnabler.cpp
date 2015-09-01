/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMPropertyEnabler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include "MrDTMDataRef.h"
#include <ScalableTerrainModel/IMrDTM.h>
#include <DgnPlatform\DgnECTypeAdapters.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_EC

USING_NAMESPACE_BENTLEY_MRDTM

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

enum DisplayParamTypes
    {
    DPID_TRIANGLES,
    DPID_MAJORCONTOUR,
    DPID_MINORCONTOUR,
    DPID_FLOWARROWS,
    DPID_HIGHPOINTS,
    DPID_LOWPOINTS,
    DPID_SPOTS,

    DPID_BOUNDARIES,
    DPID_BREAKLINES,
    DPID_CONTOURS,
    DPID_HOLES,
    DPID_ISLANDS,
    DPID_VOIDS,
    DPID_FEATURESPOTS,
    DPID_RASTERDRAPING,
    DPID_REGIONS,
    DPID_END
    };

WCharCP DisplayParamTypesString[] =
    {
    L"Triangles",
    L"MajorContours",
    L"MinorContours",
    L"FlowArrows",
    L"HighPoints",
    L"LowPoints",
    L"TriangleVertices",

    L"Boundary",
    L"Breaklines",
    L"ImportedContours",
    L"Holes",
    L"Islands",
    L"Voids",
    L"FeatureSpots",
    L"RasterDraping",
    L"Regions",
    L"End"
    };

static bool IsBaseId (EachPropertyBaseArg& arg) { return 0 != (PROPSCALLBACK_FLAGS_IsBaseID & arg.GetPropertyFlags()) && 0 == (PROPSCALLBACK_FLAGS_UndisplayedID & arg.GetPropertyFlags()); }

//*=====================================================================================
//* @bsiclass
//===============+===============+===============+===============+===============+======
struct DTMElementDataCache : IECPerDelegateData, IQueryProperties
    {
    public:
        struct ParamData
            {
            RefCountedPtr<DTMElementSubHandler::SymbologyParams> param;
            int textStyleId;
            int oldTextStyleId;
            LevelId additionalLevelId;
            int oldAdditionalLevelId;
            bool dirty;
            DTMSubElementId subElementId;
            DisplayParamTypes paramType;
            WString materialName;
            bool materialNameChanged;
            bool gotMaterial;
            ParamData ()
                {
                gotMaterial = false;
                dirty = false;
                materialNameChanged = false;
                }
            ParamData (RefCountedPtr<DTMElementSubHandler::SymbologyParams> param, DTMSubElementId subElementId, DisplayParamTypes paramType, int textStyleId, LevelId additionalLevelId)
                {
                gotMaterial = false;
                materialNameChanged = false;
                dirty = false;
                this->textStyleId = this->oldTextStyleId = textStyleId;
                this->additionalLevelId = this->oldAdditionalLevelId = additionalLevelId;

                this->paramType = paramType;
                this->param = param;
                this->subElementId = subElementId;
                }
            };
        typedef bmap <UInt32, ParamData> ParamsMap;

        struct MrDTMSpecificData
            {
            bool    canLocate;
            bool    clipActivation;
            double  shadedViewPointDensity;
            double  unshadedViewPointDensity;
            bool    viewFlags[MAX_VIEWS];
            };

        ElementRefP m_elemRef;
        MSElementDescrCP m_descr;
    private:
        UInt32 m_paramId [DPID_END+1];
        ParamsMap m_params;
        bool m_isSTM;
        bool m_originalCanHaveOverrideSymbology;
        ElementId m_templateElementId;
        int m_displayStyleId;

        mutable int m_hasCalculatedStatistics;
        mutable DTMFeatureStatisticsInfo m_info;

        long m_edgeMethod;
        double m_edgeLength;
        WString m_originalName;
        WString m_name;
        bool m_canHaveOverrideSymbology;
        bool m_hasOverrideSymbology;

        bool m_originalHasOverrideSymbology;
        ElementId m_originalTemplateElementId;
        int m_originalDisplayStyleId;
        long m_originalEdgeMethod;
        double m_originalEdgeLength;
        bool m_isActiveModelReadOnly;
        DRange3d m_dtmRange;
        DgnModelRefP m_symbologyModelRef;
        DgnFileP m_symbologyDgnFile;
        MrDTMSpecificData m_mrDTMSpecificData;

        // GrapHeaderValues
        LevelId m_headerLevel;
        UInt32 m_headerColor;
        Int32 m_headerLsId;
        UInt32 m_headerWeight;
        LevelId m_originalHeaderLevel;
        UInt32 m_originalHeaderColor;
        Int32 m_originalHeaderLsId;
        UInt32 m_originalHeaderWeight;
        //LineStyleParams m_headerLsParams;
        
    public:
        DgnModelRefP m_activeModel;
        bool m_symbologyReadOnly;
        bool m_inActiveModel;
        bool m_symbologyInActiveModel;

        // IQueryProperties
        virtual ElementProperties   _GetQueryPropertiesMask() override
            {
            return (ElementProperties)(ELEMENT_PROPERTY_ElementTemplate | ELEMENT_PROPERTY_Level | ELEMENT_PROPERTY_Color | ELEMENT_PROPERTY_Linestyle | ELEMENT_PROPERTY_Weight);
            }

    virtual void _EachElementTemplateCallback (EachElementTemplateArg& arg) override
        {
        if (IsBaseId (arg))
            {
            m_templateElementId = arg.GetStoredValue();
            if (0 == m_templateElementId)
                m_templateElementId = INVALID_ELEMENTID;
            }
        }

    virtual void _EachLevelCallback (EachLevelArg& arg) override
        {
        if (m_headerLevel == INVALID_LEVEL)
            m_originalHeaderLevel = m_headerLevel = arg.GetStoredValue ();
        }
    virtual void                _EachWeightCallback (EachWeightArg& arg) override
        {
        if (m_headerWeight == INVALID_WEIGHT)
            m_originalHeaderWeight = m_headerWeight = arg.GetStoredValue ();
        }
    virtual void                _EachLineStyleCallback (EachLineStyleArg& arg) override
        {
        if (m_headerLsId == INVALID_STYLE)
            m_originalHeaderLsId = m_headerLsId = arg.GetStoredValue ();
        //m_headerLsParams = *arg.GetParams ();
        }
    virtual void                _EachColorCallback (EachColorArg& arg) override
        {
        if (m_headerColor == INVALID_COLOR)
            m_originalHeaderColor = m_headerColor = arg.GetStoredValue ();
        }

        //---------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        DTMElementDataCache (ElementHandleCR element)
            {
            m_elemRef = element.GetElementRef();
            m_descr = element.PeekElementDescrCP();

            InitializeDataCache  (element);
            }
    protected:
        void InitializeDataCache (ElementHandleCR element)
            {
            m_hasCalculatedStatistics = false;
            ElementHandle symbologyElement = element;
            // Find the model that is active.
            DgnModelRefP m_activeModel = element.GetModelRef ();

            while (m_activeModel && T_HOST.GetDgnECAdmin ()._IsModelRefReadOnly (*m_activeModel))
                m_activeModel = m_activeModel->GetParentModelRefP ();

            m_canHaveOverrideSymbology = TMSymbologyOverrideManager::CanHaveSymbologyOverride (element);

            if (m_activeModel)
                m_isActiveModelReadOnly = T_HOST.GetDgnECAdmin ()._IsModelRefReadOnly (*m_activeModel);
            else
                m_isActiveModelReadOnly = true;

            if (m_canHaveOverrideSymbology)
                {
                if (TMSymbologyOverrideManager::GetElementForSymbology (element, symbologyElement, element.GetModelRef ()->GetRoot ()))
                    {
                    m_hasOverrideSymbology = element.GetElementRef () != symbologyElement.GetElementRef ();
                    }
                else
                    m_hasOverrideSymbology = false;
                }
            else
                m_hasOverrideSymbology = false;

            m_symbologyReadOnly = !(m_activeModel && m_activeModel->IsSameOrParentOf (*symbologyElement.GetModelRef ()));
            m_symbologyInActiveModel = m_activeModel && symbologyElement.GetModelRef () == m_activeModel;
            m_inActiveModel = m_activeModel && element.GetModelRef () == m_activeModel;

            if (DTMElementHandlerManager::IsFriendModel (symbologyElement, m_activeModel))
                m_symbologyInActiveModel = true;

            DTMElementHandlerManager::GetName (element, m_name);
            DTMDataRefPtr dtmDataRef;
            if (DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, element) == SUCCESS)
                {
                Transform trfs;
                DTMElementHandlerManager::GetStorageToUORMatrix(trfs, element.GetModelRef(), element, true);
                if (dtmDataRef->GetExtents (m_dtmRange))
                    {
                    trfs.multiply(&m_dtmRange, &m_dtmRange);
                    }
                m_isSTM = dtmDataRef->IsMrDTM();

                if(!m_isSTM)
                    {
                    DTMPtr dtm = dtmDataRef->GetDTM (None);
                    if (dtm.IsValid())
                        {
                        double pointTol;
                        double lineTol;

                        BcDTMP bcDTM = dtm->GetBcDTM ();
                        bcDTM->GetTriangulationParameters (pointTol, lineTol, m_edgeMethod, m_edgeLength);
                        m_edgeLength *= trfs.form3d[2][2];
                        if (GetDTMInfo (element).hasHull)
                            m_edgeMethod = 0;

                        }
                    }
                else
                    {
                    MrDTMDataRef* mrDTMDataRef = dynamic_cast<MrDTMDataRef* >(&*dtmDataRef);
                    IMrDTMPtr mrDtm = mrDTMDataRef->GetMrDTM ();
                    if (mrDtm.IsValid())
                        {
                        int edgeMethod;
                        mrDTMDataRef->GetEdgeMethod(edgeMethod);
                        m_edgeMethod = (long)edgeMethod;

                        mrDTMDataRef->GetEdgeMethodLength(m_edgeLength);

                        m_mrDTMSpecificData.canLocate = mrDTMDataRef->CanLocate();
                        m_mrDTMSpecificData.clipActivation = mrDTMDataRef->GetClipActivation();
                        m_mrDTMSpecificData.shadedViewPointDensity = mrDTMDataRef->GetPointDensityForShadedView();
                        m_mrDTMSpecificData.unshadedViewPointDensity = mrDTMDataRef->GetPointDensityForWireframeView();

                        for (int viewIdx = 0; viewIdx < MAX_VIEWS; viewIdx++)
                            {
                            m_mrDTMSpecificData.viewFlags[viewIdx] = mrDTMDataRef->IsVisibleForView(viewIdx);
                            }
                        }
                    }
                }

            if (symbologyElement.IsValid ())
                {
                m_symbologyModelRef = symbologyElement.GetModelRef ();
                m_symbologyDgnFile = symbologyElement.GetDgnFileP ();

                DTMSubElementIter iter (symbologyElement, true);

                for (int i = 0; i <= DPID_END; i++)
                    m_paramId[i] = 0xffffffff;

                for (; iter.IsValid (); iter.ToNext ())
                    {
                    DTMElementSubHandler::SymbologyParams* param = DTMElementSubHandler::GetParams (iter);
                    DisplayParamTypes paramType = DPID_END;
                    UInt16 subHandlerId = iter.GetCurrentId ().GetHandlerId ();
                    UInt32 id = iter.GetCurrentId ().GetId ();
                    switch (subHandlerId)
                        {
                        case DTMElementTrianglesHandler::SubHandlerId:
                            paramType = DPID_TRIANGLES;
                            break;
                        case DTMElementContoursHandler::SubHandlerId:
                            if (((DTMElementContoursHandler::DisplayParams*)param)->GetIsMajor ())
                                paramType = DPID_MAJORCONTOUR;
                            else
                                paramType = DPID_MINORCONTOUR;
                            break;
                        case DTMElementFlowArrowsHandler::SubHandlerId:
                            paramType = DPID_FLOWARROWS;
                            break;
                        case DTMElementHighPointsHandler::SubHandlerId:
                            paramType = DPID_HIGHPOINTS;
                            break;
                        case DTMElementLowPointsHandler::SubHandlerId:
                            paramType = DPID_LOWPOINTS;
                            break;
                        case DTMElementSpotsHandler::SubHandlerId:
                            paramType = DPID_SPOTS;
                            break;
                        case DTMElementRegionsHandler::SubHandlerId:
                            paramType = DPID_REGIONS;
                            break;
                        case DTMElementFeaturesHandler::SubHandlerId:
                            switch (((DTMElementFeaturesHandler::DisplayParams*)param)->GetTag ())
                                {
                                case DTMElementFeaturesHandler::Boundary:
                                    paramType = DPID_BOUNDARIES;
                                    break;
                                case DTMElementFeaturesHandler::Breakline:
                                    paramType = DPID_BREAKLINES;
                                    break;
                                case DTMElementFeaturesHandler::Contour:
                                    paramType = DPID_CONTOURS;
                                    break;
                                case DTMElementFeaturesHandler::Hole:
                                    paramType = DPID_HOLES;
                                    break;
                                case DTMElementFeaturesHandler::Island:
                                    paramType = DPID_ISLANDS;
                                    break;
                                case DTMElementFeaturesHandler::Void:
                                    paramType = DPID_VOIDS;
                                    break;
                                }
                            break;
                        case DTMElementFeatureSpotsHandler::SubHandlerId:
                            paramType = DPID_FEATURESPOTS;
                            break;
                        case DTMElementRasterDrapingHandler::SubHandlerId:
                            paramType = DPID_RASTERDRAPING;
                            break;
                        }

                    if (paramType != DPID_END)
                        {
                        m_paramId[paramType] = id;
                        int textStyleId = 0;

                        if (dynamic_cast<DTMElementSubHandler::SymbologyParamsAndTextStyle*>(param))
                            textStyleId = GetDTMTextParamId (symbologyElement, id);

                        LevelIdXAttributeParams levelIdParam;

                        levelIdParam.Get (symbologyElement, id);
                        m_params[iter.GetCurrentId ().GetId ()] = ParamData (param, iter.GetCurrentId (), paramType, textStyleId, levelIdParam.GetLevelId());
                        }
                    }


                m_headerLevel = INVALID_LEVEL;
                m_headerColor = INVALID_COLOR;
                m_headerLsId = INVALID_STYLE;
                m_headerWeight = INVALID_WEIGHT;

                PropertyContext context (this);
                symbologyElement.GetHandler ().QueryProperties (symbologyElement, context);
                DTMElementHandlerManager::GetThematicDisplayStyleIndex (symbologyElement, m_displayStyleId);
                }

            m_originalHasOverrideSymbology = m_hasOverrideSymbology;
            m_originalDisplayStyleId = m_displayStyleId;
            m_originalTemplateElementId = m_templateElementId;
            m_originalEdgeMethod = m_edgeMethod;
            m_originalEdgeLength = m_edgeLength;
            m_originalName = m_name;
            }

        virtual ~DTMElementDataCache()
            {
            }
    public:

        //---------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        static DTMElementDataCache* Create (ElementHandleCR eh)
            {
            return new DTMElementDataCache (eh);
            }

        //---------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        ECObjectsStatus        ToElement (EditElementHandleR element)
            {
            bool needsWrite = false;
            ElementRefP elementRef = element.GetElementRef ();


            if (m_headerLevel != m_originalHeaderLevel || m_headerColor != m_originalHeaderColor || m_headerLsId != m_originalHeaderLsId || m_headerWeight != m_originalHeaderWeight)
                {
                ElementPropertiesSetterPtr remapper = ElementPropertiesSetter::Create ();
                if (m_headerLevel != m_originalHeaderLevel)
                    remapper->SetLevel (m_headerLevel);

                if (m_headerColor != m_originalHeaderColor)
                    remapper->SetColor (m_headerColor);
                if (m_headerLsId != m_originalHeaderLsId)
                    remapper->SetLinestyle (m_headerLsId, nullptr);
                if (m_headerWeight != m_originalHeaderWeight)
                    remapper->SetWeight (m_headerWeight);

                remapper->Apply (element);
                needsWrite = true;
                }

            if (!m_isSTM)
                {
                if (m_originalEdgeMethod != m_edgeMethod || m_originalEdgeLength != m_edgeLength)
                    {
                    DTMDataRefPtr dtmDataRef;
                    if (DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, element) == SUCCESS)
                        {
                        DTMPtr dtm = dtmDataRef->GetDTM (None);
                        if (dtm.IsValid())
                            {
                            BcDTMP bcDTM = dtm->GetBcDTM ();
                            double pointTol;
                            double lineTol;
                            long oldEdgeMethod;
                            double oldEdgeLength;
                            if (bcDTM->GetTriangulationParameters (pointTol, lineTol, oldEdgeMethod, oldEdgeLength) == DTM_SUCCESS)
                                {
                                Transform trfs;
                                DTMElementHandlerManager::GetStorageToUORMatrix(trfs, element.GetModelRef(), element, true);
                                double edgeLength = m_edgeLength / trfs.form3d[2][2];
                                if (m_edgeMethod != oldEdgeMethod || edgeLength != oldEdgeLength)
                                    {
                                    bcDTM->SetTriangulationParameters (pointTol, lineTol, m_edgeMethod, edgeLength);
                                    bcDTM->Triangulate ();
                                    }
                                }
                            }
                        }
                    m_originalEdgeMethod = m_edgeMethod;
                    m_originalEdgeLength = m_edgeLength;
                    }
                }

            DgnModelRefP activeModel = GetActivatedModel (element, nullptr);
            if (m_originalHasOverrideSymbology != m_hasOverrideSymbology)
                {
                bool hasOverrideSymbology = false;
                ElementHandle elementHandle2;
                if (TMSymbologyOverrideManager::GetElementForSymbology (element, elementHandle2, activeModel))
                    hasOverrideSymbology = element.GetElementRef () != elementHandle2.GetElementRef ();
                else
                    hasOverrideSymbology = false;
                if (!hasOverrideSymbology && m_hasOverrideSymbology)
                    TMSymbologyOverrideManager::CreateSymbologyOverride (element, activeModel);
                else if (hasOverrideSymbology && !m_hasOverrideSymbology)
                    TMSymbologyOverrideManager::DeleteSymbologyOverride (element, activeModel);
                }

            EditElementHandle overideSymbologyElement;
            bool hasSymbology = TMSymbologyOverrideManager::GetElementForSymbology (element, overideSymbologyElement, activeModel);
            EditElementHandle& symbologyElement = hasSymbology ? overideSymbologyElement : element;
            ElementRefP symbologyElementRef = hasSymbology ? overideSymbologyElement.GetElementRef() : elementRef;

            bool needsSymbologyWrite = false;
            for (auto it = m_params.begin(); it != m_params.end(); it++)
                {
                if (it->second.dirty)
                    {
                    it->second.dirty = false;
                    needsSymbologyWrite = true;
                    if (it->second.materialNameChanged)
                        {
                        DTMElementSubHandler::SymbologyAndMaterialParams* materialParam = dynamic_cast<DTMElementSubHandler::SymbologyAndMaterialParams*>(it->second.param.get ());

                        BeAssert (materialParam);
                        if (materialParam)
                            {
                            if (it->second.materialName.size () == 0)
                                materialParam->SetMaterialElementID (0);
                            else
                                {
                                ElementId materialElementId = 0;
                                MaterialCP material;

                                material = MaterialManager::GetManagerR ().FindMaterial (nullptr, MaterialId (it->second.materialName.GetWCharCP ()), *SymbologyDgnFile (), *SymbologyDgnModelRef (), true);

                                if (material)
                                    materialElementId = material->GetElementId ();
                                else
                                    {
                                    size_t pos = it->second.materialName.find_first_of (L';');

                                    MaterialList materials;
                                    MaterialManager::GetManagerR ().FindMaterialByNameFromAnySource (nullptr, materials, it->second.materialName.substr (pos + 1).GetWCharCP (), *SymbologyDgnModelRef (), true);

                                    if (materials.size () != 0)
                                        {
                                        WString paletteName = it->second.materialName.substr (0, pos);
                                        for (MaterialList::iterator mit = materials.begin (); mit != materials.end (); mit++)
                                            {
                                            MaterialCP mat = *mit;
                                            if (paletteName.CompareToI (mat->GetPalette ().GetName ()) == 0)
                                                {
                                                MaterialId materialId;
                                                MaterialPtr newMaterial = Material::Create (*mat, *SymbologyDgnModelRef ());
                                                PaletteInfoPtr paletteInfo = PaletteInfo::Create (mat->GetPalette ().GetName ().c_str (), SymbologyDgnFile ()->GetDocument ().GetMoniker (), mat->GetPalette ().GetSource (), PaletteInfo::PALETTETYPE_Dgn);

                                                newMaterial->GetPaletteR ().Copy (*paletteInfo);

                                                if (SUCCESS == MaterialManager::GetManagerR ().SaveMaterial (&materialId, *newMaterial, SymbologyDgnFile ()))
                                                    materialElementId = materialId.GetElementId ();
                                                break;
                                                }
                                            }
                                        }
                                    }
                                materialParam->SetMaterialElementID (materialElementId);
                                }
                            }
                        }
                    DTMElementPointsHandler::DisplayParams* pointParam = dynamic_cast<DTMElementPointsHandler::DisplayParams*>(it->second.param.get());
                    if (pointParam && pointParam->GetPointCellType() == DTMElementPointsHandler::DisplayParams::Cell)
                        {
                        DTMElementHandlerManager::GetDTMElementPowerPlatformExtension()->EnsureSharedCellDefinitionExists (pointParam->GetPointCellName().GetWCharCP(), SymbologyDgnModelRef());
                        }

                    DTMElementSubHandler::SymbologyParamsAndTextStyle* textParam = dynamic_cast<DTMElementSubHandler::SymbologyParamsAndTextStyle*>(it->second.param.get());
                    if (textParam)
                        textParam->SetTextStyleID (it->second.textStyleId);

                    it->second.param->SetElement (symbologyElement, it->second.subElementId);

                    if (it->second.oldTextStyleId != it->second.textStyleId)
                        {
                        AddDTMTextStyle (symbologyElement, it->second.textStyleId, it->second.subElementId.GetId());
                        it->second.oldTextStyleId = it->second.textStyleId;
                        }

                    if (it->second.oldAdditionalLevelId != it->second.additionalLevelId)
                        {
                        LevelIdXAttributeParams levelIdParam;
                        levelIdParam.SetLevelId (it->second.additionalLevelId);
                        levelIdParam.Set (symbologyElement, it->second.subElementId.GetId());
                        it->second.oldAdditionalLevelId = it->second.additionalLevelId;
                        }
                    }
                }

            if (m_originalDisplayStyleId != m_displayStyleId)
                {
                DTMElementHandlerManager::SetThematicDisplayStyleIndex (symbologyElement, m_displayStyleId);
                m_originalDisplayStyleId = m_displayStyleId;
                needsSymbologyWrite = true;
                }

            if (m_originalTemplateElementId != m_templateElementId)
                {
                // Copied from CommonDelegates.
                ElementTemplateStatus templateStatus = ETSTATUS_Success;
                if (INVALID_ELEMENTID == m_templateElementId)
                    {
                    templateStatus = ElementTemplateUtils::ScheduleTemplateReferenceRemoval (symbologyElement);
                    needsSymbologyWrite = true;
                    }
                else
                    {
                    // NEEDSWORK_TEMPLATES: Template extenders live in PowerPlatform, but they are needed to ensure all data is copied from template to element...
                    // For now we call into PowerPlatform if it's available.

                    // If we don't do this, and only do the DgnECExternalPropertyHandler thing, then symbology doesn't get updated.
                    // If we only do this, and don't do the DgnECExternalPropertyHandler thing, custom items don't get copied from template to element (due to extenders living in PowerPlatform).
                    if (ETSTATUS_Success == ElementTemplateUtils::SetReferencedTemplateID (symbologyElement, m_templateElementId, true))
                        {
                        needsSymbologyWrite = true;
                        DgnECExternalPropertyHandler const* handler = T_HOST.GetDgnECAdmin()._GetExternalPropertyHandler();
                        if (nullptr != handler)
                            {
                            if (ECOBJECTS_STATUS_Success != handler->SetValue (ECValue ((Int64)m_templateElementId), L"MstnGraphHeader", L"Template", -1, &symbologyElement))
                                {
                                //return ECOBJECTS_STATUS_Error;
                                }
                            }
                        }
                    }
                m_originalTemplateElementId = m_templateElementId;
                }

            if (m_isSTM)
                {
                DTMDataRefPtr dtmDataRef;
                if (DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, element) == SUCCESS)
                    {
                    MrDTMDataRef* mrDTMDataRef = dynamic_cast<MrDTMDataRef* >(&*dtmDataRef);
                    IMrDTMPtr mrDtm = mrDTMDataRef->GetMrDTM ();
                    if (mrDtm.IsValid())
                        {
                        mrDTMDataRef->SetLocate(m_mrDTMSpecificData.canLocate);
                        mrDTMDataRef->SetClipActivation(m_mrDTMSpecificData.clipActivation);

                        bool viewIsOn;
                        for (short viewIdx = 0; viewIdx < MAX_VIEWS; viewIdx++)
                            {
                            viewIsOn = m_mrDTMSpecificData.viewFlags[viewIdx];
                            mrDTMDataRef->SetVisibilityForView(viewIdx, viewIsOn);
                            }

                        mrDTMDataRef->SetPointDensityForShadedView (m_mrDTMSpecificData.shadedViewPointDensity);
                        mrDTMDataRef->SetPointDensityForWireframeView (m_mrDTMSpecificData.unshadedViewPointDensity);

                        if (m_originalEdgeMethod != m_edgeMethod || m_originalEdgeLength != m_edgeLength)
                            {
                            mrDTMDataRef->SetEdgeMethodLength (m_edgeLength);
                            mrDTMDataRef->SetEdgeMethod (m_edgeMethod);
                            }
                        m_originalEdgeMethod = m_edgeMethod;
                        m_originalEdgeLength = m_edgeLength;
                        }
                    }
                }

            if (needsSymbologyWrite)
                {
                if (hasSymbology)
                    {
                    symbologyElement.ReplaceInModel (symbologyElementRef);
                    // Todo move into Override Symbology Handler.
                    RedrawElement (element);
                    InitializeDataCache (element);
                    }
                else
                    needsWrite = true;
                }

            if (m_originalName != m_name)
                {
                DTMElementHandlerManager::SetName (element, m_name.GetWCharCP());
                m_originalName = m_name;
                needsWrite = true;
                }

            if (needsWrite)
                element.ReplaceInModel (elementRef);

            return ECOBJECTS_STATUS_Success;
            }

        const DTMFeatureStatisticsInfo& GetDTMInfo (ElementHandleCR element) const
            {
            if (!m_hasCalculatedStatistics)
                {
                m_hasCalculatedStatistics = true;
                DTMDataRefPtr dtmDataRef;
                if (DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, element) == SUCCESS)
                    {
                    if (!m_isSTM)
                        {
                        DTMPtr dtm = dtmDataRef->GetDTM (None);
                        if (dtm.IsValid())
                            {
                            BcDTMP bcDTM = dtm->GetBcDTM ();

                            bcDTM->CalculateFeatureStatistics (m_info);
                            }
                        }
                    else
                        {
                        MrDTMDataRef* mrDTMDataRef = dynamic_cast<MrDTMDataRef* >(&*dtmDataRef);
                        IMrDTMPtr mrDtm = mrDTMDataRef->GetMrDTM ();
                        if (mrDtm.IsValid())
                            {
                            m_info.numBreaks = mrDtm->GetBreaklineCount();
                            m_info.numDtmFeatures = mrDtm->GetBreaklineCount();
                            m_info.numPoints = mrDtm->GetPointCount();
                            }
                        }
                    }
                }
            return m_info;
            }
        DRange3dCR GetDTMRange() const { return m_dtmRange; }
        int DisplayStyleId () { return m_displayStyleId; }
        void DisplayStyleId (int value) { m_displayStyleId = value; }
        ElementId TemplateElementId () { return m_templateElementId; }
        void TemplateElementId (ElementId value) { m_templateElementId = value; }
        bool IsSTM() { return m_isSTM; }
        bool CanHaveOverrideSymbology() { return m_canHaveOverrideSymbology; }
        bool HasOverrideSymbology() { return m_hasOverrideSymbology; }
        void HasOverrideSymbology (bool value) { m_hasOverrideSymbology = value; }

        DgnModelRefP SymbologyDgnModelRef() { return m_symbologyModelRef; }
        DgnFileP SymbologyDgnFile() { return m_symbologyDgnFile; }

        UInt32 GetIDForType (DisplayParamTypes id)
            {
            return m_paramId [id];
            }
        WCharCP GetParamTypeString (UInt32 id)
            {
            return DisplayParamTypesString [m_params[id].paramType];
            }
        int GetParamTextStyleId (UInt32 id)
            {
            return m_params[id].textStyleId;
            }
        LevelId GetParamTextLevelId (UInt32 id)
            {
            return m_params[id].additionalLevelId;
            }
        WCharCP GetParamMaterial (UInt32 id)
            {
            if (!m_params[id].gotMaterial)
                {
                m_params[id].gotMaterial = true;
                DTMElementSubHandler::SymbologyAndMaterialParams* param = dynamic_cast<DTMElementSubHandler::SymbologyAndMaterialParams*>(m_params[id].param.get());

                MaterialCP material = MaterialManager::GetManagerR ().FindMaterial (nullptr, MaterialId (param->GetMaterialElementID()), *SymbologyDgnFile(), *SymbologyDgnModelRef(), true);
                if (material)
                    {
                    m_params[id].materialName = material->GetPalette().GetName() + L";";
                    m_params[id].materialName.append (material->GetName().c_str());
                    }
                }
            return m_params[id].materialName.GetWCharCP();
            }
        void SetParamMaterial (UInt32 id, WCharCP value)
            {
            m_params[id].dirty = true;
            m_params[id].materialNameChanged = true;
            m_params[id].materialName = value;
            }
        void SetParamTextStyleId (UInt32 id, int value)
            {
            m_params[id].dirty = true;
            m_params[id].textStyleId = value;
            }
        void SetParamTextLevelId (UInt32 id, LevelId value)
            {
            m_params[id].dirty = true;
            m_params[id].additionalLevelId = value;
            }
        const DTMElementSubHandler::SymbologyParams* GetParamForType (DisplayParamTypes id) const
            {
            UInt32 paramType = m_paramId [id];
            if (paramType != 0xffffffff)
                return GetParam (paramType);
            return nullptr;
            }
        DTMElementSubHandler::SymbologyParams* GetParamForTypeForWrite (DisplayParamTypes id)
            {
            UInt32 paramType = m_paramId [id];
            if (paramType != 0xffffffff)
                return GetParamForWrite (paramType);
            return nullptr;
            }
        const DTMElementSubHandler::SymbologyParams* GetParam (UInt32 id) const
            {
            ParamsMap::const_iterator it = m_params.find (id);

            if (it != m_params.end())
                return it->second.param.get();
            return nullptr;
            }

        DTMElementSubHandler::SymbologyParams* GetParamForWrite (UInt32 id)
            {
            m_params[id].dirty = true;
            return m_params[id].param.get();
            }

        bool IsActiveModelReadOnly()
            {
            return m_isActiveModelReadOnly;
            }

        Int16 GetSubHandlerId(UInt32 id)
            {
            return m_params[id].subElementId.GetHandlerId();
            }

        ParamsMap& GetParamMap()
            {
            return m_params;
            }

        WStringCR GetName()
            {
            return m_name;
            }

        void SetName (WCharCP value)
            {
            m_name = value;
            }

        void SetEdgeOption (int edgeMethod)
            {
            if (!IsSTM())
                {
                // For TM, Edge method values are stored on 1, 2, 3
                m_edgeMethod = edgeMethod;
                }
            else
                {
                // For STM, Edge method values are stored on 0, 1, 2
                m_edgeMethod = edgeMethod - 1;
                }
            }

        int GetEdgeOption ()
            {
            if (!IsSTM())
                {
                // For TM, Edge method values are stored on 1, 2, 3
                return (int)m_edgeMethod;
                }
            else
                {
                // For STM, Edge method values are stored on 0, 1, 2
                return (int)m_edgeMethod + 1;
                }
            }

        void SetEdgeLength (double length)
            {
            m_edgeLength = length;
            }
        double GetEdgeLength()
            {
            return m_edgeLength;
            }

        const MrDTMSpecificData& GetMrDTMSpecificData () const
            {
            return m_mrDTMSpecificData;
            }
        void SetMrDTMCanLocate(bool canLocate)
            {
            m_mrDTMSpecificData.canLocate = canLocate;
            }
        void SetMrDTMClipActivation(bool clipActivation)
            {
            m_mrDTMSpecificData.clipActivation = clipActivation;
            }
        void SetMrDTMViewFlag(int viewIdx, bool viewIsOn)
            {
            if (viewIdx >= 0 && viewIdx < MAX_VIEWS)
                m_mrDTMSpecificData.viewFlags[viewIdx] = viewIsOn;
            }
        void SetMrDTMShadedViewPointDensity(double shadedViewPointDensity)
            {
            m_mrDTMSpecificData.shadedViewPointDensity = shadedViewPointDensity;
            }
        void SetMrDTMUnshadedViewPointDensity(double unshadedViewPointDensity)
            {
            m_mrDTMSpecificData.unshadedViewPointDensity = unshadedViewPointDensity;
            }


        LevelId GetHeaderLevel ()
            {
            return m_headerLevel;
            }
        void SetHeaderLevel (LevelId value)
            {
            m_headerLevel = value;
            }
        UInt32 GetHeaderColor ()
            {
            return m_headerColor;
            }
        void SetHeaderColor (UInt32 value)
            {
            m_headerColor = value;
            }
        Int32 GetHeaderLsId ()
            {
            return m_headerLsId;
            }
        void SetHeaderLsId (Int32 value)
            {
            m_headerLsId = value;
            }
        UInt32 GetHeaderWeight ()
            {
            return m_headerWeight;
            }
        void SetHeaderWeight (UInt32 value)
            {
            m_headerWeight = value;
            }


    };

static const WCharCP s_relationshipNames[] = { L"TerrainModelChildrenRelationship" };
static const uint8_t REL_TerrainModelChildren = 0;

//=======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMELEMENTECExtension : DelegatedElementECExtension, DgnECIntrinsicRelationshipProvider
    {
    DEFINE_T_SUPER (DelegatedElementECExtension);
    mutable ECSchemaCP m_schema;

    DTMELEMENTECExtension () :  DgnECIntrinsicRelationshipProvider (ElementECExtension::LookupECSchema (L"TerrainModel"), RelationshipNameList (s_relationshipNames, 1))
        {
        m_schema = &GetRelationshipSchema();
        }

private:
    struct ECClassTableEntry
        {
        RefCountedPtr<ElementECDelegate> ecDelegate;
        ECClassCP ecClass;
        //            ECClassTableEntry(ECClassTableEntry& e) { this->ecDelegate = ecDelegate; this->ecClass = ecClass; }
        };

    typedef bmap<WString, ECClassTableEntry> ECTable;
    mutable ECTable m_ecTable;
    struct ECClassEntry
        {
        WCharCP className;
        ElementECDelegateP (*CreateFunction) (DTMELEMENTECExtension const& extension);
        };

public:
    ECTable& GetMapTable () const;

    void SetPerDelegateData (DelegatedElementECInstancePtr instance, ECClassCR ecClass, DTMElementDataCache* dataCache) const;
public:
    DgnElementECInstancePtr GetInstanceWithDataCache (ElementHandleCR eh, DTMElementDataCache* dataCache, DelegatedElementECEnablerCP delEnabler, UInt32 localId, DgnECInstanceCreateContextCR context) const;
    void AddInstanceWithDataCache (DgnElementECInstanceVector& instances, ElementHandleCR eh, DTMElementDataCache* dataCache, DelegatedElementECEnablerCP delEnabler, UInt32 localId, DgnECInstanceCreateContextCR context) const;

protected:
    virtual void                    _GetECClasses (T_ECClassCPVector& classes) const override;
    virtual ElementECDelegatePtr    _SupplyDelegate (ECClassCR ecClass) const override;
    virtual void                    _InitializeCompoundDelegate (ICompoundECDelegateR del, ECN::ECClassCR ecClass) const override;
    virtual void _GenerateInstances (DgnElementECInstanceVector& instances, ElementHandleCR eh, ElementECEnablerListCR enablers, DgnECInstanceCreateContextCR context) const override;
    virtual IDgnECInstanceFinderPtr _CreateRelatedFinder (DgnECInstanceCR source, QueryRelatedClassSpecifierCR relationshipClassSpecifier, DgnECInstanceCreateContextCR createContext) const override;
    virtual void _GetSupportedRelationshipInfos (DgnECInstanceCR instance, DgnECRelationshipInfoVector& infos) const override;
    virtual DgnECInstancePtr        _LocateInstance (ElementHandleCR eh, WStringCR instanceId, DgnECInstanceCreateContextCR context) override;
    virtual bool _SupportsEditLockedElement () const { return true; }
    virtual bool _SupportsEditReadOnlyElement () const { return true; }
    virtual bool _SupportsEditElementInAttachment () const override { return true; }
    virtual void _GetPotentialInstanceChanges (DgnECInstanceChangeRecordsR changes, DgnECTxnInfoCR txnInfo, DgnECInstanceCreateContextCR context) const override;
    public:

    static DTMELEMENTECExtension* Create ()
        {
        DTMELEMENTECExtension* ext = new DTMELEMENTECExtension();
        return ext;
        }

    ECSchemaCP GetECSchemaCP () const
        {
        if (m_schema == nullptr) //.IsNull())
            m_schema = &LookupECSchema (L"TerrainModel");

        return m_schema; //.get();
        }

    static bool         IsPropertyReadOnly_AlwaysInRef (DelegatedElementECInstanceCR instance, ICompoundECDelegateCR, IElementECDelegateCR primary, UInt32 propertyIndex)
        {
        ElementHandleCR eh = instance.GetElementHandle ();
        //DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData ((ElementECDelegateCR)primary));
        if (T_HOST.GetDgnECAdmin ()._IsModelRefReadOnly (*eh.GetModelRef ()))
            return true;
        return primary.IsPropertyReadOnly (instance, propertyIndex);
        }
    static bool         IsPropertyReadOnly_EdgeMethod (DelegatedElementECInstanceCR instance, ICompoundECDelegateCR del, IElementECDelegateCR primary, UInt32 propertyIndex)
        {
        bool ret = IsPropertyReadOnly_AlwaysInRef (instance, del, primary, propertyIndex);
        if (!ret)
            {
            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData ((ElementECDelegateCR)primary));
            if (data->GetEdgeOption () == 0)
                ret = true;
            }
        return ret;
        }
    static bool         IsPropertyReadOnly_Never (DelegatedElementECInstanceCR instance, ICompoundECDelegateCR, IElementECDelegateCR primary, UInt32 propertyIndex)
        {
        DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData ((ElementECDelegateCR)primary));
        if (!data->m_inActiveModel && !data->m_symbologyReadOnly)
            return false;
        return true; // base.IsPropertyReadOnly (instance, propIdx);
        }

    static bool         IsPropertyReadOnly_Default (DelegatedElementECInstanceCR instance, ICompoundECDelegateCR compound, IElementECDelegateCR primary, UInt32 propertyIndex)
        {
        DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData ((ElementECDelegateCR)primary));
        if (data->m_inActiveModel)
            return primary.IsPropertyReadOnly (instance, propertyIndex);
        return data->m_symbologyReadOnly || !data->m_symbologyInActiveModel;
        }
    DgnECInstancePtr GetInstance (ElementHandleCR eh, UInt32 localId, DgnECInstanceCreateContextCR context, DTMElementDataCache* dataCache) const;
    inline void AddInstance (bvector<DgnECInstancePtr>& instances, ElementHandleCR eh, UInt32 localId, DgnECInstanceCreateContextCR context, DTMElementDataCache* dataCache) const
        {
        DgnECInstancePtr instance = GetInstance (eh, localId, context, dataCache);
        if (instance.IsValid ())
            instances.push_back (instance);
        }
    };

/*---------------------------------------------------------------------------------**//**
* TFS#45230: "OverrideSymbology" property is controlling read-only state of properties
* belonging to other delegates...
* @bsimethod                                                    Paul.Connelly   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DTMELEMENTECExtension::_InitializeCompoundDelegate (ICompoundECDelegateR del, ECN::ECClassCR ecClass) const
    {
    if (ecClass.GetName().Equals (L"DTMElement"))
        {
        // Following are read-only if in a reference attachment and OverrideSymbology=Yes OR No
        del.RegisterIsPropertyReadOnlyOverrideHandler (*ElementECDelegate::LookupRegisteredPrimaryDelegate (L"BaseElementSchema", L"MstnGraphHeader"), &DTMELEMENTECExtension::IsPropertyReadOnly_AlwaysInRef);
        del.RegisterIsPropertyReadOnlyOverrideHandler (*ElementECDelegate::LookupRegisteredPrimaryDelegate (L"BaseElementSchema", L"MstnLockedElement"), &DTMELEMENTECExtension::IsPropertyReadOnly_AlwaysInRef);
        del.RegisterIsPropertyReadOnlyOverrideHandler (*ElementECDelegate::LookupRegisteredPrimaryDelegate (L"TerrainModel", L"DTMInformation"), &DTMELEMENTECExtension::IsPropertyReadOnly_AlwaysInRef);
        del.RegisterIsPropertyReadOnlyOverrideHandler (*ElementECDelegate::LookupRegisteredPrimaryDelegate (L"TerrainModel", L"DTMEdgeMethod"), &DTMELEMENTECExtension::IsPropertyReadOnly_EdgeMethod);

        // All others are read-only if in a reference attachment and OverrideSymbology=No
        del.RegisterIsPropertyReadOnlyOverrideHandler (&DTMELEMENTECExtension::IsPropertyReadOnly_Default);

        // Read-only state of this one is never overridden - controls read-only state of the others.
        del.RegisterIsPropertyReadOnlyOverrideHandler (*ElementECDelegate::LookupRegisteredPrimaryDelegate (L"TerrainModel", L"DTMOverrideSymbology"), &DTMELEMENTECExtension::IsPropertyReadOnly_Never);
        }
    }

struct BaseElementECDelegate : ElementECDelegate
    {
    protected:
        DTMELEMENTECExtension const&    m_extension;

    BaseElementECDelegate (DTMELEMENTECExtension const& extension) : m_extension (extension)
        {
        }

    public:
    //*--------------------------------------------------------------------------------------
    //* @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+------
    virtual bool _AttachToInstance (DelegatedElementECInstanceCR instance) const override
        {
        if (!instance.GetPerDelegateData (*this))
            instance.SetPerDelegateData (*this, DTMElementDataCache::Create (instance.GetElementHandle()));
        return true;
        }

    //*--------------------------------------------------------------------------------------
    //* @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+------
    virtual ECObjectsStatus _Commit (EditElementHandleR eh, DelegatedElementECInstanceR instance) const override
        {
        DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
        if (!EXPECTED_CONDITION (nullptr != data))
            return ECOBJECTS_STATUS_Error;

        return data->ToElement (eh);
        }
    };

struct BaseOSElementECDelegate : BaseElementECDelegate
    {
    protected:
        BaseOSElementECDelegate (DTMELEMENTECExtension const& extension) : BaseElementECDelegate (extension)
        {
        }

    public:

    virtual bool _IsPropertyReadOnlyInLockedElement (DelegatedElementECInstanceCR instance, UInt32 propertyIndex) const override
        {
        DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
        if (!EXPECTED_CONDITION (nullptr != data))
            return ElementECDelegate::_IsPropertyReadOnlyInLockedElement (instance, propertyIndex);
        if (!data->m_inActiveModel && data->m_symbologyInActiveModel)
            return false;
        return ElementECDelegate::_IsPropertyReadOnlyInLockedElement (instance, propertyIndex);
        }
    virtual bool _IsPropertyReadOnly (DelegatedElementECInstanceCR instance, UInt32 propertyIndex) const override
        {
        DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
        if (!EXPECTED_CONDITION (nullptr != data))
            return BaseElementECDelegate::_IsPropertyReadOnly (instance, propertyIndex);

        if (data->m_inActiveModel)
            {
            if (data->m_symbologyReadOnly)
                return true;
            }
        else
            {
            return data->m_symbologyReadOnly || !data->m_symbologyInActiveModel;
            }

        return BaseElementECDelegate::_IsPropertyReadOnly (instance, propertyIndex);
        }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementInfoECDelegate : BaseElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_Name =  1,
            DTMELEMENT_PROPERTYID_NumTriangles,
            DTMELEMENT_PROPERTYID_NumBreaklines,
            DTMELEMENT_PROPERTYID_NumContours,
            DTMELEMENT_PROPERTYID_NumFeatures,
            DTMELEMENT_PROPERTYID_NumVoids,
            DTMELEMENT_PROPERTYID_NumIslands,
            DTMELEMENT_PROPERTYID_NumPointFeatures,
            DTMELEMENT_PROPERTYID_NumPoints,
            DTMELEMENT_PROPERTYID_RangeLow,
            DTMELEMENT_PROPERTYID_RangeHigh
            };

    private:
        DTMElementInfoECDelegate (DTMELEMENTECExtension const& ext) : BaseElementECDelegate (ext) {}
        virtual ~DTMElementInfoECDelegate() { }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_Name, L"Name", NULL_MAP,
                DTMELEMENT_PROPERTYID_NumTriangles, L"NumTriangles", NULL_MAP,
                DTMELEMENT_PROPERTYID_NumBreaklines, L"NumBreaklines", NULL_MAP,
                DTMELEMENT_PROPERTYID_NumContours, L"NumContours", NULL_MAP,
                DTMELEMENT_PROPERTYID_NumFeatures, L"NumFeatures", NULL_MAP,
                DTMELEMENT_PROPERTYID_NumVoids, L"NumVoids", NULL_MAP,
                DTMELEMENT_PROPERTYID_NumIslands, L"NumIslands", NULL_MAP,
                DTMELEMENT_PROPERTYID_NumPointFeatures, L"NumPointFeatures", NULL_MAP,
                DTMELEMENT_PROPERTYID_NumPoints, L"NumPoints", NULL_MAP,
                DTMELEMENT_PROPERTYID_RangeLow, L"RangeLow", NULL_MAP,
                DTMELEMENT_PROPERTYID_RangeHigh, L"RangeHigh", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual bool _AttachToInstance (DelegatedElementECInstanceCR instance) const override
            {
            if (!instance.GetPerDelegateData (*this))
                instance.SetPerDelegateData (*this, DTMElementDataCache::Create (instance.GetElementHandle()));
            return true;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _Commit (EditElementHandleR eh, DelegatedElementECInstanceR instance) const override
            {
            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            return data->ToElement (eh);
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;
            const DTMFeatureStatisticsInfo& info = data->GetDTMInfo (instance.GetElementHandle());

            if (!data->IsSTM())
                {
                switch (propertyIndex)
                    {
                    case DTMELEMENT_PROPERTYID_Name: {ecValue.SetString (data->GetName().GetWCharCP()); break; }
                    case DTMELEMENT_PROPERTYID_NumTriangles: {ecValue.SetLong (info.numTriangles); break; }
                    case DTMELEMENT_PROPERTYID_NumBreaklines: {ecValue.SetLong (info.numBreaks); break; }
                    case DTMELEMENT_PROPERTYID_NumContours: {ecValue.SetLong (info.numContourLines); break; }
                    case DTMELEMENT_PROPERTYID_NumFeatures: {ecValue.SetLong (info.numDtmFeatures); break; }
                    case DTMELEMENT_PROPERTYID_NumVoids: {ecValue.SetLong (info.numVoids); break; }
                    case DTMELEMENT_PROPERTYID_NumIslands: {ecValue.SetLong (info.numIslands); break; }
                    case DTMELEMENT_PROPERTYID_NumPointFeatures: {ecValue.SetLong (info.numGroupSpots); break; }
                    case DTMELEMENT_PROPERTYID_NumPoints: {ecValue.SetLong (info.numPoints); break; }
                    case DTMELEMENT_PROPERTYID_RangeLow: {ecValue.SetPoint3D (data->GetDTMRange().low); break; }
                    case DTMELEMENT_PROPERTYID_RangeHigh: {ecValue.SetPoint3D (data->GetDTMRange().high); break; }

                    default: {status = ECOBJECTS_STATUS_Error; break; }
                    }
                }
            else
                {
                switch (propertyIndex)
                    {
                    case DTMELEMENT_PROPERTYID_NumBreaklines: {ecValue.SetLong (info.numBreaks); break; }
                    case DTMELEMENT_PROPERTYID_NumPoints: {ecValue.SetLong (info.numPoints); break; }
                    case DTMELEMENT_PROPERTYID_RangeLow: {ecValue.SetPoint3D (data->GetDTMRange().low); break; }
                    case DTMELEMENT_PROPERTYID_RangeHigh: {ecValue.SetPoint3D (data->GetDTMRange().high); break; }

                    default: {status = ECOBJECTS_STATUS_Error; break; }
                    }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Name: {data->SetName (ecValue.GetString ()); break; }
                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }

    public:

        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementInfoECDelegate (extension);
            }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementEdgeMethodECDelegate : BaseElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_EdgeMethod =  1,
            DTMELEMENT_PROPERTYID_LengthValue
            };

    private:

        DTMElementEdgeMethodECDelegate (DTMELEMENTECExtension const& ext) : BaseElementECDelegate (ext) {}
        virtual ~DTMElementEdgeMethodECDelegate() { }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_EdgeMethod, L"EdgeMethod", NULL_MAP,
                DTMELEMENT_PROPERTYID_LengthValue, L"LengthValue", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual bool _AttachToInstance (DelegatedElementECInstanceCR instance) const override
            {
            if (!instance.GetPerDelegateData (*this))
                instance.SetPerDelegateData (*this, DTMElementDataCache::Create (instance.GetElementHandle()));
            return true;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _Commit (EditElementHandleR eh, DelegatedElementECInstanceR instance) const override
            {
            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            return data->ToElement (eh);
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_EdgeMethod: {ecValue.SetInteger (data->GetEdgeOption()); break; }

                case DTMELEMENT_PROPERTYID_LengthValue:
                    {
                    if (data->GetEdgeOption () != 3)
                        ecValue.SetIsNull (true);
                    else
                        ecValue.SetDouble (data->GetEdgeLength());
                    break;
                    }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_EdgeMethod: {data->SetEdgeOption (ecValue.GetInteger ()); break; }
                case DTMELEMENT_PROPERTYID_LengthValue: {data->SetEdgeLength (ecValue.GetDouble ()); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR instance, UInt32 propertyIndex) const override
            {
            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (data == nullptr)
                {
                _AttachToInstance (instance);
                data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
                }
            if (!EXPECTED_CONDITION (nullptr != data))
                return true;
            if (propertyIndex == DTMELEMENT_PROPERTYID_LengthValue && data->GetEdgeOption () != 3)
                return true;
            return data->IsSTM();
            }

    public:

        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementEdgeMethodECDelegate (extension);
            }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementContourPropertiesECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_WantMajorContours =  1,
            DTMELEMENT_PROPERTYID_WantMinorContours,
            };

    private:
        DTMElementContourPropertiesECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementContourPropertiesECDelegate() { }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_WantMajorContours, L"WantMajorContours", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantMinorContours, L"WantMinorContours", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            const DTMElementSubHandler::SymbologyParams* param = nullptr;
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_WantMinorContours:
                    param = data->GetParamForType (DPID_MINORCONTOUR);
                    break;
                case DTMELEMENT_PROPERTYID_WantMajorContours:
                    param = data->GetParamForType (DPID_MAJORCONTOUR);
                    break;

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            if (param)
                ecValue.SetBoolean (param->GetVisible());
            else
                ecValue.SetIsNull (true);

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue (ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            DTMElementSubHandler::SymbologyParams* param = nullptr;
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_WantMinorContours:
                    param = data->GetParamForTypeForWrite (DPID_MINORCONTOUR);
                    break;
                case DTMELEMENT_PROPERTYID_WantMajorContours:
                    param = data->GetParamForTypeForWrite (DPID_MAJORCONTOUR);
                    break;

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }
            if (param)
                param->SetVisible (ecValue.GetBoolean ());

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }

    public:

        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementContourPropertiesECDelegate (extension);
            }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementFeaturePropertiesECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_WantTriangles =  1,
            DTMELEMENT_PROPERTYID_WantRasterDraping,
            DTMELEMENT_PROPERTYID_WantSpots,
            DTMELEMENT_PROPERTYID_WantRegions,
            DTMELEMENT_PROPERTYID_WantFlowArrows,
            DTMELEMENT_PROPERTYID_WantLowPoints,
            DTMELEMENT_PROPERTYID_WantHighPoints
            };

    private:
        DTMElementFeaturePropertiesECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementFeaturePropertiesECDelegate() { }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_WantTriangles, L"WantTriangles", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantRasterDraping, L"WantRasterDraping", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantSpots, L"WantSpots", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantRegions, L"WantRegions", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantFlowArrows, L"WantFlowArrows", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantLowPoints, L"WantLowPoints", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantHighPoints, L"WantHighPoints", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            const DTMElementSubHandler::SymbologyParams* param = nullptr;
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_WantTriangles:
                    param = data->GetParamForType (DPID_TRIANGLES);
                    break;
                case DTMELEMENT_PROPERTYID_WantRasterDraping:
                    param = data->GetParamForType (DPID_RASTERDRAPING);
                    break;
                case DTMELEMENT_PROPERTYID_WantSpots:
                    param = data->GetParamForType (DPID_SPOTS);
                    break;
                case DTMELEMENT_PROPERTYID_WantRegions:
                    param = nullptr; // data->GetParamForType (DPID_REGIONS);
                    break;
                case DTMELEMENT_PROPERTYID_WantFlowArrows:
                    param = data->GetParamForType (DPID_FLOWARROWS);
                    break;
                case DTMELEMENT_PROPERTYID_WantLowPoints:
                    param = data->GetParamForType (DPID_LOWPOINTS);
                    break;
                case DTMELEMENT_PROPERTYID_WantHighPoints:
                    param = data->GetParamForType (DPID_HIGHPOINTS);
                    break;

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }
            if (param)
                ecValue.SetBoolean (param->GetVisible());
            else
                ecValue.SetIsNull (true);

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            DTMElementSubHandler::SymbologyParams* param = nullptr;
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_WantTriangles:
                    param = data->GetParamForTypeForWrite (DPID_TRIANGLES);
                    break;
                case DTMELEMENT_PROPERTYID_WantRasterDraping:
                    param = data->GetParamForTypeForWrite (DPID_RASTERDRAPING);
                    break;
                case DTMELEMENT_PROPERTYID_WantSpots:
                    param = data->GetParamForTypeForWrite (DPID_SPOTS);
                    break;
                case DTMELEMENT_PROPERTYID_WantRegions:
                    param = nullptr; // data->GetParamForTypeForWrite (DPID_REGIONS);
                    break;
                case DTMELEMENT_PROPERTYID_WantFlowArrows:
                    param = data->GetParamForTypeForWrite (DPID_FLOWARROWS);
                    break;
                case DTMELEMENT_PROPERTYID_WantLowPoints:
                    param = data->GetParamForTypeForWrite (DPID_LOWPOINTS);
                    break;
                case DTMELEMENT_PROPERTYID_WantHighPoints:
                    param = data->GetParamForTypeForWrite (DPID_HIGHPOINTS);
                    break;

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            if (param)
                param->SetVisible (ecValue.GetBoolean ());

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }

    public:

        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementFeaturePropertiesECDelegate (extension);
            }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementSourcePropertiesECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_WantBreaklineFeatures =  1,
            DTMELEMENT_PROPERTYID_WantBoundaryFeatures,
            DTMELEMENT_PROPERTYID_WantContourFeatures,
            DTMELEMENT_PROPERTYID_WantIslandFeatures,
            DTMELEMENT_PROPERTYID_WantHoleFeatures,
            DTMELEMENT_PROPERTYID_WantVoidFeatures,
            DTMELEMENT_PROPERTYID_WantSpotFeatures
            };

    private:
        DTMElementSourcePropertiesECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementSourcePropertiesECDelegate() { }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_WantBreaklineFeatures, L"WantBreaklineFeatures", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantBoundaryFeatures, L"WantBoundaryFeatures", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantContourFeatures, L"WantContourFeatures", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantIslandFeatures, L"WantIslandFeatures", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantHoleFeatures, L"WantHoleFeatures", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantVoidFeatures, L"WantVoidFeatures", NULL_MAP,
                DTMELEMENT_PROPERTYID_WantSpotFeatures, L"WantSpotFeatures", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            const DTMElementSubHandler::SymbologyParams* param = nullptr;
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_WantBreaklineFeatures:
                    param = data->GetParamForType (DPID_BREAKLINES);
                    break;
                case DTMELEMENT_PROPERTYID_WantBoundaryFeatures:
                    param = data->GetParamForType (DPID_BOUNDARIES);
                    break;
                case DTMELEMENT_PROPERTYID_WantContourFeatures:
                    param = data->GetParamForType (DPID_CONTOURS);
                    break;
                case DTMELEMENT_PROPERTYID_WantIslandFeatures:
                    param = data->GetParamForType (DPID_ISLANDS);
                    break;
                case DTMELEMENT_PROPERTYID_WantHoleFeatures:
                    param = data->GetParamForType (DPID_HOLES);
                    break;
                case DTMELEMENT_PROPERTYID_WantVoidFeatures:
                    param = data->GetParamForType (DPID_VOIDS);
                    break;
                case DTMELEMENT_PROPERTYID_WantSpotFeatures:
                    param = data->GetParamForType (DPID_FEATURESPOTS);
                    break;

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            if (param)
                ecValue.SetBoolean (param->GetVisible());
            else
                ecValue.SetIsNull (true);

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            DTMElementSubHandler::SymbologyParams* param = nullptr;
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_WantBreaklineFeatures:
                    param = data->GetParamForTypeForWrite (DPID_BREAKLINES);
                    break;
                case DTMELEMENT_PROPERTYID_WantBoundaryFeatures:
                    param = data->GetParamForTypeForWrite (DPID_BOUNDARIES);
                    break;
                case DTMELEMENT_PROPERTYID_WantContourFeatures:
                    param = data->GetParamForTypeForWrite (DPID_CONTOURS);
                    break;
                case DTMELEMENT_PROPERTYID_WantIslandFeatures:
                    param = data->GetParamForTypeForWrite (DPID_ISLANDS);
                    break;
                case DTMELEMENT_PROPERTYID_WantHoleFeatures:
                    param = data->GetParamForTypeForWrite (DPID_HOLES);
                    break;
                case DTMELEMENT_PROPERTYID_WantVoidFeatures:
                    param = data->GetParamForTypeForWrite (DPID_VOIDS);
                    break;
                case DTMELEMENT_PROPERTYID_WantSpotFeatures:
                    param = data->GetParamForTypeForWrite (DPID_FEATURESPOTS);
                    break;

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            if (param)
                param->SetVisible (ecValue.GetBoolean ());

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }

    public:

        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementSourcePropertiesECDelegate (extension);
            }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementSubElementDisplayECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_Level =  1,
            DTMELEMENT_PROPERTYID_Color,
            DTMELEMENT_PROPERTYID_Weight,
            DTMELEMENT_PROPERTYID_LineStyle,
            DTMELEMENT_PROPERTYID_Transparency,
            DTMELEMENT_PROPERTYID_Display,
            };

    private:
        DTMElementSubElementDisplayECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementSubElementDisplayECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementSubElementDisplayECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_Level, L"Level", NULL_MAP,
                DTMELEMENT_PROPERTYID_Color, L"Color", NULL_MAP,
                DTMELEMENT_PROPERTYID_Weight, L"Weight", NULL_MAP,
                DTMELEMENT_PROPERTYID_LineStyle, L"LineStyle", NULL_MAP,
                DTMELEMENT_PROPERTYID_Transparency, L"Transparency", NULL_MAP,
                DTMELEMENT_PROPERTYID_Display, L"Display", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            const DTMElementSubHandler::SymbologyParams* params = data->GetParam (instance.GetLocalId());
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Level:{ ecValue.SetInteger (params->GetLevelId()); break; }
                case DTMELEMENT_PROPERTYID_Color: { ecValue.SetInteger (params->GetSymbology().color); break; }
                case DTMELEMENT_PROPERTYID_Weight: { ecValue.SetInteger (params->GetSymbology().weight); break; }
                case DTMELEMENT_PROPERTYID_LineStyle: { ecValue.SetInteger (params->GetSymbology().style); break; }
                case DTMELEMENT_PROPERTYID_Transparency: { ecValue.SetDouble (params->GetTransparency() * 100.0); break; }
                case DTMELEMENT_PROPERTYID_Display: { ecValue.SetBoolean (params->GetVisible()); break; }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }
            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            DTMElementSubHandler::SymbologyParams* params = data->GetParamForWrite (instance.GetLocalId());
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Level: { params->SetLevelId (ecValue.GetInteger ()); break; }
                case DTMELEMENT_PROPERTYID_Color: { params->GetSymbology().color = ecValue.GetInteger (); break; }
                case DTMELEMENT_PROPERTYID_Weight: { params->GetSymbology().weight = ecValue.GetInteger (); break; }
                case DTMELEMENT_PROPERTYID_LineStyle: { params->GetSymbology().style = ecValue.GetInteger (); break; }
                case DTMELEMENT_PROPERTYID_Transparency: { params->SetTransparency (ecValue.GetDouble () / 100.0); break; }
                case DTMELEMENT_PROPERTYID_Display: { params->SetVisible (ecValue.GetBoolean ()); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }

    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementGeneralContoursECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_ContourSmoothingOptions =  1,
            DTMELEMENT_PROPERTYID_ContourSmoothingFactor,
            DTMELEMENT_PROPERTYID_ContourMaxSlopeOptions,
            DTMELEMENT_PROPERTYID_ContourMaxSlopeValue,
            DTMELEMENT_PROPERTYID_ContourLabelPrecision,
            DTMELEMENT_PROPERTYID_MajorContourInterval,
            DTMELEMENT_PROPERTYID_MinorContourInterval
            };

    private:
        DTMElementGeneralContoursECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementGeneralContoursECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementGeneralContoursECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_ContourSmoothingOptions, L"ContourSmoothingOptions", NULL_MAP,
                DTMELEMENT_PROPERTYID_ContourSmoothingFactor, L"ContourSmoothingFactor", NULL_MAP,
                DTMELEMENT_PROPERTYID_ContourMaxSlopeOptions, L"ContourMaxSlopeOptions", NULL_MAP,
                DTMELEMENT_PROPERTYID_ContourMaxSlopeValue, L"ContourMaxSlopeValue", NULL_MAP,
                DTMELEMENT_PROPERTYID_ContourLabelPrecision, L"ContourLabelPrecision", NULL_MAP,
                DTMELEMENT_PROPERTYID_MajorContourInterval, L"MajorContourInterval", NULL_MAP,
                DTMELEMENT_PROPERTYID_MinorContourInterval, L"MinorContourInterval", NULL_MAP,

                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;
            const DTMElementContoursHandler::DisplayParams* majorParam = (const DTMElementContoursHandler::DisplayParams*)data->GetParamForType (DPID_MAJORCONTOUR);
            const DTMElementContoursHandler::DisplayParams* minorParam = (const DTMElementContoursHandler::DisplayParams*)data->GetParamForType (DPID_MINORCONTOUR);
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_ContourSmoothingOptions: { ecValue.SetInteger (majorParam->GetContourSmoothing()); break; }
                case DTMELEMENT_PROPERTYID_ContourSmoothingFactor: { if (majorParam->GetContourSmoothing() == (int)DTMContourSmoothing::None) ecValue.SetIsNull (true); else ecValue.SetInteger (majorParam->GetSmoothingFactor()); break; }
                case DTMELEMENT_PROPERTYID_ContourMaxSlopeOptions: { ecValue.SetInteger (majorParam->GetMaxSlopeOption()); break; }
                case DTMELEMENT_PROPERTYID_ContourMaxSlopeValue: { ecValue.SetDouble (majorParam->GetMaxSlopeValue()); break; }
                case DTMELEMENT_PROPERTYID_ContourLabelPrecision: { ecValue.SetInteger (majorParam->GetContourLabelPrecision()); break; }
                case DTMELEMENT_PROPERTYID_MajorContourInterval: { ecValue.SetDouble (majorParam->GetContourInterval()); break; }
                case DTMELEMENT_PROPERTYID_MinorContourInterval: { ecValue.SetDouble (minorParam->GetContourInterval()); break; }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            DTMElementContoursHandler::DisplayParams* majorParam = (DTMElementContoursHandler::DisplayParams*)data->GetParamForTypeForWrite (DPID_MAJORCONTOUR);
            DTMElementContoursHandler::DisplayParams* minorParam = (DTMElementContoursHandler::DisplayParams*)data->GetParamForTypeForWrite (DPID_MINORCONTOUR);

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_ContourSmoothingOptions: { majorParam->SetContourSmoothing (/*(DTMContourSmoothing)*/ecValue.GetInteger ()); minorParam->SetContourSmoothing (/*(DTMContourSmoothing)*/ecValue.GetInteger ()); break; }
                case DTMELEMENT_PROPERTYID_ContourSmoothingFactor: { majorParam->SetSmoothingFactor (ecValue.GetInteger ()); minorParam->SetSmoothingFactor (ecValue.GetInteger()); break; }
                case DTMELEMENT_PROPERTYID_ContourMaxSlopeOptions: { majorParam->SetMaxSlopeOption (ecValue.GetInteger ()); minorParam->SetMaxSlopeOption (ecValue.GetInteger ()); break; }
                case DTMELEMENT_PROPERTYID_ContourMaxSlopeValue: { majorParam->SetMaxSlopeValue (ecValue.GetDouble ()); minorParam->SetMaxSlopeValue (ecValue.GetDouble ()); break; }
                case DTMELEMENT_PROPERTYID_ContourLabelPrecision: { majorParam->SetContourLabelPrecision (ecValue.GetInteger ());  minorParam->SetContourLabelPrecision (ecValue.GetInteger ()); break; }
                case DTMELEMENT_PROPERTYID_MajorContourInterval: { majorParam->SetContourInterval (ecValue.GetDouble ()); break; }
                case DTMELEMENT_PROPERTYID_MinorContourInterval: { minorParam->SetContourInterval (ecValue.GetDouble ()); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementDepressionSymbologyECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_Color = 1,
            DTMELEMENT_PROPERTYID_Weight,
            DTMELEMENT_PROPERTYID_LineStyle,
            };

    private:
        DTMElementDepressionSymbologyECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementDepressionSymbologyECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementDepressionSymbologyECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_Color, L"DepressionColor", NULL_MAP,
                DTMELEMENT_PROPERTYID_Weight, L"DepressionWeight", NULL_MAP,
                DTMELEMENT_PROPERTYID_LineStyle, L"DepressionStyle", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            const DTMElementContoursHandler::DisplayParams* param = (const DTMElementContoursHandler::DisplayParams*)data->GetParam (instance.GetLocalId());
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Color: { ecValue.SetInteger (param->GetDepressionSymbology().color); break; }
                case DTMELEMENT_PROPERTYID_Weight: { ecValue.SetInteger (param->GetDepressionSymbology().weight); break; }
                case DTMELEMENT_PROPERTYID_LineStyle: { ecValue.SetInteger (param->GetDepressionSymbology().style); break; }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            DTMElementContoursHandler::DisplayParams* param = (DTMElementContoursHandler::DisplayParams*)data->GetParamForWrite (instance.GetLocalId());
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Color: { param->GetDepressionSymbology().color = ecValue.GetInteger (); break; }
                case DTMELEMENT_PROPERTYID_Weight: { param->GetDepressionSymbology().weight = ecValue.GetInteger (); break; }
                case DTMELEMENT_PROPERTYID_LineStyle: { param->GetDepressionSymbology().style = ecValue.GetInteger (); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementContourECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_ContourDrawText = 1,
            DTMELEMENT_PROPERTYID_ContourTextInterval,
            DTMELEMENT_PROPERTYID_ContourTextLevel
            };

    private:
        DTMElementContourECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementContourECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementContourECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_ContourDrawText, L"ContourDrawText", NULL_MAP,
                DTMELEMENT_PROPERTYID_ContourTextInterval, L"ContourTextInterval", NULL_MAP,
                DTMELEMENT_PROPERTYID_ContourTextLevel, L"ContourTextLevel", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            const DTMElementContoursHandler::DisplayParams* param = (const DTMElementContoursHandler::DisplayParams*)data->GetParam (instance.GetLocalId());
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_ContourDrawText: { ecValue.SetBoolean (param->GetDrawTextOption() != 0); break; }
                case DTMELEMENT_PROPERTYID_ContourTextInterval: { ecValue.SetDouble (param->GetTextInterval ()); break; }
                case DTMELEMENT_PROPERTYID_ContourTextLevel: { ecValue.SetInteger (data->GetParamTextLevelId(instance.GetLocalId())); break; }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            DTMElementContoursHandler::DisplayParams* param = (DTMElementContoursHandler::DisplayParams*)data->GetParamForWrite (instance.GetLocalId());
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_ContourDrawText: { param->SetDrawTextOption ((DTMElementContoursHandler::DrawTextOption)ecValue.GetBoolean ()); break; }
                case DTMELEMENT_PROPERTYID_ContourTextInterval: { param->SetTextInterval (ecValue.GetDouble ()); break; }
                case DTMELEMENT_PROPERTYID_ContourTextLevel: {  data->SetParamTextLevelId (instance.GetLocalId (), ecValue.GetInteger ()); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementTextStyleECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_TextStyle = 1,
            };

    private:
        DTMElementTextStyleECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementTextStyleECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementTextStyleECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_TextStyle, L"TextStyle", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_TextStyle: { ecValue.SetInteger (data->GetParamTextStyleId (instance.GetLocalId())); break; }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_TextStyle: { data->SetParamTextStyleId (instance.GetLocalId(), ecValue.GetInteger()); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementPointSymbologyECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PointCellPropertyId
            {
            DTMELEMENT_PROPERTYID_PointCell_Type = 1,
            DTMELEMENT_PROPERTYID_PointCell_Value,
            DTMELEMENT_PROPERTYID_PointCell_Criteria,
            };
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_WantText = 1,
            DTMELEMENT_PROPERTYID_PrefixText,
            DTMELEMENT_PROPERTYID_SuffixText,
            DTMELEMENT_PROPERTYID_Point,
            DTMELEMENT_PROPERTYID_Point_Type,
            DTMELEMENT_PROPERTYID_Point_Value,
            DTMELEMENT_PROPERTYID_Point_Criteria,
            DTMELEMENT_PROPERTYID_Scale,
            };

    private:
        DTMElementPointSymbologyECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext)
            {
            }
        virtual ~DTMElementPointSymbologyECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementPointSymbologyECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP2[] =
                {
                DTMELEMENT_PROPERTYID_PointCell_Type, L"Type", NULL_MAP,
                DTMELEMENT_PROPERTYID_PointCell_Value, L"Value", NULL_MAP,
                DTMELEMENT_PROPERTYID_PointCell_Criteria, L"Criteria", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_WantText, L"WantText", NULL_MAP,
                DTMELEMENT_PROPERTYID_PrefixText, L"PrefixText", NULL_MAP,
                DTMELEMENT_PROPERTYID_SuffixText, L"SuffixText", NULL_MAP,
                DTMELEMENT_PROPERTYID_Point, L"Point", s_BESMAP2,
                DTMELEMENT_PROPERTYID_Scale, L"Scale", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

        ECObjectsStatus status = ECOBJECTS_STATUS_Success;
        const DTMElementPointsHandler::DisplayParams* param = (const DTMElementPointsHandler::DisplayParams*)data->GetParam (instance.GetLocalId());
        switch (propertyIndex)
            {
            case DTMELEMENT_PROPERTYID_Point_Type: { ecValue.SetInteger (param->GetPointCellType()); break; }
            case DTMELEMENT_PROPERTYID_Point_Value: { ecValue.SetString (param->GetPointCellName().GetWCharCP()); break; }
            case DTMELEMENT_PROPERTYID_Point_Criteria: { ecValue.SetString (L""); break; }
            case DTMELEMENT_PROPERTYID_Scale: { ecValue.SetDouble (param->GetPointCellSize().x); break; }
            case DTMELEMENT_PROPERTYID_WantText:
            case DTMELEMENT_PROPERTYID_PrefixText:
            case DTMELEMENT_PROPERTYID_SuffixText:
                if (data->GetIDForType (DPID_FLOWARROWS) == instance.GetLocalId ())
                    ecValue.SetIsNull (true);
                else
                    {
                    switch (propertyIndex)
                        {
                        case DTMELEMENT_PROPERTYID_WantText: { ecValue.SetBoolean (param->GetWantPointText()); break; }
                        case DTMELEMENT_PROPERTYID_PrefixText: { ecValue.SetString (param->GetPointTextPrefix().GetWCharCP()); break; }
                        case DTMELEMENT_PROPERTYID_SuffixText: { ecValue.SetString (param->GetPointTextSuffix().GetWCharCP()); break; }

                        };
                    }
                break;
            default: {status = ECOBJECTS_STATUS_Error; break; }
            }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            DTMElementPointsHandler::DisplayParams* param = (DTMElementPointsHandler::DisplayParams*)data->GetParamForWrite (instance.GetLocalId());
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_WantText: { param->SetWantPointText (ecValue.GetBoolean ()); break; }
                case DTMELEMENT_PROPERTYID_PrefixText: { param->SetPointTextPrefix (ecValue.GetString ()); break; }
                case DTMELEMENT_PROPERTYID_SuffixText: { param->SetPointTextSuffix (ecValue.GetString ()); break; }
                case DTMELEMENT_PROPERTYID_Point_Type: { param->SetPointCellType ((DTMElementPointsHandler::DisplayParams::PointCellType)ecValue.GetInteger ()); break; }
                case DTMELEMENT_PROPERTYID_Point_Value: { param->SetPointCellName (ecValue.GetString ()); break; }
//                case DTMELEMENT_PROPERTYID_Point_Criteria: { ecValue.SetString (L""); break; }
                case DTMELEMENT_PROPERTYID_Scale: { double value = ecValue.GetDouble(); param->SetPointCellSize (DPoint3d::From (value, value, value)); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementLowPointsECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_MinDepth = 1,
            };

    private:
        DTMElementLowPointsECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementLowPointsECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementLowPointsECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_MinDepth, L"MinDepth", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            const DTMElementLowPointsHandler::DisplayParams* param = (const DTMElementLowPointsHandler::DisplayParams*)data->GetParam (instance.GetLocalId());
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_MinDepth: { ecValue.SetDouble (param->GetLowPointMinimumDepth()); break; }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            DTMElementLowPointsHandler::DisplayParams* param = (DTMElementLowPointsHandler::DisplayParams*)data->GetParamForWrite (instance.GetLocalId());
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_MinDepth: { param->SetLowPointMinimumDepth (ecValue.GetDouble ()); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementTrianglesECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_ThematicDisplay = 1,
            };

    private:
        DTMElementTrianglesECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementTrianglesECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementTrianglesECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_ThematicDisplay, L"ThematicDisplayStyle", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_ThematicDisplay: { ecValue.SetInteger (data->DisplayStyleId()); break; }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_ThematicDisplay: { data->DisplayStyleId (ecValue.GetInteger()); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementRegionsECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_Description = 1,
            DTMELEMENT_PROPERTYID_RegionTag,
            };

    private:
        DTMElementRegionsECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementRegionsECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementRegionsECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_Description, L"Description", NULL_MAP,
                DTMELEMENT_PROPERTYID_RegionTag, L"RegionTag", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            const DTMElementRegionsHandler::DisplayParams* param = (const DTMElementRegionsHandler::DisplayParams*)data->GetParam (instance.GetLocalId());
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Description: { ecValue.SetString (param->GetDescription().GetWCharCP()); break; }
                case DTMELEMENT_PROPERTYID_RegionTag: { WString tag; tag.Sprintf (L"%ld", param->GetTag()); ecValue.SetString (tag.GetWCharCP()); break; }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            return ECOBJECTS_STATUS_Error;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementAttachedMaterialECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_AttachedMaterial = 1,
            };

    private:
        DTMElementAttachedMaterialECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementAttachedMaterialECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementAttachedMaterialECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_AttachedMaterial, L"AttachedMaterial", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_AttachedMaterial: { ecValue.SetString (data->GetParamMaterial (instance.GetLocalId())); break; }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_AttachedMaterial: { data->SetParamMaterial (instance.GetLocalId(), ecValue.GetString()); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }
    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementOSTemplateECDelegate : BaseOSElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_Template = 1,
            };

    private:
        DTMElementOSTemplateECDelegate (DTMELEMENTECExtension const& ext) : BaseOSElementECDelegate (ext) {}
        virtual ~DTMElementOSTemplateECDelegate() { }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementOSTemplateECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_Template, L"OSTemplate", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            // ToDo ---
            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Template:
                    {
                    if (data->HasOverrideSymbology ())
                        ecValue.SetLong ((Int64)data->TemplateElementId());
                    else
                        ecValue.SetIsNull (true);
                    break;
                    }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Template: { data->TemplateElementId ((ElementId)ecValue.GetLong ()); break; }

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual bool _IsNullValue (DelegatedElementECInstanceCR instance, UInt32 localId) const override
            {
            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (data == nullptr)
                {
                _AttachToInstance (instance);
                data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
                }
            if (!EXPECTED_CONDITION (nullptr != data))
                return true;
            return data->m_activeModel || !data->HasOverrideSymbology ();
            }

    };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementGraphHeaderECDelegate : BaseElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_Level = 1,
            DTMELEMENT_PROPERTYID_Color,
            DTMELEMENT_PROPERTYID_Weight,
            DTMELEMENT_PROPERTYID_Style,
            DTMELEMENT_PROPERTYID_LineStyleParams
            };

    private:
        DTMElementGraphHeaderECDelegate (DTMELEMENTECExtension const& ext) : BaseElementECDelegate (ext)
            {
            }
        virtual ~DTMElementGraphHeaderECDelegate ()
            {
            }

    public:
        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementGraphHeaderECDelegate (extension);
            }

    protected:
        virtual const MapEntry* _GetMap () const override
            {
            //static const ElementECDelegate::MapEntry s_BESMAP_LineStyleParametersStruct[] =
            //    {
            //    BESINDEX_LineStyleParametersStruct_LSScale, L"LSScale", NULL_MAP,
            //    BESINDEX_LineStyleParametersStruct_LSWidthMode, L"LSWidthMode", NULL_MAP,
            //    BESINDEX_LineStyleParametersStruct_LSStartWidth, L"LSStartWidth", NULL_MAP,
            //    BESINDEX_LineStyleParametersStruct_LSEndWidth, L"LSEndWidth", NULL_MAP,
            //    BESINDEX_LineStyleParametersStruct_LSGlobalWidth, L"LSGlobalWidth", NULL_MAP,
            //    BESINDEX_LineStyleParametersStruct_LSTrueWidth, L"LSTrueWidth", NULL_MAP,
            //    BESINDEX_LineStyleParametersStruct_LSShiftMode, L"LSShiftMode", NULL_MAP,
            //    BESINDEX_LineStyleParametersStruct_LSDistancePhase, L"LSDistancePhase", NULL_MAP,
            //    BESINDEX_LineStyleParametersStruct_LSFractionPhase, L"LSFractionPhase", NULL_MAP,
            //    BESINDEX_LineStyleParametersStruct_LSCornerMode, L"LSCornerMode", NULL_MAP,
            //    BESINDEX_LineStyleParametersStruct_LSThroughCorner, L"LSThroughCorner", NULL_MAP,
            //    ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
            //    };
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_Level, L"Level", NULL_MAP,
                DTMELEMENT_PROPERTYID_Color, L"Color", NULL_MAP,
                DTMELEMENT_PROPERTYID_Weight, L"Weight", NULL_MAP,
                DTMELEMENT_PROPERTYID_Style, L"Style", NULL_MAP,
                //DTMELEMENT_PROPERTYID_LineStyleParams, L"LineStyleParams", s_BESMAP_LineStyleParametersStruct,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override
            {
            return false;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Level:
                    ecValue.SetInteger (data->GetHeaderLevel ());
                    break;
                case DTMELEMENT_PROPERTYID_Color:
                    ecValue.SetInteger (data->GetHeaderColor());
                    break;
                case DTMELEMENT_PROPERTYID_Weight:
                    ecValue.SetInteger (data->GetHeaderWeight());
                    break;
                case DTMELEMENT_PROPERTYID_Style:
                    ecValue.SetInteger (data->GetHeaderLsId());
                    break;

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            if (useArrayIndex)
                return ECOBJECTS_STATUS_OperationNotSupported;

            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Level:
                    data->SetHeaderLevel (ecValue.GetInteger());
                    break;
                case DTMELEMENT_PROPERTYID_Color:
                    data->SetHeaderColor (ecValue.GetInteger ());
                    break;
                case DTMELEMENT_PROPERTYID_Weight:
                    data->SetHeaderWeight (ecValue.GetInteger ());
                    break;
                case DTMELEMENT_PROPERTYID_Style:
                    {
                    int lsId = ecValue.GetInteger ();
                    data->SetHeaderLsId(lsId);
                    //if ((0 >= lsId && 7 <= lsId) && IsModified (DTMELEMENT_PROPERTYID_LineStyleParams))
                    //    {
                    //    // You can only modify LSParams if not a standard line style ID
                    //    MarkUnmodified (DTMELEMENT_PROPERTYID_LineStyleParams);
                    //    data->ClearHeaderLsParams();
                    //    }
                    }
                    break;

                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        virtual bool _IsPropertyReadOnly (DelegatedElementECInstanceCR instance, UInt32 propertyIndex) const override
            {
            //if (isHeaderPropertyDisabled (instance, propIdx, true))
            //    return true;
            return false;
            }

    };

    //*======================================================================================
    //* @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct DTMElementOverrideSymbologyECDelegate : BaseElementECDelegate
        {
        public:
            enum PropertyId
                {
                DTMELEMENT_PROPERTYID_OverrideSymbology = 1,
                };

        private:
            DTMElementOverrideSymbologyECDelegate (DTMELEMENTECExtension const& ext) : BaseElementECDelegate (ext)
                {
                }
            virtual ~DTMElementOverrideSymbologyECDelegate ()
                {
                }

        public:
            static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
                {
                return new DTMElementOverrideSymbologyECDelegate (extension);
                }

        protected:
            virtual const MapEntry* _GetMap () const override
                {
                static const ElementECDelegate::MapEntry s_BESMAP[] =
                    {
                    DTMELEMENT_PROPERTYID_OverrideSymbology, L"OverrideSymbology", NULL_MAP,
                    ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                    };
                return s_BESMAP;
                }

            //*--------------------------------------------------------------------------------------
            //* @bsimethod
            //+---------------+---------------+---------------+---------------+---------------+------
            virtual bool _IsPropertyReadOnlyInLockedElement (DelegatedElementECInstanceCR instance, UInt32 propertyIndex) const override
                {
                DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
                if (!EXPECTED_CONDITION (nullptr != data))
                    return ElementECDelegate::_IsPropertyReadOnlyInLockedElement (instance, propertyIndex);
                return data->IsActiveModelReadOnly ();
                }


            //*--------------------------------------------------------------------------------------
            //* @bsimethod
            //+---------------+---------------+---------------+---------------+---------------+------
            virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
                {
                if (useArrayIndex)
                    return ECOBJECTS_STATUS_OperationNotSupported;

                DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
                if (!EXPECTED_CONDITION (nullptr != data))
                    return ECOBJECTS_STATUS_Error;

                ECObjectsStatus status = ECOBJECTS_STATUS_Success;

                switch (propertyIndex)
                    {
                    case DTMELEMENT_PROPERTYID_OverrideSymbology:
                        {
                        if (data->CanHaveOverrideSymbology ())
                            ecValue.SetBoolean (data->HasOverrideSymbology ());
                        else
                            ecValue.SetIsNull (true);
                        break;
                        }

                    default: {status = ECOBJECTS_STATUS_Error; break; }
                    }

                return status;
                }

            //*--------------------------------------------------------------------------------------
            //* @bsimethod
            //+---------------+---------------+---------------+---------------+---------------+------
            virtual ECObjectsStatus _SetValue (ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
                {
                if (useArrayIndex)
                    return ECOBJECTS_STATUS_OperationNotSupported;

                DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
                if (!EXPECTED_CONDITION (nullptr != data))
                    return ECOBJECTS_STATUS_Error;

                ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

                switch (propertyIndex)
                    {
                    case DTMELEMENT_PROPERTYID_OverrideSymbology: { data->HasOverrideSymbology (ecValue.GetBoolean ()); break; }

                    default: { status = ECOBJECTS_STATUS_Error; break; }
                    }

                return status;
                }

            //*--------------------------------------------------------------------------------------
            //* @bsimethod
            //+---------------+---------------+---------------+---------------+---------------+------
            virtual bool _IsNullValue (DelegatedElementECInstanceCR instance, UInt32 localId) const override
                {
                DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
                if (data == nullptr)
                    {
                    _AttachToInstance (instance);
                    data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
                    }
                if (!EXPECTED_CONDITION (nullptr != data))
                    return true;
                return data->m_activeModel || !data->CanHaveOverrideSymbology ();
                }
            virtual bool _IsPropertyReadOnly (DelegatedElementECInstanceCR instance, UInt32 propertyIndex) const override
                {
                return false;
                }

        };

//*======================================================================================
//* @bsiclass
//+===============+===============+===============+===============+===============+======
struct DTMElementScalableTerrainModelECDelegate : BaseElementECDelegate
    {
    public:
        enum PropertyId
            {
            DTMELEMENT_PROPERTYID_Locate =  1,
            DTMELEMENT_PROPERTYID_Clip,
            DTMELEMENT_PROPERTYID_UnshadedViewPointDensity,
            DTMELEMENT_PROPERTYID_ShadedViewPointDensity,
            DTMELEMENT_PROPERTYID_Views
            };

    private:
        DTMElementScalableTerrainModelECDelegate (DTMELEMENTECExtension const& ext) : BaseElementECDelegate (ext) {}
        virtual ~DTMElementScalableTerrainModelECDelegate() { }

    protected:
        virtual const MapEntry* _GetMap() const override
            {
            static const ElementECDelegate::MapEntry s_BESMAP[] =
                {
                DTMELEMENT_PROPERTYID_Locate, L"Locate", NULL_MAP,
                DTMELEMENT_PROPERTYID_Clip, L"Clip", NULL_MAP,
                DTMELEMENT_PROPERTYID_UnshadedViewPointDensity, L"UnshadedViewPointDensity", NULL_MAP,
                DTMELEMENT_PROPERTYID_ShadedViewPointDensity, L"ShadedViewPointDensity", NULL_MAP,
                DTMELEMENT_PROPERTYID_Views, L"Views", NULL_MAP,
                ElementECDelegate::END_OF_MAP, NULL_ACCESSOR, NULL_MAP
                };
            return s_BESMAP;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual bool _AttachToInstance (DelegatedElementECInstanceCR instance) const override
            {
            if (!instance.GetPerDelegateData (*this))
                instance.SetPerDelegateData (*this, DTMElementDataCache::Create (instance.GetElementHandle()));
            return true;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _Commit (EditElementHandleR eh, DelegatedElementECInstanceR instance) const override
            {
            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            return data->ToElement (eh);
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _GetValue (ECValueR ecValue, DelegatedElementECInstanceCR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus status = ECOBJECTS_STATUS_Success;

            if (!data->IsSTM())
                {
                ecValue.SetIsNull (true);
                return status;
                }

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Locate: {ecValue.SetBoolean (data->GetMrDTMSpecificData ().canLocate); break; }
                case DTMELEMENT_PROPERTYID_Clip: {ecValue.SetBoolean (data->GetMrDTMSpecificData ().clipActivation); break; }
                case DTMELEMENT_PROPERTYID_UnshadedViewPointDensity:
                    {
                    int unshadedViewPointDensity = (int)MRDTM_VIEW_POINT_DENSITY_TO_GUI(data->GetMrDTMSpecificData().unshadedViewPointDensity);
                    ecValue.SetDouble ((double)unshadedViewPointDensity);
                    break;
                    }

                case DTMELEMENT_PROPERTYID_ShadedViewPointDensity:
                    {
                    int shadedViewPointDensity = (int)MRDTM_VIEW_POINT_DENSITY_TO_GUI(data->GetMrDTMSpecificData().shadedViewPointDensity);
                    ecValue.SetDouble ((double)shadedViewPointDensity);
                    break;
                    }

                case DTMELEMENT_PROPERTYID_Views:
                    {
                    if (!useArrayIndex)
                        {
                        ecValue.SetPrimitiveArrayInfo (Bentley::ECN::PRIMITIVETYPE_Boolean, MAX_VIEWS, true);
                        }
                    else
                        {
                        if (arrayIndex >= MAX_VIEWS)
                            {
                            return Bentley::ECN::ECOBJECTS_STATUS_IndexOutOfRange;
                            }

                        bool viewIsOn = data->GetMrDTMSpecificData().viewFlags[arrayIndex];
                        ecValue.SetBoolean (viewIsOn);
                        }
                    break;
                    }

                default: {status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }

        //*--------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual ECObjectsStatus _SetValue(ECN::ECValueCR ecValue, DelegatedElementECInstanceR instance, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
            {
            DTMElementDataCache* data = dynamic_cast<DTMElementDataCache*> (instance.GetPerDelegateData (*this));
            if (!EXPECTED_CONDITION (nullptr != data))
                return ECOBJECTS_STATUS_Error;

            ECObjectsStatus   status = ECOBJECTS_STATUS_Success;

            switch (propertyIndex)
                {
                case DTMELEMENT_PROPERTYID_Locate:
                    {
                    data->SetMrDTMCanLocate (ecValue.GetBoolean());
                    break;
                    }
                case DTMELEMENT_PROPERTYID_Clip:
                    {
                    data->SetMrDTMClipActivation (ecValue.GetBoolean());
                    break;
                    }
                case DTMELEMENT_PROPERTYID_UnshadedViewPointDensity:
                    {
                    data->SetMrDTMUnshadedViewPointDensity (MRDTM_GUI_TO_VIEW_POINT_DENSITY(ecValue.GetDouble()));
                    break;
                    }
                case DTMELEMENT_PROPERTYID_ShadedViewPointDensity:
                    {
                    data->SetMrDTMShadedViewPointDensity (MRDTM_GUI_TO_VIEW_POINT_DENSITY(ecValue.GetDouble()));
                    break;
                    }
                case DTMELEMENT_PROPERTYID_Views:
                    {
                    data->SetMrDTMViewFlag (arrayIndex, ecValue.GetBoolean());
                    break;
                    }
                default: { status = ECOBJECTS_STATUS_Error; break; }
                }

            return status;
            }
        virtual bool _IsNullValue (DelegatedElementECInstanceCR, UInt32) const override { return false; }

    public:

        static ElementECDelegateP  Create (DTMELEMENTECExtension const& extension)
            {
            return new DTMElementScalableTerrainModelECDelegate (extension);
            }
    };

//*--------------------------------------------------------------------------------------
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void DTMELEMENTECExtension::_GetECClasses (T_ECClassCPVector& classes) const
    {
    for (ECTable::const_iterator it = GetMapTable().begin(); it != GetMapTable().end(); it++)
        classes.push_back (it->second.ecClass);
    }

//*--------------------------------------------------------------------------------------
//* @bsimethod                                                    Daryl.Holmwood   09/12
//+---------------+---------------+---------------+---------------+---------------+------
ElementECDelegatePtr DTMELEMENTECExtension::_SupplyDelegate (ECClassCR ecClass) const
    {
    return GetMapTable()[ecClass.GetName()].ecDelegate.get();
    }

DTMELEMENTECExtension::ECTable& DTMELEMENTECExtension::GetMapTable () const
    {
    if (m_ecTable.size())
        return m_ecTable;
    static ECClassEntry DTMClasses[] =
        {
                { L"DTMElement", nullptr }, //&DTMElementECDelegate::Create},
                { L"DTMInformation", &DTMElementInfoECDelegate::Create },
                { L"DTMEdgeMethod", &DTMElementEdgeMethodECDelegate::Create },
                { L"DTMContourProperties", &DTMElementContourPropertiesECDelegate::Create },
                { L"DTMFeatureProperties", &DTMElementFeaturePropertiesECDelegate::Create },
                { L"DTMSourceProperties", &DTMElementSourcePropertiesECDelegate::Create },
                { L"DTMCalculatedProperties", nullptr },
                { L"DTMSubElementDisplay", &DTMElementSubElementDisplayECDelegate::Create },
                { L"DTMGeneralContours", &DTMElementGeneralContoursECDelegate::Create },
                { L"DTMDepressionSymbology", &DTMElementDepressionSymbologyECDelegate::Create },
                { L"DTMTextStyle", &DTMElementTextStyleECDelegate::Create },
                { L"DTMPointSymbology", &DTMElementPointSymbologyECDelegate::Create },
                { L"DTMAttachedMaterial", &DTMElementAttachedMaterialECDelegate::Create },

                { L"DTMFeature", nullptr },
                { L"DTMFeatureSpots", nullptr },

                { L"DTMFlowArrows", nullptr },
                { L"DTMHighPoints", nullptr },
                { L"DTMLowPointsBase", &DTMElementLowPointsECDelegate::Create },
                { L"DTMLowPoints", nullptr },
                { L"DTMSpots", nullptr },

                { L"DTMTrianglesBase", &DTMElementTrianglesECDelegate::Create },
                { L"DTMTriangles", nullptr },
                { L"DTMRegionsBase", &DTMElementRegionsECDelegate::Create },
                { L"DTMRegions", nullptr },
                { L"DTMContoursBase", &DTMElementContourECDelegate::Create },
                { L"DTMContours", nullptr },
                ////{L"DTMRasterDraping", nullptr},
                { L"DTMOverrideSymbology", &DTMElementOverrideSymbologyECDelegate::Create },
                { L"DTMGraphHeader", &DTMElementGraphHeaderECDelegate::Create },
                { L"DTMOSTemplate", &DTMElementOSTemplateECDelegate::Create },

                { L"STM", &DTMElementScalableTerrainModelECDelegate::Create },

                { L"DTMFeatureImportedContours", nullptr },
                { L"DTMFeatureBoundary", nullptr },
                { L"DTMFeatureBreaklines", nullptr },
                { L"DTMFeatureHoles", nullptr },
                { L"DTMFeatureIslands", nullptr },
                { L"DTMFeatureVoids", nullptr },
                { L"DTMFeatureMinorContours", nullptr },
                { L"DTMFeatureMajorContours", nullptr },
                { nullptr, nullptr }
        };

    for (ECClassEntry* entryP = DTMClasses; entryP->className; entryP++)
        {
        ECClassCP entryECClass = GetECSchemaCP()->GetClassCP (entryP->className);
        ECClassTableEntry tableEntry;
        tableEntry.ecClass = entryECClass;
        tableEntry.ecDelegate = entryP->CreateFunction ? entryP->CreateFunction (*this) : nullptr;
        m_ecTable [entryP->className] = tableEntry;
        }
    return m_ecTable;
    }

void DTMELEMENTECExtension::SetPerDelegateData (DelegatedElementECInstancePtr instance, ECClassCR ecClass, DTMElementDataCache* dataCache) const
    {
    if (&ecClass.GetSchema() == m_schema) //.get())
        {
        ElementECDelegateCP del = m_ecTable [ecClass.GetName()].ecDelegate.get();
        if (del) instance->SetPerDelegateData (*del, dataCache);
        //FOR_EACH (ECClassCP baseClass, ecClass.GetBaseClasses())
        //    SetPerDelegateData (instance,*baseClass, dataCache);
        }
    }

enum DTMElementLocalId : UInt32
    {
    LocalID_DTMElement = 0xffffffff,
    LocalID_DTMCalculatedFeatures = 0xfffffffe,
    LocalID_DTMSourceFeatures = 0xfffffffd,
    LocalID_DTMGeneralContours = 0xfffffffc
    };

DgnElementECInstancePtr DTMELEMENTECExtension::GetInstanceWithDataCache (ElementHandleCR eh,
                                                      DTMElementDataCache* dataCache, DelegatedElementECEnablerCP delEnabler, UInt32 localId, DgnECInstanceCreateContextCR context) const
    {
    if (delEnabler)
        {
        DelegatedElementECInstancePtr instance = delEnabler->CreateInstance (eh, localId, context);
        if (dataCache)
            SetPerDelegateData (instance, delEnabler->GetClass(), dataCache);

        return instance;
        }
    return nullptr;
    }

void DTMELEMENTECExtension::AddInstanceWithDataCache (DgnElementECInstanceVector& instances, ElementHandleCR eh,
                                                      DTMElementDataCache* dataCache, DelegatedElementECEnablerCP delEnabler, UInt32 localId, DgnECInstanceCreateContextCR context) const
    {
    DgnElementECInstancePtr instance = GetInstanceWithDataCache (eh, dataCache, delEnabler, localId, context);
    if (instance.IsValid())
        instances.push_back (instance);
    }

DgnECInstancePtr DTMELEMENTECExtension::_LocateInstance (ElementHandleCR eh, WStringCR instanceId, DgnECInstanceCreateContextCR context)
    {
    DgnECManagerR   manager = DgnECManager::GetManager ();
    UInt32          localId;
    PersistentElementPathP pep = nullptr;

    if (!manager.ParseECInstanceId (*pep, nullptr, &localId, instanceId.c_str ()))
        return nullptr;
    RefCountedPtr<DTMElementDataCache> dataCache = DTMElementDataCache::Create (eh);
    return GetInstance (eh, localId, context, dataCache.get());
    }

DgnECInstancePtr DTMELEMENTECExtension::GetInstance (ElementHandleCR eh, UInt32 localId, DgnECInstanceCreateContextCR context, DTMElementDataCache* dataCache) const
    {
    DgnElementECInstancePtr instance;

    bmap <WString, DelegatedElementECEnablerCP> enablerMap;
    ElementECEnablerListCR enablers = GetEnablers();
    for (ElementECEnablerList::const_iterator iter = enablers.begin(); iter != enablers.end(); ++iter)
        {
        ElementECEnablerCP enabler = (*iter);

        DelegatedElementECEnablerCP delEnabler = enabler->AsDelegatedEnabler ();
        if (nullptr == delEnabler)
            continue;

        enablerMap [delEnabler->GetClass().GetName()] = delEnabler;
        }

    switch (localId)
        {
        case LocalID_DTMElement:
            return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMElement"], LocalID_DTMElement, context);
            break;
        case LocalID_DTMGeneralContours:
            return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMGeneralContours"], LocalID_DTMGeneralContours, context);
            break;
        case LocalID_DTMCalculatedFeatures:
            return  GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMCalculatedProperties"], LocalID_DTMCalculatedFeatures, context);
            break;
        case LocalID_DTMSourceFeatures:
            return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMSourceProperties"], LocalID_DTMSourceFeatures, context);
            break;
        default:
            // Need to improve.
            for (DTMElementDataCache::ParamsMap::const_iterator it = dataCache->GetParamMap().begin(); it != dataCache->GetParamMap().end(); it++)
                {
                if (it->second.subElementId.GetId() == localId)
                    {
                    switch (it->second.subElementId.GetHandlerId())
                        {
                        case DTMElementTrianglesHandler::SubHandlerId:
                            return GetInstanceWithDataCache (eh,  dataCache, enablerMap[L"DTMTriangles"], it->second.subElementId.GetId(), context);
                        case DTMElementContoursHandler::SubHandlerId:
                            if (it->second.paramType == DPID_MAJORCONTOUR)
                                return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMFeatureMajorContours"], it->second.subElementId.GetId (), context);
                            return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMFeatureMinorContours"], it->second.subElementId.GetId (), context);
                        case DTMElementFlowArrowsHandler::SubHandlerId:
                            return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMFlowArrows"], it->second.subElementId.GetId (), context);
                        case DTMElementHighPointsHandler::SubHandlerId:
                            return GetInstanceWithDataCache (eh,  dataCache, enablerMap[L"DTMHighPoints"], it->second.subElementId.GetId(), context);
                        case DTMElementLowPointsHandler::SubHandlerId:
                            return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMLowPoints"], it->second.subElementId.GetId (), context);
                        case DTMElementSpotsHandler::SubHandlerId:
                            return GetInstanceWithDataCache (eh,  dataCache, enablerMap[L"DTMSpots"], it->second.subElementId.GetId(), context);
                        case DTMElementFeaturesHandler::SubHandlerId:
                            switch (it->second.paramType)
                                {
                                case DPID_CONTOURS:
                                    return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMFeatureImportedContours"], it->second.subElementId.GetId (), context);
                                case DPID_BOUNDARIES:
                                    return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMFeatureBoundary"], it->second.subElementId.GetId (), context);
                                case DPID_BREAKLINES:
                                    return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMFeatureBreaklines"], it->second.subElementId.GetId (), context);
                                case DPID_HOLES:
                                    return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMFeatureHoles"], it->second.subElementId.GetId (), context);
                                case DPID_ISLANDS:
                                    return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMFeatureIslands"], it->second.subElementId.GetId (), context);
                                case DPID_VOIDS:
                                    return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMFeatureVoids"], it->second.subElementId.GetId (), context);
                                default:
                                    return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMFeature"], it->second.subElementId.GetId (), context);
                                }

                        case DTMElementFeatureSpotsHandler::SubHandlerId:
                            return GetInstanceWithDataCache (eh,  dataCache, enablerMap[L"DTMFeatureSpots"], it->second.subElementId.GetId(), context);
                        case DTMElementRegionsHandler::SubHandlerId:
                            return GetInstanceWithDataCache (eh,  dataCache, enablerMap[L"DTMRegions"], it->second.subElementId.GetId(), context);
                        default:
            //                return GetInstanceWithDataCache (eh, dataCache, enablerMap[L"DTMSubElementDisplay"], it->second.subElementId.GetId(), context);
                            break;
                        }
                    }
                }
            break;
        }
    return nullptr;
    }

void DTMELEMENTECExtension::_GetPotentialInstanceChanges (DgnECInstanceChangeRecordsR changes, DgnECTxnInfoCR txnInfo, DgnECInstanceCreateContextCR context) const
    {
    bvector<DgnECInstancePtr> modified;
    RefCountedPtr<DTMElementDataCache> dataCache;
    ElementHandle eh (txnInfo.GetChangedElement ());

    if (eh.IsValid ())
        {
        ElementHandle dtmEh (txnInfo.GetChangedElement ());

        TMSymbologyOverrideManager::GetReferencedElement (eh, dtmEh);

        bool hasXAttributeChanged = false;
        for (auto const& txn : txnInfo.GetXAttrTxns ())
            {
            if (txn.GetHandlerId () != DTMElementSubHandler::GetDisplayInfoXAttributeHandlerId ())
                continue;
            if (dataCache.IsNull ())
                dataCache = DTMElementDataCache::Create (dtmEh);
            hasXAttributeChanged = true;
            AddInstance (modified, dtmEh, (UInt32)txn.GetId (), context, dataCache.get ());
            }
        if (hasXAttributeChanged)
            {
            AddInstance (modified, dtmEh, LocalID_DTMElement, context, dataCache.get ());
            AddInstance (modified, dtmEh, LocalID_DTMGeneralContours, context, dataCache.get ());
            AddInstance (modified, dtmEh, LocalID_DTMCalculatedFeatures, context, dataCache.get ());
            AddInstance (modified, dtmEh, LocalID_DTMSourceFeatures, context, dataCache.get ());
            }

        changes.AppendModifiedInstances (DgnECInstanceIterable::CreateFromVector (modified));
        }
    __super::_GetPotentialInstanceChanges (changes, txnInfo, context);
    }

void DTMELEMENTECExtension::_GenerateInstances (DgnElementECInstanceVector& instances, ElementHandleCR eh, ElementECEnablerListCR enablers, DgnECInstanceCreateContextCR context) const
    {
    RefCountedPtr<DTMElementDataCache> dataCache = DTMElementDataCache::Create (eh);

    bmap <WString, DelegatedElementECEnablerCP> enablerMap;

    for (ElementECEnablerList::const_iterator iter = enablers.begin(); iter != enablers.end(); ++iter)
        {
        ElementECEnablerCP enabler = (*iter);

        DelegatedElementECEnablerCP delEnabler = enabler->AsDelegatedEnabler ();
        if (nullptr == delEnabler)
            continue;
        enablerMap [delEnabler->GetClass().GetName()] = delEnabler;
        }


    if (eh.GetElementType () != EXTENDED_NONGRAPHIC_ELM)
        AddInstanceWithDataCache (instances, eh, dataCache.get (), enablerMap[L"DTMElement"], LocalID_DTMElement, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct TerrainModelChildrenFinder : public IDgnECInstanceFinder
    {
private:
    const DTMELEMENTECExtension* m_extension;
    const bool m_backwards;
    TerrainModelChildrenFinder (const DTMELEMENTECExtension* extension, IECPropertyValueFilterP filter, DgnECInstanceCreateContextCR createContext, bool backwards) : m_extension (extension), IDgnECInstanceFinder(filter, createContext), m_backwards(backwards) { }

    void    GetAgenda(ElementAgendaR agenda, ElementHandleCR element) const;

    virtual DgnECHostType           _GetHostType () const override {return DgnECHostType::Element;}
    virtual DgnECInstanceIterable   _GetRelatedInstances (DgnECInstanceCR instance, QueryRelatedClassSpecifierCR relationshipClassSpecifier) const override;

public:
    static TerrainModelChildrenFinder* CreateFinder (const DTMELEMENTECExtension* extension, DgnECInstanceCreateContextCR createContext, bool backwards)
        {
        return new TerrainModelChildrenFinder (extension, nullptr, createContext, backwards);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            TerrainModelChildrenFinder::GetAgenda(ElementAgendaR agenda, ElementHandleCR elemHandle) const
    {
    // ToDo?
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECInstanceIterable   TerrainModelChildrenFinder::_GetRelatedInstances (DgnECInstanceCR source, QueryRelatedClassSpecifierCR relationshipClassSpecifier) const
    {
    DgnElementECInstanceVector instances;

    DelegatedElementECInstance const* instance = dynamic_cast<DelegatedElementECInstance const*> (&source);

    if (nullptr == instance)
        return nullptr;

    ElementHandleCR eh = instance->GetElementHandle();
    RefCountedPtr<DTMElementDataCache> dataCachePtr;
    DTMElementDataCache* dataCache = nullptr;

    if (!dataCache)
        {
        dataCachePtr = DTMElementDataCache::Create (eh);
        dataCache = dataCachePtr.get();
        }

    bmap <WString, DelegatedElementECEnablerCP> enablerMap;
    ElementECEnablerListCR enablers = m_extension->GetEnablers();
    for (ElementECEnablerList::const_iterator iter = enablers.begin(); iter != enablers.end(); ++iter)
        {
        ElementECEnablerCP enabler = (*iter);

        DelegatedElementECEnablerCP delEnabler = enabler->AsDelegatedEnabler ();
        if (nullptr == delEnabler)
            continue;

        if (delEnabler->GetClass().Is (relationshipClassSpecifier.GetRelatedClass ()))
            enablerMap [delEnabler->GetClass().GetName()] = delEnabler;
        }

    UInt32 localId = instance->GetLocalId ();
    if (!m_backwards)
        {
        if (!(dataCache->IsSTM() && (DTMElementHandlerManager::GetMrDTMActivationRefCount() == 0)) && localId == LocalID_DTMElement)
            {
            // The following two lines create two sub-nodes for the TM element in the Properties dialog ("Calculated Features Display"
            // and "Source Features Display"). This is not displayed for a MrDTM if Descartes is not there.
            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMCalculatedProperties"], LocalID_DTMCalculatedFeatures, m_createContext);
            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMSourceProperties"], LocalID_DTMSourceFeatures, m_createContext);
            }

        if (localId == LocalID_DTMCalculatedFeatures)
            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMGeneralContours"], LocalID_DTMGeneralContours, m_createContext);

        if (localId != LocalID_DTMElement)
            {
            for (DTMElementDataCache::ParamsMap::const_iterator it = dataCache->GetParamMap ().begin (); it != dataCache->GetParamMap ().end (); it++)
                {
                switch (it->second.subElementId.GetHandlerId ())
                    {
                    case DTMElementTrianglesHandler::SubHandlerId:
                        if (localId == LocalID_DTMCalculatedFeatures)
                            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMTriangles"], it->second.subElementId.GetId (), m_createContext);
                        break;
                    case DTMElementContoursHandler::SubHandlerId:
                        if (localId == LocalID_DTMGeneralContours)
                            {
                            if (it->second.paramType == DPID_MAJORCONTOUR)
                                m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFeatureMajorContours"], it->second.subElementId.GetId (), m_createContext);
                            else
                                m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFeatureMinorContours"], it->second.subElementId.GetId (), m_createContext);
                            }
                        break;
                    case DTMElementFlowArrowsHandler::SubHandlerId:
                        if (localId == LocalID_DTMCalculatedFeatures)
                            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFlowArrows"], it->second.subElementId.GetId (), m_createContext);
                        break;
                    case DTMElementHighPointsHandler::SubHandlerId:
                        if (localId == LocalID_DTMCalculatedFeatures)
                            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMHighPoints"], it->second.subElementId.GetId (), m_createContext);
                        break;
                    case DTMElementLowPointsHandler::SubHandlerId:
                        if (localId == LocalID_DTMCalculatedFeatures)
                            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMLowPoints"], it->second.subElementId.GetId (), m_createContext);
                        break;
                    case DTMElementSpotsHandler::SubHandlerId:
                        if (localId == LocalID_DTMCalculatedFeatures)
                            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMSpots"], it->second.subElementId.GetId (), m_createContext);
                        break;
                    case DTMElementFeaturesHandler::SubHandlerId:
                        if (localId == LocalID_DTMSourceFeatures)
                            {
                            switch (it->second.paramType)
                                {
                                case DPID_CONTOURS:
                                    m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFeatureImportedContours"], it->second.subElementId.GetId (), m_createContext);
                                    break;
                                case DPID_BOUNDARIES:
                                    m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFeatureBoundary"], it->second.subElementId.GetId (), m_createContext);
                                    break;
                                case DPID_BREAKLINES:
                                    m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFeatureBreaklines"], it->second.subElementId.GetId (), m_createContext);
                                    break;
                                case DPID_HOLES:
                                    m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFeatureHoles"], it->second.subElementId.GetId (), m_createContext);
                                    break;
                                case DPID_ISLANDS:
                                    m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFeatureIslands"], it->second.subElementId.GetId (), m_createContext);
                                    break;
                                case DPID_VOIDS:
                                    m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFeatureVoids"], it->second.subElementId.GetId (), m_createContext);
                                    break;
                                default:
                                    m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFeature"], it->second.subElementId.GetId (), m_createContext);
                                    break;
                                }
                            }
                        break;
                    case DTMElementFeatureSpotsHandler::SubHandlerId:
                        if (localId == LocalID_DTMSourceFeatures)
                            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMFeatureSpots"], it->second.subElementId.GetId (), m_createContext);
                        break;
                    case DTMElementRegionsHandler::SubHandlerId:
                        if (localId == LocalID_DTMCalculatedFeatures)
                            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMRegions"], it->second.subElementId.GetId (), m_createContext);
                        break;
                    default:
                        //                m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMSubElementDisplay"], it->second.subElementId.GetId(), m_createContext);
                        break;
                    }
                }
            }
        }
    else
        {
        if (localId == LocalID_DTMCalculatedFeatures || localId == LocalID_DTMSourceFeatures)
            {
            // LocalID_DTMElement
            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMElement"], LocalID_DTMElement, m_createContext);
            }
        else if (localId == LocalID_DTMGeneralContours)
            {
            m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMCalculatedProperties"], LocalID_DTMCalculatedFeatures, m_createContext);
            }
        else if (localId != LocalID_DTMElement)
            {
            Int16 handlerId = dataCache->GetSubHandlerId (localId);
            switch (handlerId)
                {
                case DTMElementContoursHandler::SubHandlerId:
                    m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMGeneralContours"], LocalID_DTMGeneralContours, m_createContext);
                    break;
                case DTMElementTrianglesHandler::SubHandlerId:
                case DTMElementFlowArrowsHandler::SubHandlerId:
                case DTMElementHighPointsHandler::SubHandlerId:
                case DTMElementLowPointsHandler::SubHandlerId:
                case DTMElementSpotsHandler::SubHandlerId:
                case DTMElementRegionsHandler::SubHandlerId:
                    m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMCalculatedProperties"], LocalID_DTMCalculatedFeatures, m_createContext);
                    break;
                case DTMElementFeaturesHandler::SubHandlerId:
                case DTMElementFeatureSpotsHandler::SubHandlerId:
                    m_extension->AddInstanceWithDataCache (instances, eh, dataCache, enablerMap[L"DTMSourceProperties"], LocalID_DTMSourceFeatures, m_createContext);
                    break;
                }
            }
        }
    return DgnECInstanceIterable::CreateFromVector (instances);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DTMELEMENTECExtension::_GetSupportedRelationshipInfos (DgnECInstanceCR instance, DgnECRelationshipInfoVector& infos) const
    {
    DeduceSupportedRelationship (infos, REL_TerrainModelChildren, instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECInstanceFinderPtr DTMELEMENTECExtension::_CreateRelatedFinder (DgnECInstanceCR source, QueryRelatedClassSpecifierCR classSpec, DgnECInstanceCreateContextCR createContext) const
    {
    if (classSpec.Accept (*GetRelationshipClass (REL_TerrainModelChildren)))
        {
        if (STRENGTHDIRECTION_Forward == classSpec.GetDirection())
            return TerrainModelChildrenFinder::CreateFinder (this, createContext, false);
        else
            return TerrainModelChildrenFinder::CreateFinder (this, createContext, true);
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**
 * @bsistruct                                                    Paul.Connelly   05/13
 +---------------+---------------+---------------+---------------+---------------+------*/
struct HasTMByCellString
    {
    protected:
        WString         m_byCellString;

        HasTMByCellString ()
            {
            m_byCellString = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_FromParent);
            }
    };

//---------------------------------------------------------------------------------***
// @bsimethod                                                    Paul.Connelly   03/12
// +---------------+---------------+---------------+---------------+---------------+------
static LevelCacheCP getLevelCacheFromContext (IDgnECTypeAdapterContextCR context)
    {
    DgnModelRefP modelRef = context.GetModelRef ();
    if (NULL != modelRef)
        return &modelRef->GetLevelCache ();      // != GetDgnFile()->GetLevelCacheR() if modelRef is DgnAttachment

    DgnFileP dgnFile = context.GetDgnFile ();
    if (NULL != dgnFile)
        return &dgnFile->GetLevelCacheR ();

    return NULL;
    }
//*---------------------------------------------------------------------------------**
// * @bsistruct                                                    Paul.Connelly   03/12
// +---------------+---------------+---------------+---------------+---------------+------*/
struct ElementSymbologyQuery : IQueryProperties
    {
    private:
        Symbology& m_symbology;
        bool m_obtainedColor;
        bool m_obtainedStyle;
        bool m_obtainedWeight;

        ElementSymbologyQuery (Symbology& outSymbology) : m_symbology (outSymbology), m_obtainedColor (false), m_obtainedStyle (false), m_obtainedWeight (false)
            {
            }

        virtual ElementProperties   _GetQueryPropertiesMask () override
            {
            return (ElementProperties)(ELEMENT_PROPERTY_Color | ELEMENT_PROPERTY_Linestyle | ELEMENT_PROPERTY_Weight);
            }

        virtual void _EachColorCallback (EachColorArg& arg) override
            {
            if (!m_obtainedColor)
                {
                m_symbology.color = arg.GetEffectiveValue ();
                m_obtainedColor = true;
                }
            }
        virtual void _EachLineStyleCallback (EachLineStyleArg& arg) override
            {
            if (!m_obtainedStyle)
                {
                m_symbology.style = arg.GetEffectiveValue ();
                m_obtainedStyle = true;
                }
            }
        virtual void _EachWeightCallback (EachWeightArg& arg) override
            {
            if (!m_obtainedWeight)
                {
                m_symbology.weight = arg.GetEffectiveValue ();
                m_obtainedWeight = true;
                }
            }
    public:
        static bool GetElementSymbology (Symbology& outSymbology, IDgnECTypeAdapterContextCR context)
            {
            ElementRefP elemRef = context.GetElementRef ();
            DgnModelRefP modelRef = context.GetModelRef ();
            if (NULL == elemRef || NULL == modelRef)
                return false;

            ElementSymbologyQuery query (outSymbology);
            ElementHandle eh (elemRef, modelRef);
            PropertyContext::QueryElementProperties (eh, &query);
            return query.m_obtainedColor && query.m_obtainedStyle && query.m_obtainedWeight;
            }
    };

struct ElementLevelQuery : IQueryProperties
    {
    private:
        LevelId&            m_levelId;
        bool                m_obtained;

        ElementLevelQuery (LevelId& outId) : m_levelId (outId), m_obtained (false)
            {
            }

        virtual ElementProperties   _GetQueryPropertiesMask () override
            {
            return ELEMENT_PROPERTY_Level;
            }
        virtual void                _EachLevelCallback (EachLevelArg& arg) override
            {
            if (!m_obtained)
                {
                m_levelId = arg.GetEffectiveValue ();
                m_obtained = true;
                }
            }
    public:
        static bool GetElementLevel (LevelId& outId, IDgnECTypeAdapterContextCR context)
            {
            ElementRefP elemRef = context.GetElementRef ();
            DgnModelRefP modelRef = context.GetModelRef ();
            if (NULL == elemRef || NULL == modelRef)
                return false;

            ElementLevelQuery query (outId);
            ElementHandle eh (elemRef, modelRef);
            PropertyContext::QueryElementProperties (eh, &query);
            return query.m_obtained;
            }
    };
//---------------------------------------------------------------------------------**
// **
// * @bsistruct                                                    Paul.Connelly   03/12
// +---------------+---------------+---------------+---------------+---------------+------
bool getLevelDefFromContext (LevelHandle& level, IDgnECTypeAdapterContextCR context, bool fromDisplayParam = true)
    {
    LevelId levelId;

    if (fromDisplayParam)
        {
        auto dgnInst = dynamic_cast<DgnElementECInstanceCP>(context.GetInstanceInterface ().ObtainECInstance ()); // NEEDSWORK: avoid obtaining ECInstance?
        if (nullptr == dgnInst)
            return false;

        UInt32 id = dgnInst->GetLocalId ();


        ElementHandle element (dgnInst->GetElementRef (), &dgnInst->GetModelRef ());
        ElementHandle symbologyElement = element;
        TMSymbologyOverrideManager::GetElementForSymbology (element, symbologyElement, element.GetModelRef ()->GetRoot ());

        ElementHandle::XAttributeIter xattr (symbologyElement, DTMElementSubHandler::GetDisplayInfoXAttributeHandlerId (), id);

        BeAssert (xattr.IsValid ());
        if (xattr.IsValid ())
            {
            RefCountedPtr<DTMElementSubHandler::SymbologyParams> params = DTMElementSubHandler::GetParams (xattr);
            levelId = params->GetLevelId ();
            if (levelId != LEVEL_BYCELL)
                {
                LevelCacheCP cache = getLevelCacheFromContext (context);
                if (cache)
                    level = cache->GetLevel (levelId);
                return level.IsValid ();
                }
            }
        }

    if (ElementLevelQuery::GetElementLevel (levelId, context))
        {
        LevelCacheCP cache = getLevelCacheFromContext (context);
        if (cache)
            level = cache->GetLevel (levelId);
        }
    return level.IsValid ();
    }

//*---------------------------------------------------------------------------------****
//* @bsistruct                                                    Paul.Connelly   05/13
//+---------------+---------------+---------------+---------------+---------------+------*/
struct TMColorTypeAdapter : ColorTypeAdapter, HasTMByCellString
    {
    private:
        TMColorTypeAdapter ()
            {
            }

        virtual WCharCP _GetByCellString () const override
            {
            return m_byCellString.c_str ();
            }
        virtual bool GetUnformattedStringValue (WStringR strVal, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override
            {
            if (!v.IsInteger ())
                return false;

            UInt32 color = (UInt32)v.GetInteger ();
            if (!ColorToString (strVal, color, context.GetDgnFile (), context))
                return false;
            else if (color == COLOR_BYLEVEL)
                {
                // ByLevel - append actual color from level if possible
                LevelHandle level;
                if (getLevelDefFromContext (level, context))
                    {
                    UInt32 levelColor = level.GetByLevelColor ().GetColor ();
                    WString levelColorStr;
                    if (ColorToString (levelColorStr, levelColor, level.GetByLevelColorDefinitionFile (), context))
                        {
                        strVal.append (L" (");
                        strVal.append (levelColorStr);
                        strVal.append (L")");
                        return true;
                        }
                    }
                }
            else if (color == COLOR_BYCELL)
                {
                Symbology symb;

                if (ElementSymbologyQuery::GetElementSymbology (symb, context))
                    {
                    if (symb.color == COLOR_BYLEVEL)
                        {
                        LevelHandle level;
                        if (getLevelDefFromContext (level, context, false))
                            symb.color = level.GetByLevelColor ().GetColor ();
                        else
                            return true;
                        }
                    WString levelColorStr;
                    if (ColorToString (levelColorStr, symb.color, context.GetDgnFile (), context))
                        {
                        strVal.append (L" (");
                        strVal.append (levelColorStr);
                        strVal.append (L")");
                        return true;
                        }
                    }
                }

            // couldn't get level color, so just leave it off
            return true;
            }
    public:
        static IDgnECTypeAdapterPtr Create ()
            {
            return new TMColorTypeAdapter ();
            }
    };
static const Int32  s_maxWeight = 0x20;
static WCharCP  s_standardWeightStrings[s_maxWeight] =
    { L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L"10", L"11", L"12", L"13", L"14", L"15", L"16", L"17", L"18", L"19", L"20", L"21", L"22", L"23", L"24", L"25", L"26", L"27", L"28", L"29", L"30", L"31" };

//*---------------------------------------------------------------------------------****
//* @bsistruct                                                    Paul.Connelly   05/13
//+---------------+---------------+---------------+---------------+---------------+------*/
struct TMWeightTypeAdapter : WeightTypeAdapter, HasTMByCellString
    {
    private:
        TMWeightTypeAdapter ()
            {
            }

        virtual WCharCP _GetByCellString () const override
            {
            return m_byCellString.c_str ();
            }
        //---------------------------------------------------------------------------------****
        // * @bsimethod                                                    Paul.Connelly   03/12
        // +---------------+---------------+---------------+---------------+---------------+------
        virtual bool GetUnformattedStringValue (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override
            {
            if (PRIMITIVETYPE_Integer != v.GetPrimitiveType ())
                return false;

            Int32 weight = v.GetInteger ();
            switch ((UInt32)weight)
                {
                case INVALID_WEIGHT:
                    valueAsString = GetNoneString (context);
                    return true;
                case WEIGHT_BYLEVEL:
                    if (!_AllowByLevel ())
                        return false;

                    valueAsString = GetByLevelString ();
                    break;
                case WEIGHT_BYCELL:
                    valueAsString = GetByCellString (context);
                    break;
                default:
                    if (0 > weight || s_maxWeight <= weight)
                        return false;

                    valueAsString = s_standardWeightStrings[weight];
                    return true;
                }

            if (weight == WEIGHT_BYLEVEL)
                {
                // Weight is ByLevel; append weight value from level if possible
                LevelHandle level;
                if (getLevelDefFromContext (level, context))
                    {
                    UInt32 byLevelWeight = level.GetByLevelWeight ();
                    // Avoid Snwprintf() if possible, it is quite slow
                    if (byLevelWeight < s_maxWeight)
                        {
                        valueAsString.reserve (valueAsString.length () + 5);
                        valueAsString.append (L" (");
                        valueAsString.append (s_standardWeightStrings[byLevelWeight]);
                        valueAsString.append (L")");
                        }
                    else
                        {
                        WChar buf[0x10];
                        BeStringUtilities::Snwprintf (buf, _countof (buf), L" (%u)", byLevelWeight);
                        valueAsString.append (buf);
                        }
                    }
                }
            else
                {
                Symbology symb;

                if (ElementSymbologyQuery::GetElementSymbology (symb, context))
                    {
                    if (symb.weight == WEIGHT_BYLEVEL)
                        {
                        LevelHandle level;
                        if (getLevelDefFromContext (level, context))
                            symb.weight = level.GetByLevelWeight ();
                        else
                            return true;
                        }
                    // Avoid Snwprintf() if possible, it is quite slow
                    if (symb.weight < s_maxWeight)
                        {
                        valueAsString.reserve (valueAsString.length () + 5);
                        valueAsString.append (L" (");
                        valueAsString.append (s_standardWeightStrings[symb.weight]);
                        valueAsString.append (L")");
                        }
                    else
                        {
                        WChar buf[0x10];
                        BeStringUtilities::Snwprintf (buf, _countof (buf), L" (%u)", symb.weight);
                        valueAsString.append (buf);
                        }
                    }
                }
            return true;
            }
    public:
        static IDgnECTypeAdapterPtr Create ()
            {
            return new TMWeightTypeAdapter ();
            }
    };

//*---------------------------------------------------------------------------------****
//* @bsistruct                                                    Paul.Connelly   05/13
//+---------------+---------------+---------------+---------------+---------------+------*/
struct TMLineStyleTypeAdapter : LineStyleTypeAdapter, HasTMByCellString
    {
    private:
        TMLineStyleTypeAdapter ()
            {
            }

        virtual WCharCP _GetByCellString () const override
            {
            return m_byCellString.c_str ();
            }

        virtual bool GetUnformattedStringValue (WStringR valueAsString, ECN::ECValueCR inputVal, IDgnECTypeAdapterContextCR context) const override
            {
            DgnFileP dgnFile = context.GetDgnFile ();
            if (NULL == dgnFile)
                return false;

            if (PRIMITIVETYPE_String == inputVal.GetPrimitiveType ())
                {
                valueAsString = inputVal.GetString ();

                if (valueAsString.Equals (L"2147483646"))
                    valueAsString = m_byCellString;
                else if (valueAsString.Equals (L"2147483647"))
                    valueAsString = GetByLevelString ();
                return true;
                }
            if (PRIMITIVETYPE_Integer == inputVal.GetPrimitiveType ())
                {
                if (inputVal.GetInteger () == STYLE_BYLEVEL)
                    {
                    valueAsString = GetByLevelString ();
                    LevelHandle level;
                    if (getLevelDefFromContext (level, context))
                        {
                        UInt32 levelStyle = level.GetByLevelLineStyle ().GetStyle ();
                        WString stringFromCode = LineStyleManager::GetStringFromNumber (levelStyle, dgnFile);
                        valueAsString.reserve (3 + stringFromCode.length ());
                        valueAsString.append (L" (");
                        valueAsString.append (stringFromCode);
                        valueAsString.append (L")");
                        }
                    }
                else if (inputVal.GetInteger () == STYLE_BYCELL)
                    {
                    valueAsString = GetByCellString (context);
                    Symbology symb;

                    if (ElementSymbologyQuery::GetElementSymbology (symb, context))
                        {
                        if (symb.style == STYLE_BYLEVEL)
                            {
                            LevelHandle level;
                            if (getLevelDefFromContext (level, context))
                                symb.style = level.GetByLevelLineStyle ().GetStyle ();
                            else
                                return true;
                            }
                        WString stringFromCode = LineStyleManager::GetStringFromNumber (symb.style, dgnFile);
                        valueAsString.reserve (3 + stringFromCode.length ());
                        valueAsString.append (L" (");
                        valueAsString.append (stringFromCode);
                        valueAsString.append (L")");
                        }
                    }
                else if (inputVal.GetInteger () == INVALID_STYLE)
                    valueAsString = GetNoneString (context);
                else
                    valueAsString = LineStyleManager::GetStringFromNumber (inputVal.GetInteger (), dgnFile);
                return !valueAsString.empty ();
                }
            return false;
            }
    public:
        static IDgnECTypeAdapterPtr Create ()
            {
            return new TMLineStyleTypeAdapter ();
            }
    };

//*---------------------------------------------------------------------------------****
//* @bsistruct                                                    Paul.Connelly   10/12
//+---------------+---------------+---------------+---------------+---------------+------
struct TMElementTemplateTypeAdapter : TemplateTypeAdapterBase
    {
    private:
        TMElementTemplateTypeAdapter ()
            {
            }

        DgnModelRefP GetModelRef (IDgnECTypeAdapterContextCR context) const
            {
            ElementHandle el (context.GetElementRef (), context.GetModelRef ());
            DgnPlatform::DgnModelRef* rootModel = el.GetModelRef ()->GetRoot ();
            DgnPlatform::ElementHandle symbologyElement;
            if (TMSymbologyOverrideManager::GetElementForSymbology (el, symbologyElement, rootModel))
                {
                if (el.GetElementRef () != symbologyElement.GetElementRef () && rootModel == symbologyElement.GetModelRef ())
                    {
                    return symbologyElement.GetModelRef ();
                    }
                }
            return context.GetModelRef ();
            }
        DgnFileP GetDgnFile (IDgnECTypeAdapterContextCR context) const
            {
            return GetModelRef (context)->GetDgnFileP ();
            }

        virtual bool                GetPathForTemplate (WStringP path, ElementId templateId, IDgnECTypeAdapterContextCR context) const override
            {
            return ETSTATUS_Success == ElementTemplateUtils::GetPathForTemplate (path, templateId, GetModelRef (context));
            }
        virtual bool                GetTemplateForPath (ElementId& templateId, WCharCP path, IDgnECTypeAdapterContextCR context) const override
            {
            return ETSTATUS_Success == ElementTemplateUtils::GetTemplateIDFromPath (&templateId, path, GetDgnFile (context));
            }
    public:
        static IDgnECTypeAdapterPtr Create ()
            {
            return new TMElementTemplateTypeAdapter ();
            }
    };

struct TMElementEdgeOptionTypeAdapter : IDgnECTypeAdapter
    {
    bvector<WString> m_values;

    TMElementEdgeOptionTypeAdapter ()
        {
        m_values.push_back (TerrainModelElementResources::GetString (MSG_TERRAINMODEL_FromBoundary));
        m_values.push_back (TerrainModelElementResources::GetString (MSG_TERRAINMODEL_None));
        m_values.push_back (TerrainModelElementResources::GetString (MSG_TERRAINMODEL_Sliver));
        m_values.push_back (TerrainModelElementResources::GetString (MSG_TERRAINMODEL_MaxEdgeLength));
        }
    virtual bool            _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const
        {
        for (int i = 1; i < (int)m_values.size(); i++)
            values.push_back (m_values[i]);
        return true;
        }

    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override
        {
        if (!v.IsNull ())
            {
            UInt32 option = v.GetInteger ();

            if (option >= 0 && option < (int)m_values.size ())
                valueAsString = m_values[option];
            }

        return !valueAsString.empty ();
        }
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override
        {
        for (int i = 0; i < (int)m_values.size (); i++)
            {
            if (m_values[i].Equals (stringValue))
                {
                v.SetInteger (i);
                return true;
                }
            }
        return false;
        }
    virtual bool            _CanConvertFromString (IDgnECTypeAdapterContextCR) const override
        {
        return true;
        }
    virtual bool            _HasStandardValues () const override
        {
        return true;
        }
    virtual bool            _IsStruct () const
        {
        return false;
        }
    virtual bool            _GetPlaceholderValue (ECN::ECValueR v, IDgnECTypeAdapterContextCR) const override { return false; }
    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override
        {
        if (!v.IsNull ())
            {
            return true;
            }

        return false;
        }

    public:
        static IDgnECTypeAdapterPtr   Create ()
            {
            return new TMElementEdgeOptionTypeAdapter ();
            }
    };
//*---------------------------------------------------------------------------------****
//@bsimethod                                    Eric.Paquet                     05/2012
//---------------+---------------+---------------+---------------+---------------+------*/
struct MrDTMViewFlagsTypeAdapter : IDgnECTypeAdapter
    {
    private:
        MrDTMViewFlagsTypeAdapter ()
            {
            }
        virtual ~MrDTMViewFlagsTypeAdapter ()
            {
            }

        virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
        virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
        virtual bool            _GetPlaceholderValue (ECN::ECValueR v, IDgnECTypeAdapterContextCR) const override { return false; }
        virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
        virtual bool            _CanConvertFromString (IDgnECTypeAdapterContextCR) const override
            {
            return false;
            }
        virtual bool            _HasStandardValues () const override
            {
            return false;
            }
        virtual bool            _IsStruct () const
            {
            return false;
            }

    public:
        static IDgnECTypeAdapterPtr   Create ();
    };

//*---------------------------------------------------------------------------------****
//* @bsimethod                                    Eric.Paquet                     05/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr MrDTMViewFlagsTypeAdapter::Create ()
    {
    return new MrDTMViewFlagsTypeAdapter ();
    }

//*---------------------------------------------------------------------------------****
//* @bsimethod                                    Eric.Paquet                     05/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
bool MrDTMViewFlagsTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    if (!v.IsNull ())
        {
        return true;
        }

    return false;
    }

//*---------------------------------------------------------------------------------****
//* @bsimethod                                    Eric.Paquet                     05/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
bool MrDTMViewFlagsTypeAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR inputVal, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    if (!inputVal.IsNull () && inputVal.IsArray ())
        {
        ECN::IECInstanceInterfaceCR intfc = context.GetInstanceInterface ();
        UInt32 count = inputVal.GetArrayInfo ().GetCount ();
        WString accessor;
        for (UInt32 view = 0; view < count; view++)
            {
            ECValue v;
            accessor.Sprintf (L"Views[%d]", view);
            intfc.GetInstanceValue (v, accessor.c_str ());
            bool viewIsOn = !v.IsNull () && v.GetBoolean ();
            if (viewIsOn)
                {
                WChar     formatString[128];
                BeStringUtilities::Snwprintf (formatString, _countof (formatString), L"%d", view + 1);
                valueAsString += WString (formatString);
                }
            else
                {
                valueAsString += WString (L"  ");
                }
            if (view < 7)
                valueAsString += WString (L"-");
            }
        }

    return !valueAsString.empty ();
    }

//*---------------------------------------------------------------------------------****
//* @bsimethod                                    Eric.Paquet                     05/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
bool MrDTMViewFlagsTypeAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context) const
    {
    // Nothing needs to be done here, since the field is read-only (it is not possible to directly enter view values in the field;
    // one has to use the TypeEditor).
    return false;
    }



DTMELEMENTECExtension* extension = nullptr;
//*--------------------------------------------------------------------------------------
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void    registerECExtension ()
    {
    extension = DTMELEMENTECExtension::Create();
    ElementECExtension::RegisterExtension (DTMElementHandler::GetInstance (), *extension);
    extension = DTMELEMENTECExtension::Create ();
    ElementECExtension::RegisterExtension (TMOverrideSymbologyManager::GetInstance (), *extension);


    DgnECExtendedType::GetByName (L"TMElementTemplate").SetTypeAdapter (*TMElementTemplateTypeAdapter::Create ());
    DgnECExtendedType::GetByName (L"TMColor").SetTypeAdapter (*TMColorTypeAdapter::Create ());
    DgnECExtendedType::GetByName (L"TMStyle").SetTypeAdapter (*TMLineStyleTypeAdapter::Create ());
    DgnECExtendedType::GetByName (L"TMWeight").SetTypeAdapter (*TMWeightTypeAdapter::Create ());
    DgnECExtendedType::GetByName (L"TMEdgeOption").SetTypeAdapter (*TMElementEdgeOptionTypeAdapter::Create ());

    // View Flags Type Adapter
    DgnECExtendedType::GetByName (L"MrDTMViewsFlags").SetTypeAdapter (*MrDTMViewFlagsTypeAdapter::Create ());
    }

DTMELEMENT_EXPORT ECSchemaCP GetPresentationSchema ()
    {
    if (extension)
        return extension->GetECSchemaCP();
    return nullptr;
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
