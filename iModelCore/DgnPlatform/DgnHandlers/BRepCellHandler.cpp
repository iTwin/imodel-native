/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/BRepCellHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <RmgrTools/RscMgr/bnryport.h>

#define     EMBBREP_VERSION         2

#define     CELLNAME_Solid          L"Smart Solid"
#define     CELLNAME_Surface        L"Smart Surface"

#define     LINKAGEID_Wireframe     20371 /* 0x4f93 */

static UInt8 s_embeddedBRepLinkageConvRulesCompiled[] = 
    {
0x70,0x6F,0x76,0x63,0x98,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x8,0x0,0x0,0x0,0x1,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,0x0,0x0,0x0,0x15,0x0,0x0,0x0,0x8,0x0,0x0,
0x0,0x1,0x0,0x0,0x0,0xD,0xF0,0xAD,0xBA,0xD,0xF0,0xAD,0xBA,0x15,0x0,0x0,0x0,0x4,
0x0,0x0,0x0,0x1,0x0,0x0,0x0,0xD,0xF0,0xAD,0xBA,0xD,0xF0,0xAD,0xBA,0x5,0x0,0x0,0x0,
0x4,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x16,0x0,0x0,0x0,
0x4,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0xD,0xF0,0xAD,0xBA,0xD,0xF0,0xAD,0xBA,0xC,0x0,
0x0,0x0,0x8,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x16,0x0,
0x0,0x0,0x8,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0xD,0xF0,0xAD,0xBA,0xD,0xF0,0xAD,0xBA
    };
static void *embeddedBRepLinkageConvRulesP = s_embeddedBRepLinkageConvRulesCompiled;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct  embeddedBRepLinkageData
    {
    struct
        {
        UInt32      dirty:1;
        UInt32      version:15;
        UInt32      faceHasMat:1;
        UInt32      kernel:4;
        UInt32      unused:11;

        } flags;

    double      scale;

    } EmbeddedBRepLinkageData;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct  embeddedBRepLinkage
    {
    LinkageHeader           header;
    EmbeddedBRepLinkageData data;
    short                   padding[4];

    } EmbeddedBRepLinkage;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static UShort*  extractBRepDataLinkage (EmbeddedBRepLinkage& linkage, ElementHandleCR eh)
    {
    return (UShort*) linkage_extractFromElement (&linkage, eh.GetElementCP (), LINKAGEID_EmbeddedBRep, embeddedBRepLinkageConvRulesP, 0L, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings 04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt appendBRepDataLinkage (EditElementHandleR eeh, ISolidKernelEntity::SolidKernelType kernel, bool hasFaceMats)
    {
    UShort*             linkP;
    EmbeddedBRepLinkage linkage;

    if (NULL != (linkP = extractBRepDataLinkage (linkage, eeh)))
        {
        linkage.data.flags.version    = EMBBREP_VERSION;
        linkage.data.flags.kernel     = kernel;
        linkage.data.flags.faceHasMat = hasFaceMats;
        linkage.data.flags.dirty      = false;
        linkage.data.scale            = 0.0; // Always reset body scale

        return mdlCnv_bufferToFileFormatWithRules ((byte *) (linkP + 2), NULL, embeddedBRepLinkageConvRulesP, (byte *) &linkage.data, sizeof (linkage));
        }

    LinkageHeader   linkHeader;

    memset (&linkHeader, 0, sizeof (linkHeader));

    linkHeader.primaryID = LINKAGEID_EmbeddedBRep;
    linkHeader.user      = true;

    EmbeddedBRepLinkageData linkageData;

    memset (&linkageData, 0, sizeof (linkageData));

    linkageData.flags.version    = EMBBREP_VERSION;
    linkageData.flags.kernel     = kernel;
    linkageData.flags.faceHasMat = hasFaceMats;

    DgnV8ElementBlank   el;

    eeh.GetElementCP ()->CopyTo (el);

    if (SUCCESS != linkage_appendToElement (&el, &linkHeader, &linkageData, embeddedBRepLinkageConvRulesP))
        return ERROR;

    eeh.ReplaceElement (&el);

    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MaterialFaceAttachmentQuery : IQueryProperties
{
private:

bool m_hasAttachment;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialFaceAttachmentQuery () {m_hasAttachment = false;}
bool GetHasFaceAttachment () {return m_hasAttachment;}
virtual ElementProperties _GetQueryPropertiesMask () override {return ELEMENT_PROPERTY_Material;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _EachMaterialCallback (EachMaterialArg& arg) override
    {
    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return;

    if (arg.GetStoredValue ().IsValid ())
        m_hasAttachment = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/11
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasFaceMaterialAttachment (EditElementHandleR eeh, ISolidKernelEntityCR entity)
    {
    MaterialFaceAttachmentQuery remapper;
    PropertyContext context (&remapper);
    context.SetCurrentElemHandleP (&eeh);

    T_HOST.GetSolidsKernelAdmin ()._ProcessProperties (const_cast <ISolidKernelEntityR> (entity), context);

    return remapper.GetHasFaceAttachment ();
    }

}; // MaterialFaceAttachmentQuery

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EdgeCollector : IElementGraphicsProcessor
{
protected:

ViewContextP        m_context;
Transform           m_currentTransform;
ElementAgendaP      m_agenda;
bool                m_addWireframeId;
bool                m_setSymbology;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
explicit EdgeCollector (ElementAgendaR agenda, bool addWireframeId, bool setSymbology) {m_agenda = &agenda; m_addWireframeId = addWireframeId; m_setSymbology = setSymbology;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessAsBody (bool isCurved) const override
    {
    return false; // Output only edges...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceContext (ViewContextR context) override
    {
    m_context = &context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceTransform (TransformCP trans) override
    {
    if (trans)
        m_currentTransform = *trans;
    else
        m_currentTransform.initIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override
    {
    BeAssert (NULL != m_context->GetCurrentElement ());

    CurveVectorPtr tmpCurves = curves.Clone ();

    if (!m_currentTransform.IsIdentity ())
        tmpCurves->TransformInPlace (m_currentTransform);

    EditElementHandle       eeh;
    CurvePrimitiveIdCP      primitiveId;
    CurveTopologyId         topologyId;

    // NOTE: Always create 3d elements, don't use dimension of current model...
    if (SUCCESS != DraftingElementSchema::ToElement (eeh, *tmpCurves, NULL, true, *m_context->GetCurrentElement ()->GetDgnModelP ()) || !eeh.IsValid ())
        return ERROR;

    if (m_addWireframeId && 
        NULL != (primitiveId = curves.front()->GetId()) &&
        SUCCESS == primitiveId->GetParasolidBodyId (topologyId))
        {
        FaceId           faceIds[2];
        size_t           isolineIndex;
        DataExternalizer writer;

        memset (faceIds, 0, sizeof (faceIds));

        if (SUCCESS == topologyId.GetBRepSharedEdge (&faceIds[0], &faceIds[1]))
            {
            writer.put (faceIds[0].nodeId);
            writer.put (faceIds[0].entityId);
            writer.put (faceIds[1].nodeId);
            writer.put (faceIds[1].entityId);

            ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_Wireframe, writer); 
            }
        else if (SUCCESS == topologyId.GetBRepSheetEdge (&faceIds[0]))
            {
            writer.put (faceIds[0].nodeId);
            writer.put (faceIds[0].entityId);
            writer.put (faceIds[0].nodeId); // Face 0 and 1 set the same for a sheet edge id...
            writer.put (faceIds[0].entityId);

            ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_Wireframe, writer); 
            }
        else if (SUCCESS == topologyId.GetBRepIsoline (&faceIds[0], &isolineIndex))
            {
            writer.put (faceIds[0].nodeId);
            writer.put (faceIds[0].entityId);
            writer.put ((UInt32) 0);
            writer.put ((UInt32) isolineIndex); // Legacy Isoline index stored in place of second entityId...

            ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_Wireframe, writer); 
            }
        }

    if (m_setSymbology)
        ElementPropertiesSetter::ApplyElemDisplayParams (eeh, *m_context->GetCurrentDisplayParams ());

    m_agenda->Insert (eeh);

    return SUCCESS;
    }

}; // EdgeCollector

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FaceCollector : IElementGraphicsProcessor
{
protected:

ViewContextP        m_context;
Transform           m_currentTransform;
ElementAgendaP      m_agenda;
bool                m_inBRepDraw;
bool                m_setSymbology;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
explicit FaceCollector (ElementAgendaR agenda, bool setSymbology) {m_agenda = &agenda; m_setSymbology = setSymbology; m_inBRepDraw = false;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessAsBody (bool isCurved) const override
    {
    return true; // Output brep data...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceContext (ViewContextR context) override
    {
    m_context = &context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceTransform (TransformCP trans) override
    {
    if (trans)
        m_currentTransform = *trans;
    else
        m_currentTransform.initIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessSurface (MSBsplineSurfaceCR surface) override
    {
    BeAssert (NULL != m_context->GetCurrentElement ());

    MSBsplineSurfacePtr  tmpSurface = MSBsplineSurface::CreatePtr ();

    tmpSurface->CopyFrom (surface);

    if (!m_currentTransform.IsIdentity ())
        tmpSurface->TransformSurface (&m_currentTransform);

    EditElementHandle   eeh;

    if (BSPLINE_STATUS_Success != BSplineSurfaceHandler::CreateBSplineSurfaceElement (eeh, NULL, *tmpSurface, *m_context->GetCurrentElement ()->GetDgnModelP ()))
        return ERROR;

    if (m_setSymbology)
        ElementPropertiesSetter::ApplyElemDisplayParams (eeh, *m_context->GetCurrentDisplayParams ());

    m_agenda->Insert (eeh);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override
    {
    if (!curves.IsAnyRegionType ())
        {
        BeAssert (false); // _OutputBodyAsSurfaces error...valid planar face output is always closed...

        return SUCCESS;
        }

    BeAssert (NULL != m_context->GetCurrentElement ());

    CurveVectorPtr tmpCurves = curves.Clone ();

    if (!m_currentTransform.IsIdentity ())
        tmpCurves->TransformInPlace (m_currentTransform);

    EditElementHandle  eeh;

    // NOTE: Always create 3d elements, don't use dimension of current model...
    if (SUCCESS != DraftingElementSchema::ToElement (eeh, *tmpCurves, NULL, true, *m_context->GetCurrentElement ()->GetDgnModelP ()))
        return ERROR;

    if (m_setSymbology)
        ElementPropertiesSetter::ApplyElemDisplayParams (eeh, *m_context->GetCurrentDisplayParams ());

    m_agenda->Insert (eeh);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessSolidPrimitive (ISolidPrimitiveCR primitive) override
    {
    BeAssert (NULL != m_context->GetCurrentElement ());

    ISolidPrimitivePtr geomPtr = primitive.Clone ();

    if (!m_currentTransform.IsIdentity ())
        geomPtr->TransformInPlace (m_currentTransform);

    EditElementHandle  eeh;

    if (SUCCESS != DraftingElementSchema::ToElement (eeh, *geomPtr, NULL, *m_context->GetCurrentElement ()->GetDgnModelP ()))
        return ERROR;

    if (m_setSymbology)
        ElementPropertiesSetter::ApplyElemDisplayParams (eeh, *m_context->GetCurrentDisplayParams ());

    m_agenda->Insert (eeh);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP attachments) override
    {
    if (m_inBRepDraw)
        {
        BeAssert (false); // _OutputBodyAsSurfaces error...valid output is simple shapes/surfaces...

        return SUCCESS;
        }

    AutoRestore <bool> saveInBRep (&m_inBRepDraw, true);

    // Ask solid admin to output this body as surface geometry...
    DgnPlatformLib::Host::SolidsKernelAdmin& solidAdmin = T_HOST.GetSolidsKernelAdmin ();

    return solidAdmin._OutputBodyAsSurfaces (entity, *m_context, true, attachments);
    }

}; // FaceCollector

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            BrepCellHeaderHandler::_GetTypeName (WStringR descr, UInt32 desiredLength)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_BRep));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BrepCellHeaderHandler::_GetDescription (ElementHandleCR eh, WStringR descr, UInt32 desiredLength)
    {
    /* NOTE: Guess if smart solid or surface by checking cell name. The cells we create
             always use this naming convention. Sub-classes can implement this to check
             whatever they want. Getting an element descriptor and extracting body data to 
             be 100% correct was horrible can't be calling mdlKISolid_isSmartSolidElement. */
    WChar     cellName[MAX_CELLNAME_LENGTH];

    CellUtil::ExtractName (cellName, MAX_CELLNAME_LENGTH, eh);

    if (0 == BeStringUtilities::Wcsicmp (cellName, CELLNAME_Solid))
        {
        descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_SmartSolid));
        }
    else if (0 == BeStringUtilities::Wcsicmp (cellName, CELLNAME_Surface))
        {
        descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_SmartSurface));
        }
    else
        {
        _GetTypeName (descr, desiredLength);

        if (wcslen (cellName) > 0)
            descr.append (L": ").append (cellName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BrepCellHeaderHandler::_IsTransformGraphics (ElementHandleCR elemHandle, TransformInfoCR trans)
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            BrepCellHeaderHandler::_GetTransformOrigin (ElementHandleCR elHandle, DPoint3dR origin)
    {
    // Cell origin is only meaningful for user-defined cells...use centroid for orphan cells (el->IsHole())
    return GetRangeCenter (elHandle, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BrepCellHeaderHandler::AdjustBRepDataScale
(
EditElementHandleR eeh,
double          scale,
bool            allowBRepLinkageScale
)
    {
    if (1.0 == scale && allowBRepLinkageScale)
        return false;

    if (CELL_HEADER_ELM != eeh.GetLegacyType() && SHAREDCELL_DEF_ELM != eeh.GetLegacyType())
        return false;

    double      currentValue;

    if (SUCCESS != BrepCellHeaderHandler::GetBRepDataScale (currentValue, eeh))
        return false;

    bool        changed = false;
    double      newValue = currentValue ? currentValue * scale : scale;

    if (allowBRepLinkageScale)
        {
        changed = (SUCCESS == BrepCellHeaderHandler::SetBRepDataScale (eeh, newValue) ? true : false);
        }
    else if (!mdlElement_attributePresent (eeh.GetElementCP (), LINKAGEID_Feature, NULL))
        {
        if (CELL_HEADER_ELM == eeh.GetLegacyType())
            changed = (SUCCESS == NormalCellHeaderHandler::AdjustScale (eeh, newValue) ? true : false);
        else if (SHAREDCELL_DEF_ELM == eeh.GetLegacyType())
            changed = (SUCCESS == SharedCellDefHandler::AdjustScale (eeh, newValue) ? true : false);

        if (changed)
            BrepCellHeaderHandler::SetBRepDataScale (eeh, 0.0);
        }

    if (changed)
        eeh.GetElementP()->SetLastModifiedTime(0.0); // Body cache entry no longer valid...

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BrepCellHeaderHandler::_OnChangeOfUnits
(
EditElementHandleR eeh,
DgnModelP    sourceDgnModel,
DgnModelP    destDgnModel
)
    {
    BrepCellHeaderHandler::AdjustBRepDataScale (eeh, sourceDgnModel->GetBRepScaleToDestination (*destDgnModel), true);

    // Allow child geometry elements to adjust units, if necessary.
    return T_Super::_OnChangeOfUnits (eeh, sourceDgnModel, destDgnModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void     processMaterialAttachmentsToFaces (ElementHandleCR eh, EditElementHandleP eeh, PropertyContextR context)
    {
    if (0 == (ELEMENT_PROPERTY_Material & context.GetElementPropertiesMask ()))
        return;

    if (!BrepCellHeaderHandler::HasMaterialAttachmentToFace (eh))
        return;

    IBRepQuery* brepQuery = dynamic_cast <IBRepQuery*> (&eh.GetHandler ());

    if (NULL == brepQuery)
        return;

    ISolidKernelEntityPtr   entityPtr;

    if (SUCCESS != brepQuery->GetBRepDataEntity (eh, entityPtr, false))
        return;

    DgnPlatformLib::Host::SolidsKernelAdmin& solidAdmin = T_HOST.GetSolidsKernelAdmin ();

    if (!solidAdmin._ProcessProperties (*entityPtr, context) || !eeh)
        return;

    // Material names have been remaped, replace DgnStore component...
    BrepCellHeaderHandler::UpdateBRepDataDgnStore (*eeh, *entityPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            BrepCellHeaderHandler::_QueryHeaderProperties (ElementHandleCR eh, PropertyContextR context)
    {
    T_Super::_QueryHeaderProperties (eh, context);

    if (QueryPropertyPurpose::Match == context.GetIQueryPropertiesP ()->_GetQueryPropertiesPurpose ())
        return;

    processMaterialAttachmentsToFaces (eh, NULL, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            BrepCellHeaderHandler::_EditHeaderProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditHeaderProperties (eeh, context);

    processMaterialAttachmentsToFaces (eeh, &eeh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            BrepCellHeaderHandler::_GetElemDisplayParams (ElementHandleCR elHandle, ElemDisplayParams& params, bool wantMaterials)
    {
    ElementHandle  templateElHandle;

    if (GetComponentForDisplayParams (templateElHandle, elHandle) && CELL_HEADER_ELM != templateElHandle.GetLegacyType())
        {
        templateElHandle.GetDisplayHandler()->GetElemDisplayParams (templateElHandle, params, wantMaterials);

        #ifdef WIP_VANCOUVER_MERGE // materials
        if (wantMaterials)
            {
            if (NULL == params.GetMaterial ()) 
                params.SetMaterial (MaterialManager::GetManagerR ().FindMaterialAttachment (NULL, elHandle, *elHandle.GetDgnModelP (), false), true);// look for attachment to top level element if none found yet
            if (params.GetMaterialUVDetailP ()) // uv mapping for cubic etc also uses top level element
                params.GetMaterialUVDetailP ()->SetElementHandle (elHandle);
            }
        #endif
        params.SetIsRenderable (true);
        return;
        }

    T_Super::_GetElemDisplayParams (elHandle, params, wantMaterials);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BrepCellHeaderHandler::_IsVisible (ElementHandleCR elHandle, ViewContextR context, bool testRange, bool testLevel, bool testClass)
    {
    if (testLevel)
        {
        ElementHandle  templateElHandle;

        if (GetComponentForDisplayParams (templateElHandle, elHandle) && CELL_HEADER_ELM != templateElHandle.GetLegacyType())
            if (!templateElHandle.GetDisplayHandler()->IsVisible (templateElHandle, context, false, true, false))
                return false;
        }

    return T_Super::_IsVisible (elHandle, context, testRange, false, testClass);
    }

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   09/03
+===============+===============+===============+===============+===============+======*/
struct          DrawBrep : IStrokeForCache
{
bool            m_facetsCreated;
bool            m_cacheIsElemSizeDependent;

DrawBrep ()
    {
    m_facetsCreated = true; // StrokeForcache won't be called for an existing qvElem!
    m_cacheIsElemSizeDependent = false;
    }

virtual bool        _GetSizeDependentGeometryPossible () override   {return true;}
virtual bool        _GetSizeDependentGeometryStroked () override    {return m_cacheIsElemSizeDependent;}
virtual DrawExpense _GetDrawExpense () override                     {return DrawExpense::High;}
virtual bool        _WantLocateByStroker () override                {return false;} // Don't call _StrokeForCache, locate interior by QvElem...

bool            CacheGeomFailure () {return !m_facetsCreated;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsFeatureSolid (ElementHandleCR eh)
    {
    return mdlElement_attributePresent (eh.GetElementCP (), LINKAGEID_Feature, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsMultiSymbologyBody (ElementHandleCR eh)
    {
    return IsFeatureSolid (eh) || BrepCellHeaderHandler::HasMaterialAttachmentToFace (eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    m_facetsCreated = false;

    if (dh.GetElementHandleCP() == NULL)
        return;

    ElementHandleCR eh = *dh.GetElementHandleCP();
    IBRepQuery* brepQuery = dynamic_cast <IBRepQuery*> (&eh.GetHandler ());

    if (NULL == brepQuery)
        return;

    ISolidKernelEntityPtr   entityPtr;

    if (SUCCESS != brepQuery->GetBRepDataEntity (eh, entityPtr, true))
        return;

    DgnPlatformLib::Host::SolidsKernelAdmin& solidsAdmin = T_HOST.GetSolidsKernelAdmin ();

    // If all planar faces/edges, can use this cache representation for any zoom level...
    if (solidsAdmin._QueryEntityData (*entityPtr, ISolidKernelEntity::EntityQuery_HasCurvedFaceOrEdge))
        m_cacheIsElemSizeDependent = true;

    ElemDisplayParams   baseParams;

    eh.GetDisplayHandler ()->GetElemDisplayParams (eh, baseParams, context.GetWantMaterials ()); // Get natural display params from element, current is cooked...

    IFaceMaterialAttachmentsPtr attachments = solidsAdmin._GetFaceMaterialAttachments (*entityPtr, context, baseParams, 0, &eh);

    m_facetsCreated = (SUCCESS == context.GetIDrawGeom ().DrawBody (*entityPtr, attachments.get (), m_cacheIsElemSizeDependent ? pixelSize : 0.0));
    }

}; // DrawBrep

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            BrepCellHeaderHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    // NOTE: Don't create expensive cached representation just to erase...rely on healing!
    if (DrawPurpose::ChangedPre == context.GetDrawPurpose ())
        {
        context.DrawElementRange (thisElm.GetElementCP ());

        return;
        }

    UInt32      info = context.GetDisplayInfo (IsRenderable (thisElm));
    QvElemP     qvElem = NULL;

    // Always draw brep when output is QVis or pick, want to see/locate silhouettes in wireframe...
    if (0 != (info & DISPLAY_INFO_Surface) || context.GetIViewDraw().IsOutputQuickVision () || DrawPurpose::Pick == context.GetDrawPurpose ())
        {
        DrawBrep  stroker;

        qvElem = context.DrawCached (thisElm, stroker, 0);

        // Make sure we display something, in the case of a facetting failure display edges...
        if (stroker.CacheGeomFailure ())
            info = DISPLAY_INFO_Edge;
        }

    if (0 != (info & DISPLAY_INFO_Edge))
        {
        // Draw edge and face iso geometry for wireframe display...
        T_Super::_Draw (thisElm, context);
        }

    _DrawDecorations (thisElm, context, info, qvElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool BrepCellHeaderHandler::_ClaimElement (ElementHandleCR thisElm)
    {
    return mdlElement_attributePresent (thisElm.GetElementCP (), LINKAGEID_EmbeddedBRep, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            BrepCellHeaderHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    EditElementHandle  cellEeh;

    NormalCellHeaderHandler::CreateOrphanCellElement (cellEeh, L"From Smart Solid", true, *eeh.GetDgnModelP ());

    for (ChildElemIter childEh (eeh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
        {
        switch (childEh.GetLegacyType())
            {
            case DGNSTORE_HDR:
            case DGNSTORE_COMP:
                break; // Strip brep data...

            default:
                {
                MSElementDescrPtr tmpEdP = childEh.GetElementDescrCP()->Duplicate();

                if (!tmpEdP.IsValid())
                    break;

                EditElementHandle tmpEeh (tmpEdP.get(), false);

                NormalCellHeaderHandler::AddChildElement (cellEeh, tmpEeh);
                break;
                }
            }
        }

    NormalCellHeaderHandler::AddChildComplete (cellEeh);
    eeh.ReplaceElementDescr (cellEeh.ExtractElementDescr().get());

    eeh.GetHandler().ConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Handler                                         Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BrepCellHeaderHandler::_OnFenceStretch
(
EditElementHandleR  eeh,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    ISolidKernelEntityPtr   entityPtr;

    if (SUCCESS != _GetBRepDataEntity (eeh, entityPtr, false))
        return ERROR;

    DgnPlatformLib::Host::SolidsKernelAdmin& solidAdmin = T_HOST.GetSolidsKernelAdmin ();

    if (SUCCESS != solidAdmin._FenceStretchBody (*entityPtr, transform, *fp))
        return ERROR;

    return BrepCellHeaderHandler::CreateBRepCellElement (eeh, &eeh, *entityPtr, *eeh.GetDgnModelP ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BrepCellHeaderHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Solids & geometry.GetOptions ()))
        return ERROR;

    ElementAgenda   brepGeom;

    // Ask element to draw and collect draw output as edges/faces...
    if (DropGeometry::SOLID_Surfaces == geometry.GetSolidsOptions ())
        {
        FaceCollector faces (brepGeom, true);
        ElementGraphicsOutput::Process (faces, eh);
        }
    else
        {
        EdgeCollector edges (brepGeom, false, true);
        ElementGraphicsOutput::Process (edges, eh);
        }

    if (0 == brepGeom.GetCount ())
        return ERROR;

    EditElementHandleP curr = brepGeom.GetFirstP ();
    EditElementHandleP end  = curr + brepGeom.GetCount ();

    for (; curr < end; curr++)
        dropGeom.Insert (*curr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BrepCellHeaderHandler::IsBRepDataValid (ElementHandleCR eh)
    {
    EmbeddedBRepLinkage linkage;
    
    if (NULL == extractBRepDataLinkage (linkage, eh))
        return false;

    return !linkage.data.flags.dirty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BrepCellHeaderHandler::HasMaterialAttachmentToFace (ElementHandleCR eh)
    {
    EmbeddedBRepLinkage linkage;
    
    if (NULL == extractBRepDataLinkage (linkage, eh))
        return false;

    return linkage.data.flags.faceHasMat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BrepCellHeaderHandler::UpdateBRepDataDgnStore (EditElementHandleR eeh, ISolidKernelEntityCR entity)
    {
    void*       dataP = NULL;
    UInt32      dataSize = 0;

    if (SUCCESS != T_HOST.GetSolidsKernelAdmin ()._SaveEntityToMemory (&dataP, dataSize, entity))
        return ERROR;

    // Make sure we have a valid brep linkage and also update face material attachment flag...
    appendBRepDataLinkage (eeh, ISolidKernelEntity::SolidKernel_PSolid, MaterialFaceAttachmentQuery::HasFaceMaterialAttachment (eeh, entity));

    // Apply body transform...need to account for body to uor scale...
    double      solidScale = IBRepQuery::GetSolidKernelToUORScale (eeh.GetDgnModelP ());
    double      invSolidScale = (0.0 == solidScale) ? 1.0 : (1.0 / solidScale);
    Transform   bodyTransform;

    bodyTransform.ScaleMatrixColumns (entity.GetEntityTransform (), invSolidScale, invSolidScale, invSolidScale);
    bodyTransform.GetTranslation (eeh.GetElementP ()->ToCell_3dR().origin);

    // Set body scale in brep linkage so that cell scale only reflects user scaling...NOTE: Resets cell scale to 1.0 (ok?)
    double      scale;
    DVec3d      xVec;
    RotMatrix   rMatrix;

    bodyTransform.GetMatrix (rMatrix);
    rMatrix.GetColumn (xVec, 0);
    scale = xVec.Magnitude ();
    rMatrix.ScaleColumns (1.0/scale, 1.0/scale, 1.0/scale);

    memcpy (eeh.GetElementP()->ToCell_3dR().transform, rMatrix.form3d, 9 * sizeof (double));
    SetBRepDataScale (eeh, scale);

    // Remove external brep dependencies or ExtractDgnStoreBRepData won't search for an internal brep...
    DependencyManagerLinkage::DeleteLinkage (eeh, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_BrepPartition);
    DependencyManagerLinkage::DeleteLinkage (eeh, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_BrepData);
    
    DgnStoreHdrHandler::RemoveFromCell (eeh, DGN_STORE_ID, EMBEDDED_BREP_ID);

    return DgnStoreHdrHandler::AppendToCell (eeh, dataP, dataSize, DGN_STORE_ID, EMBEDDED_BREP_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BrepCellHeaderHandler::GetBRepDataKernel (ISolidKernelEntity::SolidKernelType& kernel, ElementHandleCR eh)
    {
    EmbeddedBRepLinkage linkage;
    
    if (NULL == extractBRepDataLinkage (linkage, eh))
        return ERROR;

    kernel = (ISolidKernelEntity::SolidKernelType) linkage.data.flags.kernel;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BrepCellHeaderHandler::GetBRepDataVersion (int& version, ElementHandleCR eh)
    {
    EmbeddedBRepLinkage linkage;
    
    if (NULL == extractBRepDataLinkage (linkage, eh))
        return ERROR;

    version = linkage.data.flags.version;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BrepCellHeaderHandler::GetExternalBRepDataElementID (ElementId& elemId, ElementHandleCR eh)
    {
    DependencyLinkageAccessor  depLinkP;

        //*** WIP_DEPENDENCY - importer must remap this dependency linkage
    if (SUCCESS != DependencyManagerLinkage::GetLinkage (&depLinkP, eh, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_BrepData))
        return ERROR;

    elemId = ElementId(depLinkP->root.elemid[0]);
    return depLinkP->u.f.invalid ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BrepCellHeaderHandler::SetExternalBRepDataElementID (EditElementHandleR eeh, ElementId elemId)
    {
        //*** WIP_DEPENDENCY - importer must remap this dependency linkage
    DgnV8ElementBlank   el;

    eeh.GetElementCP ()->CopyTo (el);

    DependencyManagerLinkage::DeleteLinkageFromMSElement (&el, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_BrepData);

    if (SUCCESS != DependencyManagerLinkage::AppendSimpleLinkageToMSElement (&el, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_BrepData, DEPENDENCY_ON_COPY_DeepCopyRootsAcrossFiles, elemId.GetValue()))
        return ERROR;

    eeh.ReplaceElement (&el);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BrepCellHeaderHandler::GetBRepDataScale (double& scale, ElementHandleCR eh)
    {
    EmbeddedBRepLinkage linkage;
    
    if (NULL == extractBRepDataLinkage (linkage, eh))
        return ERROR;

    scale = linkage.data.scale;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BrepCellHeaderHandler::SetBRepDataScale (EditElementHandleR eeh, double scale)
    {
    UShort*             linkP;
    EmbeddedBRepLinkage linkage;
    
    if (NULL == (linkP = extractBRepDataLinkage (linkage, eeh)))
        return ERROR;

    linkage.data.scale = scale;

    return (BentleyStatus) mdlCnv_bufferToFileFormatWithRules ((byte *) (linkP + 2), NULL, embeddedBRepLinkageConvRulesP, (byte *) &linkage.data, sizeof (linkage));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BrepCellHeaderHandler::ExtractDgnStoreBRepData (void** dataPP, UInt32* dataSizeP, ElementHandleCR eh)
    {
    ElementId   elemId;
    
    if (SUCCESS == BrepCellHeaderHandler::GetExternalBRepDataElementID (elemId, eh))
        {
        ElementHandle  dgnStoreEh (elemId, *eh.GetDgnModelP());

        return DgnStoreHdrHandler::Extract (dataPP, dataSizeP, DGN_STORE_ID, EMBEDDED_BREP_ID, dgnStoreEh);
        }

    return DgnStoreHdrHandler::ExtractFromCell (dataPP, dataSizeP, DGN_STORE_ID, EMBEDDED_BREP_ID, eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isAcceptableLinkage (ConstElementLinkageIterator& li)
    {
    if (!li->user)
        return true;

    switch (li->primaryID)
        {
        case LINKAGEID_Feature:         // Never want...
        case LINKAGEID_EmbeddedBRep:    // Handled by CreateBRepCellElement...
        case LINKAGEID_Wireframe:       // Handled by CreateBRepCellElement...
        case PATTERN_ID:                // Handled by ApplyTemplate...
        case STYLELINK_ID:              // Handled by ApplyTemplate...
        case LINKAGEID_Thickness:       // Handled by ApplyTemplate...
        case DISPLAY_ATTRIBUTE_ID:      // Handled by ApplyTemplate... (NEEDSWORK: Special case to allow display style attribute?)
            return false;

        case LINKAGEID_String:
            return (STRING_LINKAGE_KEY_Name != ((MSStringLinkage*) li.GetLinkage ())->data.linkageKey); // Allow if not name...

        case LINKAGEID_Dependency:
            {
            switch (((DependencyLinkageCP) (li.GetData ()))->appID)
                {
                case DEPENDENCYAPPID_Modeler:       
                case DEPENDENCYAPPID_PatternCell:
                    return false; // Never want...

                case DEPENDENCYAPPID_MicroStation:
                    {
                    switch (((DependencyLinkageCP) (li.GetData ()))->appValue)
                        {
                        case DEPENDENCYAPPVALUE_BrepData:
                        case DEPENDENCYAPPVALUE_BrepPartition:
                            return false; // Never want...

                        default:
                            return true; // Assume ok to append...
                        }
                    break;
                    }

                default:
                    return true; // Assume ok to append...
                }
            }

        default:
            return true; // Allow unknown linkages...
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void BrepCellHeaderHandler::PropagateTemplateLinkages (EditElementHandleR eeh, ElementHandleCR templateEh)
    {
    // NOTE: See mdlSolid_createBrepTemplateForEdP (appendSolidHeaderAttrs)...
    for (ConstElementLinkageIterator li = templateEh.BeginElementLinkages (); li != templateEh.EndElementLinkages (); ++li)
        {
        if (!isAcceptableLinkage (li))
            continue;

        eeh.AppendElementLinkage (NULL, *li.GetLinkage (), li.GetData ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BrepCellHeaderHandler::CreateBRepCellHeaderElement (EditElementHandleR eeh, bool isSolid, DgnModelR modelRef)
    {
    NormalCellHeaderHandler::CreateOrphanCellElement (eeh, isSolid ? CELLNAME_Solid : CELLNAME_Surface, true, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BrepCellHeaderHandler::CreateBRepCellElement
(
EditElementHandleR      outEeh,
ElementHandleCP         templateEh,
ISolidKernelEntityCR    entity,
DgnModelR            modelRef
)
    {
    // Initialize Smart Surface/Solid header...
    ISolidKernelEntity::KernelEntityType entityType = entity.GetEntityType ();

    if (ISolidKernelEntity::EntityType_Sheet != entityType && ISolidKernelEntity::EntityType_Solid != entityType)
        return ERROR;

    EditElementHandle   eeh;

    CreateBRepCellHeaderElement (eeh, ISolidKernelEntity::EntityType_Solid == entityType, modelRef);
    
    if (SUCCESS != UpdateBRepDataDgnStore (eeh, entity))
        return ERROR;

    ElementAgenda   edgeGeom;

    // Ask element to draw and collect draw output as edges...
    EdgeCollector edges (edgeGeom, true, false);
    ElementGraphicsOutput::Process (edges, eeh);

    if (0 == edgeGeom.GetCount ())
        return ERROR;

    EditElementHandleP curr = edgeGeom.GetFirstP ();
    EditElementHandleP end  = curr + edgeGeom.GetCount ();

    for (; curr < end; curr++)
        NormalCellHeaderHandler::AddChildElement (eeh, *curr);

    // NOTE: Don't call AddChildComplete...we've set an explict origin based on entity transform!!!
    if (SUCCESS != NormalCellHeaderHandler::SetCellRange (eeh))
        return ERROR;

    // Update component count...
    eeh.GetElementDescrP ()->Validate ();

    // Set symbology from template...
    if (templateEh)
        {
        BrepCellHeaderHandler::PropagateTemplateLinkages (eeh, *templateEh);
        ElementPropertiesSetter::ApplyTemplate (eeh, *templateEh);
        }

    outEeh.SetElementDescr (eeh.ExtractElementDescr().get(), false);

    return SUCCESS;
    }

static ElementRefAppData::Key s_interopConvertResultCacheKey;

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  07/13
//=======================================================================================
struct InteropConvertResultAppData : ElementRefAppData
{
private:

UInt32          m_dataSize;
void*           m_dataP;
Transform       m_entityTransform;

protected:

virtual WCharCP _GetName () override {return L"InteropConvertResultAppData";}
virtual bool    _OnElemChanged (ElementRefP host, bool qvCacheDeleted, ElemRefChangeReason reason) override {return (ELEMREF_CHANGE_REASON_ClearQVData != reason);}
virtual void    _OnCleanup(ElementRefP host, bool unloadingCache, HeapZone& zone) override { if (unloadingCache) return; if (NULL != m_dataP) zone.Free(m_dataP, m_dataSize); zone.Free(this, sizeof *this); }

public:

InteropConvertResultAppData () {m_dataSize = 0; m_dataP = NULL;}

void SaveEntity (ISolidKernelEntityCR entity, HeapZone& zone)
    {
    void*   dataP = NULL;
    UInt32  dataSize = 0;

    if (SUCCESS != T_HOST.GetSolidsKernelAdmin ()._SaveEntityToMemory (&dataP, dataSize, entity))
        return;

    m_entityTransform = entity.GetEntityTransform ();
    m_dataSize = dataSize;
    m_dataP = zone.Alloc (dataSize); // Need to copy, dataP is owned by entity...
    memcpy (m_dataP, dataP, dataSize);
    }

BentleyStatus RestoreEntity (ISolidKernelEntityPtr& entity)
    {
    if (0 == m_dataSize)
        return ERROR;

    return (BentleyStatus) T_HOST.GetSolidsKernelAdmin ()._RestoreEntityFromMemory (entity, m_dataP, m_dataSize, ISolidKernelEntity::SolidKernel_PSolid, m_entityTransform);
    }

}; // InteropConvertResultAppData

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BrepCellHeaderHandler::_GetBRepDataEntity (ElementHandleCR source, ISolidKernelEntityPtr& entity, bool useCache)
    {
    ISolidKernelEntity::SolidKernelType kernel;

    if (SUCCESS != BrepCellHeaderHandler::GetBRepDataKernel (kernel, source))
        return ERROR;

    ElementRefP elemRef = source.GetElementRef ();

    // Check for a cached result from a previous ACIS->Parasolid conversion...
    if (ISolidKernelEntity::SolidKernel_ACIS == kernel && NULL != elemRef)
        {
        InteropConvertResultAppData* resultCache = (InteropConvertResultAppData*) elemRef->FindAppData (s_interopConvertResultCacheKey);

        if (NULL != resultCache)
            return resultCache->RestoreEntity (entity);
        }

    UInt32      dataSize = 0;
    void*       dataP = NULL;

    if (SUCCESS != BrepCellHeaderHandler::ExtractDgnStoreBRepData (&dataP, &dataSize, source))
        return ERROR;

    // Compute body transform (stored relative to cell)...account for body to uor scale and brep data scale...
    DPoint3d    origin;
    RotMatrix   rMatrix;
    Transform   transform, rotateTrans;

    CellUtil::ExtractOrigin (origin, source);
    CellUtil::ExtractRotation (rMatrix, source);

    rotateTrans.InitFrom(rMatrix);
    transform.InitFrom( origin.x,  origin.y,  origin.z);
    transform.InitProduct(transform,rotateTrans);

    double      solidScale = IBRepQuery::GetSolidKernelToUORScale (source.GetDgnModelP ());
    Transform   bodyTransform, bodyToUorTrans, uorToBodyTrans;

    bodyToUorTrans.scaleMatrixColumns (NULL, solidScale, solidScale, solidScale);
    uorToBodyTrans.inverseOf (&bodyToUorTrans);

    bodyTransform.productOf (&uorToBodyTrans, &transform);
    bodyTransform.productOf (&bodyTransform, &bodyToUorTrans);
    bodyTransform.productOf (&bodyToUorTrans, &bodyTransform);

    double      scale = 0.0;

    BrepCellHeaderHandler::GetBRepDataScale (scale, source);

    // If there is a scale stored on the brep linkage apply it to the body transform...
    if (scale != 0.0 && scale != 1.0)
        {
        Transform   brepTransform;

        brepTransform.ScaleMatrixColumns (Transform::FromIdentity (), scale, scale, scale);
        bodyTransform.InitProduct(bodyTransform,brepTransform);
        }

    DgnPlatformLib::Host::SolidsKernelAdmin& solidAdmin = T_HOST.GetSolidsKernelAdmin();

    BentleyStatus   status = (BentleyStatus) solidAdmin._RestoreEntityFromMemory (entity, dataP, dataSize, kernel, bodyTransform);

    DgnStoreHdrHandler::FreeExtractedData (dataP);

    // Cache result of expensive/slow ACIS->Parasolid conversion...
    if (ISolidKernelEntity::SolidKernel_ACIS == kernel && NULL != elemRef && (SUCCESS != status || ISolidKernelEntity::EntityType_Solid == entity->GetEntityType ()))
        {
        HeapZone&  zone = elemRef->GetHeapZone ();

        InteropConvertResultAppData* resultCache = new ((InteropConvertResultAppData*) zone.Alloc (sizeof (InteropConvertResultAppData))) InteropConvertResultAppData ();

        if (SUCCESS != elemRef->AddAppData (s_interopConvertResultCacheKey, resultCache, zone))
            zone.Free (resultCache, sizeof (*resultCache));
        else if (SUCCESS == status)
            resultCache->SaveEntity (*entity, zone);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BrepCellHeaderHandler::_SetBRepDataEntity (EditElementHandleR eeh, ISolidKernelEntityR entity)
    {
    return CreateBRepCellElement (eeh, &eeh, entity, *eeh.GetDgnModelP ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void            MissingFeatureSolidAppHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_MissingFeatureSolidApp));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MissingFeatureSolidAppHandler::_ClaimElement (ElementHandleCR elHandle)
    {
    /* NOTE: Only want this for nested nodes...allow outermost header to be a NormalCellHeaderHandler
             so that the solid can still be selected in the event of a missing brep linkage. */
    if (!elHandle.GetElementCP ()->IsComplexComponent())
        return false;

    return mdlElement_attributePresent (elHandle.GetElementCP (), LINKAGEID_Feature, NULL);
    }

