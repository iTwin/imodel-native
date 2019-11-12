/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include    <Bentley/RefCounted.h>

BEGIN_BENTLEY_RENDER_NAMESPACE

struct ISprite;
typedef RefCountedPtr<ISprite> ISpritePtr;

//=======================================================================================
//! Sprites are (typically) small raster images that are drawn "on top" of Viewports by an IViewDecoration.
//! Their purpose is to draw the user's attention to something of importance.
//! <p>
//! There are two classes in the Sprites subsystem: ISprite (a Sprite Definition) and SpriteLocation.
//! Sprite Definitions are the images that define the way a type of sprite looks and are generally
//! loaded one time and saved for the rest of a session. A SpriteLocation defines the current
//! position of a single Sprite in a DgnViewport.
//! <p>
//! A SpriteLocation can be either active or inactive. It becomes active by specifying a location
//! (an x,y point) and a Sprite Definition to draw at that point. It should be obvious that a single Sprite
//! Definition can be used many times by many Sprite Locations and that a single Sprite Location can
//! change both position and which Sprite Definition is shown at that position over time.
//! <p>
//! Sprites can be of varying sizes and color depths and can have both opaque and transparent pixels. 
//! <p>
//! Element Manipulator handles and the Accusnap indicators are examples of  use of Sprites.
//! @note It is also possible to draw an ISprite onto a DgnViewport directly (via calls to IViewDraw::DrawSprite)
//! without ever using a SpritLocation. SpriteLocations are merely provided as a convenience.
//=======================================================================================
struct ISprite : IRefCounted
{
    //! Create an RGBA sprite from a buffer that contains a PNG image with alpha channel.
    //! @return the sprite definition or NULL if the PNG definition is not valid.
    DGNPLATFORM_EXPORT static ISpritePtr CreateFrom(Render::ImageSourceCR);

    //! Get the RGBA definition from this Sprite Definition.
    //! @return the RGBA definition or NULL
    virtual Byte const* _GetRgbaDefinition() {return nullptr;}

    //! Get the size (in pixels) of this Sprite Definition.
    //! @return the size in pixels of this sprite definition.
    virtual Point2d _GetSize() = 0;
};

END_BENTLEY_RENDER_NAMESPACE
