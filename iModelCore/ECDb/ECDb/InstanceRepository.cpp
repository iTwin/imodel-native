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
void InstanceRepository::Reset(bool clearHandlers) {
    BeMutexHolder _(m_mutex);
    m_reader.Reset();
    if (clearHandlers) {
        m_handlers.clear();
        m_handlerMap.clear();
    } else {
        for (auto& handler : m_handlers) {
            handler->Reset();
        }
    }
    m_cache.Empty();
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

    auto& handlers = TryGetHandlers(classId, inFmt, Operation::Insert, in, userOptions);
    if (!handlers.empty()) {
        ECInstanceId instanceId;
        for (auto handler : handlers) {
            handler->OnNextId(instanceId);
            if (instanceId.IsValid()) {
                options.UseInstanceId(instanceId);
                break;
            }
        }

        const auto instKey = ECInstanceKey(classId, instanceId);
        options.SetCustomBindHandler([&](ECN::ECPropertyCR prop, IECSqlBinder& binder, BeJsConst instance, BeJsConst val, ECSqlStatus& rc) {
            for (auto handler : handlers) {
                auto status = handler->OnBindECProperty(prop, val, binder, rc);
                if (status == PropertyHandlerResult::Handled) {
                    return PropertyHandlerResult::Handled;
                }
            }
            return PropertyHandlerResult::Continue;
        });
        options.SetUserPropertyHandler([&](Utf8CP name, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) {
            for (auto handler : handlers) {
                auto result = handler->OnBindUserProperty(name, val, finder, rc);
                if (result == PropertyHandlerResult::Handled) {
                    return PropertyHandlerResult::Handled;
                }
            }
            return PropertyHandlerResult::Continue;
        });
        options.SetCanBindHandler([&](ECN::ECPropertyCR prop) {
            for (auto handler : handlers) {
                if (handler->m_customHandledProperties.find(prop.GetName()) != handler->m_customHandledProperties.end()) {
                    return false;
                }
            }
            return true;
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
std::vector<IClassHandler*>& InstanceRepository::TryGetHandlers(ECN::ECClassId classId, JsFormat fmt, Operation operation, BeJsConst& instance, BeJsConst& userOptions)
    const {
    auto& handlers = TryGetHandlers(classId);
    for (auto handler : handlers) {
        handler->SetContext(m_ecdb, instance, userOptions, fmt, operation, m_cache);
    }
    return handlers;
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

    auto& handlers = TryGetHandlers(instKey.GetClassId(), inFmt, Operation::Update, in, userOptions);
    if (!handlers.empty()) {

        options.SetCustomBindHandler([&](ECN::ECPropertyCR prop, IECSqlBinder& binder, BeJsConst instance, BeJsConst val, ECSqlStatus& rc) {
            for (auto handler : handlers) {
                auto status = handler->OnBindECProperty(prop, val, binder, rc);
                if (status == PropertyHandlerResult::Handled) {
                    return PropertyHandlerResult::Handled;
                }
            }
            return PropertyHandlerResult::Continue;
        });
        options.SetUserPropertyHandler([&](Utf8CP name, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) {
            for (auto handler : handlers) {
                auto result = handler->OnBindUserProperty(name, val, finder, rc);
                if (result == PropertyHandlerResult::Handled) {
                    return PropertyHandlerResult::Handled;
                }
            }
            return PropertyHandlerResult::Continue;
        });
        options.SetCanBindHandler([&](ECN::ECPropertyCR prop) {
            for (auto handler : handlers) {
                if (handler->m_customHandledProperties.find(prop.GetName()) != handler->m_customHandledProperties.end()) {
                    return false;
                }
            }
            return true;
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
    auto nullValue = BeJsDocument::Null();
    auto& handlers = TryGetHandlers(instKey.GetClassId(), fmt, Operation::Read, nullValue, userOptions);
    InstanceReader::Position pos(instKey.GetInstanceId(), instKey.GetClassId());
    InstanceReader::Options options;
    options.SetForceSeek(true);
    auto rc = BE_SQLITE_ROW;
    if (!m_reader.Seek(pos, [&](const InstanceReader::IRowContext& row, PropertyReader::Finder finder) {
            ECSqlRowAdaptor adaptor(m_ecdb);
            adaptor.GetOptions().SetAbbreviateBlobs(false);
            adaptor.GetOptions().SetConvertClassIdsToClassNames(fmt == JsFormat::JsName);
            adaptor.GetOptions().SetUseJsNames(fmt == JsFormat::JsName);
            adaptor.GetOptions().SetUseClassFullNameInsteadofClassName(fmt == JsFormat::JsName);
            if (!handlers.empty()) {
                adaptor.SetCustomHandler([&](BeJsValue out, IECSqlValue const& val) {
                    ECSqlStatus status = ECSqlStatus::Success;
                    for (auto handler : handlers) {
                        auto result = handler->OnReadECProperty(*val.GetColumnInfo().GetProperty(), val, out, status);
                        if (result == PropertyHandlerResult::Handled) {
                            return PropertyHandlerResult::Handled;
                        }
                    }
                    return PropertyHandlerResult::Continue;
                });
                adaptor.SetSkipPropertyHandler([&](ECPropertyCR prop) {
                    for (auto handler : handlers) {
                        if (handler->m_customHandledProperties.find(prop.GetName()) != handler->m_customHandledProperties.end())
                            return true;
                    }
                    return false;
                });
            }

            if (ERROR == adaptor.RenderRowAsObject(outInstance, row)) {
                rc = BE_SQLITE_ERROR;
            }

            ECSqlStatus status = ECSqlStatus::Success;
            for (auto handler : handlers) {
                status = handler->OnReadComplete(outInstance, finder);
                if (!status.IsSuccess()) {
                    rc = BE_SQLITE_ERROR;
                    break;
                }
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
CachedECSqlStatementPtr InstanceRepository::IClassHandler::GetPreparedStatement(Utf8CP ecsql) {
    return m_cache->GetPreparedStatement(*m_db, ecsql);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECInstanceKey InstanceRepository::IClassHandler::ParseInstanceKey() const {
    if (GetInstance().isNull()) {
        return ECInstanceKey();
    }
    ECInstanceKey key;
    GetDb<ECDb>().GetInstanceWriter().TryGetInstanceKey(key, GetInstance(), GetFormat());
    return key;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECClassId InstanceRepository::IClassHandler::ParseClassId() const {
    if (GetInstance().isNull()) {
        return ECClassId();
    }
    ECClassId key;
    GetDb<ECDb>().GetInstanceWriter().TryGetClassId(key, GetInstance(), GetFormat());
    return key;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECInstanceId InstanceRepository::IClassHandler::ParseInstanceId() const {
    if (GetInstance().isNull()) {
        return ECInstanceId();
    }
    ECInstanceId key;
    GetDb<ECDb>().GetInstanceWriter().TryGetId(key, GetInstance(), GetFormat());
    return key;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InstanceRepository::IClassHandler::SetError(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    m_error.VSprintf(fmt, args);
    va_end(args);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InstanceRepository::IClassHandler::SetContext(ECDbCR db, BeJsConst& instance, BeJsConst& userOptions, JsFormat fmt, Operation operation, ECSqlStatementCache& cache) {
    m_db = &db;
    m_error.clear();
    m_instance = &instance;
    m_userOptions = &userOptions;
    format = fmt;
    m_operation = operation;
    if (!m_class) {
        m_class = db.Schemas().GetClass(m_classId);
    }
    m_cache = &cache;
}

END_BENTLEY_SQLITE_EC_NAMESPACE