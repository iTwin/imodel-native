/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handlerNET/DTMElement.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"

#include "DTMElement.h"

#define CREATE_IF_NOT_FOUND  (true)


// the purpose of this class is to use as an automatic to ensure that the element's finalization requirements are considered. 
struct FinalizeAdjuster
    {
    msclr::gcroot<DGNET::Elements::Element^>    m_elementToAdjust;
    FinalizeAdjuster (DGNET::Elements::Element^ element) : m_elementToAdjust (element)
        {
        }
    ~FinalizeAdjuster ()
        {
        m_elementToAdjust->AdjustFinalizeRequirement();
        }
    };

// When you want to use the ElementHandle member of a managed element as a native EditElementHandleP in a member method, do this:
// PIN_ELEMENT_HANDLE
// then use "thisEeh" as the EditElementHandleP.
#define PIN_ELEMENTHANDLE pin_ptr<byte>arrayPtr = &ElementHandle[0]; EditElementHandleP thisEeh = reinterpret_cast <EditElementHandleP> (arrayPtr);

// When you want to use the ElementHandle member of an arbitray managed element as a native EditElementHandleP, do this:
// PIN_ELEMENT_HANDLE_OF (element)
// then use "elementEeh" as the EditElementHandleP.
#define PIN_ELEMENTHANDLE_OF(NAME) \
    pin_ptr<byte> NAME##ArrayPtr=nullptr; \
    EditElementHandleP NAME##Eeh = NULL; \
    if (nullptr != NAME) \
        { \
        NAME##ArrayPtr = &NAME->ElementHandle[0]; \
        NAME##Eeh      = reinterpret_cast <EditElementHandleP> ( NAME##ArrayPtr); \
        }

// When you want are going to change the element of an ElementHandle, so it will become finalizable, use PIN_ELEMENTHANDLE_ADJUSTFINALIZE
// then use "thisEeh" as the EditElementHandleP.
#define PIN_ELEMENTHANDLE_ADJUSTFINALIZE \
    pin_ptr<byte>arrayPtr = &ElementHandle[0]; EditElementHandleP thisEeh = reinterpret_cast <EditElementHandleP> (arrayPtr); \
    FinalizeAdjuster finalizeAjuster (this);

// When you want to use the ElementHandle member of an arbitray managed element as a native EditElementHandleP, do this:
// PIN_ELEMENT_HANDLE_OF (element)
// then use "elementEeh" as the EditElementHandleP.
#define PIN_ELEMENTHANDLE_ADJUSTFINALIZE_OF(NAME) \
    pin_ptr<byte> NAME##ArrayPtr=nullptr; \
    EditElementHandleP NAME##Eeh = NULL; \
    if (nullptr != NAME) \
        { \
        NAME##ArrayPtr = &NAME->ElementHandle[0]; \
        NAME##Eeh      = reinterpret_cast <EditElementHandleP> ( NAME##ArrayPtr); \
        } \
    FinalizeAdjuster finalizeAjuster (NAME);

#define GETDISPLAYPARAM(paramType) paramType& params = *dynamic_cast<paramType*>(m_params);

BEGIN_BENTLEY_NAMESPACE

namespace DgnPlatformNET { namespace Elements {

    struct ManagedElementFactoryExtension : DgnPlatform::Handler::Extension
    {
    ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS (ManagedElementFactoryExtension,)
    virtual Bentley::DgnPlatformNET::Elements::Element^ _CreateManagedElement(ElementHandleCR) = 0;
    };

struct ManagedElementFactory : ManagedElementFactoryExtension
    {
    msclr::gcroot<DGNET::Elements::ElementFactoryDelegate^>     m_factory;

    ManagedElementFactory (DGNET::Elements::ElementFactoryDelegate^factory) : m_factory (factory) {}
    virtual ~ManagedElementFactory() {}

    virtual Bentley::DgnPlatformNET::Elements::Element^ _CreateManagedElement(ElementHandleCR eh)
        {
        return m_factory->Invoke (eh);
        }

    static  Bentley::DgnPlatformNET::Elements::Element^ CreateManagedElement(ElementHandleCR eh)
        {
        using ::System::Runtime::InteropServices::GCHandle;

        ManagedElementFactoryExtension* factoryExt = ManagedElementFactoryExtension::Cast (eh.GetHandler());
        if (NULL == factoryExt)
            return nullptr;  //  Since we register Element, this should be impossible.

        Bentley::DgnPlatformNET::Elements::Element^ retval = factoryExt->_CreateManagedElement(eh);

        return retval;
        }
    };

}}
END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_TERRAINMODELNET_ELEMENT_NAMESPACE

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMElement::~DTMElement (void)
    {
    DoDispose();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMElement::!DTMElement (void)
    {
    DoDispose();
    BeAssert (true);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 03/11
//=======================================================================================
void DTMElement::DoDispose ()
    {
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/13
//=======================================================================================
bool DTMElement::CanHaveSymbologyOverride::get()
    {
    PIN_ELEMENTHANDLE

    return TMSymbologyOverrideManager::CanHaveSymbologyOverride (*thisEeh);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/13
//=======================================================================================
bool DTMElement::HasSymbologyOverride::get()
    {
    PIN_ELEMENTHANDLE
    DgnPlatform::DgnModelRef* rootModel = thisEeh->GetModelRef()->GetRoot();
    DgnPlatform::ElementHandle symbologyElement;
    if (TMSymbologyOverrideManager::GetElementForSymbology (*thisEeh, symbologyElement, rootModel))
        return thisEeh->GetElementRef () != symbologyElement.GetElementRef () && rootModel == symbologyElement.GetModelRef();
    return false;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/13
//=======================================================================================
void DTMElement::HasSymbologyOverride::set (bool value)
    {
//    PIN_ELEMENTHANDLE
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 10/13
//=======================================================================================
DTMElement^ DTMElement::GetSymbologyOverrideElement ()
    {
    PIN_ELEMENTHANDLE
    DgnPlatform::DgnModelRef* rootModel = thisEeh->GetModelRef()->GetRoot();
    DgnPlatform::ElementHandle symbologyElement;
    if (TMSymbologyOverrideManager::GetElementForSymbology (*thisEeh, symbologyElement, rootModel))
        {
        if (thisEeh->GetElementRef () != symbologyElement.GetElementRef () && rootModel == symbologyElement.GetModelRef())
            {
            DGNET::Elements::Element^ element = DGNET::Convert::ElementToManaged (symbologyElement);
            return dynamic_cast<DTMElement^>(element);
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 03/11
//=======================================================================================
System::String^ DTMElement::Name::get ()
    {
    PIN_ELEMENTHANDLE
    WString name;

    if (DTMElementHandlerManager::GetName (*thisEeh, name) == SUCCESS)
        return gcnew System::String (name.GetWCharCP());
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 03/11
//=======================================================================================
void DTMElement::Name::set (System::String^ value)
    {
    PIN_ELEMENTHANDLE_ADJUSTFINALIZE
    pin_ptr<const wchar_t> p = PtrToStringChars (value);

    DTMElementHandlerManager::SetName (*thisEeh, p);
    thisEeh->ReplaceInModel(thisEeh->GetElementRef()); // ToDo is this needed?
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/11
//=======================================================================================
System::String^ DTMElement::ThematicDisplayStyle::get ()
    {
    PIN_ELEMENTHANDLE
    WString name;
    int dsIndex;
    DTMElementHandlerManager::GetThematicDisplayStyleIndex (*thisEeh, dsIndex);
    DisplayStyleCP ds = DisplayStyleManager::GetDisplayStyleByIndex (dsIndex, *thisEeh->GetDgnFileP());

    if (ds)
        return gcnew System::String (ds->GetName().GetWCharCP ());
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/11
//=======================================================================================
void DTMElement::ThematicDisplayStyle::set (System::String^ value)
    {
    PIN_ELEMENTHANDLE_ADJUSTFINALIZE
    pin_ptr<const wchar_t> p = PtrToStringChars (value);

    int dsIndex = -1;
    if (value)
        {
        DisplayStyleCP ds = DisplayStyleManager::FindDisplayStyleByName (p, thisEeh->GetDgnFileP ());

        if (ds)
            {
            ds = DisplayStyleManager::EnsureDisplayStyleIsInFile(*ds, *thisEeh->GetDgnFileP());
            dsIndex = ds->GetIndex();
            }
        }
    int oldDsIndex;
    DTMElementHandlerManager::GetThematicDisplayStyleIndex (*thisEeh, oldDsIndex);
    if (oldDsIndex != dsIndex)
        DTMElementHandlerManager::SetThematicDisplayStyleIndex (*thisEeh, dsIndex);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMTrianglesElement^ DTMElement::TrianglesElement::get()
    {
    PIN_ELEMENTHANDLE

    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementTrianglesHandler* hand = dynamic_cast<DTMElementTrianglesHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            return gcnew DTMTrianglesElement(iter.GetCurrentId(), this);
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMFeatureElement^ DTMElement::FeatureElement::get()
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementFeaturesHandler* hand = dynamic_cast<DTMElementFeaturesHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            return gcnew DTMFeatureElement(iter.GetCurrentId(), this);
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 07/08
//=======================================================================================
DTMFeatureElement^ DTMElement::GetFeatureElement(DTMElementFeaturesHandler::FeatureTypes type)
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementFeaturesHandler* hand = dynamic_cast<DTMElementFeaturesHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            DTMElementFeaturesHandler::DisplayParams featureDisplayParams (iter);
            if (featureDisplayParams.GetTag() == (short)type)
                {
                return gcnew DTMFeatureElement(iter.GetCurrentId(), this);
                }
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMFeatureElement^ DTMElement::FeatureBreaklineElement::get()
    {
    return GetFeatureElement (DTMElementFeaturesHandler::Breakline);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 09/10
//=======================================================================================
DTMFeatureElement^ DTMElement::FeatureHoleElement::get()
    {
    return GetFeatureElement (DTMElementFeaturesHandler::Hole);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 09/10
//=======================================================================================
DTMFeatureElement^ DTMElement::FeatureIslandElement::get()
    {
    return GetFeatureElement (DTMElementFeaturesHandler::Island);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 09/10
//=======================================================================================
DTMFeatureElement^ DTMElement::FeatureVoidElement::get()
    {
    return GetFeatureElement (DTMElementFeaturesHandler::Void);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 09/10
//=======================================================================================
DTMFeatureElement^ DTMElement::FeatureBoundaryElement::get()
    {
    return GetFeatureElement (DTMElementFeaturesHandler::Boundary);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 09/10
//=======================================================================================
DTMFeatureElement^ DTMElement::FeatureContourElement::get()
    {
    return GetFeatureElement (DTMElementFeaturesHandler::Contour);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 01/11
//=======================================================================================
DTMFeatureSpotElement^ DTMElement::FeatureSpotElement::get()
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementFeatureSpotsHandler* hand = dynamic_cast<DTMElementFeatureSpotsHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            return gcnew DTMFeatureSpotElement(iter.GetCurrentId(), this);
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMSpotElement^ DTMElement::SpotsElement::get()
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementSpotsHandler* hand = dynamic_cast<DTMElementSpotsHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            return gcnew DTMSpotElement(iter.GetCurrentId(), this);
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 08/10
//=======================================================================================
DTMLowPointElement^ DTMElement::LowPointElement::get()
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementLowPointsHandler* hand = dynamic_cast<DTMElementLowPointsHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            return gcnew DTMLowPointElement(iter.GetCurrentId(), this);
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 08/10
//=======================================================================================
DTMHighPointElement^ DTMElement::HighPointElement::get()
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementHighPointsHandler* hand = dynamic_cast<DTMElementHighPointsHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            return gcnew DTMHighPointElement(iter.GetCurrentId(), this);
            }
        }
    return nullptr;
    }

#ifdef INCLUDE_CATCHMENT
//=======================================================================================
// @bsimethod                                                   Steve.Jones 08/10
//=======================================================================================
DTMCatchmentAreaElement^ DTMElement::CatchmentAreaElement::get()
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementCatchmentAreasHandler* hand = dynamic_cast<DTMElementCatchmentAreasHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            return gcnew DTMCatchmentAreaElement(iter.GetCurrentId(), this);
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 08/10
//=======================================================================================
DTMPondElement^ DTMElement::PondElement::get()
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementPondsHandler* hand = dynamic_cast<DTMElementPondsHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            return gcnew DTMPondElement(iter.GetCurrentId(), this);
            }
        }
    return nullptr;
    }
#endif

//=======================================================================================
// @bsimethod                                                   Steve.Jones 08/10
//=======================================================================================
DTMFlowArrowElement^ DTMElement::FlowArrowElement::get()
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementFlowArrowsHandler* hand = dynamic_cast<DTMElementFlowArrowsHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            return gcnew DTMFlowArrowElement(iter.GetCurrentId(), this);
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMContourElement^ DTMElement::MajorContourElement::get()
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementContoursHandler* hand = dynamic_cast<DTMElementContoursHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            DTMElementContoursHandler::DisplayParams params (iter);
            if (params.GetIsMajor())
                {
                return gcnew DTMContourElement(iter.GetCurrentId(), this);
                }
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMContourElement^ DTMElement::MinorContourElement::get()
    {
    PIN_ELEMENTHANDLE
    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementContoursHandler* hand = dynamic_cast<DTMElementContoursHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            DTMElementContoursHandler::DisplayParams params (iter);
            if (!params.GetIsMajor())
                {
                return gcnew DTMContourElement(iter.GetCurrentId(), this);
                }
            }
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
cli::array<DTMRegionElement^>^ DTMElement::GetRegionElements()
    {
    PIN_ELEMENTHANDLE
    System::Collections::Generic::List<DTMRegionElement^>^ list = gcnew System::Collections::Generic::List<DTMRegionElement^>();

    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementRegionsHandler* hand = dynamic_cast<DTMElementRegionsHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (hand)
            {
            list->Add (gcnew DTMRegionElement(iter.GetCurrentId(), this));
            }
        }
    return list->ToArray();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
cli::array<DTMSubElement^>^ DTMElement::GetSubElements()
    {
    PIN_ELEMENTHANDLE
    System::Collections::Generic::List<DTMSubElement^>^ list = gcnew System::Collections::Generic::List<DTMSubElement^>();

    DTMSubElementIter iter (*thisEeh, true);

    for (; iter.IsValid(); iter.ToNext())
        {
        DTMElementSubHandler* hand = dynamic_cast<DTMElementSubHandler*>(DTMElementSubHandler::FindHandler(iter));

        if (dynamic_cast<DTMElementRegionsHandler*>(hand))
            {
            list->Add(gcnew DTMRegionElement(iter.GetCurrentId(), this));
            }
        else if (dynamic_cast<DTMElementContoursHandler*>(hand))
            {
            list->Add(gcnew DTMContourElement(iter.GetCurrentId(), this));
            }
        else if (dynamic_cast<DTMElementSpotsHandler*>(hand))
            {
            list->Add(gcnew DTMSpotElement(iter.GetCurrentId(), this));
            }
        else if (dynamic_cast<DTMElementFeaturesHandler*>(hand))
            {
            list->Add(gcnew DTMFeatureElement(iter.GetCurrentId(), this));
            }
        else if (dynamic_cast<DTMElementRasterDrapingHandler*>(hand))
            {
            list->Add(gcnew DTMRasterDrapingElement(iter.GetCurrentId(), this));
            }
        else if (dynamic_cast<DTMElementTrianglesHandler*>(hand))
            {
            BeAssert(dynamic_cast<DTMElementRasterDrapingHandler*>(hand) == 0);
            list->Add(gcnew DTMTrianglesElement(iter.GetCurrentId(), this));
            }       
        else if (dynamic_cast<DTMElementFlowArrowsHandler*>(hand))
            {
            list->Add(gcnew DTMFlowArrowElement(iter.GetCurrentId(), this));
            }
        else if (dynamic_cast<DTMElementLowPointsHandler*>(hand))
            {
            list->Add(gcnew DTMLowPointElement(iter.GetCurrentId(), this));
            }
        else if (dynamic_cast<DTMElementHighPointsHandler*>(hand))
            {
            list->Add(gcnew DTMHighPointElement(iter.GetCurrentId(), this));
            }
        else if (dynamic_cast<DTMElementFeatureSpotsHandler*>(hand))
            {
            list->Add(gcnew DTMFeatureSpotElement(iter.GetCurrentId(), this));
            }
#ifdef INCLUDE_CATCHMENT
        else if (dynamic_cast<DTMElementCatchmentAreasHandler*>(hand))
            {
            list->Add(gcnew DTMCatchmentAreaElement(iter.GetCurrentId(), this));
            }
        else if (dynamic_cast<DTMElementPondsHandler*>(hand))
            {
            list->Add(gcnew DTMPondElement(iter.GetCurrentId(), this));
            }
#endif
        }

    return list->ToArray();
    }

DTMSubElement::DTMSubElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement)
    {
    m_id = new Bentley::TerrainModel::Element::DTMSubElementId (xAttrId);
    m_dtmElement = dtmElement;

    PIN_ELEMENTHANDLE_OF (dtmElement);

    m_params = nullptr;
    ElementHandle::XAttributeIter iter (*dtmElementEeh, DTMElementSubHandler::GetDisplayInfoXAttributeHandlerId(), xAttrId.GetId());

    if (iter.IsValid())
        {
        m_params = DTMElementSubHandler::GetParams (iter);
        m_params->AddRef();
        }
    }

DTMSubElement::!DTMSubElement()
    {
    if (m_id)
        {
        delete m_id;
        m_id = nullptr;
        }
    if (m_params)
        {
        m_params->Release();
        m_params = nullptr;
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMSubElement::Commit (DTMElement^ element)
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);

    PIN_ELEMENTHANDLE_ADJUSTFINALIZE_OF (element);

    params.SetElement (*elementEeh, *m_id);
    }


DTMElementFeaturesHandler::FeatureTypes DTMFeatureElement::GetFeatureType::get()
    {
    GETDISPLAYPARAM(DTMElementFeaturesHandler::DisplayParams)
    return params.GetTag ();
    }

DTMContourElement::DTMContourElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMSubElementTextStyle (xAttrId, dtmElement)
    {
    PIN_ELEMENTHANDLE_OF (dtmElement);
    LevelIdXAttributeParams levelIdParam;

    levelIdParam.Get (*dtmElementEeh, m_id->GetId());
    m_additionalLevelId = levelIdParam.GetLevelId ();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
bool DTMContourElement::IsMajorContour::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams)
    return params.GetIsMajor ();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
double DTMContourElement::ContourInterval::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams)
    return params.GetContourInterval ();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMContourElement::ContourInterval::set (double value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetContourInterval (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMContourSmoothingMethod DTMContourElement::ContourSmoothing::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return (DTMContourSmoothingMethod)params.GetContourSmoothing ();
    }

void DTMContourElement::ContourSmoothing::set (DTMContourSmoothingMethod value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetContourSmoothing ((int)(DTMContourSmoothing)value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
double DTMContourElement::SmoothingFactor::get()
    {
    GETDISPLAYPARAM (DTMElementContoursHandler::DisplayParams);
    return params.GetSmoothingFactor();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMContourElement::SmoothingFactor::set (double value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetSmoothingFactor (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
int DTMContourElement::ContourLabelPrecision::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return params.GetContourLabelPrecision ();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMContourElement::ContourLabelPrecision::set (int value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetContourLabelPrecision (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
int DTMContourElement::SplineDensification::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return params.GetSplineDensification ();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMContourElement::SplineDensification::set (int value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetSplineDensification (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMContourElement::ContourDrawTextOption DTMContourElement::DrawTextOption::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return (DTMContourElement::ContourDrawTextOption)params.GetDrawTextOption ();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMContourElement::DrawTextOption::set (DTMContourElement::ContourDrawTextOption value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetDrawTextOption ((DTMElementContoursHandler::DrawTextOption)value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DGNET::LevelId DTMContourElement::TextLevelId::get ()
    {
    return m_additionalLevelId;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMContourElement::TextLevelId::set (DGNET::LevelId value)
    {
    m_additionalLevelId = value;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMContourElement::ContourTextPosition DTMContourElement::TextPosition::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return (DTMContourElement::ContourTextPosition)params.GetTextPosition();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMContourElement::TextPosition::set (DTMContourElement::ContourTextPosition value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetTextPosition ((short)value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMContourElement::ContourTextFrequency DTMContourElement::TextFrequency::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return (DTMContourElement::ContourTextFrequency)params.GetTextFrequency ();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMContourElement::TextFrequency::set (DTMContourElement::ContourTextFrequency value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetTextFrequency ((short)value);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 07/10
//=======================================================================================
unsigned char DTMContourElement::MaxSlopeOption::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return params.GetMaxSlopeOption ();
    }

//=======================================================================================
// @bsimethod                                                    Steve.Jones  07/10
//=======================================================================================
void DTMContourElement::MaxSlopeOption::set (unsigned char value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetMaxSlopeOption (value);
    }

//=======================================================================================
// @bsimethod                                                    Steve.Jones  07/10
//=======================================================================================
double DTMContourElement::MaxSlopeValue::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return params.GetMaxSlopeValue ();
    }

//=======================================================================================
// @bsimethod                                                    Steve.Jones  07/10
//=======================================================================================
void DTMContourElement::MaxSlopeValue::set (double value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetMaxSlopeValue (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
double DTMContourElement::TextInterval::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return params.GetTextInterval ();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMContourElement::TextInterval::set (double value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetTextInterval (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
bool DTMContourElement::NoTextForSmallContours::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return params.GetNoTextForSmallContours ();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMContourElement::NoTextForSmallContours::set (bool value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetNoTextForSmallContours (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
double DTMContourElement::SmallContourLength::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return params.GetSmallContourLength ();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMContourElement::SmallContourLength::set (double value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.SetSmallContourLength (value);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  05/15
//=======================================================================================
void DTMContourElement::Commit (DTMElement^ element)
    {
    PIN_ELEMENTHANDLE_OF (element);
    LevelIdXAttributeParams levelIdParam;
    levelIdParam.Get (*elementEeh, m_id->GetId ());
    if (levelIdParam.GetLevelId () != (DgnPlatform::LevelId)m_additionalLevelId)
        {
        levelIdParam.SetLevelId ((DgnPlatform::LevelId)m_additionalLevelId);
        levelIdParam.Set (*elementEeh, m_id->GetId ());
        }
    __super::Commit (element);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DGNET::DgnTextStyle^ DTMSubElementTextStyle::TextStyle::get()
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParamsAndTextStyle);
    return DGNET::DgnTextStyle::GetById ((DGNET::ElementId)(__int64)params.GetTextStyleID (), GetElement()->DgnModelRef->GetDgnFile());
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMSubElementTextStyle::TextStyle::set (DGNET::DgnTextStyle^ value)
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParamsAndTextStyle);
    if (value)
        params.SetTextStyleID ((int)(Int64)value->Id);
    else
        params.SetTextStyleID (0);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones  11/10
//=======================================================================================
System::UInt32 DTMContourElement::DepressionColor::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return params.GetDepressionSymbology().color;
    }

//=======================================================================================
// @bsimethod                                                    Steve.Jones  11/10
//=======================================================================================
void DTMContourElement::DepressionColor::set (System::UInt32 value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.GetDepressionSymbology().color = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Steve.Jones  11/10
+---------------+---------------+---------------+---------------+---------------+------*/
System::Int32 DTMContourElement::DepressionLineStyle::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return params.GetDepressionSymbology().style;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Steve.Jones  11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DTMContourElement::DepressionLineStyle::set (System::Int32 value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.GetDepressionSymbology().style = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Steve.Jones  11/10
+---------------+---------------+---------------+---------------+---------------+------*/
System::UInt32 DTMContourElement::DepressionWeight::get()
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    return params.GetDepressionSymbology().weight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Steve.Jones  11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DTMContourElement::DepressionWeight::set (System::UInt32 value)
    {
    GETDISPLAYPARAM(DTMElementContoursHandler::DisplayParams);
    params.GetDepressionSymbology().weight = value;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
bool DTMSpotElement::DisplayCell::get()
    {
    GETDISPLAYPARAM(DTMElementSpotsHandler::DisplayParams);
    return params.GetWantSpotCells ();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMSpotElement::DisplayCell::set (bool value)
    {
    GETDISPLAYPARAM(DTMElementSpotsHandler::DisplayParams);
    params.SetWantSpotCells (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMPointElement::Commit (DTMElement^ element)
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    if (params.GetPointCellType() == DTMElementPointsHandler::DisplayParams::Cell)
        {
        PIN_ELEMENTHANDLE_OF (element);
        DTMElementHandlerManager::GetDTMElementPowerPlatformExtension()->EnsureSharedCellDefinitionExists (params.GetPointCellName().GetWCharCP(), elementEeh->GetModelRef());
        }
    __super::Commit (element);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
bool DTMPointElement::DisplayText::get()
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    return params.GetWantPointText ();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMPointElement::DisplayText::set (bool value)
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    params.SetWantPointText (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
System::String^ DTMPointElement::CellName::get()
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    return gcnew System::String (params.GetPointCellName().GetWCharCP());
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMPointElement::CellName::set (System::String^ value)
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    pin_ptr<const wchar_t> p = PtrToStringChars (value);
    params.SetPointCellName (p);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 08/10
//=======================================================================================
int DTMPointElement::CellType::get()
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    return params.GetPointCellType();
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 08/10
//=======================================================================================
void DTMPointElement::CellType::set (int value)
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    params.SetPointCellType ((DTMElementSpotsHandler::DisplayParams::PointCellType)(value));
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 11/10
//=======================================================================================
System::String^ DTMPointElement::TextPrefix::get()
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    return gcnew System::String (params.GetPointTextPrefix().GetWCharCP ());
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 11/10
//=======================================================================================
void DTMPointElement::TextPrefix::set (System::String^ value)
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    pin_ptr<const wchar_t> p = PtrToStringChars (value);
    params.SetPointTextPrefix (p);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 11/10
//=======================================================================================
System::String^ DTMPointElement::TextSuffix::get()
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    return gcnew System::String (params.GetPointTextSuffix().GetWCharCP ());
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 11/10
//=======================================================================================
void DTMPointElement::TextSuffix::set (System::String^ value)
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    pin_ptr<const wchar_t> p = PtrToStringChars (value);
    params.SetPointTextSuffix (p);
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 07/11
//=======================================================================================
double DTMLowPointElement::MinimumDepth::get()
    {
    GETDISPLAYPARAM(DTMElementLowPointsHandler::DisplayParams);
    return params.GetLowPointMinimumDepth();
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 07/11
//=======================================================================================
void DTMLowPointElement::MinimumDepth::set (double value)
    {
    GETDISPLAYPARAM(DTMElementLowPointsHandler::DisplayParams);
    params.SetLowPointMinimumDepth (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
BGEO::DPoint3d DTMPointElement::CellSize::get()
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    return BGEO::DPoint3d(params.GetPointCellSize().x, params.GetPointCellSize().y, params.GetPointCellSize().z);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMPointElement::CellSize::set (BGEO::DPoint3d value)
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    DPoint3d size;
    size.x = value.X;
    size.y = value.Y;
    size.z = value.Z;
    params.SetPointCellSize (size);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
BGEO::DPoint3d DTMPointElement::TextOffset::get()
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    return BGEO::DPoint3d (params.GetTextOffset().x, params.GetTextOffset().y, params.GetTextOffset().z);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMPointElement::TextOffset::set (BGEO::DPoint3d value)
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    DPoint3d textOffset;
    textOffset.x = value.X;
    textOffset.y = value.Y;
    textOffset.z = value.Z;
    params.SetTextOffset (textOffset);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
short DTMPointElement::TextAlignment::get()
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    return params.GetTextAlignment();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMPointElement::TextAlignment::set (short value)
    {
    GETDISPLAYPARAM(DTMElementPointsHandler::DisplayParams);
    params.SetTextAlignment (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
bool DTMFeatureSpotElement::DisplayCell::get()
    {
    GETDISPLAYPARAM(DTMElementFeatureSpotsHandler::DisplayParams);
    return params.GetWantFeatureSpotCells();
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMFeatureSpotElement::DisplayCell::set (bool value)
    {
    GETDISPLAYPARAM(DTMElementFeatureSpotsHandler::DisplayParams);
    params.SetWantFeatureSpotCells (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
System::String^ DTMRegionElement::Description::get()
    {
    GETDISPLAYPARAM(DTMElementRegionsHandler::DisplayParams);
    return gcnew System::String (params.GetDescription().GetWCharCP());
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMRegionElement::Description::set (System::String^ value)
    {
    GETDISPLAYPARAM(DTMElementRegionsHandler::DisplayParams);
    pin_ptr<const wchar_t> p = PtrToStringChars (value);
    params.SetDescription (p);
    }

#ifdef INCLUDE_CATCHMENT
//=======================================================================================
// @bsimethod                                                   Steve.Jones 10/10
//=======================================================================================
double DTMCatchmentAreaElement::CatchmentAreaMinimumDepth::get()
    {
    DTMElementCatchmentAreasHandler::DisplayParams params (m_dtmElement->thisEeh, *m_id);
    return params.catchmentAreaMinimumDepth;
    }

//=======================================================================================
// @bsimethod                                                    Steve.Jones  10/10
//=======================================================================================
void DTMCatchmentAreaElement::CatchmentAreaMinimumDepth::set (double value)
    {
    DTMElementCatchmentAreasHandler::DisplayParams params (m_dtmElement->thisEeh, *m_id);
    params.catchmentAreaMinimumDepth = value;
    DTMSUBELEMENT_MODIFY
    params.SetElement (editHandle, *m_id);
    }
#endif

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DGNET::MaterialId^ DTMMaterialElement::MaterialId::get()
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyAndMaterialParams);
    return gcnew DGNET::MaterialId ((DGNET::ElementId)params.GetMaterialElementID());
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMMaterialElement::MaterialId::set (DGNET::MaterialId^ value)
    {
// ToDo Vancouver    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyAndMaterialParams);
// ToDo Vancouver    params.SetMaterialElementId (value);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 08/13
//=======================================================================================
void DTMMaterialElement::SetMaterialInfo (System::String^ palette, System::String^ material)
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyAndMaterialParams);
    PIN_ELEMENTHANDLE_OF (m_dtmElement);
    if (System::String::IsNullOrEmpty (material))
        params.SetMaterialElementID (0);
    else
        {
        DgnFileP SymbologyDgnFile = m_dtmElementEeh->GetDgnFileP();
        DgnModelRefP SymbologyDgnModelRef = m_dtmElementEeh->GetModelRef();
        pin_ptr<const wchar_t> wPalette = PtrToStringChars (palette);
        pin_ptr<const wchar_t> wMaterial = PtrToStringChars (material);
        ElementId materialElementId = 0;
        MaterialCP material_local;

        material_local = MaterialManager::GetManagerR().FindMaterial (nullptr, Bentley::DgnPlatform::MaterialId (wMaterial), *SymbologyDgnFile, *SymbologyDgnModelRef, true);

        if (material_local)
            materialElementId = material_local->GetElementId();
        else
            {
            MaterialList materials;
            MaterialManager::GetManagerR().FindMaterialByNameFromAnySource (nullptr, materials, wMaterial, *SymbologyDgnModelRef, true);

            if (materials.size() != 0)
                {
                for (MaterialList::iterator mit = materials.begin(); mit != materials.end(); mit++)
                    {
                    MaterialCP mat = *mit;
                    if (wPalette == nullptr || wPalette[0] == 0 || mat->GetPalette().GetName ().CompareToI (wPalette) == 0)
                        {
                        Bentley::DgnPlatform::MaterialId materialId;
                        MaterialPtr newMaterial = Material::Create (*mat, *SymbologyDgnModelRef); 
                        PaletteInfoPtr paletteInfo = PaletteInfo::Create (mat->GetPalette ().GetName ().c_str (), SymbologyDgnFile->GetDocument ().GetMoniker (), mat->GetPalette ().GetSource (), PaletteInfo::PALETTETYPE_Dgn);

                        newMaterial->GetPaletteR ().Copy (*paletteInfo); 

                        if (SUCCESS == MaterialManager::GetManagerR ().SaveMaterial (&materialId, *newMaterial, SymbologyDgnFile))
                            materialElementId = materialId.GetElementId();
                        break;
                        }
                    }
                }
            }
        params.SetMaterialElementID (materialElementId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
 System::UInt32 DTMSubElement::Color::get()
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    return params.GetSymbology().color;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DTMSubElement::Color::set (System::UInt32 value)
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    params.GetSymbology().color = value;
    }

//=======================================================================================
// @bsimethod                                                   Steve.Jones 28/10
//=======================================================================================
bool DTMSubElement::Visible::get()
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    return params.GetVisible();
    }

//=======================================================================================
// @bsimethod                                                    Steve.Jones 28/10
//=======================================================================================
void DTMSubElement::Visible::set (bool value)
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    params.SetVisible (value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
System::Int32 DTMSubElement::LineStyle::get()
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    return params.GetSymbology().style;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DTMSubElement::LineStyle::set (System::Int32 value)
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    params.GetSymbology().style = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
System::UInt32 DTMSubElement::Weight::get()
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    return params.GetSymbology().weight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DTMSubElement::Weight::set (System::UInt32 value)
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    params.GetSymbology().weight = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
DGNET::LevelId DTMSubElement::LevelId::get()
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    return (DGNET::LevelId)params.GetLevelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DTMSubElement::LevelId::set (DGNET::LevelId value)
    {   
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    params.SetLevelId ((unsigned int)value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
System::Double DTMSubElement::Transparency::get()
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    return params.GetTransparency();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DTMSubElement::Transparency::set (System::Double value)
    {
    GETDISPLAYPARAM(DTMElementSubHandler::SymbologyParams);
    params.SetTransparency (value);
    }

DTMElement::DTMElement (Bentley::DgnPlatformNET::DgnModel^ model, Element^ templateElement, TerrainModelNET::DTM^ dtm)
    {
    BcDTMP iBcDTM = (BcDTMP)dtm->ExternalHandle.ToPointer ();   
    DgnModelP modelNative = DgnPlatformNET::DgnModel::GetNativeDgnModelP (model, false);

    PIN_ELEMENTHANDLE_OF (templateElement)

    DgnPlatform::EditElementHandle peer;
    Bentley::BentleyStatus status = Bentley::TerrainModel::Element::DTMElementDisplayHandler::CreateDTMElement (peer, templateElementEeh, *iBcDTM, *modelNative, false);

    Bentley::DgnPlatformNET::StatusHandler::HandleStatus (status);   //  throws an exception if not SUCCESS

    InitializeFromElementHandle (peer);

    System::GC::KeepAlive (templateElement);
    System::GC::KeepAlive (model);
    System::GC::KeepAlive (dtm);
    }

TerrainModelNET::DTM^ DTMElement::GetDTM ()
    {
    PIN_ELEMENTHANDLE
    RefCountedPtr<Bentley::TerrainModel::Element::DTMDataRef> dataRef;
    if (SUCCESS == Bentley::TerrainModel::Element::DTMElementHandlerManager::GetDTMDataRef (dataRef, *thisEeh))
        {
        Bentley::TerrainModel::DTMPtr dtm;

        dataRef->GetDTMReferenceDirect (dtm);
        BcDTMP bcDTM = dynamic_cast<BcDTMP>(dtm.get());
        if (bcDTM)
            return Bentley::TerrainModelNET::DTM::FromHandle (System::IntPtr (bcDTM));
       }
    return nullptr;
    }


public ref class Helper
    {
    public:
        static DGNET::Elements::Element^ FromEditElementHandle (EditElementHandle* element)
        {
        return DGNET::Convert::ElementToManaged (element);
        }
    static DGNET::Elements::Element^ FromEditElementHandleIntPtr (System::IntPtr elementPtr)
        {
        EditElementHandleP elementHandle = (EditElementHandleP)elementPtr.ToPointer ();
        if (DTMElementHandlerManager::IsDTMElement (*elementHandle))
            {
            DGNET::Elements::Element^ element = DGNET::Convert::ElementToManaged (elementHandle);
            DTMElement^ dtmElement = dynamic_cast<DTMElement^>(element);

            if (dtmElement)
                dtmElement->ForceToUseEditElemHandle ((EditElementHandle*)elementPtr.ToPointer ());
            return element;
            }
        return nullptr;
        }
    static void ReleaseElementHandler (DGNET::Elements::Element^ element)
        {
        DTMElement^ dtmElement = dynamic_cast<DTMElement^>(element);
        if (dtmElement)
           dtmElement->ReleaseElementHandler ();
        }
    };
END_BENTLEY_TERRAINMODELNET_ELEMENT_NAMESPACE

EXPORT_ATTRIBUTE void registerManagedElementHandler()
    {
    DGNET::Elements::ManagedElementFactoryExtension::RegisterExtension (DTMElementHandler::GetInstance(), *new DGNET::Elements::ManagedElementFactory (gcnew DGNET::Elements::ElementFactoryDelegate (&Bentley::TerrainModelNET::Element::DTMElement::GetDTMElement)));
    DGNET::Elements::ManagedElementFactoryExtension::RegisterExtension (TMSymbologyOverrideHandler::GetInstance(), *new DGNET::Elements::ManagedElementFactory (gcnew DGNET::Elements::ElementFactoryDelegate (&Bentley::TerrainModelNET::Element::DTMElement::GetDTMElement)));
    }

