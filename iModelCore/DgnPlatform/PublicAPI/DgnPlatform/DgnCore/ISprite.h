/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ISprite.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include    <Bentley/RefCounted.h>
//__PUBLISH_SECTION_END__
#include    <Bentley/BeIconUtilities.h>
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_RENDER_NAMESPACE

struct ISprite;
typedef RefCountedPtr<ISprite> ISpritePtr;

/*=================================================================================**//**
 @addtogroup Sprites
 Sprites are (typically) small raster images that are drawn "on top" of Viewports by an IViewDecoration.
 Their purpose is to draw the user's attention to something of importance.

 There are two classes in the Sprites subsystem: ISprite (a Sprite Definition) and SpriteLocation.
 Sprite Definitions are the images that define the way a type of sprite looks and are generally
 loaded one time and saved for the rest of a session. A SpriteLocation defines the current
 position of a single Sprite in a DgnViewport.

 A SpriteLocation can be either active or inactive. It becomes active by specifying a location
 (an x,y point) and a Sprite Definition to draw at that point. It should be obvious that a single Sprite
 Definition can be used many times by many Sprite Locations and that a single Sprite Location can
 change both position and which Sprite Definition is shown at that position over time.

 Sprites can be of varying sizes and color depths and can have both opaque and transparent pixels. 
 
 Element Manipulator handles and the Accusnap indicators are examples of  use of Sprites.
 @note It is also possible to draw an ISprite onto a DgnViewport directly (via calls to IViewDraw::DrawSprite)
 without ever using a SpritLocation. SpriteLocations are merely provided as a convenience.
 @beginGroup
 @bsiclass
+===============+===============+===============+===============+===============+======*/

/*=================================================================================**//**
 @bsiinterface
+===============+===============+===============+===============+===============+======*/
struct ISprite : IRefCounted
{
    //__PUBLISH_SECTION_END__
    //! Create a sprite that accesses the specified ICO resource in the specified DLL.
    //! @returns Pointer to new sprite object.
    //! @param[in] rscId  The ID of the icon resource in the DLL
    //! @param[in] hInstance Identifies the DLL from which the named BMP should be loaded or NULL for ustation.dll
    DGNPLATFORM_EXPORT static ISpriteP CreateFromIconResource(int rscId, BeIconUtilities::IconSourceP hInstance);

    //__PUBLISH_SECTION_START__

    //! Create an RGBA sprite from a buffer that contains a PNG image with alpha channel.
    //! @return the sprite definition or NULL if the PNG definition is not valid.
    DGNPLATFORM_EXPORT static ISpritePtr CreateFromPngBuffer(Byte const*inputBuffer, size_t numberBytes);

    //! Get the RGBA definition from this Sprite Definition.
    //! @return the RGBA definition or NULL
    virtual Byte const* GetRgbaDefinition() { return NULL; }

    //! Get the size (in pixels) of this Sprite Definition.
    //! @param        size        OUT the size in pixels of this sprite definition.
    virtual void GetSize(Point2d* size) = 0;
};


/*=================================================================================**//**
 A Sprite Location. Sprites generally move around on the screen and this object holds the current location
 and current Sprite Definition for an image of a sprite within a DgnViewport. SpriteLocations can be either
 inactive (not visible) or active.
 <p>A SpriteLocation can also specify that a Sprite Definition should be drawn partially transparent so that
 you can "see through" the Sprite.
 @bsiinterface
+===============+===============+===============+===============+===============+======*/
struct  SpriteLocation : public RefCountedBase
{
private:
    DgnViewportP       m_viewport;
    DPoint3d        m_location;
    int             m_transparency;
    ISpriteP        m_sprite;

public:
    DGNPLATFORM_EXPORT SpriteLocation();

    //! Activate this Sprite to show a specific Sprite Definition at a specific location in a DgnViewport.
    //! This call does \em not display the Sprite Definition in the DgnViewport. Rather, subsequent calls to
    //! #DecorateViewport from within an IViewDecoration \em will show the Sprite.
    //! This Sprite Location remains active until #Deactivate is called.
    //! @param[in]      sprite          The Sprite Definition to draw at this SpriteLocation
    //! @param[in]      viewport        The DgnViewport onto which the Sprite Definition is drawn
    //! @param[in]      location        The x,y posistion in DgnCoordSystem::View
    //! @param[in]      transparency    The transparency to draw the Sprite (0=opaque, 255=invisible)
    DGNPLATFORM_EXPORT void Activate(ISpriteP sprite, DgnViewportP viewport, DPoint3dCR location, int transparency);

    //! Deactivate an active Sprite Location. After this call, calls to #DecorateViewport for this
    //! Sprite Location will do nothing until it is re-Activated.
    DGNPLATFORM_EXPORT void Deactivate();

    //! If this Sprite Location is active for the specified DgnViewport, draw its Sprite Definition at the current location.
    //! Otherwise, this method does nothing.
    //! @note This method is <b>ONLY</b> valid from within an IViewDecoration::DoDecoration event callback.
    DGNPLATFORM_EXPORT void DecorateViewport(DgnViewportP);

    //! Determine whether this Sprite Location is currently active.
    //! @return true if this Sprite Location is currently active.
    bool IsActive() const {return NULL != m_viewport;}

    //! Get the sprite's location, if this SpriteLocation is active.
    DPoint3dR GetLocation() {return m_location;}

    //! Get the ISprite value, if this SpriteLocation is active.
    ISpriteP GetSprite()  {return m_sprite;}
};

/** @endGroup */
END_BENTLEY_RENDER_NAMESPACE
