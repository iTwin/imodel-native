/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/LineStyleManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../DgnPlatformLib.h"

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"
#include <Bentley/RefCounted.h>
#include "LineStyle.h"

//__PUBLISH_SECTION_END__
typedef uint32_t RscFileHandle;
#include "../DgnCore/LsLocal.h"
#include <Bentley/WString.h>
#include <Bentley/bvector.h>
#include <map>

//__PUBLISH_SECTION_START__

/*=================================================================================**//**
  @addtogroup LineStyleManagerModule Line Style Manager

  @brief The Line Style Manager is the root for all line style access.

  <h2>Line Style Maps</h2>

  A line style can be defined in a resource file or a design file.  A design file may also contain references to
  line styles defined in resource files.  A resource file that contains line style definitions contains an LsMap
  that contains an LsMapEntry for each style.  A design file has an LsDgnFileMap that has an LsMapEntry for every
  line style that it may reference. Some of these entries may be for styles defined in the design file, while
  others are for styles defined in resource files.  A resource file should contain the complete line style definition for
  every line style named in an LsMapEntry from that file.

  An LsMapEntry is just a line style number and name.  For an LsMapEntry from a design file, the style number is unique
  to that entry.  A line style is associated with an element by storing that style number on the element. Therefore,
  the style number must be unique within that design file.  For an LsMapEntry in a resource file the style number
  must be unique if it is greater that STYLE_MaxLineCode. Typically, the style numbers in a resource file are less
  than or equal to 0 and are meaningless. See the section "Resolving a Line Style" for information on how line style numbers
  are used.

  When the LineStyleManager is initialized, it locates all "known"
  line styles on the system and creates an instance of LsSystemMap to serve as a system directory of the system line styles.
  The map entries in LsSystemMap are created from the map entries in resource files. If multiple resource files contain an entry
  with the same name only the first one is added to the map. In other words, a line style defined in a resource file is ignored if a line style
  with the same name has already been added to the map.

  The LineStyleManager creates an LsDgnFileMap whenever a design file is loaded.

  A program may use an LsResourceFileMap to read or create a resource file that contains line styles.  See LsResourceFileMap for more
  information.
  <h2>Line Style Definitions</h2>
  A line style can be defined in a resource file or a design file.  Th file that contains a definition must also contain
  an LsMapEntry for the definition. The LsMapEntry is used to find the line style definition, exposed in the LineStyleManager API
  as an LsDefinition. The LsDefinition contains parameters describing the line style, plus the information needed to find
  the line style components. Each component is represented by a class derived from LsComponent. All of the components
  required for a line style must be in the same file as the line style
  definition.  Within the file, each component is uniquely identified by a type and ID. Within a design file, the type is one
  of the values defined in LsElementType and the ID is a standard ElementId.  Within a resource file, the type is one of the
  values defined in LsResourceType and the ID is a resource ID represented by a 32-bit integer. For the most part, it is not
  important to be aware of the distinction.  The line style API always uses 64-bit integer for ID's.  LsComponent::GetResourceType
  returns the appropriate LsResourceType regardless of whether the component is defined in a resource file or design file.
  LsComponent::GetElementType always returns an appropriate value regardless of where the component is defined.

  The component types are:
  - LsCompoundComponent used to represent a collection of components.
  - LsSymbolComponent used to represent a collection of elements that define the representation of the line style
  - LsStrokePatternComponent used to control how the LsSymbolComponents in a LsPointComponent are stroked.
  - LsPointComponent used to associate an LsStrokePattern with a collection of LsSymbolReferences.  An LsSymbolReference
  contains a reference to an LsSymbolComponent and parameters controlling how it is used.

  <h2>Resolving a Line Style</h2>
  This section describes how the LineStyleManager uses the line style number from an element to find an LsDefinition.

  If the line style number is STYLE_ByLevel, the LineStyleManager uses the element's level
  to get the line style number to use for the rest of this process.

  If the style number is in the range STYLE_MinLineCode to STYLE_MaxLineCode, the LineStyleManager does not retrieve an LsDefinition.  Handling of these line styles
  is strictly defined and does not require an LsDefinition.   LineStyleManager does not do any special processing for STYLE_ByCell or STYLE_Invalid. It
  will treat them like normal style numbers, but it will fail to find the LsDefinition.

  If the style number is greater than STYLE_MaxLineCode, the LineStyleManager searches for an LsMapEntry in the LsSystemMap and, if that succeeds, it uses
  the LsDefinition that the LsMapEntry references. Note that line style numbers greater than STYLE_MaxLineCode are unusual.

  If the line style number is less than STYLE_MinLineCode or the search using the LsSystemMap failed, the LineStyleManager
  searches the design file's LsDgnFileMap to search for the LsMapEntry. If that fails, it is unable to resolve the line style.
  If it succeeds, it uses the LsDefinition obtained from LsMapEntry::GetLineStyle.  The LsDefinition may refer to a line style
  defined in that design file, or to a line style defined in the resource file.
 <h2>Lifetime of Line Style Objects</h2>
  The lifetime of most objects in the line style system is controlled by reference counting. However, line style objects under control of
  the system should be treated as if the program has no control over the lifetime of the object. These objects are guaranteed to survive as long as
  the current context is valid and they provide no useful purpose when the context is no longer valid.
  Therefore, the client code needs to retrieve references to the objects
  and immediately extract all of the necessary information.

  LsResourceFileMap objects exist until the program releases the last reference. The LsResourceFileMap holds a reference to every object created via
  CreateLineStyleDefinition.  See LsResourceFileMap for a discussion of how it controls the lifetime of objects created via it's Create... methods.
  @bsiclass
+===============+===============+===============+===============+===============+======*/
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
//! The LineStyleManager class provides access to all line style related services for DgnPlatform.
//! @ingroup LineStyleManagerModule
// @bsiclass                                                      John.Gooding    06/09
//=======================================================================================
struct          LineStyleManager
//__PUBLISH_SECTION_END__
                : DgnHost::IHostObject
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

private:
    void InitializeSystemMap ();

    void TerminateSystemMap ();
    void TerminateDgnFileMaps ();

    virtual void _OnHostTermination (bool isProcessShutdown) override {delete this;}

public:
    //  DGNPLATFORM_EXPORT void OnNewMasterFile (DgnDbP);  THESE SHOULD RESPOND TO NEW PROJECT
    //  DGNPLATFORM_EXPORT void OnDesignFileClosed (DgnDbP);
    DGNPLATFORM_EXPORT static LineStyleManagerR GetManager ();

    DGNPLATFORM_EXPORT static uint32_t ConvertToResourceType (LsElementType lsType);
    DGNPLATFORM_EXPORT static LsElementType ConvertToLsElementType (uint32_t resourceType);

    DGNPLATFORM_EXPORT void Initialize ();
    DGNPLATFORM_EXPORT void Terminate ();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Get the LsSystemMap.
    //! @return a reference to the LsSystemMap for the session.
    static LsSystemMapR GetSystemLineStyleMapR () {return LsSystemMap::GetSystemMapR (); }
    //! Get the LsSystemMap.
    //! @return a reference to the LsSystemMap for the session.
    static LsSystemMapCR GetSystemLineStyleMap () {return LsSystemMap::GetSystemMap (); }

    //!  Gets a name corresponding to the style number.  It will return an empty string if the line style number is a
    //!  line code (0-7) or ByLevel or ByCell.  If you want strings for those, use GetStringFromNumber().
    //!  @param[in]     styleNumber     A style number for a style known to the design file.
    //!  @param[in]     dgnFile         The DgnFile of interest.
    //!  @return The name of the style corresponding to the number. If the style number corresponds to an entry
    //!  in the design file's LsMap then this returns the name as stored in the map.  Otherwise, this returns a zero-length string.
    DGNPLATFORM_EXPORT static WString GetNameFromNumber (int32_t styleNumber, DgnDbP dgnFile);

    //!  Gets a name corresponding to the style number.  This method will return printable strings for line codes, ByLevel, and ByCell.
    //!  @param[in]     styleNumber     A style number for a style known to the design file.
    //!  @param[in]     project         The project of interest.
    //!  @return The name of the style corresponding to the number. If the style number corresponds to an entry
    //!  in the design file's LsMap then this returns the name as stored in the map.  If the style number is one of the standard
    //!  lines codes (in the range STYLE_MinLineCode through STYLE_MaxLineCode), this returns the string generated from the number.
    //!  If style number is STYLE_ByLevel, STYLE_ByCell, or, STYLE_Invalid this returns the name of the value.
    //!  For example, if style number is STYLE_ByLevel this returns "STYLE_ByLevel".  The string is not localized -- it is the
    //!  same for any locale.   Otherwise, this returns a zero-length string.
    DGNPLATFORM_EXPORT static WString GetStringFromNumber (int32_t styleNumber, DgnDbP project);

    //!  Gets the number corresponding to the line style name.  The case must be correct for the search to succeed.
    //!  This searches the design file's LsMap to find an entry with the specified name and returns the number that
    //!  that maps to the name.  This returns the appropriate strings for the standard line codes and for the special numbers
    //!  listed in LsKnownStyleNumber.
    //!  @param[in]     name            The name to use as the search key.
    //!  @param[in]     dgnFile         The DgnFile of interest.
    //!  @return style number if found, otherwise STYLE_Invalid.
    DGNPLATFORM_EXPORT static int64_t GetNumberFromName (Utf8CP name, DgnDbP dgnFile);

    //!  Gets the number corresponding to the line style name.  The case must be correct for the search to succeed.
    //!  This searches the design file's LsMap to find an entry with the specified name and returns the number that
    //!  that maps to the name.  This returns the appropriate strings for the standard line codes and for the special numbers
    //!  listed in LsKnownStyleNumber.
    //!  @param[in]     name            The name to use as the search key.
    //!  @param[in]     dgnFile         The DgnFile of interest.
    //!  @return style number if found, otherwise STYLE_Invalid.
    DGNPLATFORM_EXPORT static int64_t GetNumberFromName (WCharCP name, DgnDbP dgnFile);

#ifdef DGNV10FORMAT_CHANGES_WIP_LINESTYLES
    //!  Gets the number corresponding to the line style name.  The case must be correct for the search to succeed.
    //! @DotNetMethodExclude
    static int32_t GetNumberFromName (Utf8CP name, DgnModelP modelRef) {return GetNumberFromName (name, GetDgnDb(modelRef));}

    //! Gets the number corresponding to the line style name, importing the style to the design file if necessary.
    //! a given design file.
    //! @param[in]      name            The name of the line style.
    //!  @param[in]     dgnFile         The DgnFile of interest.
    //! @param[in]      createIdIfNecessary Add an entry to the file LsDgnFileMap if it is not already there
    //! @param[in]      transferComponents If adding an entry to the LsDgnFileMap, also copy components to the design file.
    //! @return style number if possible, otherwise STYLE_Invalid.
    DGNPLATFORM_EXPORT static int32_t GetNumberFromName (Utf8CP name, DgnDbR dgnFile, bool createIdIfNecessary, bool transferComponents);
#endif

    //! Uses the styleNumber to find a LsMapEntry in the design file's LsDgnFileMap by name and returns the LsDefinitionP the
    //! LsMapEntry references.  The LsDefinitionP may reference a definition from the design file or from the resource file.  If it is from
    //! a resource file, the style number in the definition is unlikely to match the specified style number. Entries from the
    //! resource file are normally selected by name and the style number is meaningless.
    //!  @param[in]     styleNumber     A style number for a style known to the design file.
    //!  @param[in]     dgnFile         The DgnFile of interest.
    //! @return the LsDefinition or NULL if not found.
    DGNPLATFORM_EXPORT static LsDefinitionP ResolveLineStyle (int32_t styleNumber, DgnDbP dgnFile);

    //! Find a line style in the LsSystemMap by name.  The LsSystemMap is initialized at startup and remains valid for the entire
    //! session. That is, for a given \c lineStyleName, this method will always return the same value if the line style exists.
    //! @param[in]      name            The name of the line style
    //! @return the LsDefinition or NULL if not found.  This returns NULL for the special names like "STYLE_ByLevel" and "1" that GetNumberFromName recognizes.
    DGNPLATFORM_EXPORT static LsDefinitionP FindSystemLineStyle (Utf8CP name);

    //! Reinitializes the line style maps.
    //! The steps to reinitializing are:
    //! \li Close all resource files that are associated with the existing system map.
    //! \li Free the system map and design file maps.
    //! \li Initialize the system map, calling _GetLocalLineStylePaths to get a new list of search patterns used to find
    //!     the RSC files.
    //! <p>The design file maps are loaded as needed.
    //! @return BSISUCCESS if successul, or BSIERROR otherwise. It fails if the LineStyleManager was not previously initialized, or if there
    //!         are any LsResourceFileMap maps open.
    DGNPLATFORM_EXPORT static BentleyStatus Reinitialize ();
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
