/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/ThumbnailsManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/ThumbnailsManager.h>
#include "Utils.h"
#include "Logging.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

#define THUMBNAILS_DIR L"Thumbnails"
#define THUMBNAIL_FILE_EXT L"png"

//---------------------------------------------------------------------------------------
// @bsimethod                                    Andrius.Zonys                  04/2018
//---------------------------------------------------------------------------------------
Utf8CP Thumbnail::GetClassName(Size size)
    {
    return Size::Small == size ? ServerSchema::Class::SmallThumbnail : ServerSchema::Class::LargeThumbnail;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  04/2018
//---------------------------------------------------------------------------------------
void Thumbnail::AddHasThumbnailSelect(Utf8StringR selectString, Size size)
    {
    selectString.Sprintf("%s,%s-forward-%s.*", selectString.c_str(), ServerSchema::Relationship::HasThumbnail, GetClassName(size));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Andrius.Zonys                  04/2018
//---------------------------------------------------------------------------------------
Utf8String Thumbnail::ParseFromRelated(WSObjectsReader::Instance instance, Size size)
    {
    auto relationshipInstances = instance.GetRelationshipInstances();
    if (0 == relationshipInstances.Size())
        return nullptr;

    Utf8CP className = GetClassName(size);
    for (auto relationshipInstance : relationshipInstances)
        {
        auto relatedInstance = relationshipInstance.GetRelatedInstance();
        if (!relatedInstance.IsValid())
            continue;

        if (relatedInstance.GetObjectId().className == className)
            return relatedInstance.GetObjectId().GetRemoteId();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  04/2018
//---------------------------------------------------------------------------------------
ThumbnailsIdsTaskPtr ThumbnailsManager::GetAllThumbnailsIds
(
Thumbnail::Size size,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "ThumbnailsManager::GetAllThumbnailsIds";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    WSQuery thumbnailsQuery(ServerSchema::Schema::iModel, Thumbnail::GetClassName(size));

    return ExecuteWithRetry<bvector<Utf8String>>([=]()
        {
        return m_wsRepositoryClient->SendQueryRequest(thumbnailsQuery, nullptr, nullptr, cancellationToken)
            ->Then<ThumbnailsIdsResult>([=](const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
                return ThumbnailsIdsResult::Error(result.GetError());
                }

            bvector<Utf8String> ids;
            for (auto const& value : result.GetValue().GetInstances())
                {
                ids.push_back(value.GetObjectId().GetRemoteId());
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            return ThumbnailsIdsResult::Success(ids);
            });
        });
    }
    
//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  04/2018
//---------------------------------------------------------------------------------------
ThumbnailImageTaskPtr ThumbnailsManager::GetThumbnailByIdInternal
(
ObjectIdCR thumbnail,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "ThumbnailsManager::GetThumbnailByIdInternal";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    HttpByteStreamBodyPtr responseBody = HttpByteStreamBody::Create();
    return m_wsRepositoryClient->SendGetFileRequest(thumbnail, responseBody, nullptr, nullptr, cancellationToken)
        ->Then<ThumbnailImageResult>([=](const WSResult& streamResult)
        {
        if (!streamResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, streamResult.GetError().GetMessage().c_str());
            return ThumbnailImageResult::Error(streamResult.GetError());
            }

        ByteStream byteStream = responseBody->GetByteStream();
        Render::Image image = Render::Image::FromPng(byteStream.GetData(), byteStream.GetSize());

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
        return ThumbnailImageResult::Success(image);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  04/2018
//---------------------------------------------------------------------------------------
ThumbnailImageTaskPtr ThumbnailsManager::GetThumbnailById
(
Utf8StringCR thumbnailId,
Thumbnail::Size size,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "ThumbnailsManager::GetThumbnailById";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    //double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    ObjectId thumbnail(ServerSchema::Schema::iModel, Thumbnail::GetClassName(size), thumbnailId);

    return ExecuteWithRetry<Render::Image>([=]()
        {
        return GetThumbnailByIdInternal(thumbnail, cancellationToken);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  04/2018
//---------------------------------------------------------------------------------------
ThumbnailImageTaskPtr ThumbnailsManager::GetThumbnailByVersionId
(
Utf8StringCR versionId,
Thumbnail::Size size,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "ThumbnailsManager::GetThumbnailByVersionId";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    //double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    WSQuery query(ServerSchema::Schema::iModel, Thumbnail::GetClassName(size));

    Utf8String filter;
    filter.Sprintf("(%s-backward-%s.%s+eq+'%s')", ServerSchema::Relationship::HasThumbnail, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str());
    query.SetFilter(filter);

    return ExecuteWithRetry<Render::Image>([=]()
        {
        ThumbnailImageResultPtr finalResult = std::make_shared<ThumbnailImageResult>();
        return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)
            ->Then([=](const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
                finalResult->SetError(result.GetError());
                return;
                }
        
            auto thumbnailInstance = *result.GetValue().GetInstances().begin();
            ObjectIdCR thumbnail = thumbnailInstance.GetObjectId();

            GetThumbnailByIdInternal(thumbnail, cancellationToken)
                ->Then([=](const ThumbnailImageResult& fileResult)
                {
                if (!fileResult.IsSuccess())
                    {
                    finalResult->SetError(fileResult.GetError());
                    return;
                    }

                finalResult->SetSuccess(fileResult.GetValue());
                });
            })->Then<ThumbnailImageResult>([=]
                {
                return *finalResult;
                });
        });
    }