/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "RuntimeSchemaWriter.h"
#include <BeSQLite/BeSQLite.h>

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
    constexpr uint8_t CA           = 0x60;
    constexpr uint8_t RelConstr    = 0x70;
    constexpr uint8_t ConstrClass  = 0x71;
    constexpr uint8_t EndSchema    = 0x1F;
    constexpr uint8_t EndClass     = 0x4F;
    }

static constexpr uint32_t MAGIC = 0x43534348; // "CSCH"
static constexpr uint8_t  FORMAT_VERSION = 1;

//---------------------------------------------------------------------------------------
// String interning
//---------------------------------------------------------------------------------------
uint32_t RuntimeSchemaWriter::Intern(Utf8CP str)
    {
    str = Safe(str);
    auto it = m_stringIndex.find(std::string(str));
    if (it != m_stringIndex.end())
        return it->second;
    uint32_t idx = (uint32_t)m_stringTable.size();
    m_stringTable.push_back(Utf8String(str));
    m_stringIndex[std::string(str)] = idx;
    return idx;
    }

//---------------------------------------------------------------------------------------
// Property definition dedup - builds a table of unique PropertyDef records and per-class
// PropertyRef lists. Called before writing so the PropertyDef table can precede schema records.
//---------------------------------------------------------------------------------------
std::string RuntimeSchemaWriter::PropertyDefSignature(PropertyDefRecord const& def) const
    {
    char buf[256];
    snprintf(buf, sizeof(buf), "%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u",
        def.nameSid, def.descriptionSid, def.kind, def.primitiveType, def.extTypeSid,
        def.enumNameSid, def.koqNameSid, def.structSchemaSid, def.structClassSid,
        def.navRelSchemaSid, def.navRelClassSid, (uint32_t)def.navDirection,
        def.catNameSid, (uint32_t)def.isReadonly, def.arrayMinOccurs, def.arrayMaxOccurs);
    return std::string(buf);
    }

uint32_t RuntimeSchemaWriter::AddPropertyDef(PropertyDefRecord const& def)
    {
    auto sig = PropertyDefSignature(def);
    auto it = m_propDefIndex.find(sig);
    if (it != m_propDefIndex.end())
        return it->second;
    uint32_t idx = (uint32_t)m_propDefs.size();
    m_propDefs.push_back(def);
    m_propDefIndex[sig] = idx;
    return idx;
    }

void RuntimeSchemaWriter::CollectPropertyDedup(DbCR db)
    {
    m_propDefs.clear();
    m_propDefIndex.clear();
    m_classPropRefs.clear();

    // Single query for all properties, joined to schema for exclusion check.
    // Ordered by ClassId,Ordinal so m_classPropRefs preserves per-class order.
    Statement stmt;
    stmt.Prepare(db,
        "SELECT c.Id, s.Name, "
        "p.Name, p.DisplayLabel, p.Description, p.Kind, p.PrimitiveType, p.ExtendedTypeName, "
        "e.Name, ss.Name, sc.Name, k.Name, pc.Name, "
        "p.ArrayMinOccurs, p.ArrayMaxOccurs, "
        "nrs.Name, nrc.Name, p.NavigationDirection, "
        "p.IsReadonly, p.Priority "
        "FROM ec_Property p "
        "JOIN ec_Class c ON p.ClassId=c.Id "
        "JOIN ec_Schema s ON c.SchemaId=s.Id "
        "LEFT JOIN ec_Enumeration e ON p.EnumerationId=e.Id "
        "LEFT JOIN ec_Class sc ON p.StructClassId=sc.Id "
        "LEFT JOIN ec_Schema ss ON sc.SchemaId=ss.Id "
        "LEFT JOIN ec_KindOfQuantity k ON p.KindOfQuantityId=k.Id "
        "LEFT JOIN ec_PropertyCategory pc ON p.CategoryId=pc.Id "
        "LEFT JOIN ec_Class nrc ON p.NavigationRelationshipClassId=nrc.Id "
        "LEFT JOIN ec_Schema nrs ON nrc.SchemaId=nrs.Id "
        "ORDER BY c.Id, p.Ordinal");

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        Utf8CP schemaName = Safe(stmt.GetValueText(1));
        if (IsExcludedSchema(schemaName))
            continue;

        int64_t classId = stmt.GetValueInt64(0);

        PropertyDefRecord def;
        def.nameSid       = Intern(Safe(stmt.GetValueText(2)));
        def.descriptionSid= Intern(Safe(stmt.GetValueText(4)));
        def.kind          = (uint8_t)stmt.GetValueInt(5);
        def.primitiveType = (uint16_t)stmt.GetValueInt(6);
        def.extTypeSid    = Intern(Safe(stmt.GetValueText(7)));
        def.enumNameSid   = Intern(Safe(stmt.GetValueText(8)));
        def.structSchemaSid = Intern(Safe(stmt.GetValueText(9)));
        def.structClassSid  = Intern(Safe(stmt.GetValueText(10)));
        def.koqNameSid    = Intern(Safe(stmt.GetValueText(11)));
        def.catNameSid    = Intern(Safe(stmt.GetValueText(12)));
        def.arrayMinOccurs= (uint32_t)stmt.GetValueInt(13);
        def.arrayMaxOccurs= (uint32_t)stmt.GetValueInt(14);
        def.navRelSchemaSid = Intern(Safe(stmt.GetValueText(15)));
        def.navRelClassSid  = Intern(Safe(stmt.GetValueText(16)));
        def.navDirection  = (uint8_t)stmt.GetValueInt(17);
        def.isReadonly    = stmt.GetValueInt(18) != 0 ? (uint8_t)1 : (uint8_t)0;

        uint32_t defIdx = AddPropertyDef(def);

        PropertyRefRecord ref;
        ref.defIdx   = defIdx;
        ref.labelSid = Intern(Safe(stmt.GetValueText(3)));
        ref.priority = stmt.GetValueInt(19);

        m_classPropRefs[classId].push_back(ref);
        }
    }

//---------------------------------------------------------------------------------------
// Main orchestration - reads ec_ tables with raw SQLite, writes binary records
//---------------------------------------------------------------------------------------
void RuntimeSchemaWriter::WriteAllSchemas(DbCR db, bool includeCustomAttributes)
    {
    m_output.clear();
    m_output.reserve(2 * 1024 * 1024);
    m_stringTable.clear();
    m_stringIndex.clear();
    Intern(""); // index 0 = empty string

    // ---- Pre-pass: collect and deduplicate all property definitions ----
    CollectPropertyDedup(db);

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
        PutU32(def.enumNameSid);
        PutU32(def.structSchemaSid);
        PutU32(def.structClassSid);
        PutU32(def.koqNameSid);
        PutU32(def.catNameSid);
        PutU32(def.arrayMinOccurs);
        PutU32(def.arrayMaxOccurs);
        PutU32(def.navRelSchemaSid);
        PutU32(def.navRelClassSid);
        PutU8(def.navDirection);
        PutU8(def.isReadonly);
        PutU32(def.descriptionSid);
        }

    // ---- Prepare statements ----
    Statement schemaStmt;
    schemaStmt.Prepare(db, "SELECT Id,Name,DisplayLabel,Description,Alias,VersionDigit1,VersionDigit2,VersionDigit3 FROM ec_Schema ORDER BY Id");

    Statement schemaRefStmt;
    schemaRefStmt.Prepare(db,
        "SELECT rs.Name FROM ec_SchemaReference sr "
        "JOIN ec_Schema rs ON sr.ReferencedSchemaId=rs.Id "
        "WHERE sr.SchemaId=? ORDER BY sr.Id");

    Statement enumStmt;
    enumStmt.Prepare(db, "SELECT Name,DisplayLabel,Description,UnderlyingPrimitiveType,IsStrict,EnumValues FROM ec_Enumeration WHERE SchemaId=? ORDER BY Id");

    Statement koqStmt;
    koqStmt.Prepare(db, "SELECT Name,DisplayLabel,Description,PersistenceUnit,RelativeError,PresentationUnits FROM ec_KindOfQuantity WHERE SchemaId=? ORDER BY Id");

    Statement catStmt;
    catStmt.Prepare(db, "SELECT Name,DisplayLabel,Description,Priority FROM ec_PropertyCategory WHERE SchemaId=? ORDER BY Id");

    Statement classStmt;
    classStmt.Prepare(db, "SELECT Id,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection FROM ec_Class WHERE SchemaId=? ORDER BY Id");

    // Detect mixins: Entity classes with CoreCustomAttributes:IsMixin CA
    Statement isMixinStmt;
    isMixinStmt.Prepare(db,
        "SELECT 1 FROM ec_CustomAttribute ca "
        "JOIN ec_Class cac ON ca.ClassId=cac.Id "
        "JOIN ec_Schema cas ON cac.SchemaId=cas.Id "
        "WHERE ca.ContainerId=? AND ca.ContainerType & 30 <> 0 "
        "AND cas.Name='CoreCustomAttributes' AND cac.Name='IsMixin' LIMIT 1");

    Statement baseStmt;
    baseStmt.Prepare(db,
        "SELECT s.Name, bc.Name, b.Ordinal "
        "FROM ec_ClassHasBaseClasses b "
        "JOIN ec_Class bc ON b.BaseClassId=bc.Id "
        "JOIN ec_Schema s ON bc.SchemaId=s.Id "
        "WHERE b.ClassId=? ORDER BY b.Ordinal");

    Statement constrStmt;
    constrStmt.Prepare(db,
        "SELECT rc.Id, rc.RelationshipEnd, rc.MultiplicityLowerLimit, rc.MultiplicityUpperLimit, "
        "rc.IsPolymorphic, rc.RoleLabel, acs.Name, acc.Name "
        "FROM ec_RelationshipConstraint rc "
        "LEFT JOIN ec_Class acc ON rc.AbstractConstraintClassId=acc.Id "
        "LEFT JOIN ec_Schema acs ON acc.SchemaId=acs.Id "
        "WHERE rc.RelationshipClassId=? ORDER BY rc.RelationshipEnd");

    Statement constrClassStmt;
    constrClassStmt.Prepare(db,
        "SELECT s.Name, c.Name FROM ec_RelationshipConstraintClass rcc "
        "JOIN ec_Class c ON rcc.ClassId=c.Id "
        "JOIN ec_Schema s ON c.SchemaId=s.Id "
        "WHERE rcc.ConstraintId=? ORDER BY rcc.Id");

    // ---- Bulk-load custom attributes if requested ----
    // Note: property CAs are not emitted inline (properties use deduped PropRef).
    // Property CAs are loaded lazily via ECSQL in RuntimeSchemaContext.
    struct CaEntry { Utf8String schemaName; Utf8String className; int containerType; Utf8String instance; };
    std::unordered_map<int64_t, bvector<CaEntry>> schemaCAs, classCAs, constraintCAs;

    if (includeCustomAttributes)
        {
        Statement caStmt;
        caStmt.Prepare(db,
            "SELECT ca.ContainerId, ca.ContainerType, cas.Name, cac.Name, ca.Instance "
            "FROM ec_CustomAttribute ca "
            "JOIN ec_Class cac ON ca.ClassId=cac.Id "
            "JOIN ec_Schema cas ON cac.SchemaId=cas.Id "
            "ORDER BY ca.ContainerId, ca.Ordinal");
        while (caStmt.Step() == BE_SQLITE_ROW)
            {
            int64_t containerId = caStmt.GetValueInt64(0);
            int ct = caStmt.GetValueInt(1);
            CaEntry entry{Safe(caStmt.GetValueText(2)), Safe(caStmt.GetValueText(3)), ct, Safe(caStmt.GetValueText(4))};
            if (ct & 1)         schemaCAs[containerId].push_back(std::move(entry));
            else if (ct & 30)   classCAs[containerId].push_back(std::move(entry));
            else if (ct & 3072) constraintCAs[containerId].push_back(std::move(entry));
            }
        }

    // ---- Iterate schemas ----
    while (schemaStmt.Step() == BE_SQLITE_ROW)
        {
        int64_t schemaId = schemaStmt.GetValueInt64(0);
        Utf8CP name = Safe(schemaStmt.GetValueText(1));

        if (IsExcludedSchema(name))
            continue;

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

        // Schema references
        schemaRefStmt.Reset();
        schemaRefStmt.ClearBindings();
        schemaRefStmt.BindInt64(1, schemaId);
        while (schemaRefStmt.Step() == BE_SQLITE_ROW)
            {
            PutU8(Tag::SchemaRef);
            PutSRef(Safe(schemaRefStmt.GetValueText(0)));
            }

        // CAs on schema
        if (includeCustomAttributes)
            {
            auto it = schemaCAs.find(schemaId);
            if (it != schemaCAs.end())
                for (auto const& ca : it->second)
                    {
                    PutU8(Tag::CA);
                    PutU16((uint16_t)ca.containerType);
                    PutSRef(ca.schemaName.c_str());
                    PutSRef(ca.className.c_str());
                    PutRaw(ca.instance.c_str(), (uint32_t)ca.instance.size());
                    }
            }

        // Enumerations
        enumStmt.Reset(); enumStmt.ClearBindings();
        enumStmt.BindInt64(1, schemaId);
        while (enumStmt.Step() == BE_SQLITE_ROW)
            {
            PutU8(Tag::Enum);
            PutSRef(Safe(enumStmt.GetValueText(0))); // name
            PutU8((uint8_t)enumStmt.GetValueInt(3));  // primitiveType
            PutU8(enumStmt.GetValueInt(4) != 0 ? 1 : 0); // isStrict
            PutSRef(Safe(enumStmt.GetValueText(1))); // label
            PutSRef(Safe(enumStmt.GetValueText(2))); // description
            PutSRef(Safe(enumStmt.GetValueText(5))); // enumValues JSON
            }

        // KindOfQuantity
        koqStmt.Reset(); koqStmt.ClearBindings();
        koqStmt.BindInt64(1, schemaId);
        while (koqStmt.Step() == BE_SQLITE_ROW)
            {
            PutU8(Tag::KoQ);
            PutSRef(Safe(koqStmt.GetValueText(0))); // name
            PutSRef(Safe(koqStmt.GetValueText(1))); // label
            PutSRef(Safe(koqStmt.GetValueText(2))); // description
            PutSRef(Safe(koqStmt.GetValueText(3))); // persistenceUnit
            PutF64(koqStmt.GetValueDouble(4));        // relativeError
            PutSRef(Safe(koqStmt.GetValueText(5))); // presentationUnits
            }

        // PropertyCategory
        catStmt.Reset(); catStmt.ClearBindings();
        catStmt.BindInt64(1, schemaId);
        while (catStmt.Step() == BE_SQLITE_ROW)
            {
            PutU8(Tag::PropCat);
            PutSRef(Safe(catStmt.GetValueText(0))); // name
            PutSRef(Safe(catStmt.GetValueText(1))); // label
            PutSRef(Safe(catStmt.GetValueText(2))); // description
            PutI32(catStmt.GetValueInt(3));            // priority
            }

        // Classes
        classStmt.Reset(); classStmt.ClearBindings();
        classStmt.BindInt64(1, schemaId);
        while (classStmt.Step() == BE_SQLITE_ROW)
            {
            int64_t classId = classStmt.GetValueInt64(0);
            int classType = classStmt.GetValueInt(4);

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
                    }

            // CAs on class
            if (includeCustomAttributes)
                {
                auto it = classCAs.find(classId);
                if (it != classCAs.end())
                    for (auto const& ca : it->second)
                        {
                        PutU8(Tag::CA);
                        PutU16((uint16_t)ca.containerType);
                        PutSRef(ca.schemaName.c_str());
                        PutSRef(ca.className.c_str());
                        PutRaw(ca.instance.c_str(), (uint32_t)ca.instance.size());
                        }
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

                    // CAs on constraints
                    if (includeCustomAttributes)
                        {
                        auto it = constraintCAs.find(constraintId);
                        if (it != constraintCAs.end())
                            for (auto const& ca : it->second)
                                {
                                PutU8(Tag::CA);
                                PutU16((uint16_t)ca.containerType);
                                PutSRef(ca.schemaName.c_str());
                                PutSRef(ca.className.c_str());
                                PutRaw(ca.instance.c_str(), (uint32_t)ca.instance.size());
                                }
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
    for (auto const& s : m_stringTable)
        {
        uint32_t len = (uint32_t)s.size();
        PutU32(len);
        if (len)
            m_output.insert(m_output.end(), (Byte const*)s.c_str(), (Byte const*)s.c_str() + len);
        }

    // Patch string table offset at position 5 (after magic(4) + version(1))
    m_output[5] = (Byte)(stOffset & 0xFF);
    m_output[6] = (Byte)((stOffset >> 8) & 0xFF);
    m_output[7] = (Byte)((stOffset >> 16) & 0xFF);
    m_output[8] = (Byte)((stOffset >> 24) & 0xFF);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
