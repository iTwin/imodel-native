/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnHandlers/DgnECTypes.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnECTypes.h>
#include    "DgnECTypeAdapters.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM

DGNPLATFORM_TYPEDEFS (DgnECTypeRegistry);

typedef IDgnECTypeAdapterPtr (* DgnECTypeAdapter_Create)(void);
struct StandardTypeInfo
    {
    Utf8CP                                 name;
    DgnECExtendedType::StandardType         type;
    DgnECTypeAdapter_Create               Create;
    };

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct CompareNames
    {
    bool operator() (Utf8CP a, Utf8CP b) const { return 0 < strcmp(a, b); }
    };


/*---------------------------------------------------------------------------------**//**
* Manages DgnECExtendedTypes and associated DgnECTypeAdapters.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnECTypeRegistry : DgnHost::IHostObject
    {
    enum BasicType
        {
        // Primitives
        BasicType_Boolean,
        BasicType_String,
        BasicType_Integer,
        BasicType_Long,
        BasicType_Binary,
        BasicType_DateTime,
        BasicType_Point2D,
        BasicType_Point3D,
        BasicType_Double,

        BasicType_StructArray,          // member of a struct array
        BasicType_BooleanDisplay,       // has BooleanDisplay custom attribute
        BasicType_StandardValues,       // has StandardValues custom attribute
        BasicType_FormatString,         // has FormatString custom attribute
        BasicType_ECUnits,              // has UnitSpecification custom attribute
        BasicType_MissingExtendType,    // has ExtendType but no specialized TypeAdapter could be located, so it has limited functionality
        BasicType_Dummy,                // embedded struct or unextended array - properties for which SetValue() and ToString() have no meaning

        BasicType_Max
        };
private:
    typedef bvector<DgnECExtendedTypeP>                         ExtendedTypesList;
    typedef bmap<Utf8CP, uint16_t, CompareNames>   ExtendedTypeIdsByName;   

    ExtendedTypesList                   m_extendedTypes;
    ExtendedTypeIdsByName               m_extendedTypeIdsByName;
    IDgnECTypeAdapterPtr                m_basicTypeAdapters[BasicType_Max];
    IDgnECTypeAdapterPtr                m_stringTypeAdapter;                    // used frequently - create one and reuse for multiple properties

    DgnECTypeRegistry();
    ~DgnECTypeRegistry();

    static DgnHost::Key&                GetHostKey();
    DgnECExtendedTypeR                  GetExtendedTypeById (uint16_t id);
public:
    static DgnECTypeRegistryR           GetRegistry();

    DgnECExtendedTypeR                  GetOrCreateExtendedType (Utf8CP name);
    DgnECExtendedTypeR                  GetStandardExtendedType (DgnECExtendedType::StandardType type);
    IDgnECTypeAdapterPtr                CreateTypeAdapter (DgnECExtendedTypeCR extendedType);
    IDgnECTypeAdapterR                  GetBasicTypeAdapter (BasicType type);

    static BasicType                    BasicTypeForPrimitive (PrimitiveType primitive);
    void                                BackDoor_UnregisterExtendedType (DgnECExtendedTypeR extendType);
    };

/*---------------------------------------------------------------------------------**//**
* Some extended types exist solely so that specialized type editors can be registered,
* without defining any special type adapter logic/formatting. For these we use the type
* adapter associated with the underlying primitive type.
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
template<DgnECTypeRegistry::BasicType T> static IDgnECTypeAdapterPtr getBasicTypeAdapter()
    {
    return &DgnECTypeRegistry::GetRegistry().GetBasicTypeAdapter (T);
    }

// Names and default type converters for standard types
static StandardTypeInfo s_standardTypeInfos[] =
    {
    {   "ElementType",           DgnECExtendedType::ElementType,               NULL }, // Extended type from 8.11.9, but requires no special type conversion
    {   "Level",                 DgnECExtendedType::Level,                     NULL },
    {   "Color",                 DgnECExtendedType::Color,                     NULL },
    {   "Weight",                DgnECExtendedType::Weight,                    NULL },
    {   "Style",                 DgnECExtendedType::Style,                     NULL },
    {   "ElementClass",          DgnECExtendedType::ElementClass,              NULL },
    {   "Coordinates",           DgnECExtendedType::Coordinates,               &CoordinatesTypeAdapter::Create },
    {   "Distance",              DgnECExtendedType::Distance,                  &DistanceTypeAdapter::Create },
    {   "Area",                  DgnECExtendedType::Area,                      &AreaTypeAdapter::Create },
    {   "Volume",                DgnECExtendedType::Volume,                    &VolumeTypeAdapter::Create },
    {   "Angle",                 DgnECExtendedType::Angle,                     &AngleTypeAdapter::Create },
    {   "EditableArray",         DgnECExtendedType::EditableArray,             NULL }, // this type is purely a UI concept
    {   "ElementTemplate",       DgnECExtendedType::ElementTemplate,           NULL },
    {   "Scale",                 DgnECExtendedType::Scale,                     NULL },
    {   "TextStyle",             DgnECExtendedType::TextStyle,                 NULL },
    {   "RenderingMaterial",     DgnECExtendedType::RenderingMaterial,         NULL },
    {   "DirectionAngle",        DgnECExtendedType::DirectionAngle,            &DirectionAngleTypeAdapter::Create },
    {   "UnitlessPoint",         DgnECExtendedType::UnitlessPoint,             &UnitlessPointTypeAdapter::Create },
    {   "NamedScale",            DgnECExtendedType::NamedScale,                NULL },
    {   "ProjectionNamedGroup",  DgnECExtendedType::ProjectionNamedGroup,      NULL },
    {   "ProjectionMapping",     DgnECExtendedType::ProjectionMapping_DEPRECATED, NULL }, // Changed property to use StandardValues instead, no custom type converter needed...
    {   "Transparency",          DgnECExtendedType::Transparency,              NULL },
    {   "DetailSymbolTemplate",  DgnECExtendedType::DetailSymbolTemplate,      NULL },
    {   "FileSize",              DgnECExtendedType::FileSize,                  &FileSizeTypeAdapter::Create },
    {   "FileSizeLong",          DgnECExtendedType::FileSizeLong,              &FileSizeLongTypeAdapter::Create },
    {   "DisplayPriority",       DgnECExtendedType::DisplayPriority,           NULL },

    {   "FontName",              DgnECExtendedType::FontName,                  NULL },
    {   "ShxBigFontName",           DgnECExtendedType::ShxBigFontName,               NULL },
    {   "ElementID",             DgnECExtendedType::ElementID,                 NULL },
    {   "HatchCell",             DgnECExtendedType::HatchCell,                 NULL },
    {   "HatchName",             DgnECExtendedType::HatchName,                 NULL },
    {   "AnnotationLinkMemberType", DgnECExtendedType::AnnotationLinkMemberType,   NULL },               // managed converter just treats as string
    {   "AnnotationLinkArray",   DgnECExtendedType::AnnotationLinkArray,       NULL },
    {   "DwgCustomObjectInfoType",   DgnECExtendedType::DwgCustomObjectInfo,   NULL },
    {   "NamedGroupStruct",      DgnECExtendedType::NamedGroupStruct,          NULL },
    {   "NamedGroupArray",       DgnECExtendedType::NamedGroupArray,           NULL },
    {   "KnotDetail",            DgnECExtendedType::KnotDetail,                NULL },

    // ###TODO: the following are disabled or unimplemented in Microstation Vancouver
    {   "Discipline",              DgnECExtendedType::Discipline,              NULL },
    {   "ViewPurpose",             DgnECExtendedType::ViewPurpose,             NULL },

    {   "ViewType",                DgnECExtendedType::ViewType,                NULL },
    {   "Markers",                 DgnECExtendedType::Markers,                 NULL },
    {   "BaseDisplayStyle",        DgnECExtendedType::BaseDisplayStyle,        NULL },

    {   "PropertyManagerFileName", DgnECExtendedType::PropertyManagerFileName, NULL },
    {   "PropertyManagerFileSize", DgnECExtendedType::PropertyManagerFileSize, NULL },

    // note any special conversion/validation/standardvalues logic must reside in the type editor for DgnLinks properties,
    // because (1) it is valid to create a link to a non-existent file/model/etc, and (2) validation/standard values
    // require more context than can be provided by managed callers.
    {   "DgnFileBrowser",          DgnECExtendedType::DgnFileBrowser,          &StringTypeAdapter::Create },                    // type editor only
    {   "DgnLinks_ModelName",      DgnECExtendedType::DgnLinks_ModelName,      &StringTypeAdapter::Create },
    {   "ConfigVarFilter",         DgnECExtendedType::ConfigVarFilter,         &StringTypeAdapter::Create },                    // type editor only
    {   "DgnFolderBrowser",        DgnECExtendedType::DgnFolderBrowser,        &StringTypeAdapter::Create },                    // type editor only
    {   "LogicalName",             DgnECExtendedType::LogicalName,             &StringTypeAdapter::Create },
    {   "NamedView",               DgnECExtendedType::NamedView,               &StringTypeAdapter::Create },
    {   "Drawings",                DgnECExtendedType::Drawings,                &StringTypeAdapter::Create },                    // disabled in Vancouver
    {   "AddressBrowser",          DgnECExtendedType::AddressBrowser,          &StringTypeAdapter::Create },                    // type editor only
    {   "WordHeading",             DgnECExtendedType::WordHeading,             &StringTypeAdapter::Create },                    // type editor only
    {   "WordBookmark",            DgnECExtendedType::WordBookmark,            &StringTypeAdapter::Create },                    // type editor only
    {   "PdfBookmark",             DgnECExtendedType::PdfBookmark,             &StringTypeAdapter::Create },                    // type editor only

    {   "NamedGroupType",          DgnECExtendedType::NamedGroupType,          NULL },
    {   "XyzRotations",            DgnECExtendedType::XyzRotations,            &XyzRotationsTypeAdapter::Create },
    {   "UnitDefinition",          DgnECExtendedType::UnitDefinition,          &UnitDefinitionTypeAdapter::Create },
    {   "DateTimeFormat",          DgnECExtendedType::DateTimeFormat,          &DateTimeFormatTypeAdapter::Create },

    {   "VariableAngle",           DgnECExtendedType::VariableAngle,           &VariableAngleTypeAdapter::Create },
    {   "VariableBoolean",         DgnECExtendedType::VariableBoolean,         &VariableBooleanTypeAdapter::Create },
    {   "VariableColor",           DgnECExtendedType::VariableColor,           NULL },
    {   "VariableDistance",        DgnECExtendedType::VariableDistance,        &VariableDistanceTypeAdapter::Create },
    {   "VariableFontName",        DgnECExtendedType::VariableFontName,        NULL },
    {   "VariableDouble",          DgnECExtendedType::VariableDouble,          &VariableDoubleTypeAdapter::Create },
    {   "VariableStandardValues",  DgnECExtendedType::VariableStandardValues,  &VariableEnumTypeAdapter::Create },
    {   "VariableTextStyle",       DgnECExtendedType::VariableTextStyle,       NULL },

    {   "DisplayStyle",            DgnECExtendedType::DisplayStyle,            NULL },
    {   "DisplayStyles",           DgnECExtendedType::DisplayStyles,           NULL },
    {   "DisplayStyleType",        DgnECExtendedType::DisplayStyleType,        NULL },
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECExtendedType::IsStandardDistanceType() const
    {
    switch (m_id)
        {
    case Distance:
    case Coordinates:
    case Area:
    case Volume:
    case VariableDistance:
        return true;
    default:
        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECExtendedType::IsStandardDirectionalType() const
    {
    switch (m_id)
        {
    case Angle:
    case DirectionAngle:
    case XyzRotations:
    case VariableAngle:
        return true;
    default:
        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Non-functional type adapter, e.g. for embedded structs, arrays without ExtendType.
* Providing this allows us to assert that every ECProperty has a TypeAdapter associated
* with it.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DummyTypeAdapter : IDgnECTypeAdapter
    {
protected:
    DummyTypeAdapter () { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP) const override { return false; }
    virtual bool            _ConvertFromString(ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _GetPlaceholderValue(ECN::ECValueR, IDgnECTypeAdapterContextCR) const override { return false; }
public:
    static IDgnECTypeAdapterPtr Create () { return new DummyTypeAdapter(); }
    };

///////////////////////////////////////
//  Internal registry
//////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECTypeRegistry::BasicType DgnECTypeRegistry::BasicTypeForPrimitive (PrimitiveType primitive)
    {
    switch (primitive)
        {
    case PRIMITIVETYPE_Boolean:     return BasicType_Boolean;
    case PRIMITIVETYPE_Integer:     return BasicType_Integer;
    case PRIMITIVETYPE_Long:        return BasicType_Long;
    case PRIMITIVETYPE_Double:      return BasicType_Double;
    case PRIMITIVETYPE_String:      return BasicType_String;
    case PRIMITIVETYPE_Point2D:     return BasicType_Point2D;
    case PRIMITIVETYPE_Point3D:     return BasicType_Point3D;
    case PRIMITIVETYPE_DateTime:    return BasicType_DateTime;
    case PRIMITIVETYPE_Binary:      return BasicType_Binary;
    default:
        BeAssert (false);
        return BasicType_Dummy;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECTypeRegistry::BackDoor_UnregisterExtendedType (DgnECExtendedTypeR extendType)
    {
    uint16_t id = extendType.GetId();
    id -= DgnECExtendedType::STANDARD_First;
    m_extendedTypes[id] = NULL;
    m_extendedTypeIdsByName.erase (extendType.GetName());
    delete &extendType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECTypeRegistry::DgnECTypeRegistry()
    {
    // Initialize standard types. Note we do not create type converters until requested.
    for (int i = 0; i < _countof (s_standardTypeInfos); i++)
        {
        StandardTypeInfo* info = s_standardTypeInfos + i;
        m_extendedTypes.push_back (new DgnECExtendedType (info->name, info->type));
        m_extendedTypeIdsByName[info->name] = info->type;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECTypeRegistry::~DgnECTypeRegistry()
    {
    FOR_EACH (DgnECExtendedTypeP extendType, m_extendedTypes)
        delete extendType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHost::Key& DgnECTypeRegistry::GetHostKey()
    {
    static DgnHost::Key key;
    return key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECTypeRegistryR  DgnECTypeRegistry::GetRegistry()
    {
    DgnHost::Key& key = GetHostKey();
    DgnHost::IHostObject* hostObj = T_HOST.GetHostObject (key);
    if (NULL == hostObj)
        {
        DgnECTypeRegistry* registry = new DgnECTypeRegistry();
        T_HOST.SetHostObject (key, registry);
        return *registry;
        }

    return *(static_cast<DgnECTypeRegistry*> (hostObj));
    }

/*---------------------------------------------------------------------------------**//**
* Caller will take ownership of the TypeAdapter*
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr DgnECTypeRegistry::CreateTypeAdapter (DgnECExtendedTypeCR extendedType)
    {
    if (extendedType.IsStandardType())
        {
        DgnECExtendedType::StandardType type = extendedType.GetStandardType();
        int index = type - DgnECExtendedType::STANDARD_First;
        BeAssert (index < _countof (s_standardTypeInfos));

        StandardTypeInfo* info = s_standardTypeInfos + index;
        if (NULL != info->Create)
            {
            // avoid creating a bunch of identical string type adapters - just create one, cache and reuse
            if (&StringTypeAdapter::Create == info->Create)
                {
                if (m_stringTypeAdapter.IsNull())
                    m_stringTypeAdapter = StringTypeAdapter::Create();

                return m_stringTypeAdapter.get();
                }
            else
                return info->Create().get();
            }
        else
            return NULL;
        }

    // Adapters for non-standard types need to be registered via DgnECExtendedType::SetTypeAdapter().
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterR DgnECTypeRegistry::GetBasicTypeAdapter (BasicType type)
    {
    if (m_basicTypeAdapters[type].IsNull())
        {
        IDgnECTypeAdapterPtr adapter;
        switch (type)
            {
        case BasicType_String:              adapter = StringTypeAdapter::Create(); break;
        case BasicType_Boolean:             adapter = BooleanTypeAdapter::Create(); break;
        case BasicType_Integer:             adapter = IntegerTypeAdapter::Create(); break;
        case BasicType_Long:                adapter = LongTypeAdapter::Create(); break;
        case BasicType_Double:              adapter = DoubleTypeAdapter::Create(); break;
        case BasicType_Binary:              adapter = BinaryTypeAdapter::Create(); break;
        case BasicType_DateTime:            adapter = DateTimeTypeAdapter::Create(); break;
        case BasicType_Point2D:             adapter = Point2DTypeAdapter::Create(); break;
        case BasicType_Point3D:             adapter = UnitlessPointTypeAdapter::Create(); break;
        case BasicType_FormatString:        adapter = FormatStringTypeAdapter::Create(); break;
        case BasicType_StructArray:         adapter = StructTypeAdapter::Create(); break;
        case BasicType_MissingExtendType:   adapter = MissingExtendTypeAdapter::Create(); break;
        case BasicType_BooleanDisplay:      adapter = BooleanDisplayTypeAdapter::Create(); break;
        case BasicType_StandardValues:      adapter = StandardValuesTypeAdapter::Create(); break;
        case BasicType_ECUnits:             adapter = ECUnitsTypeAdapter::Create(); break;
        case BasicType_Dummy:               adapter = DummyTypeAdapter::Create(); break;
        default:
            BeAssert (false);
            }

        m_basicTypeAdapters[type] = adapter;
        }

    return *m_basicTypeAdapters[type];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECExtendedTypeR DgnECTypeRegistry::GetOrCreateExtendedType (Utf8CP name)
    {
    // Note if name refers to a standard type (with a StandardType id) it will already exist in m_extendedTypeIdsByName
    ExtendedTypeIdsByName::iterator foundIter = m_extendedTypeIdsByName.find (name);
    if (foundIter != m_extendedTypeIdsByName.end())
        return GetExtendedTypeById (foundIter->second);

    uint16_t newId = (uint16_t)m_extendedTypes.size() + DgnECExtendedType::STANDARD_First;
    DgnECExtendedType* extendedType = new DgnECExtendedType (name, newId);
    name = extendedType->GetName();
    m_extendedTypes.push_back (extendedType);
    m_extendedTypeIdsByName[name] = newId;
    return GetExtendedTypeById (newId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECExtendedTypeR DgnECTypeRegistry::GetStandardExtendedType (DgnECExtendedType::StandardType type)
    {
    return GetExtendedTypeById (type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECExtendedTypeR DgnECTypeRegistry::GetExtendedTypeById (uint16_t id)
    {
    id -= DgnECExtendedType::STANDARD_First;
    BeAssert (id < (uint16_t)m_extendedTypes.size() && NULL != m_extendedTypes[id]);
    return *m_extendedTypes[id];
    }

///////////////////////////////////////
//  DgnECExtendedType
//////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECExtendedType::BackDoor_Unregister()
    {
    DgnECTypeRegistry::GetRegistry().BackDoor_UnregisterExtendedType (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnECExtendedTypeP getExtendedTypeFromAttribute (IECInstanceCR customAttr)
    {
    ECValue v;
    if (ECObjectsStatus::Success == customAttr.GetValue (v, "Standard") && !v.IsNull())
        {
        int32_t standardId = v.GetInteger();
        if (standardId >= DgnECExtendedType::STANDARD_First && standardId <= DgnECExtendedType::STANDARD_Last)
            return &DgnECTypeRegistry::GetRegistry().GetStandardExtendedType ((DgnECExtendedType::StandardType)standardId);
        else
            {
            // For whatever reason, DgnECExtendedType::NotStandard is a legitimate enumeration value in 8.11.9!!
            BeAssert (static_cast <DgnECExtendedType::StandardType> (standardId) == DgnECExtendedType::NotStandard && "Custom attribute for extended type has invalid 'Standard' type");
            return NULL;
            }
        }
    else if (ECObjectsStatus::Success == customAttr.GetValue (v, "Name") && !v.IsNull())
        return &DgnECTypeRegistry::GetRegistry().GetOrCreateExtendedType (v.GetUtf8CP());
    else
        {
        BeAssert (false && "Invalid custom attribute for extended type");
        return NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterP DgnECExtendedType::GetTypeAdapter() const
    {
    if (m_typeAdapter.IsNull())
        {
        // ###TODO: we may want to avoid calling this repeatedly if it returns NULL the first time
        m_typeAdapter = DgnECTypeRegistry::GetRegistry().CreateTypeAdapter (*this);
        }

    return m_typeAdapter.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnECExtendedType::SetTypeAdapter (IDgnECTypeAdapterR adapter)
    {
    // Note: we may want to prevent external code from overriding the standard adapters. Or not.
    m_typeAdapter = &adapter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECExtendedTypeP DgnECExtendedType::GetForProperty (ECPropertyCR ecProperty)
    {
    IECInstancePtr customAttr = ecProperty.GetCustomAttribute ("ExtendType");
    return customAttr.IsValid() ? getExtendedTypeFromAttribute (*customAttr) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECExtendedTypeP DgnECExtendedType::GetForArrayProperty (ECN::ArrayECPropertyCR ecProperty)
    {
    IECInstancePtr customAttr = ecProperty.GetCustomAttribute ("MemberExtendedType");
    return customAttr.IsValid() ? getExtendedTypeFromAttribute (*customAttr) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECExtendedTypeR DgnECExtendedType::GetByName (Utf8CP name)
    {
    return DgnECTypeRegistry::GetRegistry().GetOrCreateExtendedType (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECExtendedTypeR DgnECExtendedType::GetByStandardId (StandardType standardType)
    {
    return DgnECTypeRegistry::GetRegistry().GetStandardExtendedType (standardType);
    }

///////////////////////////////////////
//  IDgnECTypeAdapter
//  NOTE: Overridden methods from IECTypeAdapter which take an IECTypeAdapterContext always assume the context is an IDgnECTypeAdapterContext and casts accordingly.
//        We have no current implementations or use cases for type adapters or context which don't derive from those defined in DgnPlatform.
//        However the type adapters are *used* in some ECObjects code (primary ECExpressions) so the base interfaces are defined without reference to dgn concepts
///////////////////////////////////////

bool IDgnECTypeAdapter::Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return _Validate (v, context);
    }
bool IDgnECTypeAdapter::_ConvertToString(Utf8StringR valueAsString, ECValueCR v, IECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    return _ConvertToString (valueAsString, v, static_cast<IDgnECTypeAdapterContextCR>(context), formatter);
    }
bool IDgnECTypeAdapter::_ConvertFromString(ECValueR v, Utf8CP stringValue, IECTypeAdapterContextCR context) const
    {
    return _ConvertFromString (v, stringValue, static_cast<IDgnECTypeAdapterContextCR>(context));
    }
bool IDgnECTypeAdapter::GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const
    {
    return HasStandardValues() ? _GetStandardValues (values, context) : false;
    }
bool IDgnECTypeAdapter::_ConvertToExpressionType (ECValueR v, IECTypeAdapterContextCR context) const
    {
    return _ConvertToExpressionType (v, static_cast<IDgnECTypeAdapterContextCR>(context));
    }
bool IDgnECTypeAdapter::_ConvertFromExpressionType (ECValueR v, IECTypeAdapterContextCR context) const
    {
    return _ConvertFromExpressionType (v, static_cast<IDgnECTypeAdapterContextCR>(context));
    }
bool IDgnECTypeAdapter::_GetPlaceholderValue(ECValueR v, IECTypeAdapterContextCR context) const
    {
    return _GetPlaceholderValue(v, static_cast<IDgnECTypeAdapterContextCR>(context));
    }
bool IDgnECTypeAdapter::_ConvertToDisplayType (ECValueR v, IECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    return _ConvertToDisplayType (v, static_cast<IDgnECTypeAdapterContextCR>(context), opts);
    }
bool IDgnECTypeAdapter::_GetDisplayType(PrimitiveType& type) const
    {
    type = PRIMITIVETYPE_String;
    return true;
    }
bool IDgnECTypeAdapter::_IsOrdinalType () const
    {
    PrimitiveType type;
    if (!GetDisplayType (type))
        return false;

    return type == PRIMITIVETYPE_DateTime ||
        type == PRIMITIVETYPE_Double   ||
        type == PRIMITIVETYPE_Integer  ||
        type == PRIMITIVETYPE_Long;
    }
bool IDgnECTypeAdapter::_CanConvertFromString (IECTypeAdapterContextCR context) const
    {
    return _CanConvertFromString (static_cast<IDgnECTypeAdapterContextCR>(context));
    }
bool IDgnECTypeAdapter::_CanConvertToString (IECTypeAdapterContextCR context) const
    {
    return _CanConvertToString (static_cast<IDgnECTypeAdapterContextCR>(context));
    }
bool IDgnECTypeAdapter::_GetUnits (UnitSpecR units, IECTypeAdapterContextCR context) const
    {
    return _GetUnits (units, static_cast<IDgnECTypeAdapterContextCR>(context));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool IDgnECTypeAdapter::_ConvertToDisplayType (ECValueR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    PrimitiveType type;
    Utf8String str;
    if (!GetDisplayType (type))
        return false;
    else if (type != PRIMITIVETYPE_String)
        {
        BeAssert (false && "If you override GetDisplayType() you must override default implementation of ConvertToDisplayType()");
        return false;
        }
    else if (!ConvertToString (str, v, context, opts))
        return false;
    
    v.SetUtf8CP(str.c_str());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterR IDgnECTypeAdapter::GetForProperty (ECPropertyCR ecProperty)
    {
    if (NULL != ecProperty.GetCachedTypeAdapter())
        return *((IDgnECTypeAdapterP)ecProperty.GetCachedTypeAdapter());

    DgnECTypeRegistryR reg = DgnECTypeRegistry::GetRegistry();
    DgnECTypeRegistry::BasicType basicType = DgnECTypeRegistry::BasicType_Max;

    // extended type?
    DgnECExtendedTypeP extendType = DgnECExtendedType::GetForProperty (ecProperty);
    if (NULL != extendType)
        {
        IDgnECTypeAdapterP adapter = extendType->GetTypeAdapter();
        if (NULL != adapter)
            {
            ecProperty.SetCachedTypeAdapter (adapter);
            return *adapter;
            }
        }

    bool isPrimitive = ecProperty.GetIsPrimitive() || (ecProperty.GetIsNavigation() && !ecProperty.GetAsNavigationProperty()->IsMultiple());

    if (!isPrimitive)
        basicType = DgnECTypeRegistry::BasicType_Dummy;
    else
        {
        PrimitiveType primitiveType = ecProperty.GetIsPrimitive() ? ecProperty.GetAsPrimitiveProperty()->GetType() : ecProperty.GetAsNavigationProperty()->GetType();

        // Check for special custom attributes
        if (ecProperty.IsDefined("Format"))
            basicType = DgnECTypeRegistry::BasicType_FormatString;
        else if (PRIMITIVETYPE_Boolean == primitiveType && ecProperty.IsDefined("BooleanDisplay"))
            basicType = DgnECTypeRegistry::BasicType_BooleanDisplay;
        else if (PRIMITIVETYPE_Integer == primitiveType && ecProperty.IsDefined("StandardValues"))
            basicType = DgnECTypeRegistry::BasicType_StandardValues;
        else if (ecProperty.IsDefined("UnitSpecificationAttr"))
            basicType = DgnECTypeRegistry::BasicType_ECUnits;
        else
            {
            // unadorned primitive type
            basicType = DgnECTypeRegistry::BasicTypeForPrimitive(primitiveType);
            }
        }

    BeAssert (DgnECTypeRegistry::BasicType_Max != basicType);
    IDgnECTypeAdapterR basicAdapter = reg.GetBasicTypeAdapter (basicType);
    if (DgnECTypeRegistry::BasicType_MissingExtendType != basicType)
        {
        // we only cache if we know a custom adapter won't be registered later for this type
        ecProperty.SetCachedTypeAdapter (&basicAdapter);
        }

    return basicAdapter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterR IDgnECTypeAdapter::GetForArrayMember (ECN::ArrayECPropertyCR arrayProperty)
    {
    if (NULL != arrayProperty.GetCachedMemberTypeAdapter())
        return *((IDgnECTypeAdapterP)arrayProperty.GetCachedMemberTypeAdapter());

    DgnECExtendedTypeP extendType = DgnECExtendedType::GetForArrayProperty (arrayProperty);
    IDgnECTypeAdapterP adapter = NULL;
    if (NULL != extendType)
        adapter = extendType->GetTypeAdapter();
    else if (arrayProperty.GetCustomAttribute ("UnitSpecificationAttr").IsValid())
        adapter = &DgnECTypeRegistry::GetRegistry().GetBasicTypeAdapter (DgnECTypeRegistry::BasicType_ECUnits);
    
    if (NULL == adapter)
        {
        DgnECTypeRegistry::BasicType type = (ARRAYKIND_Struct == arrayProperty.GetKind()) ? DgnECTypeRegistry::BasicType_StructArray : DgnECTypeRegistry::BasicTypeForPrimitive (arrayProperty.GetPrimitiveElementType());
        adapter = &DgnECTypeRegistry::GetRegistry().GetBasicTypeAdapter (type);
        }

    arrayProperty.SetCachedMemberTypeAdapter (adapter);
    return *adapter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterP IDgnECTypeAdapter::GetByFormatterName (Utf8CP fmtrName)
    {
    struct NameTypePair { Utf8CP name; DgnECTypeRegistry::BasicType type; };
    static const NameTypePair s_basicTypes[] =
        {
            {   "boolean",     DgnECTypeRegistry::BasicType_Boolean        },
            {   "int",         DgnECTypeRegistry::BasicType_Integer        },
            {   "long",        DgnECTypeRegistry::BasicType_Long           },
            {   "double",      DgnECTypeRegistry::BasicType_Double         },
            {   "string",      DgnECTypeRegistry::BasicType_String         },
            {   "dateTime",    DgnECTypeRegistry::BasicType_DateTime       },
            {   "binary",      DgnECTypeRegistry::BasicType_Binary         },
            {   "point2d",     DgnECTypeRegistry::BasicType_Point2D        },
            {   "point3d",     DgnECTypeRegistry::BasicType_Point3D        },
            {   "String",      DgnECTypeRegistry::BasicType_StandardValues }
        };
    
    for (size_t i = 0; i < _countof (s_basicTypes); i++)
        {
        if (0 == strcmp (fmtrName, s_basicTypes[i].name))
            return &DgnECTypeRegistry::GetRegistry().GetBasicTypeAdapter (s_basicTypes[i].type);
        }

    return DgnECExtendedType::GetByName (fmtrName).GetTypeAdapter();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void IDgnECTypeAdapter::PreprocessFormatterForSerialization (IECInstanceR fmtr) const           { return _PreprocessFormatterForSerialization (fmtr); }
void IDgnECTypeAdapter::PostProcessDeserializedFormatter (IECInstanceR fmtr) const              { return _PostProcessDeserializedFormatter (fmtr); }

/////////////////////////////////////////////////////////////////////////////////
//  IDgnECTypeAdapterContext
//  Note this is an interface because we may want to create the context in
//  managed code and store an IDgnECInstanceLocator in it instead of a
//  DgnECInstance; that way we can avoid having to construct the native instance
//  unless it is requested.
/////////////////////////////////////////////////////////////////////////////////
DgnModelP           IDgnECTypeAdapterContext::GetDgnModel() const { return _GetDgnModel(); }
uint32_t            IDgnECTypeAdapterContext::GetComponentIndex() const     { return _GetComponentIndex(); }
bool                IDgnECTypeAdapterContext::Is3d() const                  { return _Is3d(); }
bool                IDgnECStandaloneTypeAdapterContext::ReInitialize (ECN::ECPropertyCR ecproperty, uint32_t componentIndex, DgnModelP model)
    {
    return _ReInitialize(ecproperty, componentIndex, model);
    }

/////////////////////////////////////////////////////////////////////////////////
//  StandaloneTypeAdapterContext
/////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
// static
IDgnECStandaloneTypeAdapterContextPtr StandaloneTypeAdapterContext::Create (ECN::ECPropertyCR ecprop, uint32_t componentIndex, DgnModelP model)
    {
    return new StandaloneTypeAdapterContext(ecprop, componentIndex, model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandaloneTypeAdapterContext::Is3dContext (DgnModelP model, ECPropertyCR ecprop)
    {
    bool is3d = !model || model->Is3dModel(); // NEEDS_WORK: No idea why !model implies it's 3d - just propagating the bad code for now
    is3d = is3d && ecprop.GetCustomAttribute ("IgnoreZ").IsNull();
    return is3d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandaloneTypeAdapterContext::_Is3d() const
    {
    // most type adapters don't need this, so calculate it on demand
    if (IS3D_Uninitialized == m_is3d)
        m_is3d = Is3dContext (m_model, *m_property) ? IS3D_True : IS3D_False;

    return IS3D_True == m_is3d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandaloneTypeAdapterContext::_ReInitialize (ECN::ECPropertyCR ecproperty, uint32_t componentIndex, DgnModelP model)
    {
    m_property = &ecproperty;
    m_componentIndex = componentIndex;
    m_is3d = IS3D_Uninitialized;
    m_model = model;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECClassLocaterR StandaloneTypeAdapterContext::_GetUnitsECClassLocater() const
    {
    return m_model->GetDgnDb().GetClassLocater();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IDgnECTypeAdapterContext::RegisterFactory()
    {
    IECTypeAdapterContext::RegisterFactory(&IDgnECTypeAdapterContext::CreateBase);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
IECTypeAdapterContextPtr IDgnECTypeAdapterContext::CreateBase(ECPropertyCR ecproperty, IECInstanceCR unused, uint32_t componentIndex)
    {
    // ###TODO: slight hack for navigator.
    return StandaloneTypeAdapterContext::Create(ecproperty, componentIndex, nullptr).get();
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

