/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnECTypes.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#if defined (_MSC_VER)
    #pragma managed(push, off)
#endif // defined (_MSC_VER)

DGNPLATFORM_TYPEDEFS (DgnECExtendedType);
DGNPLATFORM_TYPEDEFS (IDgnECTypeAdapterContext);
DGNPLATFORM_TYPEDEFS (IDgnECTypeAdapter);
DGNPLATFORM_TYPEDEFS (IDgnECStandaloneTypeAdapterContext);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef RefCountedPtr<IDgnECTypeAdapter> IDgnECTypeAdapterPtr;

/*=================================================================================**//**
An ECSchema may define an extended type for an ECProperty. This adds semantics to the interpretation of the internal primitive type (int, string, etc) of the property.
A property with an extended type has a TypeAdapter associated with it that can interpret the internal value of the property in the context of its extended type.
@ingroup DgnECGroup
@see IDgnECTypeAdapter
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct DgnECExtendedType
    {
    friend struct DgnECTypeRegistry;

//__PUBLISH_CLASS_VIRTUAL__

    /*=================================================================================**//**
    Standard extended types which can be specified by integer ID in an ECSchema
    @bsiclass
    +---------------+---------------+---------------+---------------+---------------+------*/
    enum StandardType : uint16_t
        {
/*__PUBLISH_SECTION_END__*/
        NotStandard                         = static_cast <uint16_t> (-1),
/*__PUBLISH_SECTION_START__*/
        //! Element type id : int
        ElementType                         = 1,
        //! Element level id : int. Context must supply the DgnFile containing the level ID.
        Level                               = 2,
        //! Element color : int. Context must supply the DgnFile containing the color ID.
        Color                               = 3,
        //! Element weight : int
        Weight                              = 4,
        //! Element line style id : int
        Style                               = 5,
        //! Element class id : int
        ElementClass                        = 6,
        //! Point property with units : point3d. Units derived from context's model, if provided.
        Coordinates                         = 7,
        //! Distance property with units : double. Units derived from context's model, if provided.
        Distance                            = 8,
        //! Area property with units : double. Units derived from context's model, if provided.
        Area                                = 9,
        //! Volume property with units : double. Units derived from context's model, if provided.
        Volume                              = 10,
        //! Angle : double. Units derived from context's model, if provided.
        Angle                               = 11,
/*__PUBLISH_SECTION_END__*/
        EditableArray                       = 12,
        ElementTemplate                     = 13,
        Scale                               = 14,
/*__PUBLISH_SECTION_START__*/
        //! Text style id : int
        TextStyle                           = 15,
        //! Material : string
        RenderingMaterial                   = 16,
        //! Direction angle : double. Units derived from context's model, if provided.
        DirectionAngle                      = 17,
        //! Point property with no units : point3d
        UnitlessPoint                       = 18,
        //! Named scale : double
        NamedScale                          = 19,
        //! Material projection named group : string. Context must supply DgnModelRef containing named group.
        ProjectionNamedGroup                = 20,
/*__PUBLISH_SECTION_END__*/
        ProjectionMapping_DEPRECATED        = 21,
/*__PUBLISH_SECTION_END__*/
        //! Element transparency : double
        Transparency                        = 22,
/*__PUBLISH_SECTION_END__*/
        DetailSymbolTemplate                = 23,
/*__PUBLISH_SECTION_START__*/
        //! File size, with abbreviated string representation : long
        FileSize                            = 24,
        //! File size, with verbose string representation : long
        FileSizeLong                        = 25,
        //! Display priority : int
        DisplayPriority                     = 26,

        //! Font id : int. Context must supply the DgnFile containing the font ID.
        FontName                            = 27,
        //! Large Font id : int. Context must supply the DgnFile containing the font ID.
        ShxBigFontName                      = 28,
        //! Element ID : long
        ElementID                           = 29,
        //! Hatch cell name : string
        HatchCell                           = 30,
/*__PUBLISH_SECTION_END__*/
        HatchName                           = 31,

        AnnotationLinkMemberType            = 32,
        AnnotationLinkArray                 = 33,
        DwgCustomObjectInfo                 = 34,
        NamedGroupStruct                    = 35,
        NamedGroupArray                     = 36,
        KnotDetail                          = 37,

        // Views
        Discipline                          = 38,
        ViewPurpose                         = 39,
        ViewType                            = 40,
        Markers                             = 41,
        BaseDisplayStyle                    = 42,

        PropertyManagerFileName             = 43,
        PropertyManagerFileSize             = 44,

        // Links
        DgnFileBrowser                      = 45,
        DgnLinks_ModelName                  = 46,
        ConfigVarFilter                     = 47,
        DgnFolderBrowser                    = 48,
        LogicalName                         = 49,
        NamedView                           = 50,
        Drawings                            = 51,
        AddressBrowser                      = 52,
        WordHeading                         = 53,
        WordBookmark                        = 54,
        PdfBookmark                         = 55,

        NamedGroupType                      = 56,
/*__PUBLISH_SECTION_START__*/
        //! Point property containing rotations around 3 axes, formatted as angles with units. Units derived from context's DgnModelRef, if provided.
        XyzRotations                        = 57,
        //! Unit Defiition ID : int.
        UnitDefinition                      = 58,
/*__PUBLISH_SECTION_END__*/
        DateTimeFormat                      = 59,

        // Types which support "Varies Across" value for compound elements (e.g. Text Nodes)
        VariableAngle                       = 60,
        VariableBoolean                     = 61,
        VariableColor                       = 62,
        VariableDistance                    = 63,
        VariableFontName                    = 64,
        VariableDouble                      = 65,
        VariableStandardValues              = 66,
        VariableTextStyle                   = 67,

        DisplayStyle                        = 68,       // element
        DisplayStyles                       = 69,       // clip element - different type editor
        DisplayStyleType                    = 70,       // reference attachment. Note all of these confusing names ported from 8.11; probably safe to rename them...

        // IMPORTANT: This enum must be kept in synch with ECPropertyPane.PropertyExtendedType enum
        // If you add values, update STANDARD_Last and STANDARD_Count here, and add the new values to PropertyExtendedType.
        STANDARD_First                      = ElementType,
        STANDARD_Last                       = DisplayStyleType,
        STANDARD_Count                      = (STANDARD_Last - STANDARD_First + 1)
/*__PUBLISH_SECTION_START__*/
        };
/*__PUBLISH_SECTION_END__*/

private:
    Utf8String                         m_name;
    uint16_t                        m_id;
    mutable IDgnECTypeAdapterPtr    m_typeAdapter;        // mutable because it is lazily initialized

    DgnECExtendedType (Utf8CP name, uint16_t id) : m_name (name), m_id (id) { }
public:
    DGNPLATFORM_EXPORT Utf8CP                 GetName() const         { return m_name.c_str(); }

    //! Every extended type has a unique id. For standard types, this is equal to the corresponding StandardType enum value.
    //! For non-standard types, the id is dynamically assigned at run-time.
    DGNPLATFORM_EXPORT uint16_t                GetId() const           { return m_id; }
    DGNPLATFORM_EXPORT bool                    IsStandardType() const  { return m_id >= STANDARD_First && m_id <= STANDARD_Last; }
    DGNPLATFORM_EXPORT StandardType            GetStandardType() const { return IsStandardType() ? (StandardType)m_id : NotStandard; }

    DGNPLATFORM_EXPORT bool                    IsStandardDirectionalType() const;
    DGNPLATFORM_EXPORT bool                    IsStandardDistanceType() const;

    //! Get the specialized adapter for this extended type, or null if no specialized adapter exists for this type.
    DGNPLATFORM_EXPORT IDgnECTypeAdapterP      GetTypeAdapter() const;

    //! Set a custom type adapter for this extended type, overwriting any pre-existing adapter.
    //! Note that the standard type adapters are currently allowed to be overridden; this may change in the future.
    DGNPLATFORM_EXPORT void                    SetTypeAdapter (IDgnECTypeAdapterR adapter);

    //! For unit tests only. Invalidates any cached pointers to the extended type's type adapter.
    DGNPLATFORM_EXPORT void                    BackDoor_Unregister();

    DGNPLATFORM_EXPORT static DgnECExtendedTypeP    GetForProperty (ECN::ECPropertyCR ecProperty);               // from ExtendType custom attribute
    DGNPLATFORM_EXPORT static DgnECExtendedTypeP    GetForArrayProperty (ECN::ArrayECPropertyCR ecProperty);     // from MemberExtendedType custom attribute
    DGNPLATFORM_EXPORT static DgnECExtendedTypeR    GetByName (Utf8CP extendTypeName);
    DGNPLATFORM_EXPORT static DgnECExtendedTypeR    GetByStandardId (StandardType standardType);
/*__PUBLISH_SECTION_START__*/
    };

/*=================================================================================**//**
Supplies the context in which an IDgnECTypeAdapter operates.
@ingroup DgnECGroup
@see IDgnECTypeAdapter
@see DgnECExtendedType
@bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef RefCountedPtr<IDgnECTypeAdapterContext> IDgnECTypeAdapterContextPtr;
typedef RefCountedPtr<IDgnECStandaloneTypeAdapterContext> IDgnECStandaloneTypeAdapterContextPtr;
struct IDgnECTypeAdapterContext : ECN::IECTypeAdapterContext
    {
/*__PUBLISH_SECTION_END__*/
protected:
    virtual DgnModelP                       _GetDgnModel() const = 0;
    virtual uint32_t                        _GetComponentIndex() const = 0;
    virtual bool                            _Is3d() const = 0;
public:

    DGNPLATFORM_EXPORT  DgnModelP               GetDgnModel() const;
    
    //! The following are relevant to adapters for point types.
    DGNPLATFORM_EXPORT  uint32_t                GetComponentIndex() const;
    DGNPLATFORM_EXPORT  bool                    Is3d() const;

    static ECN::IECTypeAdapterContextPtr        CreateBase (ECN::ECPropertyCR ecproperty, ECN::IECInstanceCR instance, uint32_t componentIndex);
    static void RegisterFactory();

/*__PUBLISH_SECTION_START__*/
public:
    static const uint32_t COMPONENT_INDEX_None    = -1;
    };

/*=================================================================================**//**
A reusable IDgnECTypeAdapterContext.
@ingroup DgnECGroup
@bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDgnECStandaloneTypeAdapterContext : IDgnECTypeAdapterContext
    {
/*__PUBLISH_SECTION_END__*/
protected:
    virtual bool                            _ReInitialize(ECN::ECPropertyCR ecproperty, uint32_t componentIndex, DgnModelP model) = 0;
/*__PUBLISH_SECTION_START__*/
public:
    //! Re-initialize this context
    //! If instance can be cast to DgnECInstance and the file/modelRef/elemRef arguments are NULL, they will be taken from the DgnECInstance
    //! @param[in] ecproperty       The ECProperty the type adapter will operate on
    //! @param[in] componentIndex   For point properties, the component index (x=0,y=1,z=2) if the type adapter should operate only on a single component of the point
    //! @param[in] dgnFile          The DgnFile from which the property value originated
    //! @param[in] modelRef         The DgnModelRef associated with the property value
    //! @return true if the context was successfully re-initialized
    DGNPLATFORM_EXPORT bool                 ReInitialize(ECN::ECPropertyCR ecproperty, uint32_t componentIndex = COMPONENT_INDEX_None, DgnModelP model = nullptr);
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct StandaloneTypeAdapterContext : IDgnECStandaloneTypeAdapterContext
    {
/*__PUBLISH_SECTION_END__*/
private:
    enum IS3D_State { IS3D_Uninitialized, IS3D_True, IS3D_False };

    ECN::ECPropertyCP           m_property;
    uint32_t                    m_componentIndex;
    mutable IS3D_State          m_is3d;
    DgnModelP                   m_model;

    StandaloneTypeAdapterContext (ECN::ECPropertyCR prop, uint32_t comp,  DgnModelP model)
        : m_property(&prop), m_componentIndex(comp), m_is3d(IS3D_Uninitialized), m_model(model)
        {}

    static bool Is3dContext(DgnModelP model, ECN::ECPropertyCR ecprop);

    virtual ECN::ECPropertyCP   _GetProperty() const override           { return m_property; }
    virtual uint32_t            _GetComponentIndex() const override     { return m_componentIndex; }
    virtual bool                _Is3d() const override;
    virtual DgnModelP           _GetDgnModel() const override { return m_model; }
    virtual ECN::IECInstanceCP  _GetECInstance() const override         { BeAssert (false && "Unused in Graphite"); return NULL; }
    virtual bool  _ReInitialize(ECN::ECPropertyCR ecproperty, uint32_t componentIndex, DgnModelP model) override;
    virtual ECN::IECClassLocaterR _GetUnitsECClassLocater() const override;
    
/*__PUBLISH_SECTION_START__*/
/*__PUBLISH_CLASS_VIRTUAL__*/
public:
    //! Create a StandaloneTypeAdapterContext
    //! @param[in] ecproperty       The ECProperty the type adapter will operate on
    //! @param[in] componentIndex   For point properties, the component index (x=0,y=1,z=2) if the type adapter should operate only on a single component of the point
    //! @param[in] model            The DgnModel associated with the property value
    DGNPLATFORM_EXPORT static IDgnECStandaloneTypeAdapterContextPtr Create (ECN::ECPropertyCR ecprop, uint32_t componentIndex, DgnModelP model);
    };

/*=================================================================================**//**
Provides methods for converting to and from an ECProperty's internal type to a user-friendly representation.
All ECProperties are associated with a type adapter. If the property has an ExtendType or MemberExtendedType custom attribute,
DgnPlatform will attempt to locate a type adapter specific to that ExtendType, which may be supplied by external code to create custom types.
@ingroup DgnECGroup
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct IDgnECTypeAdapter : ECN::IECTypeAdapter
    {
/*__PUBLISH_SECTION_END__*/
private:
    DGNPLATFORM_EXPORT virtual bool _ConvertToString(Utf8StringR str, ECN::ECValueCR v, ECN::IECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    DGNPLATFORM_EXPORT virtual bool _ConvertFromString(ECN::ECValueR v, Utf8CP str, ECN::IECTypeAdapterContextCR context) const override;
    DGNPLATFORM_EXPORT virtual bool _ConvertToExpressionType (ECN::ECValueR v, ECN::IECTypeAdapterContextCR context) const override;
    DGNPLATFORM_EXPORT virtual bool _ConvertFromExpressionType (ECN::ECValueR v, ECN::IECTypeAdapterContextCR context) const override;
    DGNPLATFORM_EXPORT virtual bool _CanConvertFromString (ECN::IECTypeAdapterContextCR) const override;
    DGNPLATFORM_EXPORT virtual bool _CanConvertToString (ECN::IECTypeAdapterContextCR) const override;
    DGNPLATFORM_EXPORT virtual bool _GetPlaceholderValue(ECN::ECValueR v, ECN::IECTypeAdapterContextCR context) const /*final*/ override;

    DGNPLATFORM_EXPORT virtual bool _ConvertToDisplayType (ECN::ECValueR v, ECN::IECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    DGNPLATFORM_EXPORT virtual bool _GetUnits (ECN::UnitSpecR units, ECN::IECTypeAdapterContextCR context) const override;

protected:
    virtual bool            _HasStandardValues() const override     { return false; }
    virtual bool            _SupportsUnits() const override         { return false; }
    virtual bool            _IsStruct() const override              { return false; }
    virtual bool            _AllowExpandMembers() const override    { return false; }
    //! Default implementation returns false, indicating properties represented by this adapter should be ignored for EC string comparison queries
    virtual bool            _IsTreatedAsString() const override     { return false; }
    //! Unused in Graphite.
    virtual ECN::IECInstancePtr  _CreateDefaultFormatter (bool includeAllValues, bool forDwg) const override   { BeAssert (false && "Should never be used in Graphite"); return NULL; }
    //! Unused in Graphite.
    DGNPLATFORM_EXPORT virtual ECN::IECInstancePtr   _PopulateDefaultFormatterProperties (ECN::IECInstanceCR formatter) const override { BeAssert (false && "Should never be used in Graphite"); return NULL; }
    //! Unused in Graphite.
    DGNPLATFORM_EXPORT virtual ECN::IECInstancePtr   _CondenseFormatterForSerialization (ECN::IECInstanceCR formatter) const override { BeAssert (false && "Should never be used in Graphite"); return NULL; }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const = 0;
    virtual bool            _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const = 0;
    virtual bool            _ConvertFromString(ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const = 0;
    virtual bool            _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const { return false; }
    virtual bool            _CanConvertFromString (IDgnECTypeAdapterContextCR) const { return true; }
    virtual bool            _CanConvertToString (IDgnECTypeAdapterContextCR) const { return true; }
    virtual bool            _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const = 0;

    //! Default implementations assume no conversion needed.
    virtual bool            _RequiresExpressionTypeConversion (ECN::EvaluationOptions evalOptions) const override { return false; }
    virtual bool            _ConvertToExpressionType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const { return true; }
    virtual bool            _ConvertFromExpressionType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const { return true; }

    //! Default implementations assume:
    //! -If CanConvertToString(), display type is string and ConvertToDisplayType() calls ConvertToString()
    //! -Else assumes no display type and ConvertToDisplayType() returns false. 
    DGNPLATFORM_EXPORT virtual bool            _GetDisplayType (ECN::PrimitiveType& type) const override;
    DGNPLATFORM_EXPORT virtual bool            _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const;

    DGNPLATFORM_EXPORT virtual bool            _IsOrdinalType () const override;

    // Called immediately before the formatting instance is serialized to a persistent format.
    virtual void            _PreprocessFormatterForSerialization (ECN::IECInstanceR formatter) const { }
    // Called immediately after the formatting instance is deserialized from a persistent format.
    virtual void            _PostProcessDeserializedFormatter (ECN::IECInstanceR formatter) const { }

    virtual bool            _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const { return false; }

public:
    // These methods are used by TextField serialization/deserialization. Exported only for tests.
    DGNPLATFORM_EXPORT void                 PreprocessFormatterForSerialization (ECN::IECInstanceR formatter) const;
    DGNPLATFORM_EXPORT void                 PostProcessDeserializedFormatter (ECN::IECInstanceR formatter) const;

/*__PUBLISH_SECTION_START__*/
public:
    //! Determine if the proposed value is valid for the ECProperty specified by the context
    //! @param[in] v        The proposed value
    //! @param[in] context  The context in which to evaluate
    //! @return true if the value is valid for the property
    DGNPLATFORM_EXPORT bool                 Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const;

    //! Get a list of strings representing permissible values for this property
    //! @param[out] values  Permissible values, as a list of user-friendly strings
    //! @param[in] context  The context under which to evaluate
    //! @return true if the list of permissible values was populated successfully
    DGNPLATFORM_EXPORT bool                 GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const;

    //! Obtain a type adapter for the specified property. If the property specifies an ExtendType custom attribute, it will be used to locate the type adapter.
    //! @param[in] ecProperty   The property for which to obtain a type adapter
    //! @return a type adapter for the specified property
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterR    GetForProperty (ECN::ECPropertyCR ecProperty);

    //! Obtain a type adapter for members of the specified array property. If the property specifies a MemberExtendedType custom attribute, it will be used to locate the type adapter.
    //! @param[in] arrayProperty    The array property for which to obtain a member type adapter
    //! @return a type adapter for members of the specified array property
    DGNPLATFORM_EXPORT static IDgnECTypeAdapterR    GetForArrayMember (ECN::ArrayECPropertyCR arrayProperty);

/*__PUBLISH_SECTION_END__*/
    //! Internal method to get an adapter by its 'formatter name' as persisted in TextField
    static IDgnECTypeAdapterP                       GetByFormatterName (Utf8CP fmtrName);
/*__PUBLISH_SECTION_START__*/
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (_MSC_VER)
    #pragma managed(pop)
#endif // defined (_MSC_VER)

/** @endcond */
