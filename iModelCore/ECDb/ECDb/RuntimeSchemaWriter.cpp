/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "RuntimeSchemaWriter.h"
#include "ECDbLogger.h"
#include <BeSQLite/BeSQLite.h>
#include <cinttypes>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

// Schemas excluded from the runtime binary blob. These fall into three categories:
// 1. Units/Formats: empty schemas whose items (Unit, Format, Phenomenon, UnitSystem)
//    are only referenced as strings from KindOfQuantity. Including them would add
//    zero useful classes/properties but bloat the string table.
// 2. ECDb-internal: ECDbSystem, ECDbMap, ECDbFileInfo, ECDbSchemaPolicies
//    describe the EC storage layer. No runtime consumer needs these.
//    Note: ECDbMeta is NOT excluded - consumers use it for metadata queries.
// 3. Pure CA schemas: CoreCustomAttributes, ECv3ConversionAttributes,
//    EditorCustomAttributes, BisCustomAttributes, SchemaLocalizationCustomAttributes,
//    SchemaUpgradeCustomAttributes. Their classes are only CustomAttribute and Struct
//    types used for decoration. Since the blob doesn't include CA instances, including
//    these schema definitions provides no value.
// The list is checked case-insensitively via BeStringUtilities::StricmpAscii.
static bool IsExcludedSchema(Utf8CP name)
    {
    // Sorted for readability, case-insensitive comparison
    static const Utf8CP s_excluded[] =
        {
        "BisCustomAttributes",
        "CoreCustomAttributes",
        "ECDbFileInfo",
        "ECDbMap",
        "ECDbSchemaPolicies",
        "ECDbSystem",
        "ECv3ConversionAttributes",
        "EditorCustomAttributes",
        "Formats",
        "SchemaLocalizationCustomAttributes",
        "SchemaUpgradeCustomAttributes",
        "Units",
        };
    for (auto excluded : s_excluded)
        {
        if (0 == BeStringUtilities::StricmpAscii(name, excluded))
            return true;
        }
    return false;
    }

// Binary record tags - must stay in sync with Tag enum in RuntimeSchemaBinaryReader.ts
namespace Tag
    {
    constexpr uint8_t PropertyDefTable = 0x0A; // deduplicated property definition table
    constexpr uint8_t Schema       = 0x10;
    constexpr uint8_t SchemaRef    = 0x11;
    constexpr uint8_t Enum         = 0x20;
    constexpr uint8_t KoQ          = 0x30;
    constexpr uint8_t PropCat      = 0x31;
    constexpr uint8_t Class        = 0x40;
    constexpr uint8_t BaseClass    = 0x41;
    // 0x50 was inline Property in prototypes (not used in v1)
    constexpr uint8_t PropRef      = 0x51; // reference into PropertyDef table
    constexpr uint8_t RelConstr    = 0x70;
    constexpr uint8_t ConstrClass  = 0x71;
    constexpr uint8_t EndSchema    = 0x1F;
    constexpr uint8_t EndClass     = 0x4F;
    }

static constexpr uint32_t MAGIC = 0x43534348; // "CSCH"
static constexpr uint8_t  FORMAT_VERSION = 1;

// Safely narrow int64_t SQLite row ID to uint32_t for the binary format.
// EC metadata IDs are small sequential values; overflow indicates data corruption.
static uint32_t SafeU32Id(int64_t val)
    {
    if (val >= 0 && val <= (int64_t)UINT32_MAX)
        return (uint32_t)val;
    ECDbLogger::Get().warningv("RuntimeSchemaWriter::SafeU32Id: EC metadata row ID %" PRIi64 " exceeds uint32_t range, using sentinel", val);
    return UINT32_MAX;
    }

// Helper: prepare a statement and return error on failure.
static DbResult PrepareStmt(Statement& stmt, DbCR db, Utf8CP sql)
    {
    auto rc = stmt.Prepare(db, sql);
    if (rc != BE_SQLITE_OK)
        ECDbLogger::Get().errorv("RuntimeSchemaWriter::PrepareStmt: failed to prepare statement (rc=%d): %.120s", (int)rc, sql);
    return rc;
    }

//---------------------------------------------------------------------------------------
// String interning
//---------------------------------------------------------------------------------------
uint32_t RuntimeSchemaWriter::Intern(Utf8CP str)
    {
    str = Safe(str);
    std::string key(str);
    auto it = m_stringIndex.find(key);
    if (it != m_stringIndex.end())
        return it->second;
    uint32_t idx = (uint32_t)m_stringTable.size();
    auto result = m_stringIndex.emplace(std::move(key), idx);
    m_stringTable.push_back(result.first->first.c_str()); // pointer into map key (stable)
    return idx;
    }

//---------------------------------------------------------------------------------------
// Bulk-load IDs of all properties that have CoreCustomAttributes:HiddenProperty CA.
// The HiddenProperty CA is a simple presence marker - if it exists on a property, that
// property is hidden. The CA has an optional `Show` boolean (default false), but in
// practice only the presence/absence matters. We check the Instance XML only if the CA
// explicitly sets Show=True (which would mean "not hidden after all").
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::CollectHiddenPropertyIds(DbCR db)
    {
    m_hiddenPropertyIds.clear();

    Statement stmt;
    auto rc = PrepareStmt(stmt, db,
        "SELECT ca.ContainerId, ca.Instance "
        "FROM ec_CustomAttribute ca "
        "JOIN ec_Class cac ON ca.ClassId=cac.Id "
        "JOIN ec_Schema cas ON cac.SchemaId=cas.Id "
        "WHERE ca.ContainerType & 992 <> 0 "
        "AND cas.Name='CoreCustomAttributes' AND cac.Name='HiddenProperty'");
    if (rc != BE_SQLITE_OK)
        return rc;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        int64_t propertyId = stmt.GetValueInt64(0);
        Utf8CP instance = Safe(stmt.GetValueText(1));
        // Check for explicit Show=True override (case-insensitive).
        // The XML looks like: <HiddenProperty ...><Show>True</Show></HiddenProperty>
        // If Show is absent or not "true", the property is hidden.
        // We use a simple substring search on the lowercased XML instance rather than a
        // full XML parser. This is safe because HiddenProperty has exactly one optional
        // boolean property (Show) - there's no other element that could contain ">true</.".
        // If the CA schema ever adds more boolean properties, this would need revisiting.
        if (*instance != '\0')
            {
            std::string lower(instance);
            for (char& c : lower)
                if (c >= 'A' && c <= 'Z') c += 32;
            if (lower.find(">true</") != std::string::npos)
                continue; // Show=True/true/TRUE means NOT hidden
            }
        m_hiddenPropertyIds.insert(propertyId);
        }
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Bulk-load IDs of all classes that have the ECDbMap:QueryView custom attribute.
// These entity classes are ECViews - they get emitted as Class records with type=5 (View).
// records in the binary blob. Only presence matters; the ECSQL query property is
// intentionally omitted from the runtime blob.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::CollectQueryViewClassIds(DbCR db)
    {
    m_queryViewClassIds.clear();

    Statement stmt;
    auto rc = PrepareStmt(stmt, db,
        "SELECT ca.ContainerId "
        "FROM ec_CustomAttribute ca "
        "JOIN ec_Class cac ON ca.ClassId=cac.Id "
        "JOIN ec_Schema cas ON cac.SchemaId=cas.Id "
        "WHERE ca.ContainerType & 30 <> 0 "
        "AND cas.Name='ECDbMap' AND cac.Name='QueryView'");
    if (rc != BE_SQLITE_OK)
        return rc;

    while (stmt.Step() == BE_SQLITE_ROW)
        m_queryViewClassIds.insert(stmt.GetValueInt64(0));
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Pre-collect IDs of schemas in the exclusion list. This avoids per-row name checks
// and enables SQL-level filtering in the property query.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::CollectExcludedSchemaIds(DbCR db)
    {
    m_excludedSchemaIds.clear();
    Statement stmt;
    auto rc = PrepareStmt(stmt, db, "SELECT Id, Name FROM ec_Schema");
    if (rc != BE_SQLITE_OK)
        return rc;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        if (IsExcludedSchema(Safe(stmt.GetValueText(1))))
            m_excludedSchemaIds.insert(stmt.GetValueInt64(0));
        }
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Property definition dedup - builds a table of unique PropertyDef records and per-class
// PropertyRef lists. Called before writing so the PropertyDef table can precede schema records.
//---------------------------------------------------------------------------------------
uint32_t RuntimeSchemaWriter::AddPropertyDef(PropertyDefRecord const& def)
    {
    auto it = m_propDefIndex.find(def);
    if (it != m_propDefIndex.end())
        return it->second;
    uint32_t idx = (uint32_t)m_propDefs.size();
    m_propDefs.push_back(def);
    m_propDefIndex[def] = idx;
    return idx;
    }

DbResult RuntimeSchemaWriter::CollectPropertyDedup(DbCR db)
    {
    m_propDefs.clear();
    m_propDefIndex.clear();
    m_classPropRefs.clear();

    // Single query for all properties with schema names for cross-schema references.
    // Excluded schemas are filtered at the SQL level via NOT IN for efficiency.
    // Ordered by ClassId,Ordinal so m_classPropRefs preserves per-class order.
    Utf8String sql =
        "SELECT c.Id, "
        "p.Id, p.Name, p.DisplayLabel, p.Description, p.Kind, p.PrimitiveType, p.ExtendedTypeName, "
        "es.Name, e.Name, ss.Name, sc.Name, ks.Name, k.Name, pcs.Name, pc.Name, "
        "p.ArrayMinOccurs, p.ArrayMaxOccurs, "
        "nrs.Name, nrc.Name, p.NavigationDirection, "
        "p.IsReadonly, p.Priority "
        "FROM ec_Property p "
        "JOIN ec_Class c ON p.ClassId=c.Id "
        "JOIN ec_Schema s ON c.SchemaId=s.Id "
        "LEFT JOIN ec_Enumeration e ON p.EnumerationId=e.Id "
        "LEFT JOIN ec_Schema es ON e.SchemaId=es.Id "
        "LEFT JOIN ec_Class sc ON p.StructClassId=sc.Id "
        "LEFT JOIN ec_Schema ss ON sc.SchemaId=ss.Id "
        "LEFT JOIN ec_KindOfQuantity k ON p.KindOfQuantityId=k.Id "
        "LEFT JOIN ec_Schema ks ON k.SchemaId=ks.Id "
        "LEFT JOIN ec_PropertyCategory pc ON p.CategoryId=pc.Id "
        "LEFT JOIN ec_Schema pcs ON pc.SchemaId=pcs.Id "
        "LEFT JOIN ec_Class nrc ON p.NavigationRelationshipClassId=nrc.Id "
        "LEFT JOIN ec_Schema nrs ON nrc.SchemaId=nrs.Id ";
    if (!m_excludedSchemaIds.empty())
        {
        sql.append("WHERE s.Id NOT IN (");
        bool first = true;
        for (auto id : m_excludedSchemaIds)
            {
            if (!first) sql.append(",");
            sql.append(Utf8PrintfString("%" PRIi64, id));
            first = false;
            }
        sql.append(") ");
        }
    sql.append("ORDER BY c.Id, p.Ordinal");

    Statement stmt;
    auto rc = PrepareStmt(stmt, db, sql.c_str());
    if (rc != BE_SQLITE_OK)
        return rc;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        int64_t classId = stmt.GetValueInt64(0);
        int64_t propertyId = stmt.GetValueInt64(1);

        PropertyDefRecord def;
        def.nameSid       = Intern(Safe(stmt.GetValueText(2)));
        def.descriptionSid= Intern(Safe(stmt.GetValueText(4)));
        def.kind          = (uint8_t)stmt.GetValueInt(5);
        def.primitiveType = (uint16_t)stmt.GetValueInt(6);
        def.extTypeSid    = Intern(Safe(stmt.GetValueText(7)));
        def.enumSchemaSid = Intern(Safe(stmt.GetValueText(8)));
        def.enumNameSid   = Intern(Safe(stmt.GetValueText(9)));
        def.structSchemaSid = Intern(Safe(stmt.GetValueText(10)));
        def.structClassSid  = Intern(Safe(stmt.GetValueText(11)));
        def.koqSchemaSid  = Intern(Safe(stmt.GetValueText(12)));
        def.koqNameSid    = Intern(Safe(stmt.GetValueText(13)));
        def.catSchemaSid  = Intern(Safe(stmt.GetValueText(14)));
        def.catNameSid    = Intern(Safe(stmt.GetValueText(15)));
        def.arrayMinOccurs= (uint32_t)stmt.GetValueInt(16);
        def.arrayMaxOccurs= (uint32_t)stmt.GetValueInt(17);
        def.navRelSchemaSid = Intern(Safe(stmt.GetValueText(18)));
        def.navRelClassSid  = Intern(Safe(stmt.GetValueText(19)));
        def.navDirection  = (uint8_t)stmt.GetValueInt(20);
        def.isReadonly    = stmt.GetValueInt(21) != 0 ? (uint8_t)1 : (uint8_t)0;
        def.isHidden      = m_hiddenPropertyIds.count(propertyId) ? (uint8_t)1 : (uint8_t)0;

        uint32_t defIdx = AddPropertyDef(def);

        PropertyRefRecord ref;
        ref.defIdx   = defIdx;
        ref.labelSid = Intern(Safe(stmt.GetValueText(3)));
        ref.priority = stmt.GetValueInt(22);
        ref.ecInstanceId = SafeU32Id(propertyId);

        m_classPropRefs[classId].push_back(ref);
        }
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Main orchestration - reads ec_ tables with raw SQLite, writes binary records
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::WriteAllSchemas(DbCR db)
    {
    m_output.clear();
    m_output.reserve(2 * 1024 * 1024);
    m_stringTable.clear();
    m_stringIndex.clear();
    Intern(""); // index 0 = empty string

    // ---- Pre-pass: identify excluded schemas, bulk-load hidden property IDs and QueryView class IDs, then collect and dedup all properties ----
    DbResult rc;
    if ((rc = CollectExcludedSchemaIds(db)) != BE_SQLITE_OK) return rc;
    if ((rc = CollectHiddenPropertyIds(db)) != BE_SQLITE_OK) return rc;
    if ((rc = CollectQueryViewClassIds(db)) != BE_SQLITE_OK) return rc;
    if ((rc = CollectPropertyDedup(db)) != BE_SQLITE_OK) return rc;

    // Header: magic(4) + version(1) + stringTableOffset placeholder(4)
    PutU32(MAGIC);
    PutU8(FORMAT_VERSION);
    PutU32(0); // placeholder for string table offset, patched in at end

    // ---- Write PropertyDef table ----
    PutU8(Tag::PropertyDefTable);
    PutU32((uint32_t)m_propDefs.size());
    for (auto const& def : m_propDefs)
        {
        PutU32(def.nameSid);
        PutU8(def.kind);
        PutU16(def.primitiveType);
        PutU32(def.extTypeSid);
        PutU32(def.enumSchemaSid);
        PutU32(def.enumNameSid);
        PutU32(def.structSchemaSid);
        PutU32(def.structClassSid);
        PutU32(def.koqSchemaSid);
        PutU32(def.koqNameSid);
        PutU32(def.catSchemaSid);
        PutU32(def.catNameSid);
        PutU32(def.arrayMinOccurs);
        PutU32(def.arrayMaxOccurs);
        PutU32(def.navRelSchemaSid);
        PutU32(def.navRelClassSid);
        PutU8(def.navDirection);
        PutU8(def.isReadonly);
        PutU8(def.isHidden);
        PutU32(def.descriptionSid);
        }

    // ---- Prepare statements ----
    Statement schemaStmt;
    if ((rc = PrepareStmt(schemaStmt, db, "SELECT Id,Name,DisplayLabel,Description,Alias,VersionDigit1,VersionDigit2,VersionDigit3 FROM ec_Schema ORDER BY Id")) != BE_SQLITE_OK)
        return rc;

    Statement schemaRefStmt;
    if ((rc = PrepareStmt(schemaRefStmt, db,
        "SELECT rs.Name FROM ec_SchemaReference sr "
        "JOIN ec_Schema rs ON sr.ReferencedSchemaId=rs.Id "
        "WHERE sr.SchemaId=? ORDER BY sr.Id")) != BE_SQLITE_OK)
        return rc;

    Statement enumStmt;
    if ((rc = PrepareStmt(enumStmt, db, "SELECT Id,Name,DisplayLabel,Description,UnderlyingPrimitiveType,IsStrict,EnumValues FROM ec_Enumeration WHERE SchemaId=? ORDER BY Id")) != BE_SQLITE_OK)
        return rc;

    Statement koqStmt;
    if ((rc = PrepareStmt(koqStmt, db, "SELECT Id,Name,DisplayLabel,Description,PersistenceUnit,RelativeError,PresentationUnits FROM ec_KindOfQuantity WHERE SchemaId=? ORDER BY Id")) != BE_SQLITE_OK)
        return rc;

    Statement catStmt;
    if ((rc = PrepareStmt(catStmt, db, "SELECT Id,Name,DisplayLabel,Description,Priority FROM ec_PropertyCategory WHERE SchemaId=? ORDER BY Id")) != BE_SQLITE_OK)
        return rc;

    Statement classStmt;
    if ((rc = PrepareStmt(classStmt, db, "SELECT Id,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection FROM ec_Class WHERE SchemaId=? ORDER BY Id")) != BE_SQLITE_OK)
        return rc;

    // Detect mixins: Entity classes with CoreCustomAttributes:IsMixin CA
    Statement isMixinStmt;
    if ((rc = PrepareStmt(isMixinStmt, db,
        "SELECT 1 FROM ec_CustomAttribute ca "
        "JOIN ec_Class cac ON ca.ClassId=cac.Id "
        "JOIN ec_Schema cas ON cac.SchemaId=cas.Id "
        "WHERE ca.ContainerId=? AND ca.ContainerType & 30 <> 0 "
        "AND cas.Name='CoreCustomAttributes' AND cac.Name='IsMixin' LIMIT 1")) != BE_SQLITE_OK)
        return rc;

    Statement baseStmt;
    if ((rc = PrepareStmt(baseStmt, db,
        "SELECT s.Name, bc.Name, b.Ordinal "
        "FROM ec_ClassHasBaseClasses b "
        "JOIN ec_Class bc ON b.BaseClassId=bc.Id "
        "JOIN ec_Schema s ON bc.SchemaId=s.Id "
        "WHERE b.ClassId=? ORDER BY b.Ordinal")) != BE_SQLITE_OK)
        return rc;

    Statement constrStmt;
    if ((rc = PrepareStmt(constrStmt, db,
        "SELECT rc.Id, rc.RelationshipEnd, rc.MultiplicityLowerLimit, rc.MultiplicityUpperLimit, "
        "rc.IsPolymorphic, rc.RoleLabel, acs.Name, acc.Name "
        "FROM ec_RelationshipConstraint rc "
        "LEFT JOIN ec_Class acc ON rc.AbstractConstraintClassId=acc.Id "
        "LEFT JOIN ec_Schema acs ON acc.SchemaId=acs.Id "
        "WHERE rc.RelationshipClassId=? ORDER BY rc.RelationshipEnd")) != BE_SQLITE_OK)
        return rc;

    Statement constrClassStmt;
    if ((rc = PrepareStmt(constrClassStmt, db,
        "SELECT s.Name, c.Name FROM ec_RelationshipConstraintClass rcc "
        "JOIN ec_Class c ON rcc.ClassId=c.Id "
        "JOIN ec_Schema s ON c.SchemaId=s.Id "
        "WHERE rcc.ConstraintId=? ORDER BY rcc.Id")) != BE_SQLITE_OK)
        return rc;

    // ---- Iterate schemas ----
    while (schemaStmt.Step() == BE_SQLITE_ROW)
        {
        int64_t schemaId = schemaStmt.GetValueInt64(0);
        if (m_excludedSchemaIds.count(schemaId))
            continue;

        Utf8CP name = Safe(schemaStmt.GetValueText(1));

        Utf8CP displayLabel = Safe(schemaStmt.GetValueText(2));
        Utf8CP description = Safe(schemaStmt.GetValueText(3));
        Utf8CP alias = Safe(schemaStmt.GetValueText(4));
        int v1 = schemaStmt.GetValueInt(5);
        int v2 = schemaStmt.GetValueInt(6);
        int v3 = schemaStmt.GetValueInt(7);

        PutU8(Tag::Schema);
        PutSRef(name);
        PutU16((uint16_t)v1);
        PutU16((uint16_t)v2);
        PutU16((uint16_t)v3);
        PutSRef(alias);
        PutSRef(displayLabel);
        PutSRef(description);
        PutU32(SafeU32Id(schemaId));

        // Schema references
        schemaRefStmt.Reset();
        schemaRefStmt.ClearBindings();
        schemaRefStmt.BindInt64(1, schemaId);
        while (schemaRefStmt.Step() == BE_SQLITE_ROW)
            {
            PutU8(Tag::SchemaRef);
            PutSRef(Safe(schemaRefStmt.GetValueText(0)));
            }

        // Enumerations
        enumStmt.Reset(); enumStmt.ClearBindings();
        enumStmt.BindInt64(1, schemaId);
        while (enumStmt.Step() == BE_SQLITE_ROW)
            {
            PutU8(Tag::Enum);
            PutSRef(Safe(enumStmt.GetValueText(1))); // name
            PutU8((uint8_t)enumStmt.GetValueInt(4));  // primitiveType
            PutU8(enumStmt.GetValueInt(5) != 0 ? 1 : 0); // isStrict
            PutSRef(Safe(enumStmt.GetValueText(2))); // label
            PutSRef(Safe(enumStmt.GetValueText(3))); // description
            PutSRef(Safe(enumStmt.GetValueText(6))); // enumValues JSON
            PutU32(SafeU32Id(enumStmt.GetValueInt64(0))); // ecInstanceId
            }

        // KindOfQuantity
        koqStmt.Reset(); koqStmt.ClearBindings();
        koqStmt.BindInt64(1, schemaId);
        while (koqStmt.Step() == BE_SQLITE_ROW)
            {
            PutU8(Tag::KoQ);
            PutSRef(Safe(koqStmt.GetValueText(1))); // name
            PutSRef(Safe(koqStmt.GetValueText(2))); // label
            PutSRef(Safe(koqStmt.GetValueText(3))); // description
            PutSRef(Safe(koqStmt.GetValueText(4))); // persistenceUnit
            PutF64(koqStmt.GetValueDouble(5));        // relativeError
            PutSRef(Safe(koqStmt.GetValueText(6))); // presentationUnits
            PutU32(SafeU32Id(koqStmt.GetValueInt64(0))); // ecInstanceId
            }

        // PropertyCategory
        catStmt.Reset(); catStmt.ClearBindings();
        catStmt.BindInt64(1, schemaId);
        while (catStmt.Step() == BE_SQLITE_ROW)
            {
            PutU8(Tag::PropCat);
            PutSRef(Safe(catStmt.GetValueText(1))); // name
            PutSRef(Safe(catStmt.GetValueText(2))); // label
            PutSRef(Safe(catStmt.GetValueText(3))); // description
            PutI32(catStmt.GetValueInt(4));            // priority
            PutU32(SafeU32Id(catStmt.GetValueInt64(0))); // ecInstanceId
            }

        // Classes
        classStmt.Reset(); classStmt.ClearBindings();
        classStmt.BindInt64(1, schemaId);
        while (classStmt.Step() == BE_SQLITE_ROW)
            {
            int64_t classId = classStmt.GetValueInt64(0);
            int classType = classStmt.GetValueInt(4);

            // Entity classes with ECDbMap:QueryView CA are views (type 5)
            if (m_queryViewClassIds.count(classId))
                classType = 5; // View

            // Entity classes with CoreCustomAttributes:IsMixin are mixins (type 4)
            if (classType == 0) // Entity
                {
                isMixinStmt.Reset(); isMixinStmt.ClearBindings();
                isMixinStmt.BindInt64(1, classId);
                if (isMixinStmt.Step() == BE_SQLITE_ROW)
                    classType = 4; // Mixin
                }

            PutU8(Tag::Class);
            PutSRef(Safe(classStmt.GetValueText(1))); // name
            PutU8((uint8_t)classType);                  // type
            PutU8((uint8_t)classStmt.GetValueInt(5));  // modifier
            PutSRef(Safe(classStmt.GetValueText(2))); // label
            PutSRef(Safe(classStmt.GetValueText(3))); // description
            if (classType == 1) // Relationship
                {
                PutU8((uint8_t)classStmt.GetValueInt(6)); // strength
                PutU8((uint8_t)classStmt.GetValueInt(7)); // strengthDirection
                }
            PutU32(SafeU32Id(classId)); // ecInstanceId

            // Base classes
            baseStmt.Reset(); baseStmt.ClearBindings();
            baseStmt.BindInt64(1, classId);
            while (baseStmt.Step() == BE_SQLITE_ROW)
                {
                PutU8(Tag::BaseClass);
                PutSRef(Safe(baseStmt.GetValueText(0))); // schema name
                PutSRef(Safe(baseStmt.GetValueText(1))); // class name
                PutU8((uint8_t)baseStmt.GetValueInt(2));  // ordinal
                }

            // Properties (emit PropRef referencing the pre-built PropertyDef table)
            auto propIt = m_classPropRefs.find(classId);
            if (propIt != m_classPropRefs.end())
                for (auto const& ref : propIt->second)
                    {
                    PutU8(Tag::PropRef);
                    PutU32(ref.defIdx);
                    PutU32(ref.labelSid);
                    PutI32(ref.priority);
                    PutU32(ref.ecInstanceId);
                    }

            // Relationship constraints
            if (classType == 1)
                {
                constrStmt.Reset(); constrStmt.ClearBindings();
                constrStmt.BindInt64(1, classId);
                while (constrStmt.Step() == BE_SQLITE_ROW)
                    {
                    int64_t constraintId = constrStmt.GetValueInt64(0);

                    PutU8(Tag::RelConstr);
                    PutU8((uint8_t)constrStmt.GetValueInt(1));  // relEnd
                    PutU32((uint32_t)constrStmt.GetValueInt(2)); // multLower
                    PutU32((uint32_t)constrStmt.GetValueInt(3)); // multUpper
                    PutU8(constrStmt.GetValueInt(4) != 0 ? 1 : 0); // isPolymorphic
                    PutSRef(Safe(constrStmt.GetValueText(5)));  // roleLabel
                    PutSRef(Safe(constrStmt.GetValueText(6)));  // abstractSchemaName
                    PutSRef(Safe(constrStmt.GetValueText(7)));  // abstractClassName

                    // Constraint classes
                    constrClassStmt.Reset(); constrClassStmt.ClearBindings();
                    constrClassStmt.BindInt64(1, constraintId);
                    while (constrClassStmt.Step() == BE_SQLITE_ROW)
                        {
                        PutU8(Tag::ConstrClass);
                        PutSRef(Safe(constrClassStmt.GetValueText(0))); // schema name
                        PutSRef(Safe(constrClassStmt.GetValueText(1))); // class name
                        }

                    }
                }

            PutU8(Tag::EndClass);
            }

        PutU8(Tag::EndSchema);
        }

    // ---- Write string table and patch offset ----
    uint32_t stOffset = (uint32_t)m_output.size();
    PutU32((uint32_t)m_stringTable.size());
    for (auto s : m_stringTable)
        {
        uint32_t len = (uint32_t)strlen(s);
        PutU32(len);
        if (len)
            m_output.insert(m_output.end(), (Byte const*)s, (Byte const*)s + len);
        }

    // Patch string table offset at position 5 (after magic(4) + version(1))
    m_output[5] = (Byte)(stOffset & 0xFF);
    m_output[6] = (Byte)((stOffset >> 8) & 0xFF);
    m_output[7] = (Byte)((stOffset >> 16) & 0xFF);
    m_output[8] = (Byte)((stOffset >> 24) & 0xFF);
    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
