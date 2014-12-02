/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/DgnECPersistence.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnHandlers/DgnECPersistence.h>

#define DGNEC_SCHEMA_NAME L"dgn"
#define DGNEC_SCHEMA_MAJOR_VERSION 1
#define DGNEC_SCHEMA_MINOR_VERSION 1

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE_EC 
USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECStandardCategoryHelper::Initialize (ECDbR ecDb) 
    {
    ecDb.GetEC().GetSchemaManager().GetECClass (m_categoryClass, "EditorCustomAttributes", "Category");
    BeAssert (m_categoryClass != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECStandardCategoryHelper::GetCategory (StandardCategory standard)
    {
    CategoriesByStandard::iterator it = m_categoriesByStandard.find (standard);
    if (it != m_categoriesByStandard.end())
        return it->second;
    POSTCONDITION (m_categoryClass != NULL, NULL);

    IECInstancePtr instance = m_categoryClass->GetDefaultStandaloneEnabler()->CreateInstance();
    SetStringValue (*instance, L"Name", GetName (standard));
    SetStringValue (*instance, L"DisplayLabel", GetDisplayLabel (standard).c_str());
    SetIntegerValue (*instance, L"Priority", GetPriority (standard));
    SetBooleanValue (*instance, L"Expand", GetDefaultExpand (standard));

    m_categoriesByStandard.Insert ((int) standard, instance);
    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECStandardCategoryHelper::GetDisplayLabel (StandardCategory standard)
    {
    DgnHandlersMessage::Number displayLabelMsgId;
    switch (standard)
        {
        case General:
            displayLabelMsgId = DgnHandlersMessage::MSGID_ECPROPERTYCATEGORY_General;
        case Extended:
            displayLabelMsgId = DgnHandlersMessage::MSGID_ECPROPERTYCATEGORY_Extended;
        case RawData:
            displayLabelMsgId = DgnHandlersMessage::MSGID_ECPROPERTYCATEGORY_RawData;
        case Geometry:
            displayLabelMsgId = DgnHandlersMessage::MSGID_ECPROPERTYCATEGORY_Geometry;
        case Groups:
            displayLabelMsgId = DgnHandlersMessage::MSGID_ECPROPERTYCATEGORY_Groups;
        case Material:
            displayLabelMsgId = DgnHandlersMessage::MSGID_ECPROPERTYCATEGORY_Material;
        case Relationships:
            displayLabelMsgId = DgnHandlersMessage::MSGID_ECPROPERTYCATEGORY_Relationships;
        case Miscellaneous:
        default:
            displayLabelMsgId = DgnHandlersMessage::MSGID_ECPROPERTYCATEGORY_Miscellaneous;
        }

    return DgnHandlersMessage::GetStringW (displayLabelMsgId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
int ECStandardCategoryHelper::GetPriority (StandardCategory standard)
    {
    switch (standard)
        {
        case General:
            return CategorySortPriorityVeryHigh;
        case Extended:
            return CategorySortPriorityMedium;
        case RawData:
            return CategorySortPriorityVeryLow;
        case Geometry:
            return CategorySortPriorityHigh;
        case Groups:
            return CategorySortPriorityMedium + 2000;
        case Material:
            return CategorySortPriorityMedium + 1000;
        case Relationships:
            return CategorySortPriorityMedium - 1000;
        default:
            return CategorySortPriorityVeryLow + 1000;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECStandardCategoryHelper::GetDefaultExpand (StandardCategory standard)
    {
    switch (standard)
        {
        // only General is opened by default.
        case General:
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECStandardCategoryHelper::GetName (StandardCategory standard)
    {
    switch (standard)
        {
        case General:
            return L"General";
        case Extended:
            return L"Extended";
        case RawData:
            return L"RawData";
        case Geometry:
            return L"Geometry";
        case Groups:
            return L"Groups";
        case Material:
            return L"Material";
        case Relationships:
            return L"Relationships";
        }
    return L"Miscellaneous";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECStandardCategoryHelper::SetValue (IECInstanceR instance, WCharCP name, ECValueCR ecValue)
    {
    ECObjectsStatus status = instance.SetValue (name, ecValue);
    if (ECOBJECTS_STATUS_Success != status)
        { BeAssert (false); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECStandardCategoryHelper::SetStringValue (IECInstanceR instance, WCharCP name, WCharCP val)
    {
    ECValue ecValue;
    ecValue.SetString (val);
    SetValue (instance, name, ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECStandardCategoryHelper::SetBooleanValue (IECInstanceR instance, WCharCP name, bool val)
    {
    ECValue ecValue;
    ecValue.SetBoolean(val);
    SetValue (instance, name, ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECStandardCategoryHelper::SetIntegerValue (IECInstanceR instance, WCharCP name, int val)
    {
    ECValue ecValue;
    ecValue.SetInteger (val);
    SetValue (instance, name, ecValue);
    }

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECPropertyFormatterPtr DgnECPropertyFormatter::Create (DgnModelP dgnModel)
    {
    return new DgnECPropertyFormatter (dgnModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECPropertyFormatter::DgnECPropertyFormatter (DgnModelP dgnModel) : m_dgnModel (dgnModel)
    {
    if (m_dgnModel != NULL)
        m_standardCategoryHelper.Initialize (dgnModel->GetDgnProject());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPropertyFormatter::_FormattedStringFromECValue 
(
Utf8StringR strVal, 
ECValueCR ecValue, 
ECPropertyCR ecProperty, 
bool isArrayMember
) const
    {
    BeAssert (!isArrayMember || (isArrayMember && ecProperty.GetIsArray()));
    IDgnECTypeAdapterR ecTypeAdapter = isArrayMember ? IDgnECTypeAdapter::GetForArrayMember (*ecProperty.GetAsArrayProperty()) : IDgnECTypeAdapter::GetForProperty (ecProperty);

    DgnProjectP dgnFile = (m_dgnModel == NULL) ? NULL : &m_dgnModel->GetDgnProject();
    IDgnECTypeAdapterContextPtr context = StandaloneTypeAdapterContext::Create (ecProperty, IDgnECTypeAdapterContext::COMPONENT_INDEX_None, dgnFile, m_dgnModel);

    WString wstrVal;
    bool status = ecTypeAdapter.ConvertToString (wstrVal, ecValue, *context);
    POSTCONDITION (status && "Could not format property to a string", status);

    strVal = Utf8String (wstrVal.c_str()); 
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr DgnECPropertyFormatter::_GetPropertyCategory (ECN::ECPropertyCR ecProperty)
    {
    IECInstancePtr categoryAttr = ecProperty.GetCustomAttribute (L"Category");

    // If no category is specified, use the standard "Miscellaneous" category
    if (categoryAttr.IsNull())
        return m_standardCategoryHelper.GetCategory (ECStandardCategoryHelper::Miscellaneous);

    // If a category "Name" is specified, use the user defined category
    // Note: We match the 8.11.9 logic here - even if the property has a "Standard" defined, we need
    // to use the "Name" if that's defined - see TFS#67022
    ECValue ecValue;
    if (ECOBJECTS_STATUS_Success == categoryAttr->GetValue (ecValue, L"Name") && !ecValue.IsNull())
        return categoryAttr;

    // If no standard category is defined, use the user defined category. 
    if (ECOBJECTS_STATUS_Success != categoryAttr->GetValue (ecValue, L"Standard") || ecValue.IsNull())
        return categoryAttr;

    // Use one of the standard categories
    ECStandardCategoryHelper::StandardCategory standard = (ECStandardCategoryHelper::StandardCategory) ecValue.GetInteger();
    return m_standardCategoryHelper.GetCategory (standard);
    }

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      05/2014
+===============+===============+===============+===============+===============+======*/
struct DgnECPersistenceCache : public BeSQLite::DbAppData
{
private:
    DgnProjectR m_host;
    mutable unique_ptr<JsonReader> m_jsonReader;

    DgnECPersistenceCache (DgnProjectR host) : m_host (host), m_jsonReader (nullptr) {}

    virtual void _OnCleanup (BeSQLiteDbR host) override {delete this;}

    static BeSQLite::DbAppData::Key const& GetKey() {static BeSQLite::DbAppData::Key s_key; return s_key;}

    StatusInt Initialize();

public:
    virtual ~DgnECPersistenceCache() {}
    
    static DgnECPersistenceCache* GetCache (DgnProjectR host);

    JsonReader* GetJsonReader() const {return m_jsonReader.get();}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECPersistenceCache* DgnECPersistenceCache::GetCache (DgnProjectR host)
    {
    DgnECPersistenceCache* cache = 
        reinterpret_cast <DgnECPersistenceCache *> (host.AppData().Find (DgnECPersistenceCache::GetKey()));

    if (nullptr == cache)
        {
        cache = new DgnECPersistenceCache (host);
        if (SUCCESS != cache->Initialize())
            return nullptr;
        host.AppData().Add (DgnECPersistenceCache::GetKey(), cache);
        }
    return cache;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnECPersistenceCache::Initialize()
    {
    ECN::ECClassP elementClass = NULL;
    m_host.GetEC().GetSchemaManager().GetECClass (elementClass, "dgn", "Element");
    POSTCONDITION (elementClass != NULL && "Unable to find dgn:Element class", ERROR);

    m_jsonReader = unique_ptr<JsonReader> (new JsonReader (m_host, elementClass->GetId()));
    return SUCCESS;
    }
    
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
#if defined (NEEDS_WORK_DGNITEM)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECPersistence::DgnECPersistence (DgnProjectR project) : m_project (project), 
    m_ecInstanceFinder (NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECPersistence::~DgnECPersistence() 
    {
    Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECPersistence::Finalize()
    {
    if (NULL != m_ecInstanceFinder) 
        {
        delete m_ecInstanceFinder; 
        m_ecInstanceFinder = NULL;
        }
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceFinder* DgnECPersistence::GetECInstanceFinder()
    {
    if (NULL == m_ecInstanceFinder)
        m_ecInstanceFinder = new ECInstanceFinder (m_project);
    return m_ecInstanceFinder;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnECPersistence::UpgradeTables (DgnProjectR project)
    {
    CreateTables (project);
    return ImportDgnSchema (project, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECPersistence::CreateTables (DgnProjectR project)
    {
    if (!project.TableExists(DGNELEMENT_TABLE_SecondaryECInstances))
        {
        project.CreateTable (DGNELEMENT_TABLE_SecondaryECInstances, "ElementId INTEGER REFERENCES " DGNELEMENT_TABLE_Data " ON DELETE CASCADE,ECClassId INTEGER,ECId INTEGER");
        project.ExecuteSql ("CREATE INDEX dgnEC_ElementIdx ON " DGNELEMENT_TABLE_SecondaryECInstances "(ElementId)"); // For typical ECQuery use-cases of retrieving Elements from ECInstances found
        project.ExecuteSql ("CREATE INDEX dgnEC_ECIdx ON " DGNELEMENT_TABLE_SecondaryECInstances "(ECClassId,ECId)"); // For typical ECQuery use-cases of retrieving Elements from ECInstances found
        project.ExecuteSql ("CREATE UNIQUE INDEX dgnEC_DuplicatesIdx ON " DGNELEMENT_TABLE_SecondaryECInstances "(ElementId, ECClassId,ECId)");
        }

    // Create views for primary and secondary instance relationships - this is to just rename the ECClassId, ECId columns to avoid ECSQL reserved words. It's a stop gap solution 
    // until we can break backwards compatibility in Graphite06.
    project.ExecuteSql ("CREATE VIEW IF NOT EXISTS dgnEC_ElementData AS SELECT ElementId, ModelId, Level, ECClassId PrimaryECClassId, ECId PrimaryECId FROM " DGNELEMENT_TABLE_Data);
    project.ExecuteSql ("CREATE VIEW IF NOT EXISTS dgnEC_ElementHasSecondaryInstances AS SELECT RowId ECId, ElementId, ECClassId SecondaryECClassId, ECId SecondaryECId FROM " DGNELEMENT_TABLE_SecondaryECInstances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnECPersistence::SetPrimaryInstanceOnElement (EditElementHandleR editElementHandle, ECInstanceKeyCR instanceKey, DgnProjectR project)
    {
    PRECONDITION (instanceKey.IsValid() && "Invalid instance key specified", ERROR);
    PRECONDITION (!ElementHasPrimaryInstance (editElementHandle) && "Element already has a primary instance associated with it. Clear it first.", ERROR);

    ElementRefP elementRef = editElementHandle.GetElementRef(); // Secondary instances can only be specified on an element that's already been persisted in the DgnDb. 
    PRECONDITION (!(elementRef != NULL && ElementHasSecondaryInstance (elementRef->GetElementId(), project, &instanceKey)) && "Element already has the specified instance associated with it", ERROR);
    
    // TODO: Need a check for existence of the instance with the key. ECDb needs an efficient way to make that check.

    MSElementDescrP elementDescr = editElementHandle.GetElementDescrP();
    elementDescr->SetECClassId (instanceKey.GetECClassId());
    elementDescr->SetECInstanceId (instanceKey.GetECInstanceId());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnECPersistence::AddSecondaryInstanceOnElement 
(
const ElementId& elementId, 
ECInstanceKeyCR instanceKey,
DgnProjectR project
)
    {
    PRECONDITION (instanceKey.IsValid() && "Invalid instance key specified", ERROR);
    PRECONDITION (!ElementHasPrimaryInstance (elementId, project, &instanceKey) && "Element already has the specified instance associated with it", ERROR);
    PRECONDITION (!ElementHasSecondaryInstance (elementId, project, &instanceKey) && "Element already has the specified instance associated with it", ERROR);

    CachedStatementPtr stmt;
    DbResult result= project.GetCachedStatement (stmt, "INSERT INTO " DGNELEMENT_TABLE_SecondaryECInstances " (ElementId, ECClassId, ECId) VALUES(?,?,?)");
    BeAssert (result == BE_SQLITE_OK);
    stmt->BindId (1, elementId);
    stmt->BindInt64 (2, instanceKey.GetECClassId());
    stmt->BindId (3, instanceKey.GetECInstanceId());
    result = stmt->Step();

    POSTCONDITION (result == BE_SQLITE_DONE && "Sqlite error.", ERROR);
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPersistence::ElementHasPrimaryInstance 
(
ElementHandleCR elementHandle,
ECInstanceKeyCP instanceKey /* = NULL */
)
    {
    ECInstanceKey existingKey;
    MSElementDescrCP elementDescr = elementHandle.PeekElementDescrCP();
    if (elementDescr != NULL)
        existingKey = ECInstanceKey (elementDescr->GetECClassId(), elementDescr->GetECInstanceId());
    else
        {
        ElementRefP elementRef = elementHandle.GetElementRef();
        BeAssert (elementRef != NULL);
        existingKey = ECInstanceKey (elementRef->GetECClassId(), elementRef->GetECInstanceId());
        }

    return (instanceKey == NULL) ? existingKey.IsValid() : (existingKey == *instanceKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPersistence::ElementHasPrimaryInstance 
(
const ElementId& elementId,
DgnProjectCR project,
ECInstanceKeyCP instanceKey /* = NULL */
)
    {
    ECInstanceKey existingKey;
    if (!GetPrimaryInstanceOnElement (existingKey, elementId, project))
        return false;
    return (instanceKey == NULL) ? existingKey.IsValid() : (existingKey == *instanceKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPersistence::ElementHasSecondaryInstance 
(
const ElementId& elementId, 
DgnProjectCR project,
ECInstanceKeyCP instanceKey /* = NULL */
)
    {
    CachedStatementPtr stmt;
    DbResult result;
    if (instanceKey == NULL)
        {
        result = project.GetCachedStatement (stmt, "SELECT NULL FROM " DGNELEMENT_TABLE_SecondaryECInstances " WHERE ElementId=?");
        stmt->BindId (1, elementId);
        BeAssert (result == BE_SQLITE_OK);
        }
    else 
        {
        result = project.GetCachedStatement (stmt, "SELECT NULL FROM " DGNELEMENT_TABLE_SecondaryECInstances " WHERE ElementId=? AND ECClassId=? AND ECId=?");
        BeAssert (result == BE_SQLITE_OK);
        stmt->BindId (1, elementId);
        stmt->BindInt64 (2, instanceKey->GetECClassId());
        stmt->BindId (3, instanceKey->GetECInstanceId());
        }

    return (BE_SQLITE_ROW == stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPersistence::GetPrimaryInstanceOnElement 
(
ECInstanceKeyR instanceKey, 
const ElementId& elementId, 
DgnProjectCR project
)
    {
    CachedStatementPtr stmt;
    DbResult result = project.GetCachedStatement (stmt, "SELECT ECClassId, ECId FROM " DGNELEMENT_TABLE_Data " WHERE ElementId=?");
    BeAssert (result == BE_SQLITE_OK);
    stmt->BindId (1, elementId);
    result = stmt->Step();
    if (BE_SQLITE_DONE == result)
        return false;
    BeAssert (result == BE_SQLITE_ROW);

    ECClassId ecClassId = stmt->GetValueInt64 (0);
    ECInstanceId ecInstanceId = stmt->GetValueId<ECInstanceId> (1);
    instanceKey = ECInstanceKey (ecClassId, ecInstanceId);
    return instanceKey.IsValid();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPersistence::GetPrimaryInstanceOnElement 
    (
    ECInstanceKeyR instanceKey, 
    ElementHandleCR elementHandle
    )
    {
    MSElementDescrCP elementDescr = elementHandle.PeekElementDescrCP();
    if (elementDescr != NULL)
        instanceKey = ECInstanceKey (elementDescr->GetECClassId(), elementDescr->GetECInstanceId());
    else
        {
        ElementRefP elementRef = elementHandle.GetElementRef();
        BeAssert (elementRef != NULL);
        instanceKey = ECInstanceKey (elementRef->GetECClassId(), elementRef->GetECInstanceId());
        }

    return instanceKey.IsValid();
    }
    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassId DgnECPersistence::GetElementClassId (DgnProjectCR project)
    {
    // TODO: Need schema manager to expose a similar method. 
    Utf8CP sql = "SELECT ECClassId " \
        "FROM ec_Class " \
        "JOIN ec_Schema ON ec_Class.ECSchemaId = ec_Schema.ECSchemaId " \
        "WHERE ec_Class.Name = 'Element' AND ec_Schema.Name = 'dgn'";

    CachedStatementPtr stmt;
    project.GetCachedStatement (stmt, sql);
    stmt->Step();
    ECClassId classId = stmt->GetValueInt64 (0);
    BeAssert (classId > 0);
    return classId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPersistence::GetCoreInstanceOnElement 
    (
    ECInstanceKeyR instanceKey, 
    const ElementId& elementId, 
    DgnProjectCR project
    )
    {
    PRECONDITION (IsElementIdUsed (elementId, project), false);

    // TODO: Cache the elementClassId
    ECClassId elementClassId = GetElementClassId (project);
    instanceKey = ECInstanceKey (elementClassId, ECInstanceId (elementId.GetValue()));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnECPersistence::GetInstancesOnElement 
(
bvector<ECInstanceKey>& instanceKeys, 
const ElementId& elementId, 
DgnProjectCR project,
DgnECPersistence::AssociationType association /* = DgnECPersistence::AssociationType_All */
)
    {
    instanceKeys.clear();
    if (association & AssociationType_Core)
        {
        ECInstanceKey instanceKey;
        if (GetCoreInstanceOnElement (instanceKey, elementId, project))
            instanceKeys.push_back (instanceKey);
        }

    if (association & AssociationType_Primary)
        {
        ECInstanceKey instanceKey;
        if (GetPrimaryInstanceOnElement (instanceKey, elementId, project))
            instanceKeys.push_back (instanceKey);
        }

    if (association & AssociationType_Secondary)
        {
        CachedStatementPtr stmt;
        DbResult result = project.GetCachedStatement (stmt, "SELECT ECClassId, ECId FROM " DGNELEMENT_TABLE_SecondaryECInstances " WHERE ElementId=?"); // TODO: Do we really need to cache this?
        BeAssert (result == BE_SQLITE_OK);
        stmt->BindId (1, elementId);
        while (BE_SQLITE_ROW == (result = stmt->Step()))
            {
            ECClassId ecClassId = stmt->GetValueInt64 (0);
            ECInstanceId ecInstanceId = stmt->GetValueId<ECInstanceId> (1);
            instanceKeys.push_back (ECInstanceKey (ecClassId, ecInstanceId));
            }
        BeAssert (result == BE_SQLITE_DONE);
        }

    return instanceKeys.size();
    }
   
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPersistence::GetElementWithPrimaryInstance 
(
ElementId& elementId, 
const ECInstanceKey& instanceKey, 
DgnProjectCR project
)
    {
    CachedStatementPtr stmt;
    DbResult result = project.GetCachedStatement (stmt, "SELECT ElementId FROM " DGNELEMENT_TABLE_Data " WHERE ECClassId=? AND ECId=?"); // TODO: Do we really need to cache this?
    BeAssert (result == BE_SQLITE_OK);
    stmt->BindInt64 (1, instanceKey.GetECClassId());
    stmt->BindId (2, instanceKey.GetECInstanceId());
    result = stmt->Step();
    if (BE_SQLITE_DONE == result)
        return false;
    BeAssert (result == BE_SQLITE_ROW);

    Int64 id = stmt->GetValueInt64 (0);
    elementId = ElementId (id);
    return true;
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPersistence::IsGraphicalElement (const ElementId& elementId, DgnProjectCR project)
    {
    CachedStatementPtr stmt;
    DbResult result = project.GetCachedStatement (stmt, "SELECT Owner FROM " DGNELEMENT_TABLE_Data " WHERE ElementId=?");
    BeAssert (result == BE_SQLITE_OK);
    stmt->BindInt64 (1, elementId.GetValue());
    result = stmt->Step();
    BeAssert (result == BE_SQLITE_ROW);

    ElementOwnerType owner = (ElementOwnerType) stmt->GetValueInt (0);
    return (owner == OWNER_Physical || owner == OWNER_Drawing);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPersistence::IsElementIdUsed (ElementId elementId, DgnProjectCR project)
    {
    if (project.Models().FindElementById (elementId)) // First look at the cache to avoid looking up the file unless necessary
        return true;

    return project.Models().IsElementIdUsed (elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnECPersistence::GetElementsWithInstance 
(
bset<ElementId>& elementIds, 
const ECInstanceKey& instanceKey, 
DgnProjectCR project,
DgnECPersistence::AssociationType association /* = DgnECPersistence::AssociationType_All */,
bool inludeOnlyGraphicalElements /* = false */
)
    {
    elementIds.clear();
    PRECONDITION (instanceKey.IsValid(), 0);

    if (association & AssociationType_Core)
        {
        ECClassId elementClassId = GetElementClassId (project);
        ElementId elementId (instanceKey.GetECInstanceId().GetValue ());
        if (instanceKey.GetECClassId() == elementClassId && IsElementIdUsed (elementId, project))
            elementIds.insert (elementId);
        }

    if (association & AssociationType_Primary)
        {
        ElementId elementId;
        if (GetElementWithPrimaryInstance (elementId, instanceKey, project))
            {
            if (!inludeOnlyGraphicalElements || IsGraphicalElement (elementId, project))
                elementIds.insert (elementId);
            }
        }

    if (association & AssociationType_Secondary)
        {
        CachedStatementPtr stmt;
        DbResult result = project.GetCachedStatement (stmt, "SELECT ElementId FROM " DGNELEMENT_TABLE_SecondaryECInstances " WHERE ECClassId=? AND ECId=?"); // TODO: Do we really need to cache this?
        BeAssert (result == BE_SQLITE_OK);
        stmt->BindInt64 (1, instanceKey.GetECClassId());
        stmt->BindId (2, instanceKey.GetECInstanceId());
        while (BE_SQLITE_ROW == (result = stmt->Step()))
            {
            ElementId elementId (stmt->GetValueInt64 (0));
            if (!inludeOnlyGraphicalElements || IsGraphicalElement (elementId, project))
                elementIds.insert (elementId);
            }
        BeAssert (result == BE_SQLITE_DONE);
        }

    return elementIds.size();
    }
    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnECPersistence::FindElements 
(
bset<ElementId>& elementIds, 
const ECInstanceKeyMultiMap& seedInstanceKeyMap, 
ECInstanceFinder::FindOptions instanceFindOptions /* =  ECInstanceFinder::FindOptions() */, 
DgnECPersistence::AssociationType association /* = DgnECPersistence::AssociationType_All */,
bool inludeOnlyGraphicalElements /* = false */
)
    {
    elementIds.clear();

    // Recursively gather related instances
    ECInstanceKeyMultiMap instanceKeyMap;
    BentleyStatus status = GetECInstanceFinder()->FindInstances (instanceKeyMap, seedInstanceKeyMap, instanceFindOptions);
    if (status != SUCCESS)
        return status;

    // Find elements corresponding to the gathered instances
    for (ECInstanceKeyMultiMapConstIterator iter = instanceKeyMap.begin(); iter != instanceKeyMap.end(); iter++)
        {
        ECClassId ecClassId = iter->first;
        ECInstanceId ecInstanceId = iter->second;

        bset<ElementId> tmpElementIds;
        GetElementsWithInstance (tmpElementIds, ECInstanceKey (ecClassId, ecInstanceId), m_project, association, inludeOnlyGraphicalElements);
        elementIds.insert (tmpElementIds.begin(), tmpElementIds.end());
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnECPersistence::RemovePrimaryInstanceOnElement 
(
EditElementHandleR editElementHandle, 
const ECInstanceKey& instanceKey
)
    {
    PRECONDITION (ElementHasPrimaryInstance (editElementHandle, &instanceKey) && "Element does not have the specified primary instance", ERROR);
    return ClearPrimaryInstanceOnElement (editElementHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnECPersistence::RemoveSecondaryInstanceOnElement
(
const ElementId& elementId, 
ECInstanceKeyCR instanceKey,
DgnProjectR project
)
    {
    PRECONDITION (ElementHasSecondaryInstance (elementId, project, &instanceKey) && "Element does not have the specified secondary instance", ERROR);

    CachedStatementPtr stmt;
    DbResult result = project.GetCachedStatement (stmt, "DELETE FROM " DGNELEMENT_TABLE_SecondaryECInstances " WHERE ElementId = ? AND ECClassId = ? AND ECId = ?");
    BeAssert (result == BE_SQLITE_OK);
    stmt->BindId (1, elementId);
    stmt->BindInt64 (2, instanceKey.GetECClassId());
    stmt->BindId (3, instanceKey.GetECInstanceId());
    result = stmt->Step();

    POSTCONDITION (result == BE_SQLITE_DONE && "Sqlite error.", ERROR);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnECPersistence::ClearPrimaryInstanceOnElement (EditElementHandleR editElementHandle)
    {
    MSElementDescrP elementDescr = editElementHandle.GetElementDescrP();
    elementDescr->SetECClassId (0);
    elementDescr->SetECInstanceId (ECInstanceId());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnECPersistence::ClearSecondaryInstancesOnElement (const ElementId& elementId, DgnProjectR project)
    {
    CachedStatementPtr stmt;
    DbResult result = project.GetCachedStatement (stmt, "DELETE FROM " DGNELEMENT_TABLE_SecondaryECInstances " WHERE ElementId = ?");
    BeAssert (result == BE_SQLITE_OK);
    stmt->BindId (1, elementId);
    result = stmt->Step();

    POSTCONDITION (result == BE_SQLITE_DONE && "Sqlite error.", ERROR);
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
* @remarks The element with the primary instance can be a containing assembly of the supplied element. 
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPersistence::TryGetAssemblyElementWithPrimaryInstance 
(
ElementId& outElementId, 
const ElementId& inElementId, 
DgnProjectR project
)
    {
    Statement statement;       
    statement.Prepare (project, "SELECT ECClassId, ECId, AssemblyId FROM " DGNELEMENT_TABLE_Data " WHERE ElementId=?");

    ElementId currentElementId = inElementId;
    while (currentElementId.IsValid())
        {
        statement.BindId (1, currentElementId);
        DbResult result = statement.Step();
        if (!EXPECTED_CONDITION (result == BE_SQLITE_ROW && "Element not found in the " DGNELEMENT_TABLE_Data " table"))
            return false;

        ECInstanceKey primaryInstanceKey (statement.GetValueInt64 (0), statement.GetValueId<ECInstanceId> (1));
        if (primaryInstanceKey.IsValid())
            {
            outElementId = currentElementId;
            return true;
            }

        currentElementId = statement.GetValueId<ElementId>(2); // Walk up to the assembly (if there's one)
        statement.Reset();
        }

    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnECPersistence::GetModelIdFromElementId (const ElementId& elementId, DgnProjectR project)
    {
    Statement statement;       
    statement.Prepare (project, "SELECT ModelId FROM " DGNELEMENT_TABLE_Data " WHERE ElementId=?");
    statement.BindId (1, elementId);

    POSTCONDITION(BE_SQLITE_ROW == statement.Step(), DgnModelId());

    return statement.GetValueId<DgnModelId> (0);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnECPersistence::GetElementInfo (JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, const ElementId& elementId, DgnProjectR project)
    {
    JsonReader* jsonReader = (DgnECPersistenceCache::GetCache (project) != nullptr) ?
        DgnECPersistenceCache::GetCache (project)->GetJsonReader() : nullptr;
    PRECONDITION (jsonReader != nullptr && "Unable to read info on elements", ERROR);

    DgnModelId modelId = GetModelIdFromElementId (elementId, project);
    DgnModelP model = modelId.IsValid() ? project.Models().GetModelById (modelId) : project.Models().GetDictionaryModel();
    BeAssert (model != NULL);

    DgnECPropertyFormatterPtr propertyFormatter = DgnECPropertyFormatter::Create (model);
    JsonECSqlSelectAdapter::FormatOptions formatOptions (ECValueFormat::FormattedStrings, propertyFormatter.get());

    return jsonReader->Read (jsonInstances, jsonDisplayInfo, (ECInstanceId) elementId.GetValue(), formatOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnECPersistence::ImportDgnSchema (DgnProjectR project, bool updateExisting)
    {
    ECN::ECSchemaReadContextPtr ecSchemaContext = ECN::ECSchemaReadContext::CreateContext();
    ecSchemaContext->AddSchemaLocater (project.GetEC().GetSchemaLocater());

    BeFileName ecSchemaPath = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    ecSchemaPath.AppendToPath (L"ECSchemas");

    BeFileName dgnSchemaPath = ecSchemaPath;
    dgnSchemaPath.AppendToPath (L"Dgn");
    ecSchemaContext->AddSchemaPath (dgnSchemaPath);

    BeFileName standardSchemaPath = ecSchemaPath;
    standardSchemaPath.AppendToPath (L"Standard");
    ecSchemaContext->AddSchemaPath (standardSchemaPath);

    ECN::SchemaKey dgnschemaKey (DGNEC_SCHEMA_NAME, DGNEC_SCHEMA_MAJOR_VERSION, DGNEC_SCHEMA_MINOR_VERSION);
    ECN::ECSchemaPtr dgnschema = ECN::ECSchema::LocateSchema (dgnschemaKey, *ecSchemaContext);
    POSTCONDITION (dgnschema != NULL && "Could not find the dgn schema", ERROR);
    BentleyStatus status = project.GetEC().GetSchemaManager().ImportECSchemas (ecSchemaContext->GetCache(), 
        ECDbSchemaManager::ImportOptions (false/*supplementation*/, updateExisting));
    POSTCONDITION (status == SUCCESS && "Could not import dgn schema", ERROR);
    
    return SUCCESS;
    }
#endif
