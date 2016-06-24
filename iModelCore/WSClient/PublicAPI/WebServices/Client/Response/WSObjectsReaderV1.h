/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/WebServices/Client/Response/WSObjectsReaderV1.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSObjectsReaderV1 : public WSObjectsReader
    {
    private:
        Utf8String m_schemaName;
        Utf8String m_className;
        rapidjson::SizeType m_instanceCount;
        bool m_multipleInstancesFormat;

    private:
        WSObjectsReaderV1(Utf8StringCR schemaName);
        WSObjectsReaderV1(Utf8StringCR schemaName, Utf8StringCR className);

        Instance ReadInstance(const rapidjson::Value* instanceProperties, Utf8String className) const;

    public:
        //! Constructor for multiple object response
        WSCLIENT_EXPORT static std::shared_ptr<WSObjectsReaderV1> Create(Utf8StringCR schemaName);
        //! Constructor for one object request response
        WSCLIENT_EXPORT static std::shared_ptr<WSObjectsReaderV1> Create(Utf8StringCR schemaName, Utf8StringCR className);
        WSCLIENT_EXPORT virtual ~WSObjectsReaderV1();

        WSCLIENT_EXPORT virtual Instances ReadInstances(std::shared_ptr<const rapidjson::Value> data) override;

        virtual bool HasReadErrors() const override;
        virtual rapidjson::SizeType GetInstanceCount() const override;
        virtual Instance GetInstance(rapidjson::SizeType index) const override;

        virtual Instance GetRelatedInstance(const rapidjson::Value* relatedInstance) const override;
        virtual RelationshipInstance GetRelationshipInstance(const rapidjson::Value* relationshipInstance) const override;

        virtual Utf8String GetInstanceETag(const rapidjson::Value* instance) const override;
        virtual rapidjson::SizeType GetRelationshipInstanceCount(const rapidjson::Value* relationshipInstances) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
