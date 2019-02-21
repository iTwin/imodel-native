/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Sprites.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ISprite.h"
#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_RENDER_NAMESPACE

//=======================================================================================
// @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
struct RgbaSprite : RefCounted<ISprite>
{
protected:
    bool            m_isLoaded;
    Render::Image   m_image;
    DGNPLATFORM_EXPORT virtual void _LoadSprite();
    void Load(Render::ImageSourceCR);

public:
    DGNPLATFORM_EXPORT RgbaSprite();
    DGNPLATFORM_EXPORT ~RgbaSprite();

    Point2d _GetSize() override {_LoadSprite(); Point2d pt; pt.x=m_image.GetWidth(); pt.y=m_image.GetHeight(); return pt;}
    Byte const* _GetRgbaDefinition() override;
    DGNPLATFORM_EXPORT static RgbaSpritePtr CreateFrom(Render::ImageSourceCR);
};

//=======================================================================================
// @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
struct NamedSprite : RgbaSprite
{
private:
    Utf8String m_namespace;
    Utf8String m_spriteName;
    NamedSprite(Utf8CP nameSpace, Utf8CP spriteName);

public:
    //! The buffer is required to be in RGBA format. The size is given by m_size.x * m_size.y
    DGNPLATFORM_EXPORT void _LoadSprite() override;
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
    DGNPLATFORM_EXPORT StaticSprite(Utf8CP nameSpace, Utf8CP spriteName);
    DGNPLATFORM_EXPORT ISpriteP GetISpriteP();
};

END_BENTLEY_RENDER_NAMESPACE
