/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DisplayFilterHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
    #include    <DgnPlatform/DgnECSymbolProvider.h>
#endif

#include <ECObjects/ECExpressionNode.h>

USING_NAMESPACE_EC

#ifdef NEEDS_WORK_TopazMerge_AllOfDisplayFilterHandler  //  Not certain any of this is needed.

#define GET_AND_INCREMENT_DATA(value, pData) memcpy (&value, pData, sizeof (value)); pData += sizeof (value);

#define EXPRESSION_SCHEMA_NAME          L"DisplayFilterExpressionSchema"
#define EXPRESSIONS_CLASS_NAME          L"Expressions"
#define EXPRESSIONS_PROPERTY_NAME       L"Expressions"
#define EXPRESSIONS_PROPERTY_INDEX      1

/*---------------------------------------------------------------------------------**//**
* In each DgnFile we store a single ECXDInstance containing each unique ECExpression used
* for DisplayFilters by elements in that file. Each expression is associated with an ID
* corresponding to its position in the expression array. The XGraphics data stores only
* the ID.
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExpressionAppData : DgnFileAppData
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
, IPrivateECXDataHandler
#endif
    {
private:
    struct Expression
        {
        NodePtr                 m_node;
        WCharCP                 m_unparsed;
        bool                    m_storesTransform;  // if true, this expression uses some method whose inputs must be transformed. We will store the transform in the XGraphics data if not identity.
        bool                    m_isPersisted;
        bool                    m_triedParse;

        Expression() : m_unparsed(NULL), m_storesTransform(false), m_isPersisted(false), m_triedParse(false) { }
        Expression (WCharCP unparsed, bool storesTransform, bool isPersisted) : m_unparsed (unparsed && *unparsed ? unparsed : NULL), m_storesTransform(storesTransform), m_isPersisted(isPersisted), m_triedParse(false) { }
        Expression (Expression const& other) : m_node(other.m_node), m_unparsed(other.m_unparsed), m_storesTransform(other.m_storesTransform), m_isPersisted(other.m_isPersisted), m_triedParse(other.m_triedParse) { }

        NodeP                   Get()
            {
            if (!m_triedParse && NULL != m_unparsed)
                {
                m_node = ECEvaluator::ParseValueExpressionAndCreateTree (m_unparsed);
                m_triedParse = true;    // could fail and produce a null NodePtr...don't try repeatedly
                }

            return m_node.get();
            }
        };

    typedef bvector<Expression>     ExpressionList;     // fast lookup of parsed expression by ID
    typedef bmap<WString, UInt32>   ExpressionIdMap;    // maps unparsed expression to ID. Replace with unordered_map if warranted. Only used when creating new DisplayFilter expression
    typedef bset<UInt32>            ExpressionIdSet;

    ExpressionList                  m_expressionsById;
    ExpressionIdMap                 m_expressionIdMap;
    DgnFileR                        m_dgnfile;
    ExpressionIdSet                 m_pendingWrites;

    ExpressionAppData (DgnFileR dgnfile) : m_dgnfile(dgnfile) { }

    bool                        Initialize();
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
    ECXDInstancePtr             LocateInstance (bool createIfNotFound) const;
#endif
    UInt32                      InsertExpression (WCharCP expr, UInt32 id, bool storesTransform); // -1 to append; returns ID
    static bool                 ExpressionRequiresTransform (WCharCP expr);

    // DgnFileAppData
    virtual void _OnCleanup (DgnFileR host) override { delete this; }

    // IPrivateECXDataHandler
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
    virtual WCharCP             _GetPrivateECXDSchemaName() const override  { return EXPRESSION_SCHEMA_NAME; }
    virtual ECN::ECSchemaPtr    _CreatePrivateECXDSchema() const override;
    virtual bool                _SchemaShouldBeStored() const override { return false; }
    virtual bool                _SchemaShouldBeUpdated (UInt32 versionMajor, UInt32 versionMinor, DgnFileR dgnfile) const override { return true; }
#endif
public:
    WCharCP                     GetUnparsed (size_t id);
    NodeP                       GetExpression (UInt32 id);
    UInt32                      GetExpressionId (WCharCP expr);
    bool                        ExpressionRequiresTransform (UInt32 id) const;

    bool                        EnsureExpressionIsPersisted (UInt32 id);
    bool                        FlushPendingExpressions();

    static ExpressionAppData*   GetForFile (DgnFileR dgnFile);
    static UInt32               TransformExpressionId (UInt32 srcId, DgnFileR srcFile, DgnFileR dstFile);
    };
#ifdef WIP_VANCOUVER_MERGE // displayfilter
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionAppData::EnsureExpressionIsPersisted (UInt32 id)
    {
    if (m_dgnfile.IsReadOnly() || id > m_expressionsById.size())
        { BeAssert (false); return false; }

    Expression& expr = m_expressionsById[id];
    if (!expr.m_isPersisted)
        m_pendingWrites.insert (id);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionAppData::FlushPendingExpressions()
    {
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
    if (0 == m_pendingWrites.size())
        return true;

    // NB: Writing expression table to Type9 element is a non-undoable transaction.
    SaveSettingsTxnMark nonUndoableTxn;

    ECValue v;
    ECXDInstancePtr instance = LocateInstance (true);
    if (instance.IsValid() && ECOBJECTS_STATUS_Success == instance->GetValue (v, EXPRESSIONS_PROPERTY_INDEX))
        {
        // Make sure we have allocated enough space in the array to store all expressions.
        // (Note we may be allocating more than we need at this time).
        UInt32 nExpressionsInCache = (UInt32)m_expressionsById.size();
        UInt32 nExpressionsStored = v.GetArrayInfo().GetCount();
        if (nExpressionsInCache <= nExpressionsStored || ECOBJECTS_STATUS_Success == instance->AddArrayElements (EXPRESSIONS_PROPERTY_INDEX, nExpressionsInCache - nExpressionsStored))
            {
            bool valuesUpdated = true;
            for (UInt32 id: m_pendingWrites)
                {
                Expression& expr = m_expressionsById[id];

                // We prepend a sigil to indicate the expression requires a stored transform, to avoid having to create a second property for it.
                WString storedExpr;
                if (expr.m_storesTransform)
                    {                                                                                                                 
                    storedExpr = L"@";
                    storedExpr.append (expr.m_unparsed);
                    v.SetString (storedExpr.c_str(), false);
                    }
                else
                    v.SetString (expr.m_unparsed, false);

                if (NULL == v.GetString() || ECOBJECTS_STATUS_Success != instance->SetValue (EXPRESSIONS_PROPERTY_INDEX, v, id))
                    {
                    valuesUpdated = false;
                    break;
                    }

                expr.m_isPersisted = true;
                }

            if (valuesUpdated && SUCCESS == instance->WriteChanges())
                {
                m_pendingWrites.clear();
                return true;
                }
            else
                {
                // Reset the persistence flags...
                for (UInt32 id: m_pendingWrites)
                    {
                    m_expressionsById[id].m_isPersisted = false;
                    }
                }
            }
        }

    m_pendingWrites.clear();
#endif
    BeAssert (false && "Failed to write DisplayFilter expression table");
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Insert into our in-memory cache, possibly assigning an ID if it's a new expression.
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ExpressionAppData::InsertExpression (WCharCP expr, UInt32 id, bool storesTransform)
    {
    bool isNew = (-1 == id);
    if (isNew)
        {
        // May have gaps in table...find first unused ID.
        for (size_t i = 0; i < m_expressionsById.size(); i++)
            {
            if (NULL == m_expressionsById[i].m_unparsed)
                {
                id = (UInt32)i;
                break;
                }
            }

        if (-1 == id)
            {
            id = (UInt32)m_expressionsById.size();
            m_expressionsById.resize (id+1);
            }
        }

    if ((UInt32)m_expressionsById.size() > id)
        {
        ExpressionIdMap::iterator iter = m_expressionIdMap.insert (ExpressionIdMap::value_type (expr, id)).first;
        m_expressionsById[id] = Expression (iter->first.c_str(), storesTransform, !isNew);
        return id;
        }
    else
        {
        BeAssert (false);
        return -1;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionAppData::Initialize ()
    {
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
    ECXDInstancePtr instance = LocateInstance (false);
    ECValue v;
    if (instance.IsValid())
        {
        if (ECOBJECTS_STATUS_Success != instance->GetValue (v, EXPRESSIONS_PROPERTY_INDEX))
            { BeAssert(false); return false; }

        UInt32 numPersistedIds = v.GetArrayInfo().GetCount();
        m_expressionsById.resize (numPersistedIds);
        for (UInt32 i = 0; i < numPersistedIds; i++)
            {
            if (ECOBJECTS_STATUS_Success != instance->GetValue (v, EXPRESSIONS_PROPERTY_INDEX, i))
                { BeAssert (false); continue; }
            else
                {
                // Note we may have null entries from IDs which were assigned but never written to the file. Those entries' IDs are available for reuse.
                WCharCP expr = !v.IsNull() ? v.GetString() : NULL;
                bool storesTransform = NULL != expr && ('@' == *expr);
                if (storesTransform)
                    expr++;

                InsertExpression (expr, i, storesTransform);
                }
            }
        }
#endif
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionAppData* ExpressionAppData::GetForFile (DgnFileR dgnFile)
    {
    static DgnFileAppData::Key s_key;
    
    ExpressionAppData* appData = static_cast<ExpressionAppData*> (dgnFile.FindAppData (s_key));
    if (NULL == appData)
        {
        appData = new ExpressionAppData (dgnFile);
        if (!appData->Initialize () || SUCCESS != dgnFile.AddAppData (s_key, appData))
            {
            BeAssert (false);
            delete appData;
            appData = NULL;
            }
        }

    return static_cast<ExpressionAppData*> (appData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
ECXDInstancePtr ExpressionAppData::LocateInstance (bool createIfNotFound) const
    {
    EditElementHandle eeh (m_dgnfile.GetType9Element());
    ECXDInstancePtr instance = FindPrivateInstanceOnElement (EXPRESSIONS_CLASS_NAME, eeh);
    if (instance.IsNull() && createIfNotFound && !m_dgnfile.IsReadOnly())
        {
        ECXDInstanceEnablerP enabler = ObtainPrivateEnabler (EXPRESSIONS_CLASS_NAME, m_dgnfile);
        if (NULL != enabler)
            {
            instance = ObtainPrivateEnabler (EXPRESSIONS_CLASS_NAME, m_dgnfile)->CreateInstanceForElement (eeh, DgnECInstanceCreateContext());
            if (instance.IsValid() && SUCCESS != instance->WriteChanges())
                instance = NULL;
            }
        }

    return instance;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionAppData::ExpressionRequiresTransform (WCharCP expr)
    {
    // We have a handful of ViewContext methods in which the inputs are specified as points/vectors to which Transform must be applied on evaluation
    // In ExpressionFilterHandler::_OnTransform(), we will store the transform with the XGraphics data if required
    WCharCP found = wcsstr (expr, L"ViewContext");
    return NULL != found && (NULL != wcsstr (expr, L"TestAxisAngle") || NULL != wcsstr (expr, L"GetPixelSizeAt"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionAppData::ExpressionRequiresTransform (UInt32 id) const
    {
    if (id >= (UInt32)m_expressionsById.size())
        { BeAssert(false); return false; }

    return m_expressionsById[id].m_storesTransform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP ExpressionAppData::GetExpression (UInt32 id)
    {
    NodeP node = id < (UInt32)m_expressionsById.size() ? m_expressionsById[id].Get() : NULL;
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ExpressionAppData::GetUnparsed(size_t id)
    {
    return id < m_expressionsById.size() ? m_expressionsById[id].m_unparsed : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ExpressionAppData::GetExpressionId (WCharCP expr)
    {
    UInt32 id = -1;
    ExpressionIdMap::iterator iter = m_expressionIdMap.find (expr);
    if (m_expressionIdMap.end() != iter)
        id = iter->second;
    else
        id = InsertExpression (expr, -1, ExpressionRequiresTransform (expr));

    BeAssert (-1 != id);
    return id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ExpressionAppData::TransformExpressionId (UInt32 srcId, DgnFileR srcFile, DgnFileR dstFile)
    {
    UInt32 dstId = -1;
    ExpressionAppData*  srcData = GetForFile (srcFile),
                     *  dstData = GetForFile (dstFile);
    if (NULL != srcData && NULL != dstData)
        {
        WCharCP expr = NULL;
        for (ExpressionIdMap::iterator srcIter = srcData->m_expressionIdMap.begin(); srcIter != srcData->m_expressionIdMap.end(); ++srcIter)
            if (srcIter->second == srcId)
                {
                expr = srcIter->first.c_str();
                break;
                }

        if (NULL != expr)
            {
            ExpressionIdMap::iterator dstIter = dstData->m_expressionIdMap.find (expr);
            if (dstData->m_expressionIdMap.end() != dstIter)
                dstId = dstIter->second;
            else
                dstId = dstData->InsertExpression (expr, -1, srcData->ExpressionRequiresTransform (srcId));
            }
        }

    BeAssert (-1 != dstId);
    return dstId;
    }

#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ExpressionAppData::_CreatePrivateECXDSchema() const
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema (schema, EXPRESSION_SCHEMA_NAME, 1, 0);

    ECClassP ecClass;
    schema->CreateClass (ecClass, EXPRESSIONS_CLASS_NAME);

    ArrayECPropertyP arrayProp;
    ecClass->CreateArrayProperty (arrayProp, EXPRESSIONS_PROPERTY_NAME, PRIMITIVETYPE_String);

    return schema;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static DVec3d  getViewPerpendicular (ViewContextR viewContext, bool inLocalCoordinates)
    {
    DPoint4d            zColumn4d;
    DVec3d              zColumn;
    DMatrix4d           toNpc = viewContext.GetFrustumToNpc().M0;
    Transform           localToFrustumTransform;

    if (inLocalCoordinates && SUCCESS  == viewContext.GetCurrLocalToFrustumTrans (localToFrustumTransform))
        {
        DMatrix4d       localToFrustum = DMatrix4d::From (localToFrustumTransform);

        toNpc.productOf (&toNpc, &localToFrustum);
        }

    toNpc.getRow (NULL, NULL, &zColumn4d, NULL);
    zColumn.xyzOf (&zColumn4d);
    zColumn.normalize ();

    return zColumn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP getViewOrientation (ViewContextR viewContext)
    {
    static double       s_nearZero = 1.0E-8;
    if (NULL != viewContext.GetViewport())
        {
        DVec3d              viewPerpendicular = getViewPerpendicular (viewContext, false);
      
        if (fabs (viewPerpendicular.z - 1.0) < s_nearZero)
            return L"Top";

        if (fabs (viewPerpendicular.z) < s_nearZero)
            return L"Elevation";
        }

    return L"";       // Needs work.
    }

#define DRAWPURPOSE_TOSTRING(X) case DrawPurpose:: ## X ## : return L" ## X ## ";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
static WCharCP getDrawPurpose (ViewContextCR vc)
    {
    switch (vc.GetDrawPurpose())
        {
        DRAWPURPOSE_TOSTRING(NotSpecified)
        DRAWPURPOSE_TOSTRING(Update)
        DRAWPURPOSE_TOSTRING(UpdateDynamic)
        DRAWPURPOSE_TOSTRING(UpdateHealing)
        DRAWPURPOSE_TOSTRING(Hilite)
        DRAWPURPOSE_TOSTRING(Unhilite)
        DRAWPURPOSE_TOSTRING(ChangedPre)
        DRAWPURPOSE_TOSTRING(ChangedPost)
        DRAWPURPOSE_TOSTRING(RestoredPre)
        DRAWPURPOSE_TOSTRING(RestoredPost)
        DRAWPURPOSE_TOSTRING(Dynamics)
        DRAWPURPOSE_TOSTRING(RangeCalculation)
        DRAWPURPOSE_TOSTRING(Plot)
        DRAWPURPOSE_TOSTRING(Pick)
        DRAWPURPOSE_TOSTRING(Flash)
        DRAWPURPOSE_TOSTRING(TransientChanged)
        DRAWPURPOSE_TOSTRING(CaptureGeometry)
        DRAWPURPOSE_TOSTRING(GenerateThumbnail)
        DRAWPURPOSE_TOSTRING(ForceRedraw)
        DRAWPURPOSE_TOSTRING(FenceAccept)
        DRAWPURPOSE_TOSTRING(RegionFlood)
        DRAWPURPOSE_TOSTRING(FitView)
        DRAWPURPOSE_TOSTRING(XGraphicsCreate)
        DRAWPURPOSE_TOSTRING(CaptureShadowList)
        DRAWPURPOSE_TOSTRING(ExportVisibleEdges)
        DRAWPURPOSE_TOSTRING(InterferenceDetection)
        DRAWPURPOSE_TOSTRING(CutXGraphicsCreate)
        DRAWPURPOSE_TOSTRING(ModelFacet)
        DRAWPURPOSE_TOSTRING(Measure)
        DRAWPURPOSE_TOSTRING(VisibilityCalculation)
        DRAWPURPOSE_TOSTRING(ProxyHashExtraction)
        DRAWPURPOSE_TOSTRING(ComputeDgnModelRange)
        default: return L"";
        }
    }
#undef DRAWPURPOSE_TOSTRING

#define VIEWCONTEXT_SCHEMANAME      L"ViewContextSchema"
#define VIEWCONTEXT_CLASSNAME       L"ViewContext"

/*---------------------------------------------------------------------------------**//**
* Collect as much boilerplate as possible into one base class...
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct BaseECInstance : IECInstance
    {
protected:
    ECEnablerPtr                m_enabler;
    ViewContextR                m_context;

    BaseECInstance (ECEnablerR enabler, ViewContextR context) : m_enabler(&enabler), m_context(context) { }

    virtual bool                _IsReadOnly() const { return true; }
    virtual WString             _GetInstanceId() const { return L""; }
    virtual WString             _ToString (WCharCP indent) const { return L""; }
    virtual size_t              _GetOffsetToIECInstance () const { return 0; }
    virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _InsertArrayElements (UInt32 propertyIndex, UInt32 index, UInt32 size) { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _AddArrayElements (UInt32 propertyIndex, UInt32 size) { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _RemoveArrayElement (UInt32 propertyIndex, UInt32 index) { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _ClearArray (UInt32 propertyIndex) { return ECOBJECTS_STATUS_OperationNotSupported; }

    virtual ECEnablerCR         _GetEnabler() const { return *m_enabler; }
    virtual ECObjectsStatus     _GetIsPropertyNull (bool& v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
        {
        ECValue ecValue;
        ECObjectsStatus status = GetValue (ecValue, propertyIndex);
        if (ECOBJECTS_STATUS_Success == status)
            v = ecValue.IsNull();
        return status;
        }
public:
    ViewContextR                GetViewContext() const { return m_context; }
    };

enum
    {
    PropIdx_Orientation         = 1,
    PropIdx_ClipPass,
    PropIdx_InViewlet,
    PropIdx_DrawPurpose,
    PropIdx_PresentationFormId,
    PropIdx_LevelOfDetail,
    PropIdx_MAX
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewContextECInstance : BaseECInstance
    {
private:
    ViewContextECInstance (ECEnablerR enabler, ViewContextR context) : BaseECInstance (enabler, context) { }

    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
        {
        switch (propertyIndex)
            {
        case PropIdx_Orientation:
            v.SetString (getViewOrientation (m_context));
            break;

        case PropIdx_ClipPass:
            {
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
            static WCharCP s_passNames[static_cast<UInt32>(ClipVolumePass::Maximum)+1] = { L"None", L"InsideForward", L"InsideBackward", L"Outside", L"Inside", L"Cut", L"Maximum" };
            v.SetString (s_passNames[static_cast<UInt32>(m_context.GetDynamicViewStateStack().back().GetPass())]);
#endif
            }
            break;

        case PropIdx_InViewlet:
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
            v.SetBoolean (m_context.InViewlet());
#endif
            break;

        case PropIdx_DrawPurpose:
            v.SetString (getDrawPurpose (m_context));
            break;

        case PropIdx_PresentationFormId:
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
            v.SetString (m_context.GetPresentationFormId(), false);
#endif
            break;

        case PropIdx_LevelOfDetail:
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
            v.SetDouble (m_context.GetCurrentLevelOfDetail());
#endif
            break;
        
        default:
            return ECOBJECTS_STATUS_OperationNotSupported;
            }

        return ECOBJECTS_STATUS_Success;
        }
public:
    static IECInstancePtr Create (ECEnablerR enabler, ViewContextR context) { return new ViewContextECInstance (enabler, context); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewContextECEnabler : ECEnabler
    {
private:
    ViewContextECEnabler (ECSchemaR schema, ECClassR ecClass);

    ECSchemaPtr             m_schema;
    WCharCP                 m_accessors[PropIdx_MAX];

    void                    AddProperty (ECClassR ecClass, WCharCP name, UInt32 propIdx, PrimitiveType type);

    // ECEnabler
    virtual WCharCP         _GetName() const override { return L"ViewContextECEnabler"; }
    virtual ECObjectsStatus _GetPropertyIndex (UInt32& propIdx, WCharCP accessor) const override
        {
        for (UInt32 i = 1; i < PropIdx_MAX; i++)
            if (0 == wcscmp (accessor, m_accessors[i]))
                {
                propIdx = i;
                return ECOBJECTS_STATUS_Success;
                }
        return ECOBJECTS_STATUS_PropertyNotFound;
        }
    virtual ECObjectsStatus _GetAccessString (WCharCP& accessor, UInt32 propIdx) const override
        {
        if (propIdx < PropIdx_MAX && propIdx > 0)
            {
            accessor = m_accessors[propIdx];
            return ECOBJECTS_STATUS_Success;
            }
        return ECOBJECTS_STATUS_PropertyNotFound;
        }
    virtual UInt32          _GetFirstPropertyIndex (UInt32 parentIndex) const override  { return 0 == parentIndex ? PropIdx_Orientation : 0; }
    virtual UInt32          _GetNextPropertyIndex (UInt32 parentIndex, UInt32 inputIndex) const override   { return 0 == parentIndex && (inputIndex+1) < PropIdx_MAX ? inputIndex+1 : 0; }
    virtual ECObjectsStatus _GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const override
        {
        if (0 != parentIndex)
            return ECOBJECTS_STATUS_Error;
        for (UInt32 i = PropIdx_Orientation; i < PropIdx_MAX; i++)
            indices.push_back (i);
        return ECOBJECTS_STATUS_Success;
        }
    virtual bool            _IsPropertyReadOnly (UInt32 propIdx) const override { return true; }
    virtual bool            _HasChildProperties (UInt32 parentIndex) const override { return false; }
public:
    static ECEnablerPtr     Create()
        {
        ECSchemaPtr schema;
        ECSchema::CreateSchema (schema, VIEWCONTEXT_SCHEMANAME, 1, 0);
        ECClassP ecClass;
        schema->CreateClass (ecClass, VIEWCONTEXT_CLASSNAME);

        return new ViewContextECEnabler (*schema, *ecClass);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContextECEnabler::ViewContextECEnabler (ECSchemaR schema, ECClassR ecClass) : ECEnabler (ecClass, NULL), m_schema (&schema)
    {
    AddProperty (ecClass, L"Orientation",   PropIdx_Orientation,    PRIMITIVETYPE_String);
    AddProperty (ecClass, L"ClipPass",      PropIdx_ClipPass,       PRIMITIVETYPE_String);
    AddProperty (ecClass, L"InViewlet",     PropIdx_InViewlet,      PRIMITIVETYPE_Boolean);
    AddProperty (ecClass, L"DrawPurpose",   PropIdx_DrawPurpose,    PRIMITIVETYPE_String);
    AddProperty (ecClass, L"PresentationForm", PropIdx_PresentationFormId, PRIMITIVETYPE_String);
    AddProperty (ecClass, L"LevelOfDetail", PropIdx_LevelOfDetail, PRIMITIVETYPE_Double);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContextECEnabler::AddProperty (ECClassR ecClass, WCharCP name, UInt32 propIdx, PrimitiveType type)
    {
    PrimitiveECPropertyP prop;
    ecClass.CreatePrimitiveProperty (prop, name, type);
    m_accessors[propIdx] = name;
    }

//=================================================================================**//**
//! @bsiclass                                                     RayBentley      08/2013
//==============+===============+===============+===============+===============+======*/                                                                        
struct PresentationFormIdHandler : DisplayFilterHandler
{
protected:
virtual             bool        _DoConditionalDraw (ViewContextR viewContext, ElementHandleCP element, void const* data, size_t dataSize) const override 
    {
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
    return 0 == BeStringUtilities::Wcsicmp ((WCharCP) data, viewContext.GetPresentationFormId()); 
#else
    return false;
#endif
    }
virtual             WString     _GetDumpString (void const* data, size_t dataSize, DgnModelP) const override  { return WString (L"Presentation Form Id: ") + WString ((WCharCP) data); }

};  // PresentationFormIdHandler

//=================================================================================**//**
//! @bsiclass                                                     RayBentley      08/2013
//==============+===============+===============+===============+===============+======*/                                                                        
struct PresentationFormFlagHandler : DisplayFilterHandler
{
protected:
virtual             bool        _DoConditionalDraw (ViewContextR viewContext, ElementHandleCP element, void const* data, size_t dataSize) const override 
    { 
#ifdef NEEDS_WORK_TopazMerge_DgnECSymbolProvider
    return viewContext.GetPresentationFormFlag ((WCharCP) data); 
#else
    return true;
#endif
    }
virtual             WString     _GetDumpString (void const* data, size_t dataSize, DgnModelP) const override  { return WString (L"Presentation Form Flag: ") + WString ((WCharCP) data); }

};  // PresentationFormIdHandler

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExpressionFilterHandler : DisplayFilterHandler
    {
private:

bool ExpressionFilterHandler::_DoConditionalDraw (ViewContextR viewContext, ElementHandleCP element, void const* data, size_t dataSize) const override;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
WString ExpressionFilterHandler::_GetDumpString (void const* data, size_t dataSize, DgnModelP dgnModel) const override
    {
    BeAssert (sizeof(UInt32) == dataSize || (sizeof (UInt32) + sizeof (Transform)) == dataSize);

    UInt32 exprId = *((UInt32*)data);

    ExpressionAppData*  appData;
    WCharCP             unparsed;

    if (NULL == dgnModel || 
        NULL == dgnModel->GetDgnFileP() || 
        NULL == (appData = ExpressionAppData::GetForFile (*dgnModel->GetDgnFileP())) ||
        NULL == (unparsed = appData->GetUnparsed (exprId)))
        {
        WString dump;
        // We don't have the context to extract the unparsed expression...
        dump.Sprintf (L"ExpressionID %d", exprId);

        return dump;
        }
    else
        {
        return WString (unparsed);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ExpressionFilterHandler::_OnTransform (TransformInfoCR transform, void* data, size_t dataSize) const override
    {
    if (dataSize == sizeof(UInt32) + sizeof(Transform))
        {
        byte* bytes = (byte*)data;
        Transform* t = (Transform*)(bytes + sizeof(UInt32));
        *t = Transform::FromProduct (*transform.GetTransform(), *t);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpressionFilterHandler::_DoClone (void* data, size_t dataSize
#ifdef NEEDS_WORK_TopazMerge_DisplayFilter
    , ElementCopyContextR context
#endif
    ) const
    {
    // Expression IDs may differ between DgnFiles. Make sure the expression is imported into destination file, and remap ID
#ifdef NEEDS_WORK_TopazMerge_CopyContext
    if (!context.IsSameFile())
        {
        bool copied = false;
        DgnModelP srcDgnModel = context.GetSourceDgnModel(),
                     dstDgnModel = context.GetDestinationDgnModel();
        if (NULL != srcDgnModel && NULL != srcDgnModel->GetDgnFileP() && NULL != dstDgnModel && NULL != dstDgnModel->GetDgnFileP())
            {
            UInt32* pExprId = (UInt32*)data;
            *pExprId = ExpressionAppData::TransformExpressionId (*pExprId, *srcDgnModel->GetDgnFileP(), *dstDgnModel->GetDgnFileP());
            copied = (-1 != *pExprId);
            }

        // ###TODO: change method signature to return bool...if we fail there is something very wrong, but calling code will want to know about it...
        BeAssert (copied);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ExpressionFilterHandler::_OnWriteToElement (void* data, size_t dataSize, ElementHandleCR eh) const override
    {
    UInt32*             pExprId = (UInt32*)data;
    ExpressionAppData*  appdata;

    if (NULL == eh.GetDgnFileP() || NULL == (appdata = ExpressionAppData::GetForFile (*eh.GetDgnFileP())))
        return ERROR;
    else
        return appdata->EnsureExpressionIsPersisted (*pExprId) ? SUCCESS : ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   _GetExpressionData (bvector<byte>& data, WCharCP expression, DgnFileR dgnFile) const override
    {
    DisplayFilter::OperatorPtr  filterOperator = DisplayFilter::TestExpression::Create (expression, dgnFile);

    if (!filterOperator.IsValid())
        return ERROR;
   
    filterOperator->GetData (data);
    return SUCCESS;
    }

};      // ExpressionFilterHandler


/*---------------------------------------------------------------------------------**//**
* Provides properties for the "View" symbol in DisplayFilter ECExpressions.
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewInfoECInstance : BaseECInstance
    {
    ViewInfoECInstance (ECEnablerR enabler, ViewContextR context) : BaseECInstance (enabler, context) { }

    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
        {
#ifdef NEEDS_WORK_TopazMerge_ECDisplayFilter
        // NEEDSWORK: View type requires the the NamedView or the view's element handler, which we don't necessarily have...
        if (BESINDEX_MstnView_Type == propertyIndex)
            {
            for (DgnModelP model = m_context.GetCurrentModel(); NULL != model; model = model->GetParentDgnModelP())
                {
                DgnAttachmentCP     dgnAttachment;
                NamedViewPtr        namedView;
                if (NULL != (dgnAttachment = model->AsDgnAttachmentCP()) && (namedView = dgnAttachment->GetNamedView()).IsValid())
                    {
                    WCharCP name = namedView->GetViewTypeName();
                    if (NULL != name)
                        {
                        v.SetString (name);
                        break;
                        }
                    }
                }

            if (v.IsNull())
                v.SetString (L"");
            return ECOBJECTS_STATUS_Success;
            }
        v.Clear();
        /*DynamicViewSettingsCP*/void* settings = m_context.GetDynamicViewSettings (m_context.GetCurrentModel());
        ViewFlagsCP flags = m_context.GetIViewDraw().GetDrawViewFlags();    // Note - Get flags from IViewDraw - not IDrawGeom (as ViewContext::GetViewFlags)
                                                                            // as we want the real flags - not the faux ones created for ICachedDraw.
        if (NULL != flags)
            return MstnViewECDelegate::GetValueFromViewFlagsAndSettings (v, *flags, settings, propertyIndex);
        else
#endif
            return ECOBJECTS_STATUS_Success;
        }
public:
    static IECInstancePtr  Create (ViewContextR context)
        {
#ifdef NEEDS_WORK_TopazMerge_ECDisplayFilter
        ElementECDelegateP delegate = ElementECDelegate::LookupRegisteredPrimaryDelegate (L"BaseElementSchema", L"MstnView");
        BeAssert (NULL != delegate);
        ECClassCP ecClass = DgnECManager::GetManager().GetDeliveredSchemaLocator().FetchSchema (SchemaKey (L"BaseElementSchema", 1, 0)).GetClassCP (L"MstnView");
        BeAssert (NULL != ecClass);

        DelegatedElementECEnablerPtr enabler = DelegatedElementECEnabler::Create (*ecClass, *delegate);
        return new ViewInfoECInstance (*ECWrappedEnabler::Create (*enabler), context);
#else
        return NULL;
#endif
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewSymbolContext : InstanceListExpressionContext
    {
#ifdef NEEDS_WORK_TopazMerge_ViewSymbolContext
private:
    ViewContextR                        m_viewContext;
    /*DynamicViewSettingsCP*/void*               m_dynamicViewSettings;  // XAttributesHolder of which will supply extrinsic properties

    ViewSymbolContext (ViewContextR context) : InstanceListExpressionContext(), m_viewContext (context), m_dynamicViewSettings (context.GetDynamicViewSettings (context.GetCurrentModel()))
        {
        SetAllowsTypeConversion (false);
        }

    virtual void            _GetInstances (bvector<IECInstancePtr>& instances) override;
public:
    void                    SetDynamicViewSettings (/*DynamicViewSettingsCP*/void* settings)
        {
        if (settings != m_dynamicViewSettings)
            {
            Reset();
            m_dynamicViewSettings = settings;
            }
        }

    static ViewSymbolContext* Create (ViewContextR context) { return new ViewSymbolContext (context); }
#else
    IECInstancePtr                      m_viewInfoInstance;     // intrinsic MstnView properties
    virtual void            _GetInstances (bvector<IECInstancePtr>& instances) override;
#endif
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewSymbolContext::_GetInstances (bvector<IECInstancePtr>& instances)
    {
    // always include intrinsic properties.
#ifdef NEEDS_WORK_TopazMerge_ViewSymbolContext
    if (m_viewInfoInstance.IsNull())
        m_viewInfoInstance = ViewInfoECInstance::Create (m_viewContext);
#endif

    instances.push_back (m_viewInfoInstance.get());

    // never include extrinsic properties - leave that to Display Rules.
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DisplayFilterExpressionContext : SymbolExpressionContext
    {
private:
    RefCountedPtr<ViewSymbolContext>        m_viewSymbolContext;
    Transform                               m_transform;

    DisplayFilterExpressionContext (ViewSymbolContext& viewSymbolContext) 
        : SymbolExpressionContext (NULL), m_viewSymbolContext (&viewSymbolContext)
        {
        bvector<WString> symbolSets;
        symbolSets.push_back (L"DisplayFilter");
        DgnECSymbolProvider::ExternalSymbolPublisher (*this, symbolSets);

        m_transform.InitIdentity();
        }
public:
    void            Sync (ViewContextCR context)
        {
        m_viewSymbolContext->SetDynamicViewSettings (context.GetDynamicViewSettings (context.GetCurrentModel()));
        }
    TransformCR     GetTransform() const                { return m_transform; }
    void            PushTransform (TransformCR t)       { m_transform = t; }
    void            PopTransform()                      { m_transform.InitIdentity(); }

    static DisplayFilterExpressionContext* Create (ViewContextR context);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool extractViewContext (ViewContextP& vc, ECInstanceListCR ev)
    {
    vc = NULL;
    for (IECInstancePtr const& iecInstance: ev)
        {
        BaseECInstance* instance = dynamic_cast<BaseECInstance*> (iecInstance.get());
        if (NULL != instance)
            {
            vc = &instance->GetViewContext();
            break;
            }
        }

    return NULL != vc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
static TransformCR extractTransform (ViewContextCR vc)
    {
    DisplayFilterExpressionContext* exprContext = static_cast<DisplayFilterExpressionContext*> (vc.GetConditionalDrawExpressionContext());
    return exprContext->GetTransform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus ecmethod_testLevel (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    ViewContextP vc;
    Int32 levelId;
    if (!extractViewContext (vc, instanceData) || !DgnECSymbolProvider::ExtractArg (levelId, args, 0))
        return ExprStatus_UnknownError;

    LevelClassMask* levelClassMask = vc->GetLevelClassMask();
    bool result = NULL == levelClassMask || NULL == levelClassMask->levelBitMaskP || levelClassMask->levelBitMaskP->Test (((LevelId)levelId) - 1);
    evalResult.InitECValue().SetBoolean (result);
    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus ecmethod_testAxisAngle (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    ViewContextP vc;
    DPoint3d axisPt;
    if (!extractViewContext (vc, instanceData) || !DgnECSymbolProvider::ExtractArg (axisPt, args, 0))
        return ExprStatus_UnknownError;

    DVec3d axis = DVec3d::From (axisPt);
    extractTransform (*vc).MultiplyMatrixOnly (axis);
    axis.Normalize();

    DVec3d viewPerpendicular = getViewPerpendicular (*vc, true);
    double angle = acos (fabs (viewPerpendicular.dotProduct (&axis)));

    evalResult.InitECValue().SetDouble (angle);
    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus ecmethod_getPixelSizeAt (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    ViewContextP vc;
    DPoint3d origin;
    if (!extractViewContext (vc, instanceData) || !DgnECSymbolProvider::ExtractArg (origin, args, 0))
        return ExprStatus_UnknownError;

    extractTransform (*vc).Multiply (origin);
    double pixelSize = vc->GetPixelSizeAtPoint (&origin);

    evalResult.InitECValue().SetDouble (pixelSize);
    return ExprStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* This only exists because DisplayFilterHandlers must be static due to the implementation
* of DisplayFilterHandlerManager and DisplayFilter in separate dlls.
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewContextSymbolProvider : IECSymbolProvider, DgnHost::IHostObject
    {
private:
    ECEnablerPtr                m_viewContextEnabler;
    bvector<MethodSymbolPtr>    m_methods;

    ViewContextSymbolProvider() : m_viewContextEnabler (ViewContextECEnabler::Create())
        {
        DgnECSymbolProvider::GetProvider().RegisterSymbolProvider (*this);
        }

    // IHostObject
    virtual void                _OnHostTermination (bool) override { this->Release(); }
    // IECSymbolProvider
    virtual WCharCP             _GetName() const override { return L"DisplayFilter"; }
    virtual void                _PublishSymbols (ECN::SymbolExpressionContextR context, bvector<WString> const& requestedSymbolSets) override;
public:
    static ViewContextSymbolProvider&   Get()
        {
        static DgnHost::Key s_key;
        ViewContextSymbolProvider* prov = static_cast<ViewContextSymbolProvider*> (T_HOST.GetHostObject (s_key));
        if (NULL == prov)
            {
            prov = new ViewContextSymbolProvider();
            prov->AddRef();
            DgnECSymbolProvider::GetProvider().RegisterSymbolProvider (*prov);
            T_HOST.SetHostObject (s_key, prov);
            }

        return *prov;
        }

    IECInstancePtr                      CreateInstance (ViewContextR vc) const
        {
        return ViewContextECInstance::Create (*m_viewContextEnabler, vc).get();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContextSymbolProvider::_PublishSymbols (SymbolExpressionContextR context, bvector<WString> const& symbolSets)
    {
    for (WString const& symbolSet: symbolSets)
        {
        if (symbolSet.Equals (L"DisplayFilter"))
            {
            if (m_methods.empty())
                {
                m_methods.reserve (3);
                m_methods.push_back (MethodSymbol::Create (L"TestLevel", NULL, &ecmethod_testLevel));
                m_methods.push_back (MethodSymbol::Create (L"TestAxisAngle", NULL, &ecmethod_testAxisAngle));
                m_methods.push_back (MethodSymbol::Create (L"GetPixelSizeAt", NULL, &ecmethod_getPixelSizeAt));
                }

            for (MethodSymbolPtr const& method: m_methods)
                context.AddSymbol (*method);

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilterExpressionContext* DisplayFilterExpressionContext::Create (ViewContextR context)
    {
    // Note we do this first so if this is the first time, the ViewContextSymbolProvider is available to provide symbols in our c'tor.
    ViewContextSymbolProvider& vcSymbolProvider = ViewContextSymbolProvider::Get();

    ViewSymbolContext* viewSymbolContext = ViewSymbolContext::Create (context);

    // Outer context contains "ViewContext", "Element", and "View" symbols
    DisplayFilterExpressionContext* outerContext = new DisplayFilterExpressionContext (*viewSymbolContext);

    ContextSymbolPtr viewSymbol = ContextSymbol::CreateContextSymbol (L"View", *viewSymbolContext);
    outerContext->AddSymbol (*viewSymbol);

    IECInstancePtr                  viewContextInstance = vcSymbolProvider.CreateInstance (context);
    InstanceExpressionContextPtr    instanceContext = InstanceExpressionContext::Create (NULL);

    instanceContext->SetInstance (*viewContextInstance);
    ContextSymbolPtr contextSymbol = ContextSymbol::CreateContextSymbol (L"ViewContext", *instanceContext);
    outerContext->AddSymbol (*contextSymbol);

    // By default the expression evaluator does dumb stuff like localizing strings and converting boolean values to string.
    // It also converts UORs to master units.
    // We don't want it to do any of that stuff, so disable it.
    outerContext->SetAllowsTypeConversion (false);
    viewSymbolContext->SetAllowsTypeConversion (false);
    instanceContext->SetAllowsTypeConversion (false);

    return outerContext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExpressionFilterHandler::_DoConditionalDraw (ViewContextR viewContext, ElementHandleCP element, void const* data, size_t dataSize) const 
    {
    bool                doConditionalDraw = false;
    ExpressionAppData*  appData;
    DgnModelP        modelRef = viewContext.GetCurrentModel();
    DgnFileP            dgnfile = modelRef ? modelRef->GetDgnFileP() : NULL;

    if (NULL != dgnfile && NULL != (appData = ExpressionAppData::GetForFile (*dgnfile)))
        {
        UInt32 exprId = *((UInt32*)data);

        NodeP node = appData->GetExpression (exprId);
        if (NULL != node)
            {
            DisplayFilterExpressionContext* exprContext = (DisplayFilterExpressionContext*)viewContext.GetConditionalDrawExpressionContext();
            if (NULL == exprContext)
                {
                exprContext = DisplayFilterExpressionContext::Create (viewContext);
                viewContext.SetConditionalDrawExpressionContext (*exprContext);
                }
            else
                exprContext->Sync (viewContext);

            Transform t;
            if (sizeof(UInt32)+sizeof(Transform) == dataSize)
                {
                BeAssert (appData->ExpressionRequiresTransform (exprId));
                byte* bytes = (byte*)data;
                t = *((Transform*)(bytes + sizeof(UInt32)));
                }
            else
                {
                BeAssert (!appData->ExpressionRequiresTransform (exprId));
                t.InitIdentity();
                }

            exprContext->PushTransform (t);
            ECN::ValueResultPtr result;
            ECN::ECValue v;

            if (ECN::ExprStatus_Success == node->GetValue (result, *exprContext, true, true)
                && ECN::ExprStatus_Success == result->GetECValue (v) && v.ConvertToPrimitiveType (ECN::PRIMITIVETYPE_Boolean))
                {
                doConditionalDraw = !v.IsNull() && true == v.GetBoolean();
                }

            exprContext->PopTransform();
            }
        }

    return doConditionalDraw;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayFilter::TestExpression::_GetData (bvector<byte>& data) const
    {
    UInt32          offsetId = m_expressionId;

    byte const* bytes = (byte const*)(&offsetId);
    data.insert (data.end(), bytes, bytes + sizeof(offsetId));
    if (m_storesTransform)
        {
        Transform t;
        t.InitIdentity();
        bytes = (byte const*)(&t);
        data.insert (data.end(), bytes, bytes + sizeof(t));
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilter::OperatorPtr     DisplayFilter::TestExpression::Create (WCharCP expr, DgnFileR dgnfile)
    {
    ExpressionAppData* appData = ExpressionAppData::GetForFile (dgnfile);
    UInt32 exprId;
    if (NULL == appData || -1 == (exprId = appData->GetExpressionId (expr)))
        { BeAssert (false); return NULL; }

    return new TestExpression (exprId, appData->ExpressionRequiresTransform (exprId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilter::OperatorPtr      DisplayFilter::TestExpression::CreateViewSizeTest (DPoint3dCR origin, double pixelSize, bool largerThan, DgnFileR dgnfile)
    {
    // This is a convenience method to handle formatting the arguments into an expression string
    WCharCP comparisonOp = largerThan ? L">" : L"<=";
    WString expr;
    expr.Sprintf (L"ViewContext.GetPixelSizeAt(%.13g,%.13g,%.13g) %ls %.13g", origin.x, origin.y, origin.z, comparisonOp, pixelSize);
    return Create (expr.c_str(), dgnfile);
    }


//*=================================================================================**//**
//! @bsiclass                                                     RayBentley      08/2013
//===============+===============+===============+===============+===============+======*/                                                                        
struct  PresentationFormStringOperator : DisplayFilter::Operator
{
    WString     m_string;
    
    PresentationFormStringOperator (DisplayFilterHandlerId id, WCharCP string) : Operator (id), m_string (string) { }
    static DisplayFilter::OperatorPtr Create (DisplayFilterHandlerId id, WCharCP string) { return new PresentationFormStringOperator (id, string); }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void                _GetData (bvector<byte>& data) const 
    {
    size_t      bytes = (m_string.size() + 1) * sizeof (WChar);
    byte*       pByte = (byte*) m_string.c_str();

    data.insert (data.end(), pByte, pByte + bytes);
    }


};  // PresentationFormStringOperator

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilter::OperatorPtr   DisplayFilter::CreatePresentationFormIdTest (WCharCP formId)
    {
    return  PresentationFormStringOperator::Create (DisplayFilterHandlerId_PresentationFormId, formId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilter::OperatorPtr   DisplayFilter::CreatePresentationFormFlagTest (WCharCP flag)
    {
    return PresentationFormStringOperator::Create (DisplayFilterHandlerId_PresentationFormFlag, flag);
    }



/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/                                                                        
struct  ViewFlagFilterHandler : DisplayFilterHandler
{

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    _DoConditionalDraw (ViewContextR viewContext, ElementHandleCP element, void const* data, size_t dataSize) const override
    {
    UInt16 const* opSize = (UInt16 const*) data;

    if (dataSize != (sizeof (UInt16) + sizeof (ViewFlagFilterData)) || *opSize != sizeof (ViewFlagFilterData))
        {
        BeAssert (false);
        return true;
        }

    ViewFlagFilterData*     filterData = (ViewFlagFilterData*) (opSize + 1);     
    ViewFlagsCP             viewFlags;

    if (NULL == (viewFlags = viewContext.GetIViewDraw().GetDrawViewFlags()))        // Note - Get flags from IViewDraw - not IDrawGeom (as ViewContext::GetViewFlags)
        {                                                                            // as we want the real flags - not the faux ones created for ICachedDraw.
        BeAssert (false);
        return true;
        }

    bool                state;

    switch (filterData->m_flag)
        {
        case DisplayFilter::ViewFlag_BoundaryDisplay: 
            state = viewFlags->refBoundaryDisplay;
            break;
            
        case DisplayFilter::ViewFlag_Constructions:  
            state = viewFlags->constructs;
             break;

        case DisplayFilter::ViewFlag_Dimensions:     
            state = viewFlags->dimens;
            break;

        case DisplayFilter::ViewFlag_DataFields:     
            state = viewFlags->ed_fields;
            break;

        case DisplayFilter::ViewFlag_Fill:           
            state = viewFlags->fill;
            break;

        case DisplayFilter::ViewFlag_Grid:           
            state = viewFlags->grid;
            break;

        case DisplayFilter::ViewFlag_LevelOverrides:
            state = viewFlags->lev_symb;
            break;

        case DisplayFilter::ViewFlag_LineStyles:     
            state = !viewFlags->inhibitLineStyles;
            break;

        case DisplayFilter::ViewFlag_LineWeights: 
            state = viewFlags->line_wghts;
            break;

        case DisplayFilter::ViewFlag_Patterns:       
            state = viewFlags->patterns;
            break;

        case DisplayFilter::ViewFlag_Tags: 
            state = !viewFlags->tagsOff;
            break;

        case DisplayFilter::ViewFlag_Text:           
            state = !viewFlags->fast_text;
            break;

        case DisplayFilter::ViewFlag_Textnodes:      
            state = viewFlags->text_nodes;
            break;

        case DisplayFilter::ViewFlag_Transparency:
            state = viewFlags->transparency;
            break;

        default:
            BeAssert (false);
            return false;
        }

    return state == filterData->m_testState;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual     WString     _GetDumpString (void const* data, size_t dataSize, DgnModelP) const override
    {
    return WString ("View Flag Test");
    }

};  // ViewFlagFilterHandler


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/                                                                        
struct  ViewParameterFilterHandler : DisplayFilterHandler
{
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static bool   DoValueTest (Int32 currentValue, Int32 testValue, DisplayFilter::TestMode testMode)
    {
    switch (testMode)
        {
        case DisplayFilter::TestMode_Equal:
            return currentValue == testValue;

        case DisplayFilter::TestMode_NotEqual:
            return currentValue != testValue;

        case DisplayFilter::TestMode_LessThan:
            return currentValue < testValue;

        case DisplayFilter::TestMode_GreaterThan:
            return currentValue > testValue;

        default:
            BeAssert (false);
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    _DoConditionalDraw (ViewContextR viewContext, ElementHandleCP element, void const* data, size_t dataSize) const override
    {
    UInt16 const* opSize = (UInt16 const*) data;

    if (dataSize != (sizeof (UInt16) + sizeof (ViewParameterFilterData)) || *opSize != sizeof (ViewParameterFilterData))
        {
        BeAssert (false);
        return true;
        }

    Int32                           currentValue;
    ViewParameterFilterData*        filterData = (ViewParameterFilterData*) (opSize + 1);     

    if (NULL == viewContext.GetViewport())
        return DisplayFilter::TestMode_NotEqual == filterData->m_testMode;

    Int32          currentValue;

    switch (filterData->m_parameter)
        {
        case  ViewParameterFilterData::Parameter_DrawPurpose:
            currentValue = static_cast<Int32>(viewContext.GetDrawPurpose());
            break;

        case ViewParameterFilterData::Parameter_RenderMode:
            {
            ViewFlagsCP viewFlags = viewContext.GetIViewDraw().GetDrawViewFlags();    // Note - Get flags from IViewDraw - not IDrawGeom (as ViewContext::GetViewFlags)
                                                                                      // as we want the real flags - not the faux ones created for ICachedDraw.

            if (NULL == viewContext.GetViewport() || NULL == viewFlags)
                return DisplayFilter::TestMode_NotEqual == filterData->m_testMode;

            currentValue = viewFlags->renderMode;
            break;
            }

        case ViewParameterFilterData::Parameter_ClipVolumePass:
            if (viewContext.GetDynamicViewStateStack().empty())
                return DisplayFilter::TestMode_NotEqual == filterData->m_testMode;

            currentValue = static_cast<Int32> (viewContext.GetDynamicViewStateStack().back().GetPass());
            break;    

        case ViewParameterFilterData::Parameter_InViewlet:
            currentValue = (UInt32) viewContext.InViewlet();
            break;


#ifdef NEEDS_WORK
        case ViewParameterFilterData::Parameter_ViewOrientation:
            if (NULL == viewContext.GetViewport())
                return DisplayFilter::TestMode_NotEqual == filterData->m_testMode;

            currentValue = getViewOrientation (viewContext);
            break;

        case ViewParameterFilterData::Parameter_DrawingAttachmentType:
            currentValue = (UInt32) getDrawingAttachmentType (viewContext);
            break;
#endif

        default:
            BeAssert (false);
            return false;
        }
    return DoValueTest (currentValue, filterData->m_testValue, filterData->m_testMode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual     WString     _GetDumpString (void const* data, size_t dataSize, DgnModelP) const override
    {
    UInt16 const* opSize = (UInt16 const*) data;

    if (dataSize != (sizeof (UInt16) + sizeof (ViewParameterFilterData)) || *opSize != sizeof (ViewParameterFilterData))
        {
        BeAssert (false);
        return WString();
        }

    ViewParameterFilterData*     filterData = (ViewParameterFilterData*) (opSize + 1);     
    static WString      s_parameterStrings[] = { WString (L"Draw Purpose"),  WString (L"Orientation"),  WString (L"RenderMode"), WString (L"Clip Pass"), WString ("In Viewlet"), WString ("Attachment Type") };
    static WString      s_testModeStrings[]  = { WString (L"Equal"),  WString (L"Not Equal"),  WString (L"Less Than"),  WString (L"Greater Than") };

    WString             valueString;

    valueString.Sprintf (L"%d", filterData->m_testValue);

    return WString (L"View Parameter Display Filter: ") + s_parameterStrings[filterData->m_parameter - 1] + 
           WString (L" Test Mode: " ) + s_testModeStrings[filterData->m_testMode - 1] +
           WString (L" Test Value: " ) + valueString;
    }

};  //  ViewParameterFilterHandler



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayFilter::RegisterCoreHandlers ()   
    { 
    static ExpressionFilterHandler          s_ecexpressionFilterHandler;
    static PresentationFormIdHandler        s_presentationFormIdHandler;
    static PresentationFormFlagHandler      s_presentationFormFlagHandler;
    static ViewParameterFilterHandler       s_viewParameterHandler;
    static ViewFlagFilterHandler            s_viewFlagFilterHandler;

    DisplayFilterHandlerManager::GetManager().RegisterHandler (DisplayFilterHandlerId_ECExpression,         s_ecexpressionFilterHandler);
    DisplayFilterHandlerManager::GetManager().RegisterHandler (DisplayFilterHandlerId_PresentationFormId,   s_presentationFormIdHandler);
    DisplayFilterHandlerManager::GetManager().RegisterHandler (DisplayFilterHandlerId_PresentationFormFlag, s_presentationFormFlagHandler);
    DisplayFilterHandlerManager::GetManager().RegisterHandler (DisplayFilterHandlerId_Parameter,            s_viewParameterHandler);
    DisplayFilterHandlerManager::GetManager().RegisterHandler (DisplayFilterHandlerId_ViewFlag,             s_viewFlagFilterHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsHandler::EnsureExpressionsAreStored (XAttributeHandleCR xa, TransactionType type) const
    {
    if (TRANSACTIONTYPE_Action != type)
        return SUCCESS;

    StatusInt status = ERROR;

    ElementHandle eh (xa.GetElementRef());
    ExpressionAppData* appdata = NULL != eh.GetDgnFileP() ? ExpressionAppData::GetForFile (*eh.GetDgnFileP()) : NULL;
    if (NULL != appdata)
        {
        XGraphicsContainer container (xa.PeekData(), (size_t)xa.GetSize());
        status = container.OnWriteToElement (eh);
        if (!appdata->FlushPendingExpressions())
            status = ERROR;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Whenever we write XGraphics to an element, we ensure any DisplayFilter expressions
* are written to the lookup table in the file as well.
* We could be cheap and simply flush the entire in-memory table on each write, but some
* expressions are transient (e.g. see XGraphicsContext::_DrawCurveVector()). So instead
* we only write those expressions which are (a) referenced in this XGraphics stream and
* (b) not already persisted.
* Typically this means the first time we write an element of a particular type, we also
* write all of its expressions; and each time thereafter we waste a bunch of time
* confirming that the expressions are already stored.
* @bsimethod                                                    Paul.Connelly   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsHandler::_OnPostModifyData (XAttributeHandleCR xAttr, TransactionType type)       { EnsureExpressionsAreStored (xAttr, type); }
void XGraphicsHandler::_OnPostReplaceData (XAttributeHandleCR xAttr, TransactionType type)      { EnsureExpressionsAreStored (xAttr, type); }
void XGraphicsHandler::_OnPostAdd (XAttributeHandleCR xAttr, TransactionType type)              { EnsureExpressionsAreStored (xAttr, type); }

#endif
