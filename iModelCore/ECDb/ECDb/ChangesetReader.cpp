
/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String GetJsMemberName(ECN::ECPropertyCR ecProperty) {
    Utf8String memberName = ecProperty.GetName();
    const auto prim = ecProperty.GetAsPrimitiveProperty();
    if (prim && !prim->GetExtendedTypeName().empty()) {
        const auto extendTypeId = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
        if (extendTypeId == ExtendedTypeHelper::ExtendedType::Id && memberName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
            memberName = ECN::ECJsonSystemNames::Id();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::ClassId && memberName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
            memberName = ECN::ECJsonSystemNames::ClassFullName();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::SourceId && memberName.EqualsIAscii(ECDBSYS_PROP_SourceECInstanceId))
            memberName = ECN::ECJsonSystemNames::SourceId();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::SourceClassId && memberName.EqualsIAscii(ECDBSYS_PROP_SourceECClassId))
            memberName = ECN::ECJsonSystemNames::SourceClassName();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::TargetId && memberName.EqualsIAscii(ECDBSYS_PROP_TargetECInstanceId))
            memberName = ECN::ECJsonSystemNames::TargetId();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::TargetClassId && memberName.EqualsIAscii(ECDBSYS_PROP_TargetECClassId))
            memberName = ECN::ECJsonSystemNames::TargetClassName();
        else
            ECN::ECJsonUtilities::LowerFirstChar(memberName);
    } else {
        ECN::ECJsonUtilities::LowerFirstChar(memberName);
    }
    return memberName;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetReader::ChangesetReader(): m_innerReader(std::make_unique<PreparedChangesetReader>()) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetReader::~ChangesetReader() {
    m_innerReader.reset();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenChangesetFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert, PropertyFilter propertyFilter) {
    return m_innerReader->OpenChangesetFile(ecdb, changesetFile, invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenChangeGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    return m_innerReader->OpenChangeGroup(ecdb, changesetFiles, invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenInMemoryChangeset(ECDbCR ecdb, std::unique_ptr<ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    return m_innerReader->OpenInMemoryChangeset(ecdb, std::move(changeSet), invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Close() {
    return m_innerReader->Close();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::Step() {
    return m_innerReader->Step();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetTableName(Utf8StringR tableName) const {
    return m_innerReader->GetTableName(tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetOpcode(DbOpcode& opcode) const {
    return m_innerReader->GetOpcode(opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetReader::GetValue(Stage stage, int columnIndex) const {
    return m_innerReader->GetValue(stage, columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const* ChangesetReader::GetECDb() const {
    return m_innerReader->GetECDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetReader::GetColumnCount(Stage stage) const {
    return m_innerReader->GetColumnCount(stage);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    return m_innerReader->GetInstanceKey(stage, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::IsECTable(bool& isECTable) const {
    return m_innerReader->IsECTable(isECTable);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Utf8String> const* ChangesetReader::GetChangeFetchedPropertyNames() const {
    return m_innerReader->GetChangeFetchedPropertyNames();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::IsIndirectChange(bool& isIndirect) const {
    return m_innerReader->IsIndirectChange(isIndirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetTableFilters(std::vector<Utf8String> const& tableFilters) {
    return m_innerReader->SetTableFilters(tableFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) {
    return m_innerReader->SetOpcodeFilters(opcodeFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters) {
    return m_innerReader->SetECClassNameFilters(ecclassNameFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearTableFilters() {
    return m_innerReader->ClearTableFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearOpcodeFilters() {
    return m_innerReader->ClearOpcodeFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearECClassNameFilters() {
    return m_innerReader->ClearECClassNameFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::EnableStrictMode() {
    return m_innerReader->EnableStrictMode();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::DisableStrictMode() {
    return m_innerReader->DisableStrictMode();
}


END_BENTLEY_SQLITE_EC_NAMESPACE