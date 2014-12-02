/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/IDgnTextStyleApplyable.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCore/DgnTextStyle.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
//! Interface that TextBlock DOM property objects use to work with DgnTextStyle objects.
// @bsiclass                                                    Jeff.Marker     03/2010
//=======================================================================================
struct IDgnTextStyleApplyable
    {
//__PUBLISH_SECTION_END__
    protected:                      virtual bool            _HasTextStyle       () const = 0;
    protected:                      virtual UInt32          _GetTextStyleId     () const = 0;
    protected:                      virtual DgnTextStylePtr _GetTextStyleInFile () const = 0;
    protected:                      virtual void            _ToStyle            (DgnTextStyleR) const = 0;

    protected:                      virtual void            _ApplyTextStyle     (DgnTextStyleCR, bool respectOverrides) = 0;
    protected:                      virtual void            _SetProperties      (DgnTextStyleCR, DgnTextStylePropertyMaskCR) = 0;
    protected:                      virtual void            _RemoveTextStyle    () = 0;

    //! Retrieves the underlying file version of the style that this instance is based on.
    //! @note   This means the style object returned MAY NOT represent the property values stored in this instance. This is a convenience function which utilizes this instance's file to resolve a text style by the stored ID.
    public: DGNPLATFORM_EXPORT DgnTextStylePtr GetTextStyleInFile () const;

    //! Pushes this instance's values into the provided style.
    //! @note   This differs from GetTextStyleInFile in two ways: (1) the pushed property values are that of this instance, not necessarily that of the file version of the style (goverened by overrides), and (2) only this instance's properties are copied (which may be a subset of all text style properties).
    public: DGNPLATFORM_EXPORT void ToStyle (DgnTextStyleR) const;

    //! Pulls the masked properties from the provided style into this instance.
    //! @note   This differs from ApplyTextStyle in two ways: (1) the provided style is merely a property vessel, thus this method will NOT associate this instance with the provided style, and (2) only the properties identified by the mask are pulled. If this instance already has a style applied, the properties are still pulled, and overrides will be enabled for them.
    public: DGNPLATFORM_EXPORT void SetProperties (DgnTextStyleCR, DgnTextStylePropertyMaskCR);

    public: virtual ~IDgnTextStyleApplyable () { }

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! True if this instance has a text style associated with it.
    public: DGNPLATFORM_EXPORT bool HasTextStyle () const;

    //! Gets the text style ID. Zero implies no text style.
    public: DGNPLATFORM_EXPORT UInt32 GetTextStyleId () const;

    //! Applies a text style to this instance.
    //! @note   Normal operation is to provide a text style that is already in the file, and to set respectOverrides.<br><br>
    //! <table cellspacing="0" cellpadding="2" border="1">
    //!     <tr>
    //!         <td></td>
    //!         <td><b>Style Already in File</b></td>
    //!         <td><b>Style Not Already in File</b></td>
    //!     </tr>
    //!     <tr>
    //!         <td><b>respectOverrides</b></td>
    //!         <td>Properties Copied: <i>Non-Overridden</i><br>Overrides Cleared: <i>No</i><br>Associated: <i>Yes</i></td>
    //!         <td>Properties Copied: <i>All</i><br>Overrides Cleared: <i>Yes</i><br>Associated: <i>No</i></td>
    //!     </tr>
    //!     <tr>
    //!         <td><b>!respectOverrides</b></td>
    //!         <td>Properties Copied: <i>All</i><br>Overrides Cleared: <i>Yes</i><br>Associated: <i>Yes</i></td>
    //!         <td>Properties Copied: <i>All</i><br>Overrides Cleared: <i>Yes</i><br>Associated: <i>No</i></td>
    //!     </tr>
    //! </table>
    public: DGNPLATFORM_EXPORT void ApplyTextStyle (DgnTextStyleCR, bool respectOverrides);

    //! Removes any style association with this instance.
    public: DGNPLATFORM_EXPORT void RemoveTextStyle ();

    }; // IDgnTextStyleApplyable

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
