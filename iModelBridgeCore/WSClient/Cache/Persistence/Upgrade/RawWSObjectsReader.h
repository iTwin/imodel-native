/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/RawWSObjectsReader.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Client/Response/WSObjectsReader.h>
#include <WebServices/Client/Response/WSObjectsResponse.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct RawWSObjectsReader : public WSObjectsReader
    {
    public:
        struct RawInstance
            {
            std::shared_ptr<rapidjson::Document> json;
            ObjectId objectId;
            Utf8String eTag;
            };

    private:
        RawWSObjectsReader(const bvector<RawInstance>& instances);

        const bvector<RawInstance>& m_instances;
        bmap<const rapidjson::Value*, const Utf8String*> m_instancesToETags;

    public:
        static WSObjectsResponse CreateWSObjectsResponse(const bvector<RawInstance>& instances, Utf8StringCR eTag = "");

        virtual Instances ReadInstances(std::shared_ptr<const rapidjson::Value> data) override;

        virtual bool HasReadErrors() const override;
        virtual rapidjson::SizeType GetInstanceCount() const override;
        virtual Instance GetInstance(rapidjson::SizeType index) const override;
        virtual Instance GetRelatedInstance(const rapidjson::Value* relatedInstance) const override;
        virtual RelationshipInstance GetRelationshipInstance(const rapidjson::Value* relationshipInstance) const override;
        virtual Utf8String GetInstanceETag(const rapidjson::Value* instance) const override;
        virtual rapidjson::SizeType GetRelationshipInstanceCount(const rapidjson::Value* relationshipInstances) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
