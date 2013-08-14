/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/ecimagekey.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECImageKey
    {
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
        //Constructor for generic icon
        ECImageKey (WStringCR name, ImageType type)
            :m_name(name), m_type(type), m_iconType(0), m_iconId(0)
            {}

        //Constructor for rsc icon
        ECImageKey (long iconId, UInt32 iconType, MdlDesc* mdlDesc)
            :m_type(RscIcon), m_iconType(iconType), m_iconId(iconId), m_mdlDesc(mdlDesc)
            {}

        ImageType   GetImageType() const {return m_type;}
        void        SetImageType(ImageType type) {m_type = type;}
        
        WStringCR   GetImageName() const {return m_name;}
        void        SetImageName (WStringCR name) {m_name = name;}

        RscId       GetIconId () const {return m_iconId;}
        void        SetIconId (RscId iconId) {m_iconId = iconId;}

        RscType     GetIconType() const {return m_iconType;}
        void        SetIconType (RscType type) {m_iconType = type;}

        MdlDesc*    GetMdlDesc() const {return m_mdlDesc;}
        void        SetMdlDesc(MdlDesc* desc) {m_mdlDesc = desc;}
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_END__*/