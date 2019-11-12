/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Client/WSChangeset.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

typedef std::shared_ptr<WSChangeset> WSChangesetPtr;
typedef bvector<WSChangesetPtr> WSChangesets;

//=======================================================================================
//! WSChangeset that splits locks and codes requests into defined size chunks.
//@bsiclass                                      Algirdas.Mikoliunas            03/2018
//=======================================================================================
struct ChunkedWSChangeset : RefCountedBase
{
private:
    WSChangesets          m_changesets;
    WSChangesetPtr        m_currentChangeset;
    uint64_t              m_currentChangesetSize; 
    static const uint64_t s_changesetInstancesLimit = 5000;
    
public:
    ChunkedWSChangeset() : m_currentChangesetSize(0), m_currentChangeset(std::make_shared<WSChangeset>()) 
        { m_changesets.push_back(m_currentChangeset); }

    WSChangesetPtr GetCurrentChangeset() const { return m_currentChangeset; }
    WSChangesets GetChunks() const { return m_changesets; }

    //! Marks that single instance will be added to changeset.
    //! @return Returns false if current changeset is full and new one should be used.
    IMODELHUBCLIENT_EXPORT bool AddInstance();

    //! Starts using new changeset.
    IMODELHUBCLIENT_EXPORT void StartNewChangeset();
};
END_BENTLEY_IMODELHUB_NAMESPACE
