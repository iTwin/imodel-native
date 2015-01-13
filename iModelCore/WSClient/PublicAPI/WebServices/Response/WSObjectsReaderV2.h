/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/WebServices/Response/WSObjectsReaderV2.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Response/WSObjectsReader.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
class WSObjectsReaderV2 : public WSObjectsReader
    {
    private:
        rapidjson::SizeType m_instanceCount;

    private:
        WSObjectsReaderV2 ();

        ObjectId ReadObjectId (const rapidjson::Value& instanceData) const;
        Instance ReadInstance (const rapidjson::Value& instanceData) const;

    public:
        WS_EXPORT static std::shared_ptr<WSObjectsReaderV2> Create ();
        WS_EXPORT virtual ~WSObjectsReaderV2 ();

        WS_EXPORT virtual Instances ReadInstances (std::shared_ptr<const rapidjson::Value> data) override;

        virtual bool HasReadErrors () const override;
        virtual rapidjson::SizeType GetInstanceCount () const override;
        virtual Instance GetInstance (rapidjson::SizeType index) const override;

        virtual Instance GetRelatedInstance (const rapidjson::Value* relatedInstance) const override;
        virtual RelationshipInstance GetRelationshipInstance (const rapidjson::Value* relationshipInstance) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
