/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/MultilineStyle.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/Tools/BitMask.h>

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/IViewDraw.h>

//__PUBLISH_SECTION_END__
#define MLINE_MAX_POSSIBLE          32      // Use this so the data structure won't crash, thus going to 32 lines is possible by adjusting MULTILINE_MAX.
#define MLINE_VERSION       3
//__PUBLISH_SECTION_START__

DGNPLATFORM_TYPEDEFS (MultilineStyle)
DGNPLATFORM_TYPEDEFS (MultilineStylePropMask)
DGNPLATFORM_TYPEDEFS (MultilineProfile)
DGNPLATFORM_TYPEDEFS (MultilineSymbology)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef RefCountedPtr<MultilineStyle>           MultilineStylePtr;
typedef RefCountedPtr<MultilineStylePropMask>   MultilineStylePropMaskPtr;
typedef RefCountedPtr<MultilineProfile>         MultilineProfilePtr;
typedef RefCountedPtr<MultilineSymbology>       MultilineSymbologyPtr;

//! The Multi-line cap types.
//! Use these if the function takes "isCap" as well as an index
enum MultilineCapType
    {
    MULTILINE_ORG_CAP           = 0,
    MULTILINE_END_CAP           = 1,
    MULTILINE_MID_CAP           = 2,
    MULTILINE_MAX               = 16
    };

//! The Multi-line cap Indicies
//! Use these if the function takes just an index
enum MultilineCapIndex
    {
    // Use these if the function just takes a line index
    MULTILINE_ORG_CAP_INDEX  =   MULTILINE_MAX + MULTILINE_ORG_CAP,
    MULTILINE_END_CAP_INDEX  =   MULTILINE_MAX + MULTILINE_END_CAP,
    MULTILINE_MID_CAP_INDEX  =   MULTILINE_MAX + MULTILINE_MID_CAP,

    // Use this to iterate over all multilines and all caps
    MULTILINE_ITERATE_MAX    =  (MULTILINE_MID_CAP_INDEX + 1)
    };

//__PUBLISH_SECTION_END__
enum MlinestylePropFlags
    {
    // General flags
    MLINESTYLE_PROP_OrgAngle         =   0,
    MLINESTYLE_PROP_EndAngle         =   1,
    MLINESTYLE_PROP_NumLines         =   2, // It is bad news if this one is set.
    MLINESTYLE_PROP_Fill             =   3,

    // Profile or Cap
    MLINESTYLE_PROP_Class            =   0,
    MLINESTYLE_PROP_Color            =   1,
    MLINESTYLE_PROP_Weight           =   2,
    MLINESTYLE_PROP_LineStyle        =   3,
    MLINESTYLE_PROP_Level            =   4,

    // Cap Specific
    MLINESTYLE_PROP_CapColorFromSeg  =   8,
    MLINESTYLE_PROP_CapLine          =   9,
    MLINESTYLE_PROP_CapOutArc        =   10,
    MLINESTYLE_PROP_CapInArc         =   11,
    // Profile Specific
    MLINESTYLE_PROP_Offset           =   8
    };

#ifdef DGN_IMPORTER_REORG_WIP
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IMlineStyleTransactionListener
{
    virtual void   _OnMlineStyleChange (MultilineStyleCP before, MultilineStyleCP after, StyleEventType type, StyleEventSource source) = 0;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     MlineStyleEvents : IMlineStyleTransactionListener
{
};
#endif

//__PUBLISH_SECTION_START__

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultilineStylePropMask : RefCountedBase
    {
//__PUBLISH_SECTION_END__

private:

    BitMaskP m_pBitMask;


    MultilineStylePropMask ();
    ~MultilineStylePropMask ();

    bool            AreDoublesEqual (double value1, double value2, double tol);
    void            CompareSymbology (MultilineSymbologyCR pSymb1, MultilineSymbologyCR pSymb2, bool isCap, UInt32 capIndex);
    void            CompareProfiles (MultilineProfileCR pProfile1, MultilineProfileCR pProfile2, UInt32 profileIndex, double doubleTol);
    void            Dump () const;

public:
    DGNPLATFORM_EXPORT BentleyStatus   CompareStyles (MultilineStyleCR pStyle1, MultilineStyleCR pStyle2);
    DGNPLATFORM_EXPORT BentleyStatus   FromElement (ElementHandleR element);
    DGNPLATFORM_EXPORT static void     RemoveFromElement (EditElementHandleR element);
    DGNPLATFORM_EXPORT BentleyStatus   AddToElement (EditElementHandleR element) const;
    BentleyStatus   SetCapOrProfileBit (bool isCap, UInt32 index, UInt32 item, bool bitValue);
    bool            GetCapOrProfileBit (bool isCap, UInt32 index, UInt32 item) const;


    DGNPLATFORM_EXPORT BentleyStatus LogicalOperation (MultilineStylePropMaskCR propMask1, MultilineStylePropMaskCR propMask2, BitMaskOperation operation);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! Create an empty PropertyMask for a Multiline Style.
    //! @return   A Refcounted Pointer to an empty mask.
    DGNPLATFORM_EXPORT static MultilineStylePropMaskPtr    Create ();

    //! Get one of the MlinestylePropFlags flags for an overall multi-line property such as origin angle or fill.
    //! @param[in]      item  The property being queried
    //! @return   The value of the requested bit.
    DGNPLATFORM_EXPORT bool GetGeneralBit (UInt32 item) const;

    //! Set one of the MlinestylePropFlags flags for an overall multi-line property such as origin angle or fill.
    //! @param[in]      item        The property being set.
    //! @param[in]      bitValue    The new value for the property.
    //! @return   SUCCESS if the bit is set in the mask.
    DGNPLATFORM_EXPORT BentleyStatus SetGeneralBit (UInt32 item, bool bitValue);

    //! Get one of the MlinestylePropFlags flags for a cap multi-line property such as color or cap arc.
    //! @param[in]      capIndex    Which cap the property is associated with.
    //! @param[in]      item        The property being queried
    //! @return   The value of the requested bit.
    DGNPLATFORM_EXPORT bool GetCapBit (MultilineCapType capIndex, UInt32 item) const;

    //! Get one of the MlinestylePropFlags flags for a cap multi-line property such as color or cap arc.
    //! @param[in]      capIndex    Which cap the property is associated with.
    //! @param[in]      item        The property being queried
    //! @param[in]      bitValue    The new value for the property.
    //! @return   SUCCESS if the bit is set in the mask.
    DGNPLATFORM_EXPORT BentleyStatus SetCapBit (MultilineCapType capIndex, UInt32 item, bool bitValue);

    //! Get one of the MlinestylePropFlags flags for a cap multi-line property such as color or offset.
    //! @param[in]      profileIndex    Which profile the property is associated with.
    //! @param[in]      item        The property being queried
    //! @return   The value of the requested bit.
    DGNPLATFORM_EXPORT bool GetProfileBit (UInt32 profileIndex, UInt32 item) const;

    //! Get one of the MlinestylePropFlags flags for a cap multi-line property such as color or offset.
    //! @param[in]      profileIndex    Which profile the property is associated with.
    //! @param[in]      item            The property being queried
    //! @param[in]      bitValue        The new value for the property.
    //! @return   SUCCESS if the bit is set in the mask.
    DGNPLATFORM_EXPORT BentleyStatus SetProfileBit (UInt32 profileIndex, UInt32 item, bool bitValue);

    //! Tests whether any of the flags are set.
    //! @return   True if any bits are set.
    DGNPLATFORM_EXPORT bool AnyBitSet () const;

    //! Reset all the property flags to false.
    DGNPLATFORM_EXPORT void ClearAllBits ();

    //! Copies the values from another property mask.
    //! @param[in]      other  The property mask to copy.
    DGNPLATFORM_EXPORT void CopyValues (MultilineStylePropMaskCR other);

    } ;

//__PUBLISH_SECTION_END__

#ifdef DGN_IMPORTER_REORG_WIP
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MlineStyleElementIterator
{
private:
    DgnProjectR        m_dgnFile;
    DgnPlatform::ChildElemIter   m_elemIter;

public:
    MlineStyleElementIterator (DgnProjectR file);
    MlineStyleElementIterator (ElementHandleCR tableElem);

    bool IsValid() const;
    void ToNext();
    DgnPlatform::ElementHandle const& GetCurrent() const;

}; // MlineStyleElementIterator

/*=================================================================================**//**
A collection multi-line styles in a file.
@ingroup MultilineElements
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultilineStyleCollection
{
private:
    DgnProjectP m_dgnFile;

public:
    
    //!  Intialize a collection from a file
    //! @param file   Reference to the file on which to get the collection from
    DGNPLATFORM_EXPORT MultilineStyleCollection (DgnProjectR file);
    
    /*=================================================================================**//**
    * A forward iterator to go through the list of styles in a file.
    +===============+===============+===============+===============+===============+======*/
    struct          const_iterator : std::iterator<std::input_iterator_tag, MultilineStyleP const>
    {
    private:
        friend struct  MultilineStyleCollection;

        DgnPlatform::ChildElemIter  m_elemIter;
        mutable MultilineStylePtr   m_current;

        const_iterator (DgnProjectP file);

    public:
        DGNPLATFORM_EXPORT const_iterator ();                                                                           //!< Initializes an invalid iterator
        DGNPLATFORM_EXPORT const_iterator&      operator++();                                                           //!< The next item in the collection
        DGNPLATFORM_EXPORT MultilineStyleP const& operator* () const;                                                   //!< The value of the current item
        DGNPLATFORM_EXPORT bool                 operator!=(const_iterator const& rhs) const;                            //!< Compare two iterator values
        DGNPLATFORM_EXPORT bool                 operator==(const_iterator const& rhs) const {return !(*this != rhs);}   //!< Compare two iterator values
        DGNPLATFORM_EXPORT bool                 IsValid () const;                                                       //!< Test if the iterator is valid.
    };

    typedef const_iterator iterator;    //!< only const iteration is possible

    //!  Returns an iterator addressing the first item in the collection
    DGNPLATFORM_EXPORT const_iterator begin () const;

    //!  Returns an iterator that addresses the location succeeding the last item in collection
    DGNPLATFORM_EXPORT const_iterator end () const;
};

#endif

//__PUBLISH_SECTION_START__

/*=================================================================================**//**
This class represents the symbology of a multi-line profile or cap.
@ingroup MultilineElements
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultilineSymbology : RefCountedBase
{
//__PUBLISH_SECTION_END__
friend struct  MultilineStyle;
friend struct  MultilineProfile;
friend struct  MultilineHandler;

private:
    MlineProfile        m_profile;          // It's convenient to always store this but ignore the distance for caps
    LineStyleParams     m_lineStyleInfo;

protected:
    MultilineSymbology ();
    MultilineSymbology (MultilineSymbologyCR msprofile);
    MultilineSymbology (MlineSymbology const * profile, LineStyleParamsCP params);
    void Clear ();

public:

    MlineSymbology*         GetSymbology () {return &m_profile.symb;}
    MlineSymbology const &  GetSymbologyCR () const {return m_profile.symb;}
    void                    SetSymbology (MlineSymbology const & symb) {m_profile.symb = symb;}

DGNPLATFORM_EXPORT LineStyleParamsCR GetLinestyleParamsCR () const;

DGNPLATFORM_EXPORT void SetCustomLinestyleBit (bool val); // Shouldn't need this; only for legacy
DGNPLATFORM_EXPORT bool GetCustomLinestyleBit () const; // Shouldn't need this; only for legacy

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Duplicate a symbology
//! @param[in]      other  The symbology to copy
DGNPLATFORM_EXPORT void Copy (MultilineSymbologyCR other);

//! Determine if the profile or cap uses the locally defined level value
//! @return   True if the local level definition is used, false if the value from the element header is used.
//! @remarks  Level is a little different than other properties.  The value 0 is stored if the level is unused, 
//!             so use SetLevel(0) to use the value from the header.
DGNPLATFORM_EXPORT bool UsesLevel () const;

//! Set the level symbology.
DGNPLATFORM_EXPORT void SetLevel (LevelId level);

//! Get the Level Id from the symbology.
//! @return   The level Id, or 0 if the element level is used.
DGNPLATFORM_EXPORT LevelId GetLevel () const;

//! Set whether to use the line style from the symbology (true) or from the header (false).
//! @param[in]      val  The new value for the use line style symbology bit.
DGNPLATFORM_EXPORT void SetUseLinestyle (bool val);

//! Determine if the profile or cap uses the line style value defined in this symbology
//! @return   True if the local line style Id is used, false if the value from the element header is used.
DGNPLATFORM_EXPORT bool UsesLinestyle () const;

//! Set the line style symbology.  
//! @remarks You must also call SetUseLinestyle(true) for this line style to be used.
DGNPLATFORM_EXPORT void SetLinestyle (Int32 linestyleId);

//! Get the line style Id from the symbology.
//! @return   The line style Id
DGNPLATFORM_EXPORT Int32 GetLinestyle () const;

//! Get the line style parameters from the symbology
DGNPLATFORM_EXPORT LineStyleParamsP  GetLinestyleParams ();

//! Set the line style parameters in the symbology
DGNPLATFORM_EXPORT void SetLinestyleParams (LineStyleParams const & params);

//! Set whether to use the weight from the symbology (true) or from the header (false).
//! @param[in]      val  The new value for the use weight symbology bit.
DGNPLATFORM_EXPORT void SetUseWeight (bool val);

//! Determine if the profile or cap uses the weight value defined in this symbology
//! @return   True if the local weight is used, false if the value from the element header is used.
DGNPLATFORM_EXPORT bool UsesWeight () const;

//! Set the weight in the symbology
DGNPLATFORM_EXPORT void SetWeight (UInt32 weight);

//! Get the weight from the symbology
DGNPLATFORM_EXPORT UInt32 GetWeight () const;

//! Set whether to use the color from the symbology (true) or from the header (false).
//! @param[in]      val  The new value for the use color symbology bit.
DGNPLATFORM_EXPORT void SetUseColor (bool val);

//! Determine if the profile or cap uses the color value defined in this symbology
//! @return   True if the local color is used, false if the value from the element header is used.
DGNPLATFORM_EXPORT bool UsesColor () const;

//! Set the color in the symbology
DGNPLATFORM_EXPORT void SetColor (UInt32 color);

//! Get the color from the symbology
DGNPLATFORM_EXPORT UInt32 GetColor () const;

//! Set whether to use the class from the symbology (true) or from the header (false).
//! @param[in]      val  The new value for the use class symbology bit.
DGNPLATFORM_EXPORT void SetUseClass (bool val);

//! Determine if the profile or cap uses the class value defined in this symbology
//! @return   True if the local class is used, false if the value from the element header is used.
DGNPLATFORM_EXPORT bool UsesClass () const;

//! Set the class (drawing or construction) in the symbology
//! @param[in]      conclass  The new class.  The only valid values are DgnElementClass::Primary or DgnElementClass::Construction.
DGNPLATFORM_EXPORT void SetClass (DgnElementClass conclass);

//! Set the color in the symbology
DGNPLATFORM_EXPORT DgnElementClass GetClass () const;

//! Set whether to draw a line across the cap.  This is ignored on profiles.
DGNPLATFORM_EXPORT void SetCapLine (bool val);

//! Determine whether to draw a line across the cap.  This is ignored on profiles.
DGNPLATFORM_EXPORT bool UseCapLine () const;

//! Set whether to draw concentric arcs between profiles across the cap.  This is ignored on profiles.
DGNPLATFORM_EXPORT void SetCapInnerArc (bool val);

//! Determine whether to draw a concentric arcs across the cap.  This is ignored on profiles.
DGNPLATFORM_EXPORT bool UseCapInnerArc () const;

//! Set whether to draw an arc between the outermost profiles of the cap.  This is ignored on profiles.
DGNPLATFORM_EXPORT void SetCapOuterArc (bool val);

//! Determine whether to draw an arc between the outermost profiles of the cap.  This is ignored on profiles.
DGNPLATFORM_EXPORT bool UseCapOuterArc () const;

//! Set whether to use the profile color to define half the cap.  If this is set, a cap that goes between a red
//!  and blue profile will be red for 90 degrees and blue for 90 degrees.  This is ignored on profiles.
DGNPLATFORM_EXPORT void SetCapColorFromSegment (bool val);

//! Determine whether to use the profile color to define half the cap.  This is ignored on profiles.
DGNPLATFORM_EXPORT bool UseCapColorFromSegment () const;

};

/*=================================================================================**//**
This class represents a multi-line profile.  It consists of the symbology and an offset.
@ingroup MultilineElements
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultilineProfile : MultilineSymbology
{
//__PUBLISH_SECTION_END__

friend struct MultilineStyle;
friend struct MultilineHandler;

protected:
    MultilineProfile ();
    MultilineProfile (MultilineProfileCR msprofile);
    MultilineProfile (MlineProfile const * profile, LineStyleParamsCP params);
    
public:
DGNPLATFORM_EXPORT   static MultilineProfilePtr  Create (MlineProfile const * profile, LineStyleParams const * params);
//    void  Copy (MultilineSymbologyCR other);

DGNPLATFORM_EXPORT MlineProfile*           GetProfile ();
DGNPLATFORM_EXPORT MlineProfile const &    GetProfileCR () const;
DGNPLATFORM_EXPORT void                    SetProfile (MlineProfile const & profile);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
//! Create a profile
DGNPLATFORM_EXPORT static MultilineProfilePtr Create ();

//! Scale the offset in a profile
DGNPLATFORM_EXPORT void ScaleDistance (double scale);

//! Get the offset distance of a profile
DGNPLATFORM_EXPORT double GetDistance () const;

//! Set the offset distance of a profile.  
DGNPLATFORM_EXPORT void SetDistance (double dist);
};


/*=================================================================================**//**
Define a multi-line style.
@ingroup MultilineElements
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultilineStyle : RefCountedBase
{
//__PUBLISH_SECTION_END__
#ifdef DGN_IMPORTER_REORG_WIP
friend struct MultilineStyleCollection::const_iterator;
friend struct MlineStyleElementIterator;
friend struct MLineStyleDgnCacheLoaderCollection;
friend struct MlineStyleTableHandler;
#endif

private:
    bool                m_initialized;
    DgnProjectP            m_file;
    ElementId           m_elemID;  // This may go?  Not allowed to rely on it
    WString             m_name;

    double              m_orgAngle;    // Origin cap angle
    double              m_endAngle;    // End cap angle
    UInt32              m_numProfiles; // Number of lines per segment - vestige of when m_profiles was an array; should remove and use m_profiles.size()
    UInt32              m_fillColor;   // Color for filled multilines
    bool                m_isFilled;    // Fill

    bvector<MultilineProfilePtr>    m_profiles; // Limit to MLINE_MAX_POSSIBLE
    MultilineSymbology  m_orgCap;
    MultilineSymbology  m_endCap;
    MultilineSymbology  m_midCap;
    bool                m_isDefault;
    
    MultilineStyle ();    
    static ElementHandle    GetStyleElementById (ElementId id, DgnProjectR file);
    static BentleyStatus    UpgradeElement (EditElementHandleR styleElement, bool& wasChanged);

    void                    Clear ();
    BentleyStatus           ConvertToFile (DgnProjectR destFile);
    
protected:
    static MultilineStylePtr  CreateFromName (WCharCP name, DgnProjectR file);
    static MultilineStylePtr  CreateFromElement (ElementHandleCR styleElem);
    
#ifdef WIP_VANCOUVER_MERGE // mlinestyle
    static void               OnMlineStyleTransactionEvent (ElementHandleCP entryBefore, ElementHandleCP entryAfter, StyleEventType eventType, StyleEventSource source);
#endif

    static ElementHandle      GetSettingsElement (DgnProjectR file);
    static void               SetActiveStyleName (EditElementHandleR tableElm, WCharCP activeName);

    void                      SetElemID (ElementId elemID) { m_elemID = elemID; }

// Not published
public:

static BentleyStatus        UpgradeTable (EditElementHandleR styleElement, bool& wasChanged);

static ElementHandle    GetStyleElementByName (WCharCP name, DgnProjectR file);
static void             StaticInitialize ();

double GetUorScaleToModel (DgnModelP destCache) const;

DGNPLATFORM_EXPORT static WString       GetActiveStyleName (ElementHandleCR  tableElm);
DGNPLATFORM_EXPORT static ElementHandle GetTableElement (DgnProjectR file);
DGNPLATFORM_EXPORT static BentleyStatus GetTableElementForWrite (EditElementHandleR eeh, DgnProjectR file);  // Creates table if necessary
DGNPLATFORM_EXPORT static bool StyleExistsInFile (WCharCP name, DgnProjectR file);
DGNPLATFORM_EXPORT static WString GetNameFromId (ElementId styleId, DgnProjectR file);
DGNPLATFORM_EXPORT static ElementHandle GetFirstNamedStyleElement (DgnProjectR file);
#ifdef WIP_VANCOUVER_MERGE // mlinestyle
DGNPLATFORM_EXPORT static void SetTransactionListener (IMlineStyleTransactionListener* obj);
#endif
DGNPLATFORM_EXPORT MultilineSymbologyCR GetOrgCapCR () const;
DGNPLATFORM_EXPORT MultilineSymbologyCR GetEndCapCR () const;
DGNPLATFORM_EXPORT MultilineSymbologyCR GetMidCapCR () const;
DGNPLATFORM_EXPORT MultilineProfileCP GetProfileCP (UInt32 profileNum) const;
DGNPLATFORM_EXPORT BentleyStatus FromElement (ElementHandleCR styleElem);
DGNPLATFORM_EXPORT void ToElement (EditElementHandleR) const;
DGNPLATFORM_EXPORT bool IsSettings () const;
DGNPLATFORM_EXPORT void SetAsSettings (bool value);

DGNPLATFORM_EXPORT static bool      IsMlineStyleElement (DgnElementCP pCandidate);

DGNPLATFORM_EXPORT static BentleyStatus     RemapAllMlines (UInt32* numChanged, WCharCP oldStyleName, WCharCP newStyleName, DgnModelP modelRef, bool wholeFile);
DGNPLATFORM_EXPORT static bool              IsMlineStyleUsed (ElementId styleId, DgnProjectR dgnFile);

#ifdef WIP_VANCOUVER_MERGE // mlinestyle
MSCORE_EXPORT static void   AddListener (MlineStyleEvents& handler);
MSCORE_EXPORT static void   DropListener (MlineStyleEvents& handler);
#endif

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:


/*--------------------------------------------------------------------------------------**//**
Get a multi-line style from a file by Style ID.
@param[in]      id      The style ID of the style to get.
@param[in]      file    The file to look in for the style.
@return         Pointer to the style or NULL if it is not found.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT static MultilineStylePtr GetByID (ElementId id, DgnProjectR file);

/*--------------------------------------------------------------------------------------**//**
Get a multi-line style from a file by name.
@param[in]      name    The name of the style to get.
@param[in]      file    The file to look in for the style.
@return         Pointer to the style or NULL if it is not found.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT static MultilineStylePtr GetByName (WCharCP name, DgnProjectR file);

/*--------------------------------------------------------------------------------------**//**
Gets the settings multi-line style from a file.
@param[in]      file    The file to look in for the style.
@return         Pointer to the style or NULL if it is not found.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT static MultilineStylePtr GetSettings (DgnProjectR file);

/*--------------------------------------------------------------------------------------**//**
Gets the settings multi-line style from a file.
@param[in]      settings    The style to save as the active settings.
@param[in]      file    The file to look in for the style.
@return         Pointer to the style or NULL if it is not found.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT static BentleyStatus ReplaceSettings (MultilineStyleR settings, DgnProjectR file);

/*--------------------------------------------------------------------------------------**//**
Add a multi-line style to the file specified in the style.  The style must have at least one
  profile defined or it will be an error.
@return SUCCESS if the style is successfully added to the file.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT BentleyStatus Add (DgnProjectP file = NULL);

/*--------------------------------------------------------------------------------------**//**
Delete a multi-line style from the file specified in the style.
@param[in]      name   The name of the style to delete.
@param[in]      file   The file that contains the style.
@return SUCCESS if the style is successfully deleted from the file.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT static BentleyStatus Delete (WCharCP name, DgnProjectR file);

/*--------------------------------------------------------------------------------------**//**
Update a multi-line style in the file specified in the style.
@param[in]      oldName   When renaming a style, pass in the old style name to replace.  In the
                          common case where this parameter is NULL, it is assumed that the style retains its name.
@param[in]      destFile  The destination file.  Can be NULL to write to the file where the style was read.
@return SUCCESS if the style is successfully updated in the file.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT BentleyStatus Replace (WCharCP oldName, DgnProjectP destFile);

/*--------------------------------------------------------------------------------------**//**
Create a multi-line style object in memory.
@param[in]      name   A name for the style.
@param[in]      file   The file that the style will be associated with.
@return A pointer to the new style or NULL if creation failed.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT static MultilineStylePtr Create (WCharCP name, DgnProjectR file);

/*--------------------------------------------------------------------------------------**//**
Copy values from one multi-line style to another.
@param[in]      style   The style to copy values from.
@return SUCCESS if the values are copied as requested.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void CopyValues (MultilineStyleCR style);

/*--------------------------------------------------------------------------------------**//**
Compare two multi-line styles.
@param[in]      style   The style that will be compared.
@return A property mask with bits set describing which parameters are different.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT MultilineStylePropMaskPtr Compare (MultilineStyleCR style) const;

/*--------------------------------------------------------------------------------------**//**
Tests whether any elements store a persistent reference to the style.
@return   true if there are dependents.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT bool HasDependants () const;

/*--------------------------------------------------------------------------------------**//**
Remap all the elements that reference one style to different style.
@param    destStyle   IN  remap to this style.
@param    sourceStyle IN  remap from this style.
@param    file        IN  file in which to search.
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT static BentleyStatus RemapDependents (WCharCP destStyle, WCharCP sourceStyle, DgnProjectR file);

/*--------------------------------------------------------------------------------------**//**
Get the name of the multi-line style.
@return The name of the style. 
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT WString GetName () const;

/*--------------------------------------------------------------------------------------**//**
Get the ID of the multi-line style.
@return The ID of the style. 
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT ElementId GetID () const;

/*--------------------------------------------------------------------------------------**//**
Get the file associated with the multi-line style.
@return The file associated with the style. 
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT DgnProjectR GetFile () const;

/*--------------------------------------------------------------------------------------**//**
Change the in-memory name of the style.  This will not change the file until Replace is called.
@param[in]      name   The new name for the style.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT BentleyStatus SetName (WCharCP name);

/*--------------------------------------------------------------------------------------**//**
Scale the profile distances of the style.
@param[in]      scale   The scale factor for the distances.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void Scale (const double scale);

/*--------------------------------------------------------------------------------------**//**
Get the fill color from the style.  Note that a style can have a fill color but it won't show
unless the element is filled.  See GetFilled().
@return The fill color.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT UInt32 GetFillColor () const;

/*--------------------------------------------------------------------------------------**//**
Set the fill color of the .
@param[in]      value   The new fill color.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void SetFillColor (UInt32 value);

/*--------------------------------------------------------------------------------------**//**
Get the flag that determines if a multi-line is filled from the style.
@return The whether the multi-line is filled.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT bool GetFilled () const;

/*--------------------------------------------------------------------------------------**//**
Set the flag that determines if a multi-line is filled.
@param[in]      value   The new value for whether the multi-line is filled.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void SetFilled (bool value);

/*--------------------------------------------------------------------------------------**//**
Get the number of multi-line profile lines in the style.
@return The number of profile lines.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT UInt32 GetProfileLineCount () const;

/*--------------------------------------------------------------------------------------**//**
Get the origin cap angle for the multi-line style.
@return The angle of the origin cap line in radians.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT double GetOriginAngle () const;

/*--------------------------------------------------------------------------------------**//**
Set the origin cap angle for the multi-line style.
@param[in]      value   The new origin cap angle in radians.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void SetOriginAngle (double value);

/*--------------------------------------------------------------------------------------**//**
Get the end cap angle for the multi-line style.
@return The angle of the end cap line in radians.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT double GetEndAngle () const;

/*--------------------------------------------------------------------------------------**//**
Set the end cap angle for the multi-line style.
@param[in]      value   The new end cap angle in radians.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void SetEndAngle (double value);

/*--------------------------------------------------------------------------------------**//**
Get a pointer to a copy of the profile line in the style.
@param[in]      profileNum   The profile of interest.  Valid values are 0 to GetProfileLineCount()-1. 
@return A pointer to the profile object, or NULL if profileNum is out of range.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT MultilineProfilePtr GetProfile (UInt32 profileNum) const;

/*--------------------------------------------------------------------------------------**//**
Replace the profile at a given location.
@param[in]      profile      The new profile definition. 
@param[in]      profileNum   The profile of interest.  Valid values are 0 to GetProfileLineCount()-1. 
@return A pointer to the profile object, or NULL if profileNum is out of range.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT BentleyStatus ReplaceProfile (MultilineProfileCR profile, UInt32 profileNum);

/*--------------------------------------------------------------------------------------**//**
Add a profile line to the style.
@param[in]      profile   The profile to add. 
@param[in]      profileNum   The index to add the profile.  Use -1 to add it at the end.
@return SUCCESS if the profile is added.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT BentleyStatus InsertProfile (MultilineProfileCR profile, UInt32 profileNum);

/*--------------------------------------------------------------------------------------**//**
Remove a profile line from the style.
@param[in]      profileNum   The profile to remove.  Valid values are 0 to GetProfileLineCount()-1. 
@return SUCCESS if the profile is removed
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT BentleyStatus RemoveProfile (UInt32 profileNum);

/*--------------------------------------------------------------------------------------**//**
Empty all profile lines in the multi-line.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void ClearProfiles ();

/*--------------------------------------------------------------------------------------**//**
Get a pointer to a copy of the Origin Cap symbology from the style.
@return A pointer to the Origin Cap symbology.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT MultilineSymbologyPtr GetOrgCap () const;

/*--------------------------------------------------------------------------------------**//**
Replace the Origin Cap symbology of the style.
@param[in]      symb   The new symbology to use. 
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void SetOrgCap (MultilineSymbologyCR symb);

/*--------------------------------------------------------------------------------------**//**
Get a pointer to a copy of the End Cap symbology from the style.
@return A pointer to the End Cap symbology.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT MultilineSymbologyPtr GetEndCap () const;

/*--------------------------------------------------------------------------------------**//**
Replace the End Cap symbology of the style.
@param[in]      symb   The new symbology to use. 
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void SetEndCap (MultilineSymbologyCR symb);

/*--------------------------------------------------------------------------------------**//**
Get a pointer to a copy of the Mid or Joint Cap symbology from the style.
@return A pointer to the Mid or Joint Cap symbology.
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT MultilineSymbologyPtr GetMidCap () const;

/*--------------------------------------------------------------------------------------**//**
Replace the Mid or Joint Cap symbology of the style.
@param[in]      symb   The new symbology to use. 
@bsimethod
+--------------------------------------------------------------------------------------*/
DGNPLATFORM_EXPORT void SetMidCap (MultilineSymbologyCR symb);

};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
