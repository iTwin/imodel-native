/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/ICachingDataSource.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/ICachingDataSource.h>
#include "ICachingDataSource.xliff.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::SelectProvider::SelectProvider() :
m_remoteSelectProvider(std::make_shared<ISelectProvider>()),
m_cacheSelectProvider(std::make_shared<ISelectProvider>())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::SelectProvider::SelectProvider(ISelectProviderPtr provider)
    {
    SetForRemote(provider);
    SetForCache(provider);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ICachingDataSource::SelectProvider::SetForRemote(ISelectProviderPtr provider)
    {
    if (nullptr == provider)
        {
        provider = std::make_shared<ISelectProvider>();
        }
    m_remoteSelectProvider = provider;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ICachingDataSource::SelectProvider::SetForCache(ISelectProviderPtr provider)
    {
    if (nullptr == provider)
        {
        provider = std::make_shared<ISelectProvider>();
        }
    m_cacheSelectProvider = provider;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<const ISelectProvider> ICachingDataSource::SelectProvider::GetForRemote() const
    {
    return m_remoteSelectProvider;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<const ISelectProvider> ICachingDataSource::SelectProvider::GetForCache() const
    {
    return m_cacheSelectProvider;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ISelectProviderPtr ICachingDataSource::SelectProvider::GetForRemote()
    {
    return m_remoteSelectProvider;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ISelectProviderPtr ICachingDataSource::SelectProvider::GetForCache()
    {
    return m_cacheSelectProvider;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Error::Error() :
m_status(Status::Success)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vytenis.Navalinskas    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Error::Error(Status status) :
m_status(status)
    {
    if (status == ICachingDataSource::Status::Canceled)
        {
        return;
        }
    else if (status == ICachingDataSource::Status::InternalCacheError)
        {
        m_message = ICachingDataSourceLocalizedString(ERRORMESSAGE_InternalCache);
        }
    else if (status == ICachingDataSource::Status::DataNotCached)
        {
        m_message = ICachingDataSourceLocalizedString(ERRORMESSAGE_DataNotCached);
        }
    else if (status == ICachingDataSource::Status::FunctionalityNotSupported)
        {
        m_message = ICachingDataSourceLocalizedString(ERRORMESSAGE_FunctionalityNotSupported);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Error::Error(CacheStatus status) : Error(ConvertCacheStatus(status))
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Error::Error(WSErrorCR error) :
AsyncError(error.GetMessage(), error.GetDescription()),
m_status(Status::NetworkErrorsOccured),
m_wsError(error)
    {
    if (WSError::Status::Canceled == m_wsError.GetStatus())
        {
        m_status = Status::Canceled;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Error::Error(AsyncErrorCR error) :
Error(Status::InternalCacheError, error)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Error::Error(ICachingDataSource::Status status, AsyncErrorCR error) :
AsyncError(error),
m_status(status)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Error::Error(Utf8StringCR message) :
AsyncError(message),
m_status(Status::InternalCacheError)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Error::Error(ICachingDataSource::Status status, ICancellationTokenPtr cancellationToken) :
Error(status)
    {
    if (cancellationToken && cancellationToken->IsCanceled())
        {
        m_message.clear();
        m_description.clear();
        m_status = Status::Canceled;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Error::~Error()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Status ICachingDataSource::Error::ConvertCacheStatus(CacheStatus status)
    {
    if (CacheStatus::DataNotCached == status)
        {
        return ICachingDataSource::Status::DataNotCached;
        }
    if (CacheStatus::Error == status)
        {
        return ICachingDataSource::Status::InternalCacheError;
        }
    if (CacheStatus::OK == status)
        {
        return ICachingDataSource::Status::Success;
        }

    BeAssert(false);
    return ICachingDataSource::Status::InternalCacheError;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WSErrorCR ICachingDataSource::Error::GetWSError() const
    {
    return m_wsError;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::Status ICachingDataSource::Error::GetStatus() const
    {
    return m_status;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::ObjectsData::ObjectsData() :
ObjectsData(nullptr, DataOrigin::CachedData)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::ObjectsData::ObjectsData(std::shared_ptr<Json::Value> data, DataOrigin origin) :
m_data(data ? data : std::make_shared<Json::Value>()),
m_origin(origin)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCR ICachingDataSource::ObjectsData::GetJson() const
    {
    return *m_data;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueR ICachingDataSource::ObjectsData::GetJson()
    {
    return *m_data;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::DataOrigin ICachingDataSource::ObjectsData::GetOrigin() const
    {
    return m_origin;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::KeysData::KeysData() :
KeysData(nullptr, DataOrigin::CachedData)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::KeysData::KeysData(std::shared_ptr<ECInstanceKeyMultiMap> data, DataOrigin origin) :
m_data(data ? data : std::make_shared<ECInstanceKeyMultiMap>()),
m_origin(origin)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ECInstanceKeyMultiMap> ICachingDataSource::KeysData::GetKeysPtr()
    {
    return m_data;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const ECInstanceKeyMultiMap& ICachingDataSource::KeysData::GetKeys() const
    {
    return *m_data;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyMultiMap& ICachingDataSource::KeysData::GetKeys()
    {
    return *m_data;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::DataOrigin ICachingDataSource::KeysData::GetOrigin() const
    {
    return m_origin;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::FileData::FileData() :
m_origin(DataOrigin::CachedData)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::FileData::FileData(BeFileNameCR filePath, DataOrigin origin) :
m_filePath(filePath),
m_origin(origin)
    {
    BeAssert(origin == DataOrigin::CachedData || origin == DataOrigin::RemoteData);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR ICachingDataSource::FileData::GetFilePath() const
    {
    return m_filePath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::DataOrigin ICachingDataSource::FileData::GetOrigin() const
    {
    return m_origin;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::FailedObject::FailedObject(ObjectIdCR objectId, Utf8StringCR objectLabel, ErrorCR error) :
m_objectId(objectId),
m_objectLabel(objectLabel),
m_error(error)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectIdCR ICachingDataSource::FailedObject::GetObjectId() const
    {
    return m_objectId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ICachingDataSource::FailedObject::GetObjectLabel() const
    {
    return m_objectLabel;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ICachingDataSource::ErrorCR ICachingDataSource::FailedObject::GetError() const
    {
    return m_error;
    }