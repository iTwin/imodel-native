/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult InstanceRepository::Insert(BeJsValue in, BeJsConst userOptions, JsFormat inFmt, ECInstanceKeyR key) const {
    BeMutexHolder _(m_mutex);
    m_lastError.clear();
    InstanceWriter::InsertOptions options;
    options.UseJsNames(inFmt == JsFormat::JsName);
    if(userOptions.isBoolMember("forceUseId") && userOptions["forceUseId"].asBool(false))
        options.UseInstanceIdFromJs();
    ECN::ECClassId classId;
    if (!m_ecdb.GetInstanceWriter().TryGetClassId(classId, in, inFmt)) {
        m_lastError.Sprintf("Failed to get ECClassId/className/classFullName");
        return BE_SQLITE_ERROR;
    }
    auto rc = m_ecdb.GetInstanceWriter().Insert(in, options, key);
    if (rc != BE_SQLITE_OK) {
        m_lastError = m_ecdb.GetInstanceWriter().GetLastError();
    }
    return rc;
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult InstanceRepository::Update(BeJsValue in, BeJsConst userOptions, JsFormat inFmt) const {
    BeMutexHolder _(m_mutex);
    m_lastError.clear();
    InstanceWriter::UpdateOptions options;
    options.UseJsNames(inFmt == JsFormat::JsName);
    options.UseIncrementalUpdate(true);
    ECInstanceKey instKey;
    if (!m_ecdb.GetInstanceWriter().TryGetInstanceKey(instKey, in, inFmt)) {
        m_lastError.Sprintf("Failed to get ECInstanceId/id and ECClassId/className/classFullName");
        return BE_SQLITE_ERROR;
    }

    auto rc = m_ecdb.GetInstanceWriter().Update(in, options);
    if (rc != BE_SQLITE_OK) {
        m_lastError = m_ecdb.GetInstanceWriter().GetLastError();
    }
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult InstanceRepository::Delete(BeJsConst in, BeJsConst userOptions, JsFormat inFmt) const {
    BeMutexHolder _(m_mutex);
    m_lastError.clear();
    InstanceWriter::DeleteOptions options;
    options.UseJsNames(inFmt == JsFormat::JsName);
    auto rc = m_ecdb.GetInstanceWriter().Delete(in, options);
    if (rc != BE_SQLITE_OK) {
        m_lastError = m_ecdb.GetInstanceWriter().GetLastError();
    }
    return rc;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult InstanceRepository::Delete(ECInstanceKeyCR key, BeJsConst userOptions, JsFormat inFmt) const {
    BeMutexHolder _(m_mutex);
    m_lastError.clear();
    InstanceWriter::DeleteOptions options;
    options.UseJsNames(inFmt == JsFormat::JsName);
    auto rc = m_ecdb.GetInstanceWriter().Delete(key, options);
    if (rc != BE_SQLITE_OK) {
        m_lastError = m_ecdb.GetInstanceWriter().GetLastError();
    }
    return rc;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult InstanceRepository::BulkInsert(BeJsConst instances, BeJsConst userOptions, JsFormat inFmt, std::vector<ECInstanceKey>& keys, int& failedIndex) const {
    BeMutexHolder _(m_mutex);
    m_lastError.clear();
    failedIndex = -1;
    InstanceWriter::InsertOptions options;
    options.UseJsNames(inFmt == JsFormat::JsName);
    if (userOptions.isBoolMember("forceUseId") && userOptions["forceUseId"].asBool(false))
        options.UseInstanceIdFromJs();

    auto rc = m_ecdb.GetInstanceWriter().InsertBatch(instances, options, keys, failedIndex);
    if (rc != BE_SQLITE_DONE) {
        m_lastError = m_ecdb.GetInstanceWriter().GetLastError();
    }
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult InstanceRepository::BulkUpdate(BeJsConst instances, BeJsConst userOptions, JsFormat inFmt, uint64_t& affectedRows, int& failedIndex) const {
    BeMutexHolder _(m_mutex);
    m_lastError.clear();
    failedIndex = -1;
    affectedRows = 0;
    InstanceWriter::UpdateOptions options;
    options.UseJsNames(inFmt == JsFormat::JsName);
    // Incremental update re-reads each existing instance so that properties missing from the input
    // keep their current value. That matches the single instance updateInstance() behavior but is
    // expensive, so a bulk caller that always supplies complete instances can turn it off.
    const auto useIncrementalUpdate = userOptions.isBoolMember("useIncrementalUpdate")
        ? userOptions["useIncrementalUpdate"].asBool(true)
        : true;
    options.UseIncrementalUpdate(useIncrementalUpdate);

    auto rc = m_ecdb.GetInstanceWriter().UpdateBatch(instances, options, affectedRows, failedIndex);
    if (rc != BE_SQLITE_DONE) {
        m_lastError = m_ecdb.GetInstanceWriter().GetLastError();
    }
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult InstanceRepository::BulkDelete(BeJsConst keys, BeJsConst userOptions, JsFormat inFmt, uint64_t& affectedRows, int& failedIndex) const {
    BeMutexHolder _(m_mutex);
    m_lastError.clear();
    failedIndex = -1;
    affectedRows = 0;
    InstanceWriter::DeleteOptions options;
    options.UseJsNames(inFmt == JsFormat::JsName);

    auto rc = m_ecdb.GetInstanceWriter().DeleteBatch(keys, options, affectedRows, failedIndex);
    if (rc != BE_SQLITE_DONE) {
        m_lastError = m_ecdb.GetInstanceWriter().GetLastError();
    }
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult InstanceRepository::Read(ECInstanceKeyCR instKey, BeJsValue outInstance, BeJsConst userOptions, JsFormat fmt) const {
    BeMutexHolder _(m_mutex);
    m_lastError.clear();
    auto nullValue = BeJsDocument::Null();
    InstanceReader::Position pos(instKey.GetInstanceId(), instKey.GetClassId());
    InstanceReader::Options options;
    options.SetForceSeek(true);
    auto rc = BE_SQLITE_ROW;
    if (!m_ecdb.GetInstanceReader().Seek(pos, [&](const InstanceReader::IRowContext& row, PropertyReader::Finder finder) {
            ECSqlRowAdaptor adaptor(m_ecdb);
            bool wantGeometry = userOptions["wantGeometry"].asBool(false);
            adaptor.GetOptions().SetAbbreviateBlobs(false);
            adaptor.GetOptions().SetConvertClassIdsToClassNames(fmt == JsFormat::JsName);
            adaptor.GetOptions().SetUseJsNames(fmt == JsFormat::JsName);
            adaptor.GetOptions().SetUseClassFullNameInsteadofClassName(fmt == JsFormat::JsName);
            if(!wantGeometry){
                adaptor.SetSkipPropertyHandler([&](ECN::ECPropertyCR prop) {
                    if(ExtendedTypeHelper::FromProperty(prop) == ExtendedTypeHelper::ExtendedType::GeometryStream)
                        return true;
                    return false;
                });
            }
            if (ERROR == adaptor.RenderRowAsObject(outInstance, row)) {
                rc = BE_SQLITE_ERROR;
            }

        }, options)) {
        rc = BE_SQLITE_DONE;
    }
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult InstanceRepository::Read(BeJsConst in, BeJsValue outInstance, BeJsConst userOptions, JsFormat fmt) const {
    ECInstanceKey instKey;
    if (!m_ecdb.GetInstanceWriter().TryGetInstanceKey(instKey, in, fmt)) {
        return BE_SQLITE_ERROR;
    }
    return Read(instKey, outInstance, userOptions, fmt);
}


END_BENTLEY_SQLITE_EC_NAMESPACE