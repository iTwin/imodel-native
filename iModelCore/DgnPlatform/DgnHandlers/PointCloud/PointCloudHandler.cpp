/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/PointCloud/PointCloudHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnCore/DgnDocumentManager.h>
#include "PointCloudAttributes.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MakeDgnModelWritable::MakeDgnModelWritable(DgnModelR cache) : m_dgnCache(cache)
    {
    if(m_wasReadOnly = cache.IsReadOnly())
        {
        m_dgnCache.SetReadOnly(false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MakeDgnModelWritable::~MakeDgnModelWritable()
    {
    if(m_wasReadOnly)
        m_dgnCache.SetReadOnly(true);
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Eric.Paquet  11/2011
//=======================================================================================
struct PointCloudDgnModelAppData : public DgnModelAppData
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 07/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    static DgnModelAppData::Key const& GetKey () { static DgnModelAppData::Key s_key; return s_key; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 07/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    static PointCloudDgnModelAppData* GetAppDataP(DgnModelR model)               
        {         
        PointCloudDgnModelAppData* appData = (PointCloudDgnModelAppData*) model.FindAppData (PointCloudDgnModelAppData::GetKey ());
        return appData;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 07/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    static PointCloudDgnModelAppData& GetAppData(DgnModelR model)               
        {
        PointCloudDgnModelAppData* appData = (PointCloudDgnModelAppData*) model.FindAppData (PointCloudDgnModelAppData::GetKey ());
        if(appData != NULL)
            return *appData;

        // If we do not have an appdata that means no PointCloud exist in this DgnModel.
        model.AddAppData (PointCloudDgnModelAppData::GetKey (), appData = new PointCloudDgnModelAppData (model));
        return *appData;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 07/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _OnCleanup (DgnModelR host) override { delete this; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 07/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _OnFilled (DgnModelR host) override
        {
        // Make sure cache is RW because we also want to update the georeference of raster in nested reference and R-O model.
        std::auto_ptr<MakeDgnModelWritable> __makeDgnModelWritable (!m_elementRefList.empty() ? new MakeDgnModelWritable(host) : NULL);

        for (T_StdElementRefSet::iterator itr(m_elementRefList.begin()); itr != m_elementRefList.end(); ++itr)
            {
            DgnPlatformLib::GetHost ().GetPointCloudAdmin()._SyncSpatialReferenceFromFile(*itr);
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 07/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t GetNbPointClouds () { return m_elementRefList.size(); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 07/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    T_StdElementRefSet const& GetPointCloudElementRefList() const { return m_elementRefList; }
    
    /*---------------------------------------------------------------------------------**//**
    // Return true if added.
    * @bsimethod                                    Stephane.Poulin                 07/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool AddPointCloudElementRef(ElementRefP elemRefP) { return m_elementRefList.insert(elemRefP).second; }     
        
    /*---------------------------------------------------------------------------------**//**
    // Return the number of elements erased.
    * @bsimethod                                    Stephane.Poulin                 07/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t RemovePointCloudElementRef(ElementRefP elemRefP) { return m_elementRefList.erase(elemRefP); }            

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 07/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    PointCloudDgnModelAppData(DgnModelR model) 
        {
#ifdef WIP_VANCOUVER_MERGE // pointcloud
        if (GeoCoordinationManager::GetServices ())
            m_hasGcs = GeoCoordinationManager::GetServices ()->HasGCS(&model);
        else 
#endif
            m_hasGcs = false;
        }

private:
    PointCloudDgnModelAppData();
    PointCloudDgnModelAppData(PointCloudDgnModelAppData const&);
    PointCloudDgnModelAppData& operator=(PointCloudDgnModelAppData const&);

    T_StdElementRefSet  m_elementRefList;       // Point cloud elementRef that exist in this DgnModel.
    bool m_hasGcs;
};



END_BENTLEY_DGNPLATFORM_NAMESPACE
#ifdef WIP_VANCOUVER_MERGE // pointcloud
static PointCloudClipReferenceHandler s_pointCloudClipReferenceHandler;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TransformCR             PointCloudProperties::GetTransform () const {return m_transform; }
void                    PointCloudProperties::SetTransform (TransformCR trans) {m_transform = trans; }
DgnDocumentMonikerCR    PointCloudProperties::GetFileMoniker() const {return *m_monikerPtr;}
void                    PointCloudProperties::SetFileMoniker(DgnDocumentMonikerR moniker) {m_monikerPtr = &moniker;}
bool                    PointCloudProperties::GetLocate () const { return !m_flags.b.ignoreLocate;}
void                    PointCloudProperties::SetLocate (bool isOn) { m_flags.b.ignoreLocate = !isOn;}
bool                    PointCloudProperties::GetLockedGeoReference () const { return !m_flags.b.unlockedGeoRef;}
void                    PointCloudProperties::SetLockedGeoReference (bool isLocked) { m_flags.b.unlockedGeoRef = !isLocked;}
WStringCR               PointCloudProperties::GetDescription () const { return m_description; }
void                    PointCloudProperties::SetDescription (WStringCR description) { m_description = description; }
float                   PointCloudProperties::GetViewDensity() const { return m_viewDensity; }
void                    PointCloudProperties::SetViewDensity(float density) { BeAssert(IN_RANGE(density, 0.0,1.0)); m_viewDensity = density; }
double                  PointCloudProperties::GetUorPerMeter() const { return m_uorPerMeter; }
void                    PointCloudProperties::SetUorPerMeter(double val) { m_uorPerMeter = val; }
DPoint3dCR              PointCloudProperties::GetGlobalOrigin() const {return m_globalOrigin;}
void                    PointCloudProperties::SetGlobalOrigin(DPoint3dCR val) {m_globalOrigin = val;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR PointCloudProperties::GetSpatialReferenceWkt() const
    {
    return m_wktString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudProperties::SetSpatialReferenceWkt(WStringCR wktString)
    {
    m_wktString = wktString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudProperties::HasSpatialReferenceWkt() const
    { 
    return !m_wktString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                    PointCloudProperties::GetViewState(int viewNumber) const       
    { 
    if(IN_RANGE(viewNumber, 0, 7))
        return (m_viewStates & (1 << viewNumber)) != 0; 

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void                    PointCloudProperties::SetViewState(int viewNumber, bool state) 
    {
    BeAssert(IN_RANGE(viewNumber, 0,7));
    if(!IN_RANGE(viewNumber, 0,7))
        return;

    if(state)
        m_viewStates |= (1 << viewNumber);
    else
        m_viewStates &= ~(1 << viewNumber); 
    }

//BEIJING_WIP_POINTCLOUD: PointCloudHandler::_SetFileName: where to put the logic now? Should not invalidate anything related to the elementRef
//   The element is not persisted yet.


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudPropertiesPtr PointCloudProperties::Create()
    {
    return new PointCloudProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudPropertiesPtr PointCloudProperties::Create(DgnDocumentMonikerR moniker, DgnModelCR modelRef)
    {
    return new PointCloudProperties(moniker, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudPropertiesPtr PointCloudProperties::Create(ElementHandleCR eh)
    {
    PointCloudPropertiesPtr propsP = new PointCloudProperties();

    if(SUCCESS == propsP->LoadFromElement(eh))
        return propsP;

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudPropertiesPtr PointCloudProperties::Create(ElementRefP eRef)
    {
    ElementHandle eh(eRef);
    ElementHandle::XAttributeIter xAttrItr(eh, XAttributeHandlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_Attributes));
    if(!xAttrItr.IsValid() || xAttrItr.GetSize () == 0)
        return NULL;

    PointCloudPropertiesPtr propsP = new PointCloudProperties();
    DataInternalizer dataInternalizer ((byte*)xAttrItr.PeekData(), xAttrItr.GetSize());
    if(SUCCESS != propsP->InitFromRawData(dataInternalizer, NULL))
        return NULL;

    return propsP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudProperties::PointCloudProperties()
    {
    Clear(NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudProperties::PointCloudProperties(DgnDocumentMonikerR moniker, DgnModelCR modelRef)
    {
    Clear(&modelRef);
    m_monikerPtr = &moniker;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudProperties::Clear(DgnModelCP modelP)
    {
    m_transform.initIdentity();
    m_flags.s = 0;
    m_viewStates = 0xFF;
    m_viewDensity = 1.0;
    m_uorPerMeter = 1000.;
    m_description.clear();
    memset(m_reserved, 0, 4 * sizeof(UInt32));

    m_monikerPtr = NULL;

    m_globalOrigin = modelP->GetGlobalOrigin();
    m_wktString.clear();
    m_unknownAttributes.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudProperties::LoadFromElement(ElementHandleCR eh)
    {
    if (eh.GetHandler().GetHandlerId() != PointCloudHandler::GetElemHandlerId())
        return ERROR;

    ElementHandle::XAttributeIter xAttrItr(eh, XAttributeHandlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_Attributes));
    if(!xAttrItr.IsValid() || xAttrItr.GetSize () == 0)
        return ERROR;

    // Clear internal state
    Clear(eh.GetDgnModelP());

    DataInternalizer dataInternalizer ((byte*)xAttrItr.PeekData(), xAttrItr.GetSize());
    return InitFromRawData(dataInternalizer, eh.GetDgnModelP());
    }

/*---------------------------------------------------------------------------------**//**
* Method that read pointcloud attachment from XAttribute.
* Format:
*       transform    [double[3][4]]
*       filename     [variable length string]  
*       reserved     UInt32[4]
*
*      Optional:
*      For each data block
*          dataType [UInt16]
*          dataSize [UInt32]
*          data     [dataSize]
*      
*      
* @bsimethod                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudProperties::InitFromRawData(DataInternalizer& dataInternalizer, DgnModelP modelRef)
    {
    // --- get the filename ---
    WString fullPath;
    dataInternalizer.get (fullPath);
    
    // --- get the transform ---
    dataInternalizer.get (&m_transform.form3d[0][0], 12);

    // --- get flags ---
    dataInternalizer.get (&m_flags.s);

    // --- get view states ---
    dataInternalizer.get (&m_viewStates);

    // --- get view density ---
    dataInternalizer.get (&m_viewDensity);

    // --- get uor per meter ---
    dataInternalizer.get (&m_uorPerMeter);

    // --- get description ---
    dataInternalizer.get(m_description);
   
    // --- get reserved ---
    dataInternalizer.get(m_reserved, 4);

    // --- get optional data ---
    while(!dataInternalizer.AtOrBeyondEOS())
        {
        PointCloudAttributeBase::DataType dataType = 0;
        dataInternalizer.get(&dataType);

        UInt32 dataSize = 0;
        dataInternalizer.get(&dataSize);

        switch (dataType)
            {
            case PointCloudAttributeBase::AttributeId_MonikerString:
                {
                WString path;
                dataInternalizer.get (path);
                m_monikerPtr = DgnDocumentMoniker::Create(path.c_str(), PointCloudHandler::GetSearchPath(modelRef).c_str());
                break;
                }

            case PointCloudAttributeBase::AttributeId_GlobalOrigin:
                dataInternalizer.get(&m_globalOrigin.x, 3);
                break;

            case PointCloudAttributeBase::AttributeId_WktString:
                dataInternalizer.get(m_wktString);
                break;

            default:
                // Negative ids are not preserved
                if (PointCloudAttributeBase::IsPreservedIfUnknown(dataType))
                    {
                    UnknownAttributeDataPtr pData = UnknownAttributeData::Create(dataType, dataSize);
                    pData->Load(dataInternalizer);
                    m_unknownAttributes.push_back(pData);
                    }
                else
                    {
                    dataInternalizer.skip(dataSize);
                    }
                break;
            }
        }

    // Moniker string always win over fullPath but in case where the moniker string doesn't exist(8.11.7 beta) use the full path.
    if(!m_monikerPtr.IsValid())
        m_monikerPtr = DgnDocumentMoniker::CreateFromFileName(fullPath.c_str(), PointCloudHandler::GetSearchPath(modelRef).c_str());

    return SUCCESS;    
    }

/*---------------------------------------------------------------------------------**//**
* See LoadFromElement for XAttribute format.
* @bsimethod                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudProperties::StoreToElement(EditElementHandleR eeh) const
    {
    if (eeh.GetHandler().GetHandlerId() != PointCloudHandler::GetElemHandlerId())
        return ERROR; 

    BeAssert(m_monikerPtr.IsValid());

    DataExternalizer dataExternalizer;

    // --- set the filename ---
    dataExternalizer.put (m_monikerPtr->GetPortableName());

    // --- set the transform ---
    dataExternalizer.put (&m_transform.form3d[0][0], 12); 

    // --- set flags
    dataExternalizer.put (m_flags.s);

    // --- Set view states
    dataExternalizer.put(m_viewStates);

    // --- Set view density
    dataExternalizer.put(m_viewDensity);

    // --- Set uor per meter
    dataExternalizer.put(m_uorPerMeter);

    // --- Set description
    dataExternalizer.put(m_description.c_str());

    // --- set reserved ---
    dataExternalizer.put(m_reserved, 4);

    // ------------ Optional data block start here ------------

    // --- Set Moniker String data block---
        {
        DataExternalizer myExternalizer;
        myExternalizer.put (m_monikerPtr->Externalize().c_str());

        dataExternalizer.put ((PointCloudAttributeBase::DataType)PointCloudAttributeBase::AttributeId_MonikerString);
        dataExternalizer.put ((UInt32)myExternalizer.getBytesWritten());
        dataExternalizer.put ((UInt8*)myExternalizer.getBuf(), (UInt32)myExternalizer.getBytesWritten());
        }

    // --- Set Global Origin data block---
        {
        dataExternalizer.put ((PointCloudAttributeBase::DataType)PointCloudAttributeBase::AttributeId_GlobalOrigin);
        dataExternalizer.put ((UInt32)sizeof (m_globalOrigin));
        dataExternalizer.put (&m_globalOrigin.x, 3);
        }

    // Set wkt string data block
        {
        DataExternalizer myExternalizer;
        myExternalizer.put (m_wktString);

        dataExternalizer.put ((PointCloudAttributeBase::DataType)PointCloudAttributeBase::AttributeId_WktString);
        dataExternalizer.put ((UInt32)myExternalizer.getBytesWritten());
        dataExternalizer.put ((UInt8*)myExternalizer.getBuf(), (UInt32)myExternalizer.getBytesWritten());
        }

    // --- Write unknown attributes ---
    for(UnknownAttributes::const_iterator itr(m_unknownAttributes.begin()); itr != m_unknownAttributes.end(); ++itr)
        (*itr)->Store(dataExternalizer);
        
    // Tell the element handle to add this XAttribute when rewrite is called.
    return eeh.ScheduleWriteXAttribute (XAttributeHandlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_Attributes), 0, dataExternalizer.getBytesWritten(), dataExternalizer.getBuf());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
Transform PointCloudProperties::ComputeCloudTransform() const
    {
    double factor = GetUorPerMeter();
    DPoint3d globalOrg = GetGlobalOrigin();
    Transform unitToUor;
    unitToUor.initFromRowValues(factor, 0.0, 0.0, globalOrg.x,
                                0.0, factor, 0.0, globalOrg.y,
                                0.0, 0.0, factor, globalOrg.z);

    Transform cloudTrn;
    cloudTrn.productOf(&GetTransform(), &unitToUor);

    return cloudTrn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudPropertiesPtr     IPointCloudQuery::GetPointCloudProperties(ElementHandleCR eh) const {return _GetPointCloudProperties(eh);}
PointCloudClipPropertiesPtr IPointCloudQuery::GetPointCloudClipProperties(ElementHandleCR eh) const {return _GetPointCloudClipProperties(eh);}
PointCloudClipReferencePtr IPointCloudQuery::GetClipReference(ElementHandleCR eh) const            {return _GetClipReference(eh);}

StatusInt                   IPointCloudEdit::SetPointCloudProperties(EditElementHandleR eeh, PointCloudPropertiesCR props) {return _SetPointCloudProperties(eeh, props);}
StatusInt                   IPointCloudEdit::SetPointCloudClipProperties(EditElementHandleR eeh, PointCloudClipPropertiesCR props) {return _SetPointCloudClipProperties(eeh, props);}
StatusInt                   IPointCloudEdit::SetClipReference(EditElementHandleR eeh, PointCloudClipReferenceCR clipRef) {return _SetClipReference(eeh, clipRef);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
//ITransactionHandlerP    PointCloudHandler::_GetITransactionHandler() {return this;} removed in graphite

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerId PointCloudHandler::GetElemHandlerId ()
    {
    return ElementHandlerId (XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_Handler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudHandler::RegisterHandlers ()
    {
    // Register element handler
    DgnSystemDomain::GetInstance().RegisterHandler (PointCloudHandler::GetElemHandlerId (), ELEMENTHANDLER_INSTANCE(PointCloudHandler)); 
#ifdef WIP_VANCOUVER_MERGE // pointcloud
    DgnSystemDomain::GetInstance().RegisterHandler (PointCloudClipElementHandler::GetHandlerId (), ELEMENTHANDLER_INSTANCE(PointCloudClipElementHandler)); 
#endif

    //XAttributeHandlerManager::RegisterPointerContainerHandler (PointCloudClipReferenceHandler::GetXAttributeHandlerId(), &s_pointCloudClipReferenceHandler); removed in graphite

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool      PointCloudHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    // N.B. If cell is supported, we must also change {dgnfileio\lib\history\utils.cpp} isNeverComplexComponent()
    if (SupportOperation::CellGroup == stype)       
        return false;   

    return T_Super::_IsSupportedOperation (eh, stype);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudHandler::_GetTypeName(WStringR descr, UInt32 desiredLength)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_POINT_CLOUD_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudHandler::_GetDescription(ElementHandleCR eh, WStringR descr, UInt32 desiredLength)
    {
    // No point coud admin is available.
    // Default to what is stored in the element.
    PointCloudPropertiesPtr propsP = GetPointCloudProperties(eh);
    if(!propsP.IsValid())
        {
        T_Super::_GetDescription(eh, descr, desiredLength);
        return;
        }

    WString filePathW  = propsP->GetFileMoniker().ResolveDisplayName();
    if (filePathW.size() == 0)
        {
        // NEEDSWORK: &&SP - should be translated
        descr = L"Unknown Point Cloud";
        return;
        }

    // cleanup the PTSS:// scheme type
    WString schemeType (STORAGE_SCHEMETYPE);
    size_t pos =   filePathW.find(schemeType);

    WString embeddedStr;
    if (WString::npos != pos)
        {
        filePathW = filePathW.substr (pos+schemeType.length(), filePathW.length ());
        embeddedStr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_OleTypeEmbedded));
        }

    // Description : "Point Cloud [CompactPath]"
    GetTypeName (descr, desiredLength);  // Get type name part.
    UInt32 alreadyTakenChar((UInt32)descr.length() + 1);  // len(type)

    BeFileName filePath (filePathW.c_str());
    WString compactPath = filePath.Abbreviate (desiredLength-alreadyTakenChar);

    // Description : "Point Cloud [CompactPath]"
    descr.append(L" [").append(embeddedStr).append(compactPath.c_str()).append(L"]");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudHandler::_OnTransform(EditElementHandleR eeh, TransformInfoCR trans)
    {
    if (SUCCESS != T_Super::_OnTransform (eeh, trans))
        return ERROR;

    PointCloudPropertiesPtr propsP = GetPointCloudProperties(eeh);
    if(!propsP.IsValid())
        return ERROR;

    // Apply given transform
    Transform  currentTrans(propsP->GetTransform ());
    Transform  outTrans;
    outTrans.productOf (trans.GetTransform (), &currentTrans);

    propsP->SetTransform (outTrans);

    // Do not use SetPointCloudProperties because "The subclass implementation should not attempt to recompute the element's range."
    return propsP->StoreToElement(eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus PointCloudHandler::_OnGeoCoordinateReprojection (EditElementHandleR source, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain)
    {
    return REPROJECT_NoChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            PointCloudHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // we don't ever want to have pointclouds in a 2dmodel
    eeh.Invalidate ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudHandler::_Draw(ElementHandleCR eh, ViewContextR context)
    {
    //  Our logic for handling load delays by doing redraws always produces a subsequent draw to heal,
    //  effectively causing every to updates every time we decide enough data has been loaded to warrant an
    //  update.

    // BEIJING_WIP_POINTCLOUD: Minimum draw should support draw range in DgnPlatform. For now try to preserve actual range.
    DPoint3d    p[8];
    DRange3dCR range = eh.GetElementCP()->GetRange();

    p[0].x = p[3].x = p[4].x = p[5].x = range.low.x;
    p[1].x = p[2].x = p[6].x = p[7].x = range.high.x;
    p[0].y = p[1].y = p[4].y = p[7].y = range.low.y;
    p[2].y = p[3].y = p[5].y = p[6].y = range.high.y;
    p[0].z = p[1].z = p[2].z = p[3].z = range.low.z;
    p[4].z = p[5].z = p[6].z = p[7].z = range.high.z;
    context.DrawBox (p, true);

//     PointCloudPropertiesPtr propsP = GetPointCloudProperties(eh);
//     if(!propsP.IsValid())
//         return;

    }

#if defined ELEMENT_LOADING_REWORK
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudHandler::_OnAdded (ElementHandleP eh)
    {
    // A point cloud element was added to the model. Add it to the point cloud list.
    PointCloudDgnModelAppData& dgnModelData = PointCloudDgnModelAppData::GetAppData(*eh->GetDgnModelP());
    dgnModelData.AddPointCloudElementRef(eh->GetElementRef());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudHandler::_OnDeleted (ElementHandleP eh)
    {
    // A point cloud element was removed from the model. Remove it from the point cloud list.
    PointCloudDgnModelAppData& dgnModelData = PointCloudDgnModelAppData::GetAppData(*eh->GetDgnModelP());
    dgnModelData.RemovePointCloudElementRef(eh->GetElementRef());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudHandler::_OnElementLoaded (ElementHandleCR eh)
    {
    // Keep track of this loaded point cloud element (add it to the point cloud list)
    PointCloudDgnModelAppData& dgnModelData = PointCloudDgnModelAppData::GetAppData(*eh.GetDgnModelP());
    dgnModelData.AddPointCloudElementRef(eh.GetElementRef());
    }
#endif
//void PointCloudHandler::_OnUndoRedo new in Vancouver? removed in graphite?

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//Handler::PreActionStatus PointCloudHandler::_OnAdd (EditElementHandleR eeh)
//    {
//    return PRE_ACTION_Ok;
//    }

#if defined ELEMENT_LOADING_REWORK
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudHandler::_OnXAttributeChanged (XAttributeHandleCR xAttr, ChangeTrackAction action)
    {
    BeAssert(xAttr.IsValid());

    try
        {
        if(xAttr.GetHandlerId() == XAttributeHandlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_Attributes))
            ProcessAttributesChange(xAttr, action, cantBeUndoneFlag);
        }
    catch(...)
        {
        BeAssert(!"ERROR in PointCloudHandler::_OnXAttributeChanged");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void    PointCloudHandler::_OnUndoRedoXAttributeChange (XAttributeHandleCR xAttr, ChangeTrackAction action, bool isUndo)
    {
    ChangeTrackAction netAction(action);
    if(ChangeTrackAction::XAttributeDelete == action && isUndo)
        netAction = ChangeTrackAction::XAttributeAdd;
    else if(ChangeTrackAction::XAttributeAdd == action && isUndo)
        netAction = ChangeTrackAction::XAttributeDelete;

    _OnXAttributeChanged (xAttr, netAction);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudHandler::CreatePointCloudElement (EditElementHandleR eeh, DgnModelR modelRef, PointCloudPropertiesCR pointCloudProperties, DRange3d range)
    {
    if(!modelRef.Is3d())
        return ERROR;

    // Construct new instance of our element
    ExtendedElementHandler::InitializeElement (eeh, NULL/*template*/, modelRef, true/*is3d*/, false/*isComplexHeader*/);

#ifdef DGNV10FORMAT_CHANGES_WIP
    // Add xAttribute for handler
    ElementHandlerXAttribute handlerXAttr (PointCloudHandler::GetElemHandlerId (), MISSING_HANDLER_PERMISSION_None);
    ElementHandlerManager::AddHandlerToElement (eeh, handlerXAttr);
#endif

    // Write properties to the element
    IPointCloudEdit* pEdit = dynamic_cast<IPointCloudEdit*>(&eeh.GetHandler());
    pEdit->SetPointCloudProperties(eeh, pointCloudProperties);

    // Set element range
    eeh.GetElementP()->GetRangeR() = range;

    if (SUCCESS != eeh.GetDisplayHandler()->ValidateElementRange(eeh))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
T_StdElementRefSet PointCloudHandler::GetPointCloudElementRefsInModel(DgnModelR modelR)
    {
    PointCloudDgnModelAppData* dgnModelData = PointCloudDgnModelAppData::GetAppDataP(modelR);

    T_StdElementRefSet elementRefSet;
    
    if (NULL != dgnModelData)
        elementRefSet = dgnModelData->GetPointCloudElementRefList ();

    return elementRefSet;
    }

#if defined ELEMENT_LOADING_REWORK
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudHandler::ProcessAttributesChange(XAttributeHandleCR xAttr, ChangeTrackAction action, bool* cantBeUndoneFlag) const
    {
    if(!xAttr.IsValid() || xAttr.GetSize () == 0)
        return ERROR;

    switch(action)
        {
        case ChangeTrackAction::XAttributeModify:
        case ChangeTrackAction::XAttributeReplace:
            {
            ElementHandle eh (xAttr.GetElementRef(), NULL);
            PointCloudPropertiesPtr propsP = PointCloudProperties::Create(eh);

            // BEIJING_WIP_POINTCLOUD - NEEEDS_WORK Check if moniker has changed
            //if (propsP->GetFileMoniker().GetPortableName() != PointCloudManager::GetInstance().GetUnresolvedPath(xAttr.GetElementRef()))
            //    PointCloudManager::GetInstance().RemoveScene(xAttr.GetElementRef());
            }
            break;
        }

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudPropertiesPtr PointCloudHandler::_GetPointCloudProperties(ElementHandleCR eh) const
    {
    return PointCloudProperties::Create(eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudClipPropertiesPtr PointCloudHandler::_GetPointCloudClipProperties(ElementHandleCR eh) const
    {
    return PointCloudClipProperties::Create(eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudClipReferencePtr PointCloudHandler::_GetClipReference(ElementHandleCR eh) const
    {
#ifdef WIP_VANCOUVER_MERGE // pointcloud
    return PointCloudClipReference::Create(eh);
#endif
BeAssert(false);
return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudHandler::_SetPointCloudProperties(EditElementHandleR eeh, PointCloudPropertiesCR props)
    {
    if(SUCCESS != props.StoreToElement(eeh))
        return BSIERROR;

    return ValidateElementRange(eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudHandler::_SetPointCloudClipProperties(EditElementHandleR eeh, PointCloudClipPropertiesCR props)
    {
    if(SUCCESS != props.StoreToElement(eeh))
        return ERROR;

    return ValidateElementRange(eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudHandler::_SetClipReference(EditElementHandleR eeh, PointCloudClipReferenceCR clipRef)
    {
#ifdef WIP_VANCOUVER_MERGE // pointcloud
    clipRef.ScheduleWriteXAttribute(eeh);
    return ValidateElementRange(eeh, false/*setZfor2d*/);
#endif
BeAssert(false);
return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PointCloudHandler::GetNbPointClouds (DgnModelP modelRefP)
    {
    if (!modelRefP ||!modelRefP)
        return 0;

    PointCloudDgnModelAppData* dgnModelData = PointCloudDgnModelAppData::GetAppDataP(*modelRefP);

    if (NULL != dgnModelData)
        return dgnModelData->GetNbPointClouds ();

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString PointCloudHandler::GetSearchPath(DgnModelP modelRefP)
    {
    WString searchPaths;

    // A dgn folder to the search list.
    if(NULL != modelRefP)
        {
        WString dirName = BeFileName::GetDirectoryName (modelRefP->GetDgnProject().GetFileName().c_str());

        searchPaths.append(dirName.c_str());
        }

    if(searchPaths.empty())
        searchPaths.append (L"MS_RFDIR");
    else
        searchPaths.append (L";MS_RFDIR");

    return searchPaths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudCollection::PointCloudIterator::ToNext ()
    {
    for (ElementRefP refP = m_it.GetCurrentElementRef(); NULL != refP; refP = m_it.GetNextElementRef())
        {
        ElementHandle eh (refP);
        if (eh.GetLegacyType() == EXTENDED_ELM/*Quickly reject non-106*/ && eh.GetHandler().GetHandlerId() == PointCloudHandler::GetElemHandlerId())
            {
            m_elmRef = refP;
            return;
            }
        }

    m_elmRef = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudClipProperties::PointCloudClipProperties()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudClipProperties::~PointCloudClipProperties()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudClipPropertiesPtr PointCloudClipProperties::Create(ElementHandleCR rasterEh)
    {
    PointCloudClipPropertiesPtr propsP = new PointCloudClipProperties();

    if(SUCCESS == propsP->LoadFromElement(rasterEh))
        return propsP;
    else
        return PointCloudClipProperties::Create();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudClipPropertiesPtr PointCloudClipProperties::Create()
    {
    PointCloudClipPropertiesPtr propsP = new PointCloudClipProperties();
    return propsP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::GetClipBoundary(PointCloudPropertiesCR props, OrientedBoxR clipBox) const
    {
    OrientedBoxListCR clips(m_boundary);

    if (clips.empty())
        // No clip boundary box
        return ERROR;

    OrientedBox localClipBox = clips.front();

    // convert from local coordinates to UOR
    Transform cloudTransform(props.ComputeCloudTransform());

    // Just return first clip boundary box in the collection
    DPoint3d uorOrigin;
    cloudTransform.multiply(&uorOrigin, &localClipBox.GetOrigin (), 1);
    DVec3d uorXVec;
    cloudTransform.multiplyMatrixOnly(&uorXVec, &localClipBox.GetXVec ());
    DVec3d uorYVec;
    cloudTransform.multiplyMatrixOnly(&uorYVec, &localClipBox.GetYVec ());
    DVec3d uorZVec;
    cloudTransform.multiplyMatrixOnly(&uorZVec, &localClipBox.GetZVec ());

    clipBox = OrientedBox(uorXVec, uorYVec, uorZVec, uorOrigin);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::SetClipBoundary(PointCloudPropertiesCR props, OrientedBoxCR clipBox)
    {
    ClearClipBoundary ();
    Transform cloudTransform(props.ComputeCloudTransform());

    // convert UOR to local coordinates
    Transform invCloudTrn;
    invCloudTrn.inverseOf(&cloudTransform);

    DPoint3d localOrigin;
    invCloudTrn.multiply(&localOrigin, &clipBox.GetOrigin (), 1);
    DVec3d localXVec;
    invCloudTrn.multiplyMatrixOnly(&localXVec, &clipBox.GetXVec ());
    DVec3d localYVec;
    invCloudTrn.multiplyMatrixOnly(&localYVec, &clipBox.GetYVec ());
    DVec3d localZVec;
    invCloudTrn.multiplyMatrixOnly(&localZVec, &clipBox.GetZVec ());

    AddClipBoundary(OrientedBox(localXVec, localYVec, localZVec, localOrigin));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanielMcKenzie  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::SetClipBoundary (PointCloudPropertiesCR props, bvector<DPoint3d> const& polygon)
    {
    Transform invCloudTrn;
    size_t size;
    DPoint3d localPoints;
    bvector<DPoint3d> transformedPoints;

    ClearClipBoundaryPolygon ();
    Transform cloudTransform(props.ComputeCloudTransform());

    // convert UOR to local coordinates
    invCloudTrn.inverseOf(&cloudTransform);
    
    size = polygon.size();

    for (size_t i = 0; i < size; i++)
        {
        invCloudTrn.multiply(&localPoints, &polygon.at(i), 1);
        transformedPoints.push_back(localPoints);
        }

    AddClipBoundaryPolygon(transformedPoints);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::AddClipBoundary(OrientedBox const& clipBox)
    {
    m_boundary.push_back(clipBox);
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::ClearClipBoundary ()
    {
    m_boundary.clear();
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::AddClipMask(OrientedBox const& clipBox)
    {
    m_mask.push_back(clipBox);
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::ClearClipMask ()
    {
    m_mask.clear();
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
OrientedBoxListCR PointCloudClipProperties::GetClipMask() const
    {
    return m_mask;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::AddClipBoundaryPolygon(bvector<DPoint3d> const& pointsPolygon)
    {
    m_boundaryPolygon.push_back(pointsPolygon);
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::AddClipMaskPolygon(bvector<DPoint3d> const& pointsPolygon)
    {
    m_maskPolygon.push_back(pointsPolygon);
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::ClearClipBoundaryPolygon ()
    {
    m_boundaryPolygon.clear();
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::ClearClipMaskPolygon ()
    {
    m_maskPolygon.clear();
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::GetClipBoundaryPolygon(PointCloudPropertiesCR props, bvector<DPoint3d>& clipPolygon) const
    {
    if (m_boundaryPolygon.size() == 0)
        // No clip polygon.
        return ERROR;

    Transform cloudTransform(props.ComputeCloudTransform());

    bvector<bvector<DPoint3d> > const& clips(m_boundaryPolygon);
    
    bvector<DPoint3d> localClipPolygon = clips.front();
    DPoint3d localPoints;

    size_t numPoint = localClipPolygon.size();
    for (size_t i = 0; i < numPoint; i++)
        {
        cloudTransform.multiply(&localPoints, &localClipPolygon.at(i), 1);
        clipPolygon.push_back(localPoints);
        }
   
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<bvector<DPoint3d> > const& PointCloudClipProperties::GetClipMaskPolygon() const
    {
    return m_maskPolygon;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::GetClipMaskList (PointCloudPropertiesCR props, OrientedBoxListR clipBoxList) const
    {
    OrientedBoxListCR masks = GetClipMask ();

    Transform cloudTransform(props.ComputeCloudTransform());

    for (OrientedBoxList::const_iterator itr(masks.begin()); itr != masks.end(); ++itr)
        {
        //  convert from local coordinates to UOR
        OrientedBox localClipBox = *itr;
        DPoint3d uorOrigin;
        cloudTransform.multiply(&uorOrigin, &localClipBox.GetOrigin (), 1);
        DVec3d uorXVec;
        cloudTransform.multiplyMatrixOnly(&uorXVec, &localClipBox.GetXVec ());
        DVec3d uorYVec;
        cloudTransform.multiplyMatrixOnly(&uorYVec, &localClipBox.GetYVec ());
        DVec3d uorZVec;
        cloudTransform.multiplyMatrixOnly(&uorZVec, &localClipBox.GetZVec ());

        clipBoxList.push_back (OrientedBox(uorXVec, uorYVec, uorZVec, uorOrigin));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanielMcKenzie  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::GetClipMaskList (PointCloudPropertiesCR props, bvector<bvector<DPoint3d> >& clipPolygonList) const
    {
    bvector<bvector<DPoint3d> > const& masks = GetClipMaskPolygon ();
    Transform cloudTransform(props.ComputeCloudTransform());

    for (bvector<bvector<DPoint3d> >::const_iterator itr(masks.begin()); itr != masks.end(); ++itr)
        {
        bvector<DPoint3d> localClipBox = *itr;
        size_t size = localClipBox.size();
        DPoint3d localPoints;
        bvector<DPoint3d> transformedPoint;

        for (size_t i = 0; i < size; i++)
            {
            cloudTransform.multiply(&localPoints, &localClipBox.at(i), 1);
            transformedPoint.push_back(localPoints);
            }

        clipPolygonList.push_back (transformedPoint);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::SetClipMaskList (PointCloudPropertiesCR props, OrientedBoxListCR clipBoxList)
    {
    ClearClipMask ();

    Transform cloudTransform(props.ComputeCloudTransform());
    // convert from local coordinates to UOR
    Transform invCloudTrn;
    invCloudTrn.inverseOf(&cloudTransform);

    for (OrientedBoxList::const_iterator itr(clipBoxList.begin()); itr != clipBoxList.end(); ++itr)
        {
        // convert from UOR to local coordinates
        OrientedBox uorClipBox = *itr;
        DPoint3d localOrigin;
        invCloudTrn.multiply(&localOrigin, &uorClipBox.GetOrigin (), 1);
        DVec3d localXVec;
        invCloudTrn.multiplyMatrixOnly(&localXVec, &uorClipBox.GetXVec ());
        DVec3d localYVec;
        invCloudTrn.multiplyMatrixOnly(&localYVec, &uorClipBox.GetYVec ());
        DVec3d localZVec;
        invCloudTrn.multiplyMatrixOnly(&localZVec, &uorClipBox.GetZVec ());

        AddClipMask (OrientedBox(localXVec, localYVec, localZVec, localOrigin));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanielMcKenzie  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::SetClipMaskList (PointCloudPropertiesCR props, bvector<bvector<DPoint3d> > const& clipPolygonList)
    {
    DPoint3d localPoint;
    size_t size;
    bvector<DPoint3d> transformedPoints;
    ClearClipMaskPolygon ();

    Transform cloudTransform(props.ComputeCloudTransform());

    // convert UOR to local coordinates
    Transform invCloudTrn;
    invCloudTrn.inverseOf(&cloudTransform);


    for (bvector<bvector<DPoint3d> >::const_iterator itr(clipPolygonList.begin()); itr != clipPolygonList.end(); ++itr)
        {
        // convert from UOR to local coordinates
        bvector<DPoint3d> ClipPolygon = *itr;
        size = ClipPolygon.size();
        
        for (size_t i =0; i < size; i++)
            {
            invCloudTrn.multiply(&localPoint, &ClipPolygon.at(i), 1);
            transformedPoints.push_back(localPoint);
            }
	
        AddClipMaskPolygon (transformedPoints);

        transformedPoints.clear();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::ClearAllClip ()
    {
    ClearClipMask ();
    ClearClipBoundary ();
    ClearClipMaskPolygon();
    ClearClipBoundaryPolygon();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Method that read PointCloudClipProperties XAttribute.
* Format:
*       for all clips
*           clipFormat   [DataType]   --> Actually ClipBox
*           clipSize     [UInt32]   --> Data size
*           clipBox      [ClipBox]  --> Clip definition
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::LoadFromElement(ElementHandleCR eh)
    {
    if(eh.GetLegacyType() != DgnPlatform::EXTENDED_ELM)
        return ERROR;

    ElementHandle::XAttributeIter xAttrItr(eh, XAttributeHandlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_ClipAttributes));

    return LoadFromXAttribute(xAttrItr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::LoadFromXAttribute(ElementHandle::XAttributeIter const& xAttr)
    {
    if(!xAttr.IsValid() || xAttr.GetSize () == 0)
        return ERROR;

    DataInternalizer dataInternalizer ((byte*)xAttr.PeekData(), xAttr.GetSize());
    return Load(dataInternalizer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::Load(DataInternalizer& dataInternalizer)
    {
    // Clear internal state
    Clear();

    // --- get clips ---
    while(!dataInternalizer.AtOrBeyondEOS())
        {
        // --- Get clip format ---
        PointCloudAttributeBase::DataType clipFormat = 0;
        dataInternalizer.get (&clipFormat);

        // --- Get data size ---
        UInt32 dataSize;
        dataInternalizer.get (&dataSize);

        
        if (clipFormat == PointCloudClipProperties::Format_ClipBoxBoundary)
            {
            // --- Get clip box ---
            OrientedBox box(LoadClipBox(dataInternalizer));
            m_boundary.push_back(box);
            }
        else if (clipFormat == PointCloudClipProperties::Format_ClipPolygonBoundary)
            {
            bvector<DPoint3d> polygon; 
            LoadClipPolygon(dataInternalizer, polygon);
            m_boundaryPolygon.push_back(polygon);
            }
        else if (clipFormat == PointCloudClipProperties::Format_ClipPolygonMask)
            {
            bvector<DPoint3d> polygon; 
            LoadClipPolygon(dataInternalizer, polygon);
            m_maskPolygon.push_back(polygon);
            }
        else if (clipFormat == PointCloudClipProperties::Format_ClipBoxMask)
            {
            // --- Get clip box ---
            OrientedBox box(LoadClipBox(dataInternalizer));

            m_mask.push_back(box);
            }
        else
            {
            // NEEDSWORK: &&SP - Should we preserve unknown blocks?
            dataInternalizer.skip(dataSize);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
OrientedBox PointCloudClipProperties::LoadClipBox(DataInternalizer& dataInternalizer)
    {
    DVec3d xVec, yVec, zVec, origin;
    dataInternalizer.get ((double*)&xVec, 3);
    dataInternalizer.get ((double*)&yVec, 3);
    dataInternalizer.get ((double*)&zVec, 3);
    dataInternalizer.get ((double*)&origin, 3);

    OrientedBox box(xVec, yVec, zVec, origin);
    return box;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClipProperties::StoreClipBox(OrientedBox const& box, DataExternalizer& dataExternalizer)
    {
    dataExternalizer.put((double*)&box.GetXVec(), 3);
    dataExternalizer.put((double*)&box.GetYVec(), 3);
    dataExternalizer.put((double*)&box.GetZVec(), 3);
    dataExternalizer.put((double*)&box.GetOrigin(), 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanielMcKenzie  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClipProperties::LoadClipPolygon(DataInternalizer& dataInternalizer, bvector<DPoint3d>& polygon)
    {  
    int numpoint;
    dataInternalizer.get ((int*)&numpoint, 1);

    DPoint3d points;

    for (int i = 0; i < numpoint; i++)
        {
        dataInternalizer.get ((double*)&points, 3); 
        polygon.push_back(points);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClipProperties::StoreClipPolygon(bvector<DPoint3d> const& polygon, DataExternalizer& dataExternalizer)
    {
    int size = (int)polygon.size();
    dataExternalizer.put(size);

    for (int i = 0; i < size; i++)
        dataExternalizer.put((double*)&polygon.at(i), 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudClipProperties::StoreToElement(EditElementHandleR eeh) const
    {
    if(eeh.GetLegacyType() != DgnPlatform::EXTENDED_ELM)
        return ERROR;

    DataExternalizer dataExternalizer;

    // --- set all clip box boundary ---
    for (OrientedBoxList::const_iterator itr(m_boundary.begin()); itr != m_boundary.end(); ++itr)
        {
        // --- Set clip format ---
        dataExternalizer.put((PointCloudAttributeBase::DataType)PointCloudClipProperties::Format_ClipBoxBoundary);

        // Write clip data in a separate buffer so we can report the size in the "main" dataExternalizer.
        DataExternalizer clipExternalizer;

        // --- Set clip box ---
        StoreClipBox(*itr, clipExternalizer);

        // Clip data size + data.
        dataExternalizer.put((UInt32)clipExternalizer.getBytesWritten());
        dataExternalizer.put((UInt8*)clipExternalizer.getBuf(), clipExternalizer.getBytesWritten());
        }

    // --- set all clip polygon boundary ---
    for (bvector<bvector<DPoint3d> >::const_iterator itr(m_boundaryPolygon.begin()); itr != m_boundaryPolygon.end(); ++itr)
        {
        // --- Set clip format ---
        dataExternalizer.put((PointCloudAttributeBase::DataType)PointCloudClipProperties::Format_ClipPolygonBoundary);

        // Write clip data in a separate buffer so we can report the size in the "main" dataExternalizer.
        DataExternalizer clipExternalizer;

        // --- Set clip box ---
        StoreClipPolygon(*itr, clipExternalizer);

        // Clip data size + data.
        dataExternalizer.put((UInt32)clipExternalizer.getBytesWritten());
        dataExternalizer.put((UInt8*)clipExternalizer.getBuf(), clipExternalizer.getBytesWritten());
        }

    // --- set all clip mask ---
    for (OrientedBoxList::const_iterator itr(m_mask.begin()); itr != m_mask.end(); ++itr)
        {
        // --- Set clip format ---
        dataExternalizer.put((PointCloudAttributeBase::DataType)PointCloudClipProperties::Format_ClipBoxMask);

        // Write clip data in a separate buffer so we can report the size in the "main" dataExternalizer.
        DataExternalizer clipExternalizer;

        // --- Set clip box ---
        StoreClipBox(*itr, clipExternalizer);

        // Clip data size + data.
        dataExternalizer.put((UInt32)clipExternalizer.getBytesWritten());
        dataExternalizer.put((UInt8*)clipExternalizer.getBuf(), clipExternalizer.getBytesWritten());
        }
    
        // --- set all clip mask ---
    for (bvector<bvector<DPoint3d> >::const_iterator itr(m_maskPolygon.begin()); itr != m_maskPolygon.end(); ++itr)
        {
        // --- Set clip format ---
        dataExternalizer.put((PointCloudAttributeBase::DataType)PointCloudClipProperties::Format_ClipPolygonMask);

        // Write clip data in a separate buffer so we can report the size in the "main" dataExternalizer.
        DataExternalizer clipExternalizer;

        StoreClipPolygon(*itr, clipExternalizer);

        // Clip data size + data.
        dataExternalizer.put((UInt32)clipExternalizer.getBytesWritten());
        dataExternalizer.put((UInt8*)clipExternalizer.getBuf(), clipExternalizer.getBytesWritten());
        }

    // Tell the element handle to add this XAttribute when rewrite is called.
    size_t nbBytesWritten = dataExternalizer.getBytesWritten();
    if (nbBytesWritten == 0)
        {
        // There are no clip properties anymore. Delete XAttributes for Clip properties.
        eeh.ScheduleDeleteXAttribute (XAttributeHandlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_ClipAttributes), 0);

        // Recompute element range
        eeh.GetDisplayHandler()->ValidateElementRange(eeh);
        return BSISUCCESS;
        }

    return eeh.ScheduleWriteXAttribute (XAttributeHandlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_ClipAttributes), 0, nbBytesWritten, dataExternalizer.getBuf());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClipProperties::Clear()
    {
    ClearAllClip();
    }
