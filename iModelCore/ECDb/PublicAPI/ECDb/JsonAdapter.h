/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/JsonAdapter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECSqlStatement.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/BeRapidJson.h>
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! Describes the formatting of EC property values 
//! @see ECPropertyFormatter, JsonECSqlSelectAdapter
//! @ingroup ECDbGroup
//! @bsiclass                                                 Ramanujam.Raman      10/2013
//+===============+===============+===============+===============+===============+======
enum class ECValueFormat
    {
    RawNativeValues, //!< The values are output in the original native type without any formatting.
    FormattedStrings //!< The values are output as formatted strings. 
    };

struct ECPropertyFormatter;
typedef ECPropertyFormatter* ECPropertyFormatterP;
typedef RefCountedPtr<ECPropertyFormatter> ECPropertyFormatterPtr;

//=======================================================================================
//! Provides a means of extending the formatting capabilities of the JsonECSqlSelectAdapter
//! @see JsonECSqlSelectAdapter
//! @ingroup ECDbGroup
//! @bsiclass                                                Ramanujam.Raman      10/2013
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECPropertyFormatter : RefCounted<NonCopyableClass>
    {
protected:
    ECPropertyFormatter () {}
    virtual ~ECPropertyFormatter () {}

    //! Formats an EC property value as a string
    ECDB_EXPORT virtual bool _FormattedStringFromECValue
        (
        Utf8StringR strVal,
        ECN::ECValueCR ecValue,
        ECN::ECPropertyCR ecProperty,
        bool isArrayMember
        ) const;

    //! Gets the category corresponding to a property 
    ECDB_EXPORT virtual ECN::IECInstancePtr _GetPropertyCategory (ECN::ECPropertyCR ecProperty);

public:
    static ECPropertyFormatterPtr Create () { return new ECPropertyFormatter (); }

    bool FormattedStringFromECValue (Utf8StringR strVal, ECN::ECValueCR ecValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const
        {
        return _FormattedStringFromECValue (strVal, ecValue, ecProperty, isArrayMember);
        }

    ECN::IECInstancePtr GetPropertyCategory (ECN::ECPropertyCR ecProperty)
        {
        return _GetPropertyCategory (ecProperty);
        }
    };

//=======================================================================================
//! Adapts the rows returned by ECSqlStatement to the JSON format. 
//! @see ECSqlStatement, ECInstanceECSqlSelectAdapter
//! @ingroup ECDbGroup
//! @bsiclass                                                 Ramanujam.Raman      08/2012
//+===============+===============+===============+===============+===============+======
struct JsonECSqlSelectAdapter : NonCopyableClass
    {
#if !defined (DOCUMENTATION_GENERATOR)
 public:
    struct PropertyTreeNode;
    typedef PropertyTreeNode* PropertyTreeNodeP;
    typedef const PropertyTreeNode* PropertyTreeNodeCP;
    typedef PropertyTreeNode& PropertyTreeNodeR;
    typedef const PropertyTreeNode& PropertyTreeNodeCR;

    typedef bmap<Utf8String, PropertyTreeNodeP > PropertyTreeNodeByName;
    typedef bmap<Utf8String, bvector<PropertyTreeNodeCP > > PropertyTreeNodesByCategory;

    struct PropertyTree;
    typedef const PropertyTree& PropertyTreeCR;

public:
    //========================================================================================
    //* @bsiclass                                                 Ramanujam.Raman      12/2012
    //+===============+===============+===============+===============+===============+=======
    struct PropertyTreeNode
        {
    friend struct PropertyTree;
    friend struct JsonECSqlSelectAdapter;
    private:
        ECN::ECPropertyCP           m_property;
        ECN::ECClassCP              m_class;
        int                         m_instanceIndex;
        PropertyTreeNodeByName      m_childNodes;

        // Disable copy
        PropertyTreeNode (PropertyTreeNodeCR other);
        PropertyTreeNode& operator= (PropertyTreeNodeCR other);

        PropertyTreeNode ()
            : m_property (nullptr), m_class (nullptr), m_instanceIndex (-1) {}
        PropertyTreeNode (ECN::ECPropertyCP ecProperty, ECN::ECClassCP ecClass, int instanceIndex)
            : m_property (ecProperty), m_class (ecClass), m_instanceIndex (instanceIndex) {}

        ~PropertyTreeNode ()
            {
            for (auto it : m_childNodes)
                delete it.second;
            }
        };

    //========================================================================================
    //* @bsiclass                                                 Ramanujam.Raman      12/2012
    //+===============+===============+===============+===============+===============+=======
    struct PropertyTree
        {
        friend struct JsonECSqlSelectAdapter;
        private:
            PropertyTreeNode m_rootNode;

            PropertyTree () {}
            void AddInlinedStructNodes (PropertyTreeNodeR parentNode, IECSqlValue const& ecSqlValue, int instanceIndex);
            void AddChildNodes (PropertyTreeNodeR parentNode, bvector<ECN::ECClassCP>& rootClasses, IECSqlValue const& ecSqlValue);
        };

#endif

    //=======================================================================================
    //! Options to control the format of the JSON returned by methods in the JsonECSqlSelectAdapter.
    //! @see JsonECSqlSelectAdapter
    // @bsiclass                                                Ramanujam.Raman      10/2012
    //+===============+===============+===============+===============+===============+======
    struct FormatOptions
        {
#if !defined (DOCUMENTATION_GENERATOR)
        friend struct JsonECSqlSelectAdapter;
#endif
        private:
            ECValueFormat m_format;
            ECPropertyFormatterPtr m_propertyFormatter;

        public:
            //! Construct the options to control the format of the JSON returned by methods in the @ref JsonECSqlSelectAdapter.
            //! @param[in] format Specifies the format of the property values. @see ECValueFormat. 
            //! @param[in] propertyFormatter (Optional) Provides more control over the formatting of properties
            //! Normally the DgnPlatform layer provides formatters that use the dgn specific context to provide additional
            //! formatting including specification of units, property categorization, etc. 
            ECDB_EXPORT FormatOptions(ECValueFormat format = ECValueFormat::FormattedStrings, ECPropertyFormatterP propertyFormatter = nullptr);
        };

private:
    ECSqlStatementCR m_ecsqlStatement;
    FormatOptions m_formatOptions;

    static bool PrioritySortPredicate (const ECN::IECInstancePtr& priorityCA1, const ECN::IECInstancePtr& priorityCA2);
    static bool PropertySortPredicate (PropertyTreeNodeCP propertyNode1, PropertyTreeNodeCP propertyNode2);

    void CategorizeProperties (bvector<ECN::IECInstancePtr>& categories, PropertyTreeNodesByCategory& nodesByCategory, const PropertyTreeNodeByName& nodes) const;
    void SortProperties (bvector<ECN::IECInstancePtr>& categories, PropertyTreeNodesByCategory& nodesByCategory) const;

    void JsonFromPropertyTree (JsonValueR jsonValue, PropertyTreeCR propertyTree) const;

    void JsonFromPropertyRecursive (JsonValueR jsonValue, PropertyTreeNodeCR propertyTreeNode) const;
    void JsonFromProperty (JsonValueR propertyJson, ECN::ECPropertyCR ecProperty, ECN::ECClassCR ecClass, int instanceIndex) const;
    void JsonFromCategory (JsonValueR jsonValue, ECN::IECInstancePtr& IECSqlBinderCustomAttribute, const PropertyTreeNodesByCategory& nodesByCategory) const;

    void JsonFromClassesRecursive (JsonValueR jsonValue, bset<ECN::ECClassCP>& classes, PropertyTreeNodeCR propertyTreeNode) const;
    static Utf8String GetClassKey (ECN::ECClassCR ecClass);
    void JsonFromClass (JsonValueR jsonValue, ECN::ECClassCR ecClass) const;

    static bool GetIntegerValue (int& value, ECN::IECInstanceCR instance, Utf8CP propertyName);
    static bool GetStringValue (Utf8String& value, ECN::IECInstanceCR instance, Utf8CP propertyName);
    static bool GetBooleanValue (bool& value, ECN::IECInstanceCR instance, Utf8CP propertyName);
    static bool GetPriorityFromCustomAttribute (int& priority, ECN::IECInstancePtr priorityCA);
    static ECN::ECClassCP GetClassFromStructOrStructArray (ECN::ECPropertyCR ecProperty);
    static bool GetPriorityFromProperty (int& priority, ECN::ECPropertyCR ecProperty);

    bool JsonFromPropertyValue (JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const;
    bool JsonFromCell (JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const;

    bool JsonFromPrimitive (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;
    bool JsonFromStruct (JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const;
    bool JsonFromArray (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR) const;
    bool JsonFromStructArray (JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const;
    bool JsonFromPrimitiveArray (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty) const;
    bool JsonFromBinary (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;
    bool JsonFromBoolean (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;
    bool JsonFromDateTime (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;
    bool JsonFromDouble (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;
    bool JsonFromInt (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;
    bool JsonFromInt64 (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;
    bool JsonFromPoint2D (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;
    bool JsonFromPoint3D (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;
    bool JsonFromString (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;
    bool JsonFromCG(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;

    bool FormattedJsonFromECValue (JsonValueR jsonValue, ECN::ECValueCR ecValue, ECN::ECPropertyCR ecProperty, bool isArrayMember) const;

    void JsonFromClassKey (JsonValueR jsonValue, ECN::ECClassCR ecClass) const;
    void JsonFromClassLabel (JsonValueR jsonValue, ECN::ECClassCR ecClass) const;
    void JsonFromInstanceLabel (JsonValueR jsonValue, ECN::ECClassCR ecClass) const;
    bool JsonFromInstanceId (JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const;

public:

    //! Creates a new instance of the adapter
    //! @param[in] ecsqlStatement Prepared ECSQL statement
    //! @param[in] formatOptions Options to control the output. 
    //! @see ECSqlStatement
    ECDB_EXPORT JsonECSqlSelectAdapter(ECSqlStatement const& ecsqlStatement, JsonECSqlSelectAdapter::FormatOptions formatOptions = JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::FormattedStrings));
    virtual ~JsonECSqlSelectAdapter() {}

    //! Gets the presentation meta-data for displaying the individual rows read in. 
    //! @param [out] rowDisplayInfo Includes information to display the various property columns in the correct order. 
    //! @remarks
    //! The properties include keys that point to information on their classes, and instance indices that point to 
    //! the property values in the JSON obtained through @see GetRow(). 
    //!
    //! @code
    //! {
    //!     "Categories" : [CategoryInfo1, CategoryInfo2, ...] // Ordered array value of categories
    //!     "Classes" : {"ClassKey1" : ClassInfo1, "ClassKey2" : ClassInfo2, ...] // Map of class keys and information
    //! }
    //! 
    //! where each CategoryInfo:
    //! {
    //!     "CategoryName"  : <StringValue>,
    //!     "DisplayLabel"  : <StringValue>,
    //!     "Expand"        : <BooleanValue>,
    //!     "Priority"      : <IntegerValue>,
    //!     "Properties"    : [PropertyInfo1, PropertyInfo2, ...] // Ordered array value of properties
    //! }
    //! 
    //! where each PropertyInfo:
    //! {
    //!     "Name"          : <StringValue>,
    //!     "DisplayLabel"  : <StringValue>,
    //!     "IsPrimitive"   : <BooleanValue>,
    //!     "PrimitiveType" : <StringValue>,
    //!     "Priority"      : <IntegerValue>,
    //!     "Categories"    : <JsonValue> // Recursive sub-categories for embedded structs and struct arrays. 
    //!     "ClassKey"      : <StringValue>, // Key to the top level "Classes" objectValue. Provides more information on the class. 
    //!     "InstanceIndex" : <IntegerValue>, // Index to instance returned by @see GetRow(). Set to nullValue for embedded properties. 
    //! }
    //! 
    //! where each ClassInfo:
    //!     {
    //!     "Name"          : <StringValue>,
    //!     "DisplayLabel"  : <StringValue>,
    //!     "IsPrimitive"   : <BooleanValue>,
    //!     "SchemaName"    : <StringValue>,
    //!     "RelationshipPath" : <StringValue> // Unused field in this API (set to nullValue), but is used by other API to provide meaningful context. 
    //!     }
    //! 
    //! @endcode
    //! @see GetRow()
    ECDB_EXPORT void GetRowDisplayInfo(JsonValueR rowDisplayInfo) const;

    //! Gets the entire current row as JSON
    //! @param [out] currentRow Property values for the current row.
    //! @return false if there was an error in retrieving/formatting values. true otherwise.
    //! @remarks 
    //! The values are organized as an array of values, each representing an instance. 
    //! Use @ref JsonECSqlSelectAdapter::GetRowDisplayInfo to obtain the corresponding information to display the individual
    //! columns. 
    //! Summary information on instances can be obtained through special property keys starting
    //! with "$". e.g., $ECInstanceId, $ECClassLabel, $ECInstanceLabel. The latter two label 
    //! properties are included only if the format is setup to be @ref FormattedStrings. 
    //! @see JsonECSqlSelectAdapterGetRowDisplayInfo, JsonECSqlSelectAdapterGetRowInstance
    //! @code
    //! [
    //! {
    //!     "$ECInstanceId" : StringValue,
    //!     "$ECInstanceLabel" : StringValue,
    //!     "$ECClassLabel" : StringValue,
    //!     "$ECClassKey" : StringValue,
    //!     "PropertyName1": <ECJsonValue>
    //!     "PropertyName2": <ECJsonValue>
    //!     ...
    //! },
    //! {
    //!     "$ECInstanceId" : StringValue,
    //!     "$ECInstanceLabel" : StringValue,
    //!     "$ECClassLabel" : StringValue,
    //!     "$ECClassKey" : StringValue,
    //!     "PropertyName3": <ECJsonValue>
    //!     "PropertyName4": <ECJsonValue>
    //!     ...
    //! }
    //! ...
    //! ]
    //! }
    //! 
    //! where ECJsonValue:
    //! NullValue | PrimitiveValue | StructValue | ArrayValue
    //! 
    //! where PrimitiveValue:
    //! RawValue | FormattedValue // Depending on the formatting option passed in
    //! 
    //! RawValue: e.g.,
    //!     Integer:  2, 
    //!     Double: 3.5
    //!     DateTime: "2013-01-08T08:12:04.523" (ISO string)
    //!     Point3d: {"x" : 1.1, "y" : 2.2, "z" : 3.3} (JSON object value)
    //!     String: "Test"
    //!     
    //!  FormattedValue: e.g., 
    //!     Integer: "2"
    //!     Double: "3 feet 6 inches"
    //!     DateTime: "2013-01-08T08:12:04.523" (ISO string)
    //!     Point3D: "1.1, 2.2, 3.3"
    //!     String: "Test"
    //! @endcode
    ECDB_EXPORT bool GetRow(JsonValueR currentRow) const;

    //! Gets a specific instance from the current row as JSON
    //! @param [out] ecJsonInstance JSON representation of the ECInstance as a property-value map. 
    //! @param [in] ecClassId ECClassId indicating the class of the instance needed from the current row 
    //! @return false if there was an error in retrieving/formatting values. true otherwise.
    //! @remarks The method returns only the instance of the type requested.
    //! The format of the JSON is very similar to that described in @ref GetRow() except that there 
    //! is just one top level instance, instead of an array of instances. 
    //! @code
    //! [
    //! {
    //!     "$ECInstanceId" : StringValue,
    //!     "$ECInstanceLabel" : StringValue,
    //!     "$ECClassLabel" : StringValue,
    //!     "PropertyName1": <ECJsonValue>
    //!     "PropertyName2": <ECJsonValue>
    //!     ...
    //! }
    //! @endcode
    //! @see JsonECSqlSelectAdapter::GetRow
    ECDB_EXPORT bool GetRowInstance(JsonValueR ecJsonInstance, ECN::ECClassId ecClassId) const;

    
    //! Gets the first instance from the current row as JSON
    //! @param [out] ecJsonInstance JSON representation of the ECInstance as a property-value map. 
    //! @return false if there was an error in retrieving/formatting values. true otherwise.
    //! @remarks Uses the class of the first column, and thereafter ignores any columns that don't 
    //! belong to the same class. 
    //! The format of the JSON is very similar to that described in @ref GetRow() except that there 
    //! is just one top level instance, instead of an array of instances. 
    //! @code
    //! [
    //! {
    //!     "$ECInstanceId" : StringValue,
    //!     "$ECInstanceLabel" : StringValue, 
    //!     "$ECClassLabel" : StringValue,
    //!     "PropertyName1": <ECJsonValue>
    //!     "PropertyName2": <ECJsonValue>
    //!     ...
    //! }
    //! @endcode
    //! @see JsonECSqlSelectAdapter::GetRow
    ECDB_EXPORT bool GetRowInstance(JsonValueR ecJsonInstance) const;

    //! Gets the current row as is in the JSON format
    //! @param [out] ecJsonRow JSON representation of the row as an ordered array of values
    //! @return false if there was an error in retrieving/formatting values. true otherwise.
    ECDB_EXPORT bool GetRowAsIs(JsonValueR ecJsonRow) const;
    };

//=================================================================================
//! Gets a cache of related items display specifications 
//! @remarks Related Items Display Specifications are specified as schema custom attributes
//! and used to identify all relationship paths originating from a parent class that need to 
//! be presented along side the parent instance
//! @ingroup ECDbGroup
// @bsiclass                                                 Ramanujam.Raman      09/2013
//+===============+===============+===============+===============+===============+======
struct RelatedItemsDisplaySpecificationsCache : public BeSQLite::Db::AppData
{
    struct RelationshipPathInfo
    {
        Utf8String m_path;
        ECN::ECSchemaCR m_defaultSchema;
        bset<ECN::ECClassCP> m_derivedClasses;

        RelationshipPathInfo(Utf8CP path, ECN::ECSchemaCR defaultSchema) : m_path(path), m_defaultSchema(defaultSchema) {}

        RelationshipPathInfo(RelationshipPathInfo const& other) : m_defaultSchema(other.m_defaultSchema)
            {
            *this = other;
            }

        RelationshipPathInfo& operator= (RelationshipPathInfo const& other)
            {
            m_path = other.m_path;
            m_derivedClasses = other.m_derivedClasses;
            return *this;
            }

        void InsertDerivedClass(ECN::ECClassCR derivedClass)
            {
            m_derivedClasses.insert(&derivedClass);
            }
    };

    typedef bmap<ECN::ECClassId, bvector<ECN::ECRelationshipPath>> RelationshipPathsByClassId;
    typedef bmap<ECN::ECClassCP, bvector<RelationshipPathInfo>> RelationshipPathInfosByClass;

private:
    ECDbCR m_ecDb;
    RelationshipPathsByClassId m_pathsByClass;

    BentleyStatus GatherRelationshipPathInfos(RelationshipPathInfosByClass& pathInfosByClass) const;
    BentleyStatus GatherRelationshipPathInfos(RelationshipPathInfosByClass& pathInfosByClass, ECN::ECSchemaCR customAttributeContainerSchema, ECN::IECInstanceCR customAttributeSpecification) const;
    RelationshipPathInfo& AddEntryToRelationshipPathInfos(RelationshipPathInfosByClass& pathInfosByClass, ECN::ECClassCR parentClass, Utf8CP path, ECN::ECSchemaCR defaultSchema) const;

    void RemoveDuplicates(RelationshipPathInfosByClass& pathInfoByClass) const;
    void SortRelationshipPaths();

    BentleyStatus ExtractRelationshipPaths(RelationshipPathInfosByClass const& pathInfosByClass);

    void AddEntryToCache(ECN::ECClassCR parentClass, ECN::ECRelationshipPath const& path);

    void DumpCache(ECN::ECClassCP ecClass = nullptr) const;

    ECN::ECClassCP ResolveClass(Utf8StringCR possiblyQualifiedClassName, ECN::ECSchemaCR defaultSchema) const;

    static BeSQLite::Db::AppData::Key const& GetKey() { static BeSQLite::Db::AppData::Key s_key; return s_key; }

    explicit RelatedItemsDisplaySpecificationsCache(ECDbCR ecDb) : Db::AppData(), m_ecDb(ecDb) {}

    void Initialize();
public:
    ~RelatedItemsDisplaySpecificationsCache() {}

    //! Get the RelatedItemsDisplaySpecificationCache for the specified ECDb
    ECDB_EXPORT static RelatedItemsDisplaySpecificationsCache* Get(ECDbCR);

    //! Get all related paths given a parent class
    ECDB_EXPORT bool TryGetRelatedPaths(bvector<ECN::ECRelationshipPath>& relationshipPaths, ECN::ECClassCR parentClass) const;
};

//=================================================================================
//! Reads information associated with an instance of the class in the JSON format.
//! @remarks This is mainly a convenience wrapper over @ref ECSqlStatement and
//! @ref JsonECSqlSelectAdapter. The recommended use is for a simple retrieval of instances
//! of a class. The utility however also provides the capability to gather other
//! instances that have been deemed to be retrieved along with the requested
//! instance for display purposes through the RelatedItemsDisplaySpecification
//! custom attribute.
//! @ingroup ECDbGroup
// @bsiclass                                                 Ramanujam.Raman      09/2013
//+===============+===============+===============+===============+===============+======
struct JsonReader : NonCopyableClass
{
private:
    ECDbCR m_ecDb;
    ECN::ECClassCP m_ecClass;
    ECSqlStatementCache m_statementCache;

     BentleyStatus GenerateECSql(Utf8StringR ecSql, ECN::ECRelationshipPath const& pathFromRelatedClass, bool selectInstanceKeyOnly, bool isPolymorphic) const;
    BentleyStatus PrepareAndBindStatement(CachedECSqlStatementPtr& statement, Utf8StringCR ecSql, ECInstanceId ecInstanceId) const;

    BentleyStatus AddInstancesFromSpecifiedClassPath(JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, ECN::ECRelationshipPath const& pathToClass, ECInstanceId, JsonECSqlSelectAdapter::FormatOptions const&, bool) const;
    BentleyStatus AddInstancesFromRelatedItems(JsonValueR allInstances, JsonValueR allDisplayInfo, ECN::ECClassCR parentClass, ECN::ECRelationshipPath const& pathFromParent, ECInstanceId, JsonECSqlSelectAdapter::FormatOptions const&) const;

    BentleyStatus GetTrivialPathToSelf(ECN::ECRelationshipPath& emptyPath, ECN::ECClassCR) const;

    bool IsValid() const { return m_ecClass != nullptr; }

    static BentleyStatus AddInstancesFromPreparedStatement(JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, ECSqlStatement&, JsonECSqlSelectAdapter::FormatOptions const&, ECN::ECRelationshipPath const& pathFromRelatedClass);
    static void AddClasses(JsonValueR allClasses, JsonValueR addClasses);
    static void AddCategories(JsonValueR allCategories, JsonValueR addCategories, int currentInstanceIndex);
    static void AddInstances(JsonValueR allInstances, JsonValueR addInstances, int currentInstanceIndex);

    static void SetInstanceIndex(JsonValueR addCategories, int currentInstanceIndex);
    static void SetRelationshipPath(JsonValueR addClasses, Utf8StringCR pathToClassStr);
    static void OverrideCategories(JsonValueR addCategories, Utf8StringCR categoryName, Utf8StringCR categoryLabel);
public:

    //! Construct a reader for the specified class. 
    //! @param ecdb [in] ECDb
    //! @param ecClassId [in] ECClassId indicating the class of the instance that needs to be retrieved. 
    //! @remarks Holds some cached state to speed up future lookups of the same class. Keep the 
    //! reader around when retrieving many instances of the same class. Note that the cache 
    //! stores information on classes, and if schemas are re-imported the reader may have to be recreated.
    ECDB_EXPORT JsonReader (ECDbCR ecdb, ECN::ECClassId ecClassId);
    ~JsonReader() {}

    //! Reads the specified instance and any related instances that are to be displayed in the JSON format. 
    //! @param jsonInstances [out] Information on the instances in the JSON format.
    //! @param jsonDisplayInfo [out] Information to properly display the instances in the JSON format. 
    //! @param ecInstanceId [in] ECInstanceId pointing to the instance that needs to be retrieved. 
    //! @param formatOptions [in] Options to control the output. @see JsonECSqlSelectAdapter::FormatOptions
    //! @remarks 
    //! <ul>
    //! <li> Also gathers other instances that have been deemed to be retrieved along with the requested 
    //! instance for display purposes through the RelatedItemsDisplaySpecification custom attribute. 
    //! <li> See @ref JsonECSqlSelectAdapter::GetRowDisplayInfo for more details on the format of jsonDisplayInfo, and 
    //! See @ref JsonECSqlSelectAdapter::GetRow for the format of the jsonInstances. 
    //! <li> Note that it's not guaranteed that display info is the same for every every instance of the 
    //! class, i.e., multiple calls to the method with different ECInstanceId-s. The display info could be different
    //! depending on whether a related instance was retrieved or not along with the requested instance.
    //! </ul>
    //! @see JsonECSqlSelectAdapter::ReadInstance
    ECDB_EXPORT BentleyStatus Read(JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, ECInstanceId ecInstanceId, JsonECSqlSelectAdapter::FormatOptions formatOptions = JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::FormattedStrings)) const;

    //! Reads (only) the specified instance in the JSON format. 
    //! @param [out] jsonInstance JSON representation of the ECInstance as a property-value map.  
    //! @param ecInstanceId [in] ECInstanceId pointing to the instance that needs to be retrieved. 
    //! @param formatOptions [in] Options to control the output. @see JsonECSqlSelectAdapter::FormatOptions
    //! @remarks The returned JSON is a map of properties and values for the specified instance. See 
    //! @ref JsonECSqlSelectAdapter::GetRowInstance for more details. 
    //! @see Read()
    ECDB_EXPORT BentleyStatus ReadInstance(JsonValueR jsonInstance, ECInstanceId ecInstanceId, JsonECSqlSelectAdapter::FormatOptions formatOptions = JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::FormattedStrings)) const;
};

//=======================================================================================
//! Insert JSON instances into ECDb file.
//@bsiclass                                                 Ramanujam.Raman      02/2013
//+===============+===============+===============+===============+===============+======
struct JsonInserter : NonCopyableClass
{
private:
    ECN::ECClassCR m_ecClass;
    ECInstanceInserter m_ecinstanceInserter;

public:
    //! Construct an inserter for the specified class. 
    //! @param ecdb [in] ECDb
    //! @param ecClass [in] ECClass of the instance that needs to be inserted. 
    //! @remarks Holds some cached state to speed up future inserts of the same class. Keep the 
    //! inserter around when inserting many instances of the same class. 
    ECDB_EXPORT JsonInserter (ECDbCR ecdb, ECN::ECClassCR ecClass);

    //! Indicates whether this JsonInserter is valid and can be used to insert JSON instances.
    //! It is not valid, if @p ecClass is not mapped or not instantiable for example.
    //! @return true if the inserter is valid and can be used for inserting. false if it cannot be used for inserting.
    ECDB_EXPORT bool IsValid () const;

    //! Inserts the instance
    //! @param[out] newInstanceKey the ECInstanceKey generated for the inserted instance
    //! @param[in] jsonValue the instance data
    //! @return SUCCESS if insert is successful, error code otherwise
    ECDB_EXPORT BentleyStatus Insert (ECInstanceKey& newInstanceKey, JsonValueCR jsonValue) const;

    //! Inserts the instance and updates the $ECInstanceId field with the generated ECInstanceId
    //! @param[in] jsonValue the instance data
    //! @return SUCCESS if insert is successful, error code otherwise
    ECDB_EXPORT BentleyStatus Insert (JsonValueR jsonValue) const;

    //! Insert an instance created from the specified jsonValue
    //! @param[out] newInstanceKey the ECInstanceKey generated for the inserted instance
    //! @param[in] jsonValue the instance data
    //! @return SUCCESS if insert is successful, error code otherwise
    ECDB_EXPORT BentleyStatus Insert (ECInstanceKey& newInstanceKey, RapidJsonValueCR jsonValue) const;
};

//=======================================================================================
//! Update EC content in the ECDb file through JSON values
//@bsiclass                                                 Ramanujam.Raman      02/2013
//+===============+===============+===============+===============+===============+======
struct JsonUpdater : NonCopyableClass
{
private:
    ECDbCR m_ecdb;
    ECN::ECClassCR m_ecClass;
    ECInstanceUpdater m_ecinstanceUpdater;

    ECN::IECInstancePtr CreateEmptyInstance(ECN::ECClassCR ecClass) const;
    ECN::IECInstancePtr CreateEmptyInstance(ECInstanceKeyCR instanceKey) const;
    ECN::IECInstancePtr CreateEmptyRelInstance(ECN::ECRelationshipClassCR ecRelClass, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const;

public:
    //! Construct an updater for the specified class. 
    //! @param ecdb [in] ECDb
    //! @param ecClass [in] ECClass of the instance that needs to be updated. 
    //! @param[in] ecsqlOptions ECSQLOPTIONS clause appended to the ECSQL generated by the JsonUpdater.
    //!            Pass without ECSQLOPTIONS keyword.
    //! @remarks Holds some cached state to speed up future updates of the same class. Keep the 
    //! inserter around when updating many instances of the same class. 
    ECDB_EXPORT JsonUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, Utf8CP ecsqlOptions = nullptr);

    //! Indicates whether this JsonUpdater is valid and can be used to update JSON instances.
    //! It is not valid, if @p ecClass is not mapped or not instantiable for example.
    //! @return true if the updater is valid and can be used for updating. false if it cannot be used for updating.
    ECDB_EXPORT bool IsValid () const;

    //! Updates an instance from the specified jsonValue
    //! @deprecated In favour of other overloads that take an instance id
    ECDB_EXPORT BentleyStatus Update(JsonValueCR jsonValue) const;

    //! Updates an instance from the specified jsonValue
    //! @param[in] instanceId the ECInstanceId of the instance to update
    //! @param[in] jsonValue the instance data
    //! @return SUCCESS in case of successful execution of the underlying ECSQL UPDATE. This means,
    //! SUCCESS is also returned if the specified instance does not exist in the file. ERROR otherwise.
    ECDB_EXPORT BentleyStatus Update(ECInstanceId instanceId, JsonValueCR jsonValue) const;

    //! Update  a relationship instance from the specified jsonValue and source/target keys
    //! @param[in] instanceId the ECInstanceId of the instance to update
    //! @param[in] jsonValue the instance data
    //! @param[in] sourceKey ECInstanceKey for the source of the relationship
    //! @param[in] targetKey ECInstanceKey for the target of the relationship
    //! @return SUCCESS in case of successful execution of the underlying ECSQL UPDATE. This means,
    //! SUCCESS is also returned if the specified instance does not exist in the file. ERROR otherwise.
    ECDB_EXPORT BentleyStatus Update(ECInstanceId instanceId, JsonValueCR jsonValue, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const;

    //! Update an instance from the specified jsonValue
    //! @param[in] instanceId the ECInstanceId of the instance to update
    //! @param[in] jsonValue the instance data
    //! @return SUCCESS in case of successful execution of the underlying ECSQL UPDATE. This means,
    //! SUCCESS is also returned if the specified instance does not exist in the file. ERROR otherwise.
    ECDB_EXPORT BentleyStatus Update (ECInstanceId instanceId, RapidJsonValueCR jsonValue) const;

    //! Update  a relationship instance from the specified jsonValue and source/target keys
    //! @param[in] instanceId the ECInstanceId of the instance to update
    //! @param[in] jsonValue the instance data
    //! @param[in] sourceKey ECInstanceKey for the source of the relationship
    //! @param[in] targetKey ECInstanceKey for the target of the relationship
    //! @return SUCCESS in case of successful execution of the underlying ECSQL UPDATE. This means,
    //! SUCCESS is also returned if the specified instance does not exist in the file. ERROR otherwise.
    ECDB_EXPORT BentleyStatus Update(ECInstanceId instanceId, RapidJsonValueCR jsonValue, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const;
};

//=======================================================================================
//! Delete EC content in the ECDb file
//@bsiclass                                                 Ramanujam.Raman      02/2013
//+===============+===============+===============+===============+===============+======
struct JsonDeleter : NonCopyableClass
{
private:
    ECInstanceDeleter m_ecinstanceDeleter;

public:
    //! Construct an deleter for the specified class. 
    //! @param ecdb [in] ECDb
    //! @param ecClass [in] ECClass of the instance that needs to be deleted. 
    //! @remarks Holds some cached state to speed up future deletes of the same class. Keep the 
    //! deleter around when deleting many instances of the same class. 
    ECDB_EXPORT JsonDeleter (ECDbCR ecdb, ECN::ECClassCR ecClass);

    //! Indicates whether this JsonDeleter is valid and can be used to delete instances.
    //! It is not valid, if @p ecClass is not mapped for example.
    //! @return true if the deleter is valid and can be used for deleting. false if it cannot be used for delete.
    ECDB_EXPORT bool IsValid () const;

    //! Deletes the instance identified by the supplied ECInstanceId
    //! @param[in] instanceId the ECInstanceId of the instance to delete
    //! @return SUCCESS in case of successful execution of the underlying ECSQL DELETE. This means,
    //! SUCCESS is also returned if the specified instance does not exist in the file. ERROR otherwise.
    ECDB_EXPORT BentleyStatus Delete (ECInstanceId instanceId) const;
};


END_BENTLEY_SQLITE_EC_NAMESPACE
