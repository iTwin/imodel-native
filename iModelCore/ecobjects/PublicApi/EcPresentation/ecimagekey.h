/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/ecimagekey.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__PUBLISH_SECTION_START__*/

#pragma once
#include <Bentley/WString.h>
#include <ECObjects/ECObjects.h>

typedef struct mdlDesc  MdlDesc;
typedef UInt32          RscType;            /* Resource Type */
typedef Int32           RscId;              /* Resource Id */

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*=================================================================================**//**
//! ECImageKey holds an image information that can be used to get an actual image using
//! image services.
* @bsiclass                                     Abeesh.Basheer                  04/2012
+===============+===============+===============+===============+===============+======*/
struct ECImageKey
    {
    //! NEEDSWORK: Add comment here
    enum ImageType
        {
        Icon,
        Bitmap,
        Cursor,
        RscIcon
        };

    private:
        WString     m_name;
        ImageType   m_type;
        RscId       m_iconId;
        RscType     m_iconType;
        MdlDesc*    m_mdlDesc;

    public:
        //! Constructor for generic icon.
        //! @param[in] name          NEEDSWORK: Add comment here
        //! @param[in] type          NEEDSWORK: Add comment here
        ECImageKey (WStringCR name, ImageType type)
            :m_name(name), m_type(type), m_iconType(0), m_iconId(0)
            {}

        //! Constructor for rsc icon.
        //! @param[in] iconId        NEEDSWORK: Add comment here
        //! @param[in] iconType      NEEDSWORK: Add comment here
        //! @param[in] mdlDesc       NEEDSWORK: Add comment here
        ECImageKey (long iconId, UInt32 iconType, MdlDesc* mdlDesc)
            :m_type(RscIcon), m_iconType(iconType), m_iconId(iconId), m_mdlDesc(mdlDesc)
            {}

        //! NEEDSWORK: Add comment here
        ImageType   GetImageType() const {return m_type;}
        //! NEEDSWORK: Add comment here
        void        SetImageType(ImageType type) {m_type = type;}
        
        //! NEEDSWORK: Add comment here
        WStringCR   GetImageName() const {return m_name;}
        //! NEEDSWORK: Add comment here
        void        SetImageName (WStringCR name) {m_name = name;}

        //! NEEDSWORK: Add comment here
        RscId       GetIconId () const {return m_iconId;}
        //! NEEDSWORK: Add comment here
        void        SetIconId (RscId iconId) {m_iconId = iconId;}

        //! NEEDSWORK: Add comment here
        RscType     GetIconType() const {return m_iconType;}
        //! NEEDSWORK: Add comment here
        void        SetIconType (RscType type) {m_iconType = type;}

        //! NEEDSWORK: Add comment here
        MdlDesc*    GetMdlDesc() const {return m_mdlDesc;}
        //! NEEDSWORK: Add comment here
        void        SetMdlDesc(MdlDesc* desc) {m_mdlDesc = desc;}
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_END__*/