/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/Sprites.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    "ISprite.h"
#include    <Bentley/RefCounted.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef RefCountedPtr<struct RgbaSprite> RgbaSpritePtr;
//=======================================================================================
//! @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
struct RgbaSprite : public RefCounted<ISprite>
{
protected:
    Point2d         m_size;

    bvector<byte>   m_rgbaBuffer;  //  Expanded PNG
    DGNPLATFORM_EXPORT virtual void LoadSprite () = 0;

public:
    DGNPLATFORM_EXPORT RgbaSprite ();
    DGNPLATFORM_EXPORT ~RgbaSprite ();

    DGNPLATFORM_EXPORT void GetSize (Point2d* size) override;
    virtual byte const* GetRgbaDefinition() override ;

};

//=======================================================================================
//! @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
struct NamedSprite : public RgbaSprite
{
private:
    Utf8String      m_namespace;
    Utf8String      m_spriteName;
    bool            m_isLoaded;

    NamedSprite  (Utf8CP nameSpace, Utf8CP spriteName);

protected:
    void CreateRgbaSpriteFromPngBuffer(byte const*inputBuffer, size_t numberBytes);

public:

    //! The buffer is required to be in RGBA format.  The size is given by m_size.x * m_size.y

    DGNPLATFORM_EXPORT virtual void LoadSprite () override;

    DGNPLATFORM_EXPORT static RgbaSpritePtr CreateFromPng(Utf8CP nameSpace, Utf8CP spriteName);
};

//=======================================================================================
//! @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
struct StaticSprite
{
private:
    Utf8String      m_namespace;
    Utf8String      m_spriteName;
    RgbaSpritePtr   m_sprite;

public:
    DGNPLATFORM_EXPORT StaticSprite (Utf8CP nameSpace, Utf8CP spriteName);
    DGNPLATFORM_EXPORT ISpriteP GetISpriteP ();
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
