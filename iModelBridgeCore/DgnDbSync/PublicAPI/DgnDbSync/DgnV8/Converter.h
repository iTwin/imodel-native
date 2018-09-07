/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/DgnV8/Converter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnV8.h"

#include <BeSQLite/L10N.h>
#include "SyncInfo.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <BeXml/BeXml.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/LineStyle.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/Annotations/TextAnnotation.h>
#include <DgnPlatform/ViewDefinition.h>
#include <DgnPlatform/LinkElement.h>
#include <DgnPlatform/image.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/GenericDomain.h>
#include <iModelBridge/iModelBridge.h>

namespace DgnDbApi = BentleyApi::Dgn;

#define CATEGORY_NAME_Uncategorized     "Uncategorized"
#define CATEGORY_NAME_Attachments       "Attachments"       // *** WIP_SHEET - category names
#define CATEGORY_NAME_ExtractedGraphics "ExtractedGraphics" // *** WIP_CONVERT_CVE - category names
#define CATEGORY_NAME_Section           "Section"           // *** WIP_CONVERT_CVE - category names
#define CATEGORY_NAME_Plan              "Plan"              // *** WIP_CONVERT_CVE - category names
#define CATEGORY_NAME_Elevation         "Elevation"         // *** WIP_CONVERT_CVE - category names
#define CATEGORY_NAME_Detail            "Detail"            // *** WIP_CONVERT_CVE - category names
#define SUBCATEGORY_NAME_Cut            "Cut"               // *** WIP_CONVERT_CVE - category names
#define SUBCATEGORY_NAME_InsideForward  "InsideForward"     // *** WIP_CONVERT_CVE - category names
#define SUBCATEGORY_NAME_InsideBackward "InsideBackward"    // *** WIP_CONVERT_CVE - category names
#define SUBCATEGORY_NAME_Outside        "Outside"           // *** WIP_CONVERT_CVE - category names
#define SUBCATEGORY_NAME_Inside         "Inside"            // *** WIP_CONVERT_CVE - category names

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

struct V8GraphicsCollector;
struct LineStyleConverter;
struct LinkConverter;
struct SheetViewFactory;
struct DrawingViewFactory;
struct Converter;
struct ElementConverter;
struct ElementAspectConverter;
struct ModelTypeAppData;

typedef RefCountedPtr<LineStyleConverter> LineStyleConverterPtr;

//=======================================================================================
//! Identifies an ECClass in an ECSchema from a v8 repository
// @bsiclass                                                    Krischan.Eberle   10/14
//=======================================================================================
struct ECClassName
{
private:
    Utf8String m_schemaName;
    Utf8String m_className;

public:
    ECClassName() {}
    ECClassName(Utf8CP schemaName, Utf8CP className) : m_schemaName(schemaName), m_className(className) {}
    explicit ECClassName(ECObjectsV8::ECClassCR ecClass) : m_schemaName(ecClass.GetSchema().GetName().c_str()), m_className(ecClass.GetName().c_str()) {}
    explicit ECClassName(ECN::ECClassCR ecClass) : m_schemaName(ecClass.GetSchema().GetName()), m_className(ecClass.GetName()) {}

    Utf8CP GetSchemaName() const {return m_schemaName.c_str();}
    Utf8CP GetClassName() const {return m_className.c_str();}
    Utf8String GetClassFullName() const;
    bool ECClassName::operator == (ECClassName className) const {return m_schemaName.Equals(className.m_schemaName) && m_className.Equals(className.m_className);}
    bool ECClassName::operator != (ECClassName className) const {return !m_schemaName.Equals(className.m_schemaName) || !m_className.Equals(className.m_className);}

    bool IsValid() const {return !m_schemaName.empty() && !m_className.empty();}
};

//=======================================================================================
//! Identifies an ECInstance in a foreign repository
// @bsiclass                                                    Krischan.Eberle   10/14
//=======================================================================================
struct V8ECInstanceKey
{
private:
    ECClassName m_className;
    Utf8String m_instanceId;

public:
    V8ECInstanceKey() {}
    V8ECInstanceKey(ECClassName const& className, Utf8CP instanceId) : m_className(className), m_instanceId(instanceId) {}
    V8ECInstanceKey(ECClassName const& className, WCharCP v8InstanceId) : m_className(className) {m_instanceId.Assign(v8InstanceId);}

    ECClassName const& GetClassName() const {return m_className;}
    Utf8CP GetInstanceId() const {return m_instanceId.c_str();}
    bool IsValid() const {return m_className.IsValid() && !m_instanceId.empty();}
};

//=======================================================================================
//! Base class for options that control how to merge various named data structures that match specified properties
// @bsiclass                                                    Sam.Wilson      12/13
//=======================================================================================
struct ImportRule
{
protected:

    bool m_hasNewName;    //!< Was a new name specified?
    Utf8String m_newName; //!< The new name to assign to matching items.

    union
        {
        struct
            {
            uint32_t file:2; //!< Match only levels from the specified file?
            uint32_t name:2; //!< Match only levels with the specified name?
            };
        uint32_t allBits;
        } m_matchOnBase;

    Utf8String m_name; //!< The name to match
    Utf8String m_file; //!< The name of the file to match

public:
    //! Constructs an empty options object with no matching criteria specified.
    ImportRule() {m_hasNewName=false;m_matchOnBase.allBits=0;}

    Utf8String ToString() const;

    //! Parse from XML configuration
    void InitFromXml(BeXmlNode&);

    //! Test if this merge option should be applied to the item with the specified name
    //! @return \a true if this merge option should be applied to \a upgradeLevel
    bool Matches(Utf8StringCR name, DgnV8FileCR ff) const;

    //! Compute new name
    //! @param[out] newName The new name, if successful
    //! @param[in] ff The file that is being imported
    //! @return non-zero error status if no new name could be computed. In case of error, \a newName is not modified.
    BentleyStatus ComputeNewName(Utf8StringR newName, DgnV8FileCR ff) const;

    //! Compute new name
    //! @param[out] newName The new name, if successful
    //! @param[in] fm The model that is being imported
    //! @return non-zero error status if no new name could be computed. In case of error, \a newName is not modified.
    BentleyStatus ComputeNewName(Utf8StringR newName, DgnV8ModelCR fm) const;
};

//=======================================================================================
// @bsiclass                                                    Krischan.Eberle   04/15
//=======================================================================================
enum class V8ElementType
{
    Graphical,
    NonGraphical,
    NamedGroup
};

//=======================================================================================
// @bsiclass                                                    Krischan.Eberle   04/15
//=======================================================================================
enum class BisConversionRule
{
    Ignored = 0,
    IgnoredPolymorphically,

    TransformedUnbisified,
    TransformedUnbisifiedAndIgnoreInstances,

    ToAspectOnly,

    ToSpatialLocationElement,
    ToPhysicalElement, // This is used when the element has an ECClass and the ECClass will get the "BisCore:PhysicalElement" as its base class
    ToPhysicalObject, // This is used when an element has no primary ECClass and so is created using the Generic:PhysicalObject class
    ToDrawingGraphic,
    ToGroup, // This is used when the named group element has an ECClass on it, and the ECClass will get BisCore:GroupInformationElement as its base class
    ToGenericGroup, // This is used when the named group element has no primary ECClass and so is created using the Generic:Group class
    ToDefaultBisBaseClass,
    ToDefaultBisClass,
};

//=======================================================================================
// Information about the target BIM model that may be needed when deciding how to 
// convert a V8 ECClass into a BIS class.
// @bsiclass                                                    Sam.Wilson      11/2017
//=======================================================================================
struct BisConversionTargetModelInfo
{
    enum class ModelType
        {
        TwoD,
        ThreeD,
        Dictionary
        };
    ModelType m_modelType;

    explicit BisConversionTargetModelInfo(DgnModelCR);
    explicit BisConversionTargetModelInfo(ModelType mt) : m_modelType(mt) {}

    bool IsDictionary() const {return ModelType::Dictionary == m_modelType;}
    bool Is3d() const {return ModelType::ThreeD == m_modelType;}
    bool Is2d() const {return ModelType::TwoD == m_modelType;}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      1/17
//=======================================================================================
struct V8ElementECContent
{
    V8ElementType m_v8ElementType;
    ECObjectsV8::IECInstancePtr m_primaryV8Instance {};
    std::vector<std::pair<ECObjectsV8::IECInstancePtr, BisConversionRule>> m_secondaryV8Instances;
    BisConversionRule m_elementConversionRule;
    bset<DgnV8Api::ElementId> const* m_namedGroupsWithOwnershipHintPerFile {};

    bool HasPrimaryInstance() const {return m_primaryV8Instance != nullptr;}
    bool HasSecondaryInstances() const {return !m_secondaryV8Instances.empty();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PartRangeKey
{
DRange3d m_range;

static int CompareDoubleArray(double const* array1, double const* array2, uint32_t count, double tolerance)
    {
    for (uint32_t i=0; i<count; i++, array1++, array2++)
        {
        if (*array1 < *array2 - tolerance)
            return -1;
        if (*array1 > *array2 + tolerance)
            return 1;
        }

    return 0;
    }

bool operator < (PartRangeKey const& rhs) const
    {
    static double tolerance = 1.0e-8;
    int compare;

    if (0 == (compare = CompareDoubleArray(&m_range.low.x, &rhs.m_range.low.x, 3, tolerance)))
        compare = CompareDoubleArray(&m_range.high.x, &rhs.m_range.high.x, 3, tolerance);

    return compare < 0;
    }

PartRangeKey(DRange3dCR range) : m_range(range) {}

}; // PartRangeKey

//=======================================================================================
//! A V8->DgnDb model mapping that has been "resolved", that is, with pointers to the loaded source and target models (if found).
//! This is stored in a multimap, with the V8 model's id as the key. (That is, the key is: V8File SyncInfoId + V8 ModelId.)
//! Note that the key is NOT the V8 model syncinfoid. We deliberately group all spatial transforms of a given model in a single
//! multimap node.
//! Note that an instance of this class refers to a DgnModel and a DgnV8Model that are 
//! owned by somebody else. Instances of this class do not add references to their target models.
//! @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct ResolvedModelMapping
{
    protected:
    DgnV8ModelP m_v8model;
    DgnModelP m_model;
    SyncInfo::V8ModelMapping m_mapping;
    DgnV8Api::DgnAttachment const* m_v8attachment;        // If this model was found via a reference attachment, this is the first such attachment to it.

    public:
    //! Construct a ResolvedModelMapping in an invalid state
    ResolvedModelMapping() : m_model(nullptr), m_v8model(nullptr) {}
    //! Construct a ResolvedModelMapping in an valid state
    ResolvedModelMapping(DgnModelR model, DgnV8ModelR v8Model, SyncInfo::V8ModelMapping const& mapping, DgnV8Api::DgnAttachment const* a) : m_model(&model), m_v8model(&v8Model), m_mapping(mapping), m_v8attachment(a) {}
    bool IsValid() const {return m_v8model != nullptr && m_mapping.IsValid();}
    DGNDBSYNC_EXPORT bool operator< (ResolvedModelMapping const &o) const;

    TransformCR GetTransform() const {return m_mapping.GetTransform();}
    void SetTransform(TransformCR t) {m_mapping.SetTransform(t);}
    DgnV8ModelR GetV8Model() const {BeAssert(IsValid()); return *m_v8model;}
    DgnV8Api::DgnAttachment const* GetV8Attachment() const {return m_v8attachment;}
    DgnModelR GetDgnModel() const {BeAssert(IsValid()); return *m_model;}
    SyncInfo::V8ModelId GetV8ModelId() const {BeAssert(IsValid()); return m_mapping.GetV8ModelId();}
    SyncInfo::V8ModelSyncInfoId GetV8ModelSyncInfoId() const {BeAssert(IsValid()); return m_mapping.GetV8ModelSyncInfoId();}
    SyncInfo::V8FileSyncInfoId GetV8FileSyncInfoId() const {BeAssert(IsValid()); return m_mapping.GetV8FileSyncInfoId();}
    SyncInfo::V8ModelSource const& GetV8ModelSource() const {BeAssert(IsValid()); return m_mapping.GetV8ModelSource();}
    SyncInfo::V8ModelMapping const& GetV8ModelMapping() const {BeAssert(IsValid()); return m_mapping;}
};

//=======================================================================================
//! An import "job" definition, including its subject element.
//! @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct ResolvedImportJob
{
protected:
    SyncInfo::ImportJob m_mapping;
    SubjectCPtr m_jobSubject;
public:
    ResolvedImportJob() {}
    ResolvedImportJob(SyncInfo::ImportJob const& j, SubjectCR s) : m_mapping(j), m_jobSubject(&s) {}
    ResolvedImportJob(SubjectCR s) : m_jobSubject(&s) {}

    bool IsValid() const {return m_jobSubject.IsValid();}

    void FromSelect(BeSQLite::Statement& stmt) {m_mapping.FromSelect(stmt); /* WIP_IMPORT_JOB -- resolve the subject */}

    SyncInfo::ImportJob& GetImportJob() {return m_mapping;}

    //! Get the root model for this job
    SyncInfo::V8ModelSyncInfoId GetV8ModelSyncInfoId() const { return m_mapping.GetV8ModelSyncInfoId(); }

    //! Get the job subject
    SubjectCR GetSubject() const {BeAssert(IsValid()); return *m_jobSubject;}

    //! Get the type of converter that created this job
    SyncInfo::ImportJob::Type GetConverterType() const {return m_mapping.GetType();}

    //! Get the name prefix that is used by this job
    Utf8StringCR GetNamePrefix() const { return m_mapping.GetPrefix(); }

};

//=======================================================================================
//! A V8->DgnDb model mapping that has been "resolved", along with the syncinfo
//! mapping for the modeled element.
//! @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct ResolvedModelMappingWithElement : ResolvedModelMapping
{
    DEFINE_T_SUPER(ResolvedModelMapping)

    protected:
    SyncInfo::V8ElementMapping m_modeledElementMapping;

    public:
    ResolvedModelMappingWithElement(ResolvedModelMapping const& v8mm, SyncInfo::V8ElementMapping const& elm) : T_Super(v8mm), m_modeledElementMapping(elm) {}
    ResolvedModelMappingWithElement() {}

    void SetResolvedModelMapping(ResolvedModelMapping const& rmm)
        {
        m_v8model = &rmm.GetV8Model();
        m_model = &rmm.GetDgnModel();
        m_mapping = rmm.GetV8ModelMapping();
        }

    void SetModeledElementMapping(SyncInfo::V8ElementMapping const& em) {m_modeledElementMapping = em;}
    SyncInfo::V8ElementMapping const& GetModeledElementMapping() const {return m_modeledElementMapping;}
};

//=======================================================================================
//! Captures the results of converting a V8 element into one or more BIM elements,
//! is used as a guide to updating the BIM and SyncInfo, and is annotated with the results
//! of the update.
//! @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct ElementConversionResults
{
    DgnElementPtr m_element;  //!< The DgnDb element created to represent the V8 element -- optional
    V8ECInstanceKey m_v8PrimaryInstance; //!< The primary ECInstance found on the V8 element
    bvector<bpair<V8ECInstanceKey,ECN::IECInstancePtr>> m_v8SecondaryInstanceMappings; //!< The secondary ECInstances found on the v8 element plus their converted IECInstance

    bvector<ElementConversionResults> m_childElements; //!< Child elements

    bool m_wasDiscarded; //!< OUTPUT: Set by SyncInfo 
    SyncInfo::V8ElementMapping m_mapping;   //!< OUTPUT: Set by SyncInfo after inserting or updating

    ElementConversionResults() : m_element(nullptr), m_wasDiscarded(false) {}
};

//=======================================================================================
//! @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct ViewFactory
{
    // Data that is used to define a ViewDefinition from V8 data. Note: all of the data is required!
    struct ViewDefinitionParams
    {
        ResolvedModelMapping m_modelMapping;
        Bentley::ViewInfoCR m_viewInfo;
        DisplayStylePtr m_dstyle;
        CategorySelectorPtr m_categories;
        Utf8StringCR m_name;
        Utf8String m_description;
        DPoint3d m_origin;
        DVec3d m_extents;
        RotMatrix m_rot;
        Transform m_trans;
        ViewDefinitionParams(Converter* c, Utf8StringCR n, ResolvedModelMapping const& m, Bentley::ViewInfoCR vi, bool is3d);

        void Apply(ViewDefinitionR) const;
        DgnModel& GetDgnModel() const {return m_modelMapping.GetDgnModel();}
        DgnV8ModelR GetV8Model() const {return m_modelMapping.GetV8Model();}
    };

    virtual bool _Is3d() const {return false;}
    virtual ViewDefinitionPtr _MakeView(Converter& converter, ViewDefinitionParams const&) = 0;
    virtual ViewDefinitionPtr _UpdateView(Converter& converter, ViewDefinitionParams const&, DgnViewId existingViewId) = 0;
};

//=======================================================================================
//! @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct IChangeDetector
{
    enum class ChangeType {None, Update, Insert};

    //! The results of calling _IsElementChanged 
    struct SearchResults
    {
        ChangeType m_changeType;
        SyncInfo::ElementProvenance m_currentElementProvenance;
        SyncInfo::V8ElementMapping m_v8ElementMapping;

        SearchResults() : m_changeType(ChangeType::None) {}
        DgnElementId GetExistingElementId() const {return m_v8ElementMapping.m_elementId;}
    };

    virtual ~IChangeDetector() {}

    //! @name Setup/teardown 
    //! @{
    //! Called at the very start of conversion, before any files, models, or elements are read.
    virtual void _Prepare(Converter&) {}

    //! Called at the very end of conversion, after all files, models, or elements have been processed by all phases.
    virtual void _Cleanup(Converter&) {}
    //! @}

    //! @name Skipping unchanged data
    //! @{
    //! Called to check if the specified file could be skipped (i.e., because it has not changed) without even having to open it (e.g., by looking at the Windows file time).
    virtual bool _ShouldSkipFileByName(Converter&, BeFileNameCR) = 0;

    //! Called to check if the specified file could be skipped (i.e., because it has not changed) by checking timestamps that may be stored in the file.
    virtual bool _ShouldSkipFile(Converter&, DgnV8FileCR) = 0;

    //! Called to check if an entire model could be skipped (i.e., because no element in the model is changed).
    //! @note Converter must not call this during the model-discovery step but only during the element-conversion step.
    virtual bool _AreContentsOfModelUnChanged(Converter&, ResolvedModelMapping const&) = 0;

    //! Used to choose one of many existing entries in SyncInfo
    typedef std::function<bool(SyncInfo::ElementIterator::Entry const&, Converter& converter)> T_SyncInfoElementFilter;

    //! Called by a converter to detect if an element is changed or new.
    //! @param[out] prov    Information about the element that can be used to decide how or if to update it in the bim and how to record the change in syncinfo
    //! @param[in] v8eh     A V8 Element
    //! @param[in] v8mm     Mapping info for the V8 model that contains this V8 element
    //! @param[in] filter   Optional. Chooses among existing elements in SyncInfo
    //! @return true if the element is new or has changed.
    virtual bool _IsElementChanged(SearchResults& prov, Converter&, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, T_SyncInfoElementFilter* filter = nullptr) = 0;

    virtual bool _ShouldSkipLevel(DgnCategoryId&, Converter&, DgnV8Api::LevelHandle const&, DgnV8FileR, Utf8StringCR dbCategoryName) = 0;

    //! @}

    //! @name Recording V8 content seen (so that we can deduce deletes)
    //! @{
    //! Called whenever a V8 element is encountered, regardless of whether it is converted or not.
    virtual void _OnElementSeen(Converter&, DgnElementId) = 0;

    void OnElementSeen(Converter& p, DgnElementP el) {if (el != nullptr) _OnElementSeen(p, el->GetElementId());}

    //! Called when a V8 model is discovered. This callback should be invoked during the model-discovery phase,
    //! before the elements in the specified model are converted.
    virtual void _OnModelSeen(Converter&, ResolvedModelMapping const&) = 0;

    //! Called when a V8 model is first mapped into the BIM.
    //! @param rmm The V8 model and the DgnModel to which it is mapped
    virtual void _OnModelInserted(Converter&, ResolvedModelMapping const& rmm) = 0;

    //! Called when a V8 view is discovered
    virtual void _OnViewSeen(Converter&, DgnViewId) = 0;
    //! @}

    //! @name  Inferring Deletions - call these methods after processing all models in a conversion unit. Don't forget to call the ...End function when done.
    //! @{
    virtual void _DetectDeletedElements(Converter&, SyncInfo::ElementIterator&) = 0;    //!< don't forget to call _DetectDeletedElementsEnd when done
    virtual void _DetectDeletedElementsInFile(Converter&, DgnV8FileR) = 0;              //!< don't forget to call _DetectDeletedElementsEnd when done
    virtual void _DetectDeletedElementsEnd(Converter&) = 0;
    virtual void _DetectDeletedModels(Converter&, SyncInfo::ModelIterator&) = 0;        //!< don't forget to call _DetectDeletedModelsEnd when done
    virtual void _DetectDeletedModelsInFile(Converter&, DgnV8FileR) = 0;                //!< don't forget to call _DetectDeletedModelsEnd when done
    virtual void _DetectDeletedModelsEnd(Converter&) = 0;
    virtual void _DetectDeletedViews(Converter&, SyncInfo::ViewIterator&) = 0;         //!< don't forget to call _DetectDeletedViewsEnd when done
    virtual void _DetectDeletedViewsInFile(Converter&, DgnV8FileR) = 0;                //!< don't forget to call _DetectDeletedViewsEnd when done
    virtual void _DetectDeletedViewsEnd(Converter&) = 0;
    //! @}

};

//=======================================================================================
//! The implementor is called by _FinishConversion. This is a good time to create ECRelationships, 
//! since at finish-conversion time, we know how all individual V8 elements and models were converted 
//! and what they were mapped to in the BIM.
//! @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct IFinishConversion
{
    //! This is invoked by _FinishConversion, after all models and elements (and ECRelationships) have been converted.
    virtual void _OnFinishConversion(Converter&) = 0;
};

struct ISChemaImportVerifier
{
    //! This is invoked by RetrieveV8ECSchemas to determine whether a given schema should be imported or not.  It is an alternative for those bridges that do not sublass
    //! from Converter themselves, and thus cannot override the Converter::_ShouldImportSchema virtual method.
    virtual bool _ShouldImportSchema(Utf8StringCR fullSchemaName, DgnV8ModelR v8Model) = 0;
};

//=======================================================================================
//! Base class for V8-BIM converters. This base class functions as a library of conversion
//! functions for a subclass to use. The subclass may convert spatial data and non-spatial (e.g., 
//! functional) data. 
//!
//! <h2>Conversion Scope</h2>
//! Converter does not know what set of files and models should be converted
//! or how to manage memory for them.
//! It is up to a <em>subclass</em> to implement the main loop, where it discovers files and models. The subclass
//! determines how various models relate to each other and to the BIM. Some subclasses discover
//! a whole set of files and models by following reference attachments and get them all into memory.
//! Other converters process a set of files one at a time, opening processing, and closing each
//! one in sequence.
//!
//! <h2>Change-Detection</h2>
//! Converter also defines the concept of change-detection and implements a number
//! of change-detection algorithms. ChangeDetection and SyncInfo also implement change-detection functions. 
//! A subclass uses these change-detection capabilities to implement a particular incremental update algorithm. 
//! That is, the subclass calls change-detection functions at the appropriate time as it
//! converts files, models, and elements. The subclass then calls the deletion-detection
//! functions at the appropriate time and with the appropriate scope.
//!
//! @bsiclass                                                    Sam.Wilson      07/2014
//=======================================================================================
struct Converter
{
    //! Configuration for the conversion process
    struct Config
    {
    private:
        BeXmlDomPtr         m_instanceDom;
        xmlXPathContextPtr  m_xpathContextRoot;
        Converter&          m_converter;
        bmap<Utf8String,bool>       m_boolLUT;
        bmap<Utf8String,Utf8String> m_utf8LUT;
        bmap<Utf8String,double>     m_doubleLUT;
        bmap<Utf8String,int64_t>    m_int64LUT;
        BeFileName m_instanceFilename;

        void CacheOptions();

    public:
        BeFileNameCR GetInstanceFilename() {return m_instanceFilename;}
        Converter& GetConverter() {return m_converter;}
        void ReadFromXmlFile();

        Config(BentleyApi::BeFileNameCR configFile, Converter& converter) : m_instanceFilename(configFile), m_converter(converter) {m_xpathContextRoot=nullptr;}
        ~Config();

    public:
        DGNDBSYNC_EXPORT BeXmlDom* GetDom() const;
        DGNDBSYNC_EXPORT bool OptionExists(BentleyApi::Utf8CP optionName) const;

        DGNDBSYNC_EXPORT Utf8String GetOptionValueString(BentleyApi::Utf8CP optionName, Utf8CP defaultVal) const;
        DGNDBSYNC_EXPORT void SetOptionValueString(Utf8CP optionName, Utf8CP value);

        DGNDBSYNC_EXPORT bool GetOptionValueBool(BentleyApi::Utf8CP optionName, bool defaultVal) const;
        DGNDBSYNC_EXPORT void SetOptionValueBool(Utf8CP optionName, bool value);

        DGNDBSYNC_EXPORT double GetOptionValueDouble(BentleyApi::Utf8CP optionName, double defaultVal) const;
        DGNDBSYNC_EXPORT void SetOptionValueDouble(Utf8CP optionName, double value);

        DGNDBSYNC_EXPORT int64_t GetOptionValueInt64(BentleyApi::Utf8CP optionName, int64_t defaultVal) const;
        DGNDBSYNC_EXPORT void SetOptionValueInt64(Utf8CP optionName, int64_t value);

        DGNDBSYNC_EXPORT Utf8String GetXPathString(BentleyApi::Utf8CP xpathExpression, Utf8CP defaultVal) const;
        DGNDBSYNC_EXPORT bool GetXPathBool(BentleyApi::Utf8CP xpathExpression, bool defaultVal) const;
        DGNDBSYNC_EXPORT double GetXPathDouble(BentleyApi::Utf8CP xpathExpression, double defaultVal) const;
        DGNDBSYNC_EXPORT int64_t GetXPathInt64(BentleyApi::Utf8CP xpathExpression, int64_t defaultVal) const;
        DGNDBSYNC_EXPORT BentleyStatus EvaluateXPath(Utf8StringR value, Utf8CP xpathExpression) const;
    };

    //! Parameters that specify the inputs to the conversion process
    struct Params : iModelBridge::Params
    {
        enum class CopyLevel {Never=0, IfDifferent=1, Always=2, UseConfig=3};

    private:
        bool m_skipUnchangedFiles;
        bool m_wantProvenanceInBim;
        bool m_isPowerplatformBased;
        bool m_processAffected;
        bool m_convertViewsOfAllDrawings;
        StableIdPolicy m_stableIdPolicy;
        BeFileName m_rootDir;           //!< Enables us to store *relative* paths to files in syncinfo
        BeFileName m_configFile;
        BeFileName m_configFile2;
        BeFileName m_embedDir;
        bvector<BeFileName> m_embedFiles;
        Utf8String m_password;
        DateTime m_time;
        BeFileName m_v8sdkRelativeDir;
        CopyLevel m_copyLevel = CopyLevel::UseConfig;
        BeFileName m_pwExtensionDll;
        BeFileName m_pwWorkDir;
        Utf8String m_pwUser;
        Utf8String m_pwPassword;
        Utf8String m_pwDataSource;

    public:
        Params() : m_v8sdkRelativeDir(L"DgnV8") // it's relative to the library's directory
            {
            m_stableIdPolicy=StableIdPolicy::ById;
            m_time = DateTime::GetCurrentTimeUtc();
            m_skipUnchangedFiles = true;
            m_isPowerplatformBased = false;
            m_wantProvenanceInBim = false;
            m_processAffected = false;
            m_convertViewsOfAllDrawings = true;
            }

        void SetInputRootDir(BentleyApi::BeFileNameCR fileName) {m_rootDir = fileName;}
        void SetConfigFile(BentleyApi::BeFileNameCR fileName) {m_configFile = fileName;}
        void SetConfigFile2(BentleyApi::BeFileNameCR fileName) {m_configFile2 = fileName;}
        void SetEmbedDir(BeFileNameCR embedDir) {m_embedDir = embedDir;}
        void SetEmbedFiles(bvector<BeFileName> const& embedFiles) {m_embedFiles = embedFiles;}
        void SetTime(DateTime tm) {m_time=tm;}
        void SetPassword(BentleyApi::Utf8CP pw) {m_password=pw;}
        void SetStableIdPolicy(StableIdPolicy val) {m_stableIdPolicy=val;}
        void SetV8SdkRelativeDir(BeFileNameCR v8SdkDir, bool isPowerplatformBased)
            {
            m_v8sdkRelativeDir = v8SdkDir; 
            m_isPowerplatformBased = isPowerplatformBased;
            }
        void SetSkipUnchangedFiles(bool v) {m_skipUnchangedFiles = v;}
        void SetWantProvenanceInBim(bool v) {m_wantProvenanceInBim = v;}
        void SetCopyLevel(CopyLevel v) {m_copyLevel = v;}
        void SetProjectWiseExtensionDll(BeFileNameCR pwExtensionDll) {m_pwExtensionDll = pwExtensionDll;}
        void SetProjectWiseWorkDir(BeFileNameCR pwWorkDir) {m_pwWorkDir = pwWorkDir;}
        void SetProjectWiseUser(Utf8CP pwUser) {m_pwUser = pwUser;}
        void SetProjectWisePassword(Utf8CP pwPassword) {m_pwPassword = pwPassword;}
        void SetProjectWiseDataSource(Utf8CP pwDataSource) {m_pwDataSource = pwDataSource;}
        void SetProcessAffected(bool processAffected) { m_processAffected = processAffected; }
        void SetConvertViewsOfAllDrawings(bool b) { m_convertViewsOfAllDrawings = b;}

        BeFileNameCR GetInputRootDir() const {return m_rootDir;}
        BeFileNameCR GetConfigFile() const {return m_configFile;}
        BeFileNameCR GetConfigFile2() const {return m_configFile2;}
        BeFileNameCR GetEmbedDir() const {return m_embedDir;}
        bvector<BeFileName> const& GetEmbedFiles() const {return m_embedFiles;}
        StableIdPolicy GetStableIdPolicy() const {return m_stableIdPolicy;}
        DateTime GetTime() const {return m_time;}
        Utf8String GetPassword() const {return m_password;}
        BeFileNameCR GetV8SdkRelativeDir() const {return m_v8sdkRelativeDir;}
        bool GetSkipUnchangedFiles() const {return m_skipUnchangedFiles;}
        CopyLevel GetCopyLevel() const {return m_copyLevel;}
        bool AlwaysCopyLevel() const {return m_copyLevel == CopyLevel::Always;}
        bool NeverCopyLevel() const {return m_copyLevel == CopyLevel::Never;}
        bool CopyLevelIfDifferent() const {return m_copyLevel == CopyLevel::IfDifferent;}
        BeFileNameCR GetProjectWiseExtensionDll() const {return m_pwExtensionDll;}
        BeFileNameCR GetProjectWiseWorkDir() const {return  m_pwWorkDir;}
        Utf8StringCR GetProjectWiseUser() const {return  m_pwUser;}
        Utf8StringCR GetProjectWisePassword() const {return  m_pwPassword;}
        Utf8StringCR GetProjectWiseDataSource() const {return  m_pwDataSource;}
        bool GetIsPowerplatformBased() const {return m_isPowerplatformBased;}
        bool GetWantProvenanceInBim() const {return m_wantProvenanceInBim;}
        bool GetProcessAffected() const { return m_processAffected; }
        bool GetConvertViewsOfAllDrawings() const {return m_convertViewsOfAllDrawings;}
    };


    //! Determines where V8 proxy graphics will be stored as elements.
    struct ProxyGraphicsDrawingFactory
        {
        //! Create the model where V8 proxy graphics from this attachment should be stored and insert it.
        //! @note the caller is also responsible for mapping the v8Attachment to the drawing element in syncinfo
        //! @param[in] v8Attachment The attachment that is being mined for proxy graphics
        //! @param[in] parentModel The V8 model that contains the attachment
        //! @param[in] converter The converter
        //! @return the mappings for the destination model and modeled element 
        virtual ResolvedModelMappingWithElement _CreateAndInsertDrawing(DgnAttachmentCR v8Attachment, ResolvedModelMapping const& parentModel, 
                                                                        Converter& converter) = 0;

        //! Return the model where V8 proxy graphics for this attachment should be stored.
        //! @param[in] v8AttachmentId The attachment that is being mined for proxy graphics
        //! @param[in] parentModel The V8 model that contains the attachment
        //! @param[in] converter The converter
        virtual ResolvedModelMapping _GetDrawing(DgnV8Api::ElementId v8AttachmentId, 
                                                 ResolvedModelMapping const& parentModel, Converter& converter) = 0;
        };

    //! Allow an app to monitor changes as they come in
    struct Monitor : RefCountedBase
    {
        //! Called when a V8 model is first mapped into the BIM.
        //! @param rmm The V8 model and the DgnModel to which it is mapped
        virtual void _OnModelInserted(ResolvedModelMapping const&) {}

        //! Called when a DgnModel is deleted
        virtual void _OnModelDelete(DgnModelR, SyncInfo::V8ModelMapping const&) {}
    };

    //! The severity of an issue
    enum class IssueSeverity
    {
        Fatal   = 1,
        Error   = 2,
        Warning = 3,
        Info    = 4,
    };

    //! The type of change operation
    enum class ChangeOperation
    {
        Create = 1,
        Update = 2,
        Delete = 3

    };

    //! Categories for issues
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START(IssueCategory,dgnv8_issueCategory)
        L10N_STRING(Compatibility)      // =="Compatibility"==
        L10N_STRING(ConfigXml)          // =="Config"==
        L10N_STRING(CorruptData)        // =="Corrupt Data"==
        L10N_STRING(DigitalRights)      // =="Digital Rights"==
        L10N_STRING(DiskIO)             // =="Disk I/O"==
        L10N_STRING(Filtering)          // =="Filtering"==
        L10N_STRING(InconsistentData)   // =="Inconsistent Data"==
        L10N_STRING(MissingData)        // =="Missing Data"==
        L10N_STRING(Sync)               // =="SyncInfo"==
        L10N_STRING(Unknown)            // ==""==
        L10N_STRING(Unsupported)        // =="Unsupported"==
        L10N_STRING(VisualFidelity)     // =="Visual Fidelity"==
        L10N_STRING(Briefcase)          // =="Briefcase"==
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

    //! A problem in the conversion process
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START(Issue,dgnv8_issue)
        L10N_STRING(BRepConversion)              // =="BRep processed as mesh."==
        L10N_STRING(CannotCreateChangesFile)     // =="Cannot create changes file"==
        L10N_STRING(CannotEmbedFont)             // =="Could not embed font type/name %i/'%s'; a different font will used for display."==
        L10N_STRING(CannotLoadModel)             // =="Cannot load default model for file [%s], skipping"==
        L10N_STRING(CannotUseStableIds)          // =="Cannot use DgnElementIds for this kind of file"==
        L10N_STRING(CantCreateModel)             // =="Cannot create model [%s]"==
        L10N_STRING(CantCreateProject)           // =="Cannot create project file [%s]"==
        L10N_STRING(CantCreateSyncInfo)          // =="Cannot create sync info [%s]"==
        L10N_STRING(CantOpenSyncInfo)            // =="Cannot open sync info [%s]"==
        L10N_STRING(ChangesFileInconsistent)     // =="The changes file [%s] is inconsistent with the syncinfo file"==
        L10N_STRING(ChangesFileInvalid)          // =="The changes file exists but cannot be opened. It may be invalid."==
        L10N_STRING(ConfigFileError)             // =="[%s] error at [%d,%d], %s"==
        L10N_STRING(ConfigUsingDefault)          // =="Using default configuration."==
        L10N_STRING(ConvertFailure)              // =="Failed to convert [%s]"==
        L10N_STRING(DigitalRightsAccessDenied)   // =="You are not permitted to access this file. This file is protected."==
        L10N_STRING(DigitalRightsNoExportRight)  // =="You are not permitted to publish this file. This file is protected, and the Export right is not granted."==
        L10N_STRING(ElementFilteredOut)          // =="Element [%s] was not converted."==
        L10N_STRING(ElementsMayNotDisplay)       // ==" Some elements may not display properly."== <<Leading space is necessary>>
        L10N_STRING(EmbedFileError)              // =="Error embedding file: %s"==
        L10N_STRING(EmbeddedDirOrFileIsInvalid)  // =="Could not embed invalid directory/file: %s"==
        L10N_STRING(EmbeddedFileAlreadyExists)   // =="An embedded file with the name '%s' already exists; it will not be re-embedded."==
        L10N_STRING(EmbeddedFileTooBig)          // =="Not enough memory to embed file with the name '%s'."==
        L10N_STRING(EmbeddedRasterError)         // =="Can't read embedded raster: %s"==
        L10N_STRING(ExtractedGraphicCreationFailure) // =="Failed to create new DrawingElement in Model %s for ECClassId %llu, Category %llu, Code value: %s"==
        L10N_STRING(ExtractedGraphicBuildFailure) // =="Failed to build geometry for DrawingElement for V8 element %llu in model %s"==
        L10N_STRING(ExtractedGraphicMissingElement) // =="Failed to find V8 element %llu in model '%s' (%s) when creating extraction graphic"==
        L10N_STRING(Error)                       // =="Error: %s"==
        L10N_STRING(FatalError)                  // =="A fatal error has stopped the conversion"==
        L10N_STRING(FileFilteredOut)             // =="File [%s] was not converted."==
        L10N_STRING(FileInUse)                   // =="The file is in use"==
        L10N_STRING(FileNotFound)                // =="The file was not found"==
        L10N_STRING(FileReadOnly)                // =="The file is read-only"==
        L10N_STRING(FontEmbedError)              // =="Could not embed %s font '%s'. Some elements may not display properly."==
        L10N_STRING(FontIllegalNumber)           // =="Illegal font number %u."==
        L10N_STRING(FontMissing)                 // =="Missing %s font '%s'. Some elements may not display properly."==
        L10N_STRING(FontMissingRsc)              // =="Missing RSC font: %u."==
        L10N_STRING(FontNotEmbedded)             // =="Did not embed %s font '%s' due to importer configuration. Some elements may not display properly."==
        L10N_STRING(FontNumberError)             // =="Could not resolve font number %u."==
        L10N_STRING(IllegalUnits)                // =="File does not have valid linear units"==
        L10N_STRING(InvalidLevel)                // =="Invalid Level [%s] could not be converted"==
        L10N_STRING(InvalidRange)                // =="Invalid Range"==
        L10N_STRING(InvisibleElementFilteredOut) // =="Element [%s] is invisible. It was not converted."==
        L10N_STRING(LargeBRep)                   // =="Large BRep processed as surfaces."==
        L10N_STRING(LevelDefinitionChange)       // =="Level [%s] has changed in [%s]: %s. Update is not possible. You must do a full conversion."==
        L10N_STRING(LevelDisplayInconsistent)    // =="Level [%s] is turned on for some attachments but is turned off for [%s]"==
        L10N_STRING(LevelNotFoundInRoot)         // =="Level [%s] found in tile file [%s] but not in root file [%s]. The root file must define all levels."==
        L10N_STRING(LevelSymbologyInconsistent)  // =="Level [%s] has a different definition in [%s] than in other files or attachments: %s"==
        L10N_STRING(LineStyleNumberError)        // =="Could not resolve line style number %u."==
        L10N_STRING(Message)                     // =="%s"==
        L10N_STRING(MismatchGcs)                 // =="The Geographic Coordinate Systems of the DgnDb and the DgnV8 file are not based on equivalent projections"==
        L10N_STRING(MissingGCS)                  // =="The project is not geo-located because no Geographic Coordinate System was detected or supplied."==
        L10N_STRING(MissingLevel)                // =="Missing Level %d"==
        L10N_STRING(MissingLsDefinition)         // =="Could not find definition for line style [%s]. Some elements may not display properly."==
        L10N_STRING(MissingLsDefinitionFile)     // =="Could not find definition file %s. Some elements may not display properly."==
        L10N_STRING(ModelFilteredOut)            // =="Model [%s] was not converted."==
        L10N_STRING(NotADgnDb)                   // =="The file is not a DgnDb"==
        L10N_STRING(NotRecognizedFormat)         // =="File [%s] is not in a recognized format"==
        L10N_STRING(PointCloudFile)              // =="PointCloud file [%s]"==
        L10N_STRING(RasterCreationError)         // =="Can't create raster file: %s"==
        L10N_STRING(RasterFile)                  // =="Raster file [%s]"==
        L10N_STRING(RootModelChanged)            // =="The original root model was deleted, has changed units, or a different input-gcs has been specified."==
        L10N_STRING(RootModelMustBePhysical)     // =="Root model [%s] is not a 3D model. Therefore, no spatial models or elements will be converted. Drawings and sheets may be converted."==
        L10N_STRING(Detected3dViaAttachment)     // =="An attachment to a 3D model [%s] has been detected. The root model is not a 3D model, however. This indicates that the root model is incorrect."==
        L10N_STRING(SaveError)                   // =="An error occurred when saving changes (%s)"==
        L10N_STRING(SeedFileMismatch)            // =="Seed file [%s] does not match target [%s]"==
        L10N_STRING(MissingSeedFile)             // =="Missing seed file [%s]"==
        L10N_STRING(SyncInfoInconsistent)        // =="The syncInfo file [%s] is inconsistent with the project"==
        L10N_STRING(SyncInfoTooNew)              // =="Sync info was created by a later version"==
        L10N_STRING(TileHasWrongUnits)           // =="Tile file [%s] has different units from root file [%s]."==
        L10N_STRING(UpdateDoesNotChangeClass)    // =="Update cannot change the class of an element. Element: %s. Proposed class: %s."==
        L10N_STRING(V8FileError)                 // =="DgnV8 file open error %s."==
        L10N_STRING(ViewNoneFound)               // =="No view was found"==
        L10N_STRING(PointCloudCreationError)     // =="Can't create point cloud file: %s"==
        L10N_STRING(DwgFileIgnored)              // =="master DWG/DXF file [%s] is ignored - use DwgImporter to convert these"==
        L10N_STRING(FailedLoadingFileIO)         // =="Unable to load file handler %s"==
        L10N_STRING(MissingFileIOImplementer)    // =="File handler %s missing V8 file type implementation"==
        L10N_STRING(TemporaryDirectoryNotFound)  // =="Failed to find/create temporary directory %s"==
        L10N_STRING(RDSUninitialized)            // =="Failed to initialize RDSRequestManager"==
        L10N_STRING(RDSUploadFailed)             // =="Failed to upload tileset to Reality Data Server"==

        L10N_STRING(InitProjectWiseLinkError)      // =="Could not initialize ProjectWise extension. Any ProjectWise documents that are target of links will not be embedded."==
        L10N_STRING(TerminateProjectWiseLinkError) // =="Could not terminate ProjectWise extension."==
        L10N_STRING(FailedToImportDocLinkError)    // =="Failed to import document %s"==
        L10N_STRING(FailedToImportLinkError)      // =="Failed to import link on element %llu in file '%s' (new ElementId: %llu)"==
        L10N_STRING(InvalidSheetAttachment)      // =="Sheet [%s] - Unsupported sheet attachment: [%s]"==
        L10N_STRING(UnrecognizedDetailingSymbol) // =="[%s] is an unrecognized kind of detailing symbol. Capturing graphics only."==
        L10N_STRING(UnsupportedPrimaryInstance)   // =="[%s] has an unsupported primary ECInstance. Capturing graphics only."==
        L10N_STRING(WrongBriefcaseManager)        // =="You must use the UpdaterBriefcaseManager when updating a briefcase with the converter"==
        L10N_STRING(SchemaLockFailed)           // =="SchemaLockFailed"==
        L10N_STRING(CouldNotAcquireLocksOrCodes) // =="CouldNotAcquireLocksOrCodes"==
        L10N_STRING(ImportTargetECSchemas)      // =="Failed to import V8 ECSchemas"==

        IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

    //! Progress messages for the conversion process
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START(ProgressMessage,dgnv8_progress)
        L10N_STRING(STEP_CLEANUP_EMPTY_TABLES)         // =="Cleaning up empty tables"==
        L10N_STRING(STEP_COMPACTING)                   // =="Compacting File"==
        L10N_STRING(STEP_CONVERTING_STYLES)            // =="Converting Styles and Levels "==
        L10N_STRING(STEP_CONVERTING_ELEMENTS)          // =="Converting Elements"==
        L10N_STRING(STEP_CONVERTING_DRAWINGS)          // =="Converting Drawings"==
        L10N_STRING(STEP_CONVERTING_SHEETS)            // =="Converting Sheets"==
        L10N_STRING(STEP_CONVERTING_VIEWS)             // =="Converting Views"==
        L10N_STRING(STEP_CREATE_IMODEL)                // =="Creating .imodel File"==
        L10N_STRING(STEP_CREATE_THUMBNAILS)            // =="Creating Thumbnails"==
        L10N_STRING(STEP_CREATE_REALITY_MODEL_TILES)   // =="Creating Reality Model Tiles"==
        L10N_STRING(STEP_CREATING)                     // =="Creating DgnDb [%s]"==
        L10N_STRING(STEP_EMBED_FILES)                  // =="Embedding Files"==
        L10N_STRING(STEP_EMBED_FONTS)                  // =="Embedding Fonts"==
        L10N_STRING(STEP_IMPORT_SCHEMAS)               // =="Importing Schemas"==
        L10N_STRING(STEP_DISCOVER_ECSCHEMAS)           // =="Detecting ECClasses"==
        L10N_STRING(STEP_CREATE_CLASS_VIEWS)           // =="Creating ECClass Views"==
        L10N_STRING(STEP_MERGING_MODELS)               // =="Merging Models"==
        L10N_STRING(STEP_UPDATING)                     // =="Updating DgnDb"==
        L10N_STRING(TASK_CONVERTING_MODEL)             // =="Model: %s"==
        L10N_STRING(TASK_CONVERTING_SHEET_ATTACHMENTS) // =="Sheet: %s"==
        L10N_STRING(TASK_CREATING_THUMBNAIL)           // =="View: %s"==
        L10N_STRING(TASK_CONVERTING_RASTER)            // =="Raster: %s"==
        L10N_STRING(TASK_CONVERTING_POINTCLOUD)        // =="Point Cloud: %s"==
        L10N_STRING(TASK_CONVERTING_MATERIALS)         // =="Converting Materials"==
        L10N_STRING(TASK_READING_V8_ECSCHEMA)          // =="Reading ECSchema from %s"==
        L10N_STRING(TASK_MERGING_V8_ECSCHEMA)          // =="Merging ECSchema %s"==
        L10N_STRING(TASK_ANALYZE_EC_CONTENT)           // =="Analyzing: %s"==
        L10N_STRING(STEP_LOADING_V8)                   // =="Loading V8 files"==
        L10N_STRING(TASK_LOADING_REALDWG)              // =="Loading RealDWG %s"==
        L10N_STRING(TASK_CONVERTING_RELATIONSHIPS)     // =="Converting Relationships"==
        L10N_STRING(TASK_V8_PROGRESSS)                 // =="%s"==
        L10N_STRING(STEP_EMBEDDING_FILES)              // =="Embedding Files"==
        L10N_STRING(TASK_FILLING_V8_MODELS)            // =="Filling"==
        IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

    //! Other arbitrary strings required by the conversion process
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START(ConverterDataStrings,dgnv8_converterDataStrings)
        L10N_STRING(V8StyleNone) // =="V8 Default Style"==
        L10N_STRING(V8StyleNoneDescription) // =="Created from V8 active settings to handle Style (none)"==
        L10N_STRING(LinkModelDefaultName) // =="Default Link Model"==
        L10N_STRING(RDS_Description) // =="Reality Model Tileset for %s"==
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

    //! Reports conversion issues
    struct IssueReporter
    {
    public:
        struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
            {
            private:
                IssueReporter& m_issueReporter;

                virtual void _OnIssueReported(BentleyApi::Utf8CP message) const override;

            public:
                explicit ECDbIssueListener(IssueReporter& issueReporter) : BeSQLite::EC::ECDb::IIssueListener(), m_issueReporter(issueReporter) {}
                ~ECDbIssueListener() {}
            };

    private:
        bool        m_triedOpenReport;
        BeFileName  m_reportFileName;
        FILE*       m_reportFile;
        ECDbIssueListener m_ecdbIssueListener;

        BentleyStatus OpenReportFile();

        static Utf8CP ToString(IssueSeverity);

    public:
        explicit IssueReporter(BeFileNameCR filename)
            : m_triedOpenReport(false), m_reportFileName(filename), m_reportFile(nullptr), m_ecdbIssueListener(*this) {}

        ~IssueReporter() {CloseReport();}

        void CloseReport();
        bool HasIssues() const {return m_triedOpenReport;}
        BeFileNameCR GetFileName() const {return  m_reportFileName;}
        ECDbIssueListener const& GetECDbIssueListener() const {return m_ecdbIssueListener;}
        void Report(IssueSeverity severity, IssueCategory::StringId category, Utf8CP details, Utf8CP context);

        static Utf8String FmtFileBaseName(DgnV8Api::DgnFile const& ff);
        static Utf8String FmtElement(DgnV8Api::ElementHandle const&);
        static Utf8String FmtElement(DgnElementCR);
        static Utf8String FmtModel(DgnModelCR);
        static Utf8String FmtModel(DgnV8ModelCR);
        static Utf8String FmtModelRef(DgnModelRefCR);
        static Utf8String FmtAttachment(DgnAttachmentCR);
        static Utf8String FmtDouble(double value);
        static Utf8String FmtDoubles(double const* values, size_t count);
        static Utf8String FmtDPoint3d(DPoint3d const& pt);
        static Utf8String FmtTransform(BentleyApi::Transform const& trans);
    };

    typedef bpair<DgnFontType, Utf8String> T_WorkspaceFontKey;
    typedef bmap<T_WorkspaceFontKey, DgnFontPtr> T_WorkspaceFonts;
    typedef bmap<uint32_t, DgnFontPtr> T_V8EmbeddedRscFontMap;
    typedef bpair<DgnV8Api::DgnFile*, uint32_t> T_FontRemapKey;
    typedef bmap<T_FontRemapKey, DgnFontPtr> T_FontRemap;
    typedef bmap<void*, RenderMaterialId> T_MaterialRemap;
    typedef bpair<Utf8String, Utf8String> T_MaterialNameKey;
    typedef bmap<T_MaterialNameKey, RenderMaterialId> T_MaterialNameRemap;
    typedef bmap<WString, DgnTextureId> T_TextureFileNameRemap;
    typedef std::multimap<PartRangeKey, DgnGeometryPartId> RangePartIdMap;

protected:
    bool                 m_promoteTo3d = false;
    mutable bool         m_hadError = false;
    mutable bool         m_wasAborted = false;
    bool                 m_hasLoadedWorkspaceFonts = false;
    bool                 m_skipECContent = false;
    bool                 m_addDebugDgnCodes = false;
    bool                 m_rootTransHasChanged = false;
    bool                 m_spatialTransformCorrectionsApplied = false;
    uint32_t             m_elementsConverted = 0;
    uint32_t             m_elementsDiscarded = 0;
    uint32_t             m_elementsSinceLastSave = 0;
    Transform            m_rootTrans; // this is usually identity. It is non-identity in the case where we are pulling in a new dgnv8 file and we need to do a GCS or other coordinate transform to map it in. 
    Transform            m_rootTransChange;
    DgnDbPtr             m_dgndb;
    Config               m_config;
    SyncInfo             m_syncInfo;
    StableIdPolicy       m_currIdPolicy;
    mutable IssueReporter m_issueReporter;
    DgnCategoryId        m_uncategorizedCategoryId;
    DgnCategoryId        m_uncategorizedDrawingCategoryId;
    DgnModelId           m_groupModelId;
    DgnModelId           m_drawingListModelId;
    DgnModelId           m_sheetListModelId;
    CodeSpecId           m_businessKeyCodeSpecId;
    bvector<ImportRule>  m_modelImportRules;
    LineStyleConverterPtr m_lineStyleConverter;
    DgnViewId            m_defaultViewId;
    BeFileName           m_appRootPath;
    T_WorkspaceFonts     m_workspaceFonts;
    T_V8EmbeddedRscFontMap m_v8EmbeddedRscFontMap;
    bset<BeFileName>     m_tempFontFiles;
    T_FontRemap          m_fontRemap;
    T_MaterialRemap      m_materialRemap;
    T_MaterialNameRemap  m_materialNameRemap;
    bset<RenderMaterialId>  m_materialUsed;
    T_TextureFileNameRemap m_textureFileNameRemap;
    mutable ECQueryPtr   m_selectAllQueryV8;
    RefCountedPtr<Monitor> m_monitor;
    bmap<DgnViewId, int> m_viewNumberMap;
    LinkConverter*       m_linkConverter = nullptr;
    RangePartIdMap       m_rangePartIdMap;
    DgnV8Api::IDgnProgressMeter* m_v8meter = nullptr;
    ElementConverter*   m_elementConverter = nullptr;
    ElementAspectConverter* m_elementAspectConverter;
    bvector<IFinishConversion*> m_finishers;
    bvector<ISChemaImportVerifier*> m_schemaImportVerifiers;
    bmap<DgnClassId, bvector<ECN::ECClassId>> m_classToAspectMappings;
    DgnModelId          m_jobDefinitionModelId;
    DgnElementId         m_textStyleNoneId;
    bset<DgnModelId>    m_unchangedModels;
    bmap<DgnModelId, bpair<Utf8String, SyncInfo::V8FileSyncInfoId>>    m_modelsRequiringRealityTiles;;

    DGNDBSYNC_EXPORT Converter(Params const&);
    DGNDBSYNC_EXPORT ~Converter();

    DGNDBSYNC_EXPORT virtual SyncInfo::V8ElementMapping _FindFirstElementMappedTo(DgnV8Api::DisplayPath const& proxyPath, bool tail, IChangeDetector::T_SyncInfoElementFilter* filter = nullptr);
    virtual DgnV8Api::ModelInfo const& _GetModelInfo(DgnV8ModelCR v8Model) { return v8Model.GetModelInfo(); }
    virtual bool _ShouldImportSchema(Utf8StringCR fullSchemaName, DgnV8ModelR v8Model) { return true; }
    virtual void _OnSheetsConvertViewAttachment(ResolvedModelMapping const& v8SheetModelMapping, DgnAttachmentR v8DgnAttachment) {}

public:
    virtual Params const& _GetParams() const = 0;
    virtual Params& _GetParamsR() = 0;

    bool SkipECContent() const {return m_skipECContent;}

    //! Add a callback to be invoked by RetrieveV8ECSchemas
    //! @param v    Verifier to be invoked by RetrieveV8ECSchemas to determine whether a given schema should be imported
    void AddSchemaImportVerifier(ISChemaImportVerifier& v) { m_schemaImportVerifiers.push_back(&v); }

    //! Allows a bridge to determine whether a particular schema should be imported or not
    bool ShouldImportSchema(Utf8StringCR fullSchemaName, DgnV8ModelR v8Model);
    //! @}

    //! This returns false if the V8 file should not be converted by the bridge.
    DGNDBSYNC_EXPORT bool IsFileAssignedToBridge(DgnV8FileCR v8File) const;

    //! This returns true if the specified BIM model may be converted or otherwise processed by the current job subject.
    DGNDBSYNC_EXPORT bool IsBimModelAssignedToJobSubject(DgnModelId) const;

    //! This returns true if the v8Model may be converted by the current job subject.
    DGNDBSYNC_EXPORT bool IsV8ModelAssignedToJobSubject(DgnV8ModelCR) const;

    bool HasRootTransChanged() const {return m_rootTransHasChanged;}

    void SetIsUpdating(bool b) {_GetParamsR().SetIsUpdating(b);}

    static DgnDbStatus InsertLinkTableRelationship(DgnDbR db, Utf8CP relClassName, DgnElementId source, DgnElementId target, Utf8CP schemaName = BIS_ECSCHEMA_NAME)
        {
        return iModelBridge::InsertLinkTableRelationship(db, relClassName, source, target, schemaName);
        }

    DGNDBSYNC_EXPORT static void SetDllSearchPath(BentleyApi::BeFileNameCR v8Path, BentleyApi::BeFileNameCP realdwgPath = nullptr);

    //! Add a callback to be invoked by _FinishConversion
    //! @param f    Finisher to be invoked by _FinishConversion. @note f must not be freed by the caller and will not be freed by the Converter.
    void AddFinisher(IFinishConversion& f) {m_finishers.push_back(&f);}

    virtual bool _WantProvenanceInBim() {return _GetParams().GetWantProvenanceInBim();}

    //! Get the DMS document properties for a specified file, if available.
    void GetDocumentProperties(iModelBridgeDocumentProperties& docProps, BeFileNameCR localFilename)
        {
        if (nullptr != _GetParams().GetDocumentPropertiesAccessor())
            _GetParams().GetDocumentPropertiesAccessor()->_GetDocumentProperties(docProps, localFilename); 
        }


    //! @name Graphics Conversion Utilties
    //! @{
    DGNDBSYNC_EXPORT static void ConvertLineStyleParams(Render::LineStyleParams& lsParams, DgnV8Api::LineStyleParams const* v8lsParams, double uorPerMeter, double componentScale, double modelLsScale);

    DGNDBSYNC_EXPORT static void ConvertCurveVector(BentleyApi::CurveVectorPtr& clone, Bentley::CurveVectorCR v8Curves, TransformCP v8ToDgnDbTrans);

    DGNDBSYNC_EXPORT static void ConvertMSBsplineSurface(BentleyApi::MSBsplineSurfacePtr& clone, Bentley::MSBsplineSurfaceCR v8Entity);

    DGNDBSYNC_EXPORT static void ConvertSolidPrimitive(BentleyApi::ISolidPrimitivePtr& clone, Bentley::ISolidPrimitiveCR v8Entity);

    DGNDBSYNC_EXPORT static void ConvertPolyface(BentleyApi::PolyfaceHeaderPtr& clone, Bentley::PolyfaceQueryCR v8Entity);

    DGNDBSYNC_EXPORT static void ConvertTextString(TextStringPtr& clone, Bentley::TextStringCR v8Text, DgnFileR dgnFile, Converter& converter);

    DGNDBSYNC_EXPORT static void ConvertSolidKernelEntity(IBRepEntityPtr& clone, Bentley::ISolidKernelEntityCR v8Entity);

    DGNDBSYNC_EXPORT void InitGeometryParams(Render::GeometryParams& params, DgnV8Api::ElemDisplayParams& paramsV8, DgnV8Api::ViewContext& context, bool is3d, SyncInfo::V8ModelSource v8Model);

    void InitLineStyle(Render::GeometryParams& params, DgnModelRefR styleModelRef, int32_t srcLineStyleNum, DgnV8Api::LineStyleParams const* v8lsParams);

    DGNDBSYNC_EXPORT bool InitPatternParams(PatternParamsR pattern, DgnV8Api::PatternParams const& patternV8, Bentley::bvector<DgnV8Api::DwgHatchDefLine> const& defLinesV8, Bentley::DPoint3d& origin, DgnV8Api::ViewContext& context);

    //! @}

    //! @name V8Files
    //! @{

    //! Open the specified V8File
    DGNDBSYNC_EXPORT static DgnFilePtr OpenDgnV8File(DgnV8Api::DgnFileStatus&, BeFileNameCR, Utf8CP password);

    //! Open the specified V8File
    DGNDBSYNC_EXPORT DgnFilePtr OpenDgnV8File(DgnV8Api::DgnFileStatus&, BeFileNameCR);

    //! @private
    //! This is called in a separate process to check bridge file affinity only.
    DGNDBSYNC_EXPORT static BentleyStatus GetAuthoringFileInfo(WCharP buffer, const size_t bufferSize, iModelBridgeAffinityLevel& affinityLevel, BentleyApi::BeFileName const& sourceFileName);
    DGNDBSYNC_EXPORT static void InitializeDgnv8Platform(BentleyApi::BeFileName const& thisLibraryPath);
    DGNDBSYNC_EXPORT static void GetAffinity(WCharP buffer, const size_t bufferSize, iModelBridgeAffinityLevel& affinityLevel,WCharCP affinityLibraryPathStr, WCharCP sourceFileNameStr);
    
    //! Make sure that the specified V8 file is registered in SyncInfo and then return the ID assigned to it by SyncInfo.
    //! This function \em caches the result in appdata on the file.
    //! @see GetV8FileSyncInfoIdFromAppData, GetV8FileSyncInfoId
    DGNDBSYNC_EXPORT virtual SyncInfo::V8FileProvenance _GetV8FileIntoSyncInfo(DgnV8FileR, StableIdPolicy);

    //! Compute the code value and URI that should be used for a RepositoryLink to the specified file
    void ComputeRepositoryLinkCodeValueAndUri(Utf8StringR Code, Utf8StringR uri, DgnV8FileR file);
    
    //! Create a RepositoryLink to represent this file in the BIM and cache it in memory
    DgnElementId WriteRepositoryLink(DgnV8FileR file);

    //! Look in the in-memory cache for the RepositoryLink that represents this file in the BIM
    DGNDBSYNC_EXPORT DgnElementId GetRepositoryLinkFromAppData(DgnV8FileCR file);

    void SetRepositoryLinkInAppData(DgnV8FileCR file, DgnElementId rlinkId);

    //! Get the SyncInfoId for a V8 file that was previously registered by a call to _GetV8FileIntoSyncInfo. This is a very fast
    //! query on the file's appdata. This will return an invalid ID if the file has not yet been queried by _GetV8FileIntoSyncInfo during this session.
    //! @see _GetV8FileIntoSyncInfo which must be called first, in order for this function to work.
    //! @see GetV8FileSyncInfoId for a short-cut method
    DGNDBSYNC_EXPORT static SyncInfo::V8FileSyncInfoId GetV8FileSyncInfoIdFromAppData(DgnV8FileCR);

    DGNDBSYNC_EXPORT static void DiscardV8FileSyncInfoAppData(DgnV8FileR);

    //! Short cut method that first calls GetV8FileSyncInfoIdFromAppData to see if the file's SyncInfoId is already cached. 
    //! If not, this function calls _GetV8FileIntoSyncInfo .
    SyncInfo::V8FileSyncInfoId GetV8FileSyncInfoId(DgnV8FileR);

    //! Open the specified V8File and make sure that it is registered in SyncInfo. 
    //! <em>This function should not be used for spatial models.</em>
    DgnFilePtr OpenAndRegisterV8FileForDrawings(DgnV8Api::DgnFileStatus &, BeFileNameCR);

    //! Short cut to get the StableIdPolicy used for the specified file from info cached in appdata.
    //! @see _GetV8FileIntoSyncInfo which must be called first, in order for this function to work.
    static StableIdPolicy GetIdPolicyFromAppData(DgnV8FileCR);

    //! Test if the specified file is from the V8 generation. If not, it may be from the V7 generation.
    bool IsV8Format(DgnV8FileR dgnFile) const;

    //! Call this if you decide not to convert anything in the specified file.
    void CaptureFileDiscard(DgnV8FileR);
    
    //! Convenience method to get the basename of the specified filename
    static Utf8String GetFileBaseName(DgnV8FileCR ff);
    
    //! @}
    
    //! @name The target DgnDb
    //! @{
    bool IsDgnDbValid() const {return m_dgndb.IsValid();}

    void SetDgnDb(DgnDbR db) {m_dgndb=&db;}
    DgnDb& GetDgnDb() const {return *m_dgndb;}

    //! @}

    //! @name The ImportJob
    //! @{

    //! The ImportJob's subject element. Put all documentlist models and PhsyicalPartitions under the job subject (not under the root subject).
    virtual SubjectCR _GetJobSubject() const = 0;
    virtual SubjectCR _GetSpatialParentSubject() const {return _GetJobSubject();}
    SubjectCR GetJobSubject() const {return _GetJobSubject();}

    //! Get the one and only hierarchy subject for the job
    SubjectCPtr GetJobHierarchySubject();

    //! Find or create the standard set of job-specific partitions, documentlistmodels, and other definition elements that the converter uses to organize the converted information.
    //! Note that this cannot be done until the ImportJob has been created (when creating a new ImportJob) or read from syncinfo (when updating).
    void GetOrCreateJobPartitions();

    //! Look up the ImportJob that was created by a prior run of the converter for this V8 file, assuming that there is only one. This method
    //! fails if there is no ImportJob or if there is more than one ImportJob.
    ResolvedImportJob FindSoleImportJobForFile(DgnV8FileR rootFile);

    //! Look up the ImportJob that was created by a prior run of the converter for this V8 model, if any.
    ResolvedImportJob FindImportJobForModel(DgnV8ModelR rootModel);

    ResolvedImportJob GetResolvedImportJob(SyncInfo::ImportJob const&);

    void ValidateJob();

    DGNDBSYNC_EXPORT DefinitionModelPtr GetJobDefinitionModel();
    DgnModelId           GetDrawingListModelId() {return m_drawingListModelId;}
    DgnModelId           GetSheetListModelId() {return m_sheetListModelId;}

    //! @}

    //! @name DgnDb properties
    //! @{
    BentleyStatus GenerateThumbnails();
    BentleyStatus GenerateRealityModelTilesets();
    void  StoreRealityTilesetTransform(DgnModelR model, TransformCR tilesetToDb);


    bool ThumbnailUpdateRequired(ViewDefinition const& view);

    void CopyExpirationDate(DgnV8FileR);
    //! @}


    //! @name Convert V8 Levels
    //! @{
    
    //! Convert all of the levels in the specified V8 file to SpatialCategories in the output BIM. The name of each
    //! category created will be the same as the name of the input level.
    DGNDBSYNC_EXPORT void ConvertAllSpatialLevels(DgnV8FileR v8file);
    
    void ComputeSubCategoryAppearanceFromLevel(DgnSubCategory::Appearance&, DgnV8Api::LevelHandle const&);
    void MakeSubCategoryInvisibleInView(ViewDefinitionR theView, DgnSubCategoryId subcatId);
    DgnSubCategoryId ConvertLevelToSubCategory(DgnV8Api::LevelHandle const&, DgnV8ModelCR, DgnCategoryId);
    DgnSubCategoryId GetOrCreateSubCategoryId(DgnCategoryId, Utf8CP subCatName, DgnSubCategory::Appearance const& appear = DgnSubCategory::Appearance());
    DgnCategoryId ConvertLevelToCategory(DgnV8Api::LevelHandle const& level, DgnV8FileR v8File, bool is3d);
    DgnCategoryId ConvertDrawingLevel(DgnV8FileR v8file, DgnV8Api::LevelId);
    void ParseLevelConfigAndUpdateParams(Config&);
    DGNDBSYNC_EXPORT void CheckNoLevelChange(DgnV8Api::LevelHandle const&, DgnCategoryId, DgnV8FileR);
    virtual void _OnUpdateLevel(DgnV8Api::LevelHandle const& level, DgnCategoryId cat, DgnV8FileR file) = 0;
    bool IsDefaultLevel(DgnV8Api::LevelHandle const&);
    bool IsDrawingCategory(DgnCategoryId);
    bool IsSpatialCategory(DgnCategoryId);
    // Get the level of this element. If it is a complex header, then its children are searched for a valid level.
    static uint32_t GetV8Level(DgnV8EhCR v8Eh);
    //! @}

    //! @name Convert V8 Views
    //! @{
    
    //! Should the converter reject this view?
    //! @param[in] v8View the DgnV8 view
    //! @return true if the view should be ignored and not converted.
    virtual bool _FilterOutView(DgnV8ViewInfoCR v8View) {return false;}
    virtual Utf8String _ComputeViewName(Utf8StringCR defaultName, DgnV8ViewInfoCR) {return defaultName;}

    void HandleLevelAppearanceInconsistency(ViewDefinitionR, DgnAttachmentCR, DgnV8Api::LevelId, DgnCategoryId, DgnSubCategoryId, bool isV8LevelOn);
    //! Interpret the level mask from a V8 view for the specified attachment. The result will be to add the corresponding categories to \a viewDef that are on in the V8 view.
    //! This function may also create a new SubCategory and add an override for it to the view if the on/off status or symbology of the level was modified by the attachment.
    //! @param viewDef      The BIM view that holds the CategorySelector
    //! @param v8ViewInfo   The V8 view definition
    //! @param v8DgnAttachment The V8 attachment 
    //! @param levelType    Spatial or Drawing
    void ConvertAttachmentLevelMask(ViewDefinitionR viewDef, DgnV8ViewInfoCR v8ViewInfo, DgnAttachmentCR v8DgnAttachment, SyncInfo::Level::Type levelType);
    void ConvertLevelMask(ViewDefinitionR, DgnV8ViewInfoCR, DgnV8ModelRefP);
    void AddAllCategories(DgnCategoryIdSet&);

    DgnV8Api::ViewInfoPtr CreateV8ViewInfo(DgnV8ModelR rootModel, Bentley::DRange3dCR modelRange);
    Render::ViewFlags static ConvertV8Flags(DgnV8Api::ViewFlags v8flags);
    Utf8String RemapViewName(Utf8StringCR name, DgnV8FileR, Utf8StringCR suffix);

    //! convert a ClipVector
    ClipVectorPtr ConvertClip(Bentley::ClipVectorPtr, TransformCP);

    //! convert the clips from a View
    void ConvertViewClips(ViewDefinitionPtr view, DgnV8ViewInfoCR viewInfo, DgnV8ModelR v8Model, TransformCR trans);
    //! convert the grids for a view
    void ConvertViewGrids(ViewDefinitionPtr view, DgnV8ViewInfoCR viewInfo, DgnV8ModelR v8Model, double toMeters);
    //! convert the auxiliary coordinate system for a view
    void ConvertViewACS(ViewDefinitionPtr view, DgnV8ViewInfoCR viewInfo, DgnV8ModelR v8Model, TransformCR, Utf8StringCR);

    //! Convert a View
    BentleyStatus ConvertView(DgnViewId& viewId, DgnV8ViewInfoCR viewInfo, Utf8StringCR defaultName, Utf8StringCR defaultDescription, BentleyApi::TransformCR, ViewFactory&);

    //! Update a previously converted view
    void UpdateViewChildren(ViewDefinitionR newViewDef, DgnViewId existingViewDefId);

    //! Convert all views in a ViewGroup
    void ConvertViewGroup(DgnViewId& firstViewId, DgnV8Api::ViewGroup&, TransformCR, ViewFactory&);
    
    //! Convert a NamedView
    DgnViewId ConvertNamedView(DgnV8Api::NamedView&, TransformCR, ViewFactory&);

    //! Convert all 2D views of in the specified viewgroup where the view root model is equal to the specified V8 model.
    void Convert2dViewsOf(DgnV8Api::ViewInfoPtr& firstViewInfo, DgnV8Api::ViewGroup&, DgnV8ModelR, ViewFactory&);

    DgnV8Api::ViewGroupPtr FindFirstViewGroupShowing(DgnV8ModelR rootModel);

    //! Helpful in constructing ModelSelectors for SpatialViewDefinitions
    void AddAttachmentsToSelection(DgnModelIdSet& selector, DgnV8ModelRefR v8ModelRef, TransformCR trans);

    //! Helpful in constructing ModelSelectors for SpatialViewDefinitions
    void CreateModelSet(DgnModelIdSet& selection, DgnModel const& rootModel, DgnV8ModelR v8RootModel, TransformCR trans);

    enum class V8NamedViewType
    {
        None,
        Other,
        Section,
        Detail,
        Elevation,
        Plan
    };

    V8NamedViewType GetV8NamedViewType(DgnV8Api::ElementHandle const&);
    DGNDBSYNC_EXPORT V8NamedViewType GetV8NamedViewType(DgnAttachmentCR);
    DGNDBSYNC_EXPORT V8NamedViewType GetV8NamedViewTypeOfFirstAttachment(DgnV8ModelR);

    static bool IsSimpleWireframeAttachment(DgnAttachmentCR ref);

    //! Compute the range of section-like proxy graphics from the section-like V8 NamedView that generated them.
    //!<p>A note on terminology: A "section-like NamedView" is a section, elevation, or plan cut. A "section-like attachment" 
    //! is a DgnAttachment that is based on a section-like NamedView. Section-like proxy graphics are the (mostly) planar
    //! visible edges that were generated by cutting a section or elevation, etc.
    //!<p>
    //! Background:
    //! \a v8Attachment is an attachment to a sheet, and the converter is turning that into a a ViewAttachment in the BIM sheet.
    //! The view attachment is being set up by the caller to point to a view of the converted section drawing. The caller
    //! is now in the process of generating that view and wants to know its origin and extents.
    //!<p>
    //! This function detects the case where \a v8Attachment contains or leads to a V8 proxy cache that was generated by a section-like NamedView
    //! and, if so, it computes the origin and range of the proxy graphics from the origin and range of the NamedView, transformed into BIM sheet coordinates.
    //!<p>
    //! 
    //! @param[out] sectionRange            The range of the generated drawing (in BIM sheet coordinates)
    //! @param[in] v8Attachment             The attachment to check for a section-like NamedView
    //! @param[in] sheetModelMapping        The parent sheet model that contains \a v8Attachment
    //! @param[in] isSheetProxyGraphics     Pass true if you know that proxy graphics were converted into the BIM SheetModel's coordinate system.
    //!                                         That will be true if the caller generated the proxy graphics in the context of the sheet.
    //!                                         That should always be the case if v8Attachment is a direct attachment of a 3D model (based on a section-like NamdedView).
    //!                                         That may or may not be the case if v8Attachment is a direct attachment of a V8 drawing model that holds the 3D section-like attachment.
    //!                                         In the latter case, the converted proxy graphics will be in the BIM sheet's CS only if they were converted by the caller in the context of the BIM sheet.
    //!                                         If the proxy graphics were converted in the context of the drawing, then they will be in the CS of the BIM drawing.
    //! @return non-zero error status if this is not a section-like attachment.
    BentleyStatus GetRangeFromSectionView(DRange3dR sectionRange, DgnAttachmentCR v8Attachment,
                                          ResolvedModelMapping const& sheetModelMapping, bool isFromProxyGraphics);

    //! Get the NamedView-based attachment that this drawing is based one. This will return non-null
    //! only if the following is true: v8DrawingModel is a 2D model and it has one DgnAttachment and that attachment is based on
    //! a section, plan, or elevation NamedView.
    //! @param[out] nvAttachment            Optional. If not null,  is returned here.
    //! @param[in] v8DrawingModel          The drawing model
    //! @return the NamedView-based attachment to the drawing or nullptr if the drawing was not generated from an eligible named view.
    DgnV8Api::DgnAttachment const* GetDrawingGeneratorAttachment(DgnModelRefCR drawingModel);

    void ConvertDisplayStyle(DisplayStyleR style, DgnV8Api::DisplayStyle const& v8displayStyle);
    void ConvertSceneLighting(DisplayStyle3dR displayStyle, DgnV8ViewInfoCR, DgnV8ModelR);
    //! @}

    //! @name V8 ECRelationship Conversion
    //! @{
    bool DoesRelationshipExist(Utf8StringCR relName, BeSQLite::EC::ECInstanceKey const& sourceKey, BeSQLite::EC::ECInstanceKey const& targetKey);
    //! @}

    //! @name V8 ECRelationships and Named Group Conversion
    //! @{
    BentleyStatus InitGroupModel();
    BentleyStatus ConvertNamedGroupsRelationshipsInModel(DgnV8ModelR);
    BentleyStatus ConvertECRelationshipsInModel(DgnV8ModelR);
    BentleyStatus ConvertECRelationships(DgnV8Api::ElementHandle const& v8Element);
    DgnModelId FindModelIdForNamedGroup(DgnV8EhCR v8eh);
    static bool DetermineNamedGroupOwnershipFlag(ECN::ECClassCR);
    DGNDBSYNC_EXPORT DgnModelId GetTargetModelForNamedGroup(DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, DgnClassId elementClassId);
    DGNDBSYNC_EXPORT void OnNamedGroupConverted(DgnV8EhCR v8eh, DgnClassId elementClassId);
    //! @}

    //! @name V8 ECInstances
    //! @{
    DgnV8Api::ECQuery const& GetSelectAllV8ECQuery() const;
    static DgnV8Api::FindInstancesScopePtr CreateFindInstancesScope(DgnV8EhCR);
    DgnCode TryGetBusinessKey(ECObjectsV8::IECInstanceCR);
    DGNDBSYNC_EXPORT BentleyStatus GetECContentOfElement(V8ElementECContent& content, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, bool isNewElement);
    //! @}


    //! @name Links and embedded files
    //! @{
    void EmbedSpecifiedFiles();
    void EmbedFilesInSource(BeFileNameCR rootFileName);
    void InitLinkConverter();
    //! @}
    
    //! @name Extracted drawing graphics
    //! @{

    DgnCategoryId GetExtractionCategoryId(V8NamedViewType);
    DGNDBSYNC_EXPORT virtual DgnCategoryId _GetExtractionCategoryId(DgnAttachmentCR);
    DGNDBSYNC_EXPORT virtual DgnSubCategoryId _GetExtractionSubCategoryId(DgnCategoryId, DgnV8Api::ClipVolumePass pass,
                                                                          DgnV8Api::ProxyGraphicsType gtype);
    DGNDBSYNC_EXPORT void ConvertExtractionAttachments(ResolvedModelMapping const&, ProxyGraphicsDrawingFactory&, Bentley::ViewInfoCP);
    DGNDBSYNC_EXPORT virtual void _TurnOnExtractionCategories(CategorySelectorR);
    bool HasProxyGraphicsCache(DgnV8ModelR, ViewportP vp=nullptr);
    bool HasProxyGraphicsCache(DgnAttachmentR, ViewportP vp=nullptr);
    //! Make sure that all attachments that need proxy graphics have them computed.
    //! @param parentModel  The model that has attachments to be checked
    //! @param viewOfParentModel A V8 view of the parent model - determines the visibility and appearance of generated proxy graphics
    //! @praam proxyDetector Helps to detect which attachments should have proxy graphics
    DGNDBSYNC_EXPORT virtual DgnDbStatus _CreateAndInsertExtractionGraphic(ResolvedModelMapping const& drawingModelMapping,
                                                                           SyncInfo::V8ElementSource const& attachmentMapping,
                                                                           SyncInfo::V8ElementMapping const& originalElementMapping,
                                                                           DgnCategoryId categoryId, GeometryBuilder& builder);
    DGNDBSYNC_EXPORT virtual bool _DetectDeletedExtractionGraphics(ResolvedModelMapping const& v8DrawingModel,
                                                                   SyncInfo::T_V8ElementMapOfV8ElementSourceSet const& v8OriginalElementsSeen,
                                                                   SyncInfo::T_V8ElementSourceSet const& unchangedV8attachments);
    DGNDBSYNC_EXPORT virtual bool _DetectedDeletedExtractionGraphicsCategories(SyncInfo::V8ElementSource const& attachmentMapping,
                                                                               SyncInfo::V8ElementMapping const& originalElementMapping,
                                                                               bset<DgnCategoryId>& seenCategories);

    // WIP - Simplified drawing conversion.
    void CreateProxyGraphics (DgnModelRefR modelRef, ViewportR viewport);
    void MergeDrawingGraphics(Bentley::DgnModelRefR baseModelRef, ResolvedModelMapping const& v8mm, ViewportR viewport);
    void CreateSheetExtractionAttachments(ResolvedModelMapping const& v8SheetModelMapping, ProxyGraphicsDrawingFactory& drawingGenerator, Bentley::ViewInfoCP v8SheetView);
    DGNDBSYNC_EXPORT bool _UseRenderedViewAttachmentFor(DgnAttachmentR ref);
    DGNDBSYNC_EXPORT bool _GenerateProxyCacheFor(DgnAttachmentR ref);
    DGNDBSYNC_EXPORT bool HasRenderedViewAttachments(DgnV8ModelR v8ParentModel);


    //! @}

    //! @name Sheets
    //! @{
    BentleyStatus InitSheetListModel();
    Sheet::ElementPtr CreateSheet(Utf8CP label, DgnV8ModelCR);
    Sheet::ElementCPtr CreateSheetAndInsert(Utf8CP label, DgnV8ModelCR v8) {auto s = CreateSheet(label, v8); return s.IsValid()? GetDgnDb().Elements().Insert<Sheet::Element>(*s): nullptr;}
    //! Get or infer the sheet scale. If this is a "full size" sheet, then it annotation scale it the sheet scale. Otherwise, the sheet scale is inferred
    //! from the scales of its attachments.
    double SheetsComputeScale(DgnV8ModelCR v8SheetModel);
    
    //! Map a V8 sheet model to a BIM SheetModel.
    //! @param v8model the V8 sheet model
    //! @param isRootModelSpatial pass true if the root model for the output BIM is a spatial model.
    //! @note ImportSheetModel will terminate with a fatal error if isRootModelSpatial is @a false and if it encounters a reference from a sheet to a 3D model.
    void ImportSheetModel(DgnV8ModelR v8model, bool isRootModelSpatial);

    //! Convert the contents of a sheet model. This includes the sheet border, the elements in the sheet, and views of the sheet. Importantly, this also converts
    //! all reference attachments into BIM views and ViewAttachments.
    //!<pre>
    //! V8
    //! --                                                                              
    //! SheetModel                                                                      
    //!     DgnAttachment   -->  3D Model                                                       
    //!                                                             
    //! BIM
    //! ----
    //! Sheet::Model
    //!     Sheet::ViewAttachment -View-> SpatialViewDefinition 
    //!         ModelSelector --> PhysicalModel
    //!</pre>
    void SheetsConvertModelAndViews(ResolvedModelMapping const& v8mm, ViewFactory&);
    
    void AddAllSpatialCategories(DgnCategoryIdSet&);

    //! Create and insert a Drawing for the specified V8 attachment. Also inserts a mapping into syncinfo
    //! @param[in] v8Attachment         The attachment to convert
    //! @param[in] parentModel          The model that contains the attachment
    //! @return the newly inserted model and modelled element
    ResolvedModelMappingWithElement SheetsCreateAndInsertDrawing(DgnAttachmentCR v8DgnAttachment, ResolvedModelMapping const& parentModel);

    Utf8String SheetsComputeViewAttachmentName(DgnAttachmentCR v8DgnAttachment) const;
    
    //! Create a ViewAttachment element from the specified V8 attachment. 
    //! @note this function should attempt to insert or update the element. The returned element should be non-persistent.
    //! @param[out] results         The ViewAttachment element
    //! @param v8SheetModelMapping  The V8 sheet model
    //! @param v8Attachment         The attachment to convert.
    //! @param attachedViewId       The BIM view to attach
    //! @param displayPriority      The display priority of the ViewAttachment
    //! @param isViewInSheetCoordinates  Is the geometry of the attached view already in BIM sheet coordinates? This happens when the viewed model 
    //! was generated from a direct attachment to the V8 sheet based on proxy graphics.
    void SheetsCreateViewAttachment(ElementConversionResults& results, 
                                    ResolvedModelMapping const& v8SheetModelMapping, DgnAttachmentR v8Attachment, 
                                    DgnViewId attachedViewId, int displayPriority, bool isFromDirectlyAttachedProxyGraphics);
    
    //! Calls FindModelForDgnV8Model to find the BIM model that corresponds to the attached V8 model
    ResolvedModelMapping SheetsFindModelForAttachment(DgnAttachmentCR);
        
    //! Finds or generates a view of \a geometricModel that can be used as the target for a ViewAttachment that is based on \a v8DgnAttachment
    //! @param[in] isFromProxyGraphics  Was \a geometricModel generated from V8 proxy graphics?
    //! @param[in] dgnV8Attachment     The attachment to convert.
    //! @param[in] v8DgnAttachment    The viewed model. Could be a drawing model or a spatial model.
    //! @param[in] sheetModelMapping   The sheet model and its mapping to the original V8 sheet model
    //! @param[in] v8SheetModelView   Optional. The V8 view of the sheet. The generated view's CategorySelector will be built from this view's level mask tree.
    //! @param[in] nvvf                 The view factory to be used in order to convert a named view. This is needed if an attachment is linked to a V8 named view.
    DgnViewId SheetsGetViewForAttachment(bool isFromProxyGraphics, GeometricModelR geometricModel, 
                                            DgnAttachmentCR v8DgnAttachment, ResolvedModelMapping const& sheetModelMapping, 
                                            DgnV8ViewInfoCP v8SheetModelView, ViewFactory& nvvf);

    //! Convert the specified DgnAttachment in the specified V8 sheet model to a ViewAttachment.
    //! @param v8SheetModelMapping  The V8 sheet model
    //! @param v8Attachment         The attachment to convert.
    //! @param proxyModel           Optional. If this attachment is backed by a model that was generated from V8 proxy graphics, pass that model here.
    //! @param nvvf                 The view factory to be used in order to convert a named view. This is needed if an attachment is linked to a V8 named view.
    //! @param v8SheetViewInfo  Optional. The V8 view of the sheet. The generated view's CategorySelector will be built from this view's level mask tree.
    //! @param updateSequence       The v8 update sequence (relative to sheet)
    void SheetsConvertViewAttachment(ResolvedModelMapping const& v8SheetModelMapping, DgnAttachmentR v8Attachment, 
                                     GeometricModelP proxyModel, ViewFactory& nvvf, DgnV8ViewInfoCP v8SheetViewInfo, int updateSequence);

    //! Convert the DgnAttachments in the specified V8 sheet model to ViewAttachments.
    //! @param v8SheetModelMapping  The V8 sheet model
    //! @param v8SheetView          Optional. A V8 view of this sheet model, if available. This is used when generating or harvesting proxy graphics.
    //! @param nvvf                 The view factory to be used in order to convert a named view. This is needed if an attachment is linked to a V8 named view.
    void SheetsConvertViewAttachments(ResolvedModelMapping const& v8SheetModelMapping, Bentley::ViewInfoCP v8SheetView, ViewFactory& nvvf);

    //! @}

    //! @name Drawings
    //! @{
    BentleyStatus InitDrawingListModel();
    DrawingPtr CreateDrawing(Utf8CP label);
    DrawingCPtr CreateDrawingAndInsert(Utf8CP label) {auto d = CreateDrawing(label); return d.IsValid()? GetDgnDb().Elements().Insert<Drawing>(*d): nullptr;}
    SectionDrawingPtr CreateSectionDrawing(Utf8CP label);
    SectionDrawingCPtr CreateSectionDrawingAndInsert(Utf8CP label) {auto d = CreateSectionDrawing(label); return d.IsValid()? GetDgnDb().Elements().Insert<SectionDrawing>(*d): nullptr;}

    void ImportDrawingModel(ResolvedModelMapping& rootModelMapping, DgnV8ModelR v8model);

    //! Map the specified 2d model to a BIM model 
    bpair<ResolvedModelMapping,bool> Import2dModel(DgnV8ModelR v8model);

    void DrawingRegisterModelToBeMerged(bvector<ResolvedModelMapping>& tbm, DgnAttachmentR v8DgnAttachment, DgnModelR targetBimModel, TransformCR transformToParent);
    void DrawingRegisterAttachmentsToBeMerged(bvector<ResolvedModelMapping>& tbm, DgnV8ModelRefR v8modelRef, DgnModelR targetBimModel, TransformCR transformToParent);

    //! Import the 2d models that are attached to a drawing and schedule them to be merged into the drawing.
    //! Note that \a v8modelRef may be an attachment (of a drawing) to a sheet.  
    //! @param v8modelRef The parent drawing model
    //! @param drawingModelMapping The parent drawing model's mapping into the BIM
    void DrawingRegisterAttachmentsToBeMerged(DgnV8ModelRefR v8modelRef, ResolvedModelMapping const& drawingModelMapping);

    //! Callback that is invoked when a 2d model is converted.
    virtual void _OnDrawingModelFound(DgnV8ModelR) = 0;

    //! Callback that is invoked when it is likely that the specified V8 file must be kept alive by adding an explicit ref to it.
    virtual void _KeepFileAlive(DgnV8FileR) = 0;

    //! Convert the contents of a drawing model, populating a BIM drawing model, and convert views of this model. The conversion also pulls in proxy graphics from attachments.
    void DrawingsConvertModelAndViews(ResolvedModelMapping const& v8mm);

    //! Convert the elements in a sheet model. This includes only the elements actually in the V8 drawing model.
    DGNDBSYNC_EXPORT void DoConvertDrawingElementsInSheetModel(ResolvedModelMapping const&);

    //! Convert levels in v8 element (used to preconvert drawing element levels).
    void ConvertLevels(DgnV8EhCR v8eh);

    //! Default logic for converting a drawing or sheet element
    DGNDBSYNC_EXPORT void DoConvertDrawingElement(ElementConversionResults&, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, bool isNewElement);
    //! Convert a drawing or sheet element
    void _ConvertDrawingElement(DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm);

    //! @}

    //! @name 2D attachments
    //! @{
    //! Return true if the specified drawing has any 3D attachments.
    bool DrawingHas3DAttachment(DgnV8ModelR drawingModel);
    bool DrawingHas3DAttachment(DgnAttachmentCR att) {return att.GetDgnModelP()? DrawingHas3DAttachment(*att.GetDgnModelP()): false;}

    //! Detect if the model referenced by \a v8Attachment needs to be changed to match the annotation scale specified by the (root) attachment and if so call CopyAndChangeAnnotationScale to change it. 
    //! Calls MakeAttachmentsMatchRootAnnotationScale to do the same, recursively, to all nested attachments. This can end up creating a new set of models and a new attachment hierarchy.
    void MakeAttachedModelMatchRootAnnotationScale(DgnAttachmentR v8Attachment);

    //! Calls MakeAttachedModelMatchRootAnnotationScale to adjust the annotation scale of all 2D attachments to \a parentModel.
    void MakeAttachmentsMatchRootAnnotationScale(DgnV8ModelRefR parentModel);

    //! Make a copy of the specified model and change the annotation scale of all annotations to match \a newAnnotationScale.
    //! @note The name of the new model is generated and should not conflict with any existing V8 models.
    //! @note This function also changes DgnModelType::Sheet to DgnModelType::Drawing
    //! @note The returned model is marked as hidden
    Bentley::RefCountedPtr<DgnV8Api::DgnModel> CopyAndChangeAnnotationScale(DgnV8ModelP, double newAnnotationScale);

    //! Make a copy of the specified sheet model and change its model type from DgnModelType::Sheet to DgnModelType::Drawing.
    //! @note The name of the new model is generated and should not conflict with any existing V8 models.
    //! @note The returned model is marked as hidden
    Bentley::RefCountedPtr<DgnV8Api::DgnModel> CopyAndChangeSheetToDrawing(DgnV8ModelP);

    //! Make a copy of the specified model. 
    //! @note The name of the new model is generated from the original model's file and model name, plus the specified suffix.
    //! @note The returned model is marked as hidden
    Bentley::RefCountedPtr<DgnV8Api::DgnModel> CopyModel(DgnV8ModelP v8Model, WCharCP newNameSuffix);

    //! convert any attached sheets into drawings
    void TransformSheetAttachmentsToDrawings(DgnV8ModelR parentModel);

    //! Optionally transform attachments to 2d models into attachments to copies of those 2d models.
    typedef std::function<bool(DgnV8Api::DgnAttachment const&)> T_AttachmentCopyFilter;
    void TransformDrawingAttachmentsToCopies(DgnV8ModelR parentModel, T_AttachmentCopyFilter filter);

    //! @}

    //! @name Models
    //! @{
    
    //! @private Classify the 2d Normal models as either drawing or spatial, and if they are drawing, mark them as such in the ModelInfo. It does not hurt to call this multiple times.
    void ClassifyNormal2dModels(DgnV8FileR);

    void ClassifyNormal2dModel(DgnV8ModelR v8Model, ModelTypeAppData& mtAppData);
    void Classify2dModelIfNormal(DgnV8ModelR v8Model, ModelTypeAppData* mtAppData);

    //! @private Copy the effective model type that was assigned by ClassifyNormal2dModels to \a oldModel to \a newModel.
    void CopyEffectiveModelType(DgnV8ModelR newModel, DgnV8ModelR oldModel);

    //! @private Set the effective model type on a newly created model. This needed only if the model is created by the converter. ClassifyNormal2dModels handles all persistent models in V8 files.
    void SetEffectiveModelType(DgnV8ModelR, DgnV8Api::DgnModelType);

    //! @private return true if the item in the v8File's modelInfo represents a model that should be treated as a DrawingModel.
    bool IsV8DrawingModel(DgnV8FileR v8File, DgnV8Api::ModelIndexItem const& item);

    bool IsV8DrawingModel (DgnV8ModelR v8Model);

    //! Query the class of the DgnModel that would be created if the specified V8 model were converted.
    //! @see _ConsiderNormal2dModelsSpatial
    DGNDBSYNC_EXPORT DgnClassId ComputeModelType(DgnV8ModelR v8Model) {return _ComputeModelType(v8Model);}

    //! Query if the specified V8 model should be converted to bis PhysicalModel or one of its subclasses.
    //! This is the correct way to tell if a model is "spatial" or not. All other V8 models become drawings or sheets.
    bool ShouldConvertToPhysicalModel(DgnV8ModelR model) {return DgnClassId(GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalModel)) == _ComputeModelType(model);}

    Utf8String RemapModelName(Utf8StringCR name, DgnV8FileR, Utf8StringCR suffix);
    BentleyApi::Transform ComputeAttachmentTransform(BentleyApi::TransformCR parentTransform, DgnAttachmentCR v8);
    static DgnV8Api::DgnAttachmentArray* GetAttachments(DgnV8ModelRefR);

    void ParseModelConfig(Config&);
    BentleyStatus SearchForMatchingRule(ImportRule& entryOut, DgnV8ModelCR oldModel);

    //! Compute the scale factor that converts the storage units of the V8 file into meters.
    DGNDBSYNC_EXPORT double ComputeUnitsScaleFactor(DgnV8ModelCR v8Model);
    //! Compute the scale factor (as a transform) that converts the storage units of the V8 file into meters.
    DGNDBSYNC_EXPORT Transform ComputeUnitsScaleTransform(DgnV8ModelCR v8Model);

    ResolvedModelMapping FindResolvedModelMappingBySyncId(SyncInfo::V8ModelSyncInfoId sid) {return _FindResolvedModelMappingBySyncId(sid);}
    virtual ResolvedModelMapping _FindResolvedModelMappingBySyncId(SyncInfo::V8ModelSyncInfoId sid) = 0;

    void CaptureModelDiscard(DgnV8ModelR);

    ResolvedModelMapping GetModelFromSyncInfo(DgnV8ModelRefCR, TransformCR);
    virtual ResolvedModelMapping _GetModelForDgnV8Model(DgnV8ModelRefCR, TransformCR) = 0;
    ResolvedModelMapping GetModelForDgnV8Model(DgnV8ModelRefCR v8ModelRef, TransformCR toBim) {return _GetModelForDgnV8Model(v8ModelRef, toBim);}
    virtual ResolvedModelMapping _FindModelForDgnV8Model(DgnV8ModelR v8Model, TransformCR) = 0;
    ResolvedModelMapping FindModelForDgnV8Model(DgnV8ModelR v8Model, TransformCR toBim) {return _FindModelForDgnV8Model(v8Model, toBim);}
    virtual ResolvedModelMapping _FindFirstModelMappedTo(DgnV8ModelR v8Model) = 0;
    //! Looks up the first \em resolved model mapping for the specified V8 model. 
    //! @note This function does \em not look in syncinfo, but only in the list of resolved mappings.
    ResolvedModelMapping FindFirstModelMappedTo(DgnV8ModelR v8Model) {return _FindFirstModelMappedTo(v8Model);}

    //! When converting this DgnV8 model to DgnDb, decide what type of model it should become.
    //! May cause attachments to be loaded.
    //! @see _ConsiderNormal2dModelsSpatial
    DGNDBSYNC_EXPORT DgnClassId _ComputeModelType(DgnV8ModelR v8Model);

    //! Return true if normal 2d Models should be considered to be spatial models.
    virtual bool _ConsiderNormal2dModelsSpatial() = 0;

    //! When converting this DgnV8 model to DgnDb, decide what visibility it should have.
    DGNDBSYNC_EXPORT virtual bool _IsModelPrivate(DgnV8ModelCR v8Model);

    //! Compute the name to assign to \a v8Model when converting it into a DgnDb model.
    //! The default implementation is to use the original model name and then make sure it is unique. The exception to this
    //! rule is to change the name "default" to the name of the foreign file. Illegal characters are converted to '_'.
    //! @param[in] v8Model The DgnV8 model that is about to be converted.
    //! @return The name of the model to create in the DgnDb
    DGNDBSYNC_EXPORT virtual Utf8String _ComputeModelName(DgnV8ModelCR v8Model);

    //! Return the model in the DgnDb to be used as the destination for the specified input DgnV8 model.
    //! The DgnV8 model will be mapped to the returned model, and its contents will be written to that model.
    //! This function can return an existing DgnDb model, or it can create a new DgnDb model. That allows
    //! converter to implement many mapping strategies, including merging many DgnV8 models into one DgnDb model,
    //! The default implementation is to create a new, unique DgnDb model for the input DgnV8 model.
    //! The default implementation  calls _ComputeModelName, _ComputeModelType, and _IsModelPrivate to decide how
    //! to convert the input model to a DgnDb model.
    //! splitting up DgnV8 models, and creating 1 DgnDb model for each DgnV8 model.
    //! @param[in] v8Model      The DgnV8 model that is about to be converted
    //! @param[in] attachment   The DgnAttachment that was used to reach v8Model. Nullptr if v8Model is a root.
    //! @return modelId of new model. Invalid if the model should be discarded.
    //! @see _ComputeModelName, _ComputeModelType, _IsModelPrivate
    DGNDBSYNC_EXPORT virtual DgnModelId _MapModelIntoProject(DgnV8ModelR v8Model, Utf8CP, DgnV8Api::DgnAttachment const* attachment);
    
    DGNDBSYNC_EXPORT DgnModelId CreateModelFromV8Model(DgnV8ModelCR v8Model, Utf8CP newName, DgnClassId classId, DgnV8Api::DgnAttachment const*);

    DgnModelId FindFirstModelInSyncInfo(DgnV8ModelCR) const;

    void AddV8ModelToRange(Bentley::DRange3dR, DgnV8ModelR v8Model);

    //! @}

    //! @name init and clean up functions
    //! {
    DGNDBSYNC_EXPORT virtual void _OnConversionStart();
    DGNDBSYNC_EXPORT virtual void _OnConversionComplete();
    DGNDBSYNC_EXPORT void OnCreateComplete();
    DGNDBSYNC_EXPORT void OnUpdateComplete();

    //! @}

    friend struct DgnV8SymbologyConverter;
    friend struct LineStyleConverter;
    friend V8GraphicsCollector;

    //! @name Converting fonts
    //! The idea behind converting fonts from DgnV8 is to replicate the DgnV8 workspace by creating a collection of "known" fonts from lose files. As fonts are encountered in a DgnV8 file, we look for them by-type and -name in our "workspace", and make entries for them in the database. Then, as a second pass, we embed any used fonts, as dictated by the stubs in the database.
    //! This has a critical side effect: Until the second pass when fonts are embedded, if you query the database directly for a font, it will be unresolved. This is why _RemapFont returns a DgnFont object, and not an ID (though it can be used to get an ID). The DgnFont object returned by _RemapFont is owned and resolved by this converter, and is valid for direct use. Currently, all known users will use this returned font object for all intents and purposes, thus it is acceptable to do embedding as a second pass. But be forewarned, as-is, you cannot query a database for a font by-ID and use it until the embedding pass has been done.
    //! @{
    virtual DgnV8FileR _GetFontRootV8File() = 0;
    //! Used by the default implementation of _EnsureWorkspaceFontsAreLoaded. Gets a semi-colon delimited list of paths to search for V8 "workspace" fonts. When remapping fonts, all fonts in these paths will be searched by-type and -name, and are eligible for including/remapping into the database.
    virtual Utf8String _GetWorkspaceFontSearchPaths() const {return GetConfig().GetXPathString("/ImportConfig/Fonts/@searchPaths", "");}
    //! Fills in m_workspaceFonts with "workspace" fonts, which are searched by-type and -name when remapping from DgnV8. "Workspace" in this sense attempts to replicate the global font registry that DgnV8 maintained from the system and its workspace configuration. The default implementation will iterate all font files in _GetWorkspaceFontSearchPaths, and make all found fonts available for remapping.
    DGNDBSYNC_EXPORT virtual void _EnsureWorkspaceFontsAreLoaded();
    //! Fills in m_workspaceFonts with fonts embedded in the provided DgnV8 file. While they are not technically "workspace" fonts, for the intents and purposes of this converter, this is a good-enough simplification. This is typically called by _EnsureWorkspaceFontsAreLoaded, and as such, this method itself does not protect against multiple calls. Note that you can use the m_tempFontFiles member for temporary font files you want deleted when the Converter class destructs.
    DGNDBSYNC_EXPORT virtual void _LoadEmbeddedV8Fonts(DgnV8Api::DgnFile&);
    //! Return a stable pointer to a font object that can be put into the database. This implies that the converter should own the font, and hand out a pointer to it. This also implies that it can return nullptr, meaning that we could not find the original font, and to create a missing font entry. The default implementation calls _EnsureWorkspaceFontsAreLoaded to ensure m_workspaceFonts is populated, and then searches it for the font in question, returning a pointer.
    DGNDBSYNC_EXPORT virtual DgnFont const* _ImportV8Font(DgnV8Api::DgnFont const&);
    //! Return a reference to a font object that exists by-type and -name in the database, or a last resort font in case of corrupt data. As mentioned in the description of this 'Converting fonts' section, the returned object may not match the object returned by querying the database directly. The returned font object is intended to be resolved and usable, and should thus be used whenever possible. While this returned object can be used to get an ID from the database, until you call _EmbedFonts, any direct database queries may return unresolved fonts. The default implementation, along with various rules, leverages _ImportV8Font to do much of the work.
    DGNDBSYNC_EXPORT virtual DgnFont const& _RemapV8Font(DgnV8Api::DgnFile&, uint32_t v8FontId);
    //! Attempt to embed font data for all font entries in the database so that fonts can be resolved without the converter and its "workspace" being present. The default implementation takes the union of the database's font entries and any "always" embed fonts from the import configuration, and attempts to embed data by-type and -name from m_workspaceFonts.
    DGNDBSYNC_EXPORT virtual void _EmbedFonts();
    //! Try to resolve a font via the workspace. Some converter applications choose to forward FontAdmin::ResolveFont calls to the active converter so that it can provide font data from the workspace. As described in this section's documentation, font embedding happens as a second pass. Allowing the converter to resolve a font via a workspace is a way to allow converters to access fonts before they are embedded in the database.
    DGNDBSYNC_EXPORT virtual DgnFontCP _TryResolveFont(DgnFontCP);
    DgnFontId ConvertFontFromStyle(DgnV8Api::DgnTextStyle const&, DgnV8Api::TextStyleProperty);
    //! @}

    //! @name  Converting LineStyles
    //! @{
    DGNDBSYNC_EXPORT void ConvertAllLineStyles(DgnV8Api::DgnFile&v8File);
    DGNDBSYNC_EXPORT void SetLineStyleConverterRootModel(DgnV8ModelP);
    //! @}

    //! @name  Converting Materials
    //! @{
    DGNDBSYNC_EXPORT void ConvertModelMaterials(DgnV8ModelR v8Model);
    DGNDBSYNC_EXPORT BentleyStatus ConvertMaterial(Json::Value& renderMaterial, DgnV8Api::Material const& v8Material, DgnV8Api::DgnFile& v8File);
    DGNDBSYNC_EXPORT void ConvertMaterialTextureMaps(Json::Value& renderMaterial, DgnV8Api::DgnFile& v8File, DgnV8Api::DgnModelRef& modelRef);
    DGNDBSYNC_EXPORT BentleyStatus ConvertMaterialTextureMap(Json::Value& dbMapsMap, Json::Value const& v8Map, DgnV8Api::DgnFile& v8File, DgnV8Api::DgnModelRef& modelRef);
    DGNDBSYNC_EXPORT BentleyStatus ConvertMaterialTextureMapImage(Json::Value& textureMap, DgnV8Api::DgnFile& v8File, bool pseudoBackgroundTransparency);
    DGNDBSYNC_EXPORT DgnTextureId FindOrInsertTextureImage(WCharCP filename, DgnV8Api::DgnFile& v8File, bool pseudoBackgroundTransparency = false);
    DGNDBSYNC_EXPORT void SetMaterialUsed(RenderMaterialId id);
    DGNDBSYNC_EXPORT bool GetMaterialUsed(RenderMaterialId id) const;
    virtual void _RemoveUnusedMaterials() {RemoveUnusedMaterials();}
    DGNDBSYNC_EXPORT void RemoveUnusedMaterials();
    DGNDBSYNC_EXPORT RenderMaterialId GetRemappedMaterial(DgnV8Api::Material const* material);
    void AddMaterialMapping(DgnV8Api::Material const* material, Utf8StringCR name, Utf8StringCR palette, RenderMaterialId materialId);
    //! @}

    //! @name  Converting Elements
    //! @{
    DGNDBSYNC_EXPORT virtual void _OnElementConverted(DgnElementId elementId, DgnV8EhCP v8eh, ChangeOperation changeOperation);
    DGNDBSYNC_EXPORT virtual void _OnElementBeforeDelete(DgnElementId elementId);

    DgnDbStatus InsertResults(ElementConversionResults&);
    DgnDbStatus UpdateResultsForOneElement(ElementConversionResults&, DgnElementId existingElementId);
    DgnDbStatus UpdateResultsForChildren(ElementConversionResults&);
    DgnDbStatus UpdateResults(ElementConversionResults&, DgnElementId existingElementId);
    //! Writes to the DgnDb and to SyncInfo, as specified by the change type in \a searchResults. The following cases are handled:
    //! -- \a conversionResults.m_element is invalid => records a discard in SyncInfo and sets \a conversionResults.m_wasDiscarded.
    //! -- IChangeDetector::ChangeType::Insert - the (non-persistent!) element in \a conversionResults is inserted into the BIM, and a mapping is 
    //! created in SyncInfo. Sets conversionResults.m_mapping.
    //! -- IChangeDetector::ChangeType::Update - looks up the existing element in the BIM using \a existingElementId, updates that element
    //! from \a conversionResults.m_element, and then updates the provenance info for this element in in SyncInfo. Sets conversionResults.m_mapping.
    //! -- IChangeDetector::ChangeType::None - The element is not written to the BIM, and SyncInfo is not updated.
    //! In all cases except discard, this function also passes \a conversionResults.m_existingElementId to the current IChangeDetector's 
    //! _OnElementSeen method, which allows an updater to infer deletions later on.
    void ProcessConversionResults(ElementConversionResults& conversionResults, IChangeDetector::SearchResults const& searchResults, 
                                  DgnV8EhCR v8eh, ResolvedModelMapping const&);

    //! Convenience method when you know that the conversion results should be inserted. Calls ProcessConversionResults.
    void InsertConversionResults(ElementConversionResults& res, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm)
        {
        IChangeDetector::SearchResults newEle;
        newEle.m_changeType = IChangeDetector::ChangeType::Insert;
        ProcessConversionResults(res, newEle, v8eh, v8mm);
        }
    
    //! Records a mapping in SyncInfo, based on the contents of conversion results. Called by ProcessConversionResults. Rarely called directly. 
    //! @param[in,out]  conversionResults    On input, the data to be written to the BIM; on output, the result of updating the BIM and syncinfo.
    //! @param[in] v8mm The model that contains the element
    //! @param[in] updatePlan The element's current syncinfo mapping, if known.
    //! @param[in] isParentElement Set to true for stand-alone and parent elements, and false for child elements.
    //! \a updatePlan is the plan for how to update the BIM and syncinfo. It might indicate that the element is new and should be inserted.
    //! Or, it might contain syncinfo mapping if the element is already in the BIM and should be updated.
    //! On input, \a conversionResults.m_element should either be invalid, indicating that the element should be discarded, or it 
    //! should be set to a copy of the element that is to be inserted or updated. On ouput, either \a conversionResults.m_isDiscard is set to true
    //! if the element was discarded, or \a conversionResults.m_v8Mapping is set to the element's syncinfo mapping. 
    void RecordConversionResultsInSyncInfo(ElementConversionResults& conversionResults, DgnV8EhCR, ResolvedModelMapping const& v8mm, 
                                           IChangeDetector::SearchResults const& updatePlan, bool isParentElement = true);
    
    //! This function can be used to record a mapping from a V8 element to an existing BIM element. Rarely used.
    SyncInfo::V8ElementMapping RecordMappingInSyncInfo(DgnElementId bimElementId, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm);

    //! This function can be used to update the provenance data stored in an existing SyncInfo mapping. Rarely used.
    SyncInfo::V8ElementMapping UpdateMappingInSyncInfo(DgnElementId bimElementId, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm);

    //! Convenience method to create a new, non-persistent element.
    static DgnElementPtr CreateNewElement(DgnDbApi::DgnModel&, DgnClassId, DgnCategoryId, DgnCode, Utf8CP label = nullptr);

    //! Convenience method to make an editable copy of an element. This function is specialized for use by the Converter. It assumes that 
    //! \a newEl is meant to be a new version of \a originalEl. It is prepared to deal with the case where \a newEl does not have an ElementId.
    //! It has special copy logic to produce a copy of \a newEl that includes the ElementId from \a originalEl.
    static DgnElementPtr MakeCopyForUpdate(DgnElementCR newEl, DgnElementCR originalEl);

    //! Look up the specified V8 element in syncinfo.
    //! @param v8ModelSyncInfoId Identifies the V8 model to look up
    //! @param v8ElementId Identifies the V8 element to look up
    //! @param filter   Optional. Chooses among existing elements in SyncInfo
    //! @return Mapping information for this element, if found, or an invalid mapping if not.
    DGNDBSYNC_EXPORT SyncInfo::V8ElementMapping GetFirstElementBySyncInfoId(SyncInfo::V8ModelSyncInfoId v8ModelSyncInfoId, DgnV8Api::ElementId v8ElementId, 
                                                                            IChangeDetector::T_SyncInfoElementFilter* filter = nullptr);

    //! Convenience method to look up the specified V8 element in syncinfo. This function looks up the V8 model in syncinfo for you. Calls GetFirstElementBySyncInfoId.
    //! @param v8Model the V8 model to look up
    //! @param v8ElementId Identifies the V8 element to look up
    //! @param filter   Optional. Chooses among existing elements in SyncInfo
    //! @return Mapping information for this element, if found, or an invalid mapping if not.
    DGNDBSYNC_EXPORT SyncInfo::V8ElementMapping FindFirstElementMappedTo(DgnV8ModelCR v8Model, DgnV8Api::ElementId, 
                                                                         IChangeDetector::T_SyncInfoElementFilter* filter = nullptr);

    //! Convenience method to look up a V8 element in syncinfo. This function works with a V8 displaypath and it looks up the V8 model in syncinfo for you. Calls GetFirstElementBySyncInfoId.
    //! @param db   The DgnDb being converted
    //! @param v8Model the V8 model to look up
    //! @param v8ElementId Identifies the V8 element to look up
    //! @param filter   Optional. Chooses among existing elements in SyncInfo
    //! @return Mapping information for this element, if found, or an invalid mapping if the element is not found in syncinfo or if the display path is empty or invalid.
    SyncInfo::V8ElementMapping FindFirstElementMappedTo(DgnV8Api::DisplayPath const& proxyPath, bool tail, IChangeDetector::T_SyncInfoElementFilter* filter = nullptr) 
        { return _FindFirstElementMappedTo(proxyPath, tail, filter); }

    DGNDBSYNC_EXPORT void InitUncategorizedCategory();
    DGNDBSYNC_EXPORT void InitUncategorizedDrawingCategory();

    //! Get the category for spatial elements that are unable to be categorized during the DgnV8 to DgnDb conversion process.
    //! @note This should only be used as a last resort when other attempts to categorize fail
    DgnCategoryId GetUncategorizedCategory() const {return m_uncategorizedCategoryId;}

    //! Get the category for drawing elements that are unable to be categorized during the DgnV8 to DgnDb conversion process.
    //! @note This should only be used as a last resort when other attempts to categorize fail
    DgnCategoryId GetUncategorizedDrawingCategory() const {return m_uncategorizedDrawingCategoryId;}

    //! @}

    //! @name Config
    //! @{

    Config const& GetConfig() const {return m_config;}

    //! Called just after the converter configuration file is read and before conversion begins.
    virtual void _OnConfigurationRead(BeXmlDomR configDom) {}
    //! @}

    DGNDBSYNC_EXPORT virtual DgnStyleId _RemapLineStyle(double&unitsScale, DgnV8Api::DgnFile&, int32_t v8LineStyleId, bool required);


    //! Call this once before working with Converter
    //! @note Call this just \em after initializing DgnDb's DgnPlatformLib
    //! @note If you install RealDWG on a shared location, pass in an absolute path; otherwise the default \V8SDK\RealDwg\ location will be used.
    DGNDBSYNC_EXPORT static void Initialize(BentleyApi::BeFileNameCR bridgeLibraryDir, BentleyApi::BeFileNameCR bridgeAssetsDir, 
                                            BentleyApi::BeFileNameCR v8DllsRelativeDir, BentleyApi::BeFileNameCP realdwgAbsoluteDir, 
                                            bool isPowerPlatformBased, int argc, WCharCP argv[]);
    static BentleyStatus InitializeDwgHost (BentleyApi::BeFileNameCR v8dir, BentleyApi::BeFileNameCR realdwgDir);
    static BentleyStatus InitializeDwgSettings (Converter* v8converter);
    static void InitV8ForeignFileTypes (Converter* v8converter);
    static void RegisterForeignFileTypes (BentleyApi::BeFileNameCR v8dir, BentleyApi::BeFileNameCR dwgDir);
    //! Sniff a file and see if it smells like DWG, DXF or DXB format.
    DGNDBSYNC_EXPORT static bool IsDwgOrDxfFile (BeFileNameCR filename);

    //! @name Converting Elements. Also see @ref Drawings
    //! @{

    //! Convert a control element
    DGNDBSYNC_EXPORT virtual void _ConvertControlElement(ElementConversionResults&, DgnV8EhCR v8eh, ResolvedModelMapping const&);
    //! Default logic for converting a control element
    DGNDBSYNC_EXPORT void DoConvertControlElement(ElementConversionResults&, DgnV8EhCR v8eh, ResolvedModelMapping const& m, bool isNewElement);

    //! Converts a V8 element to a BIS element, where the class may be computed from the V8 primary ECInstance if any. Also calls on handler
    //! extensions to help with the conversion.
    BentleyStatus ConvertElement(ElementConversionResults&, DgnV8EhCR, ResolvedModelMapping const&, DgnCategoryId catid, bool ignoreElementWithoutECInstance, bool isNewElement=true);

    DGNDBSYNC_EXPORT virtual DgnClassId _ComputeElementClass(DgnV8EhCR v8eh, V8ElementECContent const& ecContent, ResolvedModelMapping const& v8mm);

    DGNDBSYNC_EXPORT DgnClassId ComputeElementClassIgnoringEcContent(DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm);

    //! Creates a geometric element. The geometry is converted from the V8 element.
    DGNDBSYNC_EXPORT virtual BentleyStatus _CreateElementAndGeom(ElementConversionResults&, ResolvedModelMapping const&, DgnClassId elementClassId, 
                                                                 bool hasV8PrimaryECInstance, DgnCategoryId, DgnCode elementCode, DgnV8EhCR);

    static BentleyApi::CurveVectorPtr ConvertV8Curve(Bentley::CurveVectorCR v8Curves, TransformCP v8ToDgnDbTrans);

    RangePartIdMap& GetRangePartIdMap() {return m_rangePartIdMap;}
    void PopulateRangePartIdMap();

    //! @}

    DgnFontCP TryResolveFont(DgnFontCP requestedFont) {return _TryResolveFont(requestedFont);}

    //! @name Converting Sections, Plans, Elevations, etc. that use extracted drawing graphics
    //! @{
    DgnV8Api::ProxyDisplayHitInfo const* GetProxyDisplayHitInfo(DgnV8Api::ViewContext&);
    DgnCategoryId GetExtractionCategoryId(DgnAttachmentCR a) {return _GetExtractionCategoryId(a);}
    DgnSubCategoryId GetExtractionSubCategoryId(DgnCategoryId c, DgnV8Api::ClipVolumePass pass, DgnV8Api::ProxyGraphicsType gtype) {return _GetExtractionSubCategoryId(c, pass, gtype);}
    DgnCategoryId GetOrCreateDrawingCategoryId(DefinitionModelR model, Utf8StringCR categoryName, DgnSubCategory::Appearance const& appear = DgnSubCategory::Appearance());
    //! @}

    //! @name Converting text
    //! @{
    DGNDBSYNC_EXPORT virtual DgnElementId _GetOrCreateTextStyleNoneId(DgnV8Api::DgnFile&);
    void ConvertColorFromStyle(AnnotationColorType&, ColorDef&, DgnV8Api::DgnTextStyle const&, DgnV8Api::TextStyleProperty v8ColorProperty, DgnV8Api::TextStyleProperty v8FlagProperty);
    DGNDBSYNC_EXPORT virtual DgnElementId _RemapV8TextStyle(DgnV8Api::DgnFile&, DgnV8Api::ElementId);
    DGNDBSYNC_EXPORT virtual AnnotationTextStylePtr _ConvertV8TextStyle(DgnV8Api::DgnTextStyle const&);
    DGNDBSYNC_EXPORT virtual AnnotationTextBlockPtr _ConvertV8TextBlock(DgnV8Api::TextBlock const&);
    void ConvertTextStyleForDocument(AnnotationTextBlock&, DgnV8Api::DgnTextStyle const&);
    void ConvertTextStyleForParagraph(AnnotationParagraph&, DgnV8Api::DgnTextStyle const&);
    void ConvertTextStyleForRun(AnnotationRunBase&, DgnV8Api::DgnTextStyle const&);
    //! @}

    //! @name  Tags
    //! @{
    DGNDBSYNC_EXPORT virtual void _ConvertDgnV8Tags(bvector<DgnV8FileP> const& v8Files, bvector<DgnV8ModelP> const& uniqueModels);
    static WCharCP GetV8TagSetDefinitionSchemaName() {return L"V8TagSetDefinitions";}
    //! @}

    //! @name Codes
    //! @{

    //! Compute the code that will be assigned to the specified element
    DGNDBSYNC_EXPORT virtual DgnCode _ComputeElementCode(DgnV8EhCR, V8ElementECContent const&);

    DGNDBSYNC_EXPORT void InitBusinessKeyCodeSpec();

    //! Get the ID of the CodeSpec which determines element codes based on v8 BusinessKeySpecification custom attributes.
    CodeSpecId GetBusinessKeyCodeSpec() const {return m_businessKeyCodeSpecId;}

    DGNDBSYNC_EXPORT DgnCode CreateCode(Utf8StringCR value) const;
    DgnCode CreateDebuggingCode(DgnV8EhCR v8eh);

    bool WantDebugCodes() const {return m_addDebugDgnCodes;}
    void SetWantDebugCodes(bool value) {m_addDebugDgnCodes = value;}

    //! @}

    //! @name Miscellaneous Converter Properties
    //! @{

    //! Get the number of elements converted.
    uint32_t GetElementsConverted() const {return m_elementsConverted;}

    //! Get the number of elements discarded.
    uint32_t GetElementsDiscarded() const {return m_elementsDiscarded;}

    //! Query if the converter should use DgnElementId when matching elements.
    DGNDBSYNC_EXPORT bool UseElementId() const;

    //! A subclass must override this to supply the prefix that should be applied to all generated names, such as Category and View codes.
    virtual Utf8String _GetNamePrefix() const = 0;

    Params const& GetParams() const {return _GetParams();}

    //! Returns the transform to apply to all in-coming V8 elements. This is usually identity. 
    //! It is non-identity in the case where we are pulling in a new dgnv8 file and we need to do a GCS or other coordinate transform to map it in.
    TransformCR GetRootTrans() const {return m_rootTrans;}

    //! Utility method to compare transforms with a tolerance.
    DGNDBSYNC_EXPORT static bool IsTransformEqualWithTolerance(TransformCR lhs, TransformCR rhs);

    //! @}

    //! @name Error and Progress Reporting
    //! @{

    //! This is the core function that is called to report an issue. The default implementation writes the issue to the issues file.
    DGNDBSYNC_EXPORT virtual void _ReportIssue(IssueSeverity, IssueCategory::StringId, Utf8CP message, Utf8CP context) const;

    //! Report an issue, where the message is to be formatted with addition arguments.
    DGNDBSYNC_EXPORT void ReportIssueV(IssueSeverity, IssueCategory::StringId, Issue::StringId, Utf8CP context, ...);

    //! Report an issue
    DGNDBSYNC_EXPORT void ReportIssue(IssueSeverity, IssueCategory::StringId, Issue::StringId, Utf8CP details, Utf8CP context = nullptr);

    //! Report an error
    DGNDBSYNC_EXPORT void ReportError(IssueCategory::StringId, Issue::StringId, Utf8CP details);

    //! Report an error, where the message is to be formatted with addition arguments.
    void ReportError(IssueCategory::StringId category, Issue::StringId issue, WCharCP details) {ReportError(category,issue,Utf8String(details).c_str());}

    //! Report a problem opening a V8 file
    DGNDBSYNC_EXPORT void ReportDgnV8FileOpenError(DgnV8Api::DgnFileStatus, WCharCP fn);

    //! Report a problem accessing a V8 file
    void ReportDgnFileStatus(BeSQLite::DbResult fileStatus, BeFileNameCR projectFileName);

    //! Report an issue, where the problem has mostly to do with provenance.
    DGNDBSYNC_EXPORT void ReportSyncInfoIssue(IssueSeverity, IssueCategory::StringId, Issue::StringId, Utf8CP details);

    //! Signal a fatal error
    DGNDBSYNC_EXPORT BentleyStatus OnFatalError(IssueCategory::StringId cat=IssueCategory::Unknown(), Issue::StringId num=Issue::FatalError(), ...) const;
    virtual void _OnFatalError() const {m_wasAborted=true;}

    //! Query if there was a fatal error
    bool WasAborted() const {return m_wasAborted;}

    //! Query if there were any conversion errors (not necessarily fatal).
    bool ReportedAnyErrors() const {return m_hadError;}

    //! Report progress and detect if user has indicated that he wants to cancel.
    DGNDBSYNC_EXPORT void ReportProgress() const;

    //! Report progress. 
    void ShowProgress() const {GetProgressMeter().ShowProgress();}
    
    //! Specify how many steps remain in the current task
    void AddSteps(int32_t n) const;
    //! Set the name of the current step. The progress meter is expected to update its display and decrement the number of remaining steps as a result.
    DGNDBSYNC_EXPORT void SetStepName(ProgressMessage::StringId, ...) const;
    //! Specify how many tasks remain in the conversion
    void AddTasks(int32_t n) const;
    //! Set the name of the current task. The progress meter is expected to update its display and decrement the number of remaining steps as a result.
    DGNDBSYNC_EXPORT void SetTaskName(ProgressMessage::StringId, ...) const;
    //! Get the progress meter.
    DGNDBSYNC_EXPORT DgnProgressMeter& GetProgressMeter() const;
    
    //! Set up the V8 progress meter
    void SetV8ProgressMeter();
    //! Remove the V8 progress meter
    void ClearV8ProgressMeter();
    
    //! Add model requiring reality tiles.
    void AddModelRequiringRealityTiles(DgnModelId id, Utf8StringCR sourceFile, SyncInfo::V8FileSyncInfoId fileId) { m_modelsRequiringRealityTiles.Insert(id, bpair<Utf8String, SyncInfo::V8FileSyncInfoId>(sourceFile, fileId)); }
    //! @}

    //! @name Change Monitoring
    //! @{
    Monitor& GetMonitor() {return *m_monitor;}
    void SetMonitor(Monitor& m) {m_monitor = &m;}
    //! @}

    //! @name  Change-Detection
    //! @{
    
    SyncInfo& GetSyncInfo() {return m_syncInfo;}
    DGNDBSYNC_EXPORT BentleyStatus AttachSyncInfo();

    // Get the change detector that is used by the current conversion.
    IChangeDetector& GetChangeDetector() {return _GetChangeDetector();}

    // A subclass must override this method to return the change detector that is used by the current conversion.
    virtual IChangeDetector& _GetChangeDetector() = 0;
    
    // Subclass should return true if it has a changedetector
    virtual bool _HaveChangeDetector() = 0;

    //! Returns true if the converter is running in update mode. In update mode, the converter expects to find models, elements, and supporting
    //! data such as Categories already in the BIM. It then uses the current change detector to detect differences and do updates.
    //! @note If this returns false, that does \em not necessarily mean that we are creating a new DgnDb. See IsCreatingNewDgnDb to see if that's true.
    //! We sometimes create a new DgnDb from a DGN file, and sometimes we import multiple DGN files into an existing DgnDb. This function returns false
    //! the first time that we see a given V8 root model and true if we are updating a root model that was previously converted.
    bool IsUpdating() const {return _GetParams().IsUpdating();}
    
    //! Returns true if the converter is helping to create a new DgnDb
    bool IsCreatingNewDgnDb() const {return _GetParams().IsCreatingNewDgnDb();}

    //! Return true if ElementId is not a reliable way to identify elements in the specified file.
    //! @param[in] v8file The file that is being converted.
    DGNDBSYNC_EXPORT virtual StableIdPolicy _GetIdPolicy(DgnV8FileR v8file) const;

    StableIdPolicy GetCurrentIdPolicy() const {return m_currIdPolicy;}

    //! Modify the user data linkage of a DgnV8 element in preparation for computing its digest or comparing it to a saved state.
    //! \a eeh is a temporary copy of the real DgnV8 element. It is not used to convert the element to DGNDB format.
    DGNDBSYNC_EXPORT virtual void _TweakLinkageForComparisonAndHashPurposes(DgnV8EehR eeh, DgnV8Api::ElementLinkageIterator& il);
    //! Modify the data of a DgnV8 element in preparation for computing its digest or comparing it to a saved state
    //! \a eeh is a temporary copy of the real DgnV8 element. It is not used to convert the element to DGNDB format.
    DGNDBSYNC_EXPORT virtual void _TweakElementForComparisonAndHashPurposes(DgnV8EehR eeh, DgnV8Api::MSElement const& elementData);

    //! Detect if the specified document still exists.
    //! If docGuidStr is a valid BeGuid, then the DMS is checked.
    //! Otherwise, localFileName is used to check the local file system.
    DGNDBSYNC_EXPORT bool DoesDocumentExist(Utf8StringCR docGuidStr, Utf8String localFileName);

    //! Delete all content derived from files that were recorded in syncinfo but not longer exist
    DGNDBSYNC_EXPORT virtual void _DetectDeletedDocuments();

    //! @private
    DGNDBSYNC_EXPORT virtual void _DeleteFileAndContents(SyncInfo::V8FileSyncInfoId filesid);
    //! @private
    DGNDBSYNC_EXPORT virtual void _DeleteModel(SyncInfo::V8ModelMapping const&);
    //! @private
    DGNDBSYNC_EXPORT virtual void _DeleteElement(DgnElementId);

};

//=======================================================================================
//! Helper functions to filter syncinfo queries
// @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct ElementFilters
{
    static IChangeDetector::T_SyncInfoElementFilter GetViewAttachmentElementFiter()
        {
        return [](SyncInfo::ElementIterator::Entry const& entry, Converter& converter)
            {
            return converter.GetDgnDb().Elements().Get<Sheet::ViewAttachment>(entry.GetElementId()).IsValid();
            };
        }
        
    static IChangeDetector::T_SyncInfoElementFilter GetDrawingElementFilter()
        {
        return [](SyncInfo::ElementIterator::Entry const& entry, Converter& converter)
            {
            return converter.GetDgnDb().Elements().Get<Drawing>(entry.GetElementId()).IsValid();
            };
        }

     static IChangeDetector::T_SyncInfoElementFilter GetViewDefinitionElementFiter()
        {
        return [](SyncInfo::ElementIterator::Entry const& entry, Converter& converter)
            {
            return converter.GetDgnDb().Elements().Get<ViewDefinition>(entry.GetElementId()).IsValid();
            };
         }
};

//=======================================================================================
//! A nop "change detector" that is used when a converter knows that it is importing
//! a set of V8 files for the first time. 
// @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct CreatorChangeDetector : IChangeDetector
{
    bool _ShouldSkipFileByName(Converter&, BeFileNameCR) override {return false;}
    bool _ShouldSkipFile(Converter&, DgnV8FileCR) override {return false;}
    bool _AreContentsOfModelUnChanged(Converter&, ResolvedModelMapping const&) override {return false;}
    bool _ShouldSkipLevel(DgnCategoryId&, Converter&, DgnV8Api::LevelHandle const&, DgnV8FileR, Utf8StringCR) override {return false;}
    void _OnElementSeen(Converter&, DgnElementId) override {}
    void _OnModelSeen(Converter&, ResolvedModelMapping const&) override {}
    void _OnModelInserted(Converter&, ResolvedModelMapping const&) override {}
    void _OnViewSeen(Converter&, DgnViewId) override {}
    void _DetectDeletedElements(Converter&, SyncInfo::ElementIterator&) override {}
    void _DetectDeletedElementsInFile(Converter&, DgnV8FileR) override {}
    void _DetectDeletedElementsEnd(Converter&) override {}
    void _DetectDeletedModels(Converter&, SyncInfo::ModelIterator&) override {}
    void _DetectDeletedModelsInFile(Converter&, DgnV8FileR) override {}
    void _DetectDeletedModelsEnd(Converter&) override {}
    void _DetectDeletedViews(Converter&, SyncInfo::ViewIterator&) override {}
    void _DetectDeletedViewsInFile(Converter&, DgnV8FileR) override {}
    void _DetectDeletedViewsEnd(Converter&) override {}
    
    DGNDBSYNC_EXPORT bool _IsElementChanged(SearchResults&, Converter&, DgnV8EhCR, ResolvedModelMapping const&, T_SyncInfoElementFilter* filter) override; // fills in element MD5 and returns true

    CreatorChangeDetector() {}
};

//=======================================================================================
//! ChangeDetector is a set of algorithms that can be used to detect changes to elements in V8 files,
//! plus algorithms that can update the corresponding elements in the output DgnDb file.
//! An updating converter uses an instance of ChangeDetector to detect changes.
// @bsiclass                                                    Sam.Wilson      04/2015
//=======================================================================================
struct ChangeDetector : IChangeDetector
{
    SyncInfo::ByV8ElementIdIter* m_byIdIter;
    SyncInfo::ByHashIter*       m_byHashIter;
    DgnElementIdSet             m_elementsSeen;
    bset<DgnViewId>             m_viewsSeen;

    bset<SyncInfo::V8ModelSyncInfoId> m_v8ModelsSeen;
    bset<SyncInfo::V8ModelSyncInfoId> m_v8ModelsSkipped;
    bset<SyncInfo::V8ModelSyncInfoId> m_newlyDiscoveredModels; // models created during this run

    void ReleaseIterators() { DELETE_AND_CLEAR(m_byIdIter); DELETE_AND_CLEAR(m_byHashIter); }
    void PrepareIterators(DgnDbCR db);

    DGNDBSYNC_EXPORT bool _ShouldSkipFile(Converter&, DgnV8FileCR) override;
    void _OnElementSeen(Converter&, DgnElementId id) override {if (id.IsValid()) m_elementsSeen.insert(id);}
    DGNDBSYNC_EXPORT void _Prepare(Converter&) override;
    DGNDBSYNC_EXPORT void _Cleanup(Converter&) override;
    DGNDBSYNC_EXPORT bool _ShouldSkipFileByName(Converter&, BeFileNameCR) override;
    bool _ShouldSkipLevel(DgnCategoryId&, Converter&, DgnV8Api::LevelHandle const&, DgnV8FileR, Utf8StringCR) override {return false;}
    DGNDBSYNC_EXPORT void _OnModelSeen(Converter&, ResolvedModelMapping const&);
    DGNDBSYNC_EXPORT void _OnModelInserted(Converter&, ResolvedModelMapping const&);
    DGNDBSYNC_EXPORT void _OnViewSeen(Converter&, DgnViewId id);
    DGNDBSYNC_EXPORT bool _AreContentsOfModelUnChanged(Converter&, ResolvedModelMapping const&) ;
    DGNDBSYNC_EXPORT bool _IsElementChanged(SearchResults&, Converter&, DgnV8EhCR, ResolvedModelMapping const&, T_SyncInfoElementFilter* filter) override;

    //! @name  Inferring Deletions - call these methods after processing all models in a conversion unit. Don't forget to call the ...End function when done.
    //! @{
    DGNDBSYNC_EXPORT void _DetectDeletedElements(Converter&, SyncInfo::ElementIterator&) override;    //!< don't forget to call _DetectDeletedElementsEnd when done
    DGNDBSYNC_EXPORT void _DetectDeletedElementsInFile(Converter&, DgnV8FileR) override;              //!< don't forget to call _DetectDeletedElementsEnd when done
    DGNDBSYNC_EXPORT void _DetectDeletedElementsEnd(Converter&) override {m_elementsSeen.clear();}

    DGNDBSYNC_EXPORT void _DetectDeletedModels(Converter&, SyncInfo::ModelIterator&) override;        //!< don't forget to call _DetectDeletedModelsEnd when done
    DGNDBSYNC_EXPORT void _DetectDeletedModelsInFile(Converter&, DgnV8FileR) override;                //!< don't forget to call _DetectDeletedModelsEnd when done
    DGNDBSYNC_EXPORT void _DetectDeletedModelsEnd(Converter&) override {m_v8ModelsSeen.clear();}

    DGNDBSYNC_EXPORT void _DetectDeletedViews(Converter&, SyncInfo::ViewIterator&) override;          //!< don't forget to call _DetectDeletedViewsEnd when done
    DGNDBSYNC_EXPORT void _DetectDeletedViewsInFile(Converter&, DgnV8FileR) override;                 //!< don't forget to call _DetectDeletedViewsEnd when done
    DGNDBSYNC_EXPORT void _DetectDeletedViewsEnd(Converter&) override { m_viewsSeen.clear(); }
    //! @}

    ChangeDetector() : m_byIdIter(nullptr), m_byHashIter(nullptr) {}
    DGNDBSYNC_EXPORT ~ChangeDetector();
};

//=======================================================================================
//! Base class for converters that import spatial data. All spatial converters
//! are based on the concept that there is one root spatial model. The root model
//! effectively defines the origin of the spatial coordinate system. All other models 
//! are related to the root in some way.
//! In root-model converters, other models are related to the root by modeling transforms. 
//! In tiled-file converters, other models are known to be in the root's coordinate system already.
//! The root model's DgnFile defines the levels and styles used by all other tiles.
//! 
//! <h2>ImportJob</h2>
//! You can project the contents of multiple V8 "projects" into a single BIM. A V8 project is defined as
//! a root model and a set of models that are related to it, where the elements in the models describe 
//! something in space. Each converted V8 project is called an ImportJob and is recorded in syncinfo.
//!
//! <h2>GCS</h2>
//! A BIM can have a Geographic Coordinate System (GCS). If so, all physical elements in the BIM are in this GCS. The BIM may also have a global
//! origin (which is a fixed offset applied to all physical coordinates).
//! A BIM can get its GCS in one of three ways:
//! 1. The BIM may be created by a native BIM app, which defines the BIM's GCS directly.
//! 1. The BIM may be created by the converter from a V8 root model that has a GCS. In that case, the BIM's GCS is set up to match the GCS of the root model. The BIM's global origin is also taken from the root model.
//! 1. The BIM may be created by the converter, and the caller specifies a GCS in SpatialParams.
//!
//! A given root model and its references are mapped into the GCS of the BIM, as follows:
//! 1. If the BIM has a GCS and the input root model does not, then the converter assumes that the origin of the root V8 model coincides with 0,0,0 of the DgnDb's GCS.
//! Suitable corrections are made for differences in global origin.
//! 1. If the BIM and the input root model both have the same GCS, then the converter knows they have the same geo-origin.
//! Suitable corrections are made for differences in global origin.
//! 1. If the BIM and the input root model have different GCSs, then the converter will reproject the V8 model and its reference attachments into the GCS of the BIM. 
//! 
// @bsiclass                                                    Sam.Wilson      04/2015
//=======================================================================================
struct SpatialConverterBase : Converter
{
    DEFINE_T_SUPER(Converter)

public:
    //! The status of attempting to create a new ImportJob
    enum class ImportJobCreateStatus
        {
        Success,                    //!< The ImportJob was created successfully
        FailedExistingRoot,         //!< The ImportJob could not be created, because an ImportJob already exists for the root file. Try update instead.
        FailedExistingNonRootModel, //!< The ImportJob could not be created, because the selected root model for the root file was already brought into the BIM as a reference attachment to some other ImportJob
        FailedInsertFailure         //!< The ImportJob could not be created for unknown reasons
        };

    //! The status of attempting to load an existing ImportJob
    enum class ImportJobLoadStatus 
        {
        Success,                    //!< The ImportJob was loaded successfully
        FailedNotFound,             //!< The ImportJob could not be loaded, because no ImportJob exists for the root file. Try embed instead.
        };

    //! Parameters that specify the inputs to the conversion process
    struct SpatialParams : Params
    {
        BeFileName              m_rootFileName;
        Utf8String              m_namePrefix;
        DgnModelId              m_rootModelId;
        iModelBridge::GCSCalculationMethod    m_gcsCalculationMethod{};

        void SetNamePrefix(BentleyApi::Utf8CP s) {m_namePrefix=s;}
        Utf8String GetNamePrefix() const {return m_namePrefix;}

        void SetInputGcsDefinition(iModelBridge::GCSDefinition const& gcs) {m_inputGcs=gcs;}
        iModelBridge::GCSDefinition GetInputGcsDefinition() const {return m_inputGcs;}

        void SetOutputGcsDefinition(iModelBridge::GCSDefinition const& gcs) {m_outputGcs=gcs;}
        iModelBridge::GCSDefinition GetOutputGcsDefinition() const {return m_outputGcs;}

        void SetGcsCalculationMethod(iModelBridge::GCSCalculationMethod method) {m_gcsCalculationMethod = method;}
        iModelBridge::GCSCalculationMethod GetGcsCalculationMethod() const {return m_gcsCalculationMethod;}
    };

    //! How or if a given V8 model is known to the BIM.
    enum class ImportJobModelStatus {Unknown, ExistingRoot, ExistingNonRootModel};

protected:
    bool m_isRootModelSpatial {}; // if false, then we cannot do the spatial part of the conversion 
    // double m_rootScaleFactor; // *** NEEDS WORK: What is this for? It is never set anywhere in the converter.
    mutable DgnFilePtr m_rootFile;
    mutable DgnV8ModelRefP m_rootModelRef {};
    ResolvedModelMapping m_rootModelMapping;
    ResolvedImportJob m_importJob;
    SubjectCPtr m_spatialParentSubject;
    DgnGCSPtr m_outputDgnGcs; // This is the GCS to which we are converting input DgnV8 models.

    enum class ModelSubjectType {Hierarchy, References};
    SubjectCPtr GetOrCreateModelSubject(SubjectCR parent, Utf8StringCR, ModelSubjectType);

    virtual void _SetChangeDetector(bool isUpdate) = 0;

    virtual void _AddResolvedModelMapping(ResolvedModelMapping const&) {}

    BentleyStatus FindRootModelFromImportJob();
    void ApplyJobTransformToRootTrans();
    void DetectRootTransformChange();
    void CorrectSpatialTransform(ResolvedModelMapping&);

    BentleyStatus MakeSchemaChanges(bvector<DgnFileP> const&, bvector<DgnV8ModelP> const&);
    void CreateProvenanceTables();

    SpatialConverterBase(SpatialParams const& p) : T_Super(p) {}

    DgnV8Api::ModelInfo const& _GetModelInfo(DgnV8ModelCR v8Model) override { return m_rootModelRef->GetDgnModelP()->GetModelInfo(); }

public:
    virtual SpatialParams const& _GetSpatialParams() const = 0;

    //! Sets the Params BridgeJobName property
    DGNDBSYNC_EXPORT void ComputeDefaultImportJobName();

    //! Delete all content derived from files that were recorded in syncinfo but not longer exist
    DGNDBSYNC_EXPORT void _DetectDeletedDocuments() override;

    //! @name  Root Model
    //! @{
    //! Get the name of the root file as specified by the caller in the input params.
    BeFileNameCR GetRootFileName() const {return _GetSpatialParams().GetInputFileName();}
    
    //! Get the currently open root V8 file or nullptr if no root file is open.
    DgnFileP GetRootV8File() {return m_rootFile.get();}

    //! Set the currently open root V8 file or nullptr if no root file is open.
    void SetRootV8File(DgnFileP rootFile) { m_rootFile = rootFile; }

    //! Get the currently open root model as a modelref.
    DgnV8ModelRefP GetRootModelRefP() {return m_rootModelRef;}

    //! Get the currently open root model as a DgnV8Model.
    DgnV8ModelP GetRootModelP() {return GetRootModelRefP()->GetDgnModelP();}

    //! Test if we should promote 2D models to 3D. This option is useful only 
    //! in special cases. Reference attachments to such a 2D root model will be ignored.
    DGNDBSYNC_EXPORT bool Promote2dTo3d(bool isRoot) const;

    DgnV8FileR _GetFontRootV8File() override {return *GetRootV8File();}
    //! @}

    //! @name  ImportJob - a root model (and related models) that was previously converted and projected into a BIM.
    //! @{

    Utf8String _GetNamePrefix() const override {return _GetSpatialParams().GetNamePrefix();}

    //! Attempt to load an existing import job. Possible only if this converter converted this data source previously. This function is called after _OpenSource.
    DGNDBSYNC_EXPORT ImportJobLoadStatus FindJob();

    //! Create a new import job and the information that it depends on. Called when FindJob fails, indicating that this is the initial conversion of this data source.
    //! @param comments         Optional description of the job
    //! @param v8ConverterType  type of V8 converter
    protected:
    DGNDBSYNC_EXPORT ImportJobCreateStatus InitializeJob(Utf8CP comments, SyncInfo::ImportJob::Type v8ConverterType);
    public:

    //! Query the job info for this conversion.
    ResolvedImportJob const& GetImportJob() const {return m_importJob;}

    //! Get the job subject. This is the parent of all job-wide resources, such as the physical root model subject, the drawings list, etc.
    SubjectCR _GetJobSubject() const override {return GetImportJob().GetSubject();}

    //! Get the current spatial parent subject. This is the root model subject at the outset and is then set to referenced model subjects as we recurse through the hierarchy.
    SubjectCR _GetSpatialParentSubject() const override {BeAssert(m_spatialParentSubject.IsValid()); return *m_spatialParentSubject;}

    //! Set the current spatial parent subject. This is the root model subject at the outset and is then set to referenced model subjects as we recurse through the hierarchy.
    void SetSpatialParentSubject(SubjectCR s) {m_spatialParentSubject = &s;}

    //! @}

    //! @name  Converting Elements
    //! @{
    DGNDBSYNC_EXPORT virtual void _ConvertSpatialElement(ElementConversionResults&, DgnV8EhCR v8eh, ResolvedModelMapping const&);
    DGNDBSYNC_EXPORT BentleyStatus DoConvertSpatialElement(ElementConversionResults&, DgnV8EhCR v8eh, ResolvedModelMapping const& m, bool isNewElement);
    void ConvertElementList(DgnV8Api::PersistentElementRefList* list, ResolvedModelMapping const&);
    //! @}

    //! @name  Converting Rasters
    //! @{
    DGNDBSYNC_EXPORT virtual BentleyStatus _ConvertRasterElement(DgnV8EhCR v8eh, ResolvedModelMapping const&, bool copyRaster, bool isNewElement);
    DGNDBSYNC_EXPORT virtual bool _RasterMustBeExported(Dgn::ImageFileFormat fileFormat);
    //! @}

    //! @name  Converting Point Clouds
    //! @{
    DGNDBSYNC_EXPORT virtual BentleyStatus _ConvertPointCloudElement(DgnV8EhCR v8eh, ResolvedModelMapping const&, bool copyPointCloud, bool isNewElement);
    void ConvertV8PointCloudViewSettings(SpatialViewControllerR, DgnV8ViewInfoCR viewInfo);
    //! @}

    //! @name Converting or creating a GCS
    //! @{
    //! Set up the output DgnDb's GCS. By default, this method does nothing if the DgnDb already has a GCS.
    //! If not, it creates the DgnDb's GCS to the GCS that is specified by Params::GCSDefinition, if any.
    //! If none is specified, then it leaves the DgnDb's GCS unset. (That is no an error.)
    //! @note This method sets m_outputDgnGcs to the DgnDb's GCS.
    //! @return non-zero if a GCS was specified but is invalid. Otherwise, return 0 (even if no GCS is defined).
    DGNDBSYNC_EXPORT virtual DgnDbStatus _SetOutputGCS();

    //! Sets up m_rootTrans to be used to transform points in the source root model to the DgnDb's coordinate system and sets up the global origin.
    //! @note This function might re-open the root file and might re-define the root model.
    //! At a minimum, this function sets up m_rootTrans to scale the source points to meters.
    //! When creating a new DgnDb, this function also adopt's root model's global origin.
    //! When creating a new DgnDb, this function sets up the DgnDb's GCS, either by adopting the root model's GCS or defining the user-specified GCS.
    //! When updating a DgnDb, this function sets up the transformation from the source root's GCS to the DgnDb's GCS, if any.
    //! This might entail a full GCS reprojection (in which case m_rootTrans will contain on the units scale factor). In that case, it
    //! reopens the root file and redefines the root model. Or, the GCS transformation might be simple enough that it can be built into m_rootTrans
    //! as a simple linear transformation between GCS origins (and global origins) and azimuth angles.
    //! This function is called just after the root model is detected.
    DGNDBSYNC_EXPORT virtual DgnV8Api::DgnFileStatus _ComputeCoordinateSystemTransform();

    //! Called from to compute the transform and global origin from the source root model to the BIM file's coordinates.
    DGNDBSYNC_EXPORT void ComputeTransformAndGlobalOriginFromRootModel(DgnV8ModelCR rootModel, bool adoptSourceGoIfBimIsEmpty);

    DgnGCS* GetDgnGcs() {return m_outputDgnGcs.get();}
    //! @}

    //! @name Converting levels
    //! @{
    DGNDBSYNC_EXPORT virtual void _ConvertSpatialLevelTable(DgnV8FileR v8file);
    DGNDBSYNC_EXPORT void _OnUpdateLevel(DgnV8Api::LevelHandle const& level, DgnCategoryId cat, DgnV8FileR file) override;
    //! @}
};

//=======================================================================================
//! Project a single DgnV8 model, plus all of its reference attachments, into a DgnDb.
//!
//! Note that the output DgnDb might be new or it might already contain other content.
//! See IsCreatingNewDgnDb.
//!
//! <h3>ECSchemas</h3>
//! RootModelConverter always attempts to import the ECSchemas used by the input DgnV8 root model at its attachments.
//! If the DgnDb is new, then this is a straight import. If the DgnDb is not new,
//! then RootModelConverter will reject an input ECSchema that already exists in the DgnDb but was created from a different version.
//!
//! <h3>Levels and Styles</h3>
//! RootModelConverter always attempts to import the levels and styles used by the input DgnV8 root model at its attachments.
//! If the DgnDb is new, then this is a straight import. If the DgnDb is not new,
//! then RootModelConverter will attempt to merge the incoming levels and styles into the DgnCategories and styles that are already in the DgnDb.
//!
//! <h3>The root model and physical coordinates</h3>
//! The starting 3-D design model is called the "root model". This model determines origin of the
//! physical coordinate system of the DgnDb. RootModelConverter converts
//! all 3-D design models reachable from the root model by means of DgnAttachments and
//! transforms them into the DgnDb's physical coordinate system. Note that all models are converted
//! to use meters as storage units. Each model preserves its working units. Only 3-D design models
//! are converted into the DgnDb's physical coordinate space.
//! By default, the root model is the root of the first view in the active view group of the specified root V8 file.
//! Call #SetChooseRootModel to specify a different root model.
//!
//! If the DgnDb is new, then RootModelConverter defines physical coordinate system of the DgnDb from the input DgnV8 root model.
//! If the DgnDb is not new, then RootModelConverter transforms the input DgnV8 root model and its attachments into
//! the DgnDb's existing coordinate system.
//!
//! The DgnDb's physical coordinate system is optionally paired with a Geographic Coorindate System (GCS) and a global origin.
//! If a DgnDb has a GCS, then geo-located DgnV8 models are transformed or reprojected into the DgnDb's GCS as necessary.
//!
//! @note Normally the root model must be 3-D.
//! The Configuration Consider2dModelsSpatial option can be used to cause 2d models (other than Drawing and Sheet models) to be converted to spatial models.
//!
//! @note 3-D V8 Sheet models are *always* treated as 2-D.
//!
// @bsiclass                                                    Keith.Bentley   01/15
//=======================================================================================
struct RootModelConverter : SpatialConverterBase
{
    DEFINE_T_SUPER(SpatialConverterBase)

    struct RootModelChoice
    {
        enum class Method {
            ByName,                 //!< Choose the root model by name
            ById,                   //!< Choose the root model by V8 ModelId
            FromActiveViewGroup,    //!< Choose the root of the first open view of the active view group as the root
            UseDefaultModel         //!< Use the default model of the root file as the root model
            };
        bool m_isSet;
        Method m_method;
        DgnV8Api::ModelId m_id;
        Utf8String m_name;

        RootModelChoice() : m_isSet(false), m_method(Method::FromActiveViewGroup) {}
        RootModelChoice(DgnV8Api::ModelId id) : m_isSet(true), m_method(Method::ById), m_id(id) {}
        RootModelChoice(Utf8StringCR n) {SetModelName(n.c_str());}

        void SetUseActiveViewGroup() {m_isSet=true; m_method = Method::FromActiveViewGroup;}
        void SetUseDefaultModel() {m_isSet=true; m_method = Method::UseDefaultModel;}
        void SetModelName(Utf8CP name) {m_isSet=true; m_method = Method::ByName; m_name.assign(name);}

        bool IsSet() const {return m_isSet;}
    };

    //! Parameters that specify the inputs to the conversion process
    struct RootModelSpatialParams : SpatialParams
    {
        RootModelChoice m_rootModelChoice;
        bool m_considerNormal2dModelsSpatial {};

        void SetConsiderNormal2dModelsSpatial(bool b) {m_considerNormal2dModelsSpatial=b;}
        bool GetConsiderNormal2dModelsSpatial() const {return m_considerNormal2dModelsSpatial;}

        void SetRootModelChoice(RootModelChoice const& c) {m_rootModelChoice=c;}
        RootModelChoice const& GetRootModelChoice() const {return m_rootModelChoice;}
        void AddDrawingOrSheetFile(BeFileNameCR fn) {m_drawingAndSheetFiles.push_back(fn);}

        DGNDBSYNC_EXPORT void Legacy_Converter_Init(BeFileNameCR bcName);

    };

protected:
    RootModelSpatialParams& m_params;   // NB: Must store a *reference* to the bridge's Params, as they may change after our constructor is called
    bvector<DgnV8FileP> m_v8Files;
    bvector<Bentley::DgnModelPtr> m_drawingModelsKeepAlive;
    bvector<Bentley::DgnFilePtr> m_filesKeepAlive;
    DgnV8Api::ViewGroupPtr m_viewGroup;
    bmultiset<ResolvedModelMapping> m_v8ModelMappings; // NB: the V8Model pointer is the key
    bvector<DgnV8ModelP> m_spatialModelsInAttachmentOrder;
    bset<DgnV8ModelP> m_spatialModelsSeen;
    bset<DgnV8ModelP> m_nonSpatialModelsSeen;
    bvector<DgnV8ModelP> m_nonSpatialModelsInModelIndexOrder;
    std::unique_ptr<IChangeDetector> m_changeDetector;
    bool m_considerNormal2dModelsSpatial;   // Unlike the member in RootModelSpatialParams, this considers the config file, too. It is checked often, so calulated once in the constructor.

    void CorrectSpatialTransforms();
    bool ShouldCorrectSpatialTransform(ResolvedModelMapping const& rmm) {return rmm.GetDgnModel().IsSpatialModel() && IsFileAssignedToBridge(*rmm.GetV8Model().GetDgnFileP());}

    bool _HaveChangeDetector() override {return m_changeDetector != nullptr;}
    IChangeDetector& _GetChangeDetector() override {return *m_changeDetector;}
    DGNDBSYNC_EXPORT void _SetChangeDetector(bool isUpdate) override;

    //! @name Params
    //! @{
    Params const& _GetParams() const override {return m_params;}
    Params& _GetParamsR() override {return m_params;}
    SpatialParams const& _GetSpatialParams() const override {return m_params;}
    //! @}

    //! @name  Models
    //! @{
    DGNDBSYNC_EXPORT void _AddResolvedModelMapping(ResolvedModelMapping const&) override;
    DGNDBSYNC_EXPORT ResolvedModelMapping _FindModelForDgnV8Model(DgnV8ModelR v8Model, TransformCR) override;
    DGNDBSYNC_EXPORT ResolvedModelMapping _FindFirstModelMappedTo(DgnV8ModelR v8Model) override;
    DGNDBSYNC_EXPORT ResolvedModelMapping _GetModelForDgnV8Model(DgnV8ModelRefCR, TransformCR) override;
    ResolvedModelMapping GetModelForDgnV8Model(DgnV8ModelRefCR v8Model, TransformCR toBim) {return _GetModelForDgnV8Model(v8Model, toBim);}
    DGNDBSYNC_EXPORT ResolvedModelMapping MapDgnV8ModelToDgnDbModel(DgnV8ModelR, TransformCR, DgnModelId targetModelId); // Like GetModelForDgnV8Model, except that caller already knows the target model
    DGNDBSYNC_EXPORT void _OnDrawingModelFound(DgnV8ModelR v8model) override;
    DGNDBSYNC_EXPORT void _KeepFileAlive(DgnV8FileR) override;
    DGNDBSYNC_EXPORT ResolvedModelMapping _FindResolvedModelMappingBySyncId(SyncInfo::V8ModelSyncInfoId sid) override;
    DGNDBSYNC_EXPORT bvector<ResolvedModelMapping> FindMappingsToV8Model(DgnV8ModelR v8Model);
    bool IsLessInMappingOrder(DgnV8ModelP a, DgnV8ModelP b);
    void UnmapModelsNotAssignedToBridge();

    // in the RootModelConverter, treatment of normal 2d models depends the user's input parameters.
    DGNDBSYNC_EXPORT bool _ConsiderNormal2dModelsSpatial() override;

    //! @}

    //! @name  ECRelationships
    //! @{
    BentleyStatus DropElementRefersToElementsIndices(bmap<Utf8String, Utf8String>& indexDdlList);
    BentleyStatus RecreateElementRefersToElementsIndices(bmap<Utf8String, Utf8String>& indexDdlList);
    void ConvertNamedGroupsAndECRelationships();
    BentleyStatus ConvertECRelationships();
    BentleyStatus ConvertNamedGroupsRelationships();
    //! @}

    //! @name  V8Files
    //! @{
    //! Override to make sure that all files encountered by the converter are cached in m_v8Files
    DGNDBSYNC_EXPORT SyncInfo::V8FileProvenance _GetV8FileIntoSyncInfo(DgnV8FileR, StableIdPolicy) override;

    //! @}

    //! @name The RootModelConverter framework
    //! @{
    DGNDBSYNC_EXPORT virtual DgnV8Api::ModelId _GetRootModelId();
    DGNDBSYNC_EXPORT virtual DgnV8Api::ModelId _GetRootModelIdFromViewGroup();
    // Then call BootstrapImportJob
    DGNDBSYNC_EXPORT virtual DgnV8Api::DgnFileStatus _InitRootModel();
    DGNDBSYNC_EXPORT virtual void _BeginConversion();
    DGNDBSYNC_EXPORT virtual void _ConvertSpatialViews();
    DGNDBSYNC_EXPORT virtual void _ConvertSpatialLevels();
    DGNDBSYNC_EXPORT virtual void _ConvertLineStyles();
    DGNDBSYNC_EXPORT virtual void _ConvertModels();
    DGNDBSYNC_EXPORT virtual void _ConvertSpatialElements();
    DGNDBSYNC_EXPORT virtual void _ConvertSheets();
    DGNDBSYNC_EXPORT virtual void _ConvertDrawings();
    DGNDBSYNC_EXPORT virtual void _FinishConversion();
    //! override this to filter out specific DgnAttachments from the spatial model hierarchy
    virtual bool _WantAttachment(DgnAttachmentCR attach) const {return true;}
    
    //! override this to control how drawing and sheet models are found
    DGNDBSYNC_EXPORT virtual void _ImportDrawingAndSheetModels(ResolvedModelMapping& rootModelMapping);

    //! @}

    //! @private
    void ConvertElementsInModel(ResolvedModelMapping const& v8Model);
    //! @private
    void DoConvertSpatialElements();
    //! @private
    void ForceAttachmentsToSpatial(Bentley::DgnAttachmentArrayR attachments);
    //! @private
    void ImportSpatialModels(bool& haveFoundSpatialRoot, DgnV8ModelRefR, TransformCR);
    //! @private
    void UpdateCalculatedProperties();
    //! @private
    void CreatePresentationRules();

    void FindSpatialV8Models(DgnV8ModelRefR rootModelRef);
    void FindV8DrawingsAndSheets();
    void RegisterNonSpatialModel(DgnV8ModelR);
    void RegisterSheetModel(DgnV8FileR v8File, DgnV8Api::ModelIndexItem const& item);
    void RegisterDrawingModel(DgnV8FileR v8File, DgnV8Api::ModelIndexItem const& item);
    void Transform2dAttachments(DgnV8ModelR v8ParentModel);


public:
    static WCharCP GetRegistrySubKey() {return L"IModelBridgeForMstn";}

    DGNDBSYNC_EXPORT explicit RootModelConverter(RootModelSpatialParams&);
    DGNDBSYNC_EXPORT  ~RootModelConverter();

    DGNDBSYNC_EXPORT BentleyStatus MakeSchemaChanges();

    //! Create a new import job and the information that it depends on. Called when FindJob fails, indicating that this is the initial conversion of this data source.
    //! The name of the job is specified by _GetParams().GetBridgeJobName(). This must be a non-empty string that is unique among all job subjects.
    //! @param comments         Optional description of the job
    ImportJobCreateStatus InitializeJob(Utf8CP comments = nullptr) {return T_Super::InitializeJob(comments, SyncInfo::ImportJob::Type::RootModels);}

    //! Try to open the root model that is specified in SpatialParams.
    //! When running in update mode, this function looks up the ImportJob from the previous conversion gets the root model from that.
    //! @note You can only have one root model for a given root file.
    //! @return non-zero error status if the root file could not be opened or the root model could not be found.
    //! @see SpatialParams::SetRootFileName for how the root file is specified.
    //! @see _GetRootModelId for how the root model is specified. Note that subclasses define the root model within the root file in different ways.
    DGNDBSYNC_EXPORT DgnV8Api::DgnFileStatus InitRootModel() {return _InitRootModel();}

    //! Allow access for PowerProduct element handler bridge-extensions. V8Files have appdata tracing back to Repository Links (to decorate).
    //! @return bvector with const v8Files for this converter.
    DGNDBSYNC_EXPORT bvector<DgnV8FileP> const & GetV8Files() const { return m_v8Files; }

    //! Do the conversion. @see HadFatalError
    DGNDBSYNC_EXPORT BentleyStatus Process();
};

//=======================================================================================
//! Convert a collection of DgnV8 files that are organized in tiles, not by reference attachments, into a DgnDb.
//! @note Only the default model of each file is converted.
//! @note For now, tiled file conversion does \em not support GCS reprojection 
// @bsiclass                                                    Keith.Bentley   02/15
//=======================================================================================
struct TiledFileConverter : SpatialConverterBase
{
    DEFINE_T_SUPER(SpatialConverterBase)

    struct ChangeDetectorForTiles : ChangeDetector
    {
        DgnV8FileR m_rootFile;
        DGNDBSYNC_EXPORT bool _ShouldSkipLevel(DgnCategoryId&, Converter&, DgnV8Api::LevelHandle const& v8Level, DgnV8FileR v8File, Utf8StringCR dbCategoryName) override;
        ChangeDetectorForTiles(DgnV8FileR f) : m_rootFile(f) {}
    };

protected:
    SpatialParams& m_params; // NB: Must store a *reference* to the bridge's Params, as they may change after our constructor is called
    std::unique_ptr<IChangeDetector> m_changeDetector;
    
    bool _HaveChangeDetector() override {return m_changeDetector != nullptr;}
    IChangeDetector& _GetChangeDetector() override {return *m_changeDetector;}
    DGNDBSYNC_EXPORT void _SetChangeDetector(bool isUpdate) override;

    Params const& _GetParams() const override {return m_params;}
    Params& _GetParamsR() override {return m_params;}
    SpatialParams const& _GetSpatialParams() const override {return m_params;}

    //! The TiledFileConverter framework
    DGNDBSYNC_EXPORT virtual void _BeginConversion();
    DGNDBSYNC_EXPORT virtual void _FinishConversion();
    DGNDBSYNC_EXPORT virtual DgnV8Api::DgnFileStatus _InitRootModel();
    DGNDBSYNC_EXPORT virtual void _ConvertSpatialLevels();
    DGNDBSYNC_EXPORT virtual void _ConvertLineStyles();
    DGNDBSYNC_EXPORT virtual void _ConvertElementsInModel(ResolvedModelMapping const&);
    DGNDBSYNC_EXPORT virtual void _ConvertSpatialViews();
    DGNDBSYNC_EXPORT virtual void _OnFileComplete(DgnV8FileR v8File);
    virtual bool _FilterTileByName(BeFileNameCR name) {return false;}
    ResolvedModelMapping _FindResolvedModelMappingBySyncId(SyncInfo::V8ModelSyncInfoId sid) override {BeAssert(false && "TBD"); return ResolvedModelMapping();}

    DgnV8Api::ModelId GetDefaultModelId(DgnV8FileR v8File);
    DGNDBSYNC_EXPORT ResolvedModelMapping _GetModelForDgnV8Model(DgnV8ModelRefCR v8ModelRef, TransformCR) override;
    ResolvedModelMapping _FindModelForDgnV8Model(DgnV8ModelR v8Model, TransformCR) override {return m_rootModelMapping;}
    ResolvedModelMapping _FindFirstModelMappedTo(DgnV8ModelR v8Model) override {return m_rootModelMapping;}
    ResolvedModelMapping MapDgnV8ModelToDgnDbModel(DgnV8ModelR v8Model, DgnModelId targetModelId);
    void _OnDrawingModelFound(DgnV8ModelR v8model) override {}
    void _KeepFileAlive(DgnV8FileR) override {}

    // in the tiled converter, we always consider normal 2d models to be spatial models.
    bool _ConsiderNormal2dModelsSpatial() override {return true;}

    DGNDBSYNC_EXPORT SyncInfo::ImportJob GenerateImportJobInfo();
    DGNDBSYNC_EXPORT void ConvertElements(ResolvedModelMapping const&);

public:
    static WCharCP GetRegistrySubKey() {return L"TiledDgnV8Bridge";}

    TiledFileConverter(SpatialParams& p) : T_Super(p), m_params(p) {;}
    //! Create a new import job and the information that it depends on. Called when FindJob fails, indicating that this is the initial conversion of this data source.
    //! @param comments         Optional description of the job
    ImportJobCreateStatus InitializeJob(Utf8CP comments = nullptr) {return T_Super::InitializeJob(comments, SyncInfo::ImportJob::Type::TiledFile);}
    //! Try to open the root model that is specified in SpatialParams.
    //! When running in update mode, this function looks up the root model from the existing ImportJob.
    //! @return non-zero error status if the root file could not be opened or the root model could not be found.
    //! @see SpatialParams::SetRootFileName for how the root file is specified.
    //! @see _GetRootModelId for how the root model is specified. Note that subclasses define the root model within the root file in different ways.
    DGNDBSYNC_EXPORT DgnV8Api::DgnFileStatus InitRootModel() {return _InitRootModel();}
    DGNDBSYNC_EXPORT void ConvertRootModel();
    DGNDBSYNC_EXPORT void ConvertTile(BeFileNameCR);
    DGNDBSYNC_EXPORT void FinishedConversion() {_FinishConversion(); _OnConversionComplete();}
    DGNDBSYNC_EXPORT BentleyStatus MakeSchemaChanges();

};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct SpatialViewFactory : ViewFactory
{
    SpatialConverterBase& m_spatialConverter;
    ViewDefinitionPtr _MakeView(Converter& converter, ViewDefinitionParams const&) override;
    ViewDefinitionPtr _UpdateView(Converter& converter, ViewDefinitionParams const&, DgnViewId viewId) override;
    bool _Is3d() const override final {return true;}
    SpatialViewFactory(SpatialConverterBase& s) : m_spatialConverter(s) {}
};

/*=================================================================================**//**
  Converts line styles and line style components
  @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LineStyleConverter : RefCountedBase
{
private:
    struct V8Location
        {
private:
        bool                 m_isElement;
        SyncInfo::V8FileSyncInfoId   m_v8fileId;             //  Foreign file ID or RSC handle
        DgnV8Api::ElementId  m_v8componentKey;       //  Element ID or RSC ID
        uint32_t             m_v8componentType;      //  LsResourceType or LsElementType
public:
        V8Location(DgnV8Api::LsCacheComponent const& component, Converter&);
        V8Location() : m_isElement(false), m_v8componentKey(0), m_v8componentType(0) {}
        V8Location(V8Location const& l) : m_isElement(l.m_isElement), m_v8fileId(l.m_v8fileId), m_v8componentKey(l.m_v8componentKey), m_v8componentType(l.m_v8componentType) {}
        bool operator< (V8Location const &o) const;
        };

    bmap<V8Location, uint32_t>      m_v8ComponentToV10Id;
    bmap<Utf8String, DgnStyleId>    m_lsDefNameToIdMap;
    Converter&                      m_converter;
    DgnV8ModelP                     m_rootModel {};

    LineStyleStatus ConvertLsComponent(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheComponent const& component, double lsScale);
    DgnDbR GetDgnDb() {return m_converter.GetDgnDb();}
    SyncInfo& GetSyncInfo() {return m_converter.GetSyncInfo();}
    DgnV8ModelP GetRootModel() {return m_rootModel;}

    static void SetDescription(Dgn::V10ComponentBase*v10, DgnV8Api::LsCacheComponent const& component);
    static void SetDescription(JsonValueR json, DgnV8Api::LsCacheComponent const& component);

    LineStyleStatus ConvertCompoundComponent(Dgn::LsComponentId&v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheCompoundComponent const& compoundComponent, double lsScale);
    LineStyleStatus ConvertLineCode(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheStrokePatternComponent const& strokePattern, double lsScale);
    LineStyleStatus ConvertLinePoint(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCachePointComponent const& linePointComponent, double lsScale);
    LineStyleStatus ConvertPointSymbol(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheSymbolComponent const& symbolComponent, double lsScale);
    LineStyleStatus ConvertRasterImageComponent(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsRasterImageComponent const& rasterComponent);
    LineStyleStatus CreateRasterImageComponent(Dgn::LsComponentId& v10Id, bvector<uint8_t>&data, uint32_t width, uint32_t height, uint32_t flags, double trueWidth, double lsScale);
    LineStyleStatus ConvertElementLineCode(DgnStyleId& newId, uint32_t lineCode);
    //  void GenerateImageOfLineStyleIteration(Dgn::LsComponentId& v10Id, Utf8CP lineStyleName, LsComponentP topComponent, uint32_t minimumPixelSize);

    LineStyleConverter(Converter& converter) : m_converter(converter) {}

public:
    static RefCountedPtr<LineStyleConverter> Create(Converter& converter);
    LineStyleStatus ConvertLineStyle(DgnStyleId& newId, double& componentScale, DgnV8Api::LsDefinition*v8ls, DgnV8Api::DgnFile&v8File);
    LineStyleStatus GetOrConvertElementLineCode(DgnStyleId& newId, uint32_t lineCode);
    void SetRootModel(DgnV8ModelP r) {m_rootModel=r;}
};

typedef BisConversionTargetModelInfo const& BisConversionTargetModelInfoCR;

//=======================================================================================
// This extension is used during DgnV8 > DgnDb conversion to allow handlers to be directly involved in the conversion process.
// @bsiclass                                                    Jeff.Marker     08/2015
//=======================================================================================
struct ConvertToDgnDbElementExtension : DgnV8Api::Handler::Extension
{
    DGNV8_ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS(ConvertToDgnDbElementExtension, DGNDBSYNC_EXPORT);

    enum class Result {Proceed=0, SkipElement=1};
    virtual Result _PreConvertElement(DgnV8EhCR, Converter&, ResolvedModelMapping const&) {return Result::Proceed;}
    virtual BisConversionRule _DetermineBisConversionRule(DgnV8EhCR v8eh, DgnDbR dgndb, BisConversionTargetModelInfoCR) {return BisConversionRule::ToDefaultBisBaseClass;}
    virtual void _DetermineElementParams(DgnClassId&, DgnCode&, DgnCategoryId&, DgnV8EhCR, Converter&, ECObjectsV8::IECInstance const* primaryV8Instance, ResolvedModelMapping const&) {/* do nothing to let caller fallback */ }
    virtual void _ProcessResults(ElementConversionResults&, DgnV8EhCR, ResolvedModelMapping const&, Converter&) {/* do nothing to accept basic conversion */ }
    virtual bool _GetBasisTransform(Bentley::Transform&, DgnV8EhCR, Converter&) {return false; /* caller will derive placement transform from geometry */ }
    virtual void _InitDgnDomain() {}/*Callers can initialize their domain here and register with DgnDomains*/
    virtual void _ImportSchema(DgnDbR) {} /* extension may import schemas. NB: call db.BriefcaseManager().LockSchemas() before calling db.ImportSchemas */
    virtual bool _IgnorePublicChildren() {return false;} // When true, don't create an assembly for a V8 cell with public children unless there are category changes.
    virtual bool _DisablePostInstancing() {return false;} // When true, don't try to detect identical geometry and create GeometryParts from non-instanced V8 geometry.
    virtual void _UpdateResourceDefinitions (iModelBridge::IDocumentPropertiesAccessor& accessor) {}
};

//=======================================================================================
//! Interface for a "cross-cutting domain" - an extension that is applied to every element,
//! regardless of what Element Handler it has.
//! @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct XDomain
{
    /* Use this callback to initialize your domain then call DgnDomains::RegisterDomain
     * @param bridgeAssetsDir   The directory that contains the assets for the bridge (and the dgnplatform). ECSchemas will be located under this directory.
    */
    virtual void _RegisterDomain(BentleyApi::BeFileNameCR bridgeAssetsDir) {} 

    // Called just before the specified element is converted (and after the applicable ConvertToDgnDbElementExtension is called).
    // return SkipElement if the specified element should not be converted.
    enum class Result {Proceed=0, SkipElement=1};
    virtual Result _PreConvertElement(DgnV8EhCR, Converter&, ResolvedModelMapping const&) {return Result::Proceed;}

    // Called just before the specified element is converted and after the applicable ConvertToDgnDbElementExtension is called.
    // Compute the class, code, and/or category that should be used for the specified element. 
    // On input, each parameter contains the default values that will be used to convert the element.
    // Modify any parameter that you want to change.
    virtual void _DetermineElementParams(DgnClassId&, DgnCode&, DgnCategoryId&, DgnV8EhCR, Converter&, ECObjectsV8::IECInstance const* primaryV8Instance, ResolvedModelMapping const&) {;}

    // Called just after the specified element is converted but before it is written to the BIM. Called after the applicable ConvertToDgnDbElementExtension is called.
    // The conversion results parameter contains the converted element.
    // This is the right place to convert user data linkages that the default converter might not know about. You can write the results as properties of the
    // converted element.
    // After this function returns, the element will be written.
    virtual void _ProcessResults(ElementConversionResults&, DgnV8EhCR, ResolvedModelMapping const&, Converter&) {}

    /* XDomain may import schemas. NB: call db.BriefcaseManager().LockSchemas() before you call db.ImportSchemas */
    virtual BentleyStatus _ImportSchema(DgnDbR) {return BSISUCCESS;} 

    // Override the BIS conversion rule that will be applied to this element. Called after the applicable ConvertToDgnDbElementExtension is called.
    virtual void _DetermineBisConversionRule(BisConversionRule&, DgnV8EhCR v8eh, DgnDbR dgndb, BisConversionTargetModelInfoCR) {;}

    virtual bool _GetBasisTransform(Bentley::Transform&, DgnV8EhCR, Converter&) {return false; /* caller will derive placement transform from geometry */ }
    virtual bool _IgnorePublicChildren() {return false;} // When true, don't create an assembly for a V8 cell with public children unless there are category changes.
    virtual bool _DisablePostInstancing() {return false;} // When true, don't try to detect identical geometry and create GeometryParts from non-instanced V8 geometry.

    //! An element's hash must capture the state of *all* of the data that will be used by the conversion logic to produce a BIM element, 
    //! including any external data that your _DetermineElementParams or _ProcessResults methods might somehow access and factor into the result.
    //! This hash value is stored in syncinfo and is used by the update logic to detect if the input data has changed. (For files such as V7, 
    //! the hash value actually serves as the source element's unique ID.) Only elements with a changed hash value are updated.
    //! Override this method in order to update the element's hash with the external data that your _DetermineElementParams or _ProcessResults methods will use.
    virtual void _ComputeHash(BentleyApi::MD5& hasher, DgnV8EhCR v8eh) {;}

    //! Register an XDomain to be used by the converter. Do not free xd after calling this!
    DGNDBSYNC_EXPORT static void Register(XDomain& xd);
    //! Un-Register an XDomain. 
    DGNDBSYNC_EXPORT static void UnRegister(XDomain& xd);
};

//=======================================================================================
// Handles conversion from DgnV8 TEXT_ELM and TEXT_NODE_ELM to DgnDb TextAnnotationElements.
// @bsiclass                                                    Jeff.Marker     08/2015
//=======================================================================================
struct ConvertV8TextToDgnDbExtension : ConvertToDgnDbElementExtension
{
    static void Register();

    BisConversionRule _DetermineBisConversionRule(DgnV8EhCR v8eh, DgnDbR dgndb, BisConversionTargetModelInfoCR) override;
    void _DetermineElementParams(DgnClassId&, DgnCode&, DgnCategoryId&, DgnV8EhCR, Converter&, ECObjectsV8::IECInstance const* primaryV8Instance, ResolvedModelMapping const&) override;
    void _ProcessResults(ElementConversionResults&, DgnV8EhCR, ResolvedModelMapping const&, Converter&) override;
    bool _GetBasisTransform(Bentley::Transform&, DgnV8EhCR, Converter&) override;
};


//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2016                                                                    
//=======================================================================================
struct ConvertV8TagToDgnDbExtension : ConvertToDgnDbElementExtension
{
    static void Register();
    virtual Result _PreConvertElement(DgnV8EhCR, Converter&, ResolvedModelMapping const&) override {return Result::SkipElement;}
};

//=======================================================================================
// A DgnV8 element handler. Exists only so it can be extended with ConverThreeMxAttachment
// @bsiclass                                                    Keith.Bentley   04/16
//=======================================================================================
struct ThreeMxElementHandler : DgnV8Api::ExtendedElementHandler
{
    enum {XATTRIBUTEID_ThreeMxAttachment=22886};
    DEFINE_T_SUPER(DgnV8Api::ExtendedElementHandler)
    DGNV8_ELEMENTHANDLER_DECLARE_MEMBERS(ThreeMxElementHandler, );
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct ConvertThreeMxAttachment : ConvertToDgnDbElementExtension
{
    static void Register();
    Result _PreConvertElement(DgnV8EhCR, Converter&, ResolvedModelMapping const&) override;
};

//=============================================================================================
// A DgnV8 element handler. Exists only so it can be extended with ConverScalableMeshAttachment
// @bsiclass                                                    Mathieu.St-Pierre 07/17
//=============================================================================================
struct ScalableMeshElementHandler : DgnV8Api::ExtendedElementHandler
{    
    enum { XATTRIBUTEID_ScalableMeshAttachment = 22899};
    DEFINE_T_SUPER(DgnV8Api::ExtendedElementHandler)
    DGNV8_ELEMENTHANDLER_DECLARE_MEMBERS(ScalableMeshElementHandler, );
};
//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2016
//=======================================================================================
struct RealityMeshAttachmentConversion
{
    static StatusInt ExtractAttachment (BentleyApi::Utf8StringR rootUrl, Transform& location, BentleyApi::Dgn::ClipVectorPtr& clipVector, ModelSpatialClassifiers& classifiers, uint32_t& activeClassifierId, DgnV8EhCR v8el, Converter& converter, ResolvedModelMapping const& v8mm, uint16_t majorXAttributeId);
    static void ForceClassifierAttachmentLoad (DgnModelRefR modelRef);
                         
};


//=======================================================================================
// @bsiclass                                                    Mathieu.St-Pierre 07/17
//=======================================================================================
struct ConvertScalableMeshAttachment : ConvertToDgnDbElementExtension
{
    static void Register();
    Result _PreConvertElement(DgnV8EhCR, Converter&, ResolvedModelMapping const&) override;
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      09/2016
//=======================================================================================
struct ConvertDetailingSymbolExtension: ConvertToDgnDbElementExtension, IFinishConversion
{
    enum class DetType{Callout, DrawingBoundary};

    static void Register();
    void Initialize(Converter&);
    BeSQLite::CachedStatementPtr GetInsertStatement(DgnDbR, DetType);
    BeSQLite::CachedStatementPtr GetSelectStatement(DgnDbR, DetType);
    void RecordCalloutDependency(Converter& converter, DgnV8EhCR v8Eh, ResolvedModelMapping const& v8mm);
    void RecordDrawingBoundaryDependency(Converter& converter, DgnV8EhCR v8Eh, DgnV8Api::IDetailingSymbol&, ResolvedModelMapping const& v8mm);
    void RelateViewAttachmentLabelToViewAttachment(Converter&, BeSQLite::Statement&);
    void RelateCalloutToDrawing(Converter&, BeSQLite::Statement&);

    DgnCategoryId GetCategoryFor(DgnV8EhCR v8Eh, Converter& converter);
    void _DetermineElementParams(DgnClassId&, DgnCode&, DgnCategoryId&, DgnV8EhCR, Converter&, ECObjectsV8::IECInstance const* primaryV8Instance, ResolvedModelMapping const&) override;
    void _ProcessResults(ElementConversionResults&, DgnV8EhCR, ResolvedModelMapping const&, Converter&) override;
    void _OnFinishConversion(Converter&) override;
};

//=======================================================================================
// Handles conversion from DgnV8 light sources (cells) to LightLocation elements.
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ConvertV8Lights : ConvertToDgnDbElementExtension
{
    static void Register();
    void _DetermineElementParams(DgnClassId&, DgnCode&, DgnCategoryId&, DgnV8EhCR, Converter&, ECObjectsV8::IECInstance const* primaryV8Instance, ResolvedModelMapping const&) override;
    void _ProcessResults(ElementConversionResults&, DgnV8EhCR, ResolvedModelMapping const&, Converter&) override;
    bool _IgnorePublicChildren() override {return true;}
    bool _DisablePostInstancing() override {return true;}
    BisConversionRule _DetermineBisConversionRule(DgnV8EhCR v8eh, DgnDbR dgndb, BisConversionTargetModelInfoCR) override;
};

//=======================================================================================
// Efficient access to a set of pre-allocated loggers.
// @bsiclass                                                    Sam.Wilson          03/16
//=======================================================================================
struct ConverterLogging
{
    enum class Namespace {General, Level, LevelMask, Model, Performance, MaxLoggers};

    DGNDBSYNC_EXPORT static NativeLogging::ILogger& GetLogger(Namespace ns);
    DGNDBSYNC_EXPORT static bool IsSeverityEnabled(Namespace ns, NativeLogging::SEVERITY);
    //Logs the current elapsed time of the passed stopwatch and stops the stopwatch
    DGNDBSYNC_EXPORT static void LogPerformance(StopWatch& stopWatch, Utf8CP description, ...);
};

//=======================================================================================
//! Helper class for using the converter as a library.
//!
//! <h2>Initialization</h2>
//! You must call Converter::Initialize before calling any other method in this class.
//! @code
//! Assume that 'this' is-a iModelBridge
//! ConverterLibrary cvt(outputBim);
//! Converter::Initialize(_GetParams().GetLibraryDir(), _GetParams().GetAssetsDir(), BeFileName(L"DgnV8"), nullptr, false, argc, argv);
//! @endcode
//!
//! <h2>Coordinate System Transformation</h2>
//! Coordinates and distances in the BIM are in meters. The BIM may also have its own Geographic Coordinate System (GCS).
//! All source V8 geometry must be transformed into the BIM's coordinate system. ConvertElement does this for you. When you call geometry conversion
//! utility functions, you must do it yourself.
//!
//! Normaly, you should call SetRootModelAndSubject up front. This prepares the units and GCS transformation 
//! from a specified V8 root model to the BIM's coordinate system. This also populates the list of files, spatial models, and non-spatial models in the source file set.
//! @code
//! cvt.SetRootModelAndSubject(rootModel);
//! @endcode
//! You can then call Converter::GetRootTransform to query the computed transform. Note that SetRootModelAndSubject
//! may actually reproject your input V8 models in order to prepare for the GCS transformation.
//!
//! <h2>File and Model Mappings</h2>
//! You must 'map' source files and models to BIM models before you can convert elements
//! or even geometry. Before mapping a model from a V8 source file, you must first map
//! the V8 file itself by calling RecordFileMapping;
//! @code
//! cvt.RecordFileMapping(v8File);
//! @endcode
//!
//! Then before converting elements or geometry from a given source model, you must map
//! the source model by calling RecordModelMapping. Note that the returned ResolvedModelMapping
//! object holds the units transform for the model.
//! @code
//! auto v8Model = v8File.GetLoadedModelByIndex(i);
//! ResolvedModelMapping modelMapping = cvt.RecordModelMapping(*v8Model, someBimModel);
//! @endcode
//!
//! <h2>Level and LineStyle Mappings</h2>
//! Before converting any elements, you must define mappings from the levels and linestyles
//! used by those elements to Categories and LineStyles in the target BIM. Call 
//! ConvertAllLineStyles, InitUncategorizedCategory, ConvertAllSpatialLevels, and/or RecordLevelMappingForModel.
//! @code
//! cvt.ConvertAllLineStyles(v8File);       // This is necessary to support both element and level conversion
//! cvt.InitUncategorizedCategory();        // This is important, in case the converter hits an element with a bad or unmapped level
//! cvt.ConvertAllSpatialLevels(v8File);    // This is how you can map all (3D) levels in one shot
//! cvt.RecordLevelMappingForModel(DGNV8_LEVEL_DEFAULT_LEVEL_ID, someBimCategory->GetDefaultSubCategoryId(), v8File); // this is how to map levels one by one
//! @endcode
//!
//! <h2>Converting Elements</h2>
//! Call ConvertElement to convert the geometry of a V8 element to a BIS format. 
//! ConvertElement also transforms the element's geometry into the BIM's coordinate system (meters and GCS).
//! Note that ConverterLibrary does @em not convert EC "business data" on the V8 element, just geometry.
//! 
//! @code
//! for (auto v8El : *v8Model->GetGraphicElementsP())
//!     {
//!     DgnV8Api::EditElementHandle v8eh(v8El);
//!     ElementConversionResults results = cvt.ConvertElement(v8eh);
//!     if (!results.m_element.IsValid())
//!         continue;       // the element's geometry could not be converted for some reason?!
//!     ... do something with results ...
//!     }
//! @endcode

//! <h2>Converting Geometry</h2>
//! You can also convert geometry as such, without having or converting a V8 element. Call one of the geometry conversion
//! utility functions, including ConvertLineStyleParams, ConvertCurveVector, ConvertMSBsplineSurface, ConvertSolidPrimitive, ConvertPolyface, ConvertTextString, ConvertSolidKernelEntity
//! Note that the geometry conversion utility functions do @em not ransforms the geometry into the BIM's coordinate system. You must do that by 
//! applying the coordinate system transform to the resulting geometry. For example:
//! @code
//! DgnV8Api::ICurvePathQuery* curveQuery = dynamic_cast<DgnV8Api::ICurvePathQuery*>(&v8eh.GetHandler());
//! if (nullptr != curveQuery)
//!     {
//!     CurveVectorPtr v8Cv;
//!     curveQuery->GetCurveVector(v8eh, v8Cv));
//!     if (!v8Cv.IsValid())
//!         return ERROR;
//!     BentleyApi::CurveVectorPtr bimCv;
//!     Converter::ConvertCurveVector(bimCv, *v8Cv, nullptr);
//!     if (!bimCv.IsValid())
//!         return ERROR;
//!     bimCv->TransformInPlace(modelMapping.GetTransform()); // ConvertCurveVector does not transform the units. You must do that explicitly.
//!     }
//! @endcode
//! 
//! <h2>Reference Attachments and Coordinate System Transformation</h2>
//! If you will be processing V8 reference attachments, then you must a) map them to BIM models and b) set up the coordinate system transforms for them.
//! As noted in the description of RecordModelMapping, the option third argument must be specified for a reference attachment. You must compute this
//! transform by calling ComputeAttachmentTransform, passing in the coordinate system transform for the immediate parent model. The most natural way
//! to do this is to recursively walk the V8 reference attachment hierarchy up front, starting with the V8 root model and using the result of Converter::GetRootTrans
//! as the root transform.
//! 
//! If you don't plan to process reference attachments -- for example, if all V8 models are top-level models and are not attached to each other --
//! then you can treat each one as a root model.
//!
// @bsiclass                                                    Sam.Wilson          02/17
//=======================================================================================
struct ConverterLibrary : RootModelConverter
{
    DEFINE_T_SUPER(RootModelConverter)

private:
    SubjectCPtr m_jobSubject;
    SubjectCPtr m_spatialParentSubject;

    SubjectCR _GetJobSubject() const override {return m_jobSubject.IsValid() ? *m_jobSubject : *GetDgnDb().Elements().GetRootSubject();}
    SubjectCR _GetSpatialParentSubject() const override {return m_spatialParentSubject.IsValid() ? *m_spatialParentSubject : T_Super::_GetSpatialParentSubject();}

public:
    //! Construct an instance of this helper class
    //! @param bim     The output BIM
    //! @param params  The converter parameters
    DGNDBSYNC_EXPORT ConverterLibrary(DgnDbR bim, RootModelSpatialParams& params);

    //! Call this to set up for change-detection. This is optional unless you plan to call ConvertAllDrawingsAndSheets.
    DGNDBSYNC_EXPORT void SetChangeDetector(bool isUpdate);

    //! Record the fact that content from the specified V8 file is being converted to the target BIM
    //! @param v8File           A V8 file that you plan to use as input.
    DGNDBSYNC_EXPORT void RecordFileMapping(DgnV8FileR v8File);

    //! Compute the units transform from the specified root model into meters, and *also* prepare the transformation of geo-located data 
    //! in the root model and its attachments into the GCS of the target BIM. This also populates the list of files, spatial models, and non-spatial models in the source file set.
    //! @param rootV8Model    The root DgnModel in the input V8
    //! @param jobSubject     The Job Subject element. Its transform property, if any, is post-multiplied onto the GCS root transform.
    //! @note This function will modify the data in rootV8Model and its attachments if reprojection is necessary.
    //! @note You must call this once up front, before calling ConvertElement.
    //! @note If you want to set up a GCS on the output BIM (if it does not have one), do that before calling this function. @see Dgn::DgnGCS
    //! @note If you later call RecordModelMapping to enroll a model that is attached to this root model, you must call #ComputeAttachmentTransform
    //! in order to compute the units transform for the attached model and then pass that as the optional third argument to RecordModelMapping.
    //! @see GetRootTrans, GetSpatialModelsInAttachmentOrder, GetNonSpatialModelsInModelIndexOrder
    DGNDBSYNC_EXPORT void SetRootModelAndSubject(DgnV8ModelR rootV8Model, SubjectCR jobSubject);

    //! Get the list of all spatial models discovered by SetRootModelAndSubject
    bvector<DgnV8ModelP> const& GetSpatialModelsInAttachmentOrder() const {return m_spatialModelsInAttachmentOrder;}

    //! Get the list of all non-spatial models discovered by SetRootModelAndSubject
    bvector<DgnV8ModelP> const& GetNonSpatialModelsInModelIndexOrder() const {return m_nonSpatialModelsInModelIndexOrder;}

    //! Record a V8->BIM model mapping
    //! @param sourceV8Model    A DgnModel in the input V8
    //! @param targetBimModel   The model in the BIM where you plan to put elements that you read from \a sourceV8Model 
    //! @param transform        Optional. The transform that you plan to apply to elements from this model as you convert them. This defaults to the 
    //!                         transform necessary to convert the model's storage units to meters. 
    //!                         Supply a different transform if the source V8 model is attached more than once with different transforms.
    //!                        @see ComputeUnitsScaleTransform, ComputeAttachmentTransform
    //! @return The record of the mapping, which includes the units transform from source to BIM
    //! @note If you called SetRootModelAndSubject up front, then you must supply a transform when recording mappings for all 
    //! attached models. Call #ComputeAttachmentTransform to get this transform.
    //! @see FindModelForDgnV8Model and FindFirstModelMappedTo
    DGNDBSYNC_EXPORT ResolvedModelMapping RecordModelMapping(DgnV8ModelR sourceV8Model, DgnModelR targetBimModel, BentleyApi::TransformCP transform = nullptr);

    //! Record a V8 Level -> BIM DgnSubCategory mapping for elements in a particular model.
    //! @param sourceV8LevelId  A V8 level to be remapped. The level must be defined in the DgnFile that contains \a sourceV8Model.
    //! @param taretBimCategory The SubCategory to which \a sourceV8LevelId should be remapped
    //! @param sourceV8Model    Only the elements in this V8 model that use this level should be remapped to the specified SubCategory
    //! @note Use this function instead of the like-named function that takes a DgnV8FileR in the case where you have a reference attachment,
    //! and that attachment has overridden the definition of some of the levels from the reference file.
    DGNDBSYNC_EXPORT void RecordLevelMappingForModel(DgnV8Api::LevelId sourceV8LevelId, DgnSubCategoryId targetBimSubCategory, DgnV8ModelRefR sourceV8Model);

    //! Record a V8 Level -> BIM DgnSubCategory mapping for elements in a particular file.
    //! @param sourceV8LevelId  A V8 level to be remapped. The level must be defined in \a sourceV8File
    //! @param taretBimCategory The SubCategory to which \a sourceV8LevelId should be remapped
    //! @param sourceV8File     The elements in this V8 file that use this level should be remapped to the specified SubCategory
    DGNDBSYNC_EXPORT void RecordLevelMappingForModel(DgnV8Api::LevelId sourceV8LevelId, DgnSubCategoryId targetBimSubCategory, DgnV8FileR sourceV8File);

    //! Convert the geometry of the specified element
    //! @param[in] v8Element    The element to convert
    //! @param[in] modelMapping Optional. If not null, specifies the V8 attachment that contains \a v8Element. This only has to be supplied if
    //!             a model is attached more than once with different transforms.
    //! @return the BIM version of the element. 
    //! @see ConvertLineStyleParams, ConvertCurveVector, ConvertMSBsplineSurface, ConvertSolidPrimitive, ConvertPolyface, ConvertTextString, ConvertSolidKernelEntity
    DGNDBSYNC_EXPORT ElementConversionResults ConvertElement(DgnV8EhCR v8Element, ResolvedModelMapping const* modelMapping = nullptr);

    //! Return the Subject that will be the "root" of the job hierarchy.
    SubjectCR GetJobSubject() const {return _GetJobSubject();}
    //! Set the Subject that will be the "root" of the job hierarchy.
    void SetJobSubject(SubjectCR subject) {m_jobSubject = &subject;}

    //! Return the Subject that will be the "root" for spatial models
    SubjectCR GetSpatialParentSubject() const {return _GetSpatialParentSubject();}
    //! Set the Subject that will be the "root" for spatial models
    void SetSpatialParentSubject(SubjectCR subject) {m_spatialParentSubject = &subject;}

    //! Converts all sheets and drawings found in all files that have been mapped, plus all files passed to RootModelSpatialParams::AddDrawingOrSheetFile 
    //! @note You must call SetJobSubject first to set the job subject.
    //! @return non-zero if the converter is not prepared to import sheets and drawings or if import fails
    //! @see SetJobSubject, SetChangeDetector, RootModelSpatialParams::AddDrawingOrSheetFile
    DGNDBSYNC_EXPORT BentleyStatus ConvertAllDrawingsAndSheets();

    //! Attempts to identify the root model. The underlying logic prefers the root model of the active view group, falls back on other view groups, 
    //! and finally considers the default model. It returns an invalid ModelId if this search yields no 3-D model. That can happen if the file 
    //! contains only sheets and drawings as a root.
    DgnV8Api::ModelId GetRootModelId() { return _GetRootModelId(); }
};

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct SchemaRemapper : ECN::IECSchemaRemapper
    {
    private:
        typedef bmap<Utf8String, Utf8String> T_propertyNameMappings;
        typedef bmap<Utf8String, T_propertyNameMappings> T_ClassPropertiesMap;
        Converter& m_converter;
        mutable ECN::ECSchemaPtr m_convSchema;
        bool m_remapAsAspect;
        mutable T_ClassPropertiesMap m_renamedClassProperties;

        virtual bool _ResolvePropertyName(Utf8StringR serializedPropertyName, ECN::ECClassCR ecClass) const override;
        virtual bool _ResolveClassName(Utf8StringR serializedClassName, ECN::ECSchemaCR ecSchema) const override;

    public:
        explicit SchemaRemapper(Converter& converter) : m_converter(converter), m_remapAsAspect(false) {}
        ~SchemaRemapper() {}
        void SetRemapAsAspect(bool remapAsAspect) { m_remapAsAspect = remapAsAspect; }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct ElementConverter
    {
    private:
        mutable bmap<Utf8CP, ECN::ECInstanceReadContextPtr> m_instanceReadContextCache;

        ECN::ECInstanceReadContextPtr LocateInstanceReadContext(ECN::ECSchemaCR schema) const;

    protected:
        ECN::IECInstancePtr Transform(ECObjectsV8::IECInstance const& v8Instance, ECN::ECClassCR dgnDbClass, bool transformAsAspect = false) const;
        struct UnitResolver : ECN::ECInstanceReadContext::IUnitResolver
            {
            private:
                mutable ECN::ECSchemaPtr m_convSchema;

                virtual Utf8String _ResolveUnitName(ECN::ECPropertyCR ecProperty) const override;
            };

        Converter& m_converter;
        mutable SchemaRemapper m_schemaRemapper;
        mutable UnitResolver m_unitResolver;

        ECN::ECClassCP GetDgnDbClass(ECObjectsV8::IECInstance const& v8Instance, BentleyApi::Utf8CP aspectClassSuffix) const;
        static Utf8String ToInstanceLabel(ECObjectsV8::IECInstance const& v8Instance);

    public:
        explicit ElementConverter(Converter& converter) : m_converter(converter), m_schemaRemapper(converter) {}
        BentleyStatus ConvertToElementItem(ElementConversionResults&, ECObjectsV8::IECInstance const* v8PrimaryInstance, BisConversionRule const* primaryInstanceConversionRule) const;

    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2015
//+===============+===============+===============+===============+===============+======
struct ElementAspectConverter : ElementConverter
    {
    private:
        BentleyStatus ConvertToAspect(ElementConversionResults&, ECObjectsV8::IECInstance const& v8Instance, BentleyApi::Utf8CP aspectClassSuffix) const;

    public:
        explicit ElementAspectConverter(Converter& converter) : ElementConverter(converter) {}
        BentleyStatus ConvertToAspects(ElementConversionResults&, std::vector<std::pair<ECObjectsV8::IECInstancePtr, BisConversionRule>> const& secondaryInstances) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2015
//+===============+===============+===============+===============+===============+======
struct ECInstanceInfo : NonCopyableClass
    {
private:
    ECInstanceInfo ();
    ~ECInstanceInfo ();

public:
    static BeSQLite::EC::ECInstanceKey Find (bool& isElement, DgnDbR db, SyncInfo::V8FileSyncInfoId fileId, V8ECInstanceKey const& v8Key);
    static BentleyStatus Insert (DgnDbR db, SyncInfo::V8FileSyncInfoId fileId, V8ECInstanceKey const& v8Key, BeSQLite::EC::ECInstanceKey const& key, bool isElement);
    static BentleyStatus CreateTable (DgnDbR db);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2015
//+===============+===============+===============+===============+===============+======
struct V8NamedGroupInfo : NonCopyableClass
    {
private:
    static bmap<SyncInfo::V8FileSyncInfoId, bset<DgnV8Api::ElementId>> s_namedGroupsWithOwnershipHint;

    V8NamedGroupInfo();
    ~V8NamedGroupInfo();

public:
    static void AddNamedGroupWithOwnershipHint(DgnV8EhCR);
    static bool TryGetNamedGroupsWithOwnershipHint(bset<DgnV8Api::ElementId> const*&, SyncInfo::V8FileSyncInfoId);
    static void Reset();
    };

END_DGNDBSYNC_DGNV8_NAMESPACE
