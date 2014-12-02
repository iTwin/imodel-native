/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/SchemaLayoutElementHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnHandlers/DgnECManager.h>
#include    <map>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

using namespace std;

#define XATTRIBUTEID_ECXDSchemaLayout    0xEC3A 

#define ECXDLAYOUT_SCHEMAINDEX               0

#define ECXDLAYOUT_CLASSINDEX_SchemaLayout   0
#define ECXDLAYOUT_CLASSINDEX_ClassLayout    1
#define ECXDLAYOUT_CLASSINDEX_PropertyLayout 2


USING_NAMESPACE_EC
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

static WCharCP      SCHEMALAYOUT_CLASSNAME    = L"SchemaLayout";
static WCharCP      CLASSLAYOUT_CLASSNAME     = L"ClassLayout";
static WCharCP      PROPERTYLAYOUT_CLASSNAME  = L"PropertyLayout";

ELEMENTHANDLER_DEFINE_MEMBERS(SchemaLayoutElementHandler)

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECXDLayoutSchemaProvider : ECN::IStandaloneEnablerLocater
{
private: 

    // The Schema used to persist SchemaLayout data
    ECSchemaPtr             m_schema;
    // The enabler used to store SchemaLayout header data
    ClassLayoutCP           m_schemaClassLayout;
    ECXDInstanceEnablerPtr   m_schemaClassLayoutEnabler;

    // The enabler used to store ClassLayout data
    ClassLayoutCP           m_classLayout;
    ECXDInstanceEnablerPtr   m_classLayoutEnabler;

    // The enabler used to store PropertyLayout data
    ClassLayoutCP           m_propertyClassLayout;
    ECXDInstanceEnablerPtr   m_propertyLayoutEnabler;

    WString         GetSchemaXml ();
    ECSchemaCR      GetSchema ();

    ECClassCR       GetSchemaLayoutClass ();
    ECClassCR       GetClassLayoutClass ();
    ECClassCR       GetPropertyLayoutClass ();

    ECXDLayoutSchemaProvider ();
    virtual ~ECXDLayoutSchemaProvider () {}

    static ECXDLayoutSchemaProvider*& PeekInstance ();

    virtual    ECN::StandaloneECEnablerPtr  _LocateStandaloneEnabler (ECN::SchemaKeyCR schemaKey, const wchar_t* className) override;
public: 

/*__PUBLISH_SECTION_START__*/
    static ECXDLayoutSchemaProvider& GetInstance ();
    static void                     ClearInstance ();

    ECXDInstanceEnablerR     GetSchemaLayoutEnabler ();
    ECXDInstanceEnablerR     GetClassLayoutEnabler ();
    ECXDInstanceEnablerR     GetPropertyLayoutEnabler ();

    bool                IsInternalECClass (SchemaClassIndexPair& indexOut, ECClassCR ecClass);
    bool                IsInternalECClass (SchemaClassIndexPair& indexOut, WCharCP schemaName, WCharCP className);

}; // ECXDLayoutSchemaProvider

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDLayoutSchemaProvider*& ECXDLayoutSchemaProvider::PeekInstance ()
    {
    static ECXDLayoutSchemaProvider* s_instance = NULL;

    return s_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDLayoutSchemaProvider& ECXDLayoutSchemaProvider::GetInstance ()
    {
    ECXDLayoutSchemaProvider*& instance = PeekInstance();

    if (NULL == instance)
        instance = new ECXDLayoutSchemaProvider();

    return *instance;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDLayoutSchemaProvider::ClearInstance ()
    {
    ECXDLayoutSchemaProvider*& instance = PeekInstance();

    if (NULL == instance)
        return;

    delete instance;
    instance = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */      ECXDLayoutSchemaProvider::ECXDLayoutSchemaProvider ()
    :
    m_schema(NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString         ECXDLayoutSchemaProvider::GetSchemaXml ()
    {
    WChar xmlStr[] = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       L"<ECSchema schemaName=\"ECXDLayoutSchema\" nameSpacePrefix=\"ecxdata\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                       L"    <ECClass typeName=\"SchemaLayout\" isDomainClass=\"True\">"
                       L"        <ECProperty propertyName=\"SchemaIndex\"   typeName=\"int\" />"
                       L"        <ECProperty propertyName=\"SchemaName\"    typeName=\"string\" />"
                       L"    </ECClass>"
                       L"    <ECClass typeName=\"ClassLayout\" isDomainClass=\"True\">"
                       L"        <ECProperty propertyName=\"ClassIndex\"                typeName=\"int\" />"
                       L"        <ECProperty propertyName=\"ClassName\"                 typeName=\"string\" />"
                       L"        <ECArrayProperty propertyName=\"PropertyLayoutArray\"  typeName=\"PropertyLayout\" />"
                       L"    </ECClass>"
                       L"    <ECClass typeName=\"TypeDescriptor\" isDomainClass=\"False\" isStruct=\"True\">"
                       L"        <ECProperty propertyName=\"TypeKind\"      typeName=\"int\" />"
                       L"        <ECProperty propertyName=\"TypeKindQualifier\" typeName=\"int\" />"
                       L"    </ECClass>"                       
                       L"    <ECClass typeName=\"PropertyLayout\" isDomainClass=\"True\" isStruct=\"True\">"
                       L"        <ECProperty propertyName=\"AccessString\"      typeName=\"string\" />"
                       L"        <ECProperty propertyName=\"ParentStructIndex\" typeName=\"int\" />"
                       L"        <ECStructProperty propertyName=\"Type\"        typeName=\"TypeDescriptor\" />"
                       L"        <ECProperty propertyName=\"Offset\"            typeName=\"int\" />"
                       L"        <ECProperty propertyName=\"NullFlagsOffset\"   typeName=\"int\" />"
                       L"        <ECProperty propertyName=\"NullFlagsBitMask\"  typeName=\"int\" />"
                       L"    </ECClass>"
                       L"</ECSchema>";

    return xmlStr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCR      ECXDLayoutSchemaProvider::GetSchema ()
    {
    if (m_schema.IsNull())
        {
        //SharedSchemaCacheR  owner = DgnECManager::GetManager().GetSharedSchemaCache();

        WString  schemaXml = GetSchemaXml();

        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
        schemaContext->HideSchemasFromLeakDetection();

        ECSchema::ReadFromXmlString (m_schema, schemaXml.c_str(), *schemaContext);
        }

    return *m_schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr  ECXDLayoutSchemaProvider::_LocateStandaloneEnabler (ECN::SchemaKeyCR schemaKey, const wchar_t* className)
    {
    if (schemaKey != m_schema->GetSchemaKey())
        return NULL;
                 
    if (0 == BeStringUtilities::Wcsicmp(className, SCHEMALAYOUT_CLASSNAME))
        return &ECXDLayoutSchemaProvider::GetSchemaLayoutEnabler().GetStandaloneInstanceEnabler();

    if (0 == BeStringUtilities::Wcsicmp(className, CLASSLAYOUT_CLASSNAME))
        return &ECXDLayoutSchemaProvider::GetClassLayoutEnabler().GetStandaloneInstanceEnabler();

    if (0 == BeStringUtilities::Wcsicmp(className, PROPERTYLAYOUT_CLASSNAME))
        return &ECXDLayoutSchemaProvider::GetPropertyLayoutEnabler().GetStandaloneInstanceEnabler();

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR       ECXDLayoutSchemaProvider::GetSchemaLayoutClass ()
    {
    return *GetSchema().GetClassP (SCHEMALAYOUT_CLASSNAME);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR       ECXDLayoutSchemaProvider::GetClassLayoutClass ()
    {
    return *GetSchema().GetClassP (CLASSLAYOUT_CLASSNAME);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR       ECXDLayoutSchemaProvider::GetPropertyLayoutClass ()
    {
    return *GetSchema().GetClassP (PROPERTYLAYOUT_CLASSNAME);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceEnablerR   ECXDLayoutSchemaProvider::GetSchemaLayoutEnabler ()
    {
    if (m_schemaClassLayoutEnabler.IsNull())
        {
        ECClassCR ecClass           = GetSchemaLayoutClass ();

        m_schemaClassLayout         = ClassLayout::BuildFromClass (ecClass, ECXDLAYOUT_CLASSINDEX_SchemaLayout, ECXDLAYOUT_SCHEMAINDEX, true);
        m_schemaClassLayoutEnabler  = ECXDInstanceEnabler::CreateInternalEnabler (ecClass, *m_schemaClassLayout, this);
        }

    return *m_schemaClassLayoutEnabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceEnablerR   ECXDLayoutSchemaProvider::GetClassLayoutEnabler ()
    {
    if (m_classLayoutEnabler.IsNull())
        {
        ECClassCR ecClass       = GetClassLayoutClass ();

        m_classLayout           = ClassLayout::BuildFromClass (ecClass, ECXDLAYOUT_CLASSINDEX_ClassLayout, ECXDLAYOUT_SCHEMAINDEX, true);
        m_classLayoutEnabler    = ECXDInstanceEnabler::CreateInternalEnabler (ecClass, *m_classLayout, this);
        }

    return *m_classLayoutEnabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceEnablerR   ECXDLayoutSchemaProvider::GetPropertyLayoutEnabler ()
    {
    if (m_propertyLayoutEnabler.IsNull())
        {
        ECClassCR ecClass            = GetPropertyLayoutClass ();

        m_propertyClassLayout        = ClassLayout::BuildFromClass (ecClass, ECXDLAYOUT_CLASSINDEX_PropertyLayout, ECXDLAYOUT_SCHEMAINDEX, true);
        m_propertyLayoutEnabler      = ECXDInstanceEnabler::CreateInternalEnabler (ecClass, *m_propertyClassLayout, this);
        }

    return *m_propertyLayoutEnabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECXDLayoutSchemaProvider::IsInternalECClass (SchemaClassIndexPair& indexOut, ECClassCR ecClass)
    {
    if (0 != wcscmp (L"ECXDLayoutSchema", ecClass.GetSchema().GetName().c_str()))
        return false;

    if (&GetSchemaLayoutClass() == &ecClass)
        {
        indexOut.first  = ECXDLAYOUT_SCHEMAINDEX;
        indexOut.second = ECXDLAYOUT_CLASSINDEX_SchemaLayout;

        return true;
        }

    if (&GetClassLayoutClass() == &ecClass)
        {
        indexOut.first  = ECXDLAYOUT_SCHEMAINDEX;
        indexOut.second = ECXDLAYOUT_CLASSINDEX_ClassLayout;

        return true;
        }

    if (&GetPropertyLayoutClass() == &ecClass)
        {
        indexOut.first  = ECXDLAYOUT_SCHEMAINDEX;
        indexOut.second = ECXDLAYOUT_CLASSINDEX_PropertyLayout;

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECXDLayoutSchemaProvider::IsInternalECClass (SchemaClassIndexPair& indexOut, WCharCP schemaName, WCharCP className)
    {
    if (0 != wcscmp (L"ECXDLayoutSchema", schemaName))
        return false;

    ECSchemaCR  internalSchema = GetSchema();

    if (0 != wcscmp (internalSchema.GetName().c_str(), schemaName))
        return false;

    ECClassP    internalClass = internalSchema.GetClassP(className);

    if (NULL == internalClass)
        return false;

    return IsInternalECClass (indexOut, *internalClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaLayoutElementHandler::Register ()
    {
    ElementHandlerId    classLayoutHID (XATTRIBUTEID_ECXDSchemaLayout, 0);
    DgnSystemDomain::GetInstance().RegisterHandler (classLayoutHID, ELEMENTHANDLER_INSTANCE(SchemaLayoutElementHandler));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                SchemaLayoutElementHandler::_GetTypeName (WStringR string, UInt32 desiredLength)
    {
    string.assign (ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::IDS_TYPENAMES_SCHEMALAYOUT));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                SchemaLayoutElementHandler::_GetDescription (ElementHandleCR el, WStringR string, UInt32 desiredLength)
    {
    _GetTypeName (string, desiredLength);

    DgnElementECInstancePtr    schemaLayoutInstance = FindSchemaLayoutInstance (el);

    if (schemaLayoutInstance.IsNull())
        return;

    WString     schemaName;

    DataFromSchemaLayoutInstance (&schemaName, NULL, *schemaLayoutInstance);

    if (wcslen (schemaName.c_str()) > 0)
        string.append(L": ").append(schemaName);
    }

#ifdef DGNV10FORMAT_CHANGES_WIP
/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClassLayoutTxnCustomEntry : RefCounted<ITxn::ICustomEntry>
{    
private:
    SchemaIndex m_schemaIndex;
    ClassIndex  m_classIndex;
    DgnProjectR    m_dgnFile;
   
    ClassLayoutTxnCustomEntry (SchemaIndex si, ClassIndex ci, DgnProjectR f) : m_schemaIndex(si), m_classIndex(ci), m_dgnFile(f) {}

    virtual void _OnReverse()   override {ECXDProvider::GetProvider().OnUndoRedoClassLayout(m_schemaIndex, m_classIndex, m_dgnFile, true);}
    virtual void _OnReinstate() override {ECXDProvider::GetProvider().OnUndoRedoClassLayout(m_schemaIndex, m_classIndex, m_dgnFile, false);}

public:
    static RefCountedPtr<ClassLayoutTxnCustomEntry> Create (SchemaIndex schemaIndex, ClassIndex classIndex, DgnProjectR file)
        {
        return new ClassLayoutTxnCustomEntry (schemaIndex, classIndex, file);
        }
};
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            SchemaLayoutElementHandler::OnUndoRedoSchemaLayoutInstance (XAttributeHandleCR xAttr, bool isUndo)
    {
    // The creation of a ClassLayout has been reversed or reinstated.  This needs to be
    // reflected in the PerSchemaCache.

    ElementRefP         elemRef              = xAttr.GetElementRef();
    DgnModelP           dgnModel             = elemRef->GetDgnModelP();

    ECXDInstanceEnablerR enabler              = ECXDLayoutSchemaProvider::GetInstance().GetSchemaLayoutEnabler ();
    DgnElementECInstancePtr    schemaLayoutInstance = enabler.CreateInstance (*dgnModel, elemRef, xAttr.GetId());

    WString             schemaName;
    SchemaIndex         schemaIndex;

   if (SUCCESS != DataFromSchemaLayoutInstance (&schemaName, &schemaIndex, *schemaLayoutInstance))
        {
        BeAssert (false); // if we get a malformed instance here we should be able to track down how it happened
        return;
        }
    
    ECXDProvider::GetProvider().OnUndoRedoSchemaLayout(schemaName.c_str(), schemaIndex, elemRef, isUndo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            SchemaLayoutElementHandler::_OnUndoRedoXAttributeChange
(
XAttributeHandleCR  xAttr,
ChangeTrackAction action,
bool                isUndo,
ChangeTrackSource source
)
    {
    // When a schema is used by the ECXDProvider it is assigned a SchemaIndex.  The
    // index is persisted in the schema layout (as an ECXDInstance) and cached in the
    // ECXDPerFileCache.  When the schema layout is reversed or reinstated, we need to
    // maintain the cache.

    if (ChangeTrackAction::XAttributeAdd != action)
        return;

    if (ECXDInstanceXAttributeHandler::GetHandler().GetId() != xAttr.GetHandlerId())
        return;

    SchemaClassIndexPair indexPair = ECXDProvider::GetProvider().GetTypeIndiciesFromECXData (xAttr.PeekData());

    if (ECXDLAYOUT_SCHEMAINDEX != indexPair.first)
        return;

    // We would also be listening for ECXDLAYOUT_CLASSINDEX_ClassLayout changes here... but we can't.
    // See notes in: AddClassLayoutInstanceToElement
    if (ECXDLAYOUT_CLASSINDEX_SchemaLayout != indexPair.second)
        return;

    OnUndoRedoSchemaLayoutInstance (xAttr, isUndo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SchemaLayoutElementHandler::IsInternalECClass (SchemaClassIndexPair& index, ECClassCR ecClass)
    {
    return ECXDLayoutSchemaProvider::GetInstance().IsInternalECClass (index, ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SchemaLayoutElementHandler::IsInternalECClass (SchemaClassIndexPair& indexOut, WCharCP schemaName, WCharCP className)
    {
    return ECXDLayoutSchemaProvider::GetInstance().IsInternalECClass (indexOut, schemaName, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceEnablerP SchemaLayoutElementHandler::GetInstanceEnabler (SchemaClassIndexPair const& index)
    {
    if (ECXDLAYOUT_SCHEMAINDEX != index.first)
        return NULL;

    switch (index.second)
        {
        case ECXDLAYOUT_CLASSINDEX_SchemaLayout:   return &ECXDLayoutSchemaProvider::GetInstance().GetSchemaLayoutEnabler();   break;
        case ECXDLAYOUT_CLASSINDEX_ClassLayout:    return &ECXDLayoutSchemaProvider::GetInstance().GetClassLayoutEnabler();    break;
        case ECXDLAYOUT_CLASSINDEX_PropertyLayout: return &ECXDLayoutSchemaProvider::GetInstance().GetPropertyLayoutEnabler(); break;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayoutElementHandler::DataFromPropertyLayoutInstance
(
WString&            accessString,
UInt32&             parentStructIndex,
ECTypeDescriptor&   typeDescriptor,
UInt32&             offset,
UInt32&             nullFlagsOffset,
NullflagsBitmask&   nullFlagsBitMask,
IECInstanceCR       propertyLayoutInstance
)
    {
    ECValue value;

    propertyLayoutInstance.GetValue (value, L"AccessString");
    if ( ! value.IsString() || value.IsNull() )
        { BeAssert (0); return ERROR; }

    accessString = value.GetString();

    propertyLayoutInstance.GetValue (value, L"ParentStructIndex");
    if ( ! value.IsInteger())
        { BeAssert (0); return ERROR; }

    parentStructIndex = value.GetInteger();

    propertyLayoutInstance.GetValue (value, L"Type.TypeKind");
    if ( ! value.IsInteger())
        { BeAssert (0); return ERROR; }

    ValueKind kind = (ValueKind) value.GetInteger();
    
    propertyLayoutInstance.GetValue (value, L"Type.TypeKindQualifier");
    if ( ! value.IsInteger())
        { BeAssert (0); return ERROR; }

    short qualifier = (short) value.GetInteger();    

    typeDescriptor = ECTypeDescriptor (kind, qualifier);  

    if (typeDescriptor.IsStruct())
        {
        offset           = 0;
        nullFlagsOffset  = 0;
        nullFlagsBitMask = 0;

        return SUCCESS;
        }

    propertyLayoutInstance.GetValue (value, L"Offset");
    if ( ! value.IsInteger())
        { BeAssert (0); return ERROR; }

    offset = value.GetInteger();

    propertyLayoutInstance.GetValue (value, L"NullFlagsOffset");
    if ( ! value.IsInteger())
        { BeAssert (0); return ERROR; }

    nullFlagsOffset = value.GetInteger();

    propertyLayoutInstance.GetValue (value, L"NullFlagsBitMask");
    if ( ! value.IsInteger())
        { BeAssert (0); return ERROR; }

    nullFlagsBitMask = value.GetInteger();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayoutElementHandler::DataFromClassLayoutInstance
(
WStringP        className, 
ClassIndex*     classIndex, 
IECInstanceCR   classLayoutInstance
)
    {
    ECValue value;

    if (NULL != className)
        {
        classLayoutInstance.GetValue (value, L"ClassName");
        if ( ! value.IsString())
            { BeAssert (0); return ERROR; }

        *className = value.GetString();
        }

    if (NULL != classIndex)
        {
        classLayoutInstance.GetValue (value, L"ClassIndex");
        if ( ! value.IsInteger())
            { BeAssert (0); return ERROR; }

        *classIndex = static_cast<ClassIndex>(value.GetInteger());
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayoutElementHandler::DataFromSchemaLayoutInstance
(
WStringP        schemaName,
SchemaIndex*    schemaIndex, 
IECInstanceCR   schemaLayoutInstance
)
    {
    ECValue value;

    if (schemaName)
        {
        schemaLayoutInstance.GetValue (value, L"SchemaName");
        if ( ! value.IsString())
            { BeAssert (0); return ERROR; }

        *schemaName = value.GetString();
        }

    if (schemaIndex)
        {
        schemaLayoutInstance.GetValue (value, L"SchemaIndex");
        if ( ! value.IsInteger())
            { BeAssert (0); return ERROR; }

        *schemaIndex = static_cast<ClassIndex>(value.GetInteger());
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementECInstancePtr    SchemaLayoutElementHandler::FindSchemaLayoutInstance (ElementHandleCR element)
    {
    ECXDInstanceEnablerCR    schemaLayoutEnabler = ECXDLayoutSchemaProvider::GetInstance().GetSchemaLayoutEnabler ();

    // Search for the SchemaLayout XData xattr
    DgnElementECInstanceVector schemaLayoutInstances;

    ECXDInstanceFilteredFinderPtr    finder = ECXDInstanceFilteredFinder::CreateFinder();

    finder->AddEnabler(schemaLayoutEnabler);
    finder->FindElementInstances (schemaLayoutInstances, element, element.GetDgnModel());

    BeAssert (1 == schemaLayoutInstances.size() && "SchemaLayoutElement should have exactly one SchemaLayoutInstance");

    if (0 >= schemaLayoutInstances.size())
        NULL;

    return schemaLayoutInstances[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    SchemaLayoutElementHandler::VisitSchemaLayoutElements (SchemaLayoutElementVisitor& visitor, DgnProjectCR dgnFile)
    {
    DgnModelP             dictionaryModel     = (const_cast <DgnProjectR> (dgnFile)).GetDictionaryModel();

    // Go through the Dictionary model looking for SchemaLayout elements
    FOR_EACH (PersistentElementRefP elemRef , *dictionaryModel->GetControlElementsP())
        {
        ElementHandle eh (elemRef);
        HandlerR handler = eh.GetHandler();
        if (&handler != this)
            continue;

        DgnElementECInstancePtr    schemaLayoutInstance = FindSchemaLayoutInstance (ElementHandle (elemRef, dictionaryModel));

        if (schemaLayoutInstance.IsNull())
            continue;

        SchemaIndex schemaIndex;
        WString     schemaName;

        DataFromSchemaLayoutInstance (&schemaName, &schemaIndex, *schemaLayoutInstance);

        if (visitor.VisitSchemaLayoutElement (schemaName.c_str(), schemaIndex, elemRef))
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP SchemaLayoutElementHandler::FindSchemaLayoutElement (SchemaIndex& schemaIndexOut, WCharCP schemaNameIn, DgnProjectCR dgnFile)
    {
    struct Visitor : SchemaLayoutElementVisitor
        {
        WCharCP   m_searchName;
        SchemaIndex m_foundIndex;
        ElementRefP m_foundElem;

        Visitor (WCharCP searchName) : m_searchName(searchName), m_foundIndex (0), m_foundElem (NULL) { }

        bool VisitSchemaLayoutElement (WCharCP schemaName, SchemaIndex schemaIndex, ElementRefP elemRef) override
            {
            // Is this the schemaLayout we are looking for?
            if (0 != wcscmp (m_searchName, schemaName))
                return false;

            m_foundIndex = schemaIndex;
            m_foundElem  = elemRef;

            return true;
            }
        };

    Visitor visitor (schemaNameIn);

    VisitSchemaLayoutElements (visitor, dgnFile);

    if (NULL == visitor.m_foundElem)
        return NULL;

    schemaIndexOut = visitor.m_foundIndex;

    return visitor.m_foundElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP SchemaLayoutElementHandler::FindSchemaLayoutElement (SchemaIndex schemaIndexIn, DgnProjectCR dgnFile)
    {
    struct Visitor : SchemaLayoutElementVisitor
        {
        SchemaIndex m_searchIndex;
        ElementRefP m_foundElem;

        Visitor (SchemaIndex searchIndex) : m_searchIndex(searchIndex), m_foundElem (NULL) { }

        bool VisitSchemaLayoutElement (WCharCP schemaName, SchemaIndex schemaIndex, ElementRefP elemRef) override
            {
            // Is this the schemaLayout we are looking for?
            if (m_searchIndex != schemaIndex)
                return false;

            m_foundElem  = elemRef;

            return true;
            }
        };

    Visitor visitor (schemaIndexIn);

    VisitSchemaLayoutElements (visitor, dgnFile);

    return visitor.m_foundElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutP    SchemaLayoutElementHandler::CreateClassLayout (ECN::IECInstanceR classLayoutInstance, SchemaIndex schemaIndex)
    {
    WString     className;
    ClassIndex  classIndex;

    DataFromClassLayoutInstance (&className, &classIndex, classLayoutInstance);

    // Build the classLayout from the data in the element
    ClassLayoutP        classLayout = ClassLayout::CreateEmpty (className.c_str(), classIndex, schemaIndex);

    ECValue value;
    UInt32  propertyCount = 0;

    classLayoutInstance.GetValue (value, L"PropertyLayoutArray");
    if (EXPECTED_CONDITION (value.IsArray()))
        propertyCount = value.GetArrayInfo().GetCount();

    for (UInt32 iProperty = 0; iProperty < propertyCount; iProperty++)
        {
        classLayoutInstance.GetValue (value, L"PropertyLayoutArray", iProperty);
        if ( ! EXPECTED_CONDITION (value.IsStruct()))
            continue;

        IECInstancePtr propertyLayoutInstance = value.GetStruct();

        WString          accessString;        
        UInt32           parentStructIndex;
        ECTypeDescriptor typeDescriptor;
        UInt32           offset;
        UInt32           nullFlagsOffset;
        NullflagsBitmask nullFlagsBitMask;

        DataFromPropertyLayoutInstance (accessString, parentStructIndex, typeDescriptor, offset, nullFlagsOffset, nullFlagsBitMask, *propertyLayoutInstance);

        classLayout->AddPropertyDirect (accessString.c_str(), parentStructIndex, typeDescriptor, offset, nullFlagsOffset, nullFlagsBitMask);
        }

    classLayout->FinishLayout();

    return classLayout;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaLayoutP   SchemaLayoutElementHandler::LoadSchemaLayout (ElementHandleCR schemaLayoutElem)
    {
    if ( ! EXPECTED_CONDITION (schemaLayoutElem.IsValid()))
        return NULL;

    if ( ! EXPECTED_CONDITION (&schemaLayoutElem.GetHandler() == this))
        return NULL;

    DgnElementECInstancePtr    schemaLayoutInstance = FindSchemaLayoutInstance (schemaLayoutElem);

    if (schemaLayoutInstance.IsNull())
        return NULL;

    SchemaIndex schemaIndex;

    DataFromSchemaLayoutInstance (NULL, &schemaIndex, *schemaLayoutInstance);

    SchemaLayoutR schemaLayout = *(SchemaLayout::Create (schemaIndex));

    // Find all the ClassLayout ECInstances
    ECXDInstanceEnablerCR    classLayoutEnabler = ECXDLayoutSchemaProvider::GetInstance().GetClassLayoutEnabler ();
    DgnElementECInstanceVector     classLayoutInstances;

    ECXDInstanceFilteredFinderPtr    finder = ECXDInstanceFilteredFinder::CreateFinder();

    finder->AddEnabler(classLayoutEnabler);
    finder->FindElementInstances (classLayoutInstances, schemaLayoutElem, schemaLayoutElem.GetDgnModel());

    FOR_EACH (DgnElementECInstancePtr classLayoutInstance , classLayoutInstances)
        {
        // Build the classLayout from the data in the element
        ClassLayoutP  classLayout = CreateClassLayout (*classLayoutInstance, schemaLayout.GetSchemaIndex());

        // Add the classLayout to the schemaLayout
        schemaLayout.AddClassLayout (*classLayout, classLayout->GetClassIndex());
        }

    return &schemaLayout;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementECInstancePtr    SchemaLayoutElementHandler::FindClassLayoutInstance (ClassIndex classIndexIn, ElementHandleCR element)
    {
    ECXDInstanceEnablerCR    classLayoutEnabler = ECXDLayoutSchemaProvider::GetInstance().GetClassLayoutEnabler ();

    // Search for the ClassLayout XData xattr
    DgnElementECInstanceVector classLayoutInstances;

    ECXDInstanceFilteredFinderPtr    finder = ECXDInstanceFilteredFinder::CreateFinder();

    finder->AddEnabler(classLayoutEnabler);
    finder->FindElementInstances (classLayoutInstances, element, element.GetDgnModel());

    FOR_EACH (DgnElementECInstancePtr classLayoutInstance , classLayoutInstances)
        {
        ClassIndex classIndex;

        DataFromClassLayoutInstance (NULL, &classIndex, *classLayoutInstance);

        if (classIndexIn == classIndex)
            return classLayoutInstance;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutP    SchemaLayoutElementHandler::ReadClassLayout (SchemaIndex schemaIndex, ClassIndex classIndex, DgnProjectR dgnFile)
    {
    ElementRefP schemaLayoutElem = FindSchemaLayoutElement (schemaIndex, dgnFile);

    if (NULL == schemaLayoutElem)
        return NULL;

    DgnElementECInstancePtr classLayoutInstance = FindClassLayoutInstance (classIndex, ElementHandle(schemaLayoutElem));

    return CreateClassLayout (*classLayoutInstance, schemaIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayoutElementHandler::AddPropertyLayoutToClassLayoutInstance (PropertyLayoutCR propertyLayout, UInt32 propIndex, IECInstanceR classLayoutInstance)
    {
    ECXDInstanceEnablerCR    enabler = ECXDLayoutSchemaProvider::GetInstance().GetPropertyLayoutEnabler ();
    StandaloneECInstancePtr wipInstance = enabler.GetPrivateWipInstance();

    ECValue value;

    value.SetString(propertyLayout.GetAccessString());
    wipInstance->SetValue (L"AccessString", value);

    value.SetInteger(propertyLayout.GetParentStructIndex());
    wipInstance->SetValue (L"ParentStructIndex", value);

    value.SetInteger(propertyLayout.GetTypeDescriptor().GetTypeKind());
    wipInstance->SetValue (L"Type.TypeKind", value);

    value.SetInteger(propertyLayout.GetTypeDescriptor().GetTypeKindQualifier());
    wipInstance->SetValue (L"Type.TypeKindQualifier", value);

    if ( ! propertyLayout.GetTypeDescriptor().IsStruct())
        {
        value.SetInteger(propertyLayout.GetOffset());
        wipInstance->SetValue (L"Offset", value);

        value.SetInteger(propertyLayout.GetNullflagsOffset());
        wipInstance->SetValue (L"NullFlagsOffset", value);

        value.SetInteger(propertyLayout.GetNullflagsBitmask());
        wipInstance->SetValue (L"NullFlagsBitMask", value);
        }

    value.SetStruct (wipInstance.get());

    return (SUCCESS == classLayoutInstance.SetValue (L"PropertyLayoutArray", value, propIndex)) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayoutElementHandler::AddClassLayoutInstanceToElement (ClassLayoutCR classLayout, ElementRefP elem)
    {
    ECXDInstanceEnablerCR    enabler = ECXDLayoutSchemaProvider::GetInstance().GetClassLayoutEnabler ();
    StandaloneECInstanceR   wipInstance = enabler.GetSharedWipInstance();
    ECValue value;

    value.SetString(classLayout.GetECClassName().c_str());
    wipInstance.SetValue (L"ClassName", value);

    value.SetInteger(classLayout.GetClassIndex());
    wipInstance.SetValue (L"ClassIndex", value);

    DgnElementECInstancePtr  newInstance;

    if (SUCCESS != enabler.CreateInstanceOnElement (&newInstance, wipInstance, *elem->GetDgnModelP(), elem))
        return ERROR;

    UInt32      numProperties = classLayout.GetPropertyCount();

    newInstance->AddArrayElements (L"PropertyLayoutArray", numProperties);

    for (UInt32 iProperty = 0; iProperty < numProperties; iProperty++)
        {
        PropertyLayoutCP propertyLayout = NULL;

        if (SUCCESS != classLayout.GetPropertyLayoutByIndex (propertyLayout, iProperty))
            continue;

        if (SUCCESS != AddPropertyLayoutToClassLayoutInstance (*propertyLayout, iProperty, *newInstance))
            BeAssert (false);
        }

#ifdef DGNV10FORMAT_CHANGES_WIP
    // When a classlayout 'add' is reversed or reinstated (undo/redo) we need to notify the ECXDProvider
    // to maintain the ECXDPerFileCache.  We can't do that notification in the normal '_OnUndoRedoXAttributeChange'
    // since when the ClassLayout instance is reinstated, the PropertyLayout instance won't exist yet and we won't
    // be able to read the class layout.
    // Here, we add an ITXN::CustomEntry to the transaction journal.  When the entry is notified of its
    // reinstatement, we know that both the class layout and all the property layouts exist.
    if (ITxnManager::GetCurrentTxn().SupportsUndoableChanges())
        {
        SchemaIndex schemaIndex = classLayout.GetSchemaIndex();
        ClassIndex  classIndex  = classLayout.GetClassIndex();
        DgnProjectP    dgnFile     = elem->GetDgnModelP()->GetDgnProject();

        ITxnManager::GetCurrentTxn().SaveCustomEntryInUndo (*ClassLayoutTxnCustomEntry::Create(schemaIndex, classIndex, *dgnFile));
        }
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayoutElementHandler::AddSchemaLayoutInstanceToElement (WCharCP schemaName, SchemaIndex schemaIndex, ElementRefP elem)
    {
    ECXDInstanceEnablerCR    enabler = ECXDLayoutSchemaProvider::GetInstance().GetSchemaLayoutEnabler ();
    StandaloneECInstanceR   wipInstance = enabler.GetSharedWipInstance();

    ECValue value;

    value.SetString(schemaName);
    wipInstance.SetValue (L"SchemaName", value);

    value.SetInteger(schemaIndex);
    wipInstance.SetValue (L"SchemaIndex", value);

    return (SUCCESS == enabler.CreateInstanceOnElement (NULL, wipInstance, *elem->GetDgnModelP(), elem)) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP SchemaLayoutElementHandler::AddNewSchemaLayoutElement ( WCharCP schemaName, SchemaIndex schemaIndex, DgnProjectR dgnFile)
    {
    // Create the type 107 element
    DgnModelP   dictionaryModel = dgnFile.GetDictionaryModel();
    EditElementHandle eeh;

    ExtendedNonGraphicsHandler::InitializeElement (eeh, NULL, dictionaryModel);

    eeh.ChangeElementHandler(*this);
    eeh.AddToModel ();

    BeAssert (NULL != dynamic_cast <SchemaLayoutElementHandler*> (&eeh.GetHandler()));
    //        DgnECManager::GetManager().GetLogger().debugv (L"Adding SchemaLayout element with index %d for %ls in %ls.", schemaIndex, schemaName, dgnFile->GetName().c_str());
    AddSchemaLayoutInstanceToElement (schemaName, schemaIndex, eeh.GetElementRef());
    return  eeh.GetElementRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayoutElementHandler::WriteClassLayout (ClassLayoutCR classLayout, WCharCP schemaName, DgnProjectR dgnFile)
    {
    SchemaIndex     schemaIndex;
    ElementRefP     elemRef = FindSchemaLayoutElement (schemaIndex, schemaName, dgnFile);

    if (NULL == elemRef)
        elemRef = AddNewSchemaLayoutElement (schemaName, classLayout.GetSchemaIndex(), dgnFile);
    else
        BeAssert (classLayout.GetSchemaIndex() == schemaIndex);

    if (NULL == elemRef)
        { BeAssert (false); return ERROR; }


#if defined (CHECK_REDUNDANT)
    // make sure we don't already have this class.
    ECXDInstanceEnablerCR    classLayoutEnabler = ECXDLayoutSchemaProvider::GetInstance().GetClassLayoutEnabler ();
    DgnElementECInstanceVector     classLayoutInstances;

    ECXDInstanceFilteredFinderPtr    finder = ECXDInstanceFilteredFinder::CreateFinder();

    finder->AddEnabler(classLayoutEnabler);
    finder->FindElementInstances (classLayoutInstances, elemRef, elemRef->GetDgnModelP());

    FOR_EACH (DgnElementECInstancePtr classLayoutInstance , classLayoutInstances)
        {
        WString     className;
        SchemaIndex classIndex;

        DataFromClassLayoutInstance (&className, &classIndex, *classLayoutInstance);

        if (0 == classLayout.GetECClassName().compare (className))
            BeAssert (false && "The name of this class is already in the SchemaLayoutElement");

        if (classLayout.GetClassIndex() == classIndex)
            BeAssert (false && "The index for this class is already in the SchemaLayoutElement");
        }
#endif


    if (SUCCESS != AddClassLayoutInstanceToElement (classLayout, elemRef))
        { BeAssert (false); return ERROR; }

    return SUCCESS;
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

