/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/Readers.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <BimTeleporter/BimTeleporter.h>

#include "SyncInfo.h"
BEGIN_BIM_TELEPORTER_NAMESPACE

#pragma once
struct BisJson1ImporterImpl;

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct Reader
    {
    protected:
        BisJson1ImporterImpl* m_importer;

        BentleyStatus RemapCodeSpecId(Json::Value& element);
        CodeSpecId GetMappedCodeSpecId(Json::Value& element);
        BentleyStatus RemapPropertyElementId(ECN::IECInstanceR properties, Utf8CP propertyName);
        DgnElementId GetMappedElementId(Json::Value& element, Utf8CP propertyName);
        static NativeLogging::ILogger& GetLogger() { return *NativeLogging::LoggingManager::GetLogger("BimTeleporter"); }
        DgnDbP GetDgnDb();
        SyncInfo* GetSyncInfo();

        ECN::ECClassCP GetClassFromKey(Utf8CP classKey);

        virtual Utf8CP _GetElementType() { return "Object"; }
        virtual BentleyStatus _Read(Json::Value& object) = 0;
        virtual ECN::IECInstancePtr _CreateInstance(Json::Value& object);

    public:
        Reader(BisJson1ImporterImpl* importer);
        BentleyStatus Read(Json::Value& object) { return _Read(object); }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct ModelReader : Reader
    {
    private:
        bool m_isDictionary;

    protected:
        BentleyStatus _Read(Json::Value& object) override;

    public:
        ModelReader(BisJson1ImporterImpl* importer, bool isDictionary) : Reader(importer), m_isDictionary(isDictionary) {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct ElementReader : Reader
    {
    DEFINE_T_SUPER(Reader);
    protected:
        uint64_t m_instanceId;

        BentleyStatus RemapModelId(Json::Value& element);
        DgnModelId GetMappedModelId(Json::Value& element);
        DgnCode CreateCodeFromJson(Json::Value& element);
        BentleyStatus RemapCategoryId(Json::Value& element);
        BentleyStatus RemapParentId(Json::Value& element);

        ECN::IECInstancePtr _CreateInstance(Json::Value& object) override;
        BentleyStatus _Read(Json::Value& object) override;
        Utf8CP _GetElementType() override { return "Element"; }

        virtual BentleyStatus _OnInstanceCreated(ECN::IECInstanceR instance) { return SUCCESS; }
        virtual BentleyStatus _OnElementCreated(DgnElementR element, ECN::IECInstanceR properties) { return SUCCESS; }
        virtual ECN::ECClassId _GetRelationshipClassId();
    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct GeometryReader : ElementReader
    {
    DEFINE_T_SUPER(ElementReader);
    protected:
        GeometryStream m_geomStream;
        BentleyStatus _OnInstanceCreated(ECN::IECInstanceR instance) override;
        BentleyStatus _OnElementCreated(DgnElementR element, ECN::IECInstanceR properties) override = 0;
    public:
        using ElementReader::ElementReader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct GeometricElementReader : GeometryReader
    {
    DEFINE_T_SUPER(GeometryReader);
    private:
        bool m_is3d;

    protected:
        BentleyStatus _OnElementCreated(DgnElementR element, ECN::IECInstanceR properties) override;
        Utf8CP _GetElementType() override { return "GeometricElement"; }

    public:
        GeometricElementReader(BisJson1ImporterImpl* importer, bool is3d) : GeometryReader(importer), m_is3d(is3d) {}

    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct GeometryPartReader : GeometryReader
    {
    DEFINE_T_SUPER(GeometryReader);

    protected:
        BentleyStatus _OnElementCreated(DgnElementR element, ECN::IECInstanceR properties) override;
        Utf8CP _GetElementType() override { return "GeometryPart"; }

    public:
        using GeometryReader::GeometryReader;

    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct ViewDefinitionReader : ElementReader
    {
    DEFINE_T_SUPER(ElementReader);
    private:
        bool m_is3d;

    protected:
        BentleyStatus _Read(Json::Value& object) override;
        BentleyStatus _OnInstanceCreated(ECN::IECInstanceR instance) override;
        Utf8CP _GetElementType() override { return "View Definition"; }
    public:
        ViewDefinitionReader(BisJson1ImporterImpl* importer, bool is3d) : ElementReader(importer), m_is3d(is3d) {}

    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct CategorySelectorReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& object) override;
        Utf8CP _GetElementType() override { return "CategorySelector"; }

    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct ModelSelectorReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& object) override;
        Utf8CP _GetElementType() override { return "ModelSelector"; }

    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct DisplayStyleReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& object) override;
        Utf8CP _GetElementType() override { return "DisplayStyle"; }

    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct CodeSpecReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& object) override;

    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct CategoryReader : ElementReader
    {
    DEFINE_T_SUPER(ElementReader);

    protected:
        BentleyStatus _Read(Json::Value& object) override;

    public:
        using ElementReader::ElementReader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct SubCategoryReader : ElementReader
    {
    DEFINE_T_SUPER(Reader);

    private:
        bool m_isDefault;

    protected:
        BentleyStatus _Read(Json::Value& object) override;
    public:
        SubCategoryReader(BisJson1ImporterImpl* importer, bool isDefault) : ElementReader(importer), m_isDefault(isDefault) {}
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct SchemaReader : Reader
    {
    private:
        BentleyStatus ImportSchema(ECN::ECSchemaP schema);

    protected:
        BentleyStatus _Read(Json::Value& object) override;
    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct SubjectReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& object) override;
    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct PartitionReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& partition) override;
    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct ElementRefersToElementReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& relationship) override;
    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct ElementGroupsMembersReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& relationship) override;
    public:
        using Reader::Reader;
    };

END_BIM_TELEPORTER_NAMESPACE