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
DbResult InstanceRepository::Insert(BeJsValue in, BeJsConst userOptions, JsFormat inFmt, ECInstanceKeyR key, bool forceUseECInstanceIdFromJson) const {
    BeMutexHolder _(m_mutex);
    m_lastError.clear();
    InstanceWriter::InsertOptions options;
    options.UseJsNames(inFmt == JsFormat::JsName);
    if(forceUseECInstanceIdFromJson)
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