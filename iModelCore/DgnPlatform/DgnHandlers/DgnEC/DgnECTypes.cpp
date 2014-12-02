/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnHandlers/DgnEC/DgnECTypes.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnHandlers/DgnECTypes.h>
#include    "DgnECTypeAdapters.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM

DGNPLATFORM_TYPEDEFS (DgnECTypeRegistry);

typedef IDgnECTypeAdapterPtr (* DgnECTypeAdapter_Create)(void);
struct StandardTypeInfo
    {
    WCharCP                                 name;
    DgnECExtendedType::StandardType         type;
    DgnECTypeAdapter_Create               Create;
    };

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct CompareNames
    {
    bool operator() (WCharCP a, WCharCP b) const {return 0 < wcscmp (a, b);}
    };


/*---------------------------------------------------------------------------------**//**
* Manages DgnECExtendedTypes and associated DgnECTypeAdapters.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnECTypeRegistry : DgnHost::HostObjectBase
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
    typedef bmap<WCharCP, UInt16, CompareNames>   ExtendedTypeIdsByName;   

    ExtendedTypesList                   m_extendedTypes;
    ExtendedTypeIdsByName               m_extendedTypeIdsByName;
    IDgnECTypeAdapterPtr                m_basicTypeAdapters[BasicType_Max];
    IDgnECTypeAdapterPtr                m_stringTypeAdapter;                    // used frequently - create one and reuse for multiple properties

    DgnECTypeRegistry();
    ~DgnECTypeRegistry();

    static DgnHost::Key&                GetHostKey();
    DgnECExtendedTypeR                  GetExtendedTypeById (UInt16 id);
public:
    static DgnECTypeRegistryR           GetRegistry();

    DgnECExtendedTypeR                  GetOrCreateExtendedType (WCharCP name);
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
    {   L"ElementType",           DgnECExtendedType::ElementType,               NULL }, // Extended type from 8.11.9, but requires no special type conversion
    {   L"Level",                 DgnECExtendedType::Level,                     NULL },
    {   L"Color",                 DgnECExtendedType::Color,                     NULL },
    {   L"Weight",                DgnECExtendedType::Weight,                    NULL },
    {   L"Style",                 DgnECExtendedType::Style,                     NULL },
    {   L"ElementClass",          DgnECExtendedType::ElementClass,              NULL },
    {   L"Coordinates",           DgnECExtendedType::Coordinates,               &CoordinatesTypeAdapter::Create },
    {   L"Distance",              DgnECExtendedType::Distance,                  &DistanceTypeAdapter::Create },
    {   L"Area",                  DgnECExtendedType::Area,                      &AreaTypeAdapter::Create },
    {   L"Volume",                DgnECExtendedType::Volume,                    &VolumeTypeAdapter::Create },
    {   L"Angle",                 DgnECExtendedType::Angle,                     &AngleTypeAdapter::Create },
    {   L"EditableArray",         DgnECExtendedType::EditableArray,             NULL }, // this type is purely a UI concept
    {   L"ElementTemplate",       DgnECExtendedType::ElementTemplate,           NULL },
    {   L"Scale",                 DgnECExtendedType::Scale,                     NULL },
    {   L"TextStyle",             DgnECExtendedType::TextStyle,                 NULL },
    {   L"RenderingMaterial",     DgnECExtendedType::RenderingMaterial,         NULL },
    {   L"DirectionAngle",        DgnECExtendedType::DirectionAngle,            &DirectionAngleTypeAdapter::Create },
    {   L"UnitlessPoint",         DgnECExtendedType::UnitlessPoint,             &UnitlessPointTypeAdapter::Create },
    {   L"NamedScale",            DgnECExtendedType::NamedScale,                NULL },
    {   L"ProjectionNamedGroup",  DgnECExtendedType::ProjectionNamedGroup,      NULL },
    {   L"ProjectionMapping",     DgnECExtendedType::ProjectionMapping_DEPRECATED, NULL }, // Changed property to use StandardValues instead, no custom type converter needed...
    {   L"Transparency",          DgnECExtendedType::Transparency,              NULL },
    {   L"DetailSymbolTemplate",  DgnECExtendedType::DetailSymbolTemplate,      NULL },
    {   L"FileSize",              DgnECExtendedType::FileSize,                  &FileSizeTypeAdapter::Create },
    {   L"FileSizeLong",          DgnECExtendedType::FileSizeLong,              &FileSizeLongTypeAdapter::Create },
    {   L"DisplayPriority",       DgnECExtendedType::DisplayPriority,           NULL },

    {   L"FontName",              DgnECExtendedType::FontName,                  NULL },
    {   L"ShxBigFontName",           DgnECExtendedType::ShxBigFontName,               NULL },
    {   L"ElementID",             DgnECExtendedType::ElementID,                 NULL },
    {   L"HatchCell",             DgnECExtendedType::HatchCell,                 NULL },
    {   L"HatchName",             DgnECExtendedType::HatchName,                 NULL },
    {   L"AnnotationLinkMemberType", DgnECExtendedType::AnnotationLinkMemberType,   NULL },               // managed converter just treats as string
    {   L"AnnotationLinkArray",   DgnECExtendedType::AnnotationLinkArray,       NULL },
    {   L"DwgCustomObjectInfoType",   DgnECExtendedType::DwgCustomObjectInfo,   NULL },
    {   L"NamedGroupStruct",      DgnECExtendedType::NamedGroupStruct,          NULL },
    {   L"NamedGroupArray",       DgnECExtendedType::NamedGroupArray,           NULL },
    {   L"KnotDetail",            DgnECExtendedType::KnotDetail,                NULL },

    // ###TODO: the following are disabled or unimplemented in Microstation Vancouver
    {   L"Discipline",              DgnECExtendedType::Discipline,              NULL },
    {   L"ViewPurpose",             DgnECExtendedType::ViewPurpose,             NULL },

    {   L"ViewType",                DgnECExtendedType::ViewType,                NULL },
    {   L"Markers",                 DgnECExtendedType::Markers,                 NULL },
    {   L"BaseDisplayStyle",        DgnECExtendedType::BaseDisplayStyle,        NULL },

    {   L"PropertyManagerFileName", DgnECExtendedType::PropertyManagerFileName, NULL },
    {   L"PropertyManagerFileSize", DgnECExtendedType::PropertyManagerFileSize, NULL },

    // note any special conversion/validation/standardvalues logic must reside in the type editor for DgnLinks properties,
    // because (1) it is valid to create a link to a non-existent file/model/etc, and (2) validation/standard values
    // require more context than can be provided by managed callers.
    {   L"DgnFileBrowser",          DgnECExtendedType::DgnFileBrowser,          &StringTypeAdapter::Create },                    // type editor only
    {   L"DgnLinks_ModelName",      DgnECExtendedType::DgnLinks_ModelName,      &StringTypeAdapter::Create },
    {   L"ConfigVarFilter",         DgnECExtendedType::ConfigVarFilter,         &StringTypeAdapter::Create },                    // type editor only
    {   L"DgnFolderBrowser",        DgnECExtendedType::DgnFolderBrowser,        &StringTypeAdapter::Create },                    // type editor only
    {   L"LogicalName",             DgnECExtendedType::LogicalName,             &StringTypeAdapter::Create },
    {   L"NamedView",               DgnECExtendedType::NamedView,               &StringTypeAdapter::Create },
    {   L"Drawings",                DgnECExtendedType::Drawings,                &StringTypeAdapter::Create },                    // disabled in Vancouver
    {   L"AddressBrowser",          DgnECExtendedType::AddressBrowser,          &StringTypeAdapter::Create },                    // type editor only
    {   L"WordHeading",             DgnECExtendedType::WordHeading,             &StringTypeAdapter::Create },                    // type editor only
    {   L"WordBookmark",            DgnECExtendedType::WordBookmark,            &StringTypeAdapter::Create },                    // type editor only
    {   L"PdfBookmark",             DgnECExtendedType::PdfBookmark,             &StringTypeAdapter::Create },                    // type editor only

    {   L"NamedGroupType",          DgnECExtendedType::NamedGroupType,          NULL },
    {   L"XyzRotations",            DgnECExtendedType::XyzRotations,            &XyzRotationsTypeAdapter::Create },
    {   L"UnitDefinition",          DgnECExtendedType::UnitDefinition,          &UnitDefinitionTypeAdapter::Create },
    {   L"DateTimeFormat",          DgnECExtendedType::DateTimeFormat,          &DateTimeFormatTypeAdapter::Create },

    {   L"VariableAngle",           DgnECExtendedType::VariableAngle,           &VariableAngleTypeAdapter::Create },
    {   L"VariableBoolean",         DgnECExtendedType::VariableBoolean,         &VariableBooleanTypeAdapter::Create },
    {   L"VariableColor",           DgnECExtendedType::VariableColor,           NULL },
    {   L"VariableDistance",        DgnECExtendedType::VariableDistance,        &VariableDistanceTypeAdapter::Create },
    {   L"VariableFontName",        DgnECExtendedType::VariableFontName,        NULL },
    {   L"VariableDouble",          DgnECExtendedType::VariableDouble,          &VariableDoubleTypeAdapter::Create },
    {   L"VariableStandardValues",  DgnECExtendedType::VariableStandardValues,  &VariableEnumTypeAdapter::Create },
    {   L"VariableTextStyle",       DgnECExtendedType::VariableTextStyle,       NULL },

    {   L"DisplayStyle",            DgnECExtendedType::DisplayStyle,            NULL },
    {   L"DisplayStyles",           DgnECExtendedType::DisplayStyles,           NULL },
    {   L"DisplayStyleType",        DgnECExtendedType::DisplayStyleType,        NULL },
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
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP) const override { return false; }
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
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
    UInt16 id = extendType.GetId();
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
        case BasicType_Point3D:             adapter = Point3DTypeAdapter::Create(); break;
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
DgnECExtendedTypeR DgnECTypeRegistry::GetOrCreateExtendedType (WCharCP name)
    {
    // Note if name refers to a standard type (with a StandardType id) it will already exist in m_extendedTypeIdsByName
    ExtendedTypeIdsByName::iterator foundIter = m_extendedTypeIdsByName.find (name);
    if (foundIter != m_extendedTypeIdsByName.end())
        return GetExtendedTypeById (foundIter->second);

    UInt16 newId = (UInt16)m_extendedTypes.size() + DgnECExtendedType::STANDARD_First;
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
DgnECExtendedTypeR DgnECTypeRegistry::GetExtendedTypeById (UInt16 id)
    {
    id -= DgnECExtendedType::STANDARD_First;
    BeAssert (id < (UInt16)m_extendedTypes.size() && NULL != m_extendedTypes[id]);
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
    if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"Standard") && !v.IsNull())
        {
        Int32 standardId = v.GetInteger();
        if (standardId >= DgnECExtendedType::STANDARD_First && standardId <= DgnECExtendedType::STANDARD_Last)
            return &DgnECTypeRegistry::GetRegistry().GetStandardExtendedType ((DgnECExtendedType::StandardType)standardId);
        else
            {
            BeAssert (false && "Custom attribute for extended type has invalid 'Standard' type");
            return NULL;
            }
        }
    else if (ECOBJECTS_STATUS_Success == customAttr.GetValue (v, L"Name") && !v.IsNull())
        return &DgnECTypeRegistry::GetRegistry().GetOrCreateExtendedType (v.GetString());
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
    IECInstancePtr customAttr = ecProperty.GetCustomAttribute (L"ExtendType");
    return customAttr.IsValid() ? getExtendedTypeFromAttribute (*customAttr) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECExtendedTypeP DgnECExtendedType::GetForArrayProperty (ECN::ArrayECPropertyCR ecProperty)
    {
    IECInstancePtr customAttr = ecProperty.GetCustomAttribute (L"MemberExtendedType");
    return customAttr.IsValid() ? getExtendedTypeFromAttribute (*customAttr) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECExtendedTypeR DgnECExtendedType::GetByName (WCharCP name)
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
bool IDgnECTypeAdapter::_ConvertToString (WStringR valueAsString, ECValueCR v, IECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    return _ConvertToString (valueAsString, v, static_cast<IDgnECTypeAdapterContextCR>(context), formatter);
    }
bool IDgnECTypeAdapter::_ConvertFromString (ECValueR v, WCharCP stringValue, IECTypeAdapterContextCR context) const
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
bool IDgnECTypeAdapter::_ConvertToDisplayType (ECValueR v, IECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    return _ConvertToDisplayType (v, static_cast<IDgnECTypeAdapterContextCR>(context), opts);
    }
bool IDgnECTypeAdapter::_GetDisplayType(PrimitiveType& type) const
    {
    type = PRIMITIVETYPE_String;
    return true;
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
    WString str;
    if (!GetDisplayType (type))
        return false;
    else if (type != PRIMITIVETYPE_String)
        {
        BeAssert (false && "If you override GetDisplayType() you must override default implementation of ConvertToDisplayType()");
        return false;
        }
    else if (!ConvertToString (str, v, context, opts))
        return false;
    
    v.SetString (str.c_str());
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
#ifdef NEEDSWORK_TYPEADAPTERS
        // This presents a serious performance issue when doing free text search queries - we repeatedly encounter the same property with a named extended type,
        // for which no type adapter can be located. It is most likely the extended type is not *intended* to have a type adapter. So to avoid this, let's instead
        // cache a type adapter based on the property's underlying type on the ECProperty
        // ###TODO: a better solution would be to add a property to the ExtendType/MemberExtendedType custom attributes indicating whether or not we should expect
        // a type adapter to be registered for this type - but of course we then depend on people actually setting that property intelligently.
        else            // failed to locate specialized adapter; use default with limited functionality
            basicType = DgnECTypeRegistry::BasicType_MissingExtendType;
        }
    else if (!ecProperty.GetIsPrimitive())
#else
        }

    if (!ecProperty.GetIsPrimitive())
#endif
        basicType = DgnECTypeRegistry::BasicType_Dummy;
    else
        {
        PrimitiveECPropertyCP primitiveProperty = ecProperty.GetAsPrimitiveProperty();
        PrimitiveType primType = primitiveProperty->GetType();

        // Check for special custom attributes
        if (primitiveProperty->IsDefined (L"Format"))
            basicType = DgnECTypeRegistry::BasicType_FormatString;
        else if (PRIMITIVETYPE_Boolean == primType && primitiveProperty->IsDefined (L"BooleanDisplay"))
            basicType = DgnECTypeRegistry::BasicType_BooleanDisplay;
        else if (PRIMITIVETYPE_Integer == primType && primitiveProperty->IsDefined (L"StandardValues"))
            basicType = DgnECTypeRegistry::BasicType_StandardValues;
        else if (primitiveProperty->IsDefined (L"UnitSpecification"))
            basicType = DgnECTypeRegistry::BasicType_ECUnits;
        else
            {
            // unadorned primitive type
            basicType = DgnECTypeRegistry::BasicTypeForPrimitive (primType);
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
    else if (arrayProperty.GetCustomAttribute (L"UnitSpecification").IsValid())
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
IDgnECTypeAdapterP IDgnECTypeAdapter::GetByFormatterName (WCharCP fmtrName)
    {
    struct NameTypePair { WCharCP name; DgnECTypeRegistry::BasicType type; };
    static const NameTypePair s_basicTypes[] =
        {
            {   L"boolean",     DgnECTypeRegistry::BasicType_Boolean        },
            {   L"int",         DgnECTypeRegistry::BasicType_Integer        },
            {   L"long",        DgnECTypeRegistry::BasicType_Long           },
            {   L"double",      DgnECTypeRegistry::BasicType_Double         },
            {   L"string",      DgnECTypeRegistry::BasicType_String         },
            {   L"dateTime",    DgnECTypeRegistry::BasicType_DateTime       },
            {   L"binary",      DgnECTypeRegistry::BasicType_Binary         },
            {   L"point2d",     DgnECTypeRegistry::BasicType_Point2D        },
            {   L"point3d",     DgnECTypeRegistry::BasicType_Point3D        },
            {   L"String",      DgnECTypeRegistry::BasicType_StandardValues }
        };
    
    for (size_t i = 0; i < _countof (s_basicTypes); i++)
        {
        if (0 == wcscmp (fmtrName, s_basicTypes[i].name))
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

DgnModelP        IDgnECTypeAdapterContext::GetDgnModel() const           { return _GetDgnModel(); }
UInt32              IDgnECTypeAdapterContext::GetComponentIndex() const     { return _GetComponentIndex(); }
bool                IDgnECTypeAdapterContext::Is3d() const                  { return _Is3d(); }
bool                IDgnECStandaloneTypeAdapterContext::ReInitialize (ECN::ECPropertyCR ecproperty, UInt32 componentIndex, DgnProjectP dgnFile, DgnModelP modelRef)
    {
    return _ReInitialize (ecproperty, componentIndex, dgnFile, modelRef);
    }

/////////////////////////////////////////////////////////////////////////////////
//  StandaloneTypeAdapterContext
/////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
// static
IDgnECStandaloneTypeAdapterContextPtr StandaloneTypeAdapterContext::Create (ECN::ECPropertyCR ecprop, UInt32 componentIndex, DgnProjectP file, DgnModelP modelRef)
    {
    return new StandaloneTypeAdapterContext (ecprop, componentIndex, file, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandaloneTypeAdapterContext::Is3dContext (DgnModelP modelRef, ECPropertyCR ecprop)
    {
    bool is3d = (NULL == modelRef) || (DgnModelType::Physical == modelRef->GetModelType() && modelRef->Is3d());
    is3d = is3d && ecprop.GetCustomAttribute (L"IgnoreZ").IsNull();
    return is3d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandaloneTypeAdapterContext::_Is3d() const
    {
    // most type adapters don't need this, so calculate it on demand
    if (IS3D_Uninitialized == m_is3d)
        m_is3d = Is3dContext (m_modelRef, *m_property) ? IS3D_True : IS3D_False;

    return IS3D_True == m_is3d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandaloneTypeAdapterContext::_ReInitialize (ECN::ECPropertyCR ecproperty, UInt32 componentIndex, DgnProjectP dgnFile, DgnModelP modelRef)
    {
    m_property = &ecproperty;
    m_componentIndex = componentIndex;
    m_is3d = IS3D_Uninitialized;
    m_dgnfile = dgnFile;
    m_modelRef = modelRef;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECClassLocaterR StandaloneTypeAdapterContext::_GetUnitsECClassLocater() const
    {
    return m_modelRef->GetDgnProject().GetEC().GetClassLocater();
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

