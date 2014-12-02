/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RealityDataCache.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#ifndef BENTLEY_WINRT
    #include <curl/curl.h>
    #ifdef GetCurrentTime
        #undef GetCurrentTime
    #endif
#endif

DPILOG_DEFINE(RealityDataCache)
#define LOG(sev,...) {if (RealityDataCache_getLogger().isSeverityEnabled(sev)) { RealityDataCache_getLogger().messagev (sev, __VA_ARGS__); }}

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*======================================================================================+
|   RealityDataQueue
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void RealityDataQueue<T>::Push (T const& element)
    {
    BeCriticalSectionHolder lock (m_cv.GetCriticalSection ());
    auto item = Item::Create (element, m_last.get (), NULL);
    if (m_last.IsNull ())
        {
        BeAssert (m_first.IsNull ());
        m_first = m_last = item;
        }
    else
        {
        m_last->next = item;
        m_last = item;
        }
    m_cv.Wake (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool RealityDataQueue<T>::Pop (T& element)
    {
    BeCriticalSectionHolder lock (m_cv.GetCriticalSection ());
    if (m_first.IsNull ())
        return false;

    element = m_first->data;
    
    if (m_first.Equals (m_last))
        {
        m_first = m_last = NULL;
        }
    else
        {
        m_first->next->prev = NULL;
        m_first = m_first->next;
        }

    m_cv.Wake (true);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void RealityDataQueue<T>::Clear ()
    {
    BeCriticalSectionHolder lock (m_cv.GetCriticalSection ());
    auto item = m_first;
    while (item.IsValid ())
        {
        auto temp = item;
        item = item->next;
        temp->next = NULL;
        temp->prev = NULL;
        }
    m_first = NULL;
    m_last = NULL;
    m_cv.Wake (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool RealityDataQueue<T>::IsEmpty () const
    {
    BeCriticalSectionHolder lock (m_cv.GetCriticalSection ());
    return m_first.IsNull ();
    }

// explicitly implement for testing purposes, 
// note: must be done AFTER all template functions are defined
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
template struct RealityDataQueue<int>;
END_BENTLEY_DGNPLATFORM_NAMESPACE
    
/*======================================================================================+
|   IRealityData
+======================================================================================*/
IRealityDataPtr IRealityData::Create (RealityDataType type)
    {
    switch (type)
        {
        case RealityDataType::TiledRaster:
            return TiledRaster::Create ();
        }

    BeAssert (false);
    return NULL;
    }

/*======================================================================================+
|   TiledRaster
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static DateTime GetExpirationDateFromCacheControlStr (Utf8CP str)
    {
    int maxage = 0, smaxage = 0;
    int start = 0;
    int i = 0;
    while (true)
        {
        if (str[i] == ',' || str[i] == '\0')
            {
            // means we passed an option
            Utf8String option (str + start, i - start);
            option.Trim ();

            if (option.Equals ("private") || option.Equals ("no-cache") || option.Equals ("no-store"))
                return DateTime::GetCurrentTime (); 
            
            if (option.length () >= 7 && 0 == strncmp ("max-age", option.c_str (), 7))
                {
                Utf8CP value = option.c_str () + 8;
                sscanf (value, "%d", &maxage);
                }

            if (option.length () >= 8 && 0 == strncmp ("s-maxage", option.c_str (), 8))
                {
                Utf8CP value = option.c_str () + 9;
                sscanf (value, "%d", &smaxage);
                }

            if (str[i] == '\0')
                break;

            start = i + 1;
            }
        i++;
        }

    Int64 currentTime;
    DateTime::GetCurrentTime ().ToUnixMilliseconds (currentTime);

    DateTime expirationDate;
    DateTime::FromUnixMilliseconds (expirationDate, currentTime + 1000 * (0 != smaxage ? smaxage : maxage));
    return expirationDate;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetAnsiCFormattedDateTime (DateTime const& dateTime)
    {
    Utf8CP wkday = "";
    switch (dateTime.GetDayOfWeek ())
        {
        case DateTime::DayOfWeek::Sunday:   wkday = "Sun"; break;
        case DateTime::DayOfWeek::Monday:   wkday = "Mon"; break;
        case DateTime::DayOfWeek::Tuesday:  wkday = "Tue"; break;
        case DateTime::DayOfWeek::Wednesday:wkday = "Wed"; break;
        case DateTime::DayOfWeek::Thursday: wkday = "Thu"; break;
        case DateTime::DayOfWeek::Friday:   wkday = "Fri"; break;
        case DateTime::DayOfWeek::Saturday: wkday = "Sat"; break;
        }

    Utf8CP month = "";
    switch (dateTime.GetMonth ())
        {
        case 1:  month = "Jan"; break;
        case 2:  month = "Feb"; break;
        case 3:  month = "Mar"; break;
        case 4:  month = "Apr"; break;
        case 5:  month = "May"; break;
        case 6:  month = "Jun"; break;
        case 7:  month = "Jul"; break;
        case 8:  month = "Aug"; break;
        case 9:  month = "Sep"; break;
        case 10: month = "Oct"; break;
        case 11: month = "Nov"; break;
        case 12: month = "Dec"; break;
        }

    return Utf8PrintfString ("%s %s %02d %02d:%02d:%02d %d", wkday, month, dateTime.GetDay (), dateTime.GetHour (), dateTime.GetMinute (), dateTime.GetSecond (), dateTime.GetYear ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::InitFrom (Utf8CP key, bmap<Utf8String, Utf8String> const& header, bvector<byte> const& body, IRealityData::RequestOptions const& requestOptions)
    {
    auto options = dynamic_cast<RequestOptions const*> (&requestOptions);
    BeAssert (NULL != options);

    m_url.AssignOrClear (key);
    m_creationDate = DateTime::GetCurrentTime ();

    auto cacheControlIter = header.find ("Cache-Control");
    m_expirationDate = (cacheControlIter != header.end ()) ? GetExpirationDateFromCacheControlStr (cacheControlIter->second.c_str ()) : DateTime::GetCurrentTime ();

    auto etagIter = header.find ("ETag");
    if (etagIter != header.end ())
        m_entityTag.AssignOrClear (etagIter->second.c_str ());
    
    auto contentTypeIter = header.find ("Content-Type");
    if (contentTypeIter == header.end ())
        return ERROR;

    m_contentType = contentTypeIter->second.c_str ();
    m_rasterInfo = options->ExpectedImageInfo();
    m_data = body;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::UpdateFrom (bmap<Utf8String, Utf8String> const& header)
    {
    auto cacheControlIter = header.find ("Cache-Control");
    m_expirationDate = (cacheControlIter != header.end ()) ? GetExpirationDateFromCacheControlStr (cacheControlIter->second.c_str ()) : DateTime::GetCurrentTime ();

    auto etagIter = header.find ("ETag");
    if (etagIter != header.end ())
        m_entityTag.AssignOrClear (etagIter->second.c_str ());
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledRaster::_ProvideHttpRequestOptions (bmap<Utf8String, Utf8String>& options)
    {
    BeAssert (DateTime::Kind::Utc == GetExpirationDate ().GetInfo ().GetKind ());
    options["If-Modified-Since"] = GetAnsiCFormattedDateTime (GetExpirationDate ());

    if (!Utf8String::IsNullOrEmpty (GetEntityTag ()))
        options["If-None-Match"] = GetEntityTag ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::InitFrom (BeSQLite::Db& db, Utf8CP key)
    {
    HighPriorityOperationBlock highPriority;

    CachedStatementPtr stmt;
    if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement (stmt, "SELECT Raster, RasterSize, RasterInfo, Created, Expires, ETag, ContentType from " TABLE_NAME_TiledRaster " WHERE Url=?"))
        return ERROR;

    stmt->ClearBindings();
    stmt->BindText (1, key, BeSQLite::Statement::MAKE_COPY_Yes);
    if (BeSQLite::BE_SQLITE_ROW == stmt->Step ())
        {
        m_url = key;

        auto raster     = stmt->GetValueBlob (0);
        auto rasterSize = stmt->GetValueInt (1);
        m_data.assign ((byte*) raster, (byte*) raster + rasterSize);

        m_rasterInfo = DeserializeRasterInfo (stmt->GetValueText (2));
        DateTime::FromUnixMilliseconds (m_creationDate, (UInt64) stmt->GetValueInt64 (3));
        DateTime::FromUnixMilliseconds (m_expirationDate, (UInt64) stmt->GetValueInt64 (4));
        m_entityTag = stmt->GetValueText (5);
        m_contentType = stmt->GetValueText (6);

        return SUCCESS;
        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::InitializeDb (BeSQLite::Db& db)
    {
    if (db.TableExists (TABLE_NAME_TiledRaster))
        return SUCCESS;
    
    Utf8CP ddl = "Url CHAR PRIMARY KEY, \
                  Raster BLOB,          \
                  RasterSize INT,       \
                  RasterInfo CHAR,      \
                  ContentType CHAR,     \
                  Created BIGINT,       \
                  Expires BIGINT,       \
                  ETag CHAR";
    return BeSQLite::BE_SQLITE_OK == db.CreateTable (TABLE_NAME_TiledRaster, ddl) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::Persist (BeSQLite::Db& db) const
    {
    int bufferSize = (int) GetData ().size ();

    Int64 creationTime = 0;
    if (SUCCESS != GetCreationDate ().ToUnixMilliseconds (creationTime))
        return ERROR;

    Int64 expirationTime = 0;
    if (SUCCESS != GetExpirationDate ().ToUnixMilliseconds (expirationTime))
        return ERROR;

    HighPriorityOperationBlock highPriority;

    CachedStatementPtr selectStatement;
    if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement (selectStatement, "SELECT Url from " TABLE_NAME_TiledRaster " WHERE Url=?"))
        return ERROR;

    selectStatement->ClearBindings ();
    selectStatement->BindText (1, GetKey (), BeSQLite::Statement::MAKE_COPY_Yes);
    if (BeSQLite::BE_SQLITE_ROW == selectStatement->Step ())
        {
        // update
        CachedStatementPtr stmt;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement (stmt, "UPDATE " TABLE_NAME_TiledRaster " SET Expires=?, ETag=? WHERE Url=?"))
            return ERROR;

        stmt->ClearBindings ();
        stmt->BindInt64 (1, expirationTime);
        stmt->BindText  (2, GetEntityTag (), BeSQLite::Statement::MAKE_COPY_Yes);
        stmt->BindText  (3, GetKey (), BeSQLite::Statement::MAKE_COPY_Yes);
        if (BeSQLite::BE_SQLITE_DONE != stmt->Step ())
            return ERROR;
        }
    else
        {
        // insert
        CachedStatementPtr stmt;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement (stmt, "INSERT INTO " TABLE_NAME_TiledRaster " (Url, Raster, RasterSize, RasterInfo, Created, Expires, ETag, ContentType) VALUES (?,?,?,?,?,?,?,?)"))
            return ERROR;

        stmt->ClearBindings ();
        stmt->BindText  (1, GetKey (), BeSQLite::Statement::MAKE_COPY_Yes);
        stmt->BindBlob  (2, GetData ().data (), bufferSize, BeSQLite::Statement::MAKE_COPY_No);
        stmt->BindInt   (3, bufferSize);
        stmt->BindText  (4, SerializeRasterInfo (m_rasterInfo).c_str (), BeSQLite::Statement::MAKE_COPY_Yes);
        stmt->BindInt64 (5, creationTime);
        stmt->BindInt64 (6, expirationTime);
        stmt->BindText  (7, GetEntityTag (), BeSQLite::Statement::MAKE_COPY_Yes);
        stmt->BindText  (8, GetContentType (), BeSQLite::Statement::MAKE_COPY_Yes);
        if (BeSQLite::BE_SQLITE_DONE != stmt->Step ())
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TiledRaster::SerializeRasterInfo (ImageUtilities::RgbImageInfo const& info)
    {
    Json::Value json;
    json["hasAlpha"] = info.hasAlpha;
    json["height"] = info.height;
    json["width"] = info.width;
    json["isBGR"] = info.isBGR;
    json["isTopDown"] = info.isTopDown;
    return Json::FastWriter::ToString (json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageUtilities::RgbImageInfo TiledRaster::DeserializeRasterInfo (Utf8CP serializedJson)
    {
    Json::Value json;
    Json::Reader reader;
    reader.parse (serializedJson, json);

    ImageUtilities::RgbImageInfo info;
    info.hasAlpha = json["hasAlpha"].asBool ();
    info.height = json["height"].asInt ();
    info.width = json["width"].asInt ();
    info.isBGR = json["isBGR"].asBool ();
    info.isTopDown = json["isTopDown"].asBool ();
    return info;
    }

/*======================================================================================+
|   BeSQLiteRealityDataStorage
+======================================================================================*/
//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct WorkerQueueNotEmptyPredicate : IConditionVariablePredicate
    {
    RealityDataQueue<RealityDataWorkPtr> const& m_queue;
    WorkerQueueNotEmptyPredicate (RealityDataQueue<RealityDataWorkPtr> const& queue) : m_queue (queue) {}
    virtual bool _TestCondition (BeConditionVariable& cv) override 
        {
        BeCriticalSectionHolder lock (cv.GetCriticalSection ());
        return !m_queue.IsEmpty ();
        }
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct SelectDataWorkHasResultPredicate : IConditionVariablePredicate
    {
    bool const& m_hasResult;
    SelectDataWorkHasResultPredicate (bool const& hasResult) : m_hasResult (hasResult) {}
    virtual bool _TestCondition (BeConditionVariable& cv) override 
        {
        BeCriticalSectionHolder lock (cv.GetCriticalSection ());
        return m_hasResult;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::SaveChangesWork::_DoWork ()
    {
    auto& queue = m_storage->m_worker->m_workQueue;
    WorkerQueueNotEmptyPredicate predicate (queue);
    if (queue.GetConditionVariable ().WaitOnCondition (&predicate, m_idleTime))
        return;

    m_storage->wt_SaveChanges ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::PersistDataWork::_DoWork ()
    {
    m_storage->wt_Persist (*m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::SelectDataWork::_DoWork ()
    {
    BeCriticalSectionHolder lock (m_resultCV.GetCriticalSection ());
    m_result = m_storage->wt_Select (m_type, m_id, m_data);
    m_hasResult = true;
    m_resultCV.Wake (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::SelectDataWork::GetResult () const
    {
    SelectDataWorkHasResultPredicate predicate (m_hasResult);
    m_resultCV.WaitOnCondition (&predicate, BeConditionVariable::Infinite);
    return m_result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLiteRealityDataStorage::~BeSQLiteRealityDataStorage ()
    {
    m_worker->Terminate ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::wt_Initialize ()
    {
    if (m_initialized)
        return;

    if (BeSQLite::BE_SQLITE_OK != OpenBeSQLiteDb (m_filename, Db::OpenParams (Db::OpenMode::OPEN_ReadWrite)))
        {
        if (BeSQLite::BE_SQLITE_OK != CreateNewDb (m_filename))
            {
            LOG (LOG_ERROR, "%s: %s", Utf8String(m_filename).c_str(), GetLastError());
            BeAssert (false);
            return;
            }
        }

    auto result = TiledRaster::InitializeDb (*this);
    BeAssert (SUCCESS == result);

    if (BeSQLite::BE_SQLITE_OK != SaveChanges ())
        {
        LOG (LOG_ERROR, "%s: %s", Utf8String(m_filename).c_str(), GetLastError());
        BeAssert (false);
        return;
        }

    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::wt_SaveChanges ()
    {
    wt_Initialize ();
    auto result = SaveChanges ();
    BeAssert (BeSQLite::BE_SQLITE_OK == result);
    m_hasChanges = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::wt_Select (RealityDataType type, Utf8CP id, IRealityDataPtr& data)
    {
    wt_Initialize ();

    auto realityData = IRealityData::Create (type);
    if (realityData.IsNull ())
        return RealityDataStorageResult::Error;

    if (SUCCESS != realityData->InitFrom (*this, id))
        return RealityDataStorageResult::NotFound;

    data = realityData;
    if (realityData->IsExpired ())
        return RealityDataStorageResult::Expired;

    return RealityDataStorageResult::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataStorageResult BeSQLiteRealityDataStorage::Select (IRealityDataPtr& data, RealityDataType type, Utf8CP id)
    {
    auto work = SelectDataWork::Create (*this, type, id, data);
    m_worker->DoWork (*work);
    return work->GetResult ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::wt_Persist (IRealityData const& data)
    {
    wt_Initialize ();
    data.Persist (*this);
    m_hasChanges = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeSQLiteRealityDataStorage::Persist (IRealityData const& data)
    {
    m_worker->DoWork (*PersistDataWork::Create (*this, data));
    return SUCCESS;
    }

/*======================================================================================+
|   BeSQLiteRealityDataStorage::WorkerThread
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::WorkerThread::_DoWork (RealityDataWork& work)
    {
    if (IsBusy ())
        m_workQueue.Push (&work);
    else
        RealityDataWorkerThread::_DoWork (work);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::WorkerThread::_OnIdle ()
    {
    RealityDataWorkPtr work;
    if (m_workQueue.Pop (work))
        {
        DoWork (*work);
        return;
        }

    RealityDataWorkerThread::_OnIdle ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteRealityDataStorage::WorkerThreadIdleTracker::_OnThreadIdle (RealityDataWorkerThread& thread)
    {
    if (m_storage.m_hasChanges)
        {
        // give 5 seconds of idle time before actually calling SaveChanges
        m_storage.m_worker->DoWork (*SaveChangesWork::Create (m_storage, 5 * 1000));
        }
    }

/*======================================================================================+
|   HttpRealityDataSource
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpRealityDataSource::Initialize ()
    {
#ifndef BENTLEY_WINRT
    if (m_initialized)
        return SUCCESS;

    curl_global_init (CURL_GLOBAL_ALL);
    m_initialized = true;
    return SUCCESS;

#else
    return ERROR;
#endif
    }

#ifndef BENTLEY_WINRT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t HttpBodyParser (void* ptr, size_t size, size_t nmemb, void* userp)
    {
    size_t totalSize = size * nmemb;
    auto buffer = (bvector<byte>*) userp;
    buffer->insert (buffer->end (), (byte*) ptr, (byte*) ptr + totalSize);
    return totalSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t HttpHeaderParser (char* buffer, size_t size, size_t nItems, void* userData)
    {
    auto& header = *(bmap<Utf8String, Utf8String>*) userData;
    auto totalSize = size * nItems;

    Utf8String line (buffer, totalSize);
    line.Trim ();
    if (Utf8String::IsNullOrEmpty (line.c_str ()))
        return totalSize;
    
    if (header.empty ())
        {
        // expecting the "HTTP/1.x 200 OK" header
        header["HTTP"] = line;
        }
    else
        {
        auto delimiterPos = line.find (':');
        if (Utf8String::npos == delimiterPos)
            {
            LOG (LOG_INFO, "Malformed HTTP header: %s", line.c_str());
            BeAssert (false);
            return totalSize;
            }
        Utf8String key (line.begin (), line.begin () + delimiterPos);
        Utf8String value (line.begin () + delimiterPos + 2, line.end ());
        header[key] = value;
        }
    
    return totalSize;
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct CurlHolder
{
private:
    CURL* m_curl;

public:
    CurlHolder () : m_curl (curl_easy_init ()) { }
    ~CurlHolder () { curl_easy_cleanup (m_curl); }
    CURL* Get () const { return m_curl; }
};
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRealityDataSource::RequestHandler::_DoWork ()
    {
#ifndef BENTLEY_WINRT
    CurlHolder curl;

    bmap<Utf8String, Utf8String> header;
    bvector<byte> body;
    curl_easy_setopt (curl.Get (), CURLOPT_URL, m_url.c_str ());
    curl_easy_setopt (curl.Get (), CURLOPT_WRITEFUNCTION, HttpBodyParser);
    curl_easy_setopt (curl.Get (), CURLOPT_WRITEDATA, &body);
    curl_easy_setopt (curl.Get (), CURLOPT_HEADERFUNCTION, HttpHeaderParser);
    curl_easy_setopt (curl.Get (), CURLOPT_HEADERDATA, &header);

    curl_slist* httpHeaders = NULL;
    if (m_optionsProvider.IsValid ())
        {
        bmap<Utf8String, Utf8String> options;
        m_source->GetHttpRequestOptions (*m_optionsProvider, options);

        for (auto pair : options)
            httpHeaders = curl_slist_append (httpHeaders, Utf8PrintfString ("%s: %s", pair.first.c_str(), pair.second.c_str()).c_str ());
        }
    curl_easy_setopt (curl.Get (), CURLOPT_HTTPHEADER, httpHeaders);

    LOG (LOG_TRACE, "[%lld] GET %s", (UInt64)BeThreadUtilities::GetCurrentThreadId(), m_url.c_str());

    CURLcode res = curl_easy_perform (curl.Get ());
    curl_slist_free_all (httpHeaders);
    if (CURLE_OK != res)
        {
        switch (res)
            {
            case CURLE_COULDNT_RESOLVE_HOST:
                LOG (LOG_TRACE, "[%lld] CURLE_COULDNT_RESOLVE_HOST - waiting 10 seconds to re-try", (UInt64)BeThreadUtilities::GetCurrentThreadId());
                m_source->SetIgnoreRequests (10 * 1000);
                SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Error_CouldNotResolveHost));
                break;
            case CURLE_COULDNT_CONNECT:
                LOG (LOG_TRACE, "[%lld] CURLE_COULDNT_CONNECT - waiting 10 seconds to re-try", (UInt64)BeThreadUtilities::GetCurrentThreadId());
                m_source->SetIgnoreRequests (10 * 1000);
                SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Error_NoConnection));
                break;
            default:
                LOG (LOG_ERROR, "[%lld] Unkown curl error %d", (UInt64)BeThreadUtilities::GetCurrentThreadId(), res);
                BeAssert (false && "All CURL errors should be handled");
                SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Error_Unknown));
            }
        return;
        }

    long responseCode;
    if (CURLE_OK != curl_easy_getinfo (curl.Get (), CURLINFO_RESPONSE_CODE, &responseCode) || (0 == responseCode))
        {
        BeAssert (false && "All CURL errors should be handled");
        SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Error_Unknown));
        return;
        }

    LOG (LOG_TRACE, "[%lld] ResponseCode=%d", (UInt64)BeThreadUtilities::GetCurrentThreadId(), responseCode);

    switch (responseCode)
        {
        case 200:   // ok
            {
            auto realityData = IRealityData::Create (m_type);
            if (realityData.IsNull ())
                {
                LOG (LOG_INFO, "[%lld] response 200 but NULL data", (UInt64)BeThreadUtilities::GetCurrentThreadId());
                SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Error_Unknown));
                break;
                }

            if (SUCCESS != realityData->InitFrom (m_url.c_str (), header, body, *m_requestOptions))
                {
                LOG (LOG_INFO, "[%lld] response 200 but unable to initialize realityData", (UInt64)BeThreadUtilities::GetCurrentThreadId());
                SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Error_Unknown));
                break;
                }

            SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Success, realityData.get ()));
            break;
            }

        case 304:   // not modified
            SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::NotModified, header));
            break;

        case 404:   // not found
            SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Error_NotFound, header));
            break;

        case 504:   // gateway timeout
            LOG (LOG_TRACE, "504 - waiting 3 seconds to re-try");
            m_source->SetIgnoreRequests (3 * 1000);
            SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Error_GatewayTimeout));
            break;

        default:    // not expected
            //BeAssert (false && "Unhandled HTTP response");
            LOG (LOG_INFO, "Unhandled HTTP response: GET %s -> %d", m_url.c_str(), responseCode);
            SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Error_Unknown));
            break;
        }
#else
    SendResponse (RealityDataSourceResponse (RealityDataSourceResponse::Status::Error_Unknown));
#endif

    BeCriticalSectionHolder lock (m_source->m_requestsCS);
    auto requestIter = m_source->m_activeRequests.find (m_url);
    BeAssert (requestIter != m_source->m_activeRequests.end ());
    m_source->m_activeRequests.erase (requestIter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpRealityDataSource::Request (RealityDataType type, Utf8CP id, IRealityData::RequestOptions const& requestOptions, IHttpRequestOptionsProvider* optionsProvider, IResponseReceiver& responseReceiver)
    {
    if (BeTimeUtilities::GetCurrentTimeAsUnixMillis () < m_ignoreRequestsUntil)
        return ERROR;

    if (true)
        {
        BeCriticalSectionHolder lock (m_requestsCS);
        auto requestIter = m_activeRequests.find (id);
        if (requestIter != m_activeRequests.end ())
            return SUCCESS; // already pending

        m_activeRequests[id] = true;
        }

    if (SUCCESS != Initialize ())
        {
        BeAssert (false);
        return ERROR;
        }

    m_threadPool->QueueWork (*RequestHandler::Create (*this, type, id, requestOptions, optionsProvider, responseReceiver));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRealityDataSource::SetIgnoreRequests (UInt32 ignoreTime)
    {
    m_ignoreRequestsUntil = BeTimeUtilities::GetCurrentTimeAsUnixMillis () + ignoreTime;
    }

/*======================================================================================+
|   RealityDataCache
+======================================================================================*/
//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct RealityDataCache::ResponseReceiver : RefCounted<IRealityDataSource::IResponseReceiver>
{
private:
    RealityDataCache* m_cache;
    BeCriticalSection m_cs;
    ResponseReceiver (RealityDataCache& cache) : m_cache (&cache) { }

protected:
    // IRealityDataSource::IResponseReceiver implementation:
    virtual void _OnResponseReceived (RealityDataType type, Utf8CP id, RealityDataSourceResponse const& response) 
        {
        BeCriticalSectionHolder lock (m_cs);
        if (NULL != m_cache)
            m_cache->_OnResponseReceived (type, id, response);
        }

public:
    static RefCountedPtr<ResponseReceiver> Create (RealityDataCache& cache) { return new ResponseReceiver (cache); }
    void OnCacheDestroyed () { BeCriticalSectionHolder lock (m_cs); m_cache = NULL; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCache::RealityDataCache (IRealityDataStorage& storage, IRealityDataSource& source)
    : m_storage (storage), m_source (source)
    {
    auto responseReceiver = ResponseReceiver::Create (*this);
    m_responseReceiver = responseReceiver.get ();
    m_responseReceiver->AddRef ();

    m_storage.AddRef ();
    m_source.AddRef ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCache::~RealityDataCache ()
    {
    m_responseReceiver->OnCacheDestroyed ();
    m_responseReceiver->Release ();
    m_responseReceiver = NULL;

    m_storage.Release ();
    m_source.Release ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataPtr RealityDataCache::_GetCached (RealityDataType type, Utf8CP id, RealityDataGetCachedOption const& copt)
    {
    IRealityDataPtr data;
    auto result = m_storage.Select (data, type, id);
    if ((RealityDataStorageResult::Success == result) || ((RealityDataStorageResult::Expired == result) && copt.ShouldReturnExpired()))
        return data;

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataPtr RealityDataCache::_Get (RealityDataType type, Utf8CP id, IRealityData::RequestOptions const& options, RealityDataGetCachedOption const& copt)
    {
    IRealityDataPtr data;
    auto result = m_storage.Select (data, type, id);
    switch (result)
        {
        case RealityDataStorageResult::Error:
            BeAssert (false);
            return NULL;

        case RealityDataStorageResult::Expired:
            m_source.Request (type, id, options, data.get (), *m_responseReceiver);   // always request fresh data
            return copt.ShouldReturnExpired()? data: NULL;              // go ahead and return what we have if the caller says it's OK

        case RealityDataStorageResult::NotFound:
            m_source.Request (type, id, options, data.get (), *m_responseReceiver);
            return data;

        case RealityDataStorageResult::Success:
            return data;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataCache::_OnResponseReceived (RealityDataType type, Utf8CP id, RealityDataSourceResponse const& response)
    {
    IRealityDataPtr data;
    switch (response.GetStatus ())
        {
        case RealityDataSourceResponse::Status::Success:
            {
            BeAssert (response.GetRealityData ().IsValid ());
            data = response.GetRealityData ();
            break;
            }

        case RealityDataSourceResponse::Status::NotModified:
            {
            auto result = m_storage.Select (data, type, id);
            BeAssert (RealityDataStorageResult::Expired == result);
            BeAssert (NULL != response.GetHeader ());
            data->UpdateFrom (*response.GetHeader ());
            break;
            }

        case RealityDataSourceResponse::Status::Error_GatewayTimeout:
        case RealityDataSourceResponse::Status::Error_CouldNotResolveHost:
        case RealityDataSourceResponse::Status::Error_NoConnection:
            {
            return;
            }

        case RealityDataSourceResponse::Status::Error_NotFound:
            {
            // This is not that uncommon. Some servers don't support zoomLevels above 19.
            LOG (LOG_TRACE, "Resource not found in source: %s", id);
            return;
            }

        case RealityDataSourceResponse::Status::Error_Unknown:
            {
            //BeAssert (false);
            LOG (LOG_INFO, "%s: Unknown response: %d", id, response.GetStatus ());
            return;
            }

        default:
            BeAssert (false);
            return;
        }
        
    auto result = m_storage.Persist (*data);
    BeAssert (SUCCESS == result);

    // update arrivals
    BeCriticalSectionHolder lock (m_arrivalsCS);
    _Arrivals (data->GetDataType ())->Push (Utf8String (data->GetKey ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StringRealityDataQueuePtr RealityDataCache::_Arrivals (RealityDataType type) 
    {
    BeCriticalSectionHolder lock (m_arrivalsCS);

    auto it = m_arrivals.find (type);
    if (it != m_arrivals.end ())
        return it->second;

    auto queue = RefCountedRealityDataQueue<Utf8String>::Create ();
    m_arrivals[type] = queue;
    return queue;
    }

/*======================================================================================+
|   RealityDataAdmin
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataCache& DgnPlatformLib::Host::RealityDataAdmin::_SupplyCache ()
    {
    BeFileName storageFileName = T_HOST.GetIKnownLocationsAdmin ().GetLocalTempDirectoryBaseName ();
    storageFileName.AppendToPath (L"RealityDataCache.db");
    auto storage = BeSQLiteRealityDataStorage::Create (storageFileName);
    auto source  = HttpRealityDataSource::Create ();
    return *new RealityDataCache (*storage, *source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IRealityDataCache& DgnPlatformLib::Host::RealityDataAdmin::GetCache ()
    {
    if (NULL == m_dataCache)
        m_dataCache = &_SupplyCache ();

    return *m_dataCache;
    }

/*======================================================================================+
|   RealityDataThread
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataThread::RealityDataThread (Utf8CP threadName) 
    : m_threadId (-1), m_threadName (threadName)
    {
    // Don't try to start the new thread here in the constructor. The sub-class vtable has not been set up yet.
    // If the thread actually starts running before this constructor returns, then PlatformThreadRunner will 
    // call this->_Run, and that will still be a pure virtual function. In fact, that's exactly what happens in iOS.
    //BeThreadUtilities::StartNewThread (1024 * 1024, PlatformThreadRunner, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThread::Start ()
    {
    BeThreadUtilities::StartNewThread (1024 * 1024, PlatformThreadRunner, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThread::ThreadRunner (void* arg)
    {
    auto& thread = *(RealityDataThread*) arg;
    thread.AddRef ();
    thread.Run ();
    thread.Release ();
    }

/*---------------------------------------------------------------------------------**//**
* Runs on its own thread.
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThread::Run ()
    {
    m_threadId = BeThreadUtilities::GetCurrentThreadId ();

    if (!m_threadName.empty ())
        BeThreadUtilities::SetCurrentThreadName (m_threadName.c_str ());

    _Run ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
intptr_t RealityDataThread::GetThreadId () const
    {
    return m_threadId;
    }

/*======================================================================================+
|   RealityDataThreadPool
+======================================================================================*/
//#define DEBUG_THREADPOOL 1
#ifdef DEBUG_THREADPOOL
    struct DebugTimer
        {
        Utf8String  m_msg;
        UInt64      m_timeBefore;
        DebugTimer (Utf8CP msg) : m_msg (msg), m_timeBefore (BeTimeUtilities::GetCurrentTimeAsUnixMillis ()) {}
        ~DebugTimer () { BeDebugLog (Utf8PrintfString ("%s took %llu ms", m_msg.c_str (), (BeTimeUtilities::GetCurrentTimeAsUnixMillis () - m_timeBefore)).c_str ()); }
        };
    #define THREADPOOL_MSG(msg)     BeDebugLog (Utf8PrintfString ("\t %llu\t ThreadPool\t %lld\t %s", BeTimeUtilities::GetCurrentTimeAsUnixMillis (), (Int64) BeThreadUtilities::GetCurrentThreadId (), msg).c_str ());
    #define THREADPOOL_TIMER(msg)   DebugTimer _debugtimer (msg);
    
#else
    #define THREADPOOL_MSG(msg) 
    #define THREADPOOL_TIMER(msg)
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThreadPool::QueueWork (RealityDataWork& work)
    {
    THREADPOOL_TIMER ("QueueWork")
    THREADPOOL_MSG ("QueueWork")

    BeCriticalSectionHolder lock (m_workQueueCS);
    auto thread = GetIdleThread ();
    if (thread.IsNull ())
        {
        THREADPOOL_MSG ("QueueWork: No idle threads")
        if (!ShouldCreateNewThread ())
            {
            m_workQueue.Push (&work);
            THREADPOOL_MSG ("QueueWork: Added work item to queue")
            return;
            }
        thread = CreateThread ();
        THREADPOOL_MSG ("QueueWork: Created a new thread")
        }
    thread->DoWork (work);
    THREADPOOL_MSG ("QueueWork: Sent work item to thread")
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataThreadPool::~RealityDataThreadPool ()
    {
    BeCriticalSectionHolder threadsLock (m_threadsCS);
    for (auto pair : m_threads)
        {
        pair.first->Terminate ();
        pair.first->Release ();
        }
    m_threads.clear ();
    m_threadsCV.Wake (true);

    BeCriticalSectionHolder workQueueLock (m_workQueueCS);
    m_workQueue.Clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataWorkerThreadPtr RealityDataThreadPool::GetIdleThread () const
    {
    BeCriticalSectionHolder lock (m_threadsCS);
    for (auto pair : m_threads)
        {
        if (!pair.second)
            return pair.first;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataWorkerThreadPtr RealityDataThreadPool::CreateThread ()
    {
    auto thread = RealityDataWorkerThread::Create (this, "BentleyThreadPoolWorker");
    thread->AddRef ();
    thread->Start();
    BeCriticalSectionHolder lock (m_threadsCS);
    m_threads[thread.get ()] = false;
    m_threadsCV.Wake (true);
    return thread;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int RealityDataThreadPool::GetThreadsCount () const
    {
    BeCriticalSectionHolder lock (m_threadsCS);
    return (int) m_threads.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RealityDataThreadPool::ShouldCreateNewThread () const
    {
    // do not exceed the "max threads" parameter
    if (GetThreadsCount () >= m_maxThreads)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThreadPool::_OnThreadBusy (RealityDataWorkerThread& thread)
    {
    BeCriticalSectionHolder lock (m_threadsCS);
    THREADPOOL_MSG ("_OnThreadBusy: Marked the thread as busy")
    m_threads[&thread] = true;
    m_threadsCV.Wake (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThreadPool::_OnThreadIdle (RealityDataWorkerThread& thread)
    {
    THREADPOOL_MSG ("_OnThreadIdle")

    RealityDataWorkPtr work;
    if (true)
        {
        BeCriticalSectionHolder lock (m_workQueueCS);
        m_workQueue.Pop (work);
        }

    if (work.IsValid ())
        {
        THREADPOOL_MSG ("_OnThreadIdle: Popped work item from queue, send to idle thread")
        thread.DoWork (*work);
        }
    else
        {
        THREADPOOL_MSG ("_OnThreadIdle: No work items in queue")
        BeCriticalSectionHolder lock (m_threadsCS);
        int idleThreadsCount = 0;
        for (auto pair : m_threads)
            {
            if (!pair.second)
                idleThreadsCount++;
            }
        
        THREADPOOL_MSG (Utf8PrintfString ("_OnThreadIdle: Total idle threads: %d, allowed: %d", idleThreadsCount, m_maxIdleThreads).c_str ())
        BeAssert (idleThreadsCount < m_maxIdleThreads || m_maxIdleThreads == 0);
        if (idleThreadsCount >= m_maxIdleThreads)
            {
            m_threads.erase (&thread);
            thread.Release ();
            thread.Terminate ();
            THREADPOOL_MSG ("_OnThreadIdle: Terminated the thread")
            }
        else
            {
            m_threads[&thread] = false;
            THREADPOOL_MSG ("_OnThreadIdle: Marked the thread as idle")
            }

        m_threadsCV.Wake (true);
        }
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct AllThreadsIdlePredicate : IConditionVariablePredicate
{
private:
    RealityDataThreadPool const& m_pool;
public:
    AllThreadsIdlePredicate (RealityDataThreadPool const& pool) : m_pool (pool) {}
    virtual bool _TestCondition (BeConditionVariable& cv) override 
        {
        BeCriticalSectionHolder lock (m_pool.m_threadsCS);
        for (auto pair : m_pool.m_threads)
            {
            if (pair.second)
                return false;
            }
        return true;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataThreadPool::WaitUntilAllThreadsIdle () const
    {
    AllThreadsIdlePredicate predicate (*this);
    m_threadsCV.WaitOnCondition (&predicate, BeConditionVariable::Infinite);
    }

/*======================================================================================+
|   RealityDataWorkerThread
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataWorkerThread::RealityDataWorkerThread (IStateListener* stateListener, Utf8CP threadName)
    : RealityDataThread (threadName), m_cv (&m_cs), m_terminate (false), m_stateListener (stateListener), m_idleSince (BeTimeUtilities::GetCurrentTimeAsUnixMillis ()), m_busySince (0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::SetIsBusy (bool busy)
    {
    BeCriticalSectionHolder lock (m_cs);
    if (busy)
        {
        m_idleSince = 0;
        m_busySince = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();
        _OnBusy ();
        }
    else
        {
        m_idleSince = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();
        m_busySince = 0;
        _OnIdle ();
        }
    m_cv.Wake (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::_OnBusy ()
    {
    if (m_stateListener.IsValid ())
        m_stateListener->_OnThreadBusy (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::_OnIdle ()
    {
    if (m_stateListener.IsValid ())
        m_stateListener->_OnThreadIdle (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RealityDataWorkerThread::IsBusy (UInt64* busyTime) const
    {
    BeCriticalSectionHolder lock (m_cs);
    if (m_busySince != 0)
        {
        if (NULL != busyTime)
            *busyTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis () - m_busySince;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RealityDataWorkerThread::IsIdle (UInt64* idleTime) const
    {
    BeCriticalSectionHolder lock (m_cs);
    if (m_idleSince != 0)
        {
        if (NULL != idleTime)
            *idleTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis () - m_idleSince;
        return true;
        }
    return false;
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct IsIdlePredicate : IConditionVariablePredicate
{
private:
    RealityDataWorkerThread& m_thread;
public:
    IsIdlePredicate (RealityDataWorkerThread& thread) : m_thread (thread) {}
    virtual bool _TestCondition (BeConditionVariable &cv) override { return m_thread.IsIdle (); }
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct HasWorkOrTerminatesPredicate : IConditionVariablePredicate
{
private:
    RealityDataWorkerThread& m_thread;

public:
    HasWorkOrTerminatesPredicate (RealityDataWorkerThread& thread) : m_thread (thread) {}
    virtual bool _TestCondition (BeConditionVariable &cv) override { return m_thread.TerminateRequested () || m_thread.m_currentWork.IsValid (); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::_DoWork (RealityDataWork& work)
    {
    // note: if m_threadId is -1 it means that the thread hasn't been started yet
    // but that also means that we're not on that thread and should wait for idle, etc.
    if (BeThreadUtilities::GetCurrentThreadId () != GetThreadId ())
        {
        IsIdlePredicate predicate (*this);
        m_cs.Enter ();
        m_cv.ProtectedWaitOnCondition (&predicate, BeConditionVariable::Infinite);
        }

    BeAssert (m_currentWork.IsNull ());
    SetIsBusy (true);
    m_currentWork = &work;
    m_cv.Wake (true);

    if (BeThreadUtilities::GetCurrentThreadId () != GetThreadId ())
        m_cs.Leave ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::_Run ()
    {
    while (!m_terminate)
        {
        HasWorkOrTerminatesPredicate predicate (*this);
        m_cs.Enter ();
        m_cv.ProtectedWaitOnCondition (&predicate, BeConditionVariable::Infinite);

        if (!m_terminate)
            {
            BeAssert (m_currentWork.IsValid ());
            auto work = m_currentWork;
            m_currentWork = NULL;
            m_cs.Leave ();

            work->_DoWork ();
            SetIsBusy (false);
            }
        else
            {
            m_cs.Leave ();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RealityDataWorkerThread::Terminate ()
    {
    BeCriticalSectionHolder lock (m_cs);
    m_terminate = true;
    m_cv.Wake (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool RealityDataWorkerThread::TerminateRequested () const
    {
    BeCriticalSectionHolder lock (m_cs);
    return m_terminate;
    }
