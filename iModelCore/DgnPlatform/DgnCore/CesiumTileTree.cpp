/*-------------------------------------------------------------------------------------+                                                                                           
|

|     $Source: DgnCore/CesiumTileTree.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/CesiumTileTree.h>
#include <BeHttp/HttpClient.h>
#include <folly/BeFolly.h>

BEGIN_DGN_CESIUM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnDbR db, TransformCR location, Utf8CP rootResource, OutputR output) : m_db(db), m_location(location), m_output(&output)
    {
    // unless a root directory is specified, we assume it's http.
    m_isHttp = true;

    if (nullptr == rootResource)
        return;

    m_isHttp = (0 == strncmp("http:", rootResource, 5) || 0 == strncmp("https:", rootResource, 6));

    m_rootResource.assign (rootResource);

    if (m_isHttp)
        {
        m_rootResource = m_rootResource.substr(0, m_rootResource.find_last_of("/"));
        }
    else if (!m_rootResource.empty())
        {
        BeFileName rootDirectory(BeFileName::DevAndDir, BeFileName(m_rootResource));
        BeFileName::FixPathName(rootDirectory, rootDirectory, false);
        m_rootResource = rootDirectory.GetNameUtf8();
        m_isHttp = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::StartTileLoad(LoadStateR state) const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    BeAssert(m_activeLoads.end() == m_activeLoads.find(&state));
    m_activeLoads.insert(&state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::DoneTileLoad(LoadStateR state) const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    BeAssert(m_activeLoads.end() != m_activeLoads.find(&state));
    m_activeLoads.erase(&state);
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* This method gets called on the (valid) children of nodes as they are unloaded. Its purpose is to notify the loading
* threads that these nodes are no longer referenced and we shouldn't waste time loading them.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::SetAbandoned()
    {
    auto children = _GetChildren(false);
    if (nullptr != children)
        for (auto const& child : *children)
            child->SetAbandoned();

    SetLoadStatus(LoadStatus::Abandoned);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::ClearAllTiles()
    {
    if (!m_rootTile.IsValid())
        return;

    m_rootTile->SetAbandoned();
    WaitForAllLoads();

    m_rootTile = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::CancelTileLoad(TileCR tile)
    {
    // Bentley containers don't support 'transparent' comparators, meaning we can't compare a TileLoadStatePtr to a Tile even
    // though the comparator can. We should fix that - but for now, instead, we're using std::set.
    BeMutexHolder lock(m_cv.GetMutex());
    auto iter = m_activeLoads.find(&tile);
    if (iter != m_activeLoads.end())
        {
        (*iter)->SetCanceled();
        m_activeLoads.erase(iter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::CancelAllTileLoads()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    for (auto& load : m_activeLoads)
        load->SetCanceled();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Root::_RequestTile(TileR tile, LoadStateR loads)
    {
    if (!tile.IsNotLoaded()) // this should only be called when the tile is in the "not loaded" state.
        {
        BeAssert(false);
        return ERROR;
        }

    auto loader = tile._CreateLoader(loads);
    if (loader.IsNull())
        return ERROR;   
    
    return loader->Perform();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> Loader::_GetFromSource()
    {
    if (m_tile->GetRoot().IsHttp())
        {
        auto query = std::make_shared<HttpDataQuery>(m_resourceName, *m_loads);

        LoaderPtr me(this);
        return query->Perform().then([me, query] (Http::Response const& response)
            {
            if (Http::ConnectionStatus::OK != response.GetConnectionStatus() || Http::HttpStatus::OK != response.GetHttpStatus())
                return ERROR;

            if (!query->m_responseBody->GetByteStream().HasData())
                return ERROR;

            me->m_tileBytes = StreamBuffer(std::move(query->m_responseBody->GetByteStream())); // NEEDSWORK this is a copy not a move...
            me->m_contentType = response.GetHeaders().GetContentType();

            return SUCCESS;
            });
        }                                           
    else
        {
        auto query = std::make_shared<FileDataQuery>(m_resourceName, *m_loads);
     
        LoaderPtr me(this);
        return query->Perform().then([me, query](ByteStream const& data)
            {
            if (!data.HasData())
                return ERROR;

            me->m_tileBytes = StreamBuffer(std::move(data)); // NEEDSWORK this is a copy not a move...
            me->m_contentType = "";     // unknown 

            return SUCCESS;
            });         
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016                                        
//----------------------------------------------------------------------------------------
BentleyStatus Loader::LoadTile()
    {
    // During the read we may have abandoned the tile. Do not waste time loading it.
    if (IsCanceledOrAbandoned())
        return ERROR;

    BeAssert(m_tile->IsQueued());
    return _LoadTile();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> Loader::Perform()
    {
    m_tile->SetIsQueued(); // mark as queued so we don't request it again.

    LoaderPtr me(this);
    auto loadFlag = std::make_shared<LoadFlag>(*m_loads);
    return me->_GetFromSource().then(&BeFolly::ThreadPool::GetCpuPool(), [me, loadFlag](BentleyStatus status)
        {
        DgnDb::SetThreadId(DgnDb::ThreadId::CpuPool);
        auto& tile = *me->m_tile;
        if (SUCCESS != status || SUCCESS != me->LoadTile())
            {
            if (me->m_loads->IsCanceled())
                tile.SetNotLoaded();    // Mark it as not loaded so we can retry again.
            else
                tile.SetNotFound();

            return ERROR;
            }

        tile.SetIsReady();  // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.
        return SUCCESS;
        });
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
HttpDataQuery::HttpDataQuery(Utf8StringCR url, LoadStateR loads) : m_request(url), m_loads(&loads), m_responseBody(Http::HttpByteStreamBody::Create())
    {
    m_request.SetResponseBody(m_responseBody);
    m_request.SetRetryOptions(Http::Request::RetryOption::ResetTransfer, 1);
    m_request.SetCancellationToken(loads.GetCancellationToken());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<Http::Response> HttpDataQuery::Perform()
    {
    LoadStatePtr loads(m_loads);
    return GetRequest().Perform().then([loads] (Http::Response response)
        {
        return response;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ByteStream> FileDataQuery::Perform()
    {
    auto filename = m_fileName;
    LoadStatePtr loads(m_loads);
    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [filename, loads] ()
        {
        ByteStream data;
        BeFile dataFile;
        if (BeFileStatus::Success != dataFile.Open(filename.c_str(), BeFileAccess::Read))
            return data;

        if (BeFileStatus::Success != dataFile.ReadEntireFile(data))
            return data;

        return data;
        });
    }

END_DGN_CESIUM_NAMESPACE
