/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/RefCounted.h>
#include <Bentley/Nullable.h>
#include <Bentley/DateTime.h>
#include <Bentley/ByteStream.h>
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/SchemaComparer.h>

#define SCHEMAMERGER_OPTION_MERGEONLYDYNAMICSCHEMAS  "#onlyDynamicSchemas"
#define SCHEMAMERGER_OPTION_SKIPSTANDARDSCHEMAS      "#skipStandardSchemas"
#define SCHEMAMERGER_OPTION_KEEPVERSION              "#keepVersion"
#define SCHEMAMERGER_OPTION_IGNOREINCOMPATIBLEPROPERTYTYPECHANGES           "#ignoreIncompatiblePropertyTypeChanges"

#define SCHEMAMERGER_DUMPSCHEMAS                     "#dumpSchemas"
#define SCHEMAMERGER_DUMPLOCATION                    "#dumpLocation"

#define SCHEMAMERGER_RENAMESCHEMAITEMONCONFLICT               "#renameSchemaItemOnConflict"
#define SCHEMAMERGER_RENAMEPROPERTYONCONFLICT               "#renamePropertyOnConflict"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct SchemaMergeResult final
    {
    friend struct SchemaMerger;
private:
    ECSchemaCache m_schemaCache;
    IssueReporter m_issueReporter; //Once ECobjects issue reporting is available
    bvector<ECSchemaP> m_modifiedSchemas;

public:
    SchemaMergeResult() : m_schemaCache(), m_issueReporter() {}

    bvector<ECSchemaCP> GetResults() const { return m_schemaCache.GetSchemas(); }
    ECSchemaCache& GetSchemaCache() { return m_schemaCache; }
    ECOBJECTS_EXPORT bool ContainsSchema(Utf8CP schemaName) const;
    ECOBJECTS_EXPORT ECSchemaP GetSchema(Utf8CP schemaName) const;
    IssueReporter const& Issues() const { return m_issueReporter; }
    BentleyStatus AddIssueListener(IIssueListener const& issueListener) { return m_issueReporter.AddListener(issueListener); }
    bvector<ECSchemaP> GetModifiedSchemas() const { return m_modifiedSchemas; }
    };

struct SchemaMergeOptions final
    {
private:
    BeJsDocument m_json;
public:
    SchemaMergeOptions() {}

    //! Instructs the merger to only merge schemas which are marked as dynamic schemas. Defaults to false.
    void SetMergeOnlyDynamicSchemas(bool flag) { m_json[SCHEMAMERGER_OPTION_MERGEONLYDYNAMICSCHEMAS] = flag; }
    bool GetMergeOnlyDynamicSchemas() const { return m_json[SCHEMAMERGER_OPTION_MERGEONLYDYNAMICSCHEMAS].asBool(false); }

    //! Instructs the merger to skip over standard schemas and never merge them
    //! Defaults to true
    void SetSkipStandardSchemas(bool flag) { m_json[SCHEMAMERGER_OPTION_SKIPSTANDARDSCHEMAS] = flag; }
    bool GetSkipStandardSchemas() const { return m_json[SCHEMAMERGER_OPTION_SKIPSTANDARDSCHEMAS].asBool(true); }

    //! Setting this flag indicates that schema versions will not be merged
    //! defaults to false, which means the higher version number be used.
    void SetKeepVersion(bool flag) { m_json[SCHEMAMERGER_OPTION_KEEPVERSION] = flag; }
    bool GetKeepVersion() const { return m_json[SCHEMAMERGER_OPTION_KEEPVERSION].asBool(false); }

    //! Setting this flag instructs the merger to skip over illegal property type changes and just keep the type
    //! specified on the left hand side schema. Defaults to false, which means the merger will raise an error
    void SetIgnoreIncompatiblePropertyTypeChanges(bool flag) { m_json[SCHEMAMERGER_OPTION_IGNOREINCOMPATIBLEPROPERTYTYPECHANGES] = flag; }
    bool GetIgnoreIncompatiblePropertyTypeChanges() const { return m_json[SCHEMAMERGER_OPTION_IGNOREINCOMPATIBLEPROPERTYTYPECHANGES].asBool(false); }

    //! Instructs the merger to dump schemas to the given location
    void SetDumpSchemas(Utf8String path) { m_json[SCHEMAMERGER_DUMPSCHEMAS] = true; m_json[SCHEMAMERGER_DUMPLOCATION] = path; }
    void ResetDumpSchemas() { m_json.removeMember(SCHEMAMERGER_DUMPSCHEMAS); m_json.removeMember(SCHEMAMERGER_DUMPLOCATION); }
    bool GetDumpSchemas() const { return m_json[SCHEMAMERGER_DUMPSCHEMAS].asBool(false); }
    Utf8String GetDumpSchemaLocation() const { return m_json[SCHEMAMERGER_DUMPLOCATION].asString(); }

    //! Setting this flag makes the merger automatically rename schema items on a name conflict.
    //! Defaults to false
    void SetRenameSchemaItemOnConflict(bool flag) { m_json[SCHEMAMERGER_RENAMESCHEMAITEMONCONFLICT] = flag; }
    bool GetRenameSchemaItemOnConflict() const { return m_json[SCHEMAMERGER_RENAMESCHEMAITEMONCONFLICT].asBool(false); }

    //! Setting this flag makes the merger automatically rename properties on a name conflict.
    //! Defaults to false
    void SetRenamePropertyOnConflict(bool flag) { m_json[SCHEMAMERGER_RENAMEPROPERTYONCONFLICT] = flag; }
    bool GetRenamePropertyOnConflict() const { return m_json[SCHEMAMERGER_RENAMEPROPERTYONCONFLICT].asBool(false); }

    Utf8String GetJson() const { return m_json.ToUtf8CP(); }
    void ReadFromJson(Utf8CP json) { return m_json.Parse(json); }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaMerger final
{
private:
    SchemaMerger() = delete;
    ~SchemaMerger() = delete;
public:

    //! Merges a vector of schemas with existing ones
    //! @param[in] schemasToMerge - The new schemas which should be merged
    //! @param[in] existingSchemas - All existing schemas
    //! @param[out] mergedSchemas - returns the result of the merging process
    ECOBJECTS_EXPORT static BentleyStatus MergeSchemas(SchemaMergeResult& result, bvector<ECSchemaCP> const& left, bvector<ECSchemaCP> const& right, SchemaMergeOptions const& options = SchemaMergeOptions());

    using ShouldMergeSchemaFunc = std::function<bool(ECSchemaCP schema)>;

private:
    static BentleyStatus MergeSchema(SchemaMergeResult& result, ECSchemaP left, ECSchemaCP right, RefCountedPtr<SchemaChange> schemaChange, SchemaMergeOptions const& options);
    static BentleyStatus MergeClass(SchemaMergeResult& result, ECClassP left, ECClassCP right, RefCountedPtr<ClassChange> classChange, SchemaMergeOptions const& options);
    static BentleyStatus MergeProperty(SchemaMergeResult& result, ECPropertyP left, ECPropertyCP right, RefCountedPtr<PropertyChange> propertyChange, SchemaMergeOptions const& options);
    static BentleyStatus MergeRelationshipConstraint(SchemaMergeResult& result, ECRelationshipClassP left, ECClassCP right, RelationshipConstraintChange& change, SchemaMergeOptions const& options, bool isSource);

    static BentleyStatus MergeKindOfQuantity(SchemaMergeResult& result, KindOfQuantityP left, KindOfQuantityCP right, RefCountedPtr<KindOfQuantityChange> change, SchemaMergeOptions const& options);
    static BentleyStatus MergeEnumeration(SchemaMergeResult& result, ECEnumerationP left, ECEnumerationCP right, RefCountedPtr<EnumerationChange> change, SchemaMergeOptions const& options);
    static BentleyStatus MergePropertyCategory(SchemaMergeResult& result, PropertyCategoryP left, PropertyCategoryCP right, RefCountedPtr<PropertyCategoryChange> change, SchemaMergeOptions const& options);

    static BentleyStatus MergePhenomenon(SchemaMergeResult& result, PhenomenonP left, PhenomenonCP right, RefCountedPtr<PhenomenonChange> change, SchemaMergeOptions const& options);
    static BentleyStatus MergeUnitSystem(SchemaMergeResult& result, UnitSystemP left, UnitSystemCP right, RefCountedPtr<UnitSystemChange> change, SchemaMergeOptions const& options);
    static BentleyStatus MergeUnit(SchemaMergeResult& result, ECUnitP left, ECUnitCP right, RefCountedPtr<UnitChange> change, SchemaMergeOptions const& options);
    static BentleyStatus MergeFormat(SchemaMergeResult& result, ECFormatP left, ECFormatCP right, RefCountedPtr<FormatChange> change, SchemaMergeOptions const& options);

    template <typename T, typename TSetter = T, typename TParent> //TSetter may differ from T, being the const or reference version of T
    static BentleyStatus MergePrimitive(PrimitiveChange<T>& change, TParent* parent, ECObjectsStatus(TParent::*setPrimitive)(TSetter), Utf8CP parentKey, SchemaMergeResult& result, SchemaMergeOptions const& options, bool preferLeftValue = true);
    static void MergeCustomAttributes(SchemaMergeResult& result, CustomAttributeChanges& changes, Utf8CP scopeDescription, IECCustomAttributeContainerP left, IECCustomAttributeContainerCP right);

    template <typename TItemChange, typename TItem>
    using TMergeItem = BentleyStatus(*)(SchemaMergeResult&, TItem *, const TItem *, RefCountedPtr<TItemChange>, const SchemaMergeOptions &);
    template <typename TItem>
    using TCopyItem = ECObjectsStatus(ECSchema::*)(TItem * &, const TItem &, bool, Utf8CP);
    template <typename TItem>
    using TGetItemCP = const TItem *(ECSchema::*)(Utf8CP) const;
    template <typename TItem>
    using TGetItemP = TItem *(ECSchema::*)(Utf8CP);
    template <typename TItemChange, typename TItem>
    static BentleyStatus MergeItems(SchemaMergeResult& result, ECSchemaP left, ECSchemaCP right, SchemaMergeOptions const& options, ECChangeArray<TItemChange>& changes, TGetItemCP<TItem> getItemCP, TGetItemP<TItem> getItemP, TCopyItem<TItem> copyItem, TMergeItem<TItemChange, TItem> mergeItem);

    template <typename T> 
    using SchemaItemGetterFunc = std::function<T(ECSchemaP schema, Utf8StringCR itemName)>;
    template <typename T> 
    using SchemaItemSetterFunc = std::function<ECObjectsStatus(T value)>;

    template <typename T>
    static BentleyStatus MergeReferencedSchemaItem(SchemaMergeResult& result, StringChange& change, SchemaItemSetterFunc<T> setterFunc, SchemaItemGetterFunc<T> getterFunc, Utf8CP parentKey, SchemaMergeOptions const& options);
    static ECSchemaCP FindSchemaByName(bvector<ECSchemaCP> const& schemas, Utf8CP schemaName);
};

END_BENTLEY_ECOBJECT_NAMESPACE
