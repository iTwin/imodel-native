/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/ecimagekey.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <Bentley/WString.h>

typedef struct mdlDesc MdlDesc;

BEGIN_BENTLEY_EC_NAMESPACE

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
        long        m_iconId;
        UInt32      m_iconType;
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

        long    GetIconId () const {return m_iconId;}
        void    SetIconId (long iconId) {m_iconId = iconId;}

        UInt32  GetIconType() const {return m_iconType;}
        void    SetIconType (UInt32 type) {m_iconType = type;}

        MdlDesc*    GetMdlDesc() const {return m_mdlDesc;}
        void        SetMdlDesc(MdlDesc* desc) {m_mdlDesc = desc;}
    };

END_BENTLEY_EC_NAMESPACE