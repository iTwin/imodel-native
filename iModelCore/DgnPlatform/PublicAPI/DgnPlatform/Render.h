/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnModel.h"
#include "DgnCategory.h"
#include "Lighting.h"
#include "AreaPattern.h"
#include <Bentley/BeTimeUtilities.h>
#include <cmath>
#include <BRepCore/SolidKernel.h>

BEGIN_BENTLEY_RENDER_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
enum class RenderMode : int32_t
{
    Wireframe      = 0,
    HiddenLine     = 3,
    SolidFill      = 4,
    SmoothShade    = 6,
};

/*=================================================================================**//**
* Flags for view display style
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ViewFlags
{
private:
    RenderMode m_renderMode = RenderMode::Wireframe;
    uint32_t m_dimensions:1;       //!< Shows or hides dimensions.
    uint32_t m_patterns:1;         //!< Shows or hides pattern geometry.
    uint32_t m_weights:1;          //!< Controls whether non-zero line weights are used or display using weight 0.
    uint32_t m_styles:1;           //!< Controls whether custom line styles are used (e.g. control whether elements with custom line styles draw normally, or as solid lines).
    uint32_t m_transparency:1;     //!< Controls whether element transparency is used (e.g. control whether elements with transparency draw normally, or as opaque).
    uint32_t m_fill:1;             //!< Controls whether the fills on filled elements are displayed. Only applies in wireframe mode.
    uint32_t m_textures:1;         //!< Controls whether to display texture maps for material assignments. When off only material color is used for display.
    uint32_t m_materials:1;        //!< Controls whether materials are used (e.g. control whether geometry with materials draw normally, or as if it has no material).
    uint32_t m_acsTriad:1;         //!< Shows or hides the ACS triad.
    uint32_t m_grid:1;             //!< Shows or hides the grid. The grid settings are a design file setting.
    uint32_t m_visibleEdges:1;     //!< Shows or hides visible edges in the shaded render mode.
    uint32_t m_hiddenEdges:1;      //!< Shows or hides hidden edges in the shaded render mode.
    uint32_t m_sourceLights:1;     //!< Controls whether the source lights in spatial models are used
    uint32_t m_cameraLights:1;     //!< Controls whether camera (ambient, portrait, flashbulb) lights are used.
    uint32_t m_solarLight:1;       //!< Controls whether sunlight ussed
    uint32_t m_shadows:1;          //!< Shows or hides shadows.
    uint32_t m_noClipVolume:1;     //!< Controls whether the clip volume is applied.
    uint32_t m_constructions:1;    //!< Shows or hides construction class geometry.
    uint32_t m_monochrome:1;       //!< draw all graphics in a single color
    uint32_t m_noGeometryMap:1;    //!< ignore geometry maps
    uint32_t m_hLineMaterialColors:1; //!< use material colors for hidden lines
    uint32_t m_edgeMask:2;         //!< 0=none, 1=generate mask, 2=use mask
    uint32_t m_animate:1;          //!< Animate view (render continously).
    uint32_t m_backgroundMap:1;    //!< Background (web mercator) map.

public:
    BE_JSON_NAME(acs);
    BE_JSON_NAME(grid);
    BE_JSON_NAME(hidEdges);
    BE_JSON_NAME(clipVol);
    BE_JSON_NAME(noConstruct);
    BE_JSON_NAME(noDim);
    BE_JSON_NAME(noFill);
    BE_JSON_NAME(noCameraLights);
    BE_JSON_NAME(noSourceLights);
    BE_JSON_NAME(noSolarLight);
    BE_JSON_NAME(noMaterial);
    BE_JSON_NAME(noPattern);
    BE_JSON_NAME(noStyle);
    BE_JSON_NAME(noTexture);
    BE_JSON_NAME(noTransp);
    BE_JSON_NAME(noWeight);
    BE_JSON_NAME(renderMode);
    BE_JSON_NAME(shadows);
    BE_JSON_NAME(visEdges);
    BE_JSON_NAME(hlMatColors);
    BE_JSON_NAME(monochrome);
    BE_JSON_NAME(edgeMask);
    BE_JSON_NAME(animate);
    BE_JSON_NAME(backgroundMap);

    ViewFlags()
        {
        m_dimensions = 1;
        m_patterns = 1;
        m_weights = 1;
        m_styles = 1;
        m_transparency = 1;
        m_fill = 1;
        m_textures = 1;
        m_materials = 1;
        m_sourceLights = 1;
        m_cameraLights = 1;
        m_solarLight = 1;
        m_acsTriad = 0;
        m_grid = 0;
        m_visibleEdges = 0;
        m_hiddenEdges = 0;
        m_shadows = 0;
        m_noClipVolume = 0;
        m_constructions = 0;
        m_monochrome = 0;
        m_noGeometryMap = 0;
        m_hLineMaterialColors = 0;
        m_edgeMask = 0;
        m_animate = 0;
        m_backgroundMap = 0;
        }

    bool ShowDimensions() const {return m_dimensions;}
    void SetShowDimensions(bool val) {m_dimensions = val;}
    bool ShowPatterns() const {return m_patterns;}
    void SetShowPatterns(bool val) {m_patterns = val;}
    bool ShowWeights() const {return m_weights;}
    void SetShowWeights(bool val) {m_weights = val;}
    bool ShowStyles() const {return m_styles;}
    void SetShowStyles(bool val) {m_styles = val;}
    bool ShowTransparency() const {return m_transparency;}
    void SetShowTransparency(bool val) {m_transparency = val;}
    bool ShowFill() const {return m_fill;}
    void SetShowFill(bool val) {m_fill = val;}
    bool ShowTextures() const {return m_textures;}
    void SetShowTextures(bool val) {m_textures = val;}
    bool ShowMaterials() const {return m_materials;}
    void SetShowMaterials(bool val) {m_materials = val;}
    bool ShowAcsTriad() const {return m_acsTriad;}
    void SetShowAcsTriad(bool val) {m_acsTriad = val;}
    bool ShowGrid() const {return m_grid;}
    void SetShowGrid(bool val) {m_grid = val;}
    bool ShowVisibleEdges() const {return m_visibleEdges;}
    void SetShowVisibleEdges(bool val) {m_visibleEdges = val;}
    bool ShowHiddenEdges() const {return m_hiddenEdges;}
    void SetShowHiddenEdges(bool val) {m_hiddenEdges = val;}
    bool ShowSourceLights() const {return m_sourceLights;}
    void SetShowSourceLights(bool val) {m_sourceLights = val;}
    bool ShowCameraLights() const {return m_cameraLights;}
    void SetShowCameraLights(bool val) {m_cameraLights = val;}
    bool ShowSolarLight() const {return m_solarLight;}
    void SetShowSolarLight(bool val) {m_solarLight = val;}
    bool ShowShadows() const {return m_shadows;}
    void SetShowShadows(bool val) {m_shadows = val;}
    bool ShowClipVolume() const {return !m_noClipVolume;}
    void SetShowClipVolume(bool val) {m_noClipVolume = !val;}
    bool ShowConstructions() const {return m_constructions;}
    void SetShowConstructions(bool val) {m_constructions = val;}
    bool IsMonochrome() const {return m_monochrome;}
    void SetMonochrome(bool val) {m_monochrome = val;}
    bool IgnoreGeometryMap() const {return m_noGeometryMap;}
    void SetIgnoreGeometryMap(bool val) {m_noGeometryMap = val;}
    void SetUseHlineMaterialColors(bool val) {m_hLineMaterialColors = val;}
    bool UseHlineMaterialColors() const {return m_hLineMaterialColors;}
    int GetEdgeMask() const {return m_edgeMask;}
    void SetEdgeMask(int val) {m_edgeMask = val;}
    bool GetAnimate() const { return m_animate; }
    void SetAnimate(bool val) {m_animate = val;}
    bool ShowBackgroundMap() const { return m_backgroundMap; }
    void SetShowBackgroundMap(bool val) {m_backgroundMap = val;}


    RenderMode GetRenderMode() const {return m_renderMode; }
    void SetRenderMode(RenderMode value) {m_renderMode = value;}

    bool HiddenEdgesVisible() const
        {
        switch (m_renderMode)
            {
            case RenderMode::SolidFill:
            case RenderMode::HiddenLine:
                return m_hiddenEdges;

            case RenderMode::SmoothShade:
                return m_visibleEdges && m_hiddenEdges;

            default:
                return true;
            }
        }

    void InitDefaults() {*this = ViewFlags();}
    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    DGNPLATFORM_EXPORT void FromJson(JsonValueCR);
};

//=======================================================================================
//! Overrides a subset of ViewFlags.
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct ViewFlagsOverrides
{
private:
    enum PresenceFlag
    {
        kRenderMode,
        kText,
        kDimensions,
        kPatterns,
        kWeights,
        kStyles,
        kTransparency,
        kFill,
        kTextures,
        kMaterials,
        kVisibleEdges,
        kHiddenEdges,
        kSourceLights,
        kCameraLights,
        kSolarLight,
        kShadows,
        kClipVolume,
        kConstructions,
        kMonochrome,
        kGeometryMap,
        kHlineMaterialColors,
        kEdgeMask,
        kBackgroundMap,
    };

    uint32_t    m_present = 0;
    ViewFlags   m_values;

    void SetPresent(PresenceFlag flag) { m_present |= (1 << static_cast<uint32_t>(flag)); }
    bool IsPresent(PresenceFlag flag) const { return 0 != (m_present & (1 << static_cast<uint32_t>(flag))); }
public:
    //! Construct a ViewFlagsOverrides which overrides nothing
    ViewFlagsOverrides() { }

    //! Construct a ViewFlagsOverrides which overrides all flags to match the specified ViewFlags
    DGNPLATFORM_EXPORT explicit ViewFlagsOverrides(ViewFlags viewFlags);

    void SetShowDimensions(bool val) { m_values.SetShowDimensions(val); SetPresent(kDimensions); }
    void SetShowPatterns(bool val) { m_values.SetShowPatterns(val); SetPresent(kPatterns); }
    void SetShowWeights(bool val) { m_values.SetShowWeights(val); SetPresent(kWeights); }
    void SetShowStyles(bool val) { m_values.SetShowStyles(val); SetPresent(kStyles); }
    void SetShowTransparency(bool val) { m_values.SetShowTransparency(val); SetPresent(kTransparency); }
    void SetShowFill(bool val) { m_values.SetShowFill(val); SetPresent(kFill); }
    void SetShowTextures(bool val) { m_values.SetShowTextures(val); SetPresent(kTextures); }
    void SetShowMaterials(bool val) { m_values.SetShowMaterials(val); SetPresent(kMaterials); }
    void SetShowSourceLights(bool val) { m_values.SetShowSourceLights(val); SetPresent(kSourceLights); }
    void SetShowCameraLights(bool val) { m_values.SetShowCameraLights(val); SetPresent(kCameraLights); }
    void SetShowSolarLight(bool val) { m_values.SetShowSolarLight(val); SetPresent(kSolarLight); }
    void SetShowVisibleEdges(bool val) { m_values.SetShowVisibleEdges(val); SetPresent(kVisibleEdges); }
    void SetShowHiddenEdges(bool val) { m_values.SetShowHiddenEdges(val); SetPresent(kHiddenEdges); }
    void SetShowShadows(bool val) { m_values.SetShowShadows(val); SetPresent(kShadows); }
    void SetShowClipVolume(bool val) { m_values.SetShowClipVolume(val); SetPresent(kClipVolume); }
    void SetShowConstructions(bool val) { m_values.SetShowConstructions(val); SetPresent(kConstructions); }
    void SetMonochrome(bool val) { m_values.SetMonochrome(val); SetPresent(kMonochrome); }
    void SetIgnoreGeometryMap(bool val) { m_values.SetIgnoreGeometryMap(val); SetPresent(kGeometryMap); }
    void SetUseHlineMaterialColors(bool val) { m_values.SetUseHlineMaterialColors(val); SetPresent(kHlineMaterialColors); }
    void SetEdgeMask(int val) { m_values.SetEdgeMask(val); SetPresent(kEdgeMask); }
    void SetRenderMode(RenderMode val) { m_values.SetRenderMode(val); SetPresent(kRenderMode); }

    bool AnyOverridden() const { return 0 != m_present; }
    void Clear() { m_present = 0; }

    //! Apply these overrides to the supplied ViewFlags
    DGNPLATFORM_EXPORT void Apply(ViewFlags& base) const;
};

//=======================================================================================
//! An uncompressed image in Rgb (3 bytes per pixel) or Rgba (4 bytes per pixel) format suitable for rendering.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct Image
{
    enum class Format {Rgba=0, Rgb=2, Alpha=5}; // must match qvision.h values
    enum class BottomUp : bool {No=0, Yes=1}; //!< whether the rows in the image should be flipped top-to-bottom
protected:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    Format m_format = Format::Rgb;
    ByteStream m_image;

    void ClearData() {m_image.Clear();}
    void Initialize(uint32_t width, uint32_t height, Format format=Format::Rgb) {m_height=height; m_width=width; m_format=format; ClearData();}
    void ReadJpeg(uint8_t const* srcData, uint32_t srcLen, Format targetFormat, BottomUp bottomUp);
    void ReadPng(uint8_t const* srcData, uint32_t srcLen, Format targetFormat);

public:
    //! Construct a blank invalid Image
    Image() {}

    //! Construct an image from a ByteStream containing either Rgb or Rgba data.
    //! @param[in] width the width of the image in pixels
    //! @param[in] height the height of the image in pixels
    //! @param[in] image an rvalue reference to the ByteStream holding the data. The ByteStream is moved into the newly constructed Image.
    //! @param[in] format the format of the data held in image
    //! @note the ByteStream is moved by this constructor, so it will be empty after this call. To use this method, you must
    //! either pass a temporary variable or use std::move on a non-temporary variable.
    Image(uint32_t width, uint32_t height, ByteStream&& image, Format format) : m_width(width), m_height(height), m_format(format), m_image(std::move(image)) {}

    //! Construct an image from an ImageSource.
    //! @param[in] source the ImageSource from which the image is to be created.
    //! @param[in] targetFormat The format (Rgb or Rgba) for the new Image. If the source has an alpha channel and Rgb is requested, to alpha data is discarded. If
    //! the source does not have an alpha channel and Rgba is requested, all alpha values are set to 0xff.
    //! @param[in] bottomUp If Yes, the source image is flipped vertically (top-to-bottom) to create the image.
    //! @note If the source is invalid, or if the decompression fails, IsValid() will return false on the new Image.
    DGNPLATFORM_EXPORT explicit Image(ImageSourceCR source, Format targetFormat=Format::Rgba, BottomUp bottomUp=BottomUp::No);

    //! Create an Image from a Jpeg.
    //! @param[in] srcData the Jpeg data
    //! @param[in] srcLen  the number of bytes of Jpeg data
    //! @param[in] targetFormat The format (Rgb or Rgba) for the new Image. If the source has an alpha channel and Rgb is requested, to alpha data is discarded.
    //! If the source does not have an alpha channel and Rgba is requested, all alpha values are set to 0xff.
    //! @param[in] bottomUp If Yes, the source image is flipped vertically (top-to-bottom) to create the image.
    //! @return The decompressed Image, or an invalid Image if decompression failed.
    DGNPLATFORM_EXPORT static Image FromJpeg(uint8_t const* srcData, uint32_t srcLen, Format targetFormat=Format::Rgba, BottomUp bottomUp=BottomUp::No);

    //! Create an Image from a Png.
    //! @param[in] srcData the Png data
    //! @param[in] srcLen the number of bytes of Png data
    //! @param[in] targetFormat The format (Rgb or Rgba) for the new Image. If the source has an alpha channel and Rgb is requested, to alpha data is discarded.
    //! If the source does not have an alpha channel and Rgba is requested, all alpha values are set to 0xff.
    //! @return The decompressed Image, or an invalid Image if decompression failed.
    DGNPLATFORM_EXPORT static Image FromPng(uint8_t const* srcData, uint32_t srcLen, Format targetFormat=Format::Rgba);

    //! Create an Image by resizing a source Image.
    //! @param[in] width the width of the image in pixels
    //! @param[in] height the height of the image in pixels
    //! @param[in] sourceImage the source image
    DGNPLATFORM_EXPORT static Image FromResizedImage(uint32_t width, uint32_t height, ImageCR sourceImage);

    int GetBytesPerPixel()const {return m_format == Format::Rgba ? 4 : 3;} //!< get the number of bytes per pixel
    void Invalidate() {m_width=m_height=0; ClearData();} //!< Clear the contents and invalidate this image.
    uint32_t GetWidth() const {return m_width;} //!< Get the width of this image in pixels
    uint32_t GetHeight() const {return m_height;} //!< Get the height of this image in pixels
    Format GetFormat() const {return m_format;} //!< Get the format (Rgb or Rgba) of this image
    void SetFormat(Format format) {m_format=format;} //!< Change the format of this image in pixels
    bool IsValid() const {return 0!=m_width && 0!=m_height && 0!=m_image.GetSize();} //!< @return true if this image holds valid data
    ByteStream const& GetByteStream() const {return m_image;} //!< get a readonly reference to the ByteStream of this image
    ByteStream& GetByteStreamR() {return m_image;}//!< Get a writable reference to the ByteStream of this image
    void SetSize(uint32_t width, uint32_t height) {BeAssert(0 == m_width && 0 == m_height); m_width = width; m_height = height;} //!< change the size in pixels of this image
};

//=======================================================================================
//! A compressed image in either JPEG or PNG format. This is called a "source" because it is usually
//! stored externally and can be used to create an Image.
// @bsiclass                                                    Keith.Bentley   02/16
//=======================================================================================
struct ImageSource
{
    enum class Format : uint32_t {Jpeg=0, Png=2}; // don't change values, saved in DgnTexture elements

private:
    Format m_format = Format::Jpeg;
    ByteStream m_stream;

public:
    Format GetFormat() const {return m_format;} //!< Get the format of this ImageSource
    void SetFormat(Format format) {m_format=format;} //!< Change the format of this ImageSource
    ByteStream const& GetByteStream() const {return m_stream;} //!< Get a readonly reference to the ByteStream of this ImageSource.
    ByteStream& GetByteStreamR() {return m_stream;} //!< Get a writable reference to the ByteStream of this ImageSource.
    bool IsValid() const {return 0 < m_stream.GetSize();} //!< @return true if this ImageSource holds valid data
    DGNPLATFORM_EXPORT Point2d GetSize() const; //!< Reads and returns the width and height of this ImageSource

    //! Construct a blank invalid ImageSource
    ImageSource() {}

    //! Construct an ImageSource from a ByteStream containing either Jpeg or Png data.
    //! @param[in] format the format of the data held in stream
    //! @param[in] stream an rvalue reference to a ByteStream holding the data. The ByteStream is moved into the newly constructed ImageSource.
    //! @note the ByteStream is moved by this constructor, so it will be empty after this call. To use this method, you must
    //! either pass a temporary variable or use std::move on a non-temporary variable.
    explicit ImageSource(Format format, ByteStream&& stream) : m_format(format), m_stream(stream) {}

    //! Construct an ImageSource by compressing the pixels of an Image. The compression is done using either Jpeg or Png format.
    //! @param[in] image the Rgb or Rgba image data to compress to create the ImageSource
    //! @param[in] format the format (either Jpeg or Png) to compress the image
    //! @param[in] quality a value between 1 and 100 to control the level of compression. Used only if format==Jpeg.
    //! @param[in] bottomUp If Yes, the image is flipped vertically (top-to-bottom) in the new ImageSource.
    //! @note If the image is invalid, or if the compression fails, IsValid() will return false on the new ImageSource.
    DGNPLATFORM_EXPORT explicit ImageSource(ImageCR image, Format format, int quality=100, Image::BottomUp bottomUp=Image::BottomUp::No);
};

//=======================================================================================
//! An uncompressed high definition image in either RGBE or RGBM
// @bsiclass                                                    Ray.Bentley     01/2018
//=======================================================================================
struct HDRImage : Image
{
    enum class Encoding : uint32_t {RGBE = 0, RGBM = 1, RGBD = 2};

private:
    Encoding        m_encoding;

public:
    //! Create an HDRImage from a (Radiance) HDR data.
    //! @param[in] srcData the HDR data
    //! @param[in] srcLen the number of bytes of HDR data
    //! @param[in] encoding the encoding (either RGBM or RGBE)
    DGNPLATFORM_EXPORT static HDRImage FromHDR(uint8_t const* srcData, uint32_t srcLen, Encoding encoding = Encoding::RGBM);
    bvector<float> Decode () const;
    void Encode(Encoding encoding);
};

//=======================================================================================
//! Identifies a texture or material.
//! A persistent resource is identified by its element ID within the DgnDb.
//! A named resource is identified by a string ID which is unique among all such resources
//! associated with a given DgnDb.
//! A resource with a valid name can be constructed once and then looked up again later by
//! name.
//! An unnamed resource is created for one-time use and cannot be looked up again for reuse.
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
template <typename T_Id> struct ResourceKey
{
private:
    T_Id        m_id;
    Utf8String  m_name;
public:
    explicit ResourceKey(T_Id id=T_Id()) : m_id(id) { }
    explicit ResourceKey(Utf8StringCR name) : m_name(name) { }
    ResourceKey(ResourceKey const&) = default;
    ResourceKey& operator=(ResourceKey const&) = default;

    bool IsPersistent() const { return m_id.IsValid(); }
    bool IsNamed() const { return !m_name.empty(); }
    bool IsValid() const { return IsPersistent() || IsNamed(); }

    T_Id GetId() const { BeAssert(IsPersistent()); return m_id; }
    Utf8StringCR GetName() const { BeAssert(IsNamed()); return m_name; }

    bool operator!=(ResourceKey const& rhs) const { return !(*this == rhs); }
    bool operator==(ResourceKey const& rhs) const
        {
        if (IsPersistent())
            return rhs.IsPersistent() && GetId() == rhs.GetId();
        else if (IsNamed())
            return rhs.IsNamed() && GetName().Equals(rhs.GetName());
        else
            return false;
        }

    bool operator<(ResourceKey const& rhs) const
        {
        BeAssert(IsValid());
        if (IsPersistent())
            return rhs.IsPersistent() ? GetId() < rhs.GetId() : true;
        else if (IsNamed())
            return rhs.IsNamed() ? GetName().CompareTo(rhs.GetName()) < 0 : !rhs.IsPersistent();
        else
            return false;
        }

    //! @private
    Utf8String ToDebugString() const
        {
        if (IsNamed())
            return GetName();
        else if (IsPersistent())
            return GetId().ToHexStr();
        else
            return "<unnamed>";
        }
};

using TextureKey = ResourceKey<DgnTextureId>;
using MaterialKey = ResourceKey<RenderMaterialId>;

DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(TextureKey);
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(MaterialKey);

//=======================================================================================
//! A Texture for rendering
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct Texture : RefCounted<NonCopyableClass>
{
    struct CreateParams
    {
        TextureKey m_key;
        int m_pitch = 0;
        bool m_isTileSection = false;
        bool m_isGlyph = false;
        bool m_isRGBE = false;;      // HDR stored with exponent (or multiplier).

        TextureKeyCR GetKey() const { return m_key; }

        void SetIsTileSection() {m_isTileSection=true;}
        void SetPitch(int val) {m_pitch=val;}
        void SetIsRGBE() { m_isRGBE = true; }

        explicit CreateParams(TextureKeyCR key=TextureKey()) : m_key(key) { }
    };

    struct Dimensions
    {
        uint32_t width;
        uint32_t height;

        Dimensions(uint32_t w = 0, uint32_t h = 0) : width(w), height(h) { }
    };
protected:
    TextureKey m_key;
    bool m_isGlyph;
    bool m_isTileSection;

    uint32_t _GetExcessiveRefCountThreshold() const override {return 100000;}

    explicit Texture(CreateParams const& params) : m_key(params.m_key), m_isGlyph(params.m_isGlyph), m_isTileSection(params.m_isTileSection) { }
public:
    TextureKeyCR GetKey() const { return m_key; }
    bool IsGlyph() const { return m_isGlyph; }
    bool IsTileSection() const { return m_isTileSection; }

    // Named textures should preserve their image data so it can be obtained later.
    virtual ImageSourceCP GetImageSource() const { return nullptr; }
    virtual Dimensions GetDimensions() const = 0;
};

//=======================================================================================
//! Represents a texture and a description of how to map the texture to geometry.
// @bsistruct                                                   Paul.Connelly   08/17
//=======================================================================================
struct TextureMapping
{
    enum class Mode : int
    {
        None              = -1,
        Parametric        = 0,
        ElevationDrape    = 1,
        Planar            = 2,
        DirectionalDrape  = 3,
        Cubic             = 4,
        Spherical         = 5,
        Cylindrical       = 6,
        Solid             = 7,
        FrontProject      = 8, //<! Only valid for lights.
    };

    //=======================================================================================
    //! A 2x3 2d transform applied to UV parameters.
    //! See RenderingAsset::Trans2x3Builder.
    // @bsistruct                                                   Paul.Connelly   01/19
    //=======================================================================================
    struct Trans2x3
    {
        double m_val[2][3];
        Trans2x3() {}
        Trans2x3(double t00, double t01, double t02, double t10, double t11, double t12)
            {
            m_val[0][0] = t00;
            m_val[0][1] = t01;
            m_val[0][2] = t02;
            m_val[1][0] = t10;
            m_val[1][1] = t11;
            m_val[1][2] = t12;
            }

        static Trans2x3 FromIdentity() { return Trans2x3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0); }
        void InitIdentity() { *this = FromIdentity(); }
        bool IsIdentity() const
            {
            return 1.0 == m_val[0][0] && 0.0 == m_val[0][1] && 0.0 == m_val[0][2]
                && 0.0 == m_val[1][0] && 1.0 == m_val[1][1] && 0.0 == m_val[1][2];
            }

        bool AlmostEqual(Trans2x3 const& rhs) const;

        Transform GetTransform() const;

        Json::Value ToJson() const;
        static Trans2x3 FromJson(JsonValueCR);
    };

    //=======================================================================================
    //! Parameters controlling the material projection.
    // @bsistruct                                                   Paul.Connelly   01/19
    //=======================================================================================
    struct ProjectionInfo
    {
        // ###TODO: These are just placeholders for now...
        DVec3d m_offset, m_scale, m_rotation;
    };

    struct Params
    {
        Trans2x3 m_textureMat2x3;
        double m_textureWeight;
        Mode m_mapMode;
        bool m_worldMapping;

        explicit Params(Mode mode=Mode::Parametric, Trans2x3 const& trans=Trans2x3::FromIdentity(), double weight=1.0, bool worldMapping=false)
            : m_textureMat2x3(trans), m_textureWeight(weight), m_mapMode(mode), m_worldMapping(worldMapping) { }

        void SetMode(Mode val) {m_mapMode=val;}
        void SetWeight(double val) {m_textureWeight = val;} //<! Set weight for combining diffuse image and color
        void SetTransform(Trans2x3 const* val) {m_textureMat2x3 = nullptr != val ? *val : Trans2x3();} //<! Set Texture 2x3 transform
        void SetWorldMapping(bool val) {m_worldMapping = val;} //! if true world mapping, false for surface

        DGNPLATFORM_EXPORT BentleyStatus ComputeUVParams (bvector<DPoint2d>& params, PolyfaceVisitorCR visitor, TransformCP transformToDgn = nullptr) const;
    };

private:
    TextureCPtr m_texture;
    Params      m_params;
public:
    TextureMapping(TextureCR texture, Params const& params) : m_texture(&texture), m_params(params) { }
    explicit TextureMapping(TextureCP texture=nullptr) : m_texture(texture) { }
    explicit TextureMapping(TextureCR texture) : TextureMapping(&texture) { }

    bool IsValid() const { return m_texture.IsValid(); }
    TextureCP GetTexture() const { return m_texture.get(); }
    Params const& GetParams() const { return m_params; }
    void SetTransform(Trans2x3 const& transform) { m_params.m_textureMat2x3 = transform; }
};

//=======================================================================================
//! Additional parameters controlling how a material's UV params are computed.
//! For materials using one of the projection TextureMapping modes, a ProjectionInfo is
//! required.
//! For non-projection mapping modes, an optional transform can be supplied.
//! when computing UV params, a base 2d transform is computed from the material properties.
//! If the MaterialUVDetail defines its own transform, it overrides the material's
//! computed transform.
//! See RenderingAsset::Trans2x3Builder for a convenient way to construct or modify such
//! a transform.
// @bsistruct                                                   Paul.Connelly   01/19
//=======================================================================================
struct MaterialUVDetail
{
private:
    enum class Type { None, Transform, Projection };
    Type    m_type;
    union
        {
        TextureMapping::Trans2x3        m_transform;
        TextureMapping::ProjectionInfo  m_projection;
        };
public:
    MaterialUVDetail() : m_type(Type::None) { }

    bool HasTransform() const { return Type::Transform == m_type; }
    bool HasProjection() const { return Type::Projection == m_type; }
    bool HasDetail() const { return Type::None != m_type; }

    TextureMapping::Trans2x3 const* GetTransform() const { return HasTransform() ? &m_transform : nullptr; }
    TextureMapping::ProjectionInfo const* GetProjection() const { return HasProjection() ? &m_projection : nullptr; }

    void Clear() { m_type = Type::None; }
    void SetTransform(TextureMapping::Trans2x3 const& transform)
        {
        if (!transform.IsIdentity())
            {
            m_type = Type::Transform;
            m_transform = transform;
            }
        else
            {
            Clear();
            }
        }
    void SetProjection(TextureMapping::ProjectionInfo const& projection) { m_type = Type::Projection; m_projection = projection; }

    bool IsEquivalent(MaterialUVDetailCR rhs) const;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct Material : RefCounted<NonCopyableClass>
{
    struct Transparency
    {
        enum Category { None, Opaque, Translucent };
    private:
        double  m_value;
    public:
        explicit Transparency(double value = -1.0) { SetValue(value); }

        Category Categorize() const { return m_value < 0.0 ? Category::None : (m_value > 0.0 ? Category::Translucent : Category::Opaque); }
        bool IsTranslucent() const { return Translucent == Categorize(); }
        bool IsOpaque() const { return Opaque == Categorize(); }
        bool IsOverridden() const { return None != Categorize(); }

        void SetOpaque() { SetValue(0.0); }
        void SetNotOverridden() { SetValue(-1.0); }

        double GetValue() const { return m_value; }
        void SetValue(double value)
            {
            if (value < 0.0)
                value = -1.0;
            else if (value > 1.0)
                value = 1.0;

            m_value = value;
            }

        CompareResult Compare(Transparency other) const
            {
            auto a = Categorize(), b = other.Categorize();
            return a == b ? CompareResult::Equal : (a < b ? CompareResult::Less : CompareResult::Greater);
            }
    };

    // QVision defaults...
    struct Defaults
    {
        // From DgnViewMaterial.cpp...(QVision defaults)
        static constexpr double ExponentMultiplier() { return 15.0; }
        static constexpr double Finish() { return 0.9; }
        static constexpr double Specular() { return 0.4; }
        static constexpr double Diffuse() { return 0.6; }
        static constexpr double Reflect() { return 0.0; }
        static constexpr double SpecularExponent() { return Finish() * ExponentMultiplier(); }
    };

    struct CreateParams
    {
        using Defaults = Material::Defaults;

        // NB: The alpha value is ignored.
        struct MatColor
        {
            bool m_valid = false;
            ColorDef m_value;
            MatColor(){}
            MatColor(ColorDef val) {m_valid=true; m_value=val;}
            bool IsValid() const {return m_valid;}

            DGNPLATFORM_EXPORT CompareResult Compare(MatColor const& other) const;

            bool operator==(MatColor const& other) const { return CompareResult::Equal == Compare(other); }
            bool operator!=(MatColor const& other) const { return CompareResult::Equal != Compare(other); }
            bool operator<(MatColor const& other) const { return CompareResult::Less == Compare(other); }
        };

        MatColor m_diffuseColor;
        MatColor m_specularColor;
        MatColor m_emissiveColor;
        MatColor m_reflectColor;
        TextureMapping m_textureMapping;
        MaterialKey m_key;
        double m_diffuse = Defaults::Diffuse();
        double m_specular = Defaults::Specular();
        double m_specularExponent = Defaults::SpecularExponent();
        double m_reflect = Defaults::Reflect();
        Transparency m_transparency;
        double m_refract = 1.0;
        double m_ambient = .3;
        bool   m_shadows = true;

        explicit CreateParams(MaterialKeyCR key=MaterialKey()) : m_key(key) { }
        DGNPLATFORM_EXPORT CreateParams(MaterialKeyCR key, RenderingAssetCR, DgnDbR, SystemCR, TextureP texture=nullptr);

        void SetDiffuseColor(ColorDef val) {m_diffuseColor = val;} //<! Set the surface color for fill or diffuse illumination
        void SetSpecularColor(ColorDef val) {m_specularColor = val;} //<! Set the surface color for specular illumination
        void SetEmissiveColor(ColorDef val) {m_emissiveColor = val;} //<!  Set the surface emissive color
        void SetReflectColor(ColorDef val) {m_reflectColor = val;} //<!  Set the surface reflectance color
        void SetDiffuse(double val) {m_diffuse = val;} //<! Set surface diffuse reflectivity
        void SetAmbient(double val) {m_ambient = val;} //<! Set surface ambient reflectivity
        void SetSpecularExponent(double val) {m_specularExponent = val;} //<! Set surface shininess (range 0 to 128)
        void SetReflect(double val) {m_reflect = val;} //<! Set surface environmental reflectivity
        void SetSpecular(double val) {m_specular = val;} //<! Set surface specular reflectivity
        void SetRefract(double val) {m_refract = val;} //<! Set index of refraction
        void SetShadows(bool val) {m_shadows = val;} //! If false, do not cast shadows
        void MapTexture(TextureMappingCR mapping) {m_textureMapping=mapping;}
        void MapTexture(TextureCR texture, TextureMapping::Params const& params) {MapTexture(TextureMapping(texture, params));}
    };

protected:
    CreateParams    m_params;

    uint32_t _GetExcessiveRefCountThreshold() const override {return 100000;}

    explicit Material(CreateParams const& params) : m_params(params) { }
public:
    bool HasTextureMapping() const {return m_params.m_textureMapping.IsValid();}
    TextureMappingCR GetTextureMapping() const {return m_params.m_textureMapping;}
    MaterialKeyCR GetKey() const {return m_params.m_key;}
    CreateParams const& GetParams() const {return m_params;}
};

//=======================================================================================
//! Line style parameters
//! @private
//=======================================================================================
struct LineStyleParams
{
    uint32_t    modifiers;      /* see STYLEMOD_... in LineStyleResource.r.h */
    uint32_t    reserved;
    double      scale;          /* Applied to all length values        */
    double      dashScale;      /* Applied to adjustable dash strokes  */
    double      gapScale;       /* Applied to adjustable gap strokes   */
    double      startWidth;     /* Taper start width                   */
    double      endWidth;       /* Taper end width                     */
    double      distPhase;      /* Phase shift by distance             */
    double      fractPhase;     /* Phase shift by fraction             */
    uint32_t    lineMask;       /* Multiline line mask                 */
    uint32_t    mlineFlags;     /* Multiline flags                     */
    DPoint3d    normal;
    RotMatrix   rMatrix;

    void Init()
        {
        memset(this, 0, sizeof(LineStyleParams));
        this->rMatrix.form3d[0][0] = this->rMatrix.form3d[1][1] = this->rMatrix.form3d[2][2] =
        this->scale = this->gapScale = this->dashScale = this->normal.z = 1.0;
        }

    //! Compare two LineStyleParams.
    DGNPLATFORM_EXPORT bool operator==(LineStyleParamsCR rhs) const;
    void SetScale(double inScale) {modifiers |= 0x01; scale = inScale;}
    DGNPLATFORM_EXPORT void ApplyTransform(TransformCR transform, uint32_t options = 0);
};

//=======================================================================================
//! This structure contains options (modifications) that can be applied
//! to existing line styles to change their appearance without changing the line style
//! definition. Most of the options pertain to the operation of the StrokePatternComponent
//! component but the plane definition and scale factors can be used by all components.
//=======================================================================================
struct LineStyleSymb
{
private:
    // NOTE: For performance, the constructor initializes members using:
    //         memset (&m_lStyle, 0, offsetof (LineStyleSymb, m_planeByRows)- offsetof (LineStyleSymb, m_lStyle));
    //         So it will be necessary to update it if first/last member are changed. */
    ILineStyleCP m_lStyle; // if nullptr, no linestyle active
    struct
        {
        uint32_t scale:1;
        uint32_t dashScale:1;
        uint32_t gapScale:1;
        uint32_t orgWidth:1;
        uint32_t endWidth:1;
        uint32_t phaseShift:1;
        uint32_t autoPhase:1;
        uint32_t maxCompress:1;
        uint32_t iterationLimit:1;
        uint32_t treatAsSingleSegment:1;
        uint32_t plane:1;
        uint32_t cosmetic:1;
        uint32_t centerPhase:1;
        uint32_t xElemPhaseSet:1;
        uint32_t startTangentSet:1;
        uint32_t endTangentSet:1;
        uint32_t elementIsClosed:1;
        uint32_t continuationXElems:1;
        uint32_t isCurve:1;
        uint32_t isContinuous:1;
        } m_options;

    int         m_nIterate;
    double      m_scale;
    double      m_dashScale;
    double      m_gapScale;
    double      m_orgWidth;
    double      m_endWidth;
    double      m_phaseShift;
    double      m_autoPhase;
    double      m_maxCompress;
    double      m_totalLength;      // length of entire element.
    double      m_xElemPhase;       // where we left off from last element (for compound elements)
    double      m_styleWidth;
    DVec3d      m_startTangent;
    DVec3d      m_endTangent;
    bool        m_useStroker;
    bool        m_useLinePixels;
    uint32_t    m_linePixels;
    RotMatrix   m_planeByRows;

public:
    DGNPLATFORM_EXPORT LineStyleSymb();
    DGNPLATFORM_EXPORT void Init(DgnStyleId styleId, LineStyleParamsCR styleParams, DgnDbR db);

    void Clear() {m_lStyle = nullptr;}
    void Init(ILineStyleCP);

    DGNPLATFORM_EXPORT bool operator==(LineStyleSymbCR rhs) const; //!< Compare two LineStyleSymb.

    ILineStyleCP GetILineStyle() const {return m_lStyle;}
    void GetPlaneAsMatrixRows(RotMatrixR matrix) const {matrix = m_planeByRows;}
    DGNPLATFORM_EXPORT double GetScale() const;
    DGNPLATFORM_EXPORT double GetDashScale() const;
    DGNPLATFORM_EXPORT double GetGapScale() const;
    DGNPLATFORM_EXPORT double GetOriginWidth() const;
    DGNPLATFORM_EXPORT double GetEndWidth() const;
    double GetPhaseShift() const {return m_phaseShift;}
    double GetFractionalPhase() const {return m_autoPhase;}
    double GetMaxCompress() const {return m_maxCompress;}
    int GetNumIterations() const {return m_nIterate;}
    DGNPLATFORM_EXPORT double GetMaxWidth() const;
    double GetStyleWidth() const {return m_styleWidth;}
    double GetTotalLength() const {return m_totalLength;}
    DVec3dCP GetStartTangent() const {return &m_startTangent;}
    DVec3dCP GetEndTangent() const{return &m_endTangent;}

    bool IsScaled() const {return m_options.scale;}
    bool IsAutoPhase() const {return m_options.autoPhase;}
    bool IsCenterPhase() const{return m_options.centerPhase;}
    bool IsCosmetic() const {return m_options.cosmetic;}
    bool IsTreatAsSingleSegment() const {return m_options.treatAsSingleSegment;}
    bool IsElementClosed() const {return m_options.elementIsClosed;}
    bool IsCurve() const {return m_options.isCurve;}
    bool IsContinuous() const {return m_options.isContinuous;}

    bool HasDashScale() const {return m_options.dashScale;}
    bool HasGapScale() const {return m_options.gapScale;}
    bool HasOrgWidth() const {return m_options.orgWidth;}
    bool HasEndWidth() const{return m_options.endWidth;}
    bool HasPhaseShift() const {return m_options.phaseShift;}
    bool HasIterationLimit() const {return m_options.iterationLimit;}
    bool HasPlane() const {return m_options.plane;}
    bool HasStartTangent() const {return m_options.startTangentSet;}
    bool HasEndTangent() const {return m_options.endTangentSet;}
    bool HasTrueWidth() const  {return HasOrgWidth() || HasEndWidth();}
    bool HasMaxCompress() const {return m_options.maxCompress;}

    DGNPLATFORM_EXPORT void SetPlaneAsMatrixRows(RotMatrixCP);
    DGNPLATFORM_EXPORT void SetNormalVec(DPoint3dCP);
    DGNPLATFORM_EXPORT void SetOriginWidth(double width);
    DGNPLATFORM_EXPORT void SetEndWidth(double width);
    DGNPLATFORM_EXPORT void SetWidth(double width);
    DGNPLATFORM_EXPORT void SetScale(double scaleFactor);
    DGNPLATFORM_EXPORT void SetGapScale(double scaleFactor);
    DGNPLATFORM_EXPORT void SetDashScale(double scaleFactor);
    DGNPLATFORM_EXPORT void SetFractionalPhase(bool isOn, double fraction);
    DGNPLATFORM_EXPORT void SetCenterPhase(bool isOn);
    DGNPLATFORM_EXPORT void SetPhaseShift(bool isOn, double distance);
    DGNPLATFORM_EXPORT void SetTreatAsSingleSegment(bool yesNo);
    DGNPLATFORM_EXPORT void SetTangents(DVec3dCP, DVec3dCP);
    DGNPLATFORM_EXPORT void SetCosmetic(bool cosmetic);
    void SetTotalLength(double length) {m_totalLength = length;}
    void SetLineStyle(ILineStyleCP lstyle) {m_lStyle = lstyle;}
    void SetXElemPhase(double last) {m_xElemPhase = last; m_options.xElemPhaseSet=true;}
    void SetElementClosed(bool closed) {m_options.elementIsClosed = closed;}
    void SetIsCurve(bool isCurve) {m_options.isCurve = isCurve;}
    bool UseLinePixels() const {return m_useLinePixels;}
    uint32_t GetLinePixels() const {return m_linePixels;}
    void SetUseLinePixels(uint32_t linePixels){m_linePixels = linePixels; m_useLinePixels = true;}
    bool GetUseStroker() const {return m_useStroker;}
    void SetUseStroker(bool useStroker) {m_useStroker = useStroker;}

    bool ContinuationXElems() const {return m_options.continuationXElems;}
    DGNPLATFORM_EXPORT void ClearContinuationData();
    DGNPLATFORM_EXPORT void CheckContinuationData();
};

//=======================================================================================
//! Line style id and parameters
//=======================================================================================
struct LineStyleInfo : RefCountedBase
{
protected:
    DgnStyleId          m_styleId;
    LineStyleParams     m_styleParams; //!< modifiers for user defined linestyle (if applicable)
    LineStyleSymb       m_lStyleSymb; //!< cooked form of linestyle

    DGNPLATFORM_EXPORT LineStyleInfo(DgnStyleId styleId, LineStyleParamsCP params);

    uint32_t _GetExcessiveRefCountThreshold() const override {return 100000;}
public:
    DGNPLATFORM_EXPORT void CopyFrom(LineStyleInfoCR);

    //! Create an instance of a LineStyleInfo.
    DGNPLATFORM_EXPORT static LineStyleInfoPtr Create(DgnStyleId styleId, LineStyleParamsCP params);

    //! Compare two LineStyleInfo.
    DGNPLATFORM_EXPORT bool operator==(LineStyleInfoCR rhs) const;

    DgnStyleId GetStyleId() const {return m_styleId;}
    LineStyleParamsCP GetStyleParams() const {return 0 != m_styleParams.modifiers ? &m_styleParams : nullptr;}
    LineStyleParamsR GetStyleParamsR() {return m_styleParams;}
    LineStyleSymbCR GetLineStyleSymb() const {return m_lStyleSymb;}
    LineStyleSymbR GetLineStyleSymbR() {return m_lStyleSymb;}

    DGNPLATFORM_EXPORT void Resolve(DgnDbR); // Resolve effective values using the supplied DgnDb...
 };

struct ISprite;
struct DgnOleDraw;

enum class FillDisplay //!< Whether a closed region should be drawn for wireframe display with its internal area filled or not.
{
    Never    = 0, //!< don't fill, even if fill attribute is on for the viewport
    ByView   = 1, //!< fill if the fill attribute is on for the viewport
    Always   = 2, //!< always fill, even if the fill attribute is off for the viewport
    Blanking = 3, //!< always fill, fill will always be behind subsequent geometry
};

enum class BackgroundFill
{
    None    = 0, //!< single color fill uses the fill color and line color to draw either a solid or outline fill
    Solid   = 1, //!< single color fill uses the view's background color to draw a solid fill
    Outline = 2, //!< single color fill uses the view's background color and line color to draw an outline fill
};

enum class DgnGeometryClass : uint8_t
{
    Primary      = 0,
    Construction = 1,
    Dimension    = 2,
    Pattern      = 3,
};

enum class LineJoin
{
    None    = 0,
    Bevel   = 1,
    Miter   = 2,
    Round   = 3,
};

enum class LineCap
{
    None     = 0,
    Flat     = 1,
    Square   = 2,
    Round    = 3,
    Triangle = 4,
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     04/2018
//=======================================================================================
struct ThematicGradientSettings : RefCountedBase
    {
    enum class ColorScheme
        {
        BlueRed     = 0,
        RedBlue     = 1,
        Monochrome  = 2,
        Topographic = 3,
        SeaMountain = 4,
        Custom      = 5,
        };
    enum class Mode
        {
        Smooth,
        Stepped,
        SteppedWithDelimiter,
        Isolines,
        };

    private:
    uint32_t        m_stepCount = 10;
    ColorDef        m_marginColor = ColorDef(0x3f, 0x3f, 0x3f);
    Mode            m_mode = Mode::Smooth;
    ColorScheme     m_colorScheme = ColorScheme::BlueRed;
    DRange1d        m_range = DRange1d::NullRange();

    public:
                    ThematicGradientSettings() {};
                    ThematicGradientSettings(DRange1dCR range, ColorScheme colorScheme = ColorScheme::BlueRed, Mode mode = Mode::Smooth) : m_range(range), m_colorScheme(colorScheme), m_mode(mode) { }
    Json::Value     ToJson() const;
    void            FromJson(Json::Value const& value);
    uint32_t        GetStepCount() const { return m_stepCount; }
    void            SetStepCount(uint32_t stepCount) { m_stepCount = stepCount; }
    ColorDef        GetMarginColor() const { return m_marginColor; }
    void            SetMarginColor (ColorDefCR color) { m_marginColor = color; }
    Mode            GetMode() const { return m_mode; ; }
    void            SetMode(Mode mode) { m_mode = mode; }
    ColorScheme     GetColorScheme() const { return m_colorScheme; }
    void            SetColorScheme(ColorScheme colorScheme) { m_colorScheme = colorScheme; }
    DRange1dCR      GetRange() const { return m_range; }
    void            SetRange(DRange1dCR range) { m_range = range; }
    static double   GetMargin() { return .01; }

    bool operator==(ThematicGradientSettings const& rhs) const;
    bool operator<(ThematicGradientSettings const& rhs) const;
    };

//=======================================================================================
//! Parameters defining a gradient
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct GradientSymb : RefCountedBase
{
    enum
    {
        MAX_GRADIENT_KEYS = 8,
    };

    enum Flags : Byte
    {
        None                = 0,
        Invert              = (1 << 0),
        Outline             = (1 << 1),
        Deprecated          = (1 << 2), //!< Was AlwaysFilled, now controlled by FillDisplay...
    };

    enum class Mode : Byte
    {
        None                = 0,
        Linear              = 1,
        Curved              = 2,
        Cylindrical         = 3,
        Spherical           = 4,
        Hemispherical       = 5,
        Thematic            = 6,
    };


protected:
    Mode        m_mode = Mode::None;
    Flags       m_flags = Flags::None;
    uint32_t    m_nKeys = 0;
    double      m_angle = 0.0;
    double      m_tint = 0.0;
    double      m_shift = 0.0;
    ColorDef    m_colors[MAX_GRADIENT_KEYS];
    double      m_values[MAX_GRADIENT_KEYS];

    // For Thematic only...
    ThematicGradientSettingsPtr m_thematicSettings;

public:
    GradientSymb() {}
    // For thematic display....
    DGNPLATFORM_EXPORT GradientSymb(ThematicGradientSettingsR thematicSettings);

    DGNPLATFORM_EXPORT void CopyFrom(GradientSymbCR);

    //! Create an instance of a GradientSymb.
    static GradientSymbPtr Create() {return new GradientSymb();}

    //! Compare two GradientSymb.
    DGNPLATFORM_EXPORT bool operator==(GradientSymbCR rhs) const;
    DGNPLATFORM_EXPORT bool operator<(GradientSymbCR rhs) const;

    uint32_t GetNKeys() const {return m_nKeys;}
    Mode GetMode() const {return m_mode;}
    Flags GetFlags() const {return m_flags;}
    double GetShift() const {return m_shift;}
    double GetTint() const {return m_tint;}
    double GetAngle() const {return m_angle;}
    bool GetIsOutlined() const {return 0 != (m_flags & Outline); }
    void GetKey(ColorDef& color, double& value, int index) const {color = m_colors[index]; value = m_values[index];}
    void SetMode(Mode mode) {m_mode = mode;}
    void SetFlags(Flags flags) {m_flags = flags;}
    void SetAngle(double angle) {m_angle = angle;}
    void SetTint(double tint) {m_tint = tint;}
    void SetShift(double shift) {m_shift = shift;}
    DGNPLATFORM_EXPORT void SetKeys(uint32_t nKeys, ColorDef const* colors, double const* values);
    ColorDef MapColor(double value) const;
    DGNPLATFORM_EXPORT bool HasTransparency() const;
    DGNPLATFORM_EXPORT Image GetImage(uint32_t width, uint32_t height) const;
    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    DGNPLATFORM_EXPORT BentleyStatus FromJson(Json::Value const& json);
    BentleyStatus GetKey(ColorDef& color, double& value, uint32_t iKey) const;
    // Thematic display.
    bool IsThematic() const { return m_mode == Mode::Thematic; }
    ThematicGradientSettingsCPtr GetThematicSettings() const { return m_thematicSettings; }
    ThematicGradientSettingsPtr GetThematicSettingsR() { return m_thematicSettings; }
    void SetThematicSettings (ThematicGradientSettingsR settings) { m_thematicSettings = &settings; }
};


//=======================================================================================
//! This structure holds the displayable parameters of a GeometrySource
// @bsiclass
//=======================================================================================
struct GeometryParams
{
private:
    struct AppearanceOverrides
        {
        bool m_color:1;
        bool m_weight:1;
        bool m_style:1;
        bool m_material:1;
        bool m_fill:1; // If not set, fill is an opaque fill that matches sub-category appearance color...
        AppearanceOverrides() {memset(this, 0, sizeof(*this));}
        };

    AppearanceOverrides m_appearanceOverrides; //!< flags for parameters that override SubCategory::Appearance.
    bool m_resolved = false; //!< whether Resolve has established SubCategory::Appearance/effective values.
    DgnCategoryId m_categoryId; //!< the Category Id on which the geometry is drawn.
    DgnSubCategoryId m_subCategoryId; //!< the SubCategory Id that controls the appearance of subsequent geometry.
    RenderMaterialId m_materialId; //!< render material Id.
    MaterialUVDetail m_materialUVDetail;
    int32_t m_elmPriority = 0; //!< display priority (applies to 2d only)
    int32_t m_netPriority = 0; //!< net display priority for element/category (applies to 2d only)
    uint32_t m_weight = 0;
    ColorDef m_lineColor;
    ColorDef m_fillColor; //!< fill color (applicable only if filled)
    BackgroundFill m_backgroundFill = BackgroundFill::None; //!< support for fill using the view's background color.
    FillDisplay m_fillDisplay = FillDisplay::Never; //!< whether or not the element should be displayed filled
    double m_elmTransparency = 0; //!< transparency, 1.0 == completely transparent.
    double m_netElmTransparency = 0; //!< net transparency for element/category.
    double m_fillTransparency = 0;  //!< fill transparency, 1.0 == completely transparent.
    double m_netFillTransparency = 0; //!< net transparency for fill/category.
    DgnGeometryClass m_geometryClass = DgnGeometryClass::Primary; //!< geometry class
    LineStyleInfoPtr m_styleInfo; //!< line style id plus modifiers.
    GradientSymbPtr m_gradient; //!< gradient fill settings.
    PatternParamsPtr m_pattern; //!< area pattern settings.

public:
    GeometryParams() {}
    GeometryParams(DgnCategoryId categoryId, DgnSubCategoryId subCategoryId = DgnSubCategoryId()) : m_categoryId(categoryId), m_subCategoryId(subCategoryId.IsValid() ? subCategoryId : DgnCategory::GetDefaultSubCategoryId(categoryId)) {}

    DGNPLATFORM_EXPORT GeometryParams(GeometryParamsCR rhs);
    DGNPLATFORM_EXPORT void ResetAppearance(); //!< Like Init, but saves and restores category and sub-category around the call to Init. This is particularly useful when a single element draws objects of different symbology, but its draw code does not have easy access to reset the category.
    DGNPLATFORM_EXPORT void Resolve(DgnDbR); // Resolve effective values using the supplied DgnDb
    DGNPLATFORM_EXPORT void Resolve(ViewContextR); // Resolve effective values using the supplied ViewContext.
    bool IsResolved() const { return m_resolved; } // Whether effective values have been resolved.

    void SetCategoryId(DgnCategoryId categoryId, bool clearAppearanceOverrides = true) {m_categoryId = categoryId; m_subCategoryId = DgnCategory::GetDefaultSubCategoryId(categoryId); if (clearAppearanceOverrides) memset(&m_appearanceOverrides, 0, sizeof(m_appearanceOverrides)); m_resolved = false;} // Setting the Category Id also sets the SubCategory to the default.
    void SetSubCategoryId(DgnSubCategoryId subCategoryId, bool clearAppearanceOverrides = true) {m_subCategoryId = subCategoryId; if (clearAppearanceOverrides) memset(&m_appearanceOverrides, 0, sizeof(m_appearanceOverrides)); m_resolved = false;}
    void SetWeight(uint32_t weight) {m_appearanceOverrides.m_weight = true; m_weight = weight;}
    void SetLineStyle(LineStyleInfoP styleInfo) {m_appearanceOverrides.m_style = true; m_styleInfo = styleInfo; if (styleInfo) m_resolved = false;}
    void SetLineColor(ColorDef color) {m_appearanceOverrides.m_color = true; m_lineColor = color;}
    void SetFillDisplay(FillDisplay display) {m_fillDisplay = display;}
    void SetFillColor(ColorDef color) {m_appearanceOverrides.m_fill = true; m_fillColor = color; m_backgroundFill = BackgroundFill::None;}
    void SetFillColorFromViewBackground(bool outline=false) {m_appearanceOverrides.m_fill = true; m_backgroundFill = outline ? BackgroundFill::Outline : BackgroundFill::Solid; m_resolved = false;}
    void SetGradient(GradientSymbP gradient) {m_gradient = gradient;}
    void SetGeometryClass(DgnGeometryClass geomClass) {m_geometryClass = geomClass;}
    void SetTransparency(double transparency) {m_elmTransparency = m_netElmTransparency = m_fillTransparency = m_netFillTransparency = transparency; m_resolved = false;} // NOTE: Sets BOTH element and fill transparency...
    void SetFillTransparency(double transparency) {m_fillTransparency = m_netFillTransparency = transparency; m_resolved = false;}
    void SetDisplayPriority(int32_t priority) {m_elmPriority = m_netPriority = priority; m_resolved = false;} // Set display priority (2d only).
    void SetMaterialId(RenderMaterialId materialId) {m_appearanceOverrides.m_material = true; m_materialId = materialId;}
    void SetMaterialUVDetail(MaterialUVDetailCR detail) {m_materialUVDetail = detail;}
    void SetPatternParams(PatternParamsP patternParams) {m_pattern = patternParams;}

    //! @cond DONTINCLUDEINDOC
    double GetNetTransparency() const {BeAssert(m_resolved); return m_netElmTransparency;}
    double GetNetFillTransparency() const {BeAssert(m_resolved); return m_netFillTransparency;}

    int32_t GetNetDisplayPriority() const {return m_netPriority;} // Get net display priority (2d only).
    void SetNetDisplayPriority(int32_t priority) {m_netPriority = priority;} // RASTER USE ONLY!!!

    void SetLineColorToSubCategoryAppearance() {m_resolved = m_appearanceOverrides.m_color = false;}
    void SetWeightToSubCategoryAppearance() {m_resolved = m_appearanceOverrides.m_weight = false;}
    void SetLineStyleToSubCategoryAppearance() {m_resolved = m_appearanceOverrides.m_style = false;}
    void SetMaterialToSubCategoryAppearance() {m_resolved = m_appearanceOverrides.m_material = false;}
    void SetFillColorToSubCategoryAppearance() {m_resolved = m_appearanceOverrides.m_fill = false;}

    bool IsLineColorFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_color;}
    bool IsWeightFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_weight;}
    bool IsLineStyleFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_style;}
    bool IsMaterialFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_material;}
    bool IsFillColorFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_fill;}
    //! @endcond

    //! Compare two GeometryParams for equivalence, i.e. both values are from sub-category appearance or have the same override.
    DGNPLATFORM_EXPORT bool IsEquivalent(GeometryParamsCR) const;

    //! copy operator
    DGNPLATFORM_EXPORT GeometryParamsR operator=(GeometryParamsCR rhs);

    //! Get element category
    DgnCategoryId GetCategoryId() const {return m_categoryId;}

    //! Get element sub-category
    DgnSubCategoryId GetSubCategoryId() const {return m_subCategoryId;}

    //! Get element color
    ColorDef GetLineColor() const {BeAssert(m_appearanceOverrides.m_color || m_resolved); return m_lineColor;}

    //! Get element fill color
    ColorDef GetFillColor() const {BeAssert((m_appearanceOverrides.m_fill && BackgroundFill::None == m_backgroundFill) || m_resolved); return m_fillColor;}

    //! Get fill display setting
    FillDisplay GetFillDisplay() const {return m_fillDisplay;}

    //! Get solid fill color type setting
    bool IsFillColorFromViewBackground(bool* outline=nullptr) const {if (outline) *outline = BackgroundFill::Outline == m_backgroundFill; return BackgroundFill::None != m_backgroundFill;}

    //! Get gradient fill information. Valid when FillDisplay::Never != GetFillDisplay() and not nullptr.
    GradientSymbCP GetGradient() const {return m_gradient.get();}

    //! Get the area pattern params.
    PatternParamsCP GetPatternParams() const {return m_pattern.get();}
    PatternParamsP GetPatternParamsP() {return m_pattern.get();}

    //! Get the geometry class.
    DgnGeometryClass GetGeometryClass() const {return m_geometryClass;}

    //! Get line style information.
    LineStyleInfoCP GetLineStyle() const {BeAssert(m_appearanceOverrides.m_style || m_resolved); return m_styleInfo.get();}

    //! Get line weight.
    uint32_t GetWeight() const {BeAssert(m_appearanceOverrides.m_weight || m_resolved); return m_weight;}

    //! Get transparency.
    double GetTransparency() const {return m_elmTransparency;}

    //! Get fill/gradient transparency.
    double GetFillTransparency() const {return m_fillTransparency;}

    //! Get render material.
    RenderMaterialId GetMaterialId() const {BeAssert(m_appearanceOverrides.m_material || m_resolved); return m_materialId;}
    MaterialUVDetailCR GetMaterialUVDetail() const {return m_materialUVDetail;}

    //! Get display priority (2d only).
    int32_t GetDisplayPriority() const {return m_elmPriority;}

    bool HasStrokedLineStyle() const {BeAssert(m_appearanceOverrides.m_style || m_resolved); return (m_styleInfo.IsValid() ? (nullptr != m_styleInfo->GetLineStyleSymb().GetILineStyle() && m_styleInfo->GetLineStyleSymb().GetUseStroker()) : false);}

    //! Get whether this GeometryParams contains information that needs to be transformed (ex. to apply local to world).
    bool IsTransformable() const {return m_pattern.IsValid() || m_styleInfo.IsValid();}

    //! Transform GeometryParams data like PatternParams and LineStyleInfo.
    DGNPLATFORM_EXPORT void ApplyTransform(TransformCR transform, uint32_t options = 0);
};

//=======================================================================================
//! Built in line code patterns
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
enum class LinePixels : uint32_t
    {
    Solid = 0,
    Code0 = Solid,      // 0
    Code1 = 0x80808080, // 1
    Code2 = 0xf8f8f8f8, // 2
    Code3 = 0xffe0ffe0, // 3
    Code4 = 0xfe10fe10, // 4
    Code5 = 0xe0e0e0e0, // 5
    Code6 = 0xf888f888, // 6
    Code7 = 0xff18ff18, // 7
    HiddenLine = 0xcccccccc,  // hidden lines
    Invisible = 0x00000001, // nearly invisible
    Invalid = 0xffffffff,
    };

//=======================================================================================
//! The "cooked" material and symbology for a Render::Graphic. This determines the appearance
//! (e.g. texture, color, width, linestyle, etc.) used to draw Geometry.
//=======================================================================================
struct GraphicParams
{
private:
    bool m_isFilled = false;
    bool m_isBlankingRegion = false;
    uint32_t m_linePixels = (uint32_t) LinePixels::Solid;
    uint32_t m_rasterWidth = 1;
    ColorDef m_lineColor;
    ColorDef m_fillColor;
    double m_trueWidthStart = 0;
    double m_trueWidthEnd = 0;
    TexturePtr m_lineTexture;
    MaterialPtr m_material;
    GradientSymbPtr m_gradient;
    MaterialUVDetail m_materialUVDetail;

public:

    void Cook(GeometryParamsCR, ViewContextR);

    void Init() {*this = GraphicParams();}
    GraphicParams() {}
    DGNPLATFORM_EXPORT GraphicParams(GraphicParamsCR rhs);

    //! @name Query Methods
    //@{

    //! Compare two GraphicParams.
    DGNPLATFORM_EXPORT bool operator==(GraphicParamsCR rhs) const;

    //! copy operator
    DGNPLATFORM_EXPORT GraphicParamsR operator=(GraphicParamsCR rhs);

    //! Get the TBGR line color from this GraphicParams
    ColorDef GetLineColor() const {return m_lineColor;}

    //! Get the TBGR fill color from this GraphicParams.
    ColorDef GetFillColor() const {return m_fillColor;}

    //! Get the width in pixels from this GraphicParams.
    uint32_t GetWidth() const {return m_rasterWidth;}

    //! Get width at start in world coords from this GraphicParams.
    double GetTrueWidthStart() const {return m_trueWidthStart;}

    //! Get width at end in world coords from this GraphicParams.
    double GetTrueWidthEnd() const {return m_trueWidthEnd;}

    //! Get the texture applied to lines for this GraphicParams
    TextureP GetLineTexture() const {return m_lineTexture.get();}

    //! Get the linear pixel pattern for this GraphicParams. This is only valid for overlay decorators in pixel mode.
    uint32_t GetLinePixels() const {return m_linePixels;}

    //! Determine whether the fill flag is on for this GraphicParams.
    bool IsFilled() const {return m_isFilled;}

    //! Determine whether the fill represents blanking region.
    bool IsBlankingRegion() const {return m_isBlankingRegion;}

    //! Get the GradientSymb from this GraphicParams.
    GradientSymbCP GetGradientSymb() const {return m_gradient.get();}

    //! Get the render material.
    MaterialP GetMaterial() const {return m_material.get();}
    MaterialUVDetailCR GetMaterialUVDetail() const {return m_materialUVDetail;}

    //@}

    //! @name Set Methods
    //@{

    //! Set the current line color for this GraphicParams.
    //! @param[in] lineColor the new TBGR line color for this GraphicParams.     a
    void SetLineColor(ColorDef lineColor) {m_lineColor = lineColor;}
    void SetLineTransparency(Byte transparency) {m_lineColor.SetAlpha(transparency);}

    //! Set the current fill color for this GraphicParams.
    //! @param[in] fillColor the new TBGR fill color for this GraphicParams.
    void SetFillColor(ColorDef fillColor) {m_fillColor = fillColor;}
    void SetFillTransparency(Byte transparency) {m_fillColor.SetAlpha(transparency);}

    //! Set the width in pixels for this GraphicParams.
    //! @param[in] rasterWidth the width in pixels of lines drawn using this GraphicParams.
    //! @note if either TrueWidthStart or TrueWidthEnd are non-zero, this value is ignored.
    void SetWidth(uint32_t rasterWidth) {m_rasterWidth = rasterWidth;}

    //! Set width at start in world coords from this GraphicParams.
    void SetTrueWidthStart(double width) {m_trueWidthStart = width;}

    //! Set width at end in world coords from this GraphicParams.
    void SetTrueWidthEnd(double width) {m_trueWidthEnd = width;}

    //! Set a LineTexture for this GraphicParams
    void SetLineTexture(TextureP texture) {m_lineTexture = texture;}

    //! Set the linear pixel pattern for this GraphicParams. This is only valid for overlay decorators in pixel mode.
    void SetLinePixels(LinePixels code) {m_linePixels = (uint32_t) code; m_lineTexture=nullptr;}

    //! Turn on or off the fill flag for this GraphicParams.
    //! @param[in] filled if true, the interior of elements drawn using this GraphicParams will be filled using the fill color.
    void SetIsFilled(bool filled) {m_isFilled = filled;}

    //! Set that fill is always behind other geometry.
    void SetIsBlankingRegion(bool blanking) {m_isBlankingRegion = blanking;}

    //! Set the gradient symbology
    void SetGradient(GradientSymbP gradient) {m_gradient = gradient;}

    //! Set the render material.
    void SetMaterial(MaterialP material) {m_material = material;}
    void SetMaterialUVDetail(MaterialUVDetailCR detail) {m_materialUVDetail = detail;}
    //@}

    static GraphicParams FromSymbology(ColorDef lineColor, ColorDef fillColor, int lineWidth, LinePixels linePixels=LinePixels::Solid)
        {
        GraphicParams graphicParams;
        graphicParams.SetLineColor(lineColor);
        graphicParams.SetFillColor(fillColor);
        graphicParams.SetWidth(lineWidth);
        graphicParams.SetLinePixels(linePixels);
        return graphicParams;
        }

    static GraphicParams FromBlankingFill(ColorDef fillColor)
        {
        GraphicParams graphicParams;
        graphicParams.SetFillColor(fillColor);
        graphicParams.SetIsBlankingRegion(true);
        return graphicParams;
        }
};

//=======================================================================================
//! Describes the type of a Graphic. Used when creating a GraphicBuilder to specify the purpose of the Graphic.
//! For Graphics like overlays and view background for which depth testing is disabled:
//!  - The individual geometric primitives are rendered in the order in which they were defined in the GraphicBuilder; and
//!  - The individual Graphics within the DecorationList are rendered in the order in which they appear in the list.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
enum class GraphicType
{
    //! Renders behind all other graphics. Coordinates: view. RenderMode: smooth. Lighting: none. Z-testing: disabled.
    ViewBackground,
    //! Renders as if it were part of the scene. Coordinates: world. RenderMode: from view. Lighting: from view. Z-testing: enabled.
    //! Used for the scene itself, dynamics, and 'normal' decorations.
    Scene,
    //! Renders within the scene. Coordinates: world. RenderMode: smooth. Lighting: default. Z-testing: enabled
    WorldDecoration,
    //! Renders atop the scene. Coordinates: world. RenderMode: smooth. Lighting: none. Z-testing: disabled
    //! Used for things like the ACS triad and the grid.
    WorldOverlay,
    //! Renders atop the scene. Coordinates: view. RenderMode: smooth. Lighting: none. Z-testing: disabled
    //! Used for things like the locate circle.
    ViewOverlay
};

//=======================================================================================
//! A renderer-specific object that can be placed into a display list.
// @bsistruct                                                   Paul.Connelly   05/16
//=======================================================================================
struct Graphic : RefCounted<NonCopyableClass>
{
    friend struct ViewContext;
protected:
    DgnDbR      m_dgndb;

    virtual ~Graphic() {}
    uint32_t _GetExcessiveRefCountThreshold() const override {return 100000;}

public:
    explicit Graphic(DgnDbR db) : m_dgndb(db) {}

    DgnDbR GetDgnDb() const { return m_dgndb; }
};

//=======================================================================================
//! Exposes methods for constructing a Graphic from geometric primitives.
// @bsistruct                                                   Paul.Connelly   05/16
//=======================================================================================
struct GraphicBuilder : RefCountedBase
{
    struct TileCorners
    {
        DPoint3d m_pts[4];
    };

    //! Parameters used to construct a GraphicBuilder.
    struct CreateParams
    {
    private:
        DgnDbR          m_dgndb;
        Transform       m_placement;
        GraphicType     m_type;

    public:
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, TransformCR tf, GraphicType type);
        // ###TODO: Need an alternate way to specify the viewport for computing pixel-size/tolerance...DGNPLATFORM_EXPORT CreateParams(DgnViewportR vp, TransformCR tf, GraphicType type);

        //! Create params for a graphic in world coordinates, not necessarily associated with any viewport.
        //! This function is chiefly used for tile generation code as the tolerance for faceting the graphic's geometry is independent of any viewport.
        //! If this function is used outside of tile generation context, a default coarse tolerance will be used.
        //! To get a tolerance appropriate to a viewport, use the overload accepting a DgnViewport.
        static CreateParams Scene(DgnDbR db, TransformCR placement=Transform::FromIdentity())
            { return CreateParams(db, placement, GraphicType::Scene); }

        //! Create params for a subgraphic
        CreateParams SubGraphic(TransformCR placement=Transform::FromIdentity()) const
            { return CreateParams(m_dgndb, placement, m_type); }

        DgnDbR GetDgnDb() const { return m_dgndb; }
        TransformCR GetPlacement() const { return m_placement; }
        GraphicType GetType() const { return m_type; }
        bool IsViewCoordinates() const { return GraphicType::ViewBackground == GetType() || GraphicType::ViewOverlay == GetType(); }
        bool IsWorldCoordinates() const { return !IsViewCoordinates(); }
        bool IsSceneGraphic() const { return GraphicType::Scene == GetType(); }
        bool IsViewBackground() const { return GraphicType::ViewBackground == GetType(); }
        bool IsOverlay() const { return GraphicType::ViewOverlay == GetType() || GraphicType::WorldOverlay == GetType(); }

        void SetPlacement(TransformCR tf) { m_placement=tf; }
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(CreateParams);

    enum class AsThickenedLine { No=0, Yes=1 };

protected:
    CreateParams    m_createParams;
    ClipVectorPtr   m_currClip;

    GraphicBuilder(CreateParams const& params) : m_createParams(params) { }

    virtual bool _IsOpen() const = 0;
    virtual GraphicPtr _Finish() = 0;
    virtual GeometryStreamEntryIdCP _GetGeometryStreamEntryId() const {return nullptr;}
    virtual void _SetGeometryStreamEntryId(GeometryStreamEntryIdCP) {}
    virtual void _ActivateGraphicParams(GraphicParamsCR graphicParams, GeometryParamsCP geomParams) = 0;
    virtual void _AddLineString(int numPoints, DPoint3dCP points) = 0;
    virtual void _AddLineString2d(int numPoints, DPoint2dCP points, double zDepth) = 0;
    virtual void _AddPointString(int numPoints, DPoint3dCP points) = 0;
    virtual void _AddPointString2d(int numPoints, DPoint2dCP points, double zDepth) = 0;
    virtual void _AddShape(int numPoints, DPoint3dCP points, bool filled) = 0;
    virtual void _AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth) = 0;
    virtual void _AddTriStrip(int numPoints, DPoint3dCP points, AsThickenedLine asThickenedLine) = 0;
    virtual void _AddTriStrip2d(int numPoints, DPoint2dCP points, AsThickenedLine asThickenedLine, double zDepth) = 0;
    virtual void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) = 0;
    virtual void _AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth) = 0;
    virtual void _AddBSplineCurve(MSBsplineCurveCR curve, bool filled) = 0;
    virtual void _AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) = 0;
    virtual void _AddCurveVector(CurveVectorCR curves, bool isFilled) = 0;
    virtual void _AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) = 0;
    virtual void _AddSolidPrimitive(ISolidPrimitiveCR primitive) = 0;
    virtual void _AddBSplineSurface(MSBsplineSurfaceCR surface) = 0;
    virtual void _AddPolyface(PolyfaceQueryCR meshData, bool filled = false) = 0;
    virtual void _AddBody(IBRepEntityCR) = 0;
    virtual void _AddTextString(TextStringCR text) = 0;
    virtual void _AddTextString2d(TextStringCR text, double zDepth) = 0;
    virtual void _AddDgnOle(DgnOleDraw*) = 0;
    virtual void _AddSubGraphic(GraphicR, TransformCR, GraphicParamsCR, ClipVectorCP clip) = 0;
    virtual GraphicBuilderPtr _CreateSubGraphic(TransformCR, ClipVectorCP clip) const = 0;
    virtual bool _WantStrokeLineStyle(LineStyleSymbCR, IFacetOptionsPtr&) {return true;}
    virtual bool _WantStrokePattern(PatternParamsCR pattern) {return true;}

    virtual void _AddBSplineCurveR(RefCountedMSBsplineCurveR curve, bool filled) { _AddBSplineCurve(curve, filled); }
    virtual void _AddBSplineCurve2dR(RefCountedMSBsplineCurveR curve, bool filled, double zDepth) { _AddBSplineCurve2d(curve, filled, zDepth); }
    virtual void _AddCurveVectorR(CurveVectorR curves, bool isFilled) { _AddCurveVector(curves, isFilled); }
    virtual void _AddCurveVector2dR(CurveVectorR curves, bool isFilled, double zDepth) { _AddCurveVector2d(curves, isFilled, zDepth); }
    virtual void _AddSolidPrimitiveR(ISolidPrimitiveR primitive) { _AddSolidPrimitive(primitive); }
    virtual void _AddBSplineSurfaceR(RefCountedMSBsplineSurfaceR surface) { _AddBSplineSurface(surface); }
    virtual void _AddPolyfaceR(PolyfaceHeaderR meshData, bool filled = false) { _AddPolyface(meshData, filled); }
    virtual void _AddBodyR(IBRepEntityR body) { _AddBody(body); }
    virtual void _AddTextStringR(TextStringR text) { _AddTextString(text); }
    virtual void _AddTextString2dR(TextStringR text, double zDepth) { _AddTextString2d(text, zDepth); }
public:
    // NOTE: subToGraphic is provided to allow stroking in world coords...
    DGNPLATFORM_EXPORT GraphicBuilderPtr CreateSubGraphic(TransformCR subToGraphic, ClipVectorCP clip=nullptr) const { return _CreateSubGraphic(subToGraphic, clip); }

    GraphicPtr Finish() { BeAssert(IsOpen()); return IsOpen() ? _Finish() : nullptr; }

    void SetCurrentClip(ClipVectorP clip) { m_currClip = clip; }
    ClipVectorCP GetCurrentClip() const {return m_currClip.get();}
    CreateParams const& GetCreateParams() const {return m_createParams;}
    DgnDbR GetDgnDb() const {return m_createParams.GetDgnDb();}
    TransformCR GetLocalToWorldTransform() const {return m_createParams.GetPlacement();}
    bool IsWorldCoordinates() const {return m_createParams.IsWorldCoordinates();}
    bool IsViewCoordinates() const {return m_createParams.IsViewCoordinates();}
    bool WantStrokeLineStyle(LineStyleSymbCR symb, IFacetOptionsPtr& facetOptions) { return _WantStrokeLineStyle(symb, facetOptions); }
    bool WantStrokePattern(PatternParamsCR pattern) { return _WantStrokePattern(pattern); }

    bool IsOpen() const {return _IsOpen();}

    //! Get the current GeometryStreamEntryId.
    //! @return A GeometryStream entry identifier for the graphics that are currently being drawn.
    GeometryStreamEntryIdCP GetGeometryStreamEntryId() const {return _GetGeometryStreamEntryId();}

    //! Set the current GeometryStreamEntryId.
    void SetGeometryStreamEntryId(GeometryStreamEntryIdCP entry) {_SetGeometryStreamEntryId(entry);}

    //! Set an GraphicParams to be the "active" GraphicParams for this Render::Graphic.
    //! @param[in] graphicParams The new active GraphicParams. All geometry drawn via calls to this Render::Graphic will
    //! @param[in] geomParams The source GeometryParams if graphicParams was created by cooking geomParams, nullptr otherwise.
    void ActivateGraphicParams(GraphicParamsCR graphicParams, GeometryParamsCP geomParams=nullptr) {_ActivateGraphicParams(graphicParams, geomParams);}

    //! Draw a 3D line string.
    //! @param[in] numPoints Number of vertices in points array.
    //! @param[in] points Array of vertices in the line string.
    void AddLineString(int numPoints, DPoint3dCP points) {_AddLineString(numPoints, points);}

    //! Draw a 2D line string.
    //! @param[in] numPoints Number of vertices in points array.
    //! @param[in] points Array of vertices in the line string.
    //! @param[in] zDepth Z depth value in local coordinates.
    void AddLineString2d(int numPoints, DPoint2dCP points, double zDepth) {_AddLineString2d(numPoints, points, zDepth);}

    //! Draw a 3D point string. A point string is displayed as a series of points, one at each vertex in the array, with no vectors connecting the vertices.
    //! @param[in] numPoints Number of vertices in points array.
    //! @param[in] points Array of vertices in the point string.
    void AddPointString(int numPoints, DPoint3dCP points) {_AddPointString(numPoints, points);}

    //! Draw a 2D point string. A point string is displayed as a series of points, one at each vertex in the array, with no vectors connecting the vertices.
    //! @param[in] numPoints Number of vertices in points array.
    //! @param[in] points Array of vertices in the point string.
    //! @param[in] zDepth Z depth value.
    void AddPointString2d(int numPoints, DPoint2dCP points, double zDepth) {_AddPointString2d(numPoints, points, zDepth);}

    //! Draw a closed 3D shape.
    //! @param[in] numPoints Number of vertices in \c points array. If the last vertex in the array is not the same as the first vertex, an
    //!                      additional vertex will be added to close the shape.
    //! @param[in] points Array of vertices of the shape.
    //! @param[in] filled If true, the shape will be drawn filled.
    void AddShape(int numPoints, DPoint3dCP points, bool filled) {_AddShape(numPoints, points, filled);}

    //! Draw a 2D shape.
    //! @param[in] numPoints Number of vertices in \c points array. If the last vertex in the array is not the same as the first vertex, an
    //!                                     additional vertex will be added to close the shape.
    //! @param[in] points Array of vertices of the shape.
    //! @param[in] zDepth Z depth value.
    //! @param[in] filled If true, the shape will be drawn filled.
    void AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth) {_AddShape2d(numPoints, points, filled, zDepth);}

    //! Draw a 3D elliptical arc or ellipse.
    //! @param[in] ellipse arc data.
    //! @param[in] isEllipse If true, and if full sweep, then draw as an ellipse instead of an arc.
    //! @param[in] filled If true, and isEllipse is also true, then draw ellipse filled.
    void AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) {_AddArc(ellipse, isEllipse, filled);}

    //! Draw a 2D elliptical arc or ellipse.
    //! @param[in] ellipse arc data.
    //! @param[in] isEllipse If true, and if full sweep, then draw as an ellipse instead of an arc.
    //! @param[in] filled If true, and isEllipse is also true, then draw ellipse filled.
    //! @param[in] zDepth Z depth value
    void AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth) {_AddArc2d(ellipse, isEllipse, filled, zDepth);}

    //! Draw a BSpline curve.
    void AddBSplineCurve(MSBsplineCurveCR curve, bool filled) {_AddBSplineCurve(curve, filled);}

    //! Draw a BSpline curve as 2d geometry with display priority.
    //! @note Only necessary for non-ICachedDraw calls to support non-zero display priority.
    void AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) {_AddBSplineCurve2d(curve, filled, zDepth);}

    //! Draw a curve vector.
    void AddCurveVector(CurveVectorCR curves, bool isFilled) {_AddCurveVector(curves, isFilled);}

    //! Draw a curve vector as 2d geometry with display priority.
    //! @note Only necessary for non-ICachedDraw calls to support non-zero display priority.
    void AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) {_AddCurveVector2d(curves, isFilled, zDepth);}

    //! Draw a light-weight surface or solid primitive.
    //! @remarks Solid primitives can be capped or uncapped, they include cones, torus, box, spheres, and sweeps.
    void AddSolidPrimitive(ISolidPrimitiveCR primitive) {_AddSolidPrimitive(primitive);}

    //! Draw a BSpline surface.
    void AddBSplineSurface(MSBsplineSurfaceCR surface) {_AddBSplineSurface(surface);}

    //! @remarks Wireframe fill display supported for non-illuminated meshes.
    void AddPolyface(PolyfaceQueryCR meshData, bool filled = false) {_AddPolyface(meshData, filled);}

    //! Draw a BRep surface/solid entity from the solids kernel.
    void AddBody(IBRepEntityCR entity) {_AddBody(entity);}

    //! Draw a series of Glyphs.
    //! @param[in] text Text drawing parameters
    void AddTextString(TextStringCR text) {_AddTextString(text);}

    //! Draw a series of Glyphs with display priority.
    //! @param[in] text   Text drawing parameters
    //! @param[in] zDepth Priority value in 2d
    void AddTextString2d(TextStringCR text, double zDepth) {_AddTextString2d(text, zDepth);}

    //! Draw a filled triangle strip from 3D points.
    //! @param[in] numPoints Number of vertices in \c points array.
    //! @param[in] points Array of vertices.
    //! @param[in] asThickenedLine whether the tri-strip represents a thickened line.
    void AddTriStrip(int numPoints, DPoint3dCP points, AsThickenedLine asThickenedLine) {_AddTriStrip(numPoints, points, asThickenedLine);}

    //! Draw a filled triangle strip from 2D points.
    //! @param[in] numPoints Number of vertices in \c points array.
    //! @param[in] points Array of vertices.
    //! @param[in] zDepth Z depth value.
    //! @param[in] asThickenedLine whether the tri-strip represents a thickened line.
    void AddTriStrip2d(int numPoints, DPoint2dCP points, AsThickenedLine asThickenedLine, double zDepth) {_AddTriStrip2d(numPoints, points, asThickenedLine, zDepth);}

    //! Helper Methods to draw simple SolidPrimitives.
    void AddTorus(DPoint3dCR center, DVec3dCR vectorX, DVec3dCR vectorY, double majorRadius, double minorRadius, double sweepAngle, bool capped) {AddSolidPrimitive(*ISolidPrimitive::CreateDgnTorusPipe(DgnTorusPipeDetail(center, vectorX, vectorY, majorRadius, minorRadius, sweepAngle, capped)));}
    void AddBox(DVec3dCR primary, DVec3dCR secondary, DPoint3dCR basePoint, DPoint3dCR topPoint, double baseWidth, double baseLength, double topWidth, double topLength, bool capped) {AddSolidPrimitive(*ISolidPrimitive::CreateDgnBox(DgnBoxDetail::InitFromCenters(basePoint, topPoint, primary, secondary, baseWidth, baseLength, topWidth, topLength, capped)));}

    //! Add DRange3d edges
    void AddRangeBox(DRange3dCR range)
        {
        DPoint3d p[8], tmpPts[9];

        p[0].x = p[3].x = p[4].x = p[5].x = range.low.x;
        p[1].x = p[2].x = p[6].x = p[7].x = range.high.x;
        p[0].y = p[1].y = p[4].y = p[7].y = range.low.y;
        p[2].y = p[3].y = p[5].y = p[6].y = range.high.y;
        p[0].z = p[1].z = p[2].z = p[3].z = range.low.z;
        p[4].z = p[5].z = p[6].z = p[7].z = range.high.z;

        tmpPts[0] = p[0]; tmpPts[1] = p[1]; tmpPts[2] = p[2];
        tmpPts[3] = p[3]; tmpPts[4] = p[5]; tmpPts[5] = p[6];
        tmpPts[6] = p[7]; tmpPts[7] = p[4]; tmpPts[8] = p[0];

        AddLineString(9, tmpPts);
        AddLineString(2, DSegment3d::From(p[0], p[3]).point);
        AddLineString(2, DSegment3d::From(p[4], p[5]).point);
        AddLineString(2, DSegment3d::From(p[1], p[7]).point);
        AddLineString(2, DSegment3d::From(p[2], p[6]).point);
        }

    //! Add DRange2d edges
    void AddRangeBox2d(DRange2dCR range, double zDepth)
        {
        DPoint2d tmpPts[5];

        tmpPts[0] = DPoint2d::From(range.low.x, range.low.y);
        tmpPts[1] = DPoint2d::From(range.high.x, range.low.y);
        tmpPts[2] = DPoint2d::From(range.high.x, range.high.y);
        tmpPts[3] = DPoint2d::From(range.low.x, range.high.y);
        tmpPts[4] = tmpPts[0];

        AddLineString2d(5, tmpPts, zDepth);
        }

    //! Draw OLE object.
    void AddDgnOle(DgnOleDraw* ole) {_AddDgnOle(ole);}

    void AddSubGraphic(GraphicR graphic, TransformCR subToGraphic, GraphicParamsCR params, ClipVectorCP clip=nullptr) {_AddSubGraphic(graphic, subToGraphic, params, clip);}

    //! Set symbology for decorations that are only used for display purposes. Pickable decorations require a category, must initialize
    //! a GeometryParams and cook it into a GraphicParams to have a locatable decoration.
    void SetSymbology(ColorDef lineColor, ColorDef fillColor, int lineWidth, LinePixels linePixels=LinePixels::Solid)
        {
        ActivateGraphicParams(GraphicParams::FromSymbology(lineColor, fillColor, lineWidth, linePixels));
        }

    //! Set blanking fill symbology for decorations that are only used for display purposes. Pickable decorations require a category, must initialize
    //! a GeometryParams and cook it into a GraphicParams to have a locatable decoration.
    void SetBlankingFill(ColorDef fillColor)
        {
        ActivateGraphicParams(GraphicParams::FromBlankingFill(fillColor));
        }

    // The following potentially take ownership of and/or modify the input graphic primitives.
    // Callers should prefer these functions when they do not need to preserve the input primitives.
    // Implementations should implement these functions if they would otherwise clone or take ownership of the input primitives.
    // The default implementations forward to the corresponding function taking const primitives.
    void AddBSplineCurveR(RefCountedMSBsplineCurveR curve, bool filled) { _AddBSplineCurveR(curve, filled); }
    void AddBSplineCurve2dR(RefCountedMSBsplineCurveR curve, bool filled, double zDepth) { _AddBSplineCurve2dR(curve, filled, zDepth); }
    void AddCurveVectorR(CurveVectorR curves, bool isFilled) { _AddCurveVectorR(curves, isFilled); }
    void AddCurveVector2dR(CurveVectorR curves, bool isFilled, double zDepth) { _AddCurveVector2dR(curves, isFilled, zDepth); }
    void AddSolidPrimitiveR(ISolidPrimitiveR primitive) { _AddSolidPrimitiveR(primitive); }
    void AddBSplineSurfaceR(RefCountedMSBsplineSurfaceR surface) { _AddBSplineSurfaceR(surface); }
    void AddPolyfaceR(PolyfaceHeaderR meshData, bool filled = false) { _AddPolyfaceR(meshData, filled); }
    void AddBodyR(IBRepEntityR body) { _AddBodyR(body); }
    void AddTextStringR(TextStringR text) { _AddTextStringR(text); }
    void AddTextString2dR(TextStringR text, double zDepth) { _AddTextString2dR(text, zDepth); }
};

//=======================================================================================
//! Defines non-uniform color for a graphic primitive.
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct ColorIndex
{
    struct NonUniform
    {
        uint32_t const* m_colors; // RGBT color values (see ColorDef), or nullptr if uniform color.
        uint16_t const* m_indices; // per-vertex indices into m_colors
        bool            m_hasAlpha; // true if any value in m_colors has transparency

        void Set(uint32_t const* colors, uint16_t const* indices, bool hasAlpha)
            {
            m_colors = colors;
            m_indices = indices;
            m_hasAlpha = hasAlpha;
            }
    };

    union
    {
        uint32_t    m_uniform;    // if m_numColors == 1
        NonUniform  m_nonUniform; // if m_numColors > 1
    };

    uint16_t        m_numColors;

    ColorIndex() { Reset(); }

    bool IsValid() const { return m_numColors > 0; }
    bool IsUniform() const { BeAssert(m_numColors > 0); return 1 == m_numColors; }
    bool HasAlpha() const { return IsUniform() ? 0 != (m_uniform & 0xff000000) : m_nonUniform.m_hasAlpha; }

    void Reset() { SetUniform(ColorDef::White()); }
    void SetUniform(ColorDef color) { SetUniform(color.GetValue()); }
    void SetUniform(uint32_t color) { m_numColors = 1; m_uniform = color; }
    void SetNonUniform(uint16_t numColors, uint32_t const* colors, uint16_t const* indices, bool hasAlpha)
        {
        BeAssert(numColors > 1);
        m_numColors = numColors;
        m_nonUniform.Set(colors, indices, hasAlpha);
        }
};

//=======================================================================================
//! Describes 0 or more Features within a graphic primitive.
//! The featureIDs refer to indices into a FeatureTable associated with the Graphic
//! containing the primitive.
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct FeatureIndex
{
    enum class Type
    {
        Empty, //!< No Features defined for this primitive. Union members invalid.
        Uniform, //!< One Feature defined for this primitive. m_featureID holds the feature ID.
        NonUniform //!< Multiple Features defined for this primitive. m_featureIDs holds per-vertex feature IDs.
    };

    Type    m_type = Type::Empty;
    union
        {
        uint32_t        m_featureID;        // If m_numFeatures == 1, the ID of the single Feature within this primitive
        uint32_t const* m_featureIDs;       // If m_numFeatures > 1, per-vertex Feature IDs
        };

    FeatureIndex() { m_featureIDs = nullptr; }

    constexpr bool IsUniform() const { return Type::Uniform == m_type; }
    constexpr bool IsEmpty() const { return Type::Empty == m_type; }
    constexpr bool IsNonUniform() const { return Type::NonUniform == m_type; }

    void Reset() { *this = FeatureIndex(); }
};

//=======================================================================================
//! Represents a normal vector compressed to a 16-bit unsigned integer value. This is
//! a lossy compression.
//! Oct encoding is a compact representation of unit length vectors.
//! The 'oct' encoding is described in "A Survey of Efficient Representations of Independent Unit Vectors",
//! Cigolle et al 2014: http://jcgt.org/published/0003/02/01/
//! @bsistruct                                                   Paul.Connelly   06/17
//=======================================================================================
struct OctEncodedNormal
{
private:
    uint16_t    m_value;

    static constexpr double Clamp(double val, double minVal, double maxVal) { return val < minVal ? minVal : (val > maxVal ? maxVal : val); }
    static constexpr double SignNotZero(double val) { return val < 0.0 ? -1.0 : 1.0; }
    static constexpr uint16_t ToUInt16(double val) { return static_cast<uint16_t>(.5 + (Clamp(val, -1.0, 1.0) * 0.5 + 0.5) * 255.0); }

    static DVec3d Decode(uint16_t value)
        {
        auto ex = static_cast<double>(value & 0xff),
             ey = static_cast<double>(value >> 8);
        ex = ex / 255.0 * 2.0 - 1.0;
        ey = ey / 255.0 * 2.0 - 1.0;

        DVec3d n = DVec3d::From(ex, ey, 1.0 - (std::fabs(ex) + std::fabs(ey)));
        if (n.z < 0.0)
            {
            double x = n.x, y = n.y;
            n.x = (1.0 - std::fabs(y)) * SignNotZero(x);
            n.y = (1.0 - std::fabs(x)) * SignNotZero(y);
            }

        n.Normalize();
        return n;
        }

    static uint16_t Encode(DVec3dCR vec)
        {
        VerifyNormalized(vec);
        double denom = std::fabs(vec.x) + std::fabs(vec.y) + std::fabs(vec.z),
               rx = vec.x / denom,
               ry = vec.y / denom;
        if (vec.z < 0)
            {
            double x = rx, y = ry;
            rx = (1.0 - std::fabs(y)) * SignNotZero(x);
            ry = (1.0 - std::fabs(x)) * SignNotZero(y);
            }

        uint16_t value = ToUInt16(ry) << 8 | ToUInt16(rx);
        VerifyEncoded(value, vec);

        return value;
        }

#if !defined(NDEBUG)
    DGNPLATFORM_EXPORT static void VerifyNormalized(DVec3dCR);
    DGNPLATFORM_EXPORT static void VerifyEncoded(uint16_t encoded, DVec3dCR input);
#else
    static void VerifyNormalized(DVec3dCR) { }
    static void VerifyEncoded(uint16_t, DVec3dCR) { }
#endif
public:
    //! Directly initialize from a previously-computed oct-encoding.
    void InitFrom(uint16_t value) { m_value = value; }

    //! Initialize from a vector. The input vector must be normalized. This function will not attempt to normalize it for you.
    void InitFrom(DVec3dCR vec)
        {
        m_value = Encode(vec);
        }

    //! Initialize from a vector. The input vector must be normalized. This function will not attempt to normalize it for you.
    void InitFrom(FVec3dCR vec) { InitFrom(DVec3d::From(vec)); }

    //! Returns an OctEncodedNormal computed from the input vector. The input vector must be normalized beforehand.
    static OctEncodedNormal From(DVec3dCR vec) { OctEncodedNormal n; n.InitFrom(vec); return n; }
    //! Returns an OctEncodedNormal initialized from a previously-computed oct-encoding.
    static OctEncodedNormal From(uint16_t val) { OctEncodedNormal n; n.InitFrom(val); return n; }
    //! Returns an OctEncodedNormal computed from the input vector. The input vector must be normalized beforehand.
    static OctEncodedNormal From(FVec3dCR vec) { return From(DVec3d::From(vec)); }

    //! Returns the 16-bit encoded value.
    uint16_t Value() const { return m_value; }

    //! Returns the decoded normalized vector represented by this OctEncodedNormal.
    DVec3d Decode() const { return Decode(Value()); }
    //! Returns the decoded normalized vector represented by this OctEncodedNormal.
    FVec3d Decode32() const { return FVec3d::From(Decode(Value())); }

    bool operator==(OctEncodedNormal rhs) const { return Value() == rhs.Value(); }
    bool operator!=(OctEncodedNormal rhs) const { return !(*this == rhs); }
    bool operator<(OctEncodedNormal rhs) const { return Value() < rhs.Value(); }
    bool operator>(OctEncodedNormal rhs) const { return Value() > rhs.Value(); }
    OctEncodedNormalR operator=(OctEncodedNormal rhs) { m_value = rhs.Value(); return *this; }
};

typedef bvector<OctEncodedNormal> OctEncodedNormalList;
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(OctEncodedNormalList)

typedef bpair<OctEncodedNormal,OctEncodedNormal> OctEncodedNormalPair;
typedef bvector<OctEncodedNormalPair> OctEncodedNormalPairList;
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(OctEncodedNormalPair)
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(OctEncodedNormalPairList)

//! Common operations for QPoint2d and QPoint3d
namespace Quantization
{
    constexpr double RangeScale() { return static_cast<double>(0xffff); }
    constexpr double ComputeScale(double extent) { return 0.0 == extent ? extent : RangeScale() / extent; }

    inline bool IsInRange(double quantizedPos)
        {
        return quantizedPos >= 0.0 && quantizedPos < RangeScale() + 1.0; // rounding term of 0.5 added...double value floored when convert to uint16_t
        }

    inline /*constexpr*/ double QuantizeDouble(double pos, double origin, double scale)
        {
        return std::max(0.0, std::min(RangeScale(), 0.5 + (pos - origin) * scale));
        }

    inline bool IsQuantizable(double pos, double origin, double scale)
        {
        return IsInRange(QuantizeDouble(pos, origin, scale));
        }

    inline uint16_t Quantize(double pos, double origin, double scale)
        {
        double qpos = QuantizeDouble(pos, origin, scale);
        return static_cast<uint16_t>(qpos);
        }

    constexpr double Unquantize(uint16_t pos, double origin, double scale)
        {
        return 0.0 == scale ? origin : origin + pos/scale;
        }

    constexpr double UnquantizeAboutCenter(uint16_t pos, double origin, double scale)
        {
        return 0.0 == scale ? 0.0 : (static_cast<double>(pos) - 0x7fff) * (pos/scale);
        }

    template<typename T> class QPointList : public bvector<T>
    {
    public:
        typedef typename T::Params Params;
        typedef typename T::T_Range Range;
        typedef typename T::T_DPoint DPoint;
        typedef typename T::T_DVec DVec;
        typedef typename T::T_FPoint FPoint;

        //! Construct an empty list to be quantized by the specified params
        explicit QPointList(Params const& params) : m_params(params) { }
        //! Construct an empty list to be quantized to the specified range
        explicit QPointList(Range const& range ) : QPointList(Params(range)) { }
        QPointList() { }
        //! Construct a copy of another list
        QPointList(QPointList const& src) : bvector<T>(src), m_params(src.m_params) { }
        //! Move-construct a copy of another list
        QPointList(QPointList&& src) : bvector<T>(std::move(src)), m_params(src.m_params) { }
        //! Populate a list of points quantized to the range of the input points
        template<typename T_UnquantizedPoint> QPointList(T_UnquantizedPoint const* pts, size_t nPts) { InitFrom(pts, nPts); }

        QPointList& operator=(QPointList const& src) { Assign(src.data(), src.size(), src.m_params); return *this; }
        QPointList& operator=(QPointList&& src) { this->swap(src); m_params = src.m_params; return *this; }

        //! Reset this list's parameters to the range of the input points, and replace its contents with the input points quantized to that range.
        template<typename T_UnquantizedPoint> void InitFrom(T_UnquantizedPoint const* pts, size_t nPts)
            {
            Range range = Range::NullRange();
            for (size_t i = 0; i < nPts; i++)
                range.Extend(ToDPoint(pts[i]));

            Reset(Params(range));
            for (size_t i = 0; i < nPts; i++)
                Add(pts[i]);
            }

        //! Replace the contents of this list with the specified points, quantized to the specified params
        void Assign(T const* points, size_t nPoints, Params const& params)
            {
            m_params = params;
            this->assign(points, points+nPoints);
            }

        //! Empty this list and change its quantization parameters
        void Reset(Params const& params)
            {
            m_params = params;
            this->clear();
            }

        Params const& GetParams() const { return m_params; }
        void SetParams(Params const& params) { m_params = params; }

        //! Quantize the specified point and add it to this list
        void Add(DPoint const& dpt) { this->push_back(T(dpt, GetParams())); }
        //! Quantize the specified point and add it to this list
        void Add(FPoint const& fpt) { Add(ToDPoint(fpt)); }
        //! Return the unquantized point at the specified index.
        DPoint Unquantize(size_t index) const { return UnquantizeAsVector(index); }
        //! Return the point at the specified index, unquantized as a vector type.
        DVec UnquantizeAsVector(size_t index) const { BeAssert(index < this->size()); return (*this)[index].UnquantizeAsVector(GetParams()); }
        //! Return the point at the specified index.
        FPoint Unquantize32(size_t index) const { return ToFPoint(Unquantize(index)); }

        //! Requantize all the points in this list to the new parameters, and update the list's parameters.
        void Requantize(Params const& params)
            {
            for (auto& qpt : *this)
                {
                auto dpt = qpt.Unquantize(m_params);
                qpt = T(dpt, params);
                }

            m_params = params;
            }
    private:
        Params  m_params;

        // Because FPoint2d/3d and DPoint2d/3d interfaces have annoying differences which prevent us from writing generic code against them...
        static FPoint ToFPoint(DPoint const& dpt) { return T::ToFPoint(dpt); }
        static DPoint ToDPoint(FPoint const& fpt) { return T::ToDPoint(fpt); }
        static FPoint ToFPoint(FPoint const& fpt) { return fpt; }
        static DPoint ToDPoint(DPoint const& dpt) { return dpt; }
    };
}

//=======================================================================================
//! Represents a DPoint3d quantized within some known range to a triplet of 16-bit
//! integers. This is a lossy compression technique.
// @bsistruct                                                   Ray.Bentley     01/2017
//=======================================================================================
struct QPoint3d
{
    using T_Range = DRange3d;
    using T_DPoint = DPoint3d;
    using T_DVec = DVec3d;
    using T_FPoint = FPoint3d;

    static FPoint3d ToFPoint(DPoint3dCR dpt) { return FPoint3d::From(dpt); }
    static DPoint3d ToDPoint(FPoint3dCR fpt) { return DPoint3d::From(fpt); }

    uint16_t x, y, z;

    //! Describes the range associated with a QPoint3d.
    struct Params
    {
        DPoint3d    origin;
        DPoint3d    scale;

        Params() : Params(DRange3d::NullRange()) { }
        explicit Params(DRange3dCR range) : origin(range.low)
            {
            DVec3d diagonal = range.DiagonalVector();
            scale.x = Quantization::ComputeScale(diagonal.x);
            scale.y = Quantization::ComputeScale(diagonal.y);
            scale.z = Quantization::ComputeScale(diagonal.z);
            }

        //! Create params suitable for quantizing points with components in the range [-1.0,1.0].
        //! Depending on precision needs, consider using OctEncodedNormal instead of QPoint3d to quantize normals.
        static Params FromNormalizedRange()
            {
            return Params(DRange3d::From(DPoint3d::FromXYZ(-1.0,-1.0,-1.0), DPoint3d::FromXYZ(1.0,1.0,1.0)));
            }

        DPoint3dCR GetOrigin() const { return origin; }
        DPoint3dCR GetScale() const { return scale; }
        DRange3d GetRange() const { return DRange3d::From (origin, QPoint3d((uint16_t) Quantization::RangeScale(), (uint16_t)Quantization::RangeScale(), (uint16_t)Quantization::RangeScale()).Unquantize(*this)); }

    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Params);

    QPoint3d() { }
    QPoint3d(uint16_t x_, uint16_t y_, uint16_t z_) : x(x_), y(y_), z(z_) { }
    QPoint3d(DPoint3dCR pt, DRange3dCR range) : QPoint3d(pt, Params(range)) { }
    QPoint3d(DPoint3dCR pt, ParamsCR params)
        {
        x = Quantization::Quantize(pt.x, params.origin.x, params.scale.x);
        y = Quantization::Quantize(pt.y, params.origin.y, params.scale.y);
        z = Quantization::Quantize(pt.z, params.origin.z, params.scale.z);
        }
    QPoint3d(FPoint3dCR pt, DRange3dCR range) : QPoint3d(pt, Params(range)) { }
    QPoint3d(FPoint3dCR pt, ParamsCR params) : QPoint3d(ToDPoint(pt), params) {}

    //! Decode this QPoint3d into a DPoint3d using the same params from which the QPoint3d was created.
    DPoint3d Unquantize(ParamsCR params) const { return UnquantizeAsVector(params); }

    //! Decode this QPoint3d into a DVec3d using the same params from which the QPoint3d was created.
    DVec3d UnquantizeAsVector(ParamsCR params) const
        {
        return DVec3d::From(
            Quantization::Unquantize(x, params.origin.x, params.scale.x),
            Quantization::Unquantize(y, params.origin.y, params.scale.y),
            Quantization::Unquantize(z, params.origin.z, params.scale.z));
        }

    //! Decode this QPoint3d into an FPoint3d, with the center of the original range translated to (0,0,0).
    FPoint3d UnquantizeAboutCenter(ParamsCR params) const
        {
        return FPoint3d::From(
            Quantization::UnquantizeAboutCenter(x, params.origin.x, params.scale.x),
            Quantization::UnquantizeAboutCenter(y, params.origin.y, params.scale.y),
            Quantization::UnquantizeAboutCenter(z, params.origin.z, params.scale.z));
        }

    //! Decode this QPoint3d into a FPoint3d using the same params from which the QPoint3d was created.
    FPoint3d Unquantize32(ParamsCR params) const
        {
        return ToFPoint(Unquantize(params));
        }

    bool operator==(QPoint3dCR rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    bool operator!=(QPoint3dCR rhs) const { return !(*this == rhs); }
};

//=======================================================================================
//! Represents a DPoint2d quantized within some known range to a pair of 16-bit integers.
//! This is a lossy compression technique.
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct QPoint2d
{
    using T_Range = DRange2d;
    using T_DPoint = DPoint2d;
    using T_DVec = DVec2d;
    using T_FPoint = FPoint2d;

    static FPoint2d ToFPoint(DPoint2dCR dpt) { FPoint2d fpt; fpt.x = static_cast<float>(dpt.x); fpt.y = static_cast<float>(dpt.y); return fpt; }
    static DPoint2d ToDPoint(FPoint2dCR fpt) { return DPoint2d::From(fpt.x, fpt.y); }

    uint16_t x, y;

    //! Describes the range associated with a QPoint2d.
    struct Params
    {
        DPoint2d    origin;
        DPoint2d    scale;

        Params() : Params(DRange2d::NullRange()) { }
        explicit Params(DRange2dCR range) : origin(range.low)
            {
            DVec2d diagonal = range.IsNull() ? DVec2d::From(0, 0) : DVec2d::FromStartEnd(range.low, range.high);
            scale.x = Quantization::ComputeScale(diagonal.x);
            scale.y = Quantization::ComputeScale(diagonal.y);
            }

        DPoint2dCR GetOrigin() const { return origin; }
        DPoint2dCR GetScale() const { return scale; }
        DRange2d GetRange() const { return DRange2d::From(origin, QPoint2d((uint16_t)Quantization::RangeScale(), (uint16_t)Quantization::RangeScale()).Unquantize(*this)); }
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Params);

    QPoint2d() { }
    QPoint2d(uint16_t x_, uint16_t y_) : x(x_), y(y_) { }
    QPoint2d(DPoint2dCR pt, DRange2dCR range) : QPoint2d(pt, Params(range)) { }
    QPoint2d(FPoint2dCR pt, DRange2dCR range) : QPoint2d(pt, Params(range)) { }
    QPoint2d(FPoint2dCR pt, ParamsCR params) : QPoint2d(ToDPoint(pt), params) { }
    QPoint2d(DPoint2dCR pt, ParamsCR params)
        {
        x = Quantization::Quantize(pt.x, params.origin.x, params.scale.x);
        y = Quantization::Quantize(pt.y, params.origin.y, params.scale.y);
        }

    //! Decode this QPoint2d into a DPoint2d using the same params from which the QPoint2d was created.
    DPoint2d Unquantize(ParamsCR params) const { return UnquantizeAsVector(params); }

    //! Decode this QPoint2d into a FPoint2d using the same params from which the QPoint2d was created.
    FPoint2d Unquantize32(ParamsCR params) const
        {
        return ToFPoint(Unquantize(params));
        }

    //! Decode this QPoint2d into a DVec2d using the same params from which the QPoint2d was created.
    DVec2d UnquantizeAsVector(ParamsCR params) const
        {
        return DVec2d::From(
            Quantization::Unquantize(x, params.origin.x, params.scale.x),
            Quantization::Unquantize(y, params.origin.y, params.scale.y));
        }
};

//=======================================================================================
//! Represents a scalar value quantized within some known range to a 16-bit integer.
//! This is a lossy compression technique.
// @bsistruct                                                   Paul.Connelly   06/17
//=======================================================================================
struct QPoint1d
{
    struct Params
    {
        double  origin;
        double  scale;

        Params() : Params(DRange1d::NullRange()) { }
        explicit Params(DRange1d range) : origin(range.low), scale(range.IsNull() ? 0.0 : Quantization::ComputeScale(range.high - range.low)) { }

        double GetOrigin() const { return origin; }
        double GetScale() const { return scale; }
        DRange1d GetRange() const { return DRange1d::From(origin, origin + Quantization::RangeScale() * scale); }
    };

    using T_Range = DRange1d;
    using T_DPoint = double;
    using T_DVec = double;
    using T_FPoint = float;

    static float ToFPoint(double dx) { return static_cast<float>(dx); }
    static double ToDPoint(float fx) { return fx; }

    uint16_t    x;

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Params);

    QPoint1d() { }
    QPoint1d(double x, DRange1d range) : QPoint1d(x, Params(range)) { }
    QPoint1d(float x, DRange1d range) : QPoint1d(ToDPoint(x), Params(range)) { }
    QPoint1d(float x, ParamsCR params) : QPoint1d(ToDPoint(x), params) { }
    QPoint1d(double x, ParamsCR params) : x(Quantization::Quantize(x, params.origin, params.scale)) { }

    double Unquantize(ParamsCR params) const { return UnquantizeAsVector(params); }
    float Unquantize32(ParamsCR params) const { return ToFPoint(Unquantize(params)); }
    double UnquantizeAsVector(ParamsCR params) const { return Quantization::Unquantize(x, params.origin, params.scale); }
};

typedef Quantization::QPointList<QPoint1d> QPoint1dList;
typedef Quantization::QPointList<QPoint2d> QPoint2dList;
typedef Quantization::QPointList<QPoint3d> QPoint3dList;

DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(QPoint1dList)
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(QPoint2dList)
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(QPoint3dList)

//=======================================================================================
//! Describes the type of fill associated with a mesh.
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
enum class FillFlags : uint8_t
{
    None        = 0,                    //<! No fill, e.g. for any non-planar geometry.
    ByView      = 1 << 0,               //<! Use element fill color, when fill enabled by view
    Always      = 1 << 1,               //<! Use element fill color, even when fill is disabled by view
    Behind      = 1 << 2,               //!< Always rendered behind other geometry belonging to the same element. e.g., text background.
    Blanking    = Behind | Always,    //<! Use element fill color, always rendered behind other geometry belonging to the same element.
    Background  = 1 << 3,               //<! Use background color specified by view
};

ENUM_IS_FLAGS(FillFlags);

//=======================================================================================
//! Describes the semantics of an indexed polyline primitive.
// @bsistruct                                                   Paul.Connelly   04/18
//=======================================================================================
struct PolylineFlags
{
private:
    enum { kType_Normal, kType_Edge, kType_Outline };

    struct Bits
    {
        uint8_t m_disjoint: 1;
        uint8_t m_planar: 1;
        uint8_t m_2d: 1;
        uint8_t m_type: 2;
    };

    union
    {
        uint8_t m_value;
        Bits    m_bits;
    };

    explicit PolylineFlags(uint8_t value) { m_value = value; }
public:
    PolylineFlags() : PolylineFlags(0) { }
    PolylineFlags(bool is2d, bool isPlanar) : PolylineFlags()
        {
        if (is2d)       SetIs2d();
        if (isPlanar)   SetIsPlanar();
        }

    bool IsDisjoint() const { return m_bits.m_disjoint; }
    bool IsPlanar() const { return m_bits.m_planar; }
    bool Is2d() const { return m_bits.m_2d; }
    bool IsOutlineEdge() const { return kType_Outline == m_bits.m_type; }
    bool IsNormalEdge() const { return kType_Edge == m_bits.m_type; }
    bool IsAnyEdge() const { return kType_Normal != m_bits.m_type; }

    uint8_t GetValue() const { return m_value; }

    void SetIsDisjoint() { m_bits.m_disjoint = 1; }
    void SetIsPlanar() { m_bits.m_planar = 1; }
    void SetIs2d() { m_bits.m_2d = 1; }
    void SetIsNormalEdge() { m_bits.m_type = kType_Edge; }
    void SetIsOutlineEdge() { m_bits.m_type = kType_Outline; }

    static PolylineFlags FromValue(uint8_t value) { return PolylineFlags(value); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct MeshPolyline
{
private:
    bvector<uint32_t>   m_indices;
    double              m_startDistance;
public:
    MeshPolyline () : m_startDistance(0.0) { }
    explicit MeshPolyline (double startDistance) : m_startDistance(startDistance) { }
    MeshPolyline (double startDistance, bvector<uint32_t>&& indices) : m_startDistance(startDistance), m_indices(std::move(indices)) { }

    bvector<uint32_t> const& GetIndices() const { return m_indices; }
    bvector<uint32_t>& GetIndices() { return m_indices; }
    double GetStartDistance() const { return m_startDistance; }

    void AddIndex(uint32_t index)  { if (m_indices.empty() || m_indices.back() != index) m_indices.push_back(index); }
    void Clear() { m_indices.clear(); }
 };

//=======================================================================================
//! Information needed to draw a set of indexed polylines using a shared vertex buffer.
// @bsistruct                                                   Paul.Connelly   01/17
//=======================================================================================
struct IndexedPolylineArgs
{
    //! An individual polyline which indexes into a shared set of vertices
    struct Polyline
    {
        uint32_t const* m_vertIndex = nullptr;
        uint32_t        m_numIndices = 0;
        double          m_startDistance = 0.0;

        Polyline() { }
        Polyline(uint32_t const* indices, uint32_t numIndices, double startDistance) : m_vertIndex(indices), m_numIndices(numIndices), m_startDistance(startDistance) { }

        bool IsValid() const { return 0 < m_numIndices; }
        void Reset() { m_numIndices = 0; m_vertIndex = nullptr; m_startDistance = 0.0; }
        bool Init(MeshPolylineCR polyline)
            {
            m_numIndices = static_cast<uint32_t>(polyline.GetIndices().size());
            m_vertIndex = 0 < m_numIndices ? polyline.GetIndices().data() : nullptr;
            m_startDistance = polyline.GetStartDistance();
            return IsValid();
            }
    };

    QPoint3dCP          m_points = nullptr;
    Polyline const*     m_lines = nullptr;
    uint32_t            m_numPoints = 0;
    uint32_t            m_numLines = 0;
    ColorIndex          m_colors;
    FeatureIndex        m_features;
    QPoint3d::Params    m_pointParams;
    uint32_t            m_width = 0;
    LinePixels          m_linePixels = LinePixels::Solid;
    PolylineFlags       m_flags;

    IndexedPolylineArgs() { }
    IndexedPolylineArgs(QPoint3dCP points, uint32_t numPoints, Polyline const* lines, uint32_t numLines, QPoint3d::ParamsCR pointParams, bool is2d, bool isPlanar)
        : m_points(points), m_lines(lines), m_numPoints(numPoints), m_numLines(numLines), m_pointParams(pointParams), m_flags(is2d, isPlanar) { }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     05/2017
//=======================================================================================
struct  MeshEdge
    {
    enum    Flags : uint8_t
        {
        Invisible =  1,
        Visible    = 0,
        };

    uint32_t                m_indices[2];

    MeshEdge() { }
    MeshEdge(uint32_t index0, uint32_t index1)
        {
        if (index0 < index1)
            {
            m_indices[0] = index0;
            m_indices[1] = index1;
            }
        else
            {
            m_indices[0] = index1;
            m_indices[1] = index0;
            }
        }

    bool operator < (MeshEdge const& rhs) const;
    };

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     05/2017
//=======================================================================================
struct MeshEdges : RefCountedBase
{
    bvector<MeshEdge>           m_visible;
    bvector<MeshEdge>           m_silhouette;
    bvector<MeshPolyline>       m_polylines;
    OctEncodedNormalPairList    m_silhouetteNormals;

    MeshEdges() { }
};

#ifdef UNUSED_FOR_IMODELJS
//=======================================================================================
// @bsistruct                                                   Ray.Bentley     04/2018
//=======================================================================================
template <typename T_Data>  struct AuxChannel : RefCountedBase
{
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Data);
    DEFINE_REF_COUNTED_PTR(Data);

    struct Data : RefCountedBase
        {
        private:
        float               m_input;
        bvector<T_Data>     m_values;

        public:
        float                       GetInput() const                     { return m_input; }
        bvector<T_Data> const&      GetValues() const                    { return m_values; }
        bvector<T_Data>&            GetValues()                          { return m_values; }
        T_Data const&               GetValue(size_t i) const             { return m_values.at(i); }
        size_t                      GetValueByteCount() const            { return m_values.size() * sizeof(T_Data); }
        Data(float input, bvector<T_Data>&& values) : m_input(input), m_values(values) { }
        };
private:
    bvector<DataPtr>         m_data;

public:
        AuxChannel( bvector<DataPtr>&& data) : m_data(std::move(data)) { }
        bool IsAnimatable() const { return m_data.size() > 1; }
        void AppendDataByIndex(RefCountedPtr<AuxChannel>& output, size_t index)
            {
            if (!output.IsValid())
                output =  CloneWithoutData();

            for (size_t i=0; i<m_data.size(); i++)
                output->m_data[i]->GetValues().push_back(m_data[i]->GetValue(index));
            }
        void AppendDataByIndex(RefCountedPtr<AuxChannel>& output, size_t index)
            {
            if (!output.IsValid())
                output =  CloneWithoutData();

            for (size_t i=0; i<m_data.size(); i++)
                output->m_data[i]->GetValues().push_back(m_data[i]->GetValue(index));
            }
        AuxChannel* CloneWithoutData()
            {
            bvector<DataPtr>    dataVector;

            for (auto& data : m_data)
                {
                bvector<T_Data>     values;
                dataVector.push_back (new Data(data->GetInput(), std::move(values)));
                }

            return new AuxChannel(std::move(dataVector));
            }
        bvector<DataPtr> const& GetData() const { return m_data; }
};

using AuxDisplacementChannel = AuxChannel<FPoint3d>;
using AuxParamChannel = AuxChannel<FPoint2d>;

DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(AuxDisplacementChannel);
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(AuxParamChannel);
DEFINE_REF_COUNTED_PTR(AuxDisplacementChannel);
DEFINE_REF_COUNTED_PTR(AuxParamChannel);
#endif

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     04/2017
//=======================================================================================
struct EdgeArgs
{
    MeshEdgeCP          m_edges = nullptr;
    uint32_t            m_numEdges = 0;

    void Clear() { *this = EdgeArgs(); }
    bool Init(MeshEdgesCR meshEdges);
    bool IsValid() const { BeAssert((nullptr == m_edges) == (0 == m_numEdges)); return nullptr != m_edges; }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     04/2017
//=======================================================================================
struct SilhouetteEdgeArgs : EdgeArgs
{
    OctEncodedNormalPairCP  m_normals = nullptr;

    void Clear() { *this = SilhouetteEdgeArgs(); }
    bool Init(MeshEdgesCR);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct PolylineEdgeArgs
 {
    using Polyline = IndexedPolylineArgs::Polyline;

    Polyline const*     m_lines = nullptr;
    uint32_t            m_numLines = 0;

    PolylineEdgeArgs() = default;
    PolylineEdgeArgs(Polyline const* lines, uint32_t numLines) : m_lines(lines), m_numLines(numLines) { }

    bool IsValid() const { BeAssert((nullptr == m_lines) == (0 == m_numLines)); return nullptr != m_lines; }
    void Clear() { *this = PolylineEdgeArgs(); }
    bool Init(bvector<Polyline> const& polylines)
        {
        m_numLines = static_cast<uint32_t>(polylines.size());
        m_lines = 0 < m_numLines ? polylines.data() : nullptr;
        return IsValid();
        }
};

//=======================================================================================
//! The material or material atlas associated with a TriMeshArgs.
// @bsistruct                                                   Paul.Connelly   07/19
//=======================================================================================
struct TriMeshMaterial
{
    // Non-null if a single material
    MaterialPtr         m_material;
    // Non-null if more than one material
    Primitives::MaterialAtlasPtr    m_atlas;
    // If more than one material, the per-vertex indices into the atlas
    uint8_t const*      m_indices;
};

//=======================================================================================
//! Information needed to draw a triangle mesh and its edges.
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct TriMeshArgs
{
    // The vertices of the edges are shared with those of the surface
    struct Edges
        {
        EdgeArgs                    m_edges;
        SilhouetteEdgeArgs          m_silhouettes;
        PolylineEdgeArgs            m_polylines;
        uint32_t                    m_width = 0;
        LinePixels                  m_linePixels = LinePixels::Solid;

        void Clear() { *this = Edges(); }
        bool IsValid() const { return m_edges.IsValid() || m_silhouettes.IsValid() || m_polylines.IsValid(); }
        };

    Edges                           m_edges;
    uint32_t                        m_numIndices = 0;
    uint32_t const*                 m_vertIndex = nullptr;
    uint32_t                        m_numPoints = 0;
    QPoint3dCP                      m_points= nullptr;
    OctEncodedNormalCP              m_normals = nullptr;
    FPoint2d const*                 m_textureUV= nullptr;
    TexturePtr                      m_texture;
    ColorIndex                      m_colors;
    FeatureIndex                    m_features;
    QPoint3d::Params                m_pointParams;
    TriMeshMaterial                 m_material;
    FillFlags                       m_fillFlags = FillFlags::None;
    bool                            m_isPlanar = false;
    bool                            m_is2d = false;
    PolyfaceAuxData::Channels       m_auxChannels;


    DGNPLATFORM_EXPORT PolyfaceHeaderPtr ToPolyface() const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct PointCloudArgs
{
    QPoint3dCP          m_points;
    ByteCP              m_colors;
    QPoint3d::Params    m_qParams;
    int32_t             m_numPoints;

    PointCloudArgs() : PointCloudArgs(QPoint3d::Params(DRange3d::NullRange()), 0, nullptr, nullptr) { }
    PointCloudArgs(QPoint3d::Params qParams, int32_t numPoints, QPoint3dCP points, ByteCP colors)
        : m_points(points), m_colors(colors), m_qParams(qParams), m_numPoints(numPoints) { }
};


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct GraphicList : RefCounted<NonCopyableClass>
{
    using List = bvector<GraphicPtr>;
private:
    List    m_list;
public:
    typedef List::const_iterator const_iterator;

    const_iterator begin() const { return m_list.begin(); }
    const_iterator end() const { return m_list.end(); }
    size_t size() const { return m_list.size(); }
    bool empty() const { return m_list.empty(); }
    void clear() { m_list.clear(); }

    void Add(GraphicR graphic) { m_list.push_back(&graphic); }
    uint32_t GetCount() const { return static_cast<uint32_t>(size()); }
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/16
//=======================================================================================
struct FrustumPlanes
{
    bool m_isValid = false;
    ClipPlane m_planes[6];

    FrustumPlanes() {}
    ~FrustumPlanes() {}
    explicit FrustumPlanes(FrustumCR frustum){Init(frustum);}
    DGNPLATFORM_EXPORT void Init(FrustumCR frustum);
    bool IsValid() const {return m_isValid;}
    enum struct Contained {Outside = 0, Partly = 1,Inside = 2,};
    Contained Contains(FrustumCR box) const {return Contains(box.m_pts, 8);}
    bool Intersects(FrustumCR box) const {return Contained::Outside != Contains(box);}
    bool ContainsPoint(DPoint3dCR pt, double tolerance=1.0e-8) const {return Contained::Outside != Contains(&pt, 1, tolerance);}

    DGNPLATFORM_EXPORT Contained Contains(DPoint3dCP, int nPts, double tolerance=1.0e-8) const;
    DGNPLATFORM_EXPORT bool IntersectsRay(DPoint3dCR origin, DVec3dCR direction);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/17
//=======================================================================================
struct HiddenLineParams
{
    struct Style
    {
        bool m_ovrColor;
        ColorDef m_color;
        LinePixels m_pattern;
        uint32_t m_width; // 0 means don't override
        Style(bool ovrColor, ColorDef color, LinePixels pattern, uint32_t width) : m_ovrColor(ovrColor), m_color(color), m_pattern(pattern), m_width(width){}
        bool operator==(Style const& rhs) const {return m_ovrColor==rhs.m_ovrColor && m_color==rhs.m_color && m_pattern==rhs.m_pattern && m_width==rhs.m_width;}
        bool operator!=(Style const& rhs) const {return !(*this==rhs);}

        BE_JSON_NAME(width);
        BE_JSON_NAME(ovrColor);
        BE_JSON_NAME(color);
        BE_JSON_NAME(pattern);
        Json::Value ToJson() const;
        void FromJson(JsonValueCR);
    };

    Style m_visible = Style(false, ColorDef(), LinePixels::Solid, 1);
    Style m_hidden = Style(false, ColorDef(), LinePixels::HiddenLine, 1);
    double m_transparencyThreshold = 1.0;

    bool operator==(HiddenLineParams const& rhs) const {return m_visible==rhs.m_visible && m_hidden==rhs.m_hidden && m_transparencyThreshold==rhs.m_transparencyThreshold;}
    bool operator!=(HiddenLineParams const& rhs) const {return !(*this==rhs);}

    BE_JSON_NAME(hidden);
    BE_JSON_NAME(visible);
    BE_JSON_NAME(transThreshold);
    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    DGNPLATFORM_EXPORT static HiddenLineParams FromJson(JsonValueCR);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/19
//=======================================================================================
struct AmbientOcclusionParams
{
    BE_JSON_NAME(bias);
    BE_JSON_NAME(zLengthCap);
    BE_JSON_NAME(intensity);
    BE_JSON_NAME(texelStepSize);
    BE_JSON_NAME(blurDelta);
    BE_JSON_NAME(blurSigma);
    BE_JSON_NAME(blurTexelStepSize);

    double m_bias = 0.25;
    double m_zLengthCap = 0.0025;
    double m_intensity = 2.0;
    double m_texelStepSize = 1.95;
    double m_blurDelta = 1.0;
    double m_blurSigma = 2.0;
    double m_blurTexelStepSize = 1.0;

    bool operator!=(AmbientOcclusionParams const& rhs) { return !(*this == rhs); }
    bool operator==(AmbientOcclusionParams const& rhs)
        {
        return m_bias == rhs.m_bias && m_zLengthCap == rhs.m_zLengthCap && m_intensity == rhs.m_intensity && m_texelStepSize == rhs.m_texelStepSize
            && m_blurDelta == rhs.m_blurDelta && m_blurSigma == rhs.m_blurSigma && m_blurTexelStepSize == rhs.m_blurTexelStepSize;
        }

    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    DGNPLATFORM_EXPORT static AmbientOcclusionParams FromJson(JsonValueCR);
};

//=======================================================================================
//! A Light that illuminates Graphics in a scene. Render::Lights are created from Lighting::Parameters
// @bsiclass                                                    Keith.Bentley   03/17
//=======================================================================================
struct Light : RefCounted<NonCopyableClass>
{
};
DEFINE_REF_COUNTED_PTR(Light)

//=======================================================================================
// @bsiclass                                                    Ray.Benley      01/2018
//=======================================================================================
struct ImageLight
{
    enum class Mapping : uint32_t { Spherical, Cylindrical, Angular, Rectangular, Invalid };

    template <typename T_Image> struct T_Map : RefCounted<NonCopyableClass>
        {
        T_Map() { }
        T_Map(T_Image&& image, Mapping mapping, DPoint2dCR offset = DPoint2d::FromZero(), double gamma = 1.0, bool viewOriented = false) :
              m_image(std::move(image)), m_mapping(mapping), m_offset(offset), m_gamma(gamma), m_viewOriented(viewOriented) { }

        T_Image         m_image;
        Mapping         m_mapping = Mapping::Invalid;
        bool            m_viewOriented = false;
        DPoint2d        m_offset = DPoint2d::FromZero();
        double          m_gamma = 0.0;
        bool IsValid() { return m_image.IsValid(); }
        T_Image const&  GetImage() { return m_image; }
        };

    typedef struct T_Map<Image>             Map;
    typedef struct T_Map<HDRImage>          HDRMap;
    typedef struct T_Map<bvector<HDRImage>> HDRMultiMap;

    DEFINE_REF_COUNTED_PTR(Map)
    DEFINE_REF_COUNTED_PTR(HDRMap)
    DEFINE_REF_COUNTED_PTR(HDRMultiMap)


    struct Solar    // Some images include a simple solar light...
        {
        DVec3d                      m_direction = DVec3d::From(0.0, 0.0, 0.0);
        ColorDef                    m_color = ColorDef::White();
        double                      m_intensity = 0.0;
        };

    static HDRMapPtr DiffuseFromSmartIBL (BeFileNameCR fileName, HDRImage::Encoding encoding = HDRImage::Encoding::RGBM);
    static HDRMapPtr ReflectionFromSmartIBL (BeFileNameCR fileName, HDRImage::Encoding encoding = HDRImage::Encoding::RGBM);
    static MapPtr    BackgroundFromSmartIBL (BeFileNameCR fileName);
    static BentleyStatus SolarFromSmartIBL(Solar& solar, BeFileNameCR fileName);

};


//=======================================================================================
//! A list of Render::Lights, plus the f-stop setting for the camera
// @bsiclass                                                    Keith.Bentley   03/17
//=======================================================================================
struct SceneLights : RefCounted<NonCopyableClass>
{
    double                          m_fstop = 0.0; //!< must be between -3 and +3
    bvector<LightPtr>               m_list;

    // Image based lighting...

    struct
        {
        Render::TexturePtr          m_environmentMap;       // Reflections
        Render::TexturePtr          m_diffuseImage;
        ImageLight::Solar           m_solar;
        } m_imageBased;

    void AddLight(LightPtr light) {if (light.IsValid()) m_list.push_back(light);}
    bool IsEmpty() const {return m_list.empty();}

};
DEFINE_REF_COUNTED_PTR(SceneLights)

//=======================================================================================
//! Describes the effect applied to hilited elements within a view.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct HiliteSettings
{
    //! Describes the width of the outline applied to hilited geometry.
    enum class Silhouette
    {
        None, //!< No silhouette
        Thin, //!< A thin silhouette
        Thick //!< A thick silhouette
    };

    struct Defaults
    {
        static ColorDef Color() {return ColorDef(0x23,0xbb,0xfc);};
        static double VisibleRatio() {return 0.25;}
        static double HiddenRatio() {return 0.0;}
        static HiliteSettings::Silhouette Width() {return HiliteSettings::Silhouette::Thin;}
    };
private:
    ColorDef    m_color;
    double      m_visibleRatio;
    double      m_hiddenRatio;
    Silhouette  m_silhouette;

    static void Clamp(double& value) { value = std::min(1.0, std::max(0.0, value)); }
public:
    explicit HiliteSettings(ColorDef color=Defaults::Color(), double visibleRatio=Defaults::VisibleRatio(), double hiddenRatio=Defaults::HiddenRatio(), Silhouette silhouette=Defaults::Width())
        : m_color(color), m_visibleRatio(visibleRatio), m_hiddenRatio(hiddenRatio), m_silhouette(silhouette)
        {
        Clamp(m_hiddenRatio);
        }

    //! The hilite color
    ColorDef GetColor() const { return m_color; }
    //! The ratio to which the hilite color is mixed with the element color for visible portions of the element. 1.0 = all hilite color; 0.0 = all element color.
    double GetVisibleRatio() const { return m_visibleRatio; }
    //! The ratio to which the hilite color is mixed with the color of geometry which occludes hilited geometry.
    double GetHiddenRatio() const { return m_hiddenRatio; }
    //! The silhouette effect.
    Silhouette GetSilhouette() const { return m_silhouette; }
    //! Change the color, preserving all other settings
    void SetColor(ColorDef color) { m_color = color; }
};

//=======================================================================================
//! Describes a "feature" within a batched Graphic. A batched Graphic can
//! contain multiple features. Each feature is associated with a unique combination of
//! attributes (element ID, subcategory, geometry class). This allows geometry to be
//! more efficiently batched on the GPU, while enabling features to be resymbolized
//! individually.
//!
//! As a simple example, a single mesh primitive may contain geometry for 3 elements,
//! all belonging to the same subcategory and geometry class. The mesh would therefore
//! contain 3 Features. Each vertex within the mesh would be associated with the
//! index of the Feature to which it belongs, where the index is determined by the
//! FeatureTable associated with the primitive.
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct Feature
{
private:
    DgnElementId        m_elementId;
    DgnSubCategoryId    m_subCategoryId;
    DgnGeometryClass    m_class = DgnGeometryClass::Primary;
public:
    Feature() : Feature(DgnElementId(), DgnSubCategoryId(), DgnGeometryClass::Primary) { }
    Feature(DgnElementId elementId, DgnSubCategoryId subCatId, DgnGeometryClass geomClass) : m_elementId(elementId), m_subCategoryId(subCatId), m_class(geomClass) { }

    DgnElementId GetElementId() const { return m_elementId; }
    DgnSubCategoryId GetSubCategoryId() const { return m_subCategoryId; }
    DgnGeometryClass GetClass() const { return m_class; }

    bool operator!=(FeatureCR rhs) const { return !(*this == rhs); }
    bool operator==(FeatureCR rhs) const
        {
        if (IsUndefined() && rhs.IsUndefined())
            return true;
        else
            return GetElementId() == rhs.GetElementId() && GetSubCategoryId() == rhs.GetSubCategoryId() && GetClass() == rhs.GetClass();
        }

    DGNPLATFORM_EXPORT bool operator<(FeatureCR rhs) const;

    bool IsDefined() const { return m_elementId.IsValid() || m_subCategoryId.IsValid() || DgnGeometryClass::Primary != m_class; }
    bool IsUndefined() const { return !IsDefined(); }
};

//=======================================================================================
//! An entry in a PackedFeatureTable. The subcategory ID is stored as a 24-bit index into
//! an array of subcategory IDs stored elsewhere.
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct PackedFeature
{
private:
    uint64_t m_elementId;
    uint32_t m_subCategoryIndexAndClass;
public:
    PackedFeature(DgnElementId elemId, uint32_t subCategoryIndex, DgnGeometryClass geomClass) : PackedFeature(elemId.GetValueUnchecked(), subCategoryIndex | (static_cast<uint32_t>(geomClass) << 24)) { }
    PackedFeature(uint64_t elemId, uint32_t subCategoryIndexAndClass) : m_elementId(elemId), m_subCategoryIndexAndClass(subCategoryIndexAndClass) { }

    DgnElementId GetElementId() const { return DgnElementId(m_elementId); }
    uint32_t GetSubCategoryIndex() const { return m_subCategoryIndexAndClass & 0x00ffffff; }
    DgnGeometryClass GetClass() const { return static_cast<DgnGeometryClass>((m_subCategoryIndexAndClass >> 24) & 0x000000ff); }

    static constexpr size_t PackedSize() { return sizeof(uint64_t) + sizeof(uint32_t); }
};

//=======================================================================================
//! Defines a look-up table for Features within a batched Graphic. Consecutive 32-bit
//! indices are assigned to each unique Feature. Primitives within the Graphic can
//! use per-vertex indices to specify the distribution of Features within the primitive.
//! A FeatureTable can be shared amongst multiple primitives within a single Graphic, and
//! amongst multiple sub-Graphics of a Graphic.
// @bsistruct                                                   Paul.Connelly   03/17
//=======================================================================================
struct FeatureTable
{
    typedef bmap<Feature, uint32_t> Map;
private:
    Map         m_map;
    DgnModelId  m_modelId;
    uint32_t    m_maxFeatures;
public:
    explicit FeatureTable(uint32_t maxFeatures) : FeatureTable(DgnModelId(), maxFeatures) { }
    FeatureTable(DgnModelId modelId, uint32_t maxFeatures) : m_modelId(modelId), m_maxFeatures(maxFeatures) { }
    FeatureTable(FeatureTable&& src) : m_map(std::move(src.m_map)), m_modelId(src.m_modelId), m_maxFeatures(src.m_maxFeatures) { }
    FeatureTable(FeatureTableCR src) : m_map(src.m_map), m_modelId(src.m_modelId), m_maxFeatures(src.m_maxFeatures) { }
    FeatureTable& operator=(FeatureTable&& src) { m_map = std::move(src.m_map); m_modelId = src.m_modelId; m_maxFeatures = src.m_maxFeatures; return *this; }
    FeatureTable& operator=(FeatureTableCR src) { *this = FeatureTable(src); return *this; }

    //! This method potentially allocates a new index, if the specified Feature does not yet exist in the lookup table.
    uint32_t GetIndex(FeatureCR feature)
        {
        BeAssert(!IsFull());
        uint32_t index = 0;
        if (!FindIndex(index, feature) && !IsFull())
            {
            index = GetNumIndices();
            m_map[feature] = index;
            }

        return index;
        }

    //! Looks up the index of an existing Feature. Returns false if the Feature does not exist in the lookup table.
    bool FindIndex(uint32_t& index, FeatureCR feature) const
        {
        auto iter = m_map.find(feature);
        bool found = (m_map.end() != iter);
        if (found)
            index = iter->second;

        return found;
        }

    bool FindFeature(FeatureR feature, uint32_t index) const
        {
        for (auto kvp : m_map)
            {
            if (kvp.second == index)
                {
                feature = kvp.first;
                return true;
                }
            }

        return false;
        }

    DgnModelId GetModelId() const { return m_modelId; }
    uint32_t GetMaxFeatures() const { return m_maxFeatures; }
    bool IsUniform() const { return 1 == size(); }
    bool IsFull() const { BeAssert(size() <= GetMaxFeatures()); return size() >= GetMaxFeatures(); }
    uint32_t GetNumIndices() const { return static_cast<uint32_t>(size()); }
    bool AnyDefined() const { return size() > 1 || (IsUniform() && begin()->first.IsDefined()); }

    typedef Map::const_iterator const_iterator;

    const_iterator begin() const { return m_map.begin(); }
    const_iterator end() const { return m_map.end(); }
    size_t size() const { return m_map.size(); }
    bool empty() const { return m_map.empty(); }
    void clear() { m_map.clear(); }

    // Used by tile reader...
    void SetMaxFeatures(uint32_t maxFeatures) { m_maxFeatures = maxFeatures; }
    bpair<Map::iterator, uint32_t> Insert(Feature feature, uint32_t index) { return m_map.Insert(feature, index); }
    void SetModelId(DgnModelId modelId) { m_modelId = modelId; }

    DGNPLATFORM_EXPORT PackedFeatureTable Pack() const;
};

//=======================================================================================
//! Packed, immutable representation of a FeatureTable, for serialization to binary format.
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct PackedFeatureTable
{
    friend struct FeatureTable;
private:
    DgnModelId m_modelId;
    ByteStream m_bytes;
    uint32_t m_maxFeatures;
    uint32_t m_numFeatures;

    PackedFeatureTable(ByteStream&& bytes, uint32_t numFeatures, DgnModelId modelId, uint32_t maxFeatures) : m_modelId(modelId),
        m_bytes(std::move(bytes)), m_maxFeatures(maxFeatures), m_numFeatures(numFeatures) { }

    uint64_t ReadUInt64(size_t byteOffset) const
        {
        // NB: No padding is inserted to enable 64-bit-aligned access, primarily because javascript cannot read 64-bit integers, aligned or otherwise.
        BeAssert(0 == byteOffset % 4);
        BeAssert(byteOffset + sizeof(uint64_t) <= m_bytes.size());
        auto u32s = reinterpret_cast<uint32_t const*>(m_bytes.data() + byteOffset);
        uint64_t lo = u32s[0];
        uint64_t hi = u32s[1];
        return lo | (hi << 32);
        }

    size_t GetSubCategoriesOffset() const { return m_numFeatures * PackedFeature::PackedSize(); }
    DgnSubCategoryId GetSubCategoryId(uint32_t index) const
        {
        size_t byteOffset = GetSubCategoriesOffset() + index * sizeof(uint64_t);
        return DgnSubCategoryId(ReadUInt64(byteOffset));
        }
    DgnElementId GetElementId(uint32_t index) const
        {
        size_t byteOffset = index * PackedFeature::PackedSize();
        return DgnElementId(ReadUInt64(byteOffset));
        }
    uint32_t GetSubCategoryIndexAndClass(uint32_t index) const
        {
        size_t byteOffset = index * PackedFeature::PackedSize() + sizeof(uint64_t);
        return *reinterpret_cast<uint32_t const*>(m_bytes.data() + byteOffset);
        }
public:
    PackedFeatureTable(PackedFeatureTable&& src) : m_modelId(src.m_modelId), m_bytes(std::move(src.m_bytes)), m_maxFeatures(src.m_maxFeatures), m_numFeatures(src.m_numFeatures) { }
    PackedFeatureTable(PackedFeatureTableCR) = delete;

    uint32_t GetNumFeatures() const { return m_numFeatures; }
    uint32_t GetMaxFeatures() const { return m_maxFeatures; }
    DgnModelId GetModelId() const { return m_modelId; }
    ByteStreamCR GetBytes() const { return m_bytes; }

    PackedFeature GetPackedFeature(uint32_t index) const
        {
        BeAssert(index < m_numFeatures);
        return PackedFeature(GetElementId(index).GetValueUnchecked(), GetSubCategoryIndexAndClass(index));
        }

    Feature GetFeature(uint32_t index) const
        {
        auto packed = GetPackedFeature(index);
        return Feature(packed.GetElementId(), GetSubCategoryId(packed.GetSubCategoryIndex()), packed.GetClass());
        }

    DGNPLATFORM_EXPORT FeatureTable Unpack() const;
};

//=======================================================================================
//! An array of GraphicPtrs, plus an optional ViewFlags that control how this set of graphics are to be rendered.
//! @note All entries are closed (and therefore may never change) when they're added to this array.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct GraphicBranch
{
    ViewFlagsOverrides m_viewFlagsOverrides;
    bvector<GraphicPtr> m_entries;

    void Add(Graphic& graphic) {m_entries.push_back(&graphic);BeAssert(m_entries.back().IsValid());}
    void Add(bvector<GraphicPtr> const& entries) { for (auto& entry : entries) Add(*entry); }
    void SetViewFlagsOverrides(ViewFlagsOverridesCR ovr) { m_viewFlagsOverrides = ovr; }
    ViewFlags GetViewFlags(ViewFlagsCR base) const { ViewFlags flags = base; m_viewFlagsOverrides.Apply(flags); return flags; }
    void Clear() {m_entries.clear();}
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     05/17
//=======================================================================================
struct MeshEdgeCreationOptions
    {
    enum    Options
        {
        NoEdges                 = 0x0000,
        SheetEdges              = 0x0001 << 0,
        CreaseEdges             = 0x0001 << 1,
        SmoothEdges             = 0x0001 << 2,
        CreateChains            = 0x0001 << 3,
        DefaultEdges            = CreaseEdges | SheetEdges,
        AllEdges                = CreaseEdges | SheetEdges | SmoothEdges
        };

    Options     m_options               = DefaultEdges;
    double      m_minCreaseAngle        = 20.0 * msGeomConst_radiansPerDegree;

    MeshEdgeCreationOptions(Options options) : m_options(options) {}
    MeshEdgeCreationOptions(Options options, double minCreaseAngle) :m_options(options) { }

    bool GenerateAllEdges() const    { return m_options == AllEdges; }
    bool GenerateNoEdges() const     { return m_options == NoEdges;   }
    bool GenerateSheetEdges() const  { return 0 != (m_options & SheetEdges); }
    bool GenerateCreaseEdges() const { return 0 != (m_options & CreaseEdges); }
    bool CreateEdgeChains() const    { return 0 != (m_options & CreateChains); }     // Create edge chains for polyfaces that do not already have them.
    };

//=======================================================================================
//! A Render::System is the renderer-specific factory for creating Render::Graphics, Render::Textures, and Render::Materials.
//! @note The methods of this class may be called from any thread.
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct System
{
    virtual ~System() { }

    //! Initialize the rendering system. Return a non-zero value in case of error. The client thread waits for the result.
    virtual int _Initialize(void* systemWindow, bool swRendering) = 0;

    //! Find a previously-created Material by key. Returns null if no such material exists.
    virtual MaterialPtr _FindMaterial(MaterialKeyCR key, DgnDbR db) const = 0;

    //! Get or create a material from a material element, by id
    //! The default implementation uses _FindMaterial() and calls _CreateMaterial() if not found.
    DGNPLATFORM_EXPORT virtual MaterialPtr _GetMaterial(RenderMaterialId, DgnDbR) const;

    //! Create a Material from parameters
    virtual MaterialPtr _CreateMaterial(Material::CreateParams const&, DgnDbR) const = 0;

    virtual GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const& params) const = 0;

    //! Create a triangle mesh primitive
    virtual GraphicPtr _CreateTriMesh(TriMeshArgsCR args, DgnDbR dgndb) const = 0;

    //! Create an indexed polyline primitive
    virtual GraphicPtr _CreateIndexedPolylines(IndexedPolylineArgsCR args, DgnDbR dgndb) const = 0;

    //! Create a Graphic consisting of a list of Graphics
    virtual GraphicPtr _CreateGraphicList(bvector<GraphicPtr>&& primitives, DgnDbR dgndb) const = 0;

    //! Create a Graphic consisting of a list of Graphics, with optional transform, clip, and view flag overrides applied to the list
    virtual GraphicPtr _CreateBranch(GraphicBranch&& branch, DgnDbR dgndb, TransformCR transform, ClipVectorCP clips) const = 0;

    //! Return the maximum number of Features allowed within a Batch.
    virtual uint32_t _GetMaxFeaturesPerBatch() const = 0;

    //! Create a Graphic consisting of batched Features.
    virtual GraphicPtr _CreateBatch(GraphicR graphic, FeatureTable&& features, DRange3dCR range) const = 0;

    //! Find a previously-created Texture by key. Returns null if no such texture exists.
    virtual TexturePtr _FindTexture(TextureKeyCR key, DgnDbR db) const = 0;

    //! Get or create a Texture from a DgnTexture element. Note that there is a cache of textures stored on a DgnDb, so this may return a pointer to a previously-created texture.
    //! The default implementation uses _FindTexture() and calls _CreateTexture() if not found.
    //! @param[in] textureId the DgnElementId of the texture element
    //! @param[in] db the DgnDb for textureId
    DGNPLATFORM_EXPORT virtual TexturePtr _GetTexture(DgnTextureId textureId, DgnDbR db) const;

    //! Get or create a Texture from a GradientSymb. Note that there is a cache of textures stored on a DgnDb, so this may return a pointer to a previously-created texture.
    virtual TexturePtr _GetTexture(GradientSymbCR gradient, DgnDbR db) const = 0;

    //! Create a new Texture from an Image.
    virtual TexturePtr _CreateTexture(ImageCR image, DgnDbR db, Texture::CreateParams const& params=Texture::CreateParams()) const = 0;

    //! Create a new Texture from an ImageSource.
    virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::BottomUp bottomUp, DgnDbR db, Texture::CreateParams const& params=Texture::CreateParams()) const = 0;

    //! Create a Texture from a graphic.
    virtual TexturePtr _CreateGeometryTexture(GraphicCR graphic, DRange2dCR range, bool useGeometryColors, bool forAreaPattern) const = 0;

    //! Create a Light from Light::Parameters
    virtual LightPtr _CreateLight(Lighting::Parameters const&, DVec3dCP direction, DPoint3dCP location) const = 0;

    //! Perform some small unit of work (or do nothing) during an idle frame.
    //! An idle frame is classified one tick of the render loop during which no viewports are open and the render queue is empty.
    virtual void _Idle() { }

    //! Return true to cache tiles...
    virtual bool _DoCacheTiles() const { return true; }
};

END_BENTLEY_RENDER_NAMESPACE
