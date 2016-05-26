/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Sprites.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SpriteLocation::SpriteLocation()
    {
    m_viewport = nullptr;
    m_sprite   = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void SpriteLocation::DecorateViewport(DecorateContextR context)
    {
    if (context.GetViewport() == m_viewport)
        {
        DPoint3d loc = m_viewport->WorldToView(m_location);
        loc.z = 0;
        DPoint3d xVec = {1.0, 0.0, 0.0};
        context.AddSprite(*m_sprite, loc, xVec, m_transparency);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void SpriteLocation::Activate(ISpriteP sprite, DgnViewportP viewport, DPoint3dCR location, int transparency)
    {
    if (nullptr == sprite || nullptr == viewport)
        return;

    viewport->SetNeedsRefresh();
    m_viewport = viewport;
    sprite->AddRef();
    m_sprite   = sprite;
    m_transparency = transparency;
    m_location     = location;

    viewport->WorldToNpc(&m_location, &m_location, 1);
    m_location.z = 0.0;
    viewport->NpcToWorld(&m_location, &m_location, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void SpriteLocation::Deactivate()
    {
    if (!IsActive())
        return;

    m_viewport->SetNeedsRefresh();
    m_viewport = nullptr;
    m_sprite->Release();
    m_sprite = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/10
+---------------+---------------+---------------+---------------+---------------+------*/
StaticSprite::StaticSprite(Utf8CP nameSpace, Utf8CP spriteName) : m_namespace(nameSpace), m_spriteName(spriteName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/10
+---------------+---------------+---------------+---------------+---------------+------*/
ISpriteP StaticSprite::GetISpriteP()
    {
    if (m_sprite.IsNull())
        {
        m_sprite = NamedSprite::CreateFromPng(m_namespace.c_str(), m_spriteName.c_str());
        }

    return m_sprite.get();
    }

/*=================================================================================**//**
* static sprite data
+===============+===============+===============+===============+===============+======*/
struct DgnCoreSprite : StaticSprite
    {
    DgnCoreSprite(Utf8CP spriteName) : StaticSprite("dgncore", spriteName) {}
    };

static bvector<DgnCoreSprite*> s_keypointSprites;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void initKeypointSprites()
    {
    if (!s_keypointSprites.empty())
        return;

    s_keypointSprites.push_back(new DgnCoreSprite("SnapPointOn"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapKeypoint"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapMidpoint"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapCenter"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapOrigin"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapBisector"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapIntersection"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapTangent"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapTangentPoint"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapPerpendicular"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapPerpendicularPoint"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapParallel"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapPoint"));
    s_keypointSprites.push_back(new DgnCoreSprite("SnapPointOn"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
ISpriteP SnapContext::GetSnapSprite(SnapMode snapMode)
    {
    KeypointType keypointType = GetSnapKeypointType(snapMode);

    if (KEYPOINT_TYPE_Unknown <= keypointType)
        return nullptr;

    initKeypointSprites();
    return s_keypointSprites[keypointType]->GetISpriteP ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2014
//---------------------------------------------------------------------------------------
void RgbaSprite::_LoadSprite()
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
ISpritePtr ISprite::CreateFrom(ImageSourceCR source)
    {
    RgbaSpritePtr ptr = RgbaSprite::CreateFrom(source);
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2014
//---------------------------------------------------------------------------------------
RgbaSpritePtr RgbaSprite::CreateFrom(ImageSourceCR source)
    {
    RgbaSprite* rgbaSprite = new RgbaSprite();
    rgbaSprite->Load(source);
    return rgbaSprite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
void RgbaSprite::Load(ImageSourceCR source)
    {
    m_image = Image(source);
    m_isLoaded = m_image.IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
NamedSprite::NamedSprite(Utf8CP namespaceName, Utf8CP spriteName) : m_namespace(namespaceName), m_spriteName(spriteName)
    {
    }

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
void NamedSprite::_LoadSprite()
    {
    if (m_isLoaded)
        return;

    BeFileName pngPath;

    if (T_HOST.GetIKnownLocationsAdmin()._GetSpriteContainer(pngPath, m_namespace.c_str(), m_spriteName.c_str()) != BSISUCCESS)
        return;

    if (BeFileName::IsDirectory(pngPath.c_str()))
        {
        WString  spriteName;
        spriteName.AssignUtf8(m_spriteName.c_str());
        pngPath.AppendToPath(spriteName.c_str());
        pngPath.AppendExtension(L"png");

        BeFile   pngFile;
        if (BeFileStatus::Success != pngFile.Open(pngPath, BeFileAccess::Read))
            return;

        ByteStream stream;
        pngFile.ReadEntireFile(stream);
        ImageSource source(ImageSource::Format::Png, std::move(stream), ImageSource::Alpha::No);
        Load(source);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
RgbaSprite::~RgbaSprite()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
RgbaSprite::RgbaSprite()
    {
    m_isLoaded = false;
    }

Byte const* RgbaSprite::_GetRgbaDefinition() { _LoadSprite(); return m_image.GetByteStream().GetSize() > 0 ? m_image.GetByteStream().GetData() : nullptr; }

