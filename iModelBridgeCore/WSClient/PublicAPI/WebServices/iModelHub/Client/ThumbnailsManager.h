/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/iModelHub/Client/Result.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGN
DEFINE_POINTER_SUFFIX_TYPEDEFS(ThumbnailsManager);

DEFINE_TASK_TYPEDEFS(Render::Image, ThumbnailImage);
DEFINE_TASK_TYPEDEFS(bvector<Utf8String>, ThumbnailsIds);

//=======================================================================================
//@bsistruct                                      Andrius.Zonys                  04/2018
//=======================================================================================
struct Thumbnail
{
    //! Thumbnail sizes
    enum Size ENUM_UNDERLYING_TYPE(uint32_t)
        {
        None = 0,
        Small = 1 << 0,
        Large = 1 << 1
        };

    static Utf8CP GetClassName(Size size);
    static void AddHasThumbnailSelect(Utf8StringR selectString, Size size);
    static Utf8String ParseFromRelated(WSObjectsReader::Instance instance, Size size);
};

//=======================================================================================
//@bsistruct                                      Andrius.Zonys                 04/2018
//=======================================================================================
struct ThumbnailsManager : RefCountedBase
{
private:
    friend struct iModelConnection;

    WebServices::IWSRepositoryClientPtr m_wsRepositoryClient;
    ThumbnailsManager(WebServices::IWSRepositoryClientPtr wsRepositoryClient) : m_wsRepositoryClient(wsRepositoryClient) {};
    ThumbnailsManager() {};

    ThumbnailImageTaskPtr GetThumbnailByIdInternal(ObjectIdCR thumbnail, ICancellationTokenPtr cancellationToken = nullptr) const;

public:
    //! Returns all ids of given size Thumbnails
    //! @param[in] size
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has bvector of Thumbnail's ids as the result.
    IMODELHUBCLIENT_EXPORT ThumbnailsIdsTaskPtr GetAllThumbnailsIds(Thumbnail::Size size, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns Thumbnail image by its id
    //! @param[in] thumbnailId
    //! @param[in] size
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has ByteStream of Thumbnail image as the result.
    IMODELHUBCLIENT_EXPORT ThumbnailImageTaskPtr GetThumbnailById(Utf8StringCR thumbnailId, Thumbnail::Size size, ICancellationTokenPtr cancellationToken = nullptr) const;
    
    //! Returns Thumbnail image by version id
    //! @param[in] versionId
    //! @param[in] size
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has ByteStream of Thumbnail image as the result.
    IMODELHUBCLIENT_EXPORT ThumbnailImageTaskPtr GetThumbnailByVersionId(Utf8StringCR versionId, Thumbnail::Size size, ICancellationTokenPtr cancellationToken = nullptr) const;
};

END_BENTLEY_IMODELHUB_NAMESPACE
