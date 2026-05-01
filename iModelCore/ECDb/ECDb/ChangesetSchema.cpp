/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/ChangesetSchema.h>
#include <ECDb/ChangeIterator.h>
#include "ClassMap.h"
#include "DbSchema.h"
#include "PropertyMap.h"
#include "PropertyMapVisitor.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

namespace {

// Build (or reuse) the TableSegment for (classId, tableName) by walking the live ClassMap.
// Only includes columns that physically belong to @p tableName and aren't virtual.
// Records the source-row indices (positions of every column) using ECDb::GetColumns.
// NavigationRelECClassId-typed columns are added to seg.classIdRefCols (not seg.columns),
// tagged with the fullName of the referenced relationship class.
static void PopulateTableSegment(ChangesetSchema::TableSegment& seg, ClassMap const& classMap,
                                 Utf8StringCR tableName, ECDbCR ecdb, std::map<Utf8String, std::vector<Utf8String>>& tableColumnsCache)
    {
    seg.tableName = tableName;

    auto cacheIt = tableColumnsCache.find(tableName);
    if (cacheIt == tableColumnsCache.end())
        {
        bvector<Utf8String> cols;
        ecdb.GetColumns(cols, tableName.c_str());
        std::vector<Utf8String> v(cols.begin(), cols.end());
        cacheIt = tableColumnsCache.emplace(tableName, std::move(v)).first;
        }
    auto const& columnNames = cacheIt->second;
    seg.sourceColumnCount = (int)columnNames.size();

    auto indexOf = [&](Utf8StringCR name) -> int {
        for (size_t i = 0; i < columnNames.size(); ++i)
            if (columnNames[i].EqualsI(name))
                return (int)i;
        return -1;
        };

    // Resolve PK + ECClassId column indices via DbTable
    DbTable const* dbTable = ecdb.Schemas().Main().GetDbSchema().FindTable(tableName);

    if (dbTable != nullptr)
        {
        if (auto const* idCol = dbTable->FindFirst(DbColumn::Kind::ECInstanceId))
            seg.idColumnIndex = indexOf(idCol->GetName());
        if (auto const* classIdCol = dbTable->FindFirst(DbColumn::Kind::ECClassId))
            {
            if (classIdCol->GetPersistenceType() != PersistenceType::Virtual)
                seg.classIdColumnIndex = indexOf(classIdCol->GetName());
            }
        }

    SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData, true);
    classMap.GetPropertyMaps().AcceptVisitor(visitor);
    for (PropertyMap const* pm : visitor.Results())
        {
        auto const& sm = pm->GetAs<SingleColumnDataPropertyMap>();
        DbColumn const& col = sm.GetColumn();
        if (col.IsVirtual())
            continue;
        if (!col.GetTable().GetName().EqualsI(tableName))
            continue;

        if (pm->GetType() == PropertyMap::Type::NavigationRelECClassId)
            {
            // This column holds the ID of the relationship class used for a nav prop.
            // It must be captured as a classIdRefCol (value-remapped), not a data column.
            Utf8String relClassFullName;
            PropertyMap const* parentPm = pm->GetParent();
            if (parentPm != nullptr)
                {
                if (auto const* navPm = dynamic_cast<NavigationPropertyMap const*>(parentPm))
                    {
                    if (auto const* navProp = navPm->GetProperty().GetAsNavigationProperty())
                        if (auto const* relCls = navProp->GetRelationshipClass())
                            relClassFullName = relCls->GetFullName();
                    }
                }
            ChangesetSchema::ClassIdRefColumn refCol;
            refCol.columnName = col.GetName();
            refCol.tableName = tableName;
            refCol.sourceColumnIndex = indexOf(col.GetName());
            refCol.referencedClassFullName = relClassFullName;
            // Store the relationship class ID at extraction time so Compute can detect remaps.
            refCol.referencedClassId = pm->GetAs<NavigationPropertyMap::RelECClassIdPropertyMap>().GetDefaultClassId();
            seg.classIdRefCols[col.GetName()] = std::move(refCol);
            continue;
            }

        ChangesetSchema::PropertyColumnMap m;
        m.accessString = sm.GetAccessString();
        m.tableName = tableName;
        m.columnName = col.GetName();
        m.sourceColumnIndex = indexOf(col.GetName());
        seg.columns[m.accessString] = m;
        }
    }

} // anonymous

//=======================================================================================
// ChangesetSchema::ExtractFrom
//=======================================================================================
ChangesetSchema ChangesetSchema::ExtractFrom(ECDbCR ecdb, ChangeStream& changes)
    {
    ChangesetSchema result;
    std::map<Utf8String, std::vector<Utf8String>> tableColumnsCache;

    ChangeIterator iter(ecdb, changes);
    for (auto const& row : iter)
        {
        if (!row.IsMapped())
            continue;

        ECClassCP cls = row.GetPrimaryClass();
        if (cls == nullptr)
            continue;

        Utf8StringCR tableName = row.GetTableName();
        ClassMap const* classMap = ecdb.Schemas().Main().GetClassMap(*cls);
        if (classMap == nullptr)
            continue;

        if (cls->IsRelationshipClass())
            {
            // Only capture link-table relationship rows (FK end-table rows surface as entity rows).
            if (classMap->GetType() != ClassMap::Type::RelationshipLinkTable)
                continue;

            ECClassId classId = cls->GetId();
            auto& classEntry = result.m_classes[classId];
            if (classEntry.fullName.empty())
                {
                classEntry.classId = classId;
                classEntry.fullName = cls->GetFullName();
                classEntry.isRelationshipClass = true;
                }

            if (classEntry.tableSegments.find(tableName) != classEntry.tableSegments.end())
                continue;

            TableSegment seg;
            PopulateTableSegment(seg, *classMap, tableName, ecdb, tableColumnsCache);

            // Capture SourceECClassId and TargetECClassId as classIdRefCols (empty referencedClassFullName
            // means "apply general entity classIdRemap" during Transform).
            auto const& relMap = classMap->GetAs<RelationshipClassMap>();
            auto indexOf = [&](Utf8StringCR name) -> int {
                auto cacheIt = tableColumnsCache.find(tableName);
                if (cacheIt == tableColumnsCache.end()) return -1;
                for (size_t i = 0; i < cacheIt->second.size(); ++i)
                    if (cacheIt->second[i].EqualsI(name)) return (int)i;
                return -1;
                };
            auto captureConstraintClassId = [&](ConstraintECClassIdPropertyMap const* cpm) {
                if (cpm == nullptr)
                    return;
                auto const* perTablePm = cpm->FindDataPropertyMap(tableName.c_str());
                if (perTablePm == nullptr)
                    return;
                DbColumn const& col = perTablePm->GetColumn();
                if (col.IsVirtual())
                    return;
                ClassIdRefColumn refCol;
                refCol.columnName = col.GetName();
                refCol.tableName = tableName;
                refCol.sourceColumnIndex = indexOf(col.GetName());
                // empty referencedClassFullName = general entity classIdRemap
                seg.classIdRefCols[col.GetName()] = std::move(refCol);
                };
            captureConstraintClassId(relMap.GetSourceECClassIdPropMap());
            captureConstraintClassId(relMap.GetTargetECClassIdPropMap());

            // Also capture SourceECInstanceId and TargetECInstanceId as regular data columns
            // so their values flow through the normal Transform binding path.
            auto captureConstraintInstanceId = [&](ConstraintECInstanceIdPropertyMap const* ipm, Utf8CP accessString) {
                if (ipm == nullptr)
                    return;
                auto const* perTablePm = ipm->FindDataPropertyMap(tableName.c_str());
                if (perTablePm == nullptr)
                    return;
                DbColumn const& col = perTablePm->GetColumn();
                if (col.IsVirtual())
                    return;
                ChangesetSchema::PropertyColumnMap m;
                m.accessString = accessString;
                m.tableName = tableName;
                m.columnName = col.GetName();
                m.sourceColumnIndex = indexOf(col.GetName());
                seg.columns[m.accessString] = m;
                };
            captureConstraintInstanceId(relMap.GetSourceECInstanceIdPropMap(), "SourceECInstanceId");
            captureConstraintInstanceId(relMap.GetTargetECInstanceIdPropMap(), "TargetECInstanceId");

            classEntry.tableSegments[tableName] = std::move(seg);
            continue;
            }

        ECClassId classId = cls->GetId();

        auto& classEntry = result.m_classes[classId];
        if (classEntry.fullName.empty())
            {
            classEntry.classId = classId;
            classEntry.fullName = cls->GetFullName();
            }

        if (classEntry.tableSegments.find(tableName) != classEntry.tableSegments.end())
            continue; // already populated

        TableSegment seg;
        PopulateTableSegment(seg, *classMap, tableName, ecdb, tableColumnsCache);
        classEntry.tableSegments[tableName] = std::move(seg);
        }

    return result;
    }

//=======================================================================================
// ChangesetSchema::To / From
//=======================================================================================
void ChangesetSchema::To(BeJsValue value) const
    {
    value.SetEmptyArray();
    for (auto const& kv : m_classes)
        {
        BeJsValue classObj = value.appendObject();
        classObj["classId"] = kv.second.classId.ToHexStr();
        classObj["fullName"] = kv.second.fullName;
        if (kv.second.isRelationshipClass)
            classObj["isRelationshipClass"] = true;
        BeJsValue segs = classObj["segments"];
        segs.SetEmptyArray();
        for (auto const& sk : kv.second.tableSegments)
            {
            BeJsValue segObj = segs.appendObject();
            segObj["table"] = sk.second.tableName;
            segObj["idColumnIndex"] = sk.second.idColumnIndex;
            segObj["classIdColumnIndex"] = sk.second.classIdColumnIndex;
            segObj["sourceColumnCount"] = sk.second.sourceColumnCount;
            BeJsValue colsArr = segObj["columns"];
            colsArr.SetEmptyArray();
            for (auto const& ck : sk.second.columns)
                {
                BeJsValue colObj = colsArr.appendObject();
                colObj["accessString"] = ck.second.accessString;
                colObj["columnName"] = ck.second.columnName;
                colObj["sourceColumnIndex"] = ck.second.sourceColumnIndex;
                }
            if (!sk.second.classIdRefCols.empty())
                {
                BeJsValue refColsArr = segObj["classIdRefCols"];
                refColsArr.SetEmptyArray();
                for (auto const& rk : sk.second.classIdRefCols)
                    {
                    BeJsValue refObj = refColsArr.appendObject();
                    refObj["columnName"] = rk.second.columnName;
                    refObj["sourceColumnIndex"] = rk.second.sourceColumnIndex;
                    if (!rk.second.referencedClassFullName.empty())
                        {
                        refObj["referencedClassFullName"] = rk.second.referencedClassFullName;
                        refObj["referencedClassId"] = rk.second.referencedClassId.ToHexStr();
                        }
                    }
                }
            }
        }
    }

ChangesetSchema ChangesetSchema::From(BeJsConst value)
    {
    ChangesetSchema result;
    if (!value.isArray())
        {
        // log a warning via local logger
        return result;
        }

    value.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst classObj) {
        if (!classObj.isObject())
            return false;
        ClassEntry entry;
        Utf8String classIdStr = classObj["classId"].asString();
        BentleyStatus stat;
        BeInt64Id raw = BeInt64Id::FromString(classIdStr.c_str(), &stat);
        if (stat != SUCCESS)
            return false;
        entry.classId = ECClassId(raw.GetValueUnchecked());
        entry.fullName = classObj["fullName"].asString();
        entry.isRelationshipClass = classObj["isRelationshipClass"].asBool(false);
        BeJsConst segs = classObj["segments"];
        if (segs.isArray())
            {
            segs.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst segObj) {
                if (!segObj.isObject())
                    return false;
                TableSegment seg;
                seg.tableName = segObj["table"].asString();
                seg.idColumnIndex = segObj["idColumnIndex"].asInt(-1);
                seg.classIdColumnIndex = segObj["classIdColumnIndex"].asInt(-1);
                seg.sourceColumnCount = segObj["sourceColumnCount"].asInt(0);
                BeJsConst colsArr = segObj["columns"];
                if (colsArr.isArray())
                    {
                    colsArr.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst colObj) {
                        if (!colObj.isObject())
                            return false;
                        PropertyColumnMap m;
                        m.accessString = colObj["accessString"].asString();
                        m.tableName = seg.tableName;
                        m.columnName = colObj["columnName"].asString();
                        m.sourceColumnIndex = colObj["sourceColumnIndex"].asInt(-1);
                        seg.columns[m.accessString] = m;
                        return false;
                        });
                    }
                BeJsConst refColsArr = segObj["classIdRefCols"];
                if (refColsArr.isArray())
                    {
                    refColsArr.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst refObj) {
                        if (!refObj.isObject())
                            return false;
                        ClassIdRefColumn rc;
                        rc.columnName = refObj["columnName"].asString();
                        rc.tableName = seg.tableName;
                        rc.sourceColumnIndex = refObj["sourceColumnIndex"].asInt(-1);
                        rc.referencedClassFullName = refObj["referencedClassFullName"].asString("");
                        Utf8String refCidStr = refObj["referencedClassId"].asString("");
                        if (!refCidStr.empty())
                            {
                            BentleyStatus st2;
                            BeInt64Id refRaw = BeInt64Id::FromString(refCidStr.c_str(), &st2);
                            if (st2 == SUCCESS)
                                rc.referencedClassId = ECClassId(refRaw.GetValueUnchecked());
                            }
                        if (rc.IsValid())
                            seg.classIdRefCols[rc.columnName] = std::move(rc);
                        return false;
                        });
                    }
                entry.tableSegments[seg.tableName] = std::move(seg);
                return false;
                });
            }
        result.m_classes[entry.classId] = std::move(entry);
        return false;
        });

    return result;
    }

//=======================================================================================
// ChangesetSchema::Validate
//=======================================================================================
BentleyStatus ChangesetSchema::Validate(ECDbCR ecdb, ChangeStream& changes, bvector<Utf8String>* outErrors) const
    {
    auto reportError = [&](Utf8String msg) {
        if (outErrors != nullptr)
            outErrors->push_back(std::move(msg));
        };

    ChangeIterator iter(ecdb, changes);
    bool ok = true;
    for (auto const& row : iter)
        {
        if (!row.IsMapped())
            continue;
        ECClassCP cls = row.GetPrimaryClass();
        if (cls == nullptr)
            continue;
        // Skip FK end-table relationship rows — they aren't in the schema.
        if (cls->IsRelationshipClass())
            {
            ClassMap const* cm = ecdb.Schemas().Main().GetClassMap(*cls);
            if (cm == nullptr || cm->GetType() != ClassMap::Type::RelationshipLinkTable)
                continue;
            }

        ECClassId classId = cls->GetId();
        auto cit = m_classes.find(classId);
        if (cit == m_classes.end())
            {
            reportError(Utf8PrintfString("class %s (id=%s) present in changeset but missing from schema",
                                         cls->GetFullName(), classId.ToHexStr().c_str()));
            ok = false;
            continue;
            }
        if (!cit->second.fullName.EqualsI(cls->GetFullName()))
            {
            reportError(Utf8PrintfString("class id=%s fullName mismatch: schema=%s ecdb=%s",
                                         classId.ToHexStr().c_str(), cit->second.fullName.c_str(), cls->GetFullName()));
            ok = false;
            }

        Utf8StringCR tableName = row.GetTableName();
        auto sit = cit->second.tableSegments.find(tableName);
        if (sit == cit->second.tableSegments.end())
            {
            reportError(Utf8PrintfString("class %s changeset references table %s not in schema",
                                         cls->GetFullName(), tableName.c_str()));
            ok = false;
            continue;
            }
        }
    return ok ? SUCCESS : ERROR;
    }

//=======================================================================================
// ChangesetSchemaDiff::ClassDiff::IsTransformable
//=======================================================================================
bool ChangesetSchemaDiff::ClassDiff::IsTransformable() const
    {
    if (classLost)
        return false;
    for (auto const& d : columnDiffs)
        if (d.kind == ChangeKind::PropertyLost)
            return false;
    return true;
    }

//=======================================================================================
// ChangesetSchemaDiff::IsTransformable
//=======================================================================================
bool ChangesetSchemaDiff::IsTransformable() const
    {
    for (auto const& cd : m_classDiffs)
        if (!cd.IsTransformable())
            return false;
    return true;
    }

//=======================================================================================
// ChangesetSchemaDiff::Compute
//=======================================================================================
ChangesetSchemaDiff ChangesetSchemaDiff::Compute(ChangesetSchema const& source, ECDbCR current)
    {
    ChangesetSchemaDiff result;

    for (auto const& kv : source.GetClasses())
        {
        ChangesetSchema::ClassEntry const& srcEntry = kv.second;

        ClassDiff cd;
        cd.sourceEntry = srcEntry;

        ECClassCP currentClass = current.Schemas().FindClass(srcEntry.fullName);
        if (currentClass == nullptr)
            {
            cd.classLost = true;
            result.m_classDiffs.push_back(std::move(cd));
            continue;
            }

        cd.targetClassId = currentClass->GetId();
        bool classIdChanged = (cd.targetClassId != srcEntry.classId);

        ClassMap const* classMap = current.Schemas().Main().GetClassMap(*currentClass);

        // Track whether we've already added a class-id remapping diff entry
        if (classIdChanged)
            {
            ColumnDiff cidd;
            cidd.kind = ChangeKind::ClassIdRemapped;
            cidd.accessString = "ECClassId";
            cd.columnDiffs.push_back(cidd);
            }

        // For each property in the source, look up the live mapping.
        for (auto const& sk : srcEntry.tableSegments)
            {
            for (auto const& ck : sk.second.columns)
                {
                ChangesetSchema::PropertyColumnMap const& oldMap = ck.second;
                ColumnDiff diff;
                diff.accessString = oldMap.accessString;
                diff.oldMap = oldMap;

                if (classMap == nullptr)
                    {
                    diff.kind = ChangeKind::PropertyLost;
                    cd.columnDiffs.push_back(diff);
                    continue;
                    }

                DbColumn const* mappedCol = nullptr;
                PropertyMap const* pm = classMap->GetPropertyMaps().Find(oldMap.accessString.c_str());
                if (pm != nullptr && pm->IsData())
                    {
                    // Walk to a SingleColumnData mapping
                    SearchPropertyMapVisitor v(PropertyMap::Type::SingleColumnData, true);
                    pm->AcceptVisitor(v);
                    if (!v.Results().empty())
                        {
                        auto const& sm = v.Results().front()->GetAs<SingleColumnDataPropertyMap>();
                        DbColumn const& col = sm.GetColumn();
                        if (!col.IsVirtual())
                            mappedCol = &col;
                        }
                    }

                if (mappedCol == nullptr && srcEntry.isRelationshipClass && classMap->GetType() == ClassMap::Type::RelationshipLinkTable)
                    {
                    auto const& relMap = classMap->GetAs<RelationshipClassMap>();
                    auto findConstraintCol = [&](ConstraintECInstanceIdPropertyMap const* ipm) -> DbColumn const* {
                        if (ipm == nullptr)
                            return nullptr;
                        auto const* perTablePm = ipm->FindDataPropertyMap(oldMap.tableName.c_str());
                        if (perTablePm == nullptr)
                            return nullptr;
                        DbColumn const& c = perTablePm->GetColumn();
                        if (c.IsVirtual())
                            return nullptr;
                        return &c;
                        };

                    if (oldMap.accessString.EqualsI("SourceECInstanceId"))
                        mappedCol = findConstraintCol(relMap.GetSourceECInstanceIdPropMap());
                    else if (oldMap.accessString.EqualsI("TargetECInstanceId"))
                        mappedCol = findConstraintCol(relMap.GetTargetECInstanceIdPropMap());
                    }

                if (mappedCol == nullptr)
                    {
                    diff.kind = ChangeKind::PropertyLost;
                    cd.columnDiffs.push_back(diff);
                    continue;
                    }

                ChangesetSchema::PropertyColumnMap newMap;
                newMap.accessString = oldMap.accessString;
                newMap.tableName = mappedCol->GetTable().GetName();
                newMap.columnName = mappedCol->GetName();
                newMap.sourceColumnIndex = -1;
                diff.newMap = newMap;

                bool sameTable  = newMap.tableName.EqualsI(oldMap.tableName);
                bool sameColumn = newMap.columnName.EqualsI(oldMap.columnName);

                if (sameTable && sameColumn)
                    continue; // no diff
                if (!sameTable)
                    diff.kind = ChangeKind::ColumnMoved;
                else
                    diff.kind = ChangeKind::ColumnRemapped;
                cd.columnDiffs.push_back(diff);
                }
            }

        // Process classIdRef columns.
        // Link-table: flag so Transform applies entity classIdRemap to constraint classId cols.
        // Entity with nav-prop classIdRefCols: detect if the referenced relationship class was
        // remapped and record a ClassIdRefDiff if so.
        if (srcEntry.isRelationshipClass)
            {
            for (auto const& sk : srcEntry.tableSegments)
                if (!sk.second.classIdRefCols.empty())
                    { cd.hasConstraintClassIdCols = true; break; }
            }
        else
            {
            for (auto const& sk : srcEntry.tableSegments)
                {
                for (auto const& rk : sk.second.classIdRefCols)
                    {
                    ChangesetSchema::ClassIdRefColumn const& refCol = rk.second;
                    if (refCol.referencedClassFullName.empty())
                        continue;
                    ECClassCP refClass = current.Schemas().FindClass(refCol.referencedClassFullName.c_str());
                    if (refClass == nullptr)
                        continue; // class lost — no remap; leave value as-is
                    ECClassId newRefClassId = refClass->GetId();
                    // Use the stored source class ID (captured at extraction time).
                    ECClassId oldRefClassId = refCol.referencedClassId;
                    if (!oldRefClassId.IsValid() || oldRefClassId == newRefClassId)
                        continue; // no remap needed
                    ClassIdRefDiff rid;
                    rid.tableName = refCol.tableName;
                    rid.columnName = refCol.columnName;
                    rid.sourceColumnIndex = refCol.sourceColumnIndex;
                    rid.oldClassId = oldRefClassId;
                    rid.newClassId = newRefClassId;
                    cd.classIdRefDiffs.push_back(rid);
                    }
                }
            }

        // Only retain class diffs that actually have changes
        if (classIdChanged || !cd.columnDiffs.empty() || cd.classLost ||
            !cd.classIdRefDiffs.empty() || cd.hasConstraintClassIdCols)
            result.m_classDiffs.push_back(std::move(cd));
        }

    return result;
    }

//=======================================================================================
// Transform helpers
//=======================================================================================
namespace {

// Captured value bag for one stage of one column in a source change.
struct CapturedValue
    {
    enum class Type { Null, Int, Real, Text, Blob, Undefined };
    Type type = Type::Undefined;
    int64_t intVal = 0;
    double  realVal = 0.0;
    std::string textVal;
    std::vector<Byte> blobVal;

    static CapturedValue Capture(DbValue const& v)
        {
        CapturedValue out;
        if (!v.IsValid())
            { out.type = Type::Undefined; return out; }
        switch (v.GetValueType())
            {
            case DbValueType::NullVal:    out.type = Type::Null; break;
            case DbValueType::IntegerVal: out.type = Type::Int; out.intVal = v.GetValueInt64(); break;
            case DbValueType::FloatVal:   out.type = Type::Real; out.realVal = v.GetValueDouble(); break;
            case DbValueType::TextVal:
                out.type = Type::Text;
                {
                Utf8CP t = v.GetValueText();
                if (t) out.textVal.assign(t, (size_t)v.GetValueBytes());
                }
                break;
            case DbValueType::BlobVal:
                out.type = Type::Blob;
                {
                int sz = v.GetValueBytes();
                Byte const* p = (Byte const*)v.GetValueBlob();
                if (p && sz > 0) out.blobVal.assign(p, p + sz);
                }
                break;
            default: out.type = Type::Undefined; break;
            }
        return out;
        }

    void BindTo(ChangeBuilder::Row& r, ChangeBuilder::Row::Stage stage, int colIdx) const
        {
        switch (type)
            {
            case Type::Undefined: break; // do not bind
            case Type::Null:      r.BindNull(stage, colIdx); break;
            case Type::Int:       r.BindInt64(stage, colIdx, intVal); break;
            case Type::Real:      r.BindDouble(stage, colIdx, realVal); break;
            case Type::Text:      r.BindText(stage, colIdx, textVal.c_str(), (int)textVal.size()); break;
            case Type::Blob:      r.BindBlob(stage, colIdx, blobVal.empty() ? nullptr : blobVal.data(), (int)blobVal.size()); break;
            }
        }
    bool IsBound() const { return type != Type::Undefined; }
    };

struct CapturedColumn
    {
    CapturedValue oldVal;
    CapturedValue newVal;
    };

} // anonymous

//=======================================================================================
// ChangesetSchemaDiff::Transform
//=======================================================================================
DbResult ChangesetSchemaDiff::Transform(ChangeStream& source, ChangeSet& target, ECDbCR targetDb) const
    {
    if (!IsTransformable())
        return BE_SQLITE_ERROR;

    // ---- Build lookup tables ----
    // sourceTable -> ClassDiff*
    bmap<Utf8String, ClassDiff const*> sourceTableToDiff;
    // (sourceTable, accessString) -> ColumnDiff*
    struct ColKey { Utf8String table; Utf8String accessString; bool operator<(ColKey const& o) const { int c = table.CompareToI(o.table); return c < 0 || (c == 0 && accessString.CompareToI(o.accessString) < 0); } };
    bmap<ColKey, ColumnDiff const*> sourceColDiff;
    // (sourceTable, accessString) -> oldMap (always available from source schema)
    bmap<ColKey, ChangesetSchema::PropertyColumnMap> sourceCols;
    // ECClassId-remap kind set per class
    bmap<ECClassId, ECClassId> classIdRemap;

    for (auto const& cd : m_classDiffs)
        {
        for (auto const& sk : cd.sourceEntry.tableSegments)
            sourceTableToDiff[sk.second.tableName] = &cd;
        if (cd.targetClassId.IsValid() && cd.targetClassId != cd.sourceEntry.classId)
            classIdRemap[cd.sourceEntry.classId] = cd.targetClassId;
        for (auto const& sk : cd.sourceEntry.tableSegments)
            for (auto const& ck : sk.second.columns)
                sourceCols[ColKey{sk.second.tableName, ck.second.accessString}] = ck.second;
        for (auto const& d : cd.columnDiffs)
            {
            if (d.kind == ChangeKind::ColumnRemapped || d.kind == ChangeKind::ColumnMoved)
                sourceColDiff[ColKey{d.oldMap.tableName, d.accessString}] = &d;
            }
        }

    // Reverse map: targetClassId -> sourceClassId (for re-identifying UPDATE rows queried from targetDb).
    bmap<ECClassId, ECClassId> reverseClassIdRemap;
    for (auto const& r : classIdRemap)
        reverseClassIdRemap[r.second] = r.first;

    // Build target table column layouts on demand.
    struct TargetLayout
        {
        bvector<Utf8String> columnNames;
        bvector<bool>       notnull;
        int                 idColIdx = -1;
        int                 classIdColIdx = -1;
        int Find(Utf8StringCR n) const
            {
            for (size_t i = 0; i < columnNames.size(); ++i)
                if (columnNames[i].EqualsI(n)) return (int)i;
            return -1;
            }
        };
    bmap<Utf8String, TargetLayout> targetLayouts;
    auto getTargetLayout = [&](Utf8StringCR tableName) -> TargetLayout const* {
        auto it = targetLayouts.find(tableName);
        if (it != targetLayouts.end())
            return &it->second;
        TargetLayout layout;
        Statement stmt;
        Utf8PrintfString sql("PRAGMA table_info(\"%s\")", tableName.c_str());
        if (BE_SQLITE_OK != stmt.Prepare(targetDb, sql.c_str()))
            return nullptr;
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            layout.columnNames.push_back(stmt.GetValueText(1));
            layout.notnull.push_back(stmt.GetValueInt(3) != 0);
            }
        if (auto const* dbTable = targetDb.Schemas().Main().GetDbSchema().FindTable(tableName))
            {
            if (auto const* idCol = dbTable->FindFirst(DbColumn::Kind::ECInstanceId))
                layout.idColIdx = layout.Find(idCol->GetName());
            if (auto const* cidCol = dbTable->FindFirst(DbColumn::Kind::ECClassId))
                if (cidCol->GetPersistenceType() != PersistenceType::Virtual)
                    layout.classIdColIdx = layout.Find(cidCol->GetName());
            }
        return &(targetLayouts[tableName] = std::move(layout));
        };

    ChangeBuilder builder(targetDb);

    // ---- Phase 1: walk source raw changes; group EC-mapped rows by (classId, instanceId) ----
    struct SourceTableRow
        {
        Utf8String  tableName;
        DbOpcode    opcode;
        bool        indirect = false;
        ECClassId   classId; // resolved
        ECInstanceId instanceId;
        bmap<Utf8String /*accessString*/, CapturedColumn> values; // by EC access string
        // Captured classIdRef column values (nav-prop RelECClassId, link-table SourceECClassId / TargetECClassId).
        bmap<Utf8String /*columnName*/, CapturedColumn> classIdRefColVals;
        // Captured PK + classId raw values for reuse
        CapturedValue idOld, idNew;
        CapturedValue classIdOld, classIdNew;
        };
    struct InstanceKey
        {
        ECClassId classId;
        ECInstanceId instanceId;
        bool operator<(InstanceKey const& o) const
            { return classId < o.classId || (classId == o.classId && instanceId.GetValue() < o.instanceId.GetValue()); }
        };
    struct InstanceGroup
        {
        DbOpcode primaryOpcode = DbOpcode::Update;
        bool     indirect = false;
        bmap<Utf8String /*tableName*/, SourceTableRow> rows;
        };
    bmap<InstanceKey, InstanceGroup> groups;

    DbResult rc = BE_SQLITE_OK;

    Changes srcChanges = source.GetChanges();
    for (auto const& change : srcChanges)
        {
        Utf8StringCR tableName = change.GetTableName();
        auto diffIt = sourceTableToDiff.find(tableName);
        if (diffIt == sourceTableToDiff.end())
            {
            // Non-EC-mapped row: pass through
            DbResult r = builder.AppendChange(change);
            if (r != BE_SQLITE_OK) { rc = r; break; }
            continue;
            }

        {
        ClassDiff const& cd = *diffIt->second;
        // Locate the source TableSegment
        auto segIt = cd.sourceEntry.tableSegments.find(tableName);
        if (segIt == cd.sourceEntry.tableSegments.end())
            {
            DbResult r = builder.AppendChange(change);
            if (r != BE_SQLITE_OK) { rc = r; break; }
            continue;
            }
        ChangesetSchema::TableSegment const& seg = segIt->second;

        Changes::Change::Stage oldS = Changes::Change::Stage::Old;
        Changes::Change::Stage newS = Changes::Change::Stage::New;

        // Opcode-guarded value helpers: sqlite3changeset_old asserts on INSERT rows and
        // sqlite3changeset_new asserts on DELETE rows, so we must check the opcode first.
        auto getOld = [&](int idx) -> DbValue {
            return (change.GetOpcode() != DbOpcode::Insert) ? change.GetValue(idx, oldS) : DbValue(nullptr);
            };
        auto getNew = [&](int idx) -> DbValue {
            return (change.GetOpcode() != DbOpcode::Delete) ? change.GetValue(idx, newS) : DbValue(nullptr);
            };

        // Resolve ECInstanceId
        ECInstanceId instanceId;
        if (seg.idColumnIndex >= 0)
            {
            DbValue idValOld = getOld(seg.idColumnIndex);
            DbValue idValNew = getNew(seg.idColumnIndex);
            uint64_t idVal = 0;
            if (idValNew.IsValid() && !idValNew.IsNull()) idVal = (uint64_t)idValNew.GetValueInt64();
            else if (idValOld.IsValid() && !idValOld.IsNull()) idVal = (uint64_t)idValOld.GetValueInt64();
            instanceId = ECInstanceId(idVal);
            }
        if (!instanceId.IsValid())
            {
            // Cannot group; pass through unchanged
            DbResult r = builder.AppendChange(change);
            if (r != BE_SQLITE_OK) { rc = r; break; }
            continue;
            }

        // Resolve ECClassId for the row
        ECClassId rowClassId = cd.sourceEntry.classId;
        bool classIdFoundInChange = false;
        if (seg.classIdColumnIndex >= 0)
            {
            DbValue cidNew = getNew(seg.classIdColumnIndex);
            DbValue cidOld = getOld(seg.classIdColumnIndex);
            if (cidNew.IsValid() && !cidNew.IsNull())      { rowClassId = ECClassId((uint64_t)cidNew.GetValueInt64()); classIdFoundInChange = true; }
            else if (cidOld.IsValid() && !cidOld.IsNull()) { rowClassId = ECClassId((uint64_t)cidOld.GetValueInt64()); classIdFoundInChange = true; }
            }
        // For UPDATE rows ECClassId is absent from the change data; query the DB for the actual class.
        if (!classIdFoundInChange && change.GetOpcode() == DbOpcode::Update && instanceId.IsValid() && seg.classIdColumnIndex >= 0)
            {
            if (DbTable const* dbTable = targetDb.Schemas().Main().GetDbSchema().FindTable(tableName))
                {
                DbColumn const* classIdCol = dbTable->FindFirst(DbColumn::Kind::ECClassId);
                DbColumn const* idCol      = dbTable->FindFirst(DbColumn::Kind::ECInstanceId);
                if (classIdCol != nullptr && idCol != nullptr && classIdCol->GetPersistenceType() != PersistenceType::Virtual)
                    {
                    ECClassId queriedId;
                    if (SUCCESS == DbUtilities::QueryRowClassId(queriedId, targetDb, tableName, classIdCol->GetName(), idCol->GetName(), instanceId) && queriedId.IsValid())
                        {
                        // Map back to source classId in case the class was remapped between source and target schemas.
                        auto revIt = reverseClassIdRemap.find(queriedId);
                        rowClassId = (revIt != reverseClassIdRemap.end()) ? revIt->second : queriedId;
                        }
                    }
                }
            }

        InstanceKey ikey{rowClassId, instanceId};
        InstanceGroup& g = groups[ikey];
        SourceTableRow& srow = g.rows[tableName];
        srow.tableName = tableName;
        srow.opcode    = change.GetOpcode();
        srow.indirect  = change.IsIndirect();
        srow.classId   = rowClassId;
        srow.instanceId = instanceId;

        if (seg.idColumnIndex >= 0)
            {
            srow.idOld = CapturedValue::Capture(getOld(seg.idColumnIndex));
            srow.idNew = CapturedValue::Capture(getNew(seg.idColumnIndex));
            }
        if (seg.classIdColumnIndex >= 0)
            {
            srow.classIdOld = CapturedValue::Capture(getOld(seg.classIdColumnIndex));
            srow.classIdNew = CapturedValue::Capture(getNew(seg.classIdColumnIndex));
            }

        for (auto const& ck : seg.columns)
            {
            int idx = ck.second.sourceColumnIndex;
            if (idx < 0) continue;
            CapturedColumn cc;
            cc.oldVal = CapturedValue::Capture(getOld(idx));
            cc.newVal = CapturedValue::Capture(getNew(idx));
            if (cc.oldVal.IsBound() || cc.newVal.IsBound())
                srow.values[ck.second.accessString] = cc;
            }

        // Capture classIdRef column values (nav-prop RelECClassId, link-table constraint classIds).
        for (auto const& rk : seg.classIdRefCols)
            {
            int idx = rk.second.sourceColumnIndex;
            if (idx < 0) continue;
            CapturedColumn cc;
            cc.oldVal = CapturedValue::Capture(getOld(idx));
            cc.newVal = CapturedValue::Capture(getNew(idx));
            if (cc.oldVal.IsBound() || cc.newVal.IsBound())
                srow.classIdRefColVals[rk.second.columnName] = cc;
            }

        // Track primary op (use this row's opcode for now; will reconcile later)
        g.primaryOpcode = srow.opcode;
        if (srow.indirect) g.indirect = true;
        }
        }

    // ---- Phase 2: emit transformed rows per InstanceGroup ----
    for (auto& gkv : groups)
        {
        if (rc != BE_SQLITE_OK) break;
        InstanceGroup& g = gkv.second;
        // Find ClassDiff
        ClassDiff const* cd = nullptr;
        for (auto const& d : m_classDiffs)
            if (d.sourceEntry.classId == gkv.first.classId) { cd = &d; break; }
        if (cd == nullptr)
            continue; // shouldn't happen — we only added rows backed by a diff

        // For each property in source, determine its target placement.
        // Map: targetTable -> (accessString -> CapturedColumn)
        bmap<Utf8String, bmap<Utf8String, CapturedColumn>> outValues;
        // Track which target tables need to exist
        bset<Utf8String> outTables;

        // Seed: every source-table that had a row gets a corresponding target row
        // (unless every property in it was moved away — then it may still get a PK-only row).
        for (auto const& rk : g.rows)
            outTables.insert(rk.first); // target table name == source table name unless renamed (not in our model)

        // Apply per-property placement
        for (auto const& rk : g.rows)
            {
            for (auto const& vk : rk.second.values)
                {
                Utf8String accessString = vk.first;
                Utf8String targetTable = rk.first;
                auto dIt = sourceColDiff.find(ColKey{rk.first, accessString});
                if (dIt != sourceColDiff.end())
                    {
                    ColumnDiff const* d = dIt->second;
                    if (d->kind == ChangeKind::ColumnMoved)
                        targetTable = d->newMap.tableName;
                    // ColumnRemapped: same table, but column index changes — handled at emit time
                    }
                outTables.insert(targetTable);
                outValues[targetTable][accessString] = vk.second;
                }
            }

        // Determine instance id values to use for synthesised rows
        CapturedValue idOld, idNew;
        for (auto const& rk : g.rows)
            {
            if (!idOld.IsBound() && rk.second.idOld.IsBound()) idOld = rk.second.idOld;
            if (!idNew.IsBound() && rk.second.idNew.IsBound()) idNew = rk.second.idNew;
            }

        // Determine ECClassId values; track whether ECClassId was present in the source change.
        CapturedValue cidOld, cidNew;
        bool classIdWasInSourceChange = false;
        for (auto const& rk : g.rows)
            {
            if (!cidOld.IsBound() && rk.second.classIdOld.IsBound()) { cidOld = rk.second.classIdOld; classIdWasInSourceChange = true; }
            if (!cidNew.IsBound() && rk.second.classIdNew.IsBound()) { cidNew = rk.second.classIdNew; classIdWasInSourceChange = true; }
            }
        // Apply ClassIdRemapped: overwrite with target classId (relevant for Insert/Delete).
        auto cidRemapIt = classIdRemap.find(gkv.first.classId);
        if (cidRemapIt != classIdRemap.end())
            {
            CapturedValue v;
            v.type = CapturedValue::Type::Int;
            v.intVal = (int64_t)cidRemapIt->second.GetValue();
            cidOld = v; cidNew = v;
            }

        // Determine the global (instance-level) opcode
        DbOpcode primaryOp = g.primaryOpcode;
        for (auto const& rk : g.rows)
            {
            if (rk.second.opcode == DbOpcode::Insert) { primaryOp = DbOpcode::Insert; break; }
            if (rk.second.opcode == DbOpcode::Delete) { primaryOp = DbOpcode::Delete; break; }
            }

        // Emit one row per target table
        for (auto const& targetTable : outTables)
            {
            TargetLayout const* layout = getTargetLayout(targetTable);
            if (layout == nullptr || layout->columnNames.empty())
                continue;

            // Look up the source row for this table (needed for classIdRef values).
            auto srowIt = g.rows.find(targetTable);

            // Determine opcode for THIS target row
            DbOpcode rowOp = primaryOp;
            // Look up the target ClassMap to find each access string's column name in this target table
            ECClassCP targetClass = targetDb.Schemas().FindClass(cd->sourceEntry.fullName);
            ClassMap const* targetClassMap = targetClass ? targetDb.Schemas().Main().GetClassMap(*targetClass) : nullptr;

            // Collect accessString -> target column name (only for cols in this target table)
            bmap<Utf8String, Utf8String> accessToCol;
            if (targetClassMap != nullptr)
                {
                SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData, true);
                targetClassMap->GetPropertyMaps().AcceptVisitor(visitor);
                for (PropertyMap const* pm : visitor.Results())
                    {
                    auto const& sm = pm->GetAs<SingleColumnDataPropertyMap>();
                    DbColumn const& col = sm.GetColumn();
                    if (col.IsVirtual()) continue;
                    if (!col.GetTable().GetName().EqualsI(targetTable)) continue;
                    accessToCol[sm.GetAccessString()] = col.GetName();
                    }
                // For link-table relationships also map SourceECInstanceId / TargetECInstanceId.
                if (cd->sourceEntry.isRelationshipClass)
                    {
                    auto const& relMap2 = targetClassMap->GetAs<RelationshipClassMap>();
                    auto addConstraintInstId = [&](ConstraintECInstanceIdPropertyMap const* cpm, Utf8CP accessStr) {
                        if (cpm == nullptr) return;
                        auto const* perTablePm = cpm->FindDataPropertyMap(targetTable.c_str());
                        if (perTablePm == nullptr) return;
                        DbColumn const& col = perTablePm->GetColumn();
                        if (!col.IsVirtual())
                            accessToCol[accessStr] = col.GetName();
                        };
                    addConstraintInstId(relMap2.GetSourceECInstanceIdPropMap(), "SourceECInstanceId");
                    addConstraintInstId(relMap2.GetTargetECInstanceIdPropMap(), "TargetECInstanceId");
                    }
                }

            auto const& vals = outValues[targetTable];
            bool anyDataBound = false;

            DbResult r = builder.AppendRow(rowOp, targetTable.c_str(), g.indirect, [&](ChangeBuilder::Row& row) {
                using Stage = ChangeBuilder::Row::Stage;

                // Bind ECInstanceId (PK)
                if (layout->idColIdx >= 0)
                    {
                    if (rowOp == DbOpcode::Insert || rowOp == DbOpcode::Delete)
                        {
                        CapturedValue const& v = (rowOp == DbOpcode::Delete) ? idOld : idNew;
                        if (v.IsBound()) v.BindTo(row, (rowOp == DbOpcode::Delete) ? Stage::Old : Stage::New, layout->idColIdx);
                        }
                    else // Update
                        {
                        if (idOld.IsBound()) idOld.BindTo(row, Stage::Old, layout->idColIdx);
                        if (idNew.IsBound()) idNew.BindTo(row, Stage::New, layout->idColIdx);
                        else if (idOld.IsBound()) idOld.BindTo(row, Stage::New, layout->idColIdx);
                        }
                    }

                // Bind ECClassId if persisted.
                // For UPDATE, ECClassId is absent from the source change and must not be synthesised.
                if (layout->classIdColIdx >= 0)
                    {
                    if (rowOp == DbOpcode::Insert)
                        { if (cidNew.IsBound()) cidNew.BindTo(row, Stage::New, layout->classIdColIdx); }
                    else if (rowOp == DbOpcode::Delete)
                        { if (cidOld.IsBound()) cidOld.BindTo(row, Stage::Old, layout->classIdColIdx); }
                    else if (classIdWasInSourceChange)  // Update: only write if ECClassId was actually in the source change
                        {
                        if (cidOld.IsBound()) cidOld.BindTo(row, Stage::Old, layout->classIdColIdx);
                        if (cidNew.IsBound()) cidNew.BindTo(row, Stage::New, layout->classIdColIdx);
                        }
                    }

                // Bind data columns
                for (auto const& vk : vals)
                    {
                    auto colNameIt = accessToCol.find(vk.first);
                    if (colNameIt == accessToCol.end()) continue;
                    int colIdx = layout->Find(colNameIt->second);
                    if (colIdx < 0) continue;
                    CapturedColumn const& cc = vk.second;
                    if (rowOp == DbOpcode::Insert)
                        {
                        if (cc.newVal.IsBound()) { cc.newVal.BindTo(row, Stage::New, colIdx); anyDataBound = true; }
                        }
                    else if (rowOp == DbOpcode::Delete)
                        {
                        if (cc.oldVal.IsBound()) { cc.oldVal.BindTo(row, Stage::Old, colIdx); anyDataBound = true; }
                        }
                    else
                        {
                        if (cc.oldVal.IsBound()) { cc.oldVal.BindTo(row, Stage::Old, colIdx); anyDataBound = true; }
                        if (cc.newVal.IsBound()) { cc.newVal.BindTo(row, Stage::New, colIdx); anyDataBound = true; }
                        }
                    }

                // Apply classIdRef column remaps (nav-prop RelECClassId + link-table constraint classIds).
                if (srowIt != g.rows.end())
                    {
                    // Helper: apply a specific old->new classId remap to one captured value.
                    auto applySpecificRemap = [](CapturedValue const& v, ECClassId oldId, ECClassId newId) -> CapturedValue {
                        if (v.type != CapturedValue::Type::Int) return v;
                        if ((uint64_t)v.intVal != oldId.GetValue()) return v;
                        CapturedValue out = v;
                        out.intVal = (int64_t)newId.GetValue();
                        return out;
                        };
                    // Helper: apply the general entity classIdRemap to one captured value.
                    auto applyGeneralRemap = [&](CapturedValue const& v) -> CapturedValue {
                        if (v.type != CapturedValue::Type::Int) return v;
                        ECClassId id((uint64_t)v.intVal);
                        auto it = classIdRemap.find(id);
                        if (it == classIdRemap.end()) return v;
                        CapturedValue out = v;
                        out.intVal = (int64_t)it->second.GetValue();
                        return out;
                        };

                    // Nav-prop RelECClassId columns: specific remap per ClassIdRefDiff.
                    for (auto const& rid : cd->classIdRefDiffs)
                        {
                        if (!rid.tableName.EqualsI(targetTable)) continue;
                        int colIdx = layout->Find(rid.columnName);
                        if (colIdx < 0) continue;
                        auto cit = srowIt->second.classIdRefColVals.find(rid.columnName);
                        if (cit == srowIt->second.classIdRefColVals.end()) continue;
                        CapturedColumn const& cc = cit->second;
                        if (rowOp == DbOpcode::Insert)
                            { CapturedValue nv = applySpecificRemap(cc.newVal, rid.oldClassId, rid.newClassId); if (nv.IsBound()) { nv.BindTo(row, Stage::New, colIdx); anyDataBound = true; } }
                        else if (rowOp == DbOpcode::Delete)
                            { CapturedValue ov = applySpecificRemap(cc.oldVal, rid.oldClassId, rid.newClassId); if (ov.IsBound()) { ov.BindTo(row, Stage::Old, colIdx); anyDataBound = true; } }
                        else
                            {
                            if (cc.oldVal.IsBound()) { CapturedValue ov = applySpecificRemap(cc.oldVal, rid.oldClassId, rid.newClassId); ov.BindTo(row, Stage::Old, colIdx); anyDataBound = true; }
                            if (cc.newVal.IsBound()) { CapturedValue nv = applySpecificRemap(cc.newVal, rid.oldClassId, rid.newClassId); nv.BindTo(row, Stage::New, colIdx); anyDataBound = true; }
                            }
                        }

                    // Link-table constraint classId columns (SourceECClassId / TargetECClassId): general entity remap.
                    if (cd->hasConstraintClassIdCols)
                        {
                        auto segIt2 = cd->sourceEntry.tableSegments.find(targetTable);
                        if (segIt2 != cd->sourceEntry.tableSegments.end())
                            {
                            for (auto const& rk : segIt2->second.classIdRefCols)
                                {
                                ChangesetSchema::ClassIdRefColumn const& refCol = rk.second;
                                if (!refCol.referencedClassFullName.empty()) continue; // nav-prop, already handled
                                int colIdx = layout->Find(refCol.columnName);
                                if (colIdx < 0) continue;
                                auto cit = srowIt->second.classIdRefColVals.find(refCol.columnName);
                                if (cit == srowIt->second.classIdRefColVals.end()) continue;
                                CapturedColumn const& cc = cit->second;
                                if (rowOp == DbOpcode::Insert)
                                    { CapturedValue nv = applyGeneralRemap(cc.newVal); if (nv.IsBound()) { nv.BindTo(row, Stage::New, colIdx); anyDataBound = true; } }
                                else if (rowOp == DbOpcode::Delete)
                                    { CapturedValue ov = applyGeneralRemap(cc.oldVal); if (ov.IsBound()) { ov.BindTo(row, Stage::Old, colIdx); anyDataBound = true; } }
                                else
                                    {
                                    if (cc.oldVal.IsBound()) { CapturedValue ov = applyGeneralRemap(cc.oldVal); ov.BindTo(row, Stage::Old, colIdx); anyDataBound = true; }
                                    if (cc.newVal.IsBound()) { CapturedValue nv = applyGeneralRemap(cc.newVal); nv.BindTo(row, Stage::New, colIdx); anyDataBound = true; }
                                    }
                                }
                            }
                        }
                    }

                // For INSERT: any unbound non-null column in target needs a value.
                // We bind NULL as a fallback (will fail for NOT NULL columns).
                if (rowOp == DbOpcode::Insert)
                    {
                    // Collect classIdRef column names to skip in the fallback.
                    bset<Utf8String> classIdRefColNames;
                    auto segIt3 = cd->sourceEntry.tableSegments.find(targetTable);
                    if (segIt3 != cd->sourceEntry.tableSegments.end())
                        for (auto const& rk : segIt3->second.classIdRefCols)
                            classIdRefColNames.insert(Utf8String(rk.second.columnName));

                    for (size_t i = 0; i < layout->columnNames.size(); ++i)
                        {
                        if ((int)i == layout->idColIdx) continue;
                        if ((int)i == layout->classIdColIdx) continue;
                        if (classIdRefColNames.count(layout->columnNames[i]) > 0) continue;
                        // Skip columns we already bound
                        bool bound = false;
                        for (auto const& vk : vals)
                            {
                            auto cn = accessToCol.find(vk.first);
                            if (cn != accessToCol.end() && cn->second.EqualsI(layout->columnNames[i])) { bound = true; break; }
                            }
                        if (!bound)
                            row.BindNull(Stage::New, (int)i);
                        }
                    }
                });

            // Suppress empty UPDATE rows: only PK bound, no data delta and no classId remap impact.
            // (If we emitted with no data, builder still wrote it — that's acceptable for now.)
            (void)anyDataBound;

            if (r != BE_SQLITE_OK) { rc = r; break; }
            }
        }

/* end Phase */
    if (rc != BE_SQLITE_OK)
        return rc;

    return target.FromChangeGroup(builder.GetChangeGroup());
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
