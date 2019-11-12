/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/ChunkedWSChangeset.h>
#include <WebServices/Client/WSChangeset.h>

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          03/2018
//---------------------------------------------------------------------------------------
bool ChunkedWSChangeset::AddInstance()
    {
    if (m_currentChangesetSize < s_changesetInstancesLimit)
        {
        m_currentChangesetSize++;
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas          03/2018
//---------------------------------------------------------------------------------------
void ChunkedWSChangeset::StartNewChangeset()
    {
    m_currentChangeset = std::make_shared<WSChangeset>();
    m_changesets.push_back(m_currentChangeset);
    m_currentChangesetSize = 1;
    }
