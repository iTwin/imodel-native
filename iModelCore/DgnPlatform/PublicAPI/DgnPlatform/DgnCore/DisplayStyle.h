/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DisplayStyle.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../DgnPlatform.h"
#include "../Tools/BitMask.h"
#include "DgnCore.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup DisplayStyles
//! @brief A display style is a set of display overrides that can be applied to achieve different display modes and effects. It can be applied to the entire view, or to clip sections in a view.

//! @beginGroup

//! Smart pointer wrapper for DisplayStyle.
typedef RefCountedPtr<DisplayStyle> DisplayStylePtr;

//=======================================================================================
//! Lists the possible intended usages of a display style.
//! Display styles have an intrinsic 'usage' property which describes their intended use. This is taken into account when displaying lists of display styles in the user interface, depending on the list's or picker's purpose. A display style can have zero or many usages specified.
// @bsiclass
//=======================================================================================
enum class DisplayStyleBuiltInUsage
    {
    View       = 0,    //!< The display style is meant to be used for entire views
    ClipVolume = 1     //!< The display style is meant to be used for sections of clip volumes
    
    }; // DisplayStyleBuiltInUsage

//=======================================================================================
//! Describes if a display style can be applied to a view, and if not, why.
// @bsiclass
//=======================================================================================
enum class DisplayStyleApplyValidity
    {
    CanBeApplied,         //!< Display style can be applied to the view
    NonWireframeForSheet, //!< Display style's display mode is non-wireframe, and view is a sheet
    ShadowedOn2DModel,    //!< Display style has shadows enabled, and view is a sheet
    NotValidForHandler,   //! Display style handler not valid for this view
    
    }; // DisplayStyleApplyValidity

//=======================================================================================
//! Describes where the display style came from.
// @bsiclass
//=======================================================================================
enum class DisplayStyleSource
    {
    File,            //!< From a file (active or DGNLIB)
    HardCodedDefault //!< A hard-coded fallback default
    
    }; // DisplayStyleSource

//=======================================================================================
//! Lists the (fake) indices of special display styles.
//! There are several 'special' display styles (which aren't actually display styles stored in the file), generally for use within pickers and lists. This enumerates the special indices used to identify them.
// @bsiclass
//=======================================================================================
enum SpecialDisplayStyleIndex ENUM_UNDERLYING_TYPE(int)
    {
    SpecialDisplayStyleIndex_None           = -1,   //!< Specifies no display style should be used (note that 0 is the first valid display style)
    SpecialDisplayStyleIndex_FromParent     = -2    //!< For references, specifies to use the parent's display style
    
    }; // SpecialDisplayStyleIndex

/*=================================================================================**//**
* When display environment is checked descibes what type of environment is displayed
* @bsistruct
+===============+===============+===============+===============+===============+======*/
enum class EnvironmentDisplay
    {
    Color        = 0,
    Luxology     = 1,
    };

//=======================================================================================
//! This sub-structure describes the 'flags' part of a DisplayStyle.
//! The display-relevant data stored in a display style is separated into several groups. The 'flags' are generally control bits that describe the underlying rendering parameters. Note that not all flags are valid for every display mode.
//! @note This structure is not intended to be used directly.
// @bsiclass
//=======================================================================================
struct DisplayStyleFlags
    {
    public: unsigned    m_displayMode               :6;     //!< The rendering technique; valid values are MSRenderMode::Wireframe, MSRenderMode::HiddenLine, MSRenderMode::SolidFill, or MSRenderMode::SmoothShade
    public: unsigned    m_displayVisibleEdges       :1;     //!< Toggles the display of visible edges
    public: unsigned    m_displayHiddenEdges        :1;     //!< Toggles the display of hidden edges
    public: unsigned    m_hiddenEdgeLineStyle       :3;     //!< Controls the line style of hidden edges; valid values are 0-7 inclusive
    public: unsigned    m_displayShadows            :1;     //!< Toggles the diaplsy of shadows
    public: unsigned    m_legacyDrawOrder           :1;     //!< Enables 'legacy draw' (or 'file') order; means z-position is based on file position
    public: unsigned    m_overrideBackgroundColor   :1;     //!< Enables or disables the override of the background color
    public: unsigned    m_applyEdgeStyleToLines     :1;     //!< 8.11.7: applies edges style to wires / open elements
    public: unsigned    m_ignoreGeometryMaps        :1;     //!< 8.11.9 - Ignore geometry maps.
    public: unsigned    m_ignoreImageMaps           :1;     //!< 8.11.9 - Ignore image (Pattern) maps - shaded only.
    public: unsigned    m_hideInPickers             :1;     //!< 8.11.9 - Hide in pickers (but remain in display style dialog for editing, and available through API).
    public: unsigned    m_invisibleToCamera         :1;     //!< 8.11.9 - Make geometry invisible to camera, but still cast shadows and appear in reflections.  Luxology only.
    public: unsigned    m_displayGroundPlane        :1;     //!< 8.11.9  display a ground plane

    unsigned            m_unused                    :12;

    public: DGNPLATFORM_EXPORT  DisplayStyleFlags   ();
    public: DGNPLATFORM_EXPORT  ~DisplayStyleFlags  ();

    //! Performs a deep by-value equality check with another instance.
    public: DGNPLATFORM_EXPORT bool Equals (DisplayStyleFlagsCR rhs) const;

    }; // DisplayStyleFlags

#if defined (NEEDS_WORK_DGNITEM)
//=======================================================================================
//! This sub-structure describes the 'overrides' part of a DisplayStyle.
//! The display-relevant data stored in a display style is separated into several groups. The 'overrides' describe the color/style/weight that a display style applies to its underlying rendering parameters ('flags'). <br>
//! <br>
//! This structure is file-depedent, meaning that it stores indices and not fully qualified values. It itself does not store a reference to a file (since DisplayStyle does, and this structure should only exist in the context of a DisplayStyle), and any method that could depend on resolved values accepts file references as parameter(s).
//! @note This structure is not intended to be used directly.
// @bsiclass
//=======================================================================================
struct ViewDisplayOverrides
    {
    struct  OverrideFlags
        {
        public: unsigned    m_visibleEdgeColor    :1;   //!< Enables the override of edge color (not just visible anymore -- must defer name change at this time)
        public: unsigned    m_visibleEdgeWeight   :1;   //!< Enables the override of visible edge weight
        public: unsigned    m_useTransparency     :1;   //!< Enables the override of element transparency
        public: unsigned    m_elementColor        :1;   //!< Enables the override of element color
        public: unsigned    m_lineStyle           :1;   //!< Enables the override of element line style
        public: unsigned    m_lineWeight          :1;   //!< Enables the override of element line weight
        public: unsigned    m_material            :1;   //!< Enables the override of element material
        public: unsigned    m_visibleEdgeStyle    :1;   //!< 8.11.7: Enables the override of visible edge style
        public: unsigned    m_hiddenEdgeStyle     :1;   //!< 8.11.7: Enables the override of hidden edge style
        public: unsigned    m_hiddenEdgeWeightZero:1;   //!< 8.11.7: Enables the override of hidden edge weight
        public: unsigned    m_hLineTransparency   :1;   //!< 8.11.9: Disables the override of transparency threshold (hidden line views).
        public: unsigned    m_hLineMaterialColors :1;   //!< 8.11.9: Disables the override of transparency threshold (hidden line views).
        public: unsigned    m_smoothIgnoreLights  :1;   //!< 8.11.9: Ignore lighting for smooth shading.
        public: unsigned    m_useDisplayHandler   :1;   //!< 8.11.9: Use Display Handler.
        public: unsigned    m_unused             :18;
        };

    public: OverrideFlags   m_flags;
    public: UInt32          m_visibleEdgeColor;         //!< Override edge color (not just visible anymore -- must defer name change at this time)
    public: UInt32          m_visibleEdgeWeight;        //!< Override visible edge weight
    public: UInt32          m_hiddenEdgeWeight;         //!< override hidden edge weight (0xffff == same as visible).
    public: double          m_transparency;             //!< Override element transparency (0..1 inclusive)
    public: UInt32          m_elementColor;             //!< Override element color
    public: UInt32          m_lineStyle;                //!< Override element line style
    public: UInt32          m_lineWeight;               //!< Override element line weight
    public: ElementId       m_material;                 //!< Override element material
    public: UInt32          m_backgroundColor;          //!< Override view background color

    public: double          m_hLineTransparencyThreshold;

    enum
        {
        HiddenEdgeWeight_SameAsVisible = 0xffff
        };

//__PUBLISH_SECTION_END__

    private:    XAttributeHandlerId                         m_displayStyleHandlerId;
    private:    mutable struct DisplayStyleHandler const*   m_displayStyleHandler;
    private:    mutable DisplayStyleHandlerSettingsPtr      m_displayStyleHandlerSettings;

    public:                 XAttributeHandlerId             GetDisplayStyleHandlerId            () const { return m_displayStyleHandlerId; }
    public: DGNPLATFORM_EXPORT  DisplayStyleHandlerSettingsPtr  GetDisplayStyleHandlerSettingsPtr   () const;
    public:                 DisplayStyleHandler const*      GetDisplayStyleHandlerCP            () const { return m_displayStyleHandler; }
    public:                 void                            SetDisplayStyleHandlerById          (XAttributeHandlerId id) { m_displayStyleHandlerId = id; ResolveDisplayStyleHandler (); }
    public:                 void                            SetDisplayStyleHandlerP             (struct DisplayStyleHandler const* handler) { m_displayStyleHandler = handler; }
    public: DGNPLATFORM_EXPORT  void                            SetDisplayStyleHandlerSettingsPtr   (RefCountedPtr<struct DisplayStyleHandlerSettings> settings);
    public: DGNPLATFORM_EXPORT  void                            ResolveDisplayStyleHandler          () const;

    //! Performs a deep copy, and supports remapping IDs between two projects.
    public: DGNPLATFORM_EXPORT ViewDisplayOverrides Clone (DgnProjectR sourceProject, DgnProjectR destinationProject) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    public: DGNPLATFORM_EXPORT  ViewDisplayOverrides    ();
    public: DGNPLATFORM_EXPORT  ~ViewDisplayOverrides   ();

    //! Performs a deep by-value equality check between this object and another.
    //! @note This overload is file-invariant, meaning that color and material indices will NOT be remapped to equivalent values if the objects are from different files.
    public: DGNPLATFORM_EXPORT bool Equals (ViewDisplayOverridesCR) const;

    /*---------------------------------------------------------------------------------**//**
    * Return true if hidden edge weight should be same as visible.
    +---------------+---------------+---------------+---------------+---------------+------*/
    public:   bool HiddenEdgeWeightSameAsVisible () const { return HiddenEdgeWeight_SameAsVisible == m_hiddenEdgeWeight; }

    }; // ViewDisplayOverrides
#endif

//=======================================================================================
//! This sub-structure describes the 'ground plane' part of a display style, controlling its visual properties such as height and color.
// @bsiclass
//=======================================================================================
struct DisplayStyleGroundPlane
    {
    friend struct DisplayStyle;

    private:    RgbFactor   m_color;
    private:    double      m_height;               // Stored in meters
    private:    double      m_transparency;
    private:    bool        m_showGroundFromBelow;

    public: DGNPLATFORM_EXPORT  RgbFactor const&    GetGroundColor          () const;
    public: DGNPLATFORM_EXPORT  void                SetGroundColor          (RgbFactor const& color);
    public: DGNPLATFORM_EXPORT  double              GetHeight               () const;
    public: DGNPLATFORM_EXPORT  void                SetHeight               (double height);
    public: DGNPLATFORM_EXPORT  double              GetTransparency         () const;
    public: DGNPLATFORM_EXPORT  void                SetTransparency         (double transparency);
    public: DGNPLATFORM_EXPORT  bool                ShowGroundFromBelow     () const;
    public: DGNPLATFORM_EXPORT  void                SetShowGroundFromBelow  (bool showGroundFromBelow);

    public: DGNPLATFORM_EXPORT                      DisplayStyleGroundPlane ();

    public: DGNPLATFORM_EXPORT  void                Clone                   (DisplayStyleGroundPlane const& rhs);
    public: DGNPLATFORM_EXPORT  bool                Equals                  (DisplayStyleGroundPlane const& rhs) const;

    }; // DisplayStyleGroundPlane

typedef DisplayStyleGroundPlane*        DisplayStyleGroundPlaneP;
typedef DisplayStyleGroundPlane const*  DisplayStyleGroundPlaneCP;
typedef DisplayStyleGroundPlane&        DisplayStyleGroundPlaneR;
typedef DisplayStyleGroundPlane const&  DisplayStyleGroundPlaneCR;

#if defined (NEEDS_WORK_DGNITEM)
//=======================================================================================
//! A display style is a collection of view-related settings that can be applied to the whole or part of a view.
//! @note Please see additional documentation for the Display Styles module; it contains a lot of general background information and tips for display styles.
//! @note When writing a display style to a file, if you wish to then use its ID, you must re-read the display style from the file and get its ID. When writing to the file, the display style you provide is cloned and configured for the file internally; thus your copy remains unaltered, including its potentially unusable/invalid ID.
//! @note Some display style operations require DGN library support; these operations are only available on the PowerPlatform.
//! @note You must use one of the static Create factory methods to create instances of this class.
//! @see DgnStyles for managing display styles in files
// @bsiclass
//=======================================================================================
struct DisplayStyle : public RefCountedBase
    {
//__PUBLISH_SECTION_END__

    private:    DgnStyleId              m_id;
    private:    WString                 m_name;
    private:    DisplayStyleSource      m_source;
    private:    DgnProjectR             m_project;

    private:    DisplayStyleFlags       m_flags;
    private:    ViewDisplayOverrides    m_overrides;
    private:    BitMaskHolder           m_usages;
    private:    EnvironmentDisplay      m_environmentTypeDisplayed;
    private:    WString                 m_environmentName;
    private:    DisplayStyleGroundPlane m_groundPlane;

    private:    DGNPLATFORM_EXPORT                              DisplayStyle                (WCharCP name, DgnProjectR);

    public:     DGNPLATFORM_EXPORT      static DisplayStylePtr  CreateFromXMLString         (WCharCP name, Utf8CP, DgnProjectR);
    public:     DGNPLATFORM_EXPORT      static DisplayStylePtr  CreateHardCodedDefault (MSRenderMode displayMode, DgnProjectR);
    public:     DGNPLATFORM_EXPORT      Utf8String              CreateXMLString             () const;

    public:     DGNPLATFORM_EXPORT      void                    SetId                       (DgnStyleId);
    public:     /* DGNCORE */           DisplayStyleSource      GetSource                   () const;
    public:     DGNPLATFORM_EXPORT      void                    SetSource                   (DisplayStyleSource);
    public:     /* DGNCORE */           BitMask const&          GetUsages                   () const;
    public:     /* DGNCORE */           BitMask&                GetUsagesR                  ();
    public:     /* DGNCORE */           void                    SetUsages                   (BitMask const&);
    public:     /* DGNCORE */           void                    ApplyTo                     (ViewFlagsR) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Creates a new display style with the given name based on the given file. All flags are zeroed, no overrides are enabled, and the display style source is from-file.
    public: DGNPLATFORM_EXPORT static DisplayStylePtr Create (WCharCP name, DgnProjectR);

    //! Makes a deep copy of a display style.
    //! @note This overload does not remap any IDs, and keeps the same file as the original.
    public: DGNPLATFORM_EXPORT DisplayStylePtr Clone () const;

    //! Makes a deep copy of a display style.
    //! @note This overload allows ID remapping to a new destination project.
    public: DGNPLATFORM_EXPORT DisplayStylePtr Clone (DgnProjectR destinationProject) const;

    //! Performs a deep equality check.
    //! @note Enable testSettingsOnly to ignore differences in display style ID, source, and file.
    public: DGNPLATFORM_EXPORT bool Equals (DisplayStyleCR, bool testSettingsOnly) const;

    //! Gets the file that this display style is based on; useful for resolving the effective value of internal IDs.
    public: DGNPLATFORM_EXPORT DgnProjectR GetProjectR () const;

    //! Gets the ID of this display style. Display styles do not have a valid ID until they are written to the file.
    //! @note When writing a display style to a file, make sure you re-read the display style (by-name) to get its persisted ID.
    public: DGNPLATFORM_EXPORT DgnStyleId GetId () const;

    //! Gets the name.
    public: DGNPLATFORM_EXPORT WStringCR GetName () const;

    //! Sets the name.
    public: DGNPLATFORM_EXPORT void SetName (WCharCP);

    //! True if this display style originated from a file.
    public: DGNPLATFORM_EXPORT bool IsFromFile () const;

    //! True if this display style originated from a hard-coded default.
    public: DGNPLATFORM_EXPORT bool IsFromHardCodedDefault () const;

    //! Gets a read-only reference to the flags (@see DisplayStyleFlags).
    public: DGNPLATFORM_EXPORT DisplayStyleFlagsCR GetFlags () const;

    //! Gets a read/write reference to the flags (@see DisplayStyleFlags).
    public: DGNPLATFORM_EXPORT DisplayStyleFlagsR GetFlagsR ();

    //! Sets the flags (@see DisplayStyleFlags).
    public: DGNPLATFORM_EXPORT void SetFlags (DisplayStyleFlagsCR);

    //! Gets a read-only reference to the overrides (@see ViewDisplayOverrides).
    public: DGNPLATFORM_EXPORT ViewDisplayOverridesCR GetOverrides () const;

    //! Gets a read/write reference to the overrides (@see ViewDisplayOverrides).
    public: DGNPLATFORM_EXPORT ViewDisplayOverridesR GetOverridesR ();

    //! Sets the overrides (@see ViewDisplayOverrides).
    public: DGNPLATFORM_EXPORT void SetOverrides (ViewDisplayOverridesCR);

    //! True if this display style can be used for whole views.
    public: DGNPLATFORM_EXPORT bool IsUsableForViews () const;

    //! Sets if this display style can be used for whole views.
    public: DGNPLATFORM_EXPORT void SetIsUsableForViews (bool);

    //! True if this display style can be used for individual sections.
    public: DGNPLATFORM_EXPORT bool IsUsableForClipVolumes () const;

    //! Sets if this display style can be used for individual sections.
    public: DGNPLATFORM_EXPORT void SetIsUsableForClipVolumes (bool);

    //! True if this display style can be applied to the provided viewport.
    public: DGNPLATFORM_EXPORT bool IsValidForViewport (ViewportCR viewport, DisplayStyleApplyValidity* applyValidity) const;

    //! Applies this display style to the given viewport.
    public: DGNPLATFORM_EXPORT void ApplyTo (ViewportR) const;

    //! Applies this display style to the given view info.
    public: DGNPLATFORM_EXPORT void ApplyTo (PhysicalViewControllerR) const;

    public: DGNPLATFORM_EXPORT WStringCR GetEnvironmentName () const;
    public: DGNPLATFORM_EXPORT void SetEnvironmentName (WCharCP environmentName);
    public: DGNPLATFORM_EXPORT EnvironmentDisplay GetEnvironmentTypeDisplayed () const;
    public: DGNPLATFORM_EXPORT void SetEnvironmentTypeDisplayed (EnvironmentDisplay typeDisplayed);
    public: DGNPLATFORM_EXPORT DisplayStyleGroundPlaneCR GetGroundPlane () const;
    public: DGNPLATFORM_EXPORT DisplayStyleGroundPlaneP GetGroundPlaneP ();

    }; // DisplayStyle

//=======================================================================================
//! Groups methods for Display Styles.
// @bsiclass
//=======================================================================================
struct DgnDisplayStyles
{
private:
    friend struct DgnStyles;

    DgnProjectR m_project;

    //! Only the outer class is designed to construct this class.
    DgnDisplayStyles(DgnProject& project) : m_project(project) {}

public:
//__PUBLISH_SECTION_END__
    //! Adds a new display style to the project, attempting to honor the provided ID. If a display style already exists by-name or by-id, no action is performed.
    //! @note This should only be used by importers that are attempting to preserve already-unique IDs.
    DGNPLATFORM_EXPORT BentleyStatus InsertWithId(DisplayStyleCR);
//__PUBLISH_SECTION_START__

    //! Queries the project for a display by-ID, and returns a deserialized instance.
    DGNPLATFORM_EXPORT DisplayStyleCP QueryById(DgnStyleId) const;

    //! Queries the project for a display by-name, and returns a deserialized instance.
    DGNPLATFORM_EXPORT DisplayStyleCP QueryByName(Utf8CP) const;

    //! Adds a new display style to the project. The ID in the provided display style is ignored; the returned copy will contain the ID assigned. If a style already exists by-name, no action is performed.
    //! @note New IDs are guaranteed to be strictly greater than 0.
    DGNPLATFORM_EXPORT DisplayStyleCP Insert(DisplayStyleCR);

    //! Updates a display style in the project. The ID in the provided display style is used to identify which display style to update. If a style does not exist by-ID, no action is performed.
    DGNPLATFORM_EXPORT DisplayStyleCP Update(DisplayStyleCR);

    //! Deletes a display style from the project.
    //! @note When a display style is removed, no attempts are currently made to normalize existing elements. Thus elements may still attempt to reference a missing display style, but must be written to assume such a display style doesn't exist.
    DGNPLATFORM_EXPORT void Delete(DisplayStyleCR);

    //! Creates an iterator to iterate available display styles.
    DGNPLATFORM_EXPORT DgnStyles::Iterator MakeIterator(DgnStyleSort sortOrder = DgnStyleSort::None) const;

}; // DisplayStyleFunctions
#endif

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
