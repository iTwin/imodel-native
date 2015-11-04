/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DimensionStyle.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#if defined (NEEDS_WORK_DGNITEM)
#include "../DgnCore/ElementHandle.h"
#include "../Tools/BitMask.h"
#include "DimensionStyleProps.r.h"

DGNPLATFORM_TYPEDEFS (DimensionStyle)
DGNPLATFORM_TYPEDEFS (DimStylePropMask)
DGNPLATFORM_TYPEDEFS (DimStyleIterator)

//__PUBLISH_SECTION_END__
DGNPLATFORM_TYPEDEFS (DimStyleTableEditor)
#include "../DgnCore/DimStyleResource.r.h"
//__PUBLISH_SECTION_START__

#define DIMSTYLE_COMPAREOPTS_Default            (0)
#define DIMSTYLE_COMPAREOPTS_IgnoreUnusedDiffs  (0x0001 << 1)

#define DIMSTYLE_COMPAREOPTS_UserInterface      DIMSTYLE_COMPAREOPTS_IgnoreUnusedDiffs

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DimensionHandler;
struct NoteCellHeaderHandler;
typedef RefCountedPtr<DimensionStyle>   DimensionStylePtr;
typedef RefCountedPtr<DimStylePropMask> DimStylePropMaskPtr;
typedef RefCountedPtr<DimStyleIterator> DimStyleIteratorPtr;


//__PUBLISH_SECTION_END__
#ifdef DGN_IMPORTER_REORG_WIP
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IDimStyleTransactionListener
{
    virtual void   _OnDimStyleChange (DimensionStyleCP before, DimensionStyleCP after, StyleEventType type, StyleEventSource source) = 0;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     DimStyleEvents : IDimStyleTransactionListener
{
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DimStyleTableEditor
{
private:
    EditElementHandleR             m_tableElem;

    // Potentially clones the element into the table's file
    void ElementFromStyle (EditElementHandleR, DimensionStyleR newStyle);

public:
    DimStyleTableEditor (EditElementHandleR table);

    StatusInt                   ReplaceSettingsElement (DimensionStyleR settingsObj);
    StatusInt                   AddEntryElement (DimensionStyleR newStyle);
    StatusInt                   AddEntryElement (DimensionStyleCR newStyle) {return AddEntryElement(const_cast<DimensionStyleR>(newStyle));} // WIP_NONPORT - don't pass non-const ref to temporary object
    StatusInt                   ReplaceEntryElement (DimensionStyleR newStyle, WCharCP oldName);
    StatusInt                   RemoveEntryElement (WCharCP name);

}; // DimStyleTableEditor
#endif


//__PUBLISH_SECTION_START__

/*=================================================================================**//**
* Class to manipulate a dimensionstyle in a DgnFile. DimensionStyles are specific to a file and
they need to be transformed to be used in a different file.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DimensionStyle : RefCountedBase
{
    friend struct DimStyleIterator;
    friend struct DimensionHandler;
    //friend struct DimStyleEntryHandler; moved in graphite to foreignformat
    //friend struct DimStyleTableHandler;           "
    friend struct NoteCellHeaderHandler;
//__PUBLISH_SECTION_END__
    friend struct DimStyleDgnCacheLoaderCollection;
    friend struct DwgConversionDataAccessor;
    
private:

    friend struct DimStyleTableEditor;

    struct UnitLabelPair
        {
        WString         m_masterUnitLabel;
        WString         m_subUnitLabel;
        };

    enum DimStringLinkageKey : unsigned short         {
        DIMSTYLE_Name           = STRING_LINKAGE_KEY_Name,
        DIMSTYLE_Description    = STRING_LINKAGE_KEY_Description,
        DIMCELL_Prefix          = STRING_LINKAGE_KEY_DimPrefixCellName,
        DIMCELL_Suffix          = STRING_LINKAGE_KEY_DimSuffixCellName,
        DIMCELL_Arrow           = STRING_LINKAGE_KEY_DimArrowCellName,
        DIMCELL_Stroke          = STRING_LINKAGE_KEY_DimStrokeCellName,
        DIMCELL_Origin          = STRING_LINKAGE_KEY_DimOriginCellName,
        DIMCELL_Dot             = STRING_LINKAGE_KEY_DimDotCellName,
        DIMCELL_Note            = STRING_LINKAGE_KEY_DimNoteCellName,
        DIMUNIT_PrimaryMaster   = STRING_LINKAGE_KEY_MastUnitLabel,
        DIMUNIT_PrimarySub      = STRING_LINKAGE_KEY_SubUnitLabel,
        DIMUNIT_SecondayMaster  = STRING_LINKAGE_KEY_SecondaryMastUnitLabel,
        DIMUNIT_SecondarySub    = STRING_LINKAGE_KEY_SecondarySubUnitLabel,
        };

    bool                m_initialized;
    DgnDbP         m_dgndb;
    uint64_t            m_elemID;

    WString             m_name;
    WString             m_description;
    DimStyleSettings    m_data;
    DimStyleExtensions  m_extensions;

    DgnTextStylePtr     m_textStyle;

    WString             m_prefixCellName;
    WString             m_suffixCellName;
    WString             m_arrowCellName;
    WString             m_strokeCellName;
    WString             m_originCellName;
    WString             m_dotCellName;
    WString             m_noteCellName;

    UnitLabelPair       m_primary;
    UnitLabelPair       m_secondary;

    struct DimStyleComparer
        {
        uint32_t            m_compareOpts;
        DimensionStyleCR    m_dimStyle1;
        DimensionStyleCR    m_dimStyle2;
        DimStylePropMaskPtr m_differences;
        double              m_doubleTol;
        DgnModelP           m_cache1;
        DgnModelP           m_cache2;

        void                SetPropertyBitInBitArray (uint16_t pBitArray[], DimStyleProp dimStyleProp, bool    bitValue);
        void                SetTemplateBitInBitArray (uint16_t pBitArray[], DimStyleProp dimStyleProp, bool    bitValue);

        void                StripUnusedDiffs ();

        void                CompareBallAndChainAttributes   ();
        void                CompareExtensionLineAttributes  ();
        void                CompareGeneralAttributes        ();
        void                CompareMultiLineNotesAttributes ();
        void                ComparePlacementAttributes      ();
        void                CompareSymbolAttributes         ();
        void                CompareTerminatorAttributes     ();
        void                CompareTextAttributes           ();
        void                CompareToleranceAttributes      ();
        void                CompareValueAttributes          ();
        void                CompareTemplateFlags            ();
        void                CompareExtensions               ();
        void                CompareStrings                  ();

        /* ctor */          DimStyleComparer (DimensionStyleCR style1, DimensionStyleCR style2, uint32_t compareOpts);
        DimStylePropMaskPtr CompareStyles ();
        };

private:
    void                Clear ();
    WString             StringFromElement (ElementGraphicsCR, DimStringLinkageKey);
    void                StringToElement (ElementGraphicsR, WCharCP, DimStringLinkageKey) const;
    int                 GetLabelLineFormat () const;
    void                SetLabelLineFormat (int value);
    uint16_t            GetMLNoteFrameType () const;
    void                SetMLNoteFrameType (uint16_t value);
    StatusInt           GetValueFormat (DimStyleProp_Value_Format& format, DimStyleProp formatProp) const;
    StatusInt           SetValueFormat (DimStyleProp formatProp, DimStyleProp_Value_Format format);
    StatusInt           GetComparisonValue (DimStyleProp_Value_Comparison& comparison, DimStyleProp comparisonProp) const;
    StatusInt           SetComparisonValue (DimStyleProp comparisonProp, DimStyleProp_Value_Format comparison);

    StatusInt           SetTextStyleByID (uint32_t textStyleID);

    static ElementHandle    GetSettingsElement (ElementHandleCR tableElement);
    static ElementHandle          GetSettingsElementByFile (DgnDbR file);
    static ElementHandle    GetStyleElementByName (WCharCP name, ElementHandleCR table);
    static ElementHandle          GetStyleElementByName (WCharCP name, DgnDbR file);
    static ElementHandle          FetchTableElement (DgnDbR file);
    static bool                   IsDimStyleUsed (uint64_t elemID, DgnDbR);
    static ElementHandle          CreateTableElement (DgnDbR file);
protected:
    DimensionStyle (ElementHandleCR styleElem);
    DimensionStyle (DgnDimStyleCR dimStyle);
    DimensionStyle (WCharCP name, DgnDbR file);

    void FromElement (ElementHandleCR styleElem);
    void ToElement (EditElementHandleR) const;
    void FixSettingsObjectFromElement ();

    bool ProcessProperties (PropertyContextR proc, bool canChange);
    void FromDgnDimStyle (DgnDimStyleCR dgnDimStyle);
    DGNPLATFORM_EXPORT void ToDgnDimStyle(DgnDimStyleR dgnDimStyle, DgnModelP dgnCache) const;

    void InitDimText (DimText&, ElementHandleCR dim, int pointNo) const;
    void AssignTextStyle (DgnTextStyleR style) {m_textStyle = &style;}
#ifdef DGN_IMPORTER_REORG_WIP
    static void OnDimStyleTransactionEvent (ElementHandleCP entryBefore, ElementHandleCP entryAfter, StyleEventType eventType, StyleEventSource source);
#endif

    DGNPLATFORM_EXPORT virtual StatusInt     _SetAccuracyProp     (Byte valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetBooleanProp      (bool               valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetCharProp         (unsigned short     valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetDoubleProp       (double             valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetIntegerProp      (int                valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetLevelProp        (LevelId            valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetStringProp       (WCharCP            valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetFontProp         (uint32_t           valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetColorProp        (uint32_t           valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetWeightProp       (uint32_t           valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetLineStyleProp    (int32_t            valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetTextStyleProp    (DgnTextStyleCR     valueIn,    DimStyleProp iProp);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetTemplateFlagProp (unsigned short     valueIn,    DimensionType dimensionType, DimStyleProp iProp);
#if defined (NEEDS_WORK_DGNITEM)
    DGNPLATFORM_EXPORT virtual BentleyStatus _SetName (WCharCP newName);
#endif
    DGNPLATFORM_EXPORT virtual void          _CopyValues (DimensionStyleCR  source);
    DGNPLATFORM_EXPORT virtual StatusInt     _SetUnitsProp (UnitDefinitionCR masterUnit, UnitDefinitionCR subUnit, DimStyleProp iProperty);
    DGNPLATFORM_EXPORT virtual StatusInt     _GetTextStyleProp    (DgnTextStylePtr&   valueOut,   DimStyleProp iProp) const;

public:
    static void                 AddDefaultDimStyleElement ();
    DimStyleSettings*           GetSettingsInternalP () { return &m_data; }
    DimStyleExtensions const&   GetStyleExtensionsCP () const  { return m_extensions; }
    void                        SetStyleExtensions (DimStyleExtensions const& newExtensions) { m_extensions = newExtensions; }
    void                        SetTemplateData (DimTemplate const& data, DimensionType templateIndex);
    DGNPLATFORM_EXPORT unsigned short GetDwgSpecificFlags (unsigned short& primaryUnits, unsigned short& secondaryUnits) const;
    void                        SetDwgSpecificFlags (unsigned short flags);
    DGNPLATFORM_EXPORT void     SetDwgSpecificFlags (unsigned short primaryUnits, unsigned short secondaryUnits);
    unsigned short              GetDeprecatedElbowJointLocation ()              { return m_extensions.flags.uElbowJointLocation;  }
    void                        SetDeprecatedElbowJointLocation (unsigned short value)  { m_extensions.flags.uElbowJointLocation = value; }
    bool                        IsPropertyLocallyControlled (DimStyleProp prop) const;
    DGNPLATFORM_EXPORT bool                 IsWitnessVisible (ElementHandleCR dim, int pointNo) const;

#ifdef DGN_IMPORTER_REORG_WIP
    MSCORE_EXPORT DimStylePropMaskPtr       GetModifiedProperties() const;
    DGNPLATFORM_EXPORT void                 DropDependants() const;
    MSCORE_EXPORT static void               AddListener (DimStyleEvents& handler);
    MSCORE_EXPORT static void               DropListener (DimStyleEvents& handler);
    MSCORE_EXPORT static void               UpdateFileFromDgnLibs (DgnDbR dgnFile);
    MSCORE_EXPORT static DimensionStylePtr  GetActive (bool createIfNecessary);
    MSCORE_EXPORT void                      EnforceCurrentWorkMode ();

    DGNPLATFORM_EXPORT static ElementHandle  GetTableElement (DgnDbR file);
    DGNPLATFORM_EXPORT static void           SetTransactionListener (IDimStyleTransactionListener* obj);
#endif
    DGNPLATFORM_EXPORT static BentleyStatus  GetOverrideProp (DimStyleProp* overrideProp, bool* inverted, DimStyleProp property);
    DGNPLATFORM_EXPORT static BentleyStatus  GetOverriddenProp (DimStyleProp* property, bool* inverted, DimStyleProp override);
    DGNPLATFORM_EXPORT static StringListP    GetPropValuesStringList (DimStyleProp_ValueList);
    DGNPLATFORM_EXPORT static StringListP    GetPropCategoryStringList (DimStyleProp_Category);

    DGNPLATFORM_EXPORT bool                  IsUsed() const;
    DGNPLATFORM_EXPORT void                  UpdateTextStyleParams (bool* pAnyChange, DgnTextStyleCR textStyle);

    DGNPLATFORM_EXPORT static DimensionStylePtr      CreateFromDgnDimStyle (DgnDimStyleCR dgnDimStyle);
    DGNPLATFORM_EXPORT static DimensionStylePtr      CreateFromElement (ElementHandleCR styleElem);


//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    typedef DimStylePropMaskPtr PropertyMaskPtr;

    //! Create a new DimensionStyle object.  The style is not be persisted automatically.
    DGNPLATFORM_EXPORT static DimensionStylePtr      Create (WCharCP name, DgnDbR file);

    //! Find an existing persistent Named DimensionStyle.
    DGNPLATFORM_EXPORT static DimensionStylePtr      GetByName (WCharCP name, DgnDbR file);

    //! Find an existing persistent Named DimensionStyle.
    DGNPLATFORM_EXPORT static DimensionStylePtr      GetByID (uint64_t elemID, DgnDbR file);

    //! Get the active dimension settings from a file.
    DGNPLATFORM_EXPORT static DimensionStylePtr      GetSettings (DgnDbR);

    //! Overwrite the active dimension settings from a file.
    DGNPLATFORM_EXPORT static BentleyStatus          ReplaceSettings (DimensionStyleR settings, DgnDbR file);

    //! Add this DimensionStyle to a DgnFile.  Will fail if there is already a style with the same name.
    //! @param[in] file     The file in which to write the style.  If NULL, the style will be written to the file
    //!                     stored in the style object.
    //! @remark If the style is written to a file different than the file stored in the style object, then the style
    //!         object will be converted to the new file.
    //! @return SUCCESS if the style was written.
    DGNPLATFORM_EXPORT BentleyStatus         Add (DgnDbP file = NULL);

    //! Replace an existing persistent DimensionStyle in a DgnFile.  Will fail if there is not already a style with the
    //! same name.
    //! @param[in] oldName  The name of the style to replace.  If NULL, the style will be overwrite the style with the
    //!                     same name as the style object.
    //! @param[in] file     The file in which to write the style.  If NULL, the style will be written to the file
    //!                     stored in the style object.
    //! @remark If the style is written to a file different than the file stored in the style object, then the style
    //!         object will be converted to the new file.
    //! @return SUCCESS if the style was written.
    DGNPLATFORM_EXPORT BentleyStatus         Replace (WCharCP oldName = NULL, DgnDbP file = NULL);

    //! Deletes a dimension style from a dgn file.  This method will fail
    //!              if any elements refer to the style.
    //! @param[in] name     The name of the style to delete.
    //! @param[in] file     The file from which to delete the style.
    //! @return SUCCESS if the style was removed.
    //! @see HasDependents
    //! @see RemapDependents
    DGNPLATFORM_EXPORT static BentleyStatus  Delete (WCharCP name, DgnDbR file);

    DGNPLATFORM_EXPORT StatusInt             GetUnitsProp (UnitDefinitionP master, UnitDefinitionP sub, DimStyleProp iProp) const;

    //! Change the value of a units type property.  The master unit must be larger than or the same size as the sub unit.
    DGNPLATFORM_EXPORT StatusInt             SetUnitsProp (UnitDefinitionCR master, UnitDefinitionCR sub, DimStyleProp iProp);

    //! Get the name of this style.
    DGNPLATFORM_EXPORT WStringCR             GetName () const;

    //! Get the id of this style.
    DGNPLATFORM_EXPORT uint64_t           GetID () const;

    //! Get the file which serves as the context for this style.
    DGNPLATFORM_EXPORT DgnDbP GetDgnDb() const;

    //! Get the id of the TextStyle used by this style.  If this style does not refer to a TextStyle this method will return zero.
    DGNPLATFORM_EXPORT uint32_t              GetTextStyleId () const;

    //! Make a copy of this style object.
    DGNPLATFORM_EXPORT DimensionStylePtr     Copy () const;

    //! Copy all the properties from another style object into this one.
    DGNPLATFORM_EXPORT void                  CopyValues (DimensionStyleCR source);

    //! Set the name of this style.
    DGNPLATFORM_EXPORT BentleyStatus         SetName (WCharCP newName);
    DGNPLATFORM_EXPORT bool                  IsValid () const;

    //! Compare this style object with another one.
    //! @param[in] otherStyle     The other style.
    //! @param[in] compareOpts    Options that control the comparison.
    //! @remarks The following compare options are supported:
    //! <ul>
    //! <li>  DIMSTYLE_COMPAREOPTS_Default                Compare every property.
    //! <li>  DIMSTYLE_COMPAREOPTS_IgnoreUnusedDiffs      Ignore those properties that are 'unused'.
    //! </ul>
    //! An unused property will not affect dimensions placed with this style.  For example, if the style's extension color flag is OFF,
    //! that means the dimension's extension color will come from the header color.  In that case, the style's extension color value is unused.
    //! @return A mask with 'on' bits representing each difference between the two styles.  To quickly test if any difference exists
    //!         use DimStylePropMask::AnyBitSet
    DGNPLATFORM_EXPORT DimStylePropMaskPtr   Compare (DimensionStyleCR otherStyle, uint32_t compareOpts = DIMSTYLE_COMPAREOPTS_Default) const;

    //! Change the context of this style to another DgnFile.
    DGNPLATFORM_EXPORT StatusInt             ConvertToFile (DgnDbR file);

    //! Get the value of an accuracy type property.
    DGNPLATFORM_EXPORT StatusInt     GetAccuracyProp     (Byte&              valueOut,   DimStyleProp iProp) const;

    //! Get the value of a boolean type property.
    DGNPLATFORM_EXPORT StatusInt     GetBooleanProp      (bool&              valueOut,   DimStyleProp iProp) const;

    //! Get the value of a char type property.
    DGNPLATFORM_EXPORT StatusInt     GetCharProp         (unsigned short&            valueOut,   DimStyleProp iProp) const;

    //! Get the value of a double type property.
    DGNPLATFORM_EXPORT StatusInt     GetDoubleProp       (double&            valueOut,   DimStyleProp iProp) const;

    //! Get the value of an integer type property.
    DGNPLATFORM_EXPORT StatusInt     GetIntegerProp      (int&               valueOut,   DimStyleProp iProp) const;

    //! Get the value of a level type property.
    DGNPLATFORM_EXPORT StatusInt     GetLevelProp        (LevelId&           valueOut,   DimStyleProp iProp) const;

    //! Get the value of a string type property.
    DGNPLATFORM_EXPORT StatusInt     GetStringProp       (WStringR           valueOut,   DimStyleProp iProp) const;

    //! Get the value of a font type property.
    DGNPLATFORM_EXPORT StatusInt     GetFontProp         (uint32_t&            valueOut,   DimStyleProp iProp) const;

    //! Get the value of an accuracy type property.
    DGNPLATFORM_EXPORT StatusInt     GetColorProp        (uint32_t&            valueOut,   DimStyleProp iProp) const;

    //! Get the value of a weight type property.
    DGNPLATFORM_EXPORT StatusInt     GetWeightProp       (uint32_t&            valueOut,   DimStyleProp iProp) const;

    //! Get the value of a linestyle type property.
    DGNPLATFORM_EXPORT StatusInt     GetLineStyleProp    (int32_t&             valueOut,   DimStyleProp iProp) const;

    //! Get the value of a unit type property.  To change a unit property use SetUnitsProp().
    DGNPLATFORM_EXPORT StatusInt     GetOneUnitProp      (UnitDefinitionR    valueOut,   DimStyleProp iProp) const;

    //! Get the value of a text style type property.
    DGNPLATFORM_EXPORT StatusInt     GetTextStyleProp    (DgnTextStylePtr&   valueOut,   DimStyleProp iProp) const;

    //! Get the value of a template flag.
    DGNPLATFORM_EXPORT StatusInt     GetTemplateFlagProp (unsigned short&    valueOut, DimensionType dimensionType, DimStyleProp iProp) const;

    //! Change the value of an accuracy type property.
    DGNPLATFORM_EXPORT StatusInt     SetAccuracyProp     (Byte valueIn,    DimStyleProp iProp);

    //! Change the value of a boolean type property.
    DGNPLATFORM_EXPORT StatusInt     SetBooleanProp      (bool               valueIn,    DimStyleProp iProp);

    //! Change the value of a char type property.
    DGNPLATFORM_EXPORT StatusInt     SetCharProp         (unsigned short     valueIn,    DimStyleProp iProp);

    //! Change the value of a double type property.
    DGNPLATFORM_EXPORT StatusInt     SetDoubleProp       (double             valueIn,    DimStyleProp iProp);

    //! Change the value of an integer type property.
    DGNPLATFORM_EXPORT StatusInt     SetIntegerProp      (int                valueIn,    DimStyleProp iProp);

    //! Change the value of a level type property.
    DGNPLATFORM_EXPORT StatusInt     SetLevelProp        (LevelId            valueIn,    DimStyleProp iProp);

    //! Change the value of a string type property.
    DGNPLATFORM_EXPORT StatusInt     SetStringProp       (WCharCP            valueIn,    DimStyleProp iProp);

    //! Change the value of a font type property.
    DGNPLATFORM_EXPORT StatusInt     SetFontProp         (uint32_t           valueIn,    DimStyleProp iProp);

    //! Change the value of a color type property.
    DGNPLATFORM_EXPORT StatusInt     SetColorProp        (uint32_t           valueIn,    DimStyleProp iProp);

    //! Change the value of a weight type property.
    DGNPLATFORM_EXPORT StatusInt     SetWeightProp       (uint32_t           valueIn,    DimStyleProp iProp);

    //! Change the value of a line style type property.
    DGNPLATFORM_EXPORT StatusInt     SetLineStyleProp    (int32_t            valueIn,    DimStyleProp iProp);

    //! Change the value of a unit type property.
    DGNPLATFORM_EXPORT StatusInt     SetOneUnitProp      (UnitDefinitionCR   valueIn,    DimStyleProp iProp);

    //! Change the value of a text style type property.
    DGNPLATFORM_EXPORT StatusInt     SetTextStyleProp    (DgnTextStyleCR     valueIn,    DimStyleProp iProp);

    //! Change the value of a template flag.
    DGNPLATFORM_EXPORT StatusInt     SetTemplateFlagProp (unsigned short valueIn,  DimensionType dimensionType, DimStyleProp iProp);

    //__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT void          SetElemID (uint64_t elemID);

#ifdef DGN_IMPORTER_REORG_WIP
    MSCORE_EXPORT   static DimensionStylePtr      GetActive ();
    MSCORE_EXPORT   static BentleyStatus          SetActive (DimensionStyleR dimStyle);
    MSCORE_EXPORT   static BentleyStatus          MakeDefaultStyleActive ();
    MSCORE_EXPORT   static DimensionStylePtr      GetByName (WCharCP name, StyleIteratorMode mode);

    MSCORE_EXPORT   bool                          IsActive() const;

    MSCORE_EXPORT   StatusInt     GetEffectiveUnitsProp (UnitDefinitionP master, UnitDefinitionP sub, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveAccuracyProp     (Byte&             valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveBooleanProp      (bool&             valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveCharProp         (unsigned short&           valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveDoubleProp       (double&           valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveIntegerProp      (int&              valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveLevelProp        (LevelId&          valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveStringProp       (WStringR          valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveFontProp         (uint32_t&           valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveColorProp        (uint32_t&           valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveWeightProp       (uint32_t&           valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveLineStyleProp    (int32_t&            valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveOneUnitProp      (UnitDefinitionR   valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveTextStyleProp    (DgnTextStylePtr&  valueOut, bool* inherited, DimStyleProp iProp) const;
    MSCORE_EXPORT   StatusInt     GetEffectiveTemplateFlagProp (unsigned short&           valueOut, bool* inherited, DimensionType dimensionType, DimStyleProp iProp) const;
#endif

    //__PUBLISH_SECTION_START__
};

struct DimStyleCollection;
#ifdef DGN_IMPORTER_REORG_WIP

/*=================================================================================**//**
* A forward iterator to go through the list of styles in a file.
+===============+===============+===============+===============+===============+======*/
struct DimStyleIterator : std::iterator<std::forward_iterator_tag, DimensionStyleP const>
{
private:
    friend struct DimStyleCollection;
    Dgn::ChildElemIter  m_elemIter;
    mutable DimensionStylePtr   m_current;
    
    DimStyleIterator (DgnDbP file);

public:
    DGNPLATFORM_EXPORT DimStyleIterator ();                                                     //!< Initializes an invalid iterator.
    DGNPLATFORM_EXPORT DimStyleIterator&        operator ++();                                  //!< Advances the iterator to the next in collection.
    DGNPLATFORM_EXPORT DimensionStyleP const&   operator *() const;                             //!< Returns the style value.
    DGNPLATFORM_EXPORT bool                     operator == (DimStyleIterator const &) const;   //!< Test for equality with another iterator.
    DGNPLATFORM_EXPORT bool                     operator != (DimStyleIterator const &) const;   //!< Test for inequality with another iterator.
    DGNPLATFORM_EXPORT bool                     IsValid () const;                               //!< Test if the iterator is valid.
};
#endif

/*=================================================================================**//**
* A collection dimension styles in a file.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DimStyleCollection
{
private:
    DgnDbP m_dgnFile;

public:
    
    //!  Intialize a collection from a file
    //! @param file   Reference to the file on which to get the collection from
    DGNPLATFORM_EXPORT DimStyleCollection (DgnDbR file);
    
    typedef DimStyleIterator const_iterator;
    typedef const_iterator iterator;    //!< only const iteration is possible

    //!  Returns an iterator addressing the first element in the collection
    DGNPLATFORM_EXPORT DimStyleIterator begin () const;

    //!  Returns an iterator that addresses the location succeeding the last element in collection
    DGNPLATFORM_EXPORT DimStyleIterator end () const;
};

/*=================================================================================**//**
*  The property mask contains a series of boolean flags, one for each 
*               DimStyleProp that can be stored in a dimension style.  DimStylePropMask 
*               is most commonly used as the output from DimensionStyle::Compare and
                IDimensionQuery::GetOverrideFlags.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DimStylePropMask : RefCountedBase
{
//__PUBLISH_SECTION_END__
friend struct  DimensionStyle;

private:
    enum DimStyle_Category
        {
        DIMSTYLE_CATEGORY_BallAndChainAttributes     =  0,
        DIMSTYLE_CATEGORY_ExtensionLineAttributes    =  1,
        DIMSTYLE_CATEGORY_GeneralAttributes          =  2,
        DIMSTYLE_CATEGORY_MultiLineAttributes        =  3,
        DIMSTYLE_CATEGORY_PlacementAttributes        =  4,
        DIMSTYLE_CATEGORY_SymbolAttributes           =  5,
        DIMSTYLE_CATEGORY_TerminatorAttributes       =  6,
        DIMSTYLE_CATEGORY_TextAttributes             =  7,
        DIMSTYLE_CATEGORY_ToleranceAttributes        =  8,
        DIMSTYLE_CATEGORY_ValueAttributes            =  9,
        DIMSTYLE_CATEGORY_Template0Attributes        = 10,
        DIMSTYLE_CATEGORY_Template1Attributes        = 11,
        DIMSTYLE_CATEGORY_Template2Attributes        = 12,
        DIMSTYLE_CATEGORY_Template3Attributes        = 13,
        DIMSTYLE_CATEGORY_Template4Attributes        = 14,
        DIMSTYLE_CATEGORY_Template5Attributes        = 15,
        DIMSTYLE_CATEGORY_Template6Attributes        = 16,
        DIMSTYLE_CATEGORY_Template7Attributes        = 17,
        DIMSTYLE_CATEGORY_Template8Attributes        = 18,
        DIMSTYLE_CATEGORY_Template9Attributes        = 19,
        DIMSTYLE_CATEGORY_Template10Attributes       = 20,
        DIMSTYLE_CATEGORY_Template11Attributes       = 21,
        DIMSTYLE_CATEGORY_Template12Attributes       = 22,
        DIMSTYLE_CATEGORY_Template13Attributes       = 23,
        DIMSTYLE_CATEGORY_Template14Attributes       = 24,
        DIMSTYLE_CATEGORY_Template15Attributes       = 25,
        DIMSTYLE_CATEGORY_Template16Attributes       = 26,
        DIMSTYLE_CATEGORY_Template17Attributes       = 27,
        DIMSTYLE_CATEGORY_Template18Attributes       = 28,
        DIMSTYLE_CATEGORY_Template19Attributes       = 29,
        DIMSTYLE_CATEGORY_Template20Attributes       = 30,
        DIMSTYLE_CATEGORY_Template21Attributes       = 31,
        DIMSTYLE_CATEGORY_Template22Attributes       = 32,
        DIMSTYLE_CATEGORY_Template23Attributes       = 33,
        DIMSTYLE_CATEGORY_COUNT                      = 34,
        };

    BitMaskP m_pBitMasks [DIMSTYLE_CATEGORY_COUNT];
    static const uint16_t s_templateCategoryId [24];
private:
    DimStylePropMask ();
    ~DimStylePropMask ();

    StatusInt   SetBitMaskP (BitMaskP bitMask, DimStyle_Category categoryID);

    BitMaskP    GetBitMaskP  (DimStyle_Category categoryID);
    BitMaskCP   GetBitMaskCP (DimStyle_Category categoryID) const;

    StatusInt   SetBitByCategoryAndPosition (DimStyle_Category categoryID, int bitPosition, bool bitValue);

protected:
    StatusInt   SetBitMaskByBitArray (uint16_t pArrayIn[], int nValidBits, DimStyle_Category categoryID);
    static void MapTemplateBitPosition (int& bitPosition, DimStyleProp dimStyleProp);

public:
    DGNPLATFORM_EXPORT static void LogicalOperation (DimStylePropMaskR destPropMask, DimStylePropMaskCR sourcePropMask, BitMaskOperation operation);

    StatusInt                   ClearTemplateBits (int templateIndex);

    static  unsigned short      GetLinkageKeyFromCategoryID (DimStyle_Category catID);
    static  DimStylePropMaskPtr ExtractFromLinkages (ElementHandleCR elm);
    static  void                DeleteLinkages (ElementGraphicsR elm);
            void                AppendAsLinkages (ElementGraphicsR elm) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public: 
    //!  Create a new DimStylePropMask object.  All bits will be off.
    DGNPLATFORM_EXPORT static  DimStylePropMaskPtr CreatePropMask ();

    //!  Copy the state of another mask into this one.
    DGNPLATFORM_EXPORT void      From (DimStylePropMaskCR);

    //!  Test if any bits are on in this mask.
    DGNPLATFORM_EXPORT bool      AnyBitSet () const;

    //!  Set all bits to off in this mask.
    DGNPLATFORM_EXPORT void      ClearAllBits ();

    //!  Set the value of a single property bit.
    DGNPLATFORM_EXPORT StatusInt SetPropertyBit (DimStyleProp dimStyleProp, bool bitValue);

    //!  Set the value of a single template bit.
    DGNPLATFORM_EXPORT StatusInt SetTemplateBit (DimStyleProp dimStyleProp, bool bitValue, DimensionType dimensionType);

    //!  Test the value of a single property bit.
    DGNPLATFORM_EXPORT bool      GetPropertyBit (DimStyleProp dimStyleProp) const;

    //!  Test the value of a single template bit.
    DGNPLATFORM_EXPORT bool      GetTemplateBit (DimStyleProp dimStyleProp, DimensionType dimensionType) const;

}; // DimensionStylePropMask

//__PUBLISH_SECTION_END__

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
//!  Helper class to create and load a textstyle descriptor with predefined
//! ElementId directly into the non model cache.
* @bsimethod                                    Abeesh.Basheer                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimStyleDgnCacheLoaderCollection
    {
    private:
        bvector<DimensionStylePtr>  m_styles;
        ElementId                       m_settingsElement;
    public:
        DimStyleDgnCacheLoaderCollection () : m_settingsElement (INVALID_ELEMENTID){}
        //!  Adds the given textstyle to the collection.
        //! @param style  A pointer to a textstyle which needs to be loaded.
        //!                 The ID value is assigned on an add
        //! @param id  ElementId of the textstyle element added to the file.
        DGNPLATFORM_EXPORT BentleyStatus Add (DimensionStyleR style, ElementId id, bool isSettings = false);
        
        //! Creates and adds a textstyle table directly to the nonmodel cache.
        DGNPLATFORM_EXPORT BentleyStatus AddToFile (DgnDbR file) const;
    };
#endif

//__PUBLISH_SECTION_START__

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
/*=====================================================================================+
*
* Function declarations
*
+===============+===============+===============+===============+===============+======*/
BEGIN_BENTLEY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/04
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  Dgn::PropToOverrideMap* dgnDimStyle_getPropToOverridesMap
(
int                         *pNumMaps   // <=
);

/*---------------------------------------------------------------------------------**//**
* Tests whether given element is a valid dimension style or dimension element.
*
* @param        pCandidate      => element to tedt
* @return       true if valid style or dimension element.
* @bsimethod                                                    petri.niiranen  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public int          dimStyleEntry_isStyleOrDimension (ElementHandleCR element);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/04
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT double    mdlDimStyle_getEffectiveAnnotationScale
(
DgnDimStyleCP        dgnDimStyleP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 09/03
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    mdlDimStyle_applyAnnotationScaleToUnscaledTextSize
(
DgnDimStyleP            dgnDimStyleP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 09/03
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    mdlDimStyle_unapplyAnnotationScaleToScaledTextSize
(
DgnDimStyleP            dgnDimStyleP
);

END_BENTLEY_NAMESPACE

/*===========================================================================
  Relationship between bUsingAnnotationScale, UseAnnotationScale Lock and
  Style's Annotation Scale Override

  bUsingAnnotationScale is used only to indicate if the active dimstyle
  currently has any annotation scale in it or not. bUsingAnnotationScale's job
  is to ensure that the scale is not applied more than once. When the apps
  look at the active dimstyle, they should first look at bUsingAnnotationScale
  to make sure an annotation scale is in effect.

  There are three valid states for the active dimstyle.
  a. Using Model's annotation scale
       When: Style does not override scale & Model's UseAnnotationScale lock is on
       Then: bUsingAnnotationScale is True
  b. Using Style's annotation scale
       When: Style overrides scale
       Then: bUsingAnnotationScale is True
  c. Not Using Either scales
       When: Style does not override scale & Model's UseAnnotationScale lock is off
       Then: bUsingAnnotationSclae is False

===========================================================================*/


/*===========================================================================
  Relationship between TextLocation and Embed

  Embed flag used to indicate if the text is located inline or above the
  dimension line. TextLocation flag is needed to support autocad's Outside
  and Top-Left settings. uTextLocation will store values of 0, 1, 2.
  0 - get the value from the embed flag
  1 - outside
  2 - top-left

  Setting Values using TextLocation dimstyleprop
  0 - above     => uTextLocation = 0, embed = 0
  1 - inline    => uTextLocation = 0, embed = 1
  2 - outside   => uTextLocation = 1, embed = 0 (nonembed for backward compatibility)
  3 - top-left  => uTextLocation = 2, embed = 0 (nonembed for backward compatibility)

  Setting Values using embed dimstyleprop
  0 - above     => uTextLocation = 0, embed = 0
  1 - inline    => uTextLocation = 0, embed = 1

  Querying Values using TextLocation dimstyleprop
  uTextLocation = 0 && embed = 0 => 0 - above
  uTextLocation = 0 && embed = 1 => 1 - inline
  uTextLocation = 1              => 2 - outside
  uTextLocation = 2              => 3 - top-left

  Querying Values using embed dimstyleprop
  embed = 0                      => 0 - above
  embed = 1                      => 0 - inline
===========================================================================*/

/*===========================================================================
  Pursuing autocad absurdity, we have got ourselves into a complicated
  situation with the following flags.

  uElbowJointLocation       - introduced in cvs7.11.2.3 ms08010001
  uNoteVerLeftAttachment    - introduced now
  uNoteVerRightAttachment   - introduced now

  ===========================================================================

  SET VALUE IN V8.1 (cannot change now)
  =====================================

  Bothsides_Attachment_Value    uElbowJointLocation
       LineOfText                       0
       JustificationPoint               1
       Underline                        1

  ===========================================================================

  SET VALUE (new)
  ===============

  Left_Attachment_Value   uNoteVerLeftAttachment   uElbowJointLocation
        Top                     1                       1
        TopLine                 2                       0
        MiddleLine              3                       0
        BottomLine              4                       0
        Bottom                  5                       1
        DynamicLine             6                       0
        DynamicCorner           7                       1
        Underline               8                       2

  Right_Attachment_Value  uNoteVerRightAttachment  uElbowJointLocation
        Top                     1                       Not Affected
        TopLine                 2                       "
        MiddleLine              3                       "
        BottomLine              4                       "
        Bottom                  5                       "
        DynamicLine             6                       "
        DynamicCorner           7                       "
        Underline               8                       "

  ===========================================================================

  GET VALUE IN V8.1 (cannot change now)
  =====================================

  uElbowJointLocation     uMultiJustVertical        Bothsides_Attachment_Value
        0                       Top                     TopLine
        0                       Center                  CenterLine
        0                       Bottom                  BottomLine
        0                       Dynamic                 DynamicLine
        1                       Top                     TopJustPoint
        1                       Center                  CenterJustPoint
        1                       Bottom                  BottomJustPoint
        1                       Dynamic                 DynamicCorner
        2                       n/a                     Underline

  ==========================================================================

  GET VALUE (new)
  ===============

  uNoteVerLeftAttachment  uElbowJointLocation       Left_Attachment_Value
        0                       Use V8.1 behavior depicted above
        1                       n/a                     Top
        2                       n/a                     TopLine
        3                       n/a                     MiddleLine
        4                       n/a                     BottomLine
        5                       n/a                     Bottom
        6                       n/a                     DynamicLine
        7                       n/a                     DynamicCorner
        8                       n/a                     Underline

  uNoteVerRightAttachment uElbowJointLocation       Right_Attachment_Value
        0                       Use V8.1 behavior depicted above
        1                       n/a                     Top
        2                       n/a                     TopLine
        3                       n/a                     CenterLine
        4                       n/a                     BottomLine
        5                       n/a                     Bottom
        6                       n/a                     DynamicLine
        7                       n/a                     DynamicCorner
        8                       n/a                     Underline
===========================================================================*/

//__PUBLISH_SECTION_START__
#endif

/** @endcond */
