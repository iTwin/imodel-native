/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/PropertyContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include    "../DgnPlatform.h"
#include    "IViewDraw.h" // for LineStyleParams
#include    <Bentley/bvector.h>
#include    <DgnPlatform/DgnCore/Material.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/** @addtogroup PropertyContext */
/** @beginGroup */

//=======================================================================================
// @bsiclass 
//=======================================================================================
enum class EditPropertyPurpose
    {
    Change          =  0, // Change attributes, style changes, etc.
    Remap           =  1, // Clone and shared file id remapping
    };

enum class QueryPropertyPurpose
    {
    NotSpecified    = -1, // default reason, announce all properties of requested type.
    Match           =  0, // match element, match from cursor, etc.
    };

//=======================================================================================
//! Flags that describe how an element handler uses a property value.
// @bsiclass 
//=======================================================================================
enum PropsCallbackFlags
    {
    PROPSCALLBACK_FLAGS_NoFlagsSet          =  (0),    //!< Normal non-base id
    PROPSCALLBACK_FLAGS_ElementIgnoresID    =  (1<<0), //!< Not used, remap can zero id
    PROPSCALLBACK_FLAGS_IsBaseID            =  (1<<1), //!< Base/Primary id for element
    PROPSCALLBACK_FLAGS_IsBackgroundID      =  (1<<2), //!< element fill and text background
    PROPSCALLBACK_FLAGS_IsDecorationID      =  (1<<3), //!< hatch/pattern
    PROPSCALLBACK_FLAGS_UndisplayedID       =  (1<<4), //!< Not used or not visible, still requires remap
    };

//__PUBLISH_SECTION_END__
#define SETPROPCBEIFLAG(UseID) ((UseID) ? PROPSCALLBACK_FLAGS_NoFlagsSet : PROPSCALLBACK_FLAGS_ElementIgnoresID)

//=======================================================================================
// @bsiclass 
//=======================================================================================
enum PropsCallerFlags
    {
    PROPSCALLER_FLAGS_NoFlagsSet                      = (0),    // Caller does not request special action
    PROPSCALLER_FLAGS_SharedChildOvrSet               = (1<<0), // Caller requests overrides be set for shared children (if applicable)
    PROPSCALLER_FLAGS_SharedChildOvrClear             = (1<<1), // Caller requests overrides be cleared for shared children (if applicable)
    PROPSCALLER_FLAGS_PreserveOpaqueFill              = (1<<2), // Caller requests fill color change when changing base color when fill is the same as base color
    PROPSCALLER_FLAGS_PreserveMatchingDecorationColor = (1<<3), // Caller requests decorator color change when changing base color when decoration is the same as base color
    };
//__PUBLISH_SECTION_START__

//=======================================================================================
//! Used by IEditProperties to signal additional action to be taken regarding a style property.
// @bsiclass 
//=======================================================================================
enum class StyleParamsRemapping
    {
    Invalid     = -1,     //!< This value is invalid.
    NoChange    =  0,     //!< No additional action will be taken.
    ApplyStyle  =  1,     //!< The style will be applied to the element.
    Override    =  2,     //!< Overrides will be stored on the element for each property that doesn't match the style.
    };

//=======================================================================================
// @bsiclass 
//=======================================================================================
enum ElementProperties
{
    ELEMENT_PROPERTY_None               = (0),
    ELEMENT_PROPERTY_Category           = (1<<0),
    ELEMENT_PROPERTY_Color              = (1<<1),
    ELEMENT_PROPERTY_Linestyle          = (1<<2),
    ELEMENT_PROPERTY_Font               = (1<<3),
    ELEMENT_PROPERTY_DimStyle           = (1<<5),
    ELEMENT_PROPERTY_MLineStyle         = (1<<6),
    ELEMENT_PROPERTY_Material           = (1<<7),
    ELEMENT_PROPERTY_Weight             = (1<<8),
    ELEMENT_PROPERTY_ElementClass       = (1<<9),
    ELEMENT_PROPERTY_Transparency       = (1<<10),
    ELEMENT_PROPERTY_DisplayPriority    = (1<<11),
    ELEMENT_PROPERTY_ElementTemplate    = (1<<12),
    ELEMENT_PROPERTY_Thickness          = (1<<13),

    ELEMENT_PROPERTY_LastValue          = (1<<13), // please advance this value when new property types added
    ELEMENT_PROPERTY_All                = 0xffffffff,
};

//=======================================================================================
//! Base class common to element property callback arguments.
// @bsiclass 
//=======================================================================================
class EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
protected:
    PropertyContextR        m_processor;
    PropsCallbackFlags      m_flags;
    PropsCallerFlags        m_callerFlags;

public:
    EachPropertyBaseArg (PropsCallbackFlags flags, PropertyContextR context) : m_flags (flags), m_processor (context), m_callerFlags (PROPSCALLER_FLAGS_NoFlagsSet) {}

    PropertyContextR GetPropertyContext () {return m_processor;}
    void SetPropertyFlags (PropsCallbackFlags flags) {m_flags = flags;}
    PropsCallerFlags GetPropertyCallerFlags () {return m_callerFlags;}
    void SetPropertyCallerFlags (PropsCallerFlags flags) {m_callerFlags = flags;}

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:
    //! Flags associated with the property value.
    DGNPLATFORM_EXPORT PropsCallbackFlags GetPropertyFlags ();
};

//=======================================================================================
//! Element level information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachCategoryArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:
    DgnCategoryId m_storedValue;

public:
    DGNPLATFORM_EXPORT EachCategoryArg (DgnCategoryId stored, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the value of the property that is stored in the element.
    DGNPLATFORM_EXPORT DgnCategoryId GetStoredValue ();

    //! Get the effective value of the property that is used for display.  This is computed from
    //! the stored value considering header overrides.
    DGNPLATFORM_EXPORT DgnCategoryId GetEffectiveValue ();

    //! Assign a new value in place of the one passed to the callback.
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (DgnCategoryId newVal);
};

//=======================================================================================
//! Element color information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachColorArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:
    uint32_t m_storedValue;

public:
    DGNPLATFORM_EXPORT EachColorArg (uint32_t stored, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the value of the property that is stored in the element.
    DGNPLATFORM_EXPORT uint32_t GetStoredValue ();

    //! Get the effective value of the property that is used for display.  This is computed from
    //! the stored value considering header overrides.  If the stored value is BYCELL or BYLEVEL,
    //! the effective value will resolve the value from the cell or level respectively.
    DGNPLATFORM_EXPORT uint32_t GetEffectiveValue ();

    //! Assign a new value in place of the one passed to the callback.
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (uint32_t newVal);
};

//=======================================================================================
//! Element linestyle information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachLineStyleArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:
    int32_t           m_storedValue;
    LineStyleParams   m_params;
    bool              m_paramsChanged;

public:
    DGNPLATFORM_EXPORT EachLineStyleArg (int32_t stored, LineStyleParams const*, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the value of the property that is stored in the element.
    DGNPLATFORM_EXPORT int32_t GetStoredValue ();

    //! Get the effective value of the property that is used for display.  This is computed from
    //! the stored value considering header overrides.  If the stored value is BYCELL or BYLEVEL,
    //! the effective value will resolve the value from the cell or level respectively.
    DGNPLATFORM_EXPORT int32_t GetEffectiveValue ();

    //! For this linestyle value stored on the element, get the corresponding stored line style parameters.
    DGNPLATFORM_EXPORT LineStyleParams const* GetParams ();

    //! For this linestyle value stored on the element, get the resolved parameters taking into account level.
    DGNPLATFORM_EXPORT bool GetEffectiveParams (LineStyleParams&);

    //! Get the value of the flag stating whether the parameters have been changed.  
    DGNPLATFORM_EXPORT bool GetParamsChanged ();

    //! Assign a linestyle in place of the one passed to the callback.
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (int32_t newVal);
    
    //! Change the linestyle params.  If NULL is passed in, then the ACTIVE linestyle params are used.
    //! NOTE: You must set the line style using SetStoredValue() before setting the parameters.
    DGNPLATFORM_EXPORT StatusInt SetParams (LineStyleParams const* newParams); // NULL means use ACTIVE! Must set line code first!

    //! Explicitly force the parameters to be rewritten by marking them as changed.  Normally this is done automatically by the system when changing linestyle parameters. 
    DGNPLATFORM_EXPORT StatusInt SetParamsChanged ();
};

//=======================================================================================
//! Element font information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachFontArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:
    uint32_t m_storedValue;

public:
    DGNPLATFORM_EXPORT EachFontArg (uint32_t stored, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the value of the property that is stored in the element.
    DGNPLATFORM_EXPORT uint32_t GetStoredValue ();

    //! Assign a new value in place of the one passed to the callback.
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (uint32_t newVal);
};

//=======================================================================================
//! Element text style information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachTextStyleArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:
    uint32_t                    m_storedValue;
    StyleParamsRemapping        m_paramsRemapping;

public:
    DGNPLATFORM_EXPORT EachTextStyleArg (uint32_t stored, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the remapping action the will be applied to the element.
    //! Ignored for IQueryProperties.
    DGNPLATFORM_EXPORT StyleParamsRemapping GetRemappingAction ();

    //! Get the value of the property that is stored in the element.
    DGNPLATFORM_EXPORT uint32_t GetStoredValue ();

    //! Assign a new value in place of the one passed to the callback.
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (uint32_t newVal);

    //! Assign an action to be applied to the element related to this style.
    //! Ignored for IQueryProperties.
    DGNPLATFORM_EXPORT StatusInt SetRemappingAction (StyleParamsRemapping);
};

//=======================================================================================
//! Element dimension style information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachDimStyleArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:
    uint64_t                 m_storedValue;
    StyleParamsRemapping        m_paramsRemapping;

public:
    DGNPLATFORM_EXPORT EachDimStyleArg (uint64_t stored, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the remapping action the will be applied to the element.
    //! Ignored for IQueryProperties.
    DGNPLATFORM_EXPORT StyleParamsRemapping GetRemappingAction ();
    DGNPLATFORM_EXPORT uint64_t GetStoredValue ();
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (uint64_t newVal);
    DGNPLATFORM_EXPORT StatusInt SetRemappingAction (StyleParamsRemapping);
};

//=======================================================================================
//! Element mulitline style information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachMLineStyleArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:
    uint64_t                 m_storedValue;
    StyleParamsRemapping        m_paramsRemapping;

public:
    DGNPLATFORM_EXPORT EachMLineStyleArg (uint64_t stored, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the remapping action the will be applied to the element.
    //! Ignored for IQueryProperties.
    DGNPLATFORM_EXPORT StyleParamsRemapping GetRemappingAction ();

    //! Get the value of the property that is stored in the element.
    DGNPLATFORM_EXPORT uint64_t GetStoredValue ();

    //! Assign a new value in place of the one passed to the callback.
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (uint64_t newVal);

    //! Assign an action to be applied to the element related to this style.
    //! Ignored for IQueryProperties.
    DGNPLATFORM_EXPORT StatusInt SetRemappingAction (StyleParamsRemapping);
};

//=======================================================================================
//! Element material information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachMaterialArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:
    DgnMaterialId                   m_storedValue;
    WString                     m_subEntity;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT EachMaterialArg (DgnMaterialId stored, PropsCallbackFlags flags, PropertyContextR);
    DGNPLATFORM_EXPORT EachMaterialArg (DgnMaterialId stored, WCharCP subEntity, PropsCallbackFlags flags, PropertyContextR);

    DGNPLATFORM_EXPORT DgnMaterialId GetStoredValue ();
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (DgnMaterialId newVal);
    DGNPLATFORM_EXPORT WCharCP GetSubEntity ();
};

//=======================================================================================
//! Element weight information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachWeightArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:
    uint32_t m_storedValue;

public:
    DGNPLATFORM_EXPORT EachWeightArg (uint32_t stored, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the value of the property that is stored in the element.
    DGNPLATFORM_EXPORT uint32_t GetStoredValue ();

    //! Get the effective value of the property that is used for display.  This is computed from
    //! the stored value considering header overrides.  If the stored value is BYCELL or BYLEVEL,
    //! the effective value will resolve the value from the cell or level respectively.
    DGNPLATFORM_EXPORT uint32_t GetEffectiveValue ();

    //! Assign a new value in place of the one passed to the callback.
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (uint32_t newVal);
};

//=======================================================================================
//! Element transparency information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachTransparencyArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:
        double  m_storedValue;

public:
    DGNPLATFORM_EXPORT EachTransparencyArg (double stored, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the value of the property that is stored in the element.
    DGNPLATFORM_EXPORT double GetStoredValue ();

    //! Assign a new value in place of the one passed to the callback.
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (double newVal);
};

//=======================================================================================
//! Element display priority information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachDisplayPriorityArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:

int32_t                     m_storedValue;

public:
    DGNPLATFORM_EXPORT EachDisplayPriorityArg (int32_t stored, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the value of the property that is stored in the element.
DGNPLATFORM_EXPORT int32_t GetStoredValue ();

    //! Get the effective value of the property that is used for display.  This is computed from
    //! the stored value considering header overrides.  If the stored value is BYCELL,
    //! the effective value will resolve the value from the cell.
DGNPLATFORM_EXPORT int32_t GetEffectiveValue ();

    //! Assign a new value in place of the one passed to the callback.
DGNPLATFORM_EXPORT StatusInt SetStoredValue (int32_t newVal);
};

//=======================================================================================
//! Element template information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachElementTemplateArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:

    DgnElementId                m_storedValue;
    bool                        m_applyDefaultSymb;

public:
    DGNPLATFORM_EXPORT EachElementTemplateArg (DgnElementId stored, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! If true, the template's default symbology will be applied to the element.
    DGNPLATFORM_EXPORT bool GetApplyDefaultSymbology ();

    //! Get the value of the property that is stored in the element.
    DGNPLATFORM_EXPORT DgnElementId GetStoredValue ();

    //! Assign a new value in place of the one passed to the callback.
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (DgnElementId newVal);

    //! Set to true to apply the template's default symbology to the element.
    DGNPLATFORM_EXPORT StatusInt SetApplyDefaultSymbology (bool);
};

//=======================================================================================
//! Element thickness information supplied to "Each" callback.
//! @note Set methods ignored when using IQueryProperties. 
// @bsiclass 
//=======================================================================================
class EachThicknessArg : public EachPropertyBaseArg
{
//__PUBLISH_SECTION_END__
private:

    double                      m_storedValue;
    bool                        m_capped;
    bool                        m_haveDirection;
    bool                        m_alwaysUseDirection;
    DVec3d                      m_direction;

public:
    DGNPLATFORM_EXPORT EachThicknessArg (double stored, DVec3dP direction, bool capped, bool alwaysUseDirection, PropsCallbackFlags flags, PropertyContext&);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

    //! Get the capped flag.  If true a closed profile with thickness will be drawn as a capped
    //! solid, else it will draw as an uncapped surface.
    DGNPLATFORM_EXPORT bool GetCapped ();

    //! Set to true to draw a thickened closed profile as a capped solid or false for to draw
    //! as an uncapped surface.
    DGNPLATFORM_EXPORT StatusInt SetCapped (bool capped);

    //! Get a flag used to interpret the direction vector.  If true, the direction vector
    //! will be used even if the element has a well defined normal.
    DGNPLATFORM_EXPORT bool GetAlwaysUseDirection ();

    //! Set to true to use the direction vector even if the element has a well defined normal.
    DGNPLATFORM_EXPORT StatusInt SetAlwaysUseDirection (bool alwaysUseDirection);

    //! Get the vector along which to extrude the element.
    DGNPLATFORM_EXPORT StatusInt GetDirection (DVec3dR);

    //! Set the vector along which to extrude the element.
    DGNPLATFORM_EXPORT StatusInt SetDirection (DVec3dCP);

    //! Get the value of the property that is stored in the element.
    DGNPLATFORM_EXPORT double GetStoredValue ();

    //! Assign a new value in place of the one passed to the callback.
    DGNPLATFORM_EXPORT StatusInt SetStoredValue (double newVal);
};

//=======================================================================================
//! Interface for property callbacks used for both query and edit.
// @bsiclass 
//=======================================================================================
struct  IProcessProperties
{
    //! Called by the element handler to report levels stored on an element.
    virtual void _EachCategoryCallback (EachCategoryArg&) {}

    //! Called by the element handler to report colors stored on an element.
    virtual void _EachColorCallback (EachColorArg&) {}

    //! Called by the element handler to report line styles stored on an element.
    virtual void _EachLineStyleCallback (EachLineStyleArg&) {}

    //! Called by the element handler to report fonts stored on an element.
    virtual void _EachFontCallback (EachFontArg&) {}

    //! Called by the element handler to report text styles stored on an element.
    virtual void _EachTextStyleCallback (EachTextStyleArg&) {}

    //! Called by the element handler to report dimension styles stored on an element.
    virtual void _EachDimStyleCallback (EachDimStyleArg&) {}

    //! Called by the element handler to report multiline styles stored on an element.
    virtual void _EachMLineStyleCallback (EachMLineStyleArg&) {}

    //! Called by the element handler to report materials stored on an element.
    virtual void _EachMaterialCallback (EachMaterialArg&) {}

    //! Called by the element handler to report weights stored on an element.
    virtual void _EachWeightCallback (EachWeightArg&) {}

    //! Called by the element handler to report transparencies stored on an element.
    virtual void _EachTransparencyCallback (EachTransparencyArg&) {}

    //! Called by the element handler to report thicknesses stored on an element.
    virtual void _EachThicknessCallback (EachThicknessArg&) {}

    //! Called by the element handler to report display priorities stored on an element.
    virtual void _EachDisplayPriorityCallback (EachDisplayPriorityArg&) {}

    //! Called by the element handler to report element templates stored on an element.
    virtual void _EachElementTemplateCallback (EachElementTemplateArg&) {}
};

//=======================================================================================
//! Interface for inspecting element property values. The element handler will announce
//! the requested property types to the "Each" callbacks in the IProcessProperties interface.
// @bsiclass 
//=======================================================================================
struct  IQueryProperties : public IProcessProperties
{
//! Called by the element handler to find out which property types are of interest.
//! @return Mask of ElementProperties values.
//! @note  The IQueryProperties object is free to ignore those property types it's not
//!        interested in by not overriding that Each callback. This mask is a performance
//!        optimization.  By announcing the specific property types of interest, you
//!        allow the element handler to be more efficient.
//! @bsimethod
virtual ElementProperties _GetQueryPropertiesMask () {return ELEMENT_PROPERTY_All;}

//! Return purpose for which properties are being requested. The element handler may
//! look at this to decide what properties it thinks are important to the caller.
//! @return Reason why properties are being requested QueryPropertyPurpose.
//! @bsimethod
virtual QueryPropertyPurpose _GetQueryPropertiesPurpose () {return QueryPropertyPurpose::NotSpecified;}

//! Whether to include shared children in property query.
//! @return true to iterate over "shared" children (e.g. components of the definition of a shared cell instance.)
//! @bsimethod
virtual bool _WantSharedChildren () {return false;}

}; // IQueryProperties

//=======================================================================================
//! Interface for changing element property values. The element handler will announce
//! the requested property types to the "Each" callbacks in the IProcessProperties interface.
// @bsiclass 
//=======================================================================================
struct  IEditProperties : public IProcessProperties
{
//! Called by the element handler to find out which property types are of interest.
//! @return Mask of ElementProperties values.
//! @note  The IEditProperties object is free to ignore those property types it's not
//!        interested in by not overriding that Each callback. This mask is a performance
//!        optimization.  By announcing the specific property types of interest, you
//!        allow the element handler to be more efficient.
//! @bsimethod
virtual ElementProperties _GetEditPropertiesMask () {return ELEMENT_PROPERTY_All;}

//! Return purpose for which properties are being requested. The element handler may
//! look at this to decide what properties it thinks are important to the caller.
//! @return Reason why properties are being requested QueryPropertyPurpose.
//! @bsimethod
virtual EditPropertyPurpose _GetEditPropertiesPurpose () {return EditPropertyPurpose::Change;}

//! Destination model used when property values are being remapped when copying elements 
//! between models.
//! @return Destination model for ids when purpose is EditPropertyPurpose::Remap.
virtual DgnModelP _GetDestinationDgnModel () {return NULL;}

}; // IEditProperties

//=======================================================================================
//! Class for enumerating the common properties of elements such as color and level
//! and changing those properties. The property context holds a pointer to either a
//! IQueryProperties or IEditProperties object to which each property value is announced
//! through callbacks. The property context also holds a state for each element or
//! component element being processed so that the query/edit object can request
//! the effective property value for BYLEVEL properties and also take header overrides 
//! into account.
//=======================================================================================
struct PropertyContext
{
//__PUBLISH_SECTION_END__
private:

IQueryProperties*       m_queryObj;
IEditProperties*        m_editObj;

ElementHandleCP         m_elmHandle;
HitPathCP               m_hitPath;
bool                    m_elementChanged;
DgnCategoryId           m_currentCategory; // Needed for effective values...

public:

struct ContextMark
{
    PropertyContextP    m_context;
    int                 m_hdrOvrMark;

public:
    DGNPLATFORM_EXPORT explicit ContextMark (PropertyContextP context);
    DGNPLATFORM_EXPORT ContextMark (PropertyContextR context);
    DGNPLATFORM_EXPORT ~ContextMark ();

    DGNPLATFORM_EXPORT void Pop ();
    DGNPLATFORM_EXPORT void SetNow ();
    void Init (PropertyContextP context) {m_hdrOvrMark = 0; m_context = context;}
 };

DGNPLATFORM_EXPORT PropertyContext (IQueryProperties*);
DGNPLATFORM_EXPORT PropertyContext (IEditProperties*);
bool GetElementChanged () {return m_elementChanged;}
void SetElementChanged () {m_elementChanged = true;}

DGNPLATFORM_EXPORT DgnModelP GetSourceDgnModel ();
DGNPLATFORM_EXPORT DgnModelP GetDestinationDgnModel ();
DGNPLATFORM_EXPORT ElementProperties GetElementPropertiesMask ();

IQueryProperties* GetIQueryPropertiesP () {return m_queryObj;}
void SetIQueryPropertiesP (IQueryProperties* queryObj) {m_queryObj = queryObj;}

IEditProperties* GetIEditPropertiesP () {return m_editObj;}
void SetIEditPropertiesP (IEditProperties* editObj) {m_editObj = editObj;}

ElementHandleCP GetCurrentElemHandleP () {return m_elmHandle;}
DGNPLATFORM_EXPORT void SetCurrentElemHandleP (ElementHandleCP eh);

HitPathCP GetQueryPath () {return m_hitPath;}
void SetQueryPath (HitPathCP path) {m_hitPath = path;}

DgnCategoryId GetCurrentCategoryID () {return m_currentCategory;}
void SetCurrentCategoryID (DgnCategoryId level) {m_currentCategory = level;}

DGNPLATFORM_EXPORT bool DoColorCallback (uint32_t* pNewColor, EachColorArg& arg);
DGNPLATFORM_EXPORT bool DoColorCallback (uint32_t* pNewColor, EachColorArg const& arg) {return DoColorCallback (pNewColor, const_cast<EachColorArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoCategoryCallback (DgnCategoryId* pNewCategoryID, EachCategoryArg& arg);
DGNPLATFORM_EXPORT bool DoCategoryCallback (DgnCategoryId* pNewCategoryID, EachCategoryArg const& arg) {return DoCategoryCallback (pNewCategoryID, const_cast<EachCategoryArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoLineStyleCallback (int32_t* pNewStyleID, EachLineStyleArg& arg);
DGNPLATFORM_EXPORT bool DoLineStyleCallback (int32_t* pNewStyleID, EachLineStyleArg const& arg) {return DoLineStyleCallback (pNewStyleID, const_cast<EachLineStyleArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoFontCallback (uint32_t* pNewFontNo, EachFontArg& arg);
DGNPLATFORM_EXPORT bool DoFontCallback (uint32_t* pNewFontNo, EachFontArg const& arg) {return DoFontCallback (pNewFontNo, const_cast<EachFontArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoDimStyleCallback (uint64_t* pNewStyleID, EachDimStyleArg& arg);
DGNPLATFORM_EXPORT bool DoDimStyleCallback (uint64_t* pNewStyleID, EachDimStyleArg const& arg) {return DoDimStyleCallback (pNewStyleID, const_cast<EachDimStyleArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoMLineStyleCallback (DgnElementId* pNewStyleID, EachMLineStyleArg& arg);
DGNPLATFORM_EXPORT bool DoMLineStyleCallback (DgnElementId* pNewStyleID, EachMLineStyleArg const& arg) {return DoMLineStyleCallback (pNewStyleID, const_cast<EachMLineStyleArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoMaterialCallback (DgnMaterialId* pNewID, EachMaterialArg& arg);
DGNPLATFORM_EXPORT bool DoMaterialCallback (DgnMaterialId* pNewID, EachMaterialArg const& arg) {return DoMaterialCallback (pNewID, const_cast<EachMaterialArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoWeightCallback (uint32_t* pNewVal, EachWeightArg& arg);
DGNPLATFORM_EXPORT bool DoWeightCallback (uint32_t* pNewVal, EachWeightArg const& arg) {return DoWeightCallback (pNewVal, const_cast<EachWeightArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoTransparencyCallback (double* pNewVal, EachTransparencyArg& arg);
DGNPLATFORM_EXPORT bool DoTransparencyCallback (double* pNewVal, EachTransparencyArg const& arg) {return DoTransparencyCallback (pNewVal, const_cast<EachTransparencyArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoThicknessCallback (double* pNewVal, EachThicknessArg& arg);
DGNPLATFORM_EXPORT bool DoThicknessCallback (double* pNewVal, EachThicknessArg const& arg) {return DoThicknessCallback (pNewVal, const_cast<EachThicknessArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoDisplayPriorityCallback (int32_t* pNewVal, EachDisplayPriorityArg& arg);
DGNPLATFORM_EXPORT bool DoDisplayPriorityCallback (int32_t* pNewVal, EachDisplayPriorityArg const& arg) {return DoDisplayPriorityCallback (pNewVal, const_cast<EachDisplayPriorityArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object
DGNPLATFORM_EXPORT bool DoElementTemplateCallback (DgnElementId* pNewVal, EachElementTemplateArg& arg);
DGNPLATFORM_EXPORT bool DoElementTemplateCallback (DgnElementId* pNewVal, EachElementTemplateArg const& arg) {return DoElementTemplateCallback (pNewVal, const_cast<EachElementTemplateArg&>(arg));} // WIP_NONPORT - don't pass non-const ref to temporary object

//! Calls PropertyContext::QueryPathProperties. Property values supplied to "Each" callback have been cloned to ElementCopyContextP->GetDestinationDgnModel ().
//DGNPLATFORM_EXPORT static void QueryClonedPathProperties (HitPathCR path, IQueryProperties& queryObj, ElementCopyContextP ccP = NULL); removed in graphite

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Query the element properties specific to the location specified by the input hit path.
//! @param[in]    path        Query the properties of the component at the hit path's cursor index.
//! @param[in]    queryObj    Property callbacks will be sent to this object.
//! @DotNetMethodExclude
//! @bsimethod
DGNPLATFORM_EXPORT static void QueryPathProperties (HitPathCP path, IQueryProperties* queryObj);

//! Query the element properties for the specified element.
//! @param[in]    eh          Query the properties of this element.
//! @param[in]    queryObj    Property callbacks will be sent to this object.
//! @note Also queries the properties of public children.
//! @bsimethod
DGNPLATFORM_EXPORT static void QueryElementProperties (ElementHandleCR eh, IQueryProperties* queryObj);

//! Edit the element properties for the specified element.
//! @param[out]   eeh         Edit the properties of this element.
//! @param[in]    editObj     Property callbacks will be sent to this object.
//! @return true if the element was changed.
//! @note Also edits the properties of public children.
//! @bsimethod
DGNPLATFORM_EXPORT static bool EditElementProperties (EditElementHandleR eeh, IEditProperties* editObj);

}; // PropertyContext

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
