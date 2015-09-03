/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Sprites.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SpriteLocation::SpriteLocation ()
    {
    m_viewport = NULL;
    m_sprite   = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void    SpriteLocation::DecorateViewport (DgnViewportP viewport)
    {
    if (viewport == m_viewport)
        {
#define USE_WORLD_COORDSFOR_SPRITES 0
#if USE_WORLD_COORDSFOR_SPRITES
        //  On iOS this sometimes fails to draw.  Since sprites are always drawn in overlay
        //  mode I switched to using view coords.
        viewport->GetIViewDraw()->DrawSprite (m_sprite, &m_location, NULL, m_transparency);
#else
        DPoint3d loc;
        viewport->WorldToView (&loc, &m_location, 1);
        loc.z = 0;
        viewport->GetIViewDraw()->SetToViewCoords(true);
        viewport->GetIViewDraw()->DrawSprite (m_sprite, &loc, NULL, m_transparency);
        viewport->GetIViewDraw()->SetToViewCoords(false);
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void    SpriteLocation::Activate (ISpriteP sprite, DgnViewportP viewport, DPoint3dCR location, int transparency)
    {
    if (NULL == sprite || NULL == viewport)
        return;

    viewport->SetNeedsRefresh ();
    m_viewport = viewport;
    sprite->AddRef();
    m_sprite   = sprite;
    m_transparency = transparency;
    m_location     = location;

    viewport->WorldToNpc (&m_location, &m_location, 1);
    m_location.z = 0.0;
    viewport->NpcToWorld (&m_location, &m_location, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            SpriteLocation::Deactivate ()
    {
    if (!IsActive())
        return;

    m_viewport->SetNeedsRefresh ();
    m_viewport = NULL;
    m_sprite->Release();
    m_sprite   = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/10
+---------------+---------------+---------------+---------------+---------------+------*/
StaticSprite::StaticSprite (Utf8CP nameSpace, Utf8CP spriteName) : m_namespace(nameSpace), m_spriteName(spriteName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/10
+---------------+---------------+---------------+---------------+---------------+------*/
ISpriteP        StaticSprite::GetISpriteP ()
    {
    if (m_sprite.IsNull())
        {
        m_sprite = NamedSprite::CreateFromPng (m_namespace.c_str(), m_spriteName.c_str());
        }

    return m_sprite.get();
    }

/*=================================================================================**//**
* static sprite data
+===============+===============+===============+===============+===============+======*/
struct DgnCoreSprite : StaticSprite
    {
    DgnCoreSprite (Utf8CP spriteName) : StaticSprite ("dgncore", spriteName) {}
    };

static bvector<DgnCoreSprite*> s_keypointSprites;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void initKeypointSprites()
    {
    if (!s_keypointSprites.empty())
        return;

    s_keypointSprites.push_back (new DgnCoreSprite ("SnapPointOn"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapKeypoint"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapMidpoint"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapCenter"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapOrigin"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapBisector"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapIntersection"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapTangent"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapTangentPoint"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapPerpendicular"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapPerpendicularPoint"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapParallel"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapPoint"));
    s_keypointSprites.push_back (new DgnCoreSprite ("SnapPointOn"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
ISpriteP        SnapContext::GetSnapSprite (SnapMode snapMode)
    {
    KeypointType    keypointType = GetSnapKeypointType (snapMode);

    if (KEYPOINT_TYPE_Unknown <= keypointType)
        return NULL;

    initKeypointSprites();
    return s_keypointSprites[keypointType]->GetISpriteP ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2014
//---------------------------------------------------------------------------------------
void RgbaSprite::LoadSprite()
    {
    //  Base RgbaSprite does not support loading on demand, so it better already be loaded
    //  when this method is called.
    //
    //  NamedSprites are loaded on demand, but NamedSprite overrides LoadSprite.
    BeAssert(m_isLoaded);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2014
//---------------------------------------------------------------------------------------
ISpritePtr ISprite::CreateFromPngBuffer(Byte const*inputBuffer, size_t numberBytes)
    {
    RgbaSpritePtr ptr = RgbaSprite::CreateFromPngBuffer(inputBuffer, numberBytes);
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2014
//---------------------------------------------------------------------------------------
RgbaSpritePtr RgbaSprite::CreateFromPngBuffer(Byte const*inputBuffer, size_t numberBytes)
    {
    RgbaSprite* rgbaSprite = new RgbaSprite();
    rgbaSprite->PopulateRgbaSpriteFromPngBuffer(inputBuffer, numberBytes);
    return rgbaSprite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
void RgbaSprite::PopulateRgbaSpriteFromPngBuffer(Byte const*inputBuffer, size_t numberBytes)
    {
    ImageUtilities::RgbImageInfo info;
    if (BSISUCCESS != ImageUtilities::ReadImageFromPngBuffer (m_rgbaBuffer, info, inputBuffer, numberBytes) || !info.hasAlpha)
        {
        m_isLoaded = false;
        m_rgbaBuffer.clear();
        return;
        }

    m_size.x = (int)info.width;
    m_size.y = (int)info.height;
    m_isLoaded = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
NamedSprite::NamedSprite  (Utf8CP namespaceName, Utf8CP spriteName) : m_namespace(namespaceName), m_spriteName(spriteName)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
RgbaSpritePtr NamedSprite::CreateFromPng(Utf8CP nameSpace, Utf8CP spriteName)
    {
    return new NamedSprite(nameSpace, spriteName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
void NamedSprite::LoadSprite()
    {
    if (m_isLoaded)
        return;

    BeFileName pngPath;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
     if (T_HOST.GetGraphicsAdmin()._GetSpriteContainer(pngPath, m_namespace.c_str(), m_spriteName.c_str()) != BSISUCCESS)
        return;
#endif

    if (BeFileName::IsDirectory(pngPath.c_str()))
        {
        WString  spriteName;
        spriteName.AssignUtf8(m_spriteName.c_str());
        pngPath.AppendToPath(spriteName.c_str());
        pngPath.AppendExtension(L"png");

        BeFile   pngFile;
        if (BeFileStatus::Success != pngFile.Open(pngPath, BeFileAccess::Read))
            return;

        bvector<Byte> pngData;
        pngFile.ReadEntireFile(pngData);

        PopulateRgbaSpriteFromPngBuffer(&pngData[0], pngData.size());
        return;
        }

#if defined (LOADING_FROM_DATABASE)
    //  Otherwise treat it as a database
    BeSQLite::Db  pngDB;

    Db::OpenParams params (Db::OPEN_Readonly);
    params.SetRawSQLite();

    Utf8String  pngPathUtf8 = pngPath.GetNameUtf8();
    pngDB.OpenBeSQLiteDb(pngPathUtf8.c_str(), params);

    BeSQLite::SqlPrintfString  selectString("SELECT Value from Png_icons where Namespace='%s' And IconName='%s'", "DgnCore", m_spriteName.c_str());
    Statement stmt;
    stmt.Prepare(pngDB, selectString.GetUtf8CP());

    stmt.Step();

    int blobSize = (size_t)stmt.GetColumnBytes(0);
    BeAssert(blobSize > 0);
    Byte const* blobBytes = (Byte const*)stmt.GetValueBlob(0);
    CreateRgbaSpriteFromPngBuffer(blobBytes], (size_t)blobSize);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
RgbaSprite::~RgbaSprite ()
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    //  Destructor for statics called in the main thread. If the main thread is shutting down
    //  there is no need to do anything.
    if (DgnPlatformLib::QueryHost() != nullptr)
        // inform QVision that memory address being used as cached texture id no longer valid...
        T_HOST.GetGraphicsAdmin()._DeleteTexture ((uintptr_t) this);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
RgbaSprite::RgbaSprite ()
    {
    m_isLoaded = false;
    m_size.x = m_size.y = 0;
    }

Byte const* RgbaSprite::GetRgbaDefinition() { LoadSprite(); return m_rgbaBuffer.size() > 0 ? &m_rgbaBuffer[0] : nullptr; }
void RgbaSprite::GetSize (Point2d* size) {  LoadSprite(); *size = m_size;}

