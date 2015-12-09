/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerRevision.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerRevision.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

DgnDbServerRevision::DgnDbServerRevision(DgnRevisionPtr revision)
    : m_revision(revision)
    {}

DgnDbServerRevisionPtr DgnDbServerRevision::Create(DgnRevisionPtr revision)
    {
    return DgnDbServerRevisionPtr(new DgnDbServerRevision(revision));
    }

DgnRevisionPtr DgnDbServerRevision::GetRevision()
    {
    return m_revision;
    }

int64_t DgnDbServerRevision::GetIndex()
    {
    return m_index;
    }

Utf8StringCR DgnDbServerRevision::GetURL()
    {
    return m_url;
    }

void DgnDbServerRevision::SetIndex(int64_t index)
    {
    m_index = index;
    }

void DgnDbServerRevision::SetURL(Utf8StringCR url)
    {
    m_url = url;
    }
