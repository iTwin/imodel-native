/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
using IClassHandler = InstanceRepository::IClassHandler;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InstanceRepository::Reset() {
    BeMutexHolder _(m_mutex);
    m_handlerMap.clear();
    m_handlers.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::vector<IClassHandler*>& InstanceRepository::TryGetHandlers(ECN::ECClassId classId) const {
    BeMutexHolder _(m_mutex);
    static auto s_empty = std::vector<IClassHandler*>(); // empty list
    if (m_handlers.empty()) {
        return s_empty;
    }

    auto found = m_handlerMap.find(classId);
    if (found == m_handlerMap.end()) {
        auto classP = m_ecdb.Schemas().GetClass(classId);
        if (nullptr == classP) {
            return s_empty;
        }

        std::vector<IClassHandler*> list;
        for (auto& handler : m_handlers) {
            if (m_ecdb.Schemas().IsSubClassOf(classId, handler->GetClassId())) {
                list.push_back(handler.get());
            }
        }

        std::sort(list.begin(), list.end(), [&](IClassHandler* a, IClassHandler* b) {
            return m_ecdb.Schemas().IsSubClassOf(b->GetClassId(), a->GetClassId());
        });
        return m_handlerMap.insert({classId, list}).first->second;
    }
    return found->second;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult InstanceRepository::Insert(BeJsValue in, BeJsConst userOptions, JsFormat inFmt, ECInstanceKeyR key) const {
    BeMutexHolder _(m_mutex);
    m_lastError.clear();
    InstanceWriter::InsertOptions options;
    options.UseJsNames(inFmt == JsFormat::JsName);
    ECN::ECClassId classId;
    if (!m_ecdb.GetInstanceWriter().TryGetClassId(classId, in, inFmt)) {
        m_lastError.Sprintf("Failed to get ECClassId/className/classFullName");
        return BE_SQLITE_ERROR;
    }

    auto& handlers = TryGetHandlers(classId);
    if (!handlers.empty()) {
        ECInstanceId instanceId;
        for (auto handler : handlers) {
            if (handler->OnNextId(instanceId) == PropertyHandlerResult::Handled) {
                break;
            }
        }

        if (instanceId.IsValid()) {
            options.UseInstanceId(instanceId);
        }

        for (auto handler : handlers) {
            handler->OnBeforeInsertInstance(in, userOptions, inFmt);
        }

        const auto instKey = ECInstanceKey(classId, instanceId);
        options.SetCustomBindHandler([&](ECN::ECPropertyCR prop, IECSqlBinder& binder, BeJsConst instance, BeJsConst val, ECSqlStatus& rc) {
            auto args = WriteArgs(instKey, inFmt, userOptions, binder, val, prop, instance, true);
            for (auto handler : handlers) {
                auto result = handler->OnBindProperty(args);
                if (result == PropertyHandlerResult::Handled) {
                    return PropertyHandlerResult::Handled;
                }
            }
            return PropertyHandlerResult::Continue;
        });
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

    auto handlers = TryGetHandlers(instKey.GetClassId());
    if (!handlers.empty()) {
        for (auto handler : handlers) {
            handler->OnBeforeUpdateInstance(in, userOptions, inFmt);
        }

        options.SetCustomBindHandler([&](ECN::ECPropertyCR prop, IECSqlBinder& binder, BeJsConst instance, BeJsConst val, ECSqlStatus& rc) {
            auto args = WriteArgs(instKey, inFmt, userOptions, binder, val, prop, instance, false);
            for (auto handler : handlers) {
                auto result = handler->OnBindProperty(args);
                if (result == PropertyHandlerResult::Handled) {
                    return PropertyHandlerResult::Handled;
                }
            }
            return PropertyHandlerResult::Continue;
        });
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

    auto handlers = TryGetHandlers(instKey.GetClassId());
    InstanceReader::Position pos(instKey.GetInstanceId(), instKey.GetClassId());
    auto& reader = m_ecdb.GetInstanceReader();

    auto rc = BE_SQLITE_ROW;
    if (!reader.Seek(pos, [&](const InstanceReader::IRowContext& row) {
            ECSqlRowAdaptor adaptor(m_ecdb);
            adaptor.GetOptions().SetAbbreviateBlobs(false);
            adaptor.GetOptions().SetConvertClassIdsToClassNames(fmt == JsFormat::JsName);
            adaptor.GetOptions().SetUseJsNames(fmt == JsFormat::JsName);
            adaptor.GetOptions().SetUseClassFullNameInsteadofClassName(fmt == JsFormat::JsName);
            if (!handlers.empty()) {
                adaptor.SetCustomHandler([&](BeJsValue out, IECSqlValue const& val) {
                    auto args = ReadArgs(instKey, fmt, userOptions, val, out, outInstance);
                    for (auto handler : handlers) {
                        auto result = handler->OnReadProperty(args);
                        if (result == PropertyHandlerResult::Handled) {
                            return PropertyHandlerResult::Handled;
                        }
                    }
                    return PropertyHandlerResult::Continue;
                });
            }

            if (ERROR == adaptor.RenderRowAsObject(outInstance, row)) {
                rc = BE_SQLITE_ERROR;
            }
            for (auto handler : handlers) {
                handler->OnAfterReadInstance(outInstance, userOptions, fmt);
            }
        })) {
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