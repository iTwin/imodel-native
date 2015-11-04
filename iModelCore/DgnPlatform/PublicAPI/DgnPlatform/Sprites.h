/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Sprites.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    "ISprite.h"
#include    <Bentley/RefCounted.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef RefCountedPtr<struct RgbaSprite> RgbaSpritePtr;
//=======================================================================================
// @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
struct RgbaSprite : public RefCounted<ISprite>
{
protected:
    Point2d         m_size;
    bool            m_isLoaded;

    bvector<Byte>   m_rgbaBuffer;  //  Expanded PNG
    DGNPLATFORM_EXPORT virtual void LoadSprite ();
    void PopulateRgbaSpriteFromPngBuffer(Byte const*inputBuffer, size_t numberBytes);

public:
    DGNPLATFORM_EXPORT RgbaSprite ();
    DGNPLATFORM_EXPORT ~RgbaSprite ();

    DGNPLATFORM_EXPORT void GetSize (Point2d* size) override;
    virtual Byte const* GetRgbaDefinition() override ;
    DGNPLATFORM_EXPORT static RgbaSpritePtr CreateFromPngBuffer(Byte const*inputBuffer, size_t numberBytes);

};

//=======================================================================================
// @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
struct NamedSprite : public RgbaSprite
{
private:
    Utf8String      m_namespace;
    Utf8String      m_spriteName;

    NamedSprite  (Utf8CP nameSpace, Utf8CP spriteName);

public:

    //! The buffer is required to be in RGBA format.  The size is given by m_size.x * m_size.y

    DGNPLATFORM_EXPORT virtual void LoadSprite () override;

    DGNPLATFORM_EXPORT static RgbaSpritePtr CreateFromPng(Utf8CP nameSpace, Utf8CP spriteName);
};

//=======================================================================================
// @bsiclass                                                     KeithBentley    10/02
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
