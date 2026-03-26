/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using Stage = Changes::Change::Stage;

//=======================================================================================
// ECChangesetReader public API — delegates to Impl
//+===============+===============+===============+===============+===============+======

ECChangesetReader::ECChangesetReader() : m_pimpl(new Impl()) {}

ECChangesetReader::~ECChangesetReader() { delete m_pimpl; }

ECChangesetReader::ECChangesetReader(ECChangesetReader&& rhs) : m_pimpl(rhs.m_pimpl) {
    rhs.m_pimpl = nullptr;
}

ECChangesetReader& ECChangesetReader::operator=(ECChangesetReader&& rhs) {
    if (this != &rhs) {
        delete m_pimpl;
        m_pimpl = rhs.m_pimpl;
        rhs.m_pimpl = nullptr;
    }
    return *this;
}

void ECChangesetReader::OpenFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert) {
    m_pimpl->OpenFile(ecdb, changesetFile, invert);
}

void ECChangesetReader::OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert) {
    m_pimpl->OpenChangeStream(ecdb, std::move(changeStream), invert);
}

void ECChangesetReader::OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, Db const& db, bool invert) {
    m_pimpl->OpenGroup(ecdb, changesetFiles, db, invert);
}

void ECChangesetReader::Close() { m_pimpl->Close(); }
DbResult ECChangesetReader::Step() { return m_pimpl->Step(); }

IECSqlValue const& ECChangesetReader::GetValue(Stage stage, int columnIndex) const {
    return m_pimpl->GetValue(stage, columnIndex);
}

END_BENTLEY_SQLITE_EC_NAMESPACE