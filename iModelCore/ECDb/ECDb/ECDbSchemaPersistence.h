/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistence.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECClassEntry
    {
public:
    ECClassId    m_ecClassId;
    ECSchemaId   m_ecSchemaId;
    Utf8String   m_className;
    ECN::ECClassP m_resolvedECClass;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECSchemaEntry
    {
public:
    ECSchemaId       m_ecSchemaId;
    Utf8String       m_schemaName;
    uint32_t         m_versionMajor;
    uint32_t         m_versionMinor;
    uint32_t         m_nClassesInSchema; //This is read from Db
    uint32_t         m_nClassesLoaded;   //Every time a class is loaded from db for this schema it is incremented
    ECN::ECSchemaPtr  m_resolvedECSchema;    //Contain ECSchema which might be not complete
    bool             IsFullyLoaded() {return m_nClassesInSchema == m_nClassesLoaded;} 
    };



/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaPersistence
    {   
private:
    static int ToInt (ECN::ECRelatedInstanceDirection direction);
    static ECRelatedInstanceDirection ToECRelatedInstanceDirection (int relatedInstanceDirection);


public:
    typedef bvector<ECClassId>        ECClassIdList;
    typedef ECClassIdList*            ECClassIdListP;
    typedef ECClassIdList&            ECClassIdListR;
    typedef bvector<DbECSchemaEntry>  ECSchemaKeyList;
    typedef ECSchemaKeyList&          ECSchemaKeyListR;
    typedef ECSchemaKeyList*          ECSchemaKeyListP;

    //Insert new item
    static  BeSQLite::DbResult InsertECSchemaInfo                          (BeSQLite::Db& db, DbECSchemaInfoCR info);
    static  BeSQLite::DbResult InsertECClassInfo                           (BeSQLite::Db& db, DbECClassInfoCR info);
    static  BeSQLite::DbResult InsertBaseClass                             (BeSQLite::Db& db, DbBaseClassInfoCR info);
    static  BeSQLite::DbResult InsertECPropertyInfo                        (BeSQLite::Db& db, DbECPropertyInfoCR info);
    static  BeSQLite::DbResult InsertECRelationConstraintInfo              (BeSQLite::Db& db, DbECRelationshipConstraintInfoCR info);
    static  BeSQLite::DbResult InsertECRelationConstraintClassInfo         (BeSQLite::Db& db, DbECRelationshipConstraintClassInfoCR info);
    static  BeSQLite::DbResult InsertECRelationConstraintClassPropertyInfo (BeSQLite::Db& db, DbECRelationshipConstraintClassPropertyInfoCR info);
    static  BeSQLite::DbResult InsertCustomAttributeInfo                   (BeSQLite::Db& db, DbCustomAttributeInfoCR info);
    static  BeSQLite::DbResult InsertECSchemaReferenceInfo                 (BeSQLite::Db& db, DbECSchemaReferenceInfoCR info);

    //update existing items
    static  BeSQLite::DbResult UpdateECSchemaInfo                          (BeSQLite::Db& db, DbECSchemaInfoCR info);
    static  BeSQLite::DbResult UpdateECClassInfo                           (BeSQLite::Db& db, DbECClassInfoCR info);
    static  BeSQLite::DbResult UpdateECPropertyInfo                        (BeSQLite::Db& db, DbECPropertyInfoCR info);
    static  BeSQLite::DbResult UpdateECRelationshipConstraintClass         (BeSQLite::Db& db, DbECRelationshipConstraintClassInfoCR info);
    static  BeSQLite::DbResult UpdateCustomAttributeInfo                   (BeSQLite::Db& db, DbCustomAttributeInfoCR info);
    static  BeSQLite::DbResult UpdateECRelationConstraintInfo              (BeSQLite::Db& db, DbECRelationshipConstraintInfoCR info);

    //Create system tables

    //Find ECSchemaInfo
    static  BeSQLite::DbResult FindECSchemaInfo                         (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECSchemaInfoCR spec);
    static  BeSQLite::DbResult Step                                     (DbECSchemaInfoR info, BeSQLite::Statement& stmt);
    //Find ECClassInfo
    static  BeSQLite::DbResult FindECClassInfo                          (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECClassInfoCR spec);
    static  BeSQLite::DbResult Step                                     (DbECClassInfoR info, BeSQLite::Statement& stmt);
    //Find BaseClass
    static  BeSQLite::DbResult FindBaseClassInfo                        (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbBaseClassInfoCR spec);
    static  BeSQLite::DbResult Step                                     (DbBaseClassInfoR info, BeSQLite::Statement& stmt);
    //Find ECPropertyInfo
    static  BeSQLite::DbResult FindECPropertyInfo                       (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECPropertyInfoCR spec);
    static  BeSQLite::DbResult Step                                     (DbECPropertyInfoR info, BeSQLite::Statement& stmt);
    //Find ECRelationConstraintInfo 
    static  BeSQLite::DbResult FindECRelationshipConstraintInfo         (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECRelationshipConstraintInfoCR spec);
    static  BeSQLite::DbResult Step                                     (DbECRelationshipConstraintInfoR info, BeSQLite::Statement& stmt);
    //Find ECRelationConstraintInfo 
    static  BeSQLite::DbResult FindECRelationConstraintClassInfo        (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECRelationshipConstraintClassInfoCR spec);
    static  BeSQLite::DbResult Step                                     (DbECRelationshipConstraintClassInfoR info, BeSQLite::Statement& stmt);
    //Find CustomAttributeInfo
    static  BeSQLite::DbResult FindCustomAttributeInfo                  (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbCustomAttributeInfoCR spec);
    static  BeSQLite::DbResult Step                                     (DbCustomAttributeInfoR info, BeSQLite::Statement& stmt);
    //Find CustomAttributeInfo
    static  BeSQLite::DbResult FindECSchemaReferenceInfo                (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECSchemaReferenceInfoCR spec);
    static  BeSQLite::DbResult Step                                     (DbECSchemaReferenceInfoR info, BeSQLite::Statement& stmt);

    //Helper
    static                bool ContainsECSchemaReference                (BeSQLite::Db& db, ECSchemaId ecPrimarySchemaId, ECSchemaId ecReferencedSchemaId);
    static                bool ContainsECClass                          (BeSQLite::Db& db, ECClassCR ecClass);
    static                bool ContainsECSchemaWithId                   (BeSQLite::Db& db, ECSchemaId ecSchemaId);
    static          ECSchemaId GetECSchemaId                            (BeSQLite::Db& db, Utf8CP schemaName);
    static          ECSchemaId GetECSchemaId                            (BeSQLite::Db& db, ECSchemaCR ecSchema);
    static           ECClassId GetECClassIdBySchemaNameSpacePrefix      (BeSQLite::Db& db, Utf8CP schemaName, Utf8CP className);
    static           ECClassId GetECClassIdBySchemaName                 (BeSQLite::Db& db, Utf8CP schemaName, Utf8CP className);
    static        ECPropertyId GetECPropertyId                          (BeSQLite::Db& db, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName);

    static  BeSQLite::DbResult InitializeSystemTables                   (BeSQLite::Db& db);
    static                bool RequiredSystemTablesExist                (BeSQLite::Db& db); 
    static  BeSQLite::DbResult ResolveECClassId                         (Utf8StringR schemaName, uint32_t& versionMajor,uint32_t& versionMinor, Utf8StringR className, ECClassId ecClassId, BeSQLite::Db& db);
    static  BeSQLite::DbResult ResolveECClassId                         (DbECClassEntryR key, ECClassId ecClassId, BeSQLite::Db& db);
    static  BeSQLite::DbResult GetDerivedECClasses                      (ECClassIdListR classIds, ECClassId baseClassId, BeSQLite::Db& db);
    static  BeSQLite::DbResult GetBaseECClasses                         (ECClassIdListR baseClassIds, ECClassId ecClassId, BeSQLite::Db& db);
    static  BeSQLite::DbResult ResolveECSchemaId                        (Utf8StringR schemaName, uint32_t& versionMajor,uint32_t& versionMinor, ECSchemaId ecSchemaId, BeSQLite::Db& db);
    static  BeSQLite::DbResult ResolveECSchemaId                        (DbECSchemaEntryR key, ECSchemaId ecSchemaId, BeSQLite::Db& db);
    static  BeSQLite::DbResult DeleteECClass                            (ECClassId ecClassId, BeSQLite::Db& db);
    static  BeSQLite::DbResult DeleteECSchema                           (ECSchemaId ecSchemaId, BeSQLite::Db& db);
    static  BeSQLite::DbResult DeleteCustomAttribute                    (ECContainerId containerId, ECContainerType containerType, ECClassId customAttributeClassId, BeSQLite::Db& db);

    static  BeSQLite::DbResult GetECSchemaKeys                          (ECSchemaKeys& keys, BeSQLite::Db& db);
    static  BeSQLite::DbResult GetECClassKeys                           (ECClassKeys& keys, ECSchemaId schemaId, BeSQLite::Db& db);
    static                bool IsECSchemaMapped                         (bool* schemaNotFound, ECN::ECSchemaCR ecSchema, BeSQLite::Db& db);
    static  BeSQLite::DbResult GetClassesMappedToTable (std::vector<ECClassId>& classIds, ECDbSqlTable const& table, bool skipRelationships, BeSQLite::Db& db);
    static                bool IsCustomAttributeDefined                 (BeSQLite::Db& db, ECClassId caClassId, ECContainerId caSourceContainerId, ECContainerType caContainerType);
    static  BeSQLite::DbResult DeleteECProperty                         (ECPropertyId propertyId, BeSQLite::Db& db);
    static  ECDbPropertyPathId GetECPropertyPathId (ECPropertyId rootECPropertyId, Utf8CP accessString, BeSQLite::Db& db);

    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbBuffer
{
private:
    Byte*   m_data;
    size_t  m_length;
    bool    m_ownsBuffer;
protected:
    void     Reset();
    bool     Allocate(size_t length);

public:
    DbBuffer();
    DbBuffer(size_t length);
    virtual ~DbBuffer();
    Byte*    GetData()   const  {return m_data;} 
    size_t   GetLength() const {return m_length;}
    bool     SetData (Byte* data, size_t length, bool createCopy =false);
    void     Dettach();
    void     Resize(size_t length);
};

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbInfoBase
    {
public:
    union 
        {
        uint32_t ColsInsert;
        uint32_t ColsWhere;
        };
    union
        {
        uint32_t ColsUpdate;
        uint32_t ColsSelect;
        };
    uint32_t ColsNull;
    DbInfoBase() : ColsNull(0), ColsInsert(0), ColsUpdate(0) {}
    };

struct DbECSchemaInfo : DbInfoBase
    {
public:
    enum Columns
        {
        COL_Id              = 0x001,
        COL_Name            = 0x002,
        COL_Description     = 0x004,
        COL_NamespacePrefix = 0x008,
        COL_VersionMajor    = 0x010,
        COL_VersionMinor    = 0x020,
        COL_DisplayLabel    = 0x100,
        COL_ALL             = 0xffffffff
        };
    ECSchemaId          m_ecSchemaId;
    Utf8String          m_name;
    Utf8String          m_displayLabel;
    Utf8String          m_description;
    Utf8String          m_namespacePrefix;
    int32_t             m_versionMajor;
    int32_t             m_versionMinor;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECClassInfo : DbInfoBase
    {
public:
    enum Columns
        {
        COL_Id                        = 0x000001,
        COL_Name                      = 0x000002,
        COL_Description               = 0x000004,
        COL_IsDomainClass             = 0x000008,
        COL_IsStruct                  = 0x000010,
        COL_IsCustomAttribute         = 0x000020,
        COL_RelationStrength          = 0x000800,
        COL_DisplayLabel              = 0x004000,
        COL_SchemaId                  = 0x008000,
        COL_IsRelationship            = 0x010000,
        COL_RelationStrengthDirection = 0x020000,
        COL_ALL                       = 0xffffffff
        };      
    ECClassId                  m_ecClassId;
    ECSchemaId                 m_ecSchemaId;
    Utf8String                 m_name;
    Utf8String                 m_displayLabel;
    Utf8String                 m_description; 
    bool                       m_isDomainClass;
    bool                       m_isStruct; 
    bool                       m_isCustomAttribute;    
    StrengthType               m_relationStrength;
    ECRelatedInstanceDirection m_relationStrengthDirection;
    bool                       m_isRelationship;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbBaseClassInfo : DbInfoBase
    {
public:
    enum Columns
        {
        COL_ClassId     = 0x1,
        COL_BaseClassId = 0x2,
        COL_Ordinal     = 0x4,
        COL_ALL         = 0xffffffff
        };
    ECClassId m_ecClassId;
    ECClassId m_baseECClassId;
    int32_t   m_ecIndex;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECPropertyInfo : DbInfoBase
    {
public:
    enum Columns
        {
        COL_Id = 0x0001,
        COL_ClassId = 0x0002,
        COL_Name = 0x0004,
        COL_DisplayLabel = 0x0008,
        COL_Description = 0x0010,
        COL_IsArray = 0x0020,
        COL_PrimitiveType = 0x0040,
        COL_StructType = 0x0080,
        COL_Ordinal = 0x0100,
        COL_IsReadOnly = 0x0200,
        COL_MinOccurs = 0x1000,
        COL_MaxOccurs = 0x2000,

        COL_ALL = 0xffffffff
        };

    ECClassId     m_ecClassId;
    ECPropertyId  m_ecPropertyId;
    Utf8String    m_name;
    Utf8String    m_displayLabel;
    Utf8String    m_description; 
    bool          m_isArray;
    PrimitiveType m_primitiveType;
    ECClassId     m_structType;
    uint32_t      m_minOccurs;
    uint32_t      m_maxOccurs;
    int32_t       m_ordinal;

    bool          m_isReadOnly;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECRelationshipConstraintInfo : DbInfoBase
    {
public:
    enum Columns
        {
        COL_ClassId             = 0x01,
        COL_RelationshipEnd     = 0x02,
        COL_CardinalityLowerLimit = 0x04,
        COL_CardinalityUpperLimit = 0x08,
        COL_RoleLabel             = 0x10,
        COL_IsPolymorphic         = 0x20,
        COL_ALL                   = 0xffffffff        
        };
    
    ECClassId         m_ecClassId;
    ECRelationshipEnd m_ecRelationshipEnd;
    uint32_t          m_cardinalityLowerLimit;
    uint32_t          m_cardinalityUpperLimit;
    Utf8String        m_roleLabel;
    bool              m_isPolymorphic;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECRelationshipConstraintClassInfo : DbInfoBase
    {
public:
    enum Columns
        {
        COL_ClassId               = 0x01,
        COL_RelationshipEnd       = 0x02,
        COL_RelationClassId       = 0x04,
        COL_ALL                     = 0xffffffff        
        };
    
    ECClassId         m_ecClassId;    
    ECRelationshipEnd m_ecRelationshipEnd;
    ECClassId         m_relationECClassId;    
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECRelationshipConstraintClassPropertyInfo : DbInfoBase
    {
public:
    enum Columns
        {
        COL_ClassId            = 0x01,
        COL_RelationshipEnd    = 0x02,
        COL_RelationClassId    = 0x04,
        COL_RelationPropertyId = 0x08,
        COL_ALL                  = 0xffffffff
        };
    
    ECClassId         m_ecClassId;    
    ECRelationshipEnd m_ecRelationshipEnd;
    ECClassId         m_relationECClassId;    
    ECPropertyId      m_relationECPropertyId;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbCustomAttributeInfo : DbInfoBase
    {
public:
    enum Columns
        {
        COL_ContainerId            = 0x01,
        COL_ContainerType          = 0x02,
        COL_ClassId              = 0x04,
        COL_Ordinal                  = 0x08,
        COL_Instance               = 0x10,
        COL_ALL                    = 0xffffffff
        }; 

private:
    Utf8String        m_caInstanceXml;

public:
    //TODO should be made private eventually, too
    ECContainerId     m_containerId;        
    ECContainerType   m_containerType;
    ECClassId         m_ecClassId;
    int32_t           m_index;

    BentleyStatus     SerializeCaInstance (IECInstanceR caInstance);
    BentleyStatus     DeserializeCaInstance (IECInstancePtr& caInstance, ECSchemaCR schema) const;

    //! Sets the CA instance XML string in this object.
    void SetCaInstanceXml (Utf8CP caInstanceXml);

    //! Gets the CA instance XML string
    Utf8CP GetCaInstanceXml () const;
    void Clear()
        {
        m_containerId=0;
        m_containerType = ECONTAINERTYPE_Schema;
        m_ecClassId=0;
        m_caInstanceXml.clear();
        }
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECSchemaReferenceInfo : DbInfoBase
    {
public:
    enum Columns
        {
        COL_SchemaId          = 0x01,
        COL_ReferencedSchemaId = 0x02,
        COL_ALL               = 0xffffffff
        };
    
    ECSchemaId m_ecSchemaId;
    ECSchemaId m_referencedECSchemaId;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
