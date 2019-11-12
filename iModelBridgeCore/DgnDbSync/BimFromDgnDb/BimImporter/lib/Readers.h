/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <BimFromDgnDb/BimFromDgnDb.h>

#include "SyncInfo.h"
BEGIN_BIM_FROM_DGNDB_NAMESPACE

#define MODEL_PROP_ModeledElement "ModeledElement"
#define MODEL_PROP_IsPrivate "IsPrivate"
#define MODEL_PROP_Properties "Properties"
#define MODEL_PROP_IsTemplate "IsTemplate"
#define BIS_ELEMENT_PROP_CodeSpecId "CodeSpec"

static Utf8CP const JSON_TYPE_KEY = "Type";
static Utf8CP const JSON_OBJECT_KEY = "Object";
static Utf8CP const JSON_TYPE_Font = "Font";
static Utf8CP const JSON_TYPE_FontFaceData = "FontFaceData";
static Utf8CP const JSON_TYPE_LineStyleProperty = "LineStyleProperty";
static Utf8CP const JSON_TYPE_Model = "Model";
static Utf8CP const JSON_TYPE_CategorySelector = "CategorySelector";
static Utf8CP const JSON_TYPE_ModelSelector = "ModelSelector";
static Utf8CP const JSON_TYPE_DisplayStyle = "DisplayStyle";
static Utf8CP const JSON_TYPE_DictionaryModel = "DictionaryModel";
static Utf8CP const JSON_TYPE_CodeSpec = "CodeSpec";
static Utf8CP const JSON_TYPE_Schema = "Schema";
static Utf8CP const JSON_TYPE_Element = "Element";
static Utf8CP const JSON_TYPE_GeometricElement2d = "GeometricElement2d";
static Utf8CP const JSON_TYPE_GeometricElement3d = "GeometricElement3d";
static Utf8CP const JSON_TYPE_GeometryPart = "GeometryPart";
static Utf8CP const JSON_TYPE_Subject = "Subject";
static Utf8CP const JSON_TYPE_Partition = "Partition";
static Utf8CP const JSON_TYPE_Category = "Category";
static Utf8CP const JSON_TYPE_SubCategory = "SubCategory";
static Utf8CP const JSON_TYPE_ViewDefinition3d = "ViewDefinition3d";
static Utf8CP const JSON_TYPE_ViewDefinition2d = "ViewDefinition2d";
static Utf8CP const JSON_TYPE_LinkTable = "LinkTable";
static Utf8CP const JSON_TYPE_ElementGroupsMembers = "ElementGroupsMembers";
static Utf8CP const JSON_TYPE_ElementHasLinks = "ElementHasLinks";
static Utf8CP const JSON_TYPE_AnnotationTextStyle = "AnnotationTextStyle";
static Utf8CP const JSON_TYPE_LineStyleElement = "LineStyleElement";
static Utf8CP const JSON_TYPE_Texture = "Texture";
static Utf8CP const JSON_TYPE_Plan = "Plan";
static Utf8CP const JSON_TYPE_WorkBreakdown = "WorkBreakdown";
static Utf8CP const JSON_TYPE_Activity = "Activity";
static Utf8CP const JSON_TYPE_Baseline = "Baseline";
static Utf8CP const JSON_TYPE_TimeSpan = "TimeSpan";
static Utf8CP const JSON_TYPE_PropertyData = "PropertyData";
static Utf8CP const JSON_TYPE_ElementMultiAspect = "ElementMultiAspect";
static Utf8CP const JSON_TYPE_ElementUniqueAspect = "ElementUniqueAspect";
static Utf8CP const JSON_TYPE_TextAnnotationData = "TextAnnotationData";
static Utf8CP const JSON_TYPE_PointCloudModel = "PointCloudModel";
static Utf8CP const JSON_TYPE_ThreeMxModel = "ThreeMxModel";
static Utf8CP const JSON_TYPE_RasterFileModel = "RasterFileModel";
static Utf8CP const JSON_TYPE_EmbeddedFile = "EmbeddedFile";

static Utf8CP const  BIS_ELEMENT_PROP_CodeSpec = "CodeSpec";
static Utf8CP const  BIS_ELEMENT_PROP_CodeScope = "CodeScope";
static Utf8CP const  BIS_ELEMENT_PROP_CodeValue = "CodeValue";
static Utf8CP const  BIS_ELEMENT_PROP_Model = "Model";
static Utf8CP const  BIS_ELEMENT_PROP_Parent = "Parent";

#pragma once
struct BimFromJsonImpl;

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct Reader
    {
    protected:
        BimFromJsonImpl* m_importer;

        BentleyStatus RemapCodeSpecId(Json::Value& element);
        CodeSpecId GetMappedCodeSpecId(Json::Value& element);
        BentleyStatus RemapPropertyElementId(ECN::IECInstanceR properties, Utf8CP propertyName);
        DgnElementId GetMappedElementId(Json::Value& element, Utf8CP propertyName);
        static NativeLogging::ILogger& GetLogger() { return *NativeLogging::LoggingManager::GetLogger("BimUpgrader"); }
        DgnDbP GetDgnDb();
        SyncInfo* GetSyncInfo();

        virtual ECN::ECClassCP _GetClassFromName(Utf8CP classKey, Json::Value& element);

        virtual Utf8CP _GetElementType() { return "Object"; }
        virtual BentleyStatus _Read(Json::Value& object) = 0;
        virtual ECN::IECInstancePtr _CreateInstance(Json::Value& object);

    public:
        Reader(BimFromJsonImpl* importer);
        virtual ~Reader() {}
        BentleyStatus Read(Json::Value& object) { return _Read(object); }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct FontFaceReader : Reader
    {
    DEFINE_T_SUPER(Reader);
    protected:
        BentleyStatus _Read(Json::Value& object) override;

    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct FontReader : Reader
    {
    DEFINE_T_SUPER(Reader);
    protected:
        BentleyStatus _Read(Json::Value& object) override;

    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct LsComponentReader : Reader
    {
    DEFINE_T_SUPER(Reader);
    protected:
        BentleyStatus _Read(Json::Value& object) override;

    public:
        using Reader::Reader;
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
        ModelReader(BimFromJsonImpl* importer, bool isDictionary) : Reader(importer), m_isDictionary(isDictionary) {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct PointCloudModelReader : Reader
    {
    DEFINE_T_SUPER(Reader);
    protected:
        BentleyStatus _Read(Json::Value& object) override;

    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct ThreeMxModelReader : Reader
    {
    DEFINE_T_SUPER(Reader);
    protected:
        BentleyStatus _Read(Json::Value& object) override;

    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct RasterFileModelReader : Reader
    {
    DEFINE_T_SUPER(Reader);
    protected:
        BentleyStatus _Read(Json::Value& object) override;

    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct ElementReader : Reader
    {
    DEFINE_T_SUPER(Reader);
    protected:
        DgnElementId m_instanceId;

        BentleyStatus RemapModelId(Json::Value& element);
        DgnModelId GetMappedModelId(Json::Value& element, Utf8CP propertyName = "Model");
        DgnCode CreateCodeFromJson(Json::Value& element);
        BentleyStatus RemapCategoryId(Json::Value& element);
        BentleyStatus RemapParentId(Json::Value& element);

        ECN::IECInstancePtr _CreateInstance(Json::Value& object) override;
        BentleyStatus _Read(Json::Value& object) override;
        Utf8CP _GetElementType() override { return "Element"; }
        ECN::ECClassCP _GetClassFromName(Utf8CP classKey, Json::Value& element) override;

        virtual BentleyStatus _OnInstanceCreated(ECN::IECInstanceR instance) { return SUCCESS; }
        virtual BentleyStatus _OnElementCreated(DgnElementR element, ECN::IECInstanceR properties) { return SUCCESS; }
        virtual Utf8String _GetRelationshipClassName();
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

    protected:
        BentleyStatus _OnElementCreated(DgnElementR element, ECN::IECInstanceR properties) override;
        Utf8CP _GetElementType() override { return "GeometricElement"; }

    public:
        GeometricElementReader(BimFromJsonImpl* importer) : GeometryReader(importer) {}

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
// @bsiclass                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct AnnotationTextStyleReader : ElementReader
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
        ViewDefinitionReader(BimFromJsonImpl* importer, bool is3d) : ElementReader(importer), m_is3d(is3d) {}

    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct LineStyleReader : ElementReader
    {
    DEFINE_T_SUPER(ElementReader);
    protected:
        BentleyStatus _OnElementCreated(DgnElementR element, ECN::IECInstanceR properties) override;

    public:
        using ElementReader::ElementReader;
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
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct DisplayStyleReader : ElementReader
    {
    protected:
        BentleyStatus _Read(Json::Value& object) override;
        Utf8CP _GetElementType() override { return "DisplayStyle"; }

    public:
        using ElementReader::ElementReader;
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
    DEFINE_T_SUPER(ElementReader);

    private:
        bool m_isDefault;

    protected:
        BentleyStatus _Read(Json::Value& object) override;
    public:
        SubCategoryReader(BimFromJsonImpl* importer, bool isDefault) : ElementReader(importer), m_isDefault(isDefault) {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct SchemaReader : Reader
    {
    private:
        ECN::ECClassP m_definitionElement;
        ECN::ECClassCP m_elementClass;
        ECN::ECClassCP m_multiAspectClass;
        ECN::ECClassCP m_uniqueAspectClass;
        ECN::ECRelationshipClassCP m_elementToMulti;
        ECN::ECRelationshipClassCP m_elementToUnique;
        ECN::ECRelationshipClassCP m_elementToElement;
        bvector<ECN::ECRelationshipClassP> m_relationshipsToRemove;

        BentleyStatus ValidateBaseClasses(ECN::ECSchemaP schema);
        void SetBaseClassForRelationship(ECN::ECRelationshipClassP relClass);
        void CheckConstraints(ECN::ECRelationshipClassP relClass);

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
// @bsiclass                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct LinkTableReader : Reader
    {
    private:
        void SetNavigationProperty(DgnElementId sourceId, DgnElementId targetId, ECN::ECRelationshipClassCP relClass);
    protected:
        BentleyStatus _Read(Json::Value& relationship) override;
    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct ElementGroupsMembersReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& relationship) override;
    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct ElementHasLinksReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& relationship) override;
    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct TextureReader : ElementReader
    {
    protected:
        BentleyStatus _Read(Json::Value& texture) override;
    public:
        using ElementReader::ElementReader;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct BaselineReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& baseline) override;
    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
struct TimeSpanReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& timeSpan) override;
    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct PropertyDataReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& propData) override;

    public:
        using Reader::Reader;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct EmbeddedFileReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& fileData) override;

    public:
        using Reader::Reader;
    };
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct ElementAspectReader : Reader
    {
    private:
        bool m_isUnique = false;
    protected:
        BentleyStatus _Read(Json::Value& object) override;
    public:
        ElementAspectReader(BimFromJsonImpl* importer, bool isUnique) : Reader(importer), m_isUnique(isUnique) {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct TextAnnotationDataReader : Reader
    {
    protected:
        BentleyStatus _Read(Json::Value& object) override;
    public:
        using Reader::Reader;
    };
END_BIM_FROM_DGNDB_NAMESPACE