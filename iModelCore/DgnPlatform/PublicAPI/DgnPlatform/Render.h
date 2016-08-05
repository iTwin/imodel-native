/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Render.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnModel.h"
#include "DgnCategory.h"
#include "AreaPattern.h"
#include <Bentley/BeTimeUtilities.h>

#if defined (BENTLEYCONFIG_DISPLAY_WIN32)
    struct HICON__;
    struct HWND__;
    struct HDC__;
#endif

BEGIN_BENTLEY_RENDER_NAMESPACE

struct GraphicBuilderPtr;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
enum class RenderMode
    {
    Wireframe      = 0,
    CrossSection   = 1,
    Wiremesh       = 2,
    HiddenLine     = 3,
    SolidFill      = 4,
    ConstantShade  = 5,
    SmoothShade    = 6,
    Phong          = 7,
    RayTrace       = 8,
    Radiosity      = 10,
    Invalid        = 15,
    };

/*=================================================================================**//**
*  The flags that control view information.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ViewFlags
    {
private:
    RenderMode m_renderMode;

public:
    uint32_t m_constructions:1;    //!< Shows or hides construction class geometry.
    uint32_t m_text:1;             //!< Shows or hides text.
    uint32_t m_dimensions:1;       //!< Shows or hides dimensions.
    uint32_t m_patterns:1;         //!< Shows or hides pattern geometry.
    uint32_t m_weights:1;          //!< Controls whether non-zero line weights are used or display using weight 0.
    uint32_t m_styles:1;           //!< Controls whether custom line styles are used (e.g. control whether elements with custom line styles draw normally, or as solid lines).
    uint32_t m_transparency:1;     //!< Controls whether element transparency is used (e.g. control whether elements with transparency draw normally, or as opaque).
    uint32_t m_fill:1;             //!< Controls whether the fills on filled elements are displayed.
    uint32_t m_grid:1;             //!< Shows or hides the grid. The grid settings are a design file setting.
    uint32_t m_acsTriad:1;         //!< Shows or hides the ACS triad.
    uint32_t m_textures:1;         //!< Controls whether to display texture maps for material assignments. When off only material color is used for display.
    uint32_t m_materials:1;        //!< Controls whether materials are used (e.g. control whether geometry with materials draw normally, or as if it has no material).
    uint32_t m_sceneLights:1;      //!< Controls whether the custom scene lights or the default lighting scheme are used.
    uint32_t m_visibleEdges:1;     //!< Shows or hides visible edges in the shaded render mode. 
    uint32_t m_hiddenEdges:1;      //!< Shows or hides hidden edges in the shaded render mode. 
    uint32_t m_shadows:1;          //!< Shows or hides shadows. 
    uint32_t m_noClipVolume:1;     //!< Controls whether the clip volume is applied. 
    uint32_t m_ignoreLighting:1;   //!< Controls whether lights are used.
    uint32_t m_monochrome:1;       //!< use monochrome style
    uint32_t m_noGeometryMap:1;    //!< ignore geometry maps
    uint32_t m_edgeMask:2;         //!< 0=none, 1=generate mask, 2=use mask

    void SetRenderMode(RenderMode value) {m_renderMode = value;}
    RenderMode GetRenderMode() const {return m_renderMode;}

    DGNPLATFORM_EXPORT void InitDefaults();
    DGNPLATFORM_EXPORT void ToBaseJson(JsonValueR) const;
    DGNPLATFORM_EXPORT void FromBaseJson(JsonValueCR);
    DGNPLATFORM_EXPORT void To3dJson(JsonValueR) const;
    DGNPLATFORM_EXPORT void From3dJson(JsonValueCR);
    };

//=======================================================================================
//! A rendering task to be performed on the render thread.
// @bsiclass                                                    Keith.Bentley   07/15
//=======================================================================================
struct Task : RefCounted<NonCopyableClass>
{
    //! The rendering operation a task performs.
    enum class Operation
    {
        Initialize,
        ChangeScene,
        ChangeTerrain,
        ChangeRenderPlan,
        ChangeDynamics,
        ChangeDecorations,
        DrawProgressive,
        DrawFrame,
        Redraw,
        BeginHeal,
        FinishHeal,
        Heal,
        DefineGeometryTexture,
        FindNearestZ,
        ReadImage,
        DestroyTarget,
    };

    //! The outcome of the processing of a Task.
    enum class Outcome
    {
        Waiting,   //!< in queue, pending
        Abandoned, //!< replaced while pending
        Started,   //!< currently processing
        Aborted,   //!< aborted during processing
        Finished,  //!< successfully finished processing
    };

    friend struct Queue;

protected:
    Operation   m_operation;
    TargetPtr   m_target;
    Outcome     m_outcome = Outcome::Waiting;
    double      m_elapsedTime = 0.0;
    void Perform(StopWatch&);

public:
    //! Get the name of this task. For debugging only
    virtual Utf8CP _GetName() const = 0;

    //! Perform the rendering task.
    //! @return the Outcome of the processing of the Task.
    virtual Outcome _Process(StopWatch&) = 0;

    //! Determine whether this Task can replace a pending entry in the Queue.
    //! @param[in] other a pending task for the same Render::Target
    //! @return true if this Task should replace the other pending task.
    virtual bool _Replaces(Task& other) const {return m_operation == other.m_operation;}

    //! return true if this task changes the scene.
    virtual bool _DefinesScene() const = 0;

    //! called when this task is entered into the render queue
    virtual void _OnQueued() const {}

    Target* GetTarget() const {return m_target.get();} //!< Get the Target of this Task
    Operation GetOperation() const {return m_operation;} //!< Get the Operation of this Task.
    Outcome GetOutcome() const {return m_outcome;}   //!< The Outcome of the processing of this Task (or Waiting, if it has not been processed yet.)
    double GetElapsedTime() const {return m_elapsedTime;} //!< Elapsed time in seconds. Only valid if m_outcome is Finished or Aborted

    Task(Target* target, Operation operation) : m_target(target), m_operation(operation) {}
};

//=======================================================================================
// Base class for all tasks that change the scene
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct SceneTask : Task
{
    bool _DefinesScene() const override {return true;}
    bool _Replaces(Task& other) const override {return Render::Task::_Replaces(other) || !other._DefinesScene();}
    using Task::Task;
};

//=======================================================================================
// Base class for tasks that don't change the scene
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct NonSceneTask : Task
{
    bool _DefinesScene() const override {return false;}
    using Task::Task;
};

//=======================================================================================
//! The Render::Queue is accessed through DgnViewport::GetRenderQueue. It holds an array of Render::Tasks waiting
//! to to be processed on the render thread. Render::Tasks may be added to the Render::Queue only
//! on the main (work) thread, and may only be processed on the Render thread.
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct Queue
{
    friend DgnViewport;
private:
    BeConditionVariable m_cv;
    std::deque<TaskPtr> m_tasks;
    TaskPtr             m_currTask;

    void WaitForWork();
    void Process();
    THREAD_MAIN_DECL Main(void*);

public:
    //! Add a Render::Task to the render queue. The Task will replace any existing pending entries in the Queue
    //! for the same Render::Target for which task._CanReplace(existing) returns true.
    //! @param[in] task The Render::Task to add to the queue.
    //! @note This method may only be called from the main thread.
    DGNPLATFORM_EXPORT void AddTask(Task& task);

    //! Wait for all Tasks in the Queue to be processed.
    //! @note This method may only be called from the main thread and will wait indefinitely for the existing render tasks
    //! to complete.
    DGNPLATFORM_EXPORT void WaitForIdle();

    //! Add a task to the Queue and wait for it (and all previously queued Tasks) to complete.
    //! @param[in] task The Render::Task to add to the queue.
    //! @note This method may only be called from the main thread and will wait indefinitely for the existing render tasks
    //! to complete.
    void AddAndWait(Task& task) {AddTask(task); WaitForIdle();}

    //! @return true if the render queue is empty and no pending tasks are active.
    //! @note This method may only be called from the main thread
    DGNPLATFORM_EXPORT bool IsIdle() const;

    DGNPLATFORM_EXPORT bool HasPending(Task::Operation op) const;
    DGNPLATFORM_EXPORT bool HasActiveOrPending(Task::Operation op) const;
};


//=======================================================================================
//! An uncompressed image in Rgb (3 bytes per pixel) or Rgba (4 bytes per pixel) format suitable for rendering.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct Image
{
    enum class Format {Rgba=0, Rgb=2}; // must match qvision.h values
    enum class BottomUp : bool {No=0, Yes=1}; //!< whether the rows in the image should be flipped top-to-bottom
protected:
    uint32_t   m_width = 0;
    uint32_t   m_height = 0;
    Format     m_format = Format::Rgb;
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

    //! Create an image from a Jpeg.
    //! @param[in]      srcData      the Jpeg data
    //! @param[in]      srcLen       the number of bytes of Jpeg data
    //! @param[in]      targetFormat The format (Rgb or Rgba) for the new Image. If the source has an alpha channel and Rgb is requested, to alpha data is discarded.
    //! If the source does not have an alpha channel and Rgba is requested, all alpha values are set to 0xff.
    //! @param[in]      bottomUp     If Yes, the source image is flipped vertically (top-to-bottom) to create the image.
    //! @return The decompressed Image, or an invalid Image if decompression failed.
    DGNPLATFORM_EXPORT static Image FromJpeg(uint8_t const* srcData, uint32_t srcLen, Format targetFormat=Format::Rgba, BottomUp bottomUp=BottomUp::No);

    //! Create an image from a Png.
    //! @param[in]      srcData      the Png data
    //! @param[in]      srcLen       the number of bytes of Png data
    //! @param[in]      targetFormat The format (Rgb or Rgba) for the new Image. If the source has an alpha channel and Rgb is requested, to alpha data is discarded.
    //! If the source does not have an alpha channel and Rgba is requested, all alpha values are set to 0xff.
    //! @return The decompressed Image, or an invalid Image if decompression failed.
    DGNPLATFORM_EXPORT static Image FromPng(uint8_t const* srcData, uint32_t srcLen, Format targetFormat=Format::Rgba);

    int GetBytesPerPixel()const {return m_format == Format::Rgba ? 4 : 3;} //!< get the number of bytes per pixel
    void Invalidate() {m_width=m_height=0; ClearData();} //!< Clear the contents and invalidate this image.
    uint32_t GetWidth() const {return m_width;} //!< Get the width of this image in pixels
    uint32_t GetHeight() const {return m_height;} //!< Get the height of this image in pixels
    Format GetFormat() const {return m_format;} //!< Get the format (Rgb or Rgba) of this image
    void SetFormat(Format format) {m_format=format;} //!< Change the format of this image in pixels
    bool IsValid() {return 0!=m_width && 0!=m_height && 0!=m_image.GetSize();} //!< @return true if this image holds valid data
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
//! A Texture for rendering
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct Texture : RefCounted<NonCopyableClass>
{
    struct CreateParams
    {
        bool m_isTileSection = false;
        int m_pitch = 0;
        void SetIsTileSection() {m_isTileSection=true;}
        void SetPitch(int val) {m_pitch=val;}
    };
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct Material : RefCounted<NonCopyableClass>
{
    enum class MapMode : int
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

    struct CreateParams
    {
        struct MatColor
        {
            bool m_valid = false;
            ColorDef m_value;
            MatColor(){}
            MatColor(ColorDef val) {m_valid=true; m_value=val;}
            bool IsValid() const {return m_valid;}
        };

        MatColor m_diffuseColor;
        MatColor m_specularColor;
        MatColor m_emissiveColor;
        double m_diffuse = 0.5;
        double m_ambient = 0.5; 
        double m_specularExponent = 0.0;
        double m_reflect = 0.0;
        double m_transparency = 0.0;
        double m_specular = 0.05;
        double m_refract = 1.0;
        bool m_shadows = true;

        void SetDiffuseColor(ColorDef val) {m_diffuseColor = val;} //<! Set the surface color for fill or diffuse illumination
        void SetSpecularColor(ColorDef val) {m_specularColor = val;} //<! Set the surface color for specular illumination
        void SetEmissiveColor(ColorDef val) {m_emissiveColor = val;} //<!  Set the surface emissive color
        void SetDiffuse(double val) {m_diffuse = val;} //<! Set surface diffuse reflectivity
        void SetAmbient(double val) {m_ambient = val;} //<! Set surface ambient reflectivity
        void SetSpecularExponent(double val) {m_specularExponent = val;} //<! Set surface shininess (range 0 to 128)
        void SetReflect(double val) {m_reflect = val;} //<! Set surface environmental reflectivity
        void SetTransparency(double val) {m_transparency = val;} //<! Set surface transparency
        void SetSpecular(double val) {m_specular = val;} //<! Set surface specular reflectivity
        void SetRefract(double val) {m_refract = val;} //<! Set index of refraction
        void SetShadows(bool val) {m_shadows = val;} //! If false, do not cast shadows
    };

    struct Trans2x3 
    {
        double m_val[2][3];
        Trans2x3() {}
        Trans2x3(double t00, double t01, double t02, double t10, double t11, double t12) {m_val[0][0]=t00; m_val[0][1]=t01; m_val[0][2]=t02; m_val[1][0]=t10; m_val[1][1]=t11; m_val[1][2]=t12;}
    };
    struct TextureMapParams
    {
        double m_textureWeight = 1.0;
        Trans2x3* m_textureMat2x3 = nullptr;
        MapMode m_mapMode = MapMode::Parametric;
        bool m_worldMapping = false;
        DPoint3dCP m_basisX = nullptr;
        DPoint3dCP m_basisY = nullptr;
        DPoint3dCP m_basisZ = nullptr;
        DPoint3dCP m_basisOrg = nullptr;
        DPoint3dCP m_basisScale = nullptr;
        void SetMapMode(MapMode val) {m_mapMode=val;}
        void SetWeight(double val) {m_textureWeight = val;} //<! Set weight for combining diffuse image and color
        void SetTransform(Trans2x3* val) {m_textureMat2x3 = val;} //<! Set Texture 2x3 transform
        void SetWorldMapping(bool val) {m_worldMapping = val;} //! if true world mapping, false for surface
        void SetBasis(DPoint3dCP x, DPoint3dCP y, DPoint3dCP z, DPoint3dCP org, DPoint3dCP scale) {m_basisX = x; m_basisY = y; m_basisZ = z; m_basisOrg = org; m_basisScale = scale;}
    };

protected:
    bvector<TextureCPtr> m_mappedTextures;
    void AddMappedTexture(TextureCR texture) {m_mappedTextures.push_back(&texture);}

public:
    //! Map a texture to this material
    virtual void _MapTexture(Texture const& texture, TextureMapParams const& params = TextureMapParams()) = 0;
};

//=======================================================================================
//! Line style parameters
//! @private
//=======================================================================================
struct LineStyleParams
{
    uint32_t    modifiers;      /* see STYLEMOD_... above              */
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
    DVec3d      m_startTangent;
    DVec3d      m_endTangent;
    RotMatrix   m_planeByRows;
    TexturePtr  m_texture;
    bool        m_useLinePixels;
    uint32_t    m_linePixels;


public:
    DGNPLATFORM_EXPORT LineStyleSymb();
    DGNPLATFORM_EXPORT void Init(DgnStyleId styleId, LineStyleParamsCR styleParams, DVec3dCP startTangent, DVec3dCP endTangent, ViewContextR context, GeometryParamsR);

    void Clear() {m_lStyle = nullptr; m_texture = nullptr;}
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
    double GetTotalLength() const {return m_totalLength;}
    DVec3dCP GetStartTangent() const {return &m_startTangent;}
    DVec3dCP GetEndTangent() const{return &m_endTangent;}
    Texture* GetTexture() const {return m_texture.get();}

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
    DVec3d              m_startTangent;
    DVec3d              m_endTangent;

    DGNPLATFORM_EXPORT LineStyleInfo(DgnStyleId styleId, LineStyleParamsCP params);

public:
    DGNPLATFORM_EXPORT void CopyFrom(LineStyleInfoCR);

    //! Create an instance of a LineStyleInfo.
    DGNPLATFORM_EXPORT static LineStyleInfoPtr Create(DgnStyleId styleId, LineStyleParamsCP params);

    //! Compare two LineStyleInfo.
    DGNPLATFORM_EXPORT bool operator==(LineStyleInfoCR rhs) const;

    DgnStyleId GetStyleId() const {return m_styleId;}
    LineStyleParamsCP GetStyleParams() const {return 0 != m_styleParams.modifiers ? &m_styleParams : nullptr;}
    LineStyleSymbCR GetLineStyleSymb() const {return m_lStyleSymb;}
    LineStyleSymbR GetLineStyleSymbR() {return m_lStyleSymb;}
    DVec3dCR GetStartTangent() const {return m_startTangent;}
    DVec3dCR GetEndTangent() const {return m_endTangent;}
    void SetStartTangent(DVec3dCR startTangent) {m_startTangent = startTangent;}
    void SetEndTangent(DVec3dCR endTangent) {m_endTangent = endTangent;}

    DGNPLATFORM_EXPORT void Cook(ViewContextR, GeometryParamsR);
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

enum class DgnGeometryClass
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
        None         = 0,
        Invert       = (1 << 0),
        Outline      = (1 << 1),
        AlwaysFilled = (1 << 2),
    };

    enum class Mode : Byte
    {
        None          = 0,
        Linear        = 1,
        Curved        = 2,
        Cylindrical   = 3,
        Spherical     = 4,
        Hemispherical = 5,
    };

protected:
    Mode m_mode = Mode::None;
    Flags m_flags = Flags::None;
    uint32_t m_nKeys = 0;
    double m_angle = 0.0;
    double m_tint = 0.0;
    double m_shift = 0.0;
    ColorDef m_colors[MAX_GRADIENT_KEYS];
    double   m_values[MAX_GRADIENT_KEYS];

public:
    GradientSymb() {}
    
    DGNPLATFORM_EXPORT void CopyFrom(GradientSymbCR);

    //! Create an instance of a GradientSymb.
    static GradientSymbPtr Create() {return new GradientSymb();}

    //! Compare two GradientSymb.
    DGNPLATFORM_EXPORT bool operator==(GradientSymbCR rhs) const;

    uint32_t GetNKeys() const {return m_nKeys;}
    Mode GetMode() const {return m_mode;}
    Flags GetFlags() const {return m_flags;}
    double GetShift() const {return m_shift;}
    double GetTint() const {return m_tint;}
    double GetAngle() const {return m_angle;}
    void GetKey(ColorDef& color, double& value, int index) const {color = m_colors[index]; value = m_values[index];}
    void SetMode(Mode mode) {m_mode = mode;}
    void SetFlags(Flags flags) {m_flags = flags;}
    void SetAngle(double angle) {m_angle = angle;}
    void SetTint(double tint) {m_tint = tint;}
    void SetShift(double shift) {m_shift = shift;}
    DGNPLATFORM_EXPORT void SetKeys(uint32_t nKeys, ColorDef const* colors, double const* values);
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
        bool m_fill:1;   // If not set, fill is an opaque fill that matches sub-category appearance color...
        bool m_bgFill:1; // When set, fill is an opaque fill that matches current view background color...
        AppearanceOverrides() {memset(this, 0, sizeof(*this));}
        };

    AppearanceOverrides m_appearanceOverrides;          //!< flags for parameters that override SubCategory::Appearance.
    bool                m_resolved;                     //!< whether Resolve has established SubCategory::Appearance/effective values.
    DgnCategoryId       m_categoryId;                   //!< the Category Id on which the geometry is drawn.
    DgnSubCategoryId    m_subCategoryId;                //!< the SubCategory Id that controls the appearance of subsequent geometry.
    DgnMaterialId       m_materialId;                   //!< render material ID.
    int32_t             m_elmPriority;                  //!< display priority (applies to 2d only)
    int32_t             m_netPriority;                  //!< net display priority for element/category (applies to 2d only)
    uint32_t            m_weight;
    ColorDef            m_lineColor;
    ColorDef            m_fillColor;                    //!< fill color (applicable only if filled)
    FillDisplay         m_fillDisplay;                  //!< whether or not the element should be displayed filled
    double              m_elmTransparency;              //!< transparency, 1.0 == completely transparent.
    double              m_netElmTransparency;           //!< net transparency for element/category.
    double              m_fillTransparency;             //!< fill transparency, 1.0 == completely transparent.
    double              m_netFillTransparency;          //!< net transparency for fill/category.
    DgnGeometryClass    m_geometryClass;                //!< geometry class
    LineStyleInfoPtr    m_styleInfo;                    //!< line style id plus modifiers.
    GradientSymbPtr     m_gradient;                     //!< gradient fill settings.
    PatternParamsPtr    m_pattern;                      //!< area pattern settings.

public:
    DGNPLATFORM_EXPORT GeometryParams();
    DGNPLATFORM_EXPORT GeometryParams(GeometryParamsCR rhs);
    DGNPLATFORM_EXPORT void ResetAppearance(); //!< Like Init, but saves and restores category and sub-category around the call to Init. This is particularly useful when a single element draws objects of different symbology, but its draw code does not have easy access to reset the category.
    DGNPLATFORM_EXPORT void Resolve(DgnDbR, DgnViewportP vp=nullptr); // Resolve effective values using the supplied DgnDb and optional DgnViewport (for view bg fill and view sub-category overrides)...
    DGNPLATFORM_EXPORT void Resolve(ViewContextR); // Resolve effective values using the supplied ViewContext.

    void SetCategoryId(DgnCategoryId categoryId) {m_categoryId = categoryId; m_subCategoryId = DgnCategory::GetDefaultSubCategoryId(categoryId); memset(&m_appearanceOverrides, 0, sizeof(m_appearanceOverrides)); m_resolved = false;} // Setting the Category Id also sets the SubCategory to the default.
    void SetSubCategoryId(DgnSubCategoryId subCategoryId) {m_subCategoryId = subCategoryId; memset(&m_appearanceOverrides, 0, sizeof(m_appearanceOverrides)); m_resolved = false;}
    void SetWeight(uint32_t weight) {m_appearanceOverrides.m_weight = true; m_weight = weight;}
    void SetLineStyle(LineStyleInfoP styleInfo) {m_appearanceOverrides.m_style = true; m_styleInfo = styleInfo;}
    void SetLineColor(ColorDef color) {m_appearanceOverrides.m_color = true; m_lineColor = color;}
    void SetFillDisplay(FillDisplay display) {m_fillDisplay = display;}
    void SetFillColor(ColorDef color) {m_appearanceOverrides.m_fill = true; m_appearanceOverrides.m_bgFill = false; m_fillColor = color;}
    void SetFillColorToViewBackground() {m_appearanceOverrides.m_fill = false; m_appearanceOverrides.m_bgFill = true;} // FillDisplay::Blanking creates an opaque view background fill...
    void SetGradient(GradientSymbP gradient) {m_gradient = gradient;}
    void SetGeometryClass(DgnGeometryClass geomClass) {m_geometryClass = geomClass;}
    void SetTransparency(double transparency) {m_elmTransparency = m_netElmTransparency = m_fillTransparency = m_netFillTransparency = transparency; m_resolved = false;} // NOTE: Sets BOTH element and fill transparency...
    void SetFillTransparency(double transparency) {m_fillTransparency = m_netFillTransparency = transparency; m_resolved = false;}
    void SetDisplayPriority(int32_t priority) {m_elmPriority = m_netPriority = priority; m_resolved = false;} // Set display priority (2d only).
    void SetMaterialId(DgnMaterialId materialId) {m_appearanceOverrides.m_material = true; m_materialId = materialId;}
    void SetPatternParams(PatternParamsP patternParams) {m_pattern = patternParams;}

    //! @cond DONTINCLUDEINDOC
    double GetNetTransparency() const {BeAssert(m_resolved); return m_netElmTransparency;}
    double GetNetFillTransparency() const {BeAssert(m_resolved); return m_netFillTransparency;}

    int32_t GetNetDisplayPriority() const {BeAssert(m_resolved); return m_netPriority;} // Get net display priority (2d only).
    int32_t GetNetDisplayPriority(ViewContextR context) {Resolve(context); return m_netPriority;} // Resolve and return net display priority (2d only).
    void SetNetDisplayPriority(int32_t priority) {m_netPriority = priority;} // RASTER USE ONLY!!!

    bool IsLineColorFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_color;}
    bool IsWeightFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_weight;}
    bool IsLineStyleFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_style;}
    bool IsMaterialFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_material;}
    bool IsFillColorFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_fill && !m_appearanceOverrides.m_bgFill;}
    bool IsFillColorFromViewBackground() const {return m_appearanceOverrides.m_bgFill;}
    //! @endcond

    //! Compare two GeometryParams.
    DGNPLATFORM_EXPORT bool operator==(GeometryParamsCR rhs) const;

    //! copy operator
    DGNPLATFORM_EXPORT GeometryParamsR operator=(GeometryParamsCR rhs);

    //! Get element category
    DgnCategoryId GetCategoryId() const {return m_categoryId;}

    //! Get element sub-category
    DgnSubCategoryId GetSubCategoryId() const {return m_subCategoryId;}

    //! Get element color
    ColorDef GetLineColor() const {BeAssert(m_appearanceOverrides.m_color || m_resolved); return m_lineColor;}

    //! Get element fill color
    ColorDef GetFillColor() const {BeAssert(m_appearanceOverrides.m_fill || m_resolved); return m_fillColor;}

    //! Get fill display setting
    FillDisplay GetFillDisplay() const {return m_fillDisplay;}

    //! Get gradient fill information. Valid when FillDisplay::Never != GetFillDisplay() and not nullptr.
    GradientSymbCP GetGradient() const {return m_gradient.get();}

    //! Get the area pattern params.
    PatternParamsCP GetPatternParams() const {return m_pattern.get();}

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
    DgnMaterialId GetMaterialId() const {BeAssert(m_appearanceOverrides.m_material || m_resolved); return m_materialId;}

    //! Get display priority (2d only).
    int32_t GetDisplayPriority() const {return m_elmPriority;}
};

//=======================================================================================
//! The "cooked" material and symbology for a Render::Graphic. This determines the appearance
//! (e.g. texture, color, width, linestyle, etc.) used to draw Geometry.
//=======================================================================================
struct GraphicParams
{
private:
    bool                m_isFilled;
    bool                m_isBlankingRegion;
    uint32_t            m_linePixels;
    uint32_t            m_rasterWidth;
    ColorDef            m_lineColor;
    ColorDef            m_fillColor;
    double              m_trueWidthStart;
    double              m_trueWidthEnd;
    TexturePtr          m_lineTexture;
    MaterialPtr         m_material;
    GradientSymbPtr     m_gradient;
    PatternParamsPtr    m_patternParams;

public:

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
        Invisible = 0x00000001, // nearly invisible
        };

    void Cook(GeometryParamsCR, ViewContextR);

    GraphicParams() {Init();}
    DGNPLATFORM_EXPORT explicit GraphicParams(GraphicParamsCR rhs);
    DGNPLATFORM_EXPORT void Init();

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

    //! Get the area pattern params.
    PatternParamsCP GetPatternParams() const {return m_patternParams.get();}
    //@}

    //! @name Set Methods
    //@{

    //! Set the current line color for this GraphicParams.
    //! @param[in] lineColor the new TBGR line color for this GraphicParams.
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

    //! Set area patterning parameters.
    void SetPatternParams(PatternParamsP patternParams) {m_patternParams = patternParams;}
    //@}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct OvrGraphicParams
{
    enum Flags : uint32_t//! flags to indicate the parts of a GraphicParams that are to be overridden
    {
        FLAGS_None                   = (0),      //!< no overrides
        FLAGS_Color                  = (1<<0),   //!< override outline color
        FLAGS_ColorTransparency      = (1<<1),   //!< override outline color transparency
        FLAGS_FillColor              = (1<<2) | (0x80000000), //!< override fill color, override blanking fill with bg color
        FLAGS_FillColorTransparency  = (1<<3),   //!< override fill color transparency
        FLAGS_RastWidth              = (1<<4),   //!< override raster width
        FLAGS_Style                  = (1<<5),   //!< override style
        FLAGS_TrueWidth              = (1<<6),   //!< override true width
        FLAGS_ExtSymb                = (1<<7),   //!< override extended symbology
        FLAGS_RenderMaterial         = (1<<8),   //!< override render material
    };

private:
    uint32_t        m_flags;
    GraphicParams   m_matSymb;

public:
    OvrGraphicParams() : m_flags(FLAGS_None) {}
    GraphicParamsCR GetMatSymb() const {return m_matSymb;}
    GraphicParamsR GetMatSymbR () {return m_matSymb;}

public:
    //! Compare two OvrGraphicParams.
    bool operator==(OvrGraphicParamsCR rhs) const {if (this == &rhs) return true; if (rhs.m_flags != m_flags) return false; return rhs.m_matSymb == m_matSymb;}

    uint32_t GetFlags() const{return m_flags;}
    ColorDef GetLineColor() const {return m_matSymb.GetLineColor();}
    ColorDef GetFillColor() const {return m_matSymb.GetFillColor();}
    uint32_t GetWidth() const {return m_matSymb.GetWidth();}
    MaterialPtr GetMaterial() const {return m_matSymb.GetMaterial();}
    PatternParamsCP GetPatternParams() const {return m_matSymb.GetPatternParams();}

    void Clear() {SetFlags(FLAGS_None); m_matSymb.Init();};
    void SetFlags(uint32_t flags) {m_flags = flags;}
    void SetLineColor(ColorDef color) {m_matSymb.SetLineColor(color); m_flags |=  FLAGS_Color;}
    void SetFillColor(ColorDef color) {m_matSymb.SetFillColor(color); m_flags |= FLAGS_FillColor;}
    void SetLineTransparency(Byte trans) {m_matSymb.SetLineTransparency(trans); m_flags |= FLAGS_ColorTransparency;}
    void SetFillTransparency(Byte trans) {m_matSymb.SetFillTransparency(trans); m_flags |= FLAGS_FillColorTransparency;}
    void SetWidth(uint32_t width) {m_matSymb.SetWidth(width); m_flags |= FLAGS_RastWidth;}
    void SetLinePixels(GraphicParams::LinePixels pixels) {m_matSymb.SetLinePixels(pixels); m_flags |= FLAGS_Style;}
    void SetMaterial(Material* material) {m_matSymb.SetMaterial(material); m_flags |= FLAGS_RenderMaterial;}
    void SetPatternParams(PatternParamsP patternParams) {m_matSymb.SetPatternParams(patternParams);}
    void SetLineTexture(TextureP texture) {m_matSymb.SetLineTexture(texture); m_flags |= FLAGS_Style;}
    void SetTrueWidthStart(double width) {m_matSymb.SetTrueWidthStart(width); m_flags |= FLAGS_TrueWidth;}
    void SetTrueWidthEnd(double width) {m_matSymb.SetTrueWidthEnd(width); m_flags |= FLAGS_TrueWidth;}
};

//=======================================================================================
//! A renderer-specific object which can be placed into a display list.
// @bsistruct                                                   Paul.Connelly   05/16
//=======================================================================================
struct Graphic : RefCounted<NonCopyableClass>
{
    friend struct ViewContext;
    struct CreateParams
    {
        DgnViewportCP m_vp;
Transform     m_placement;
        double        m_pixelSize;
        CreateParams(DgnViewportCP vp=nullptr, TransformCR placement=Transform::FromIdentity(), double pixelSize=0.0) : m_vp(vp), m_pixelSize(pixelSize), m_placement(placement) {}
    };

protected:
    DgnViewportCP m_vp; //! Viewport this Graphic is valid for (Graphic is valid for any viewport if nullptr)
    double        m_pixelSize; //! Pixel size to use for stroke
    double        m_minSize; //! Minimum pixel size this Graphic is valid for (Graphic is valid for all sizes if min and max are both 0.0)
    double        m_maxSize; //! Maximum pixel size this Graphic is valid for (Graphic is valid for all sizes if min and max are both 0.0)
    Transform     m_localToWorldTransform;

    virtual ~Graphic() {}
    virtual bool _IsForDisplay() const {return false;}
    virtual StatusInt _EnsureClosed() = 0;
public:
    explicit Graphic(CreateParams const& params=CreateParams()) : m_vp(params.m_vp), m_pixelSize(params.m_pixelSize), m_minSize(0.0), m_maxSize(0.0) {m_localToWorldTransform = params.m_placement;}

    bool IsValidFor(DgnViewportCR vp, double metersPerPixel) const
        {
        if (nullptr != m_vp && m_vp != &vp)
            return false;

        if (0.0 == metersPerPixel || (0.0 == m_minSize && 0.0 == m_maxSize))
            return true;

        return (metersPerPixel >= m_minSize && metersPerPixel <= m_maxSize);
        }

    bool IsSpecificToViewport(DgnViewportCR vp) const {return nullptr != m_vp && m_vp == &vp;}
    DgnViewportCP GetViewport() const {return m_vp;}

    //! Get current local to world transform (ex. GeometrySource placement transform).
    TransformCR GetLocalToWorldTransform() const {return m_localToWorldTransform;}

    double GetPixelSize() const {return m_pixelSize;}
    void GetPixelSizeRange(double& min, double& max) const {min = m_minSize; max = m_maxSize;}
    void SetPixelSizeRange(double min, double max) {m_minSize = min; m_maxSize = max;}
    void UpdatePixelSizeRange(double newMin, double newMax) //! Update min/max only if more restrictive than current value.
        {
        m_minSize = (0.0 == m_minSize ? newMin : DoubleOps::Max(m_minSize, newMin));
        m_maxSize = (0.0 == m_maxSize ? newMax : DoubleOps::Min(m_maxSize, newMax));
        }

    //! Return whether this decoration will be drawn to a viewport as opposed to being collected for some other purpose (ex. geometry export).
    bool IsForDisplay() const {return _IsForDisplay();}
    StatusInt EnsureClosed() {return _EnsureClosed();} //!< Called when this Graphic is added to a display list, to ensure it is fully constructed and ready for display
};

//=======================================================================================
//! Interface adopted by an object which can build a Graphic from the Graphic primitives.
// @bsiclass
//=======================================================================================
struct IGraphicBuilder
{
    //=======================================================================================
    //! Information needed to draw a triangle mesh
    // @bsiclass                                                    Keith.Bentley   06/16
    //=======================================================================================
    struct TriMeshArgs
    {
        int32_t m_numIndices = 0;
        int32_t const* m_vertIndex = nullptr;
        int32_t m_numPoints = 0;
        FPoint3d const* m_points= nullptr;
        FPoint3d const* m_normals= nullptr;
        FPoint2d const* m_textureUV= nullptr;
        TexturePtr m_texture;
        int32_t m_flags = 0; // don't generate normals
        DGNPLATFORM_EXPORT PolyfaceHeaderPtr ToPolyface() const;
    };

protected:
    friend struct GraphicBuilder;

    virtual bool _IsOpen() const = 0;
    virtual StatusInt _Close() = 0;
    virtual GeometryStreamEntryIdCP _GetGeometryStreamEntryId() const {return nullptr;}
    virtual void _SetGeometryStreamEntryId(GeometryStreamEntryIdCP) {}
    virtual void _ActivateGraphicParams(GraphicParamsCR graphicParams, GeometryParamsCP geomParams) = 0;
    virtual void _AddLineString(int numPoints, DPoint3dCP points) = 0;
    virtual void _AddLineString2d(int numPoints, DPoint2dCP points, double zDepth) = 0;
    virtual void _AddPointString(int numPoints, DPoint3dCP points) = 0;
    virtual void _AddPointString2d(int numPoints, DPoint2dCP points, double zDepth) = 0;
    virtual void _AddShape(int numPoints, DPoint3dCP points, bool filled) = 0;
    virtual void _AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth) = 0;
    virtual void _AddTriStrip(int numPoints, DPoint3dCP points, int32_t usageFlags) = 0;
    virtual void _AddTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth) = 0;
    virtual void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) = 0;
    virtual void _AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth) = 0;
    virtual void _AddBSplineCurve(MSBsplineCurveCR curve, bool filled) = 0;
    virtual void _AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) = 0;
    virtual void _AddCurveVector(CurveVectorCR curves, bool isFilled) = 0;
    virtual void _AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) = 0;
    virtual void _AddSolidPrimitive(ISolidPrimitiveCR primitive) = 0;
    virtual void _AddBSplineSurface(MSBsplineSurfaceCR surface) = 0;
    virtual void _AddPolyface(PolyfaceQueryCR meshData, bool filled = false) = 0;
    virtual void _AddTriMesh(TriMeshArgs const& args) = 0;
    virtual void _AddBody(ISolidKernelEntityCR) = 0;
    virtual void _AddTextString(TextStringCR text) = 0;
    virtual void _AddTextString2d(TextStringCR text, double zDepth) = 0;
    virtual void _AddTile(TextureCR tile, DPoint3dCP corners) = 0;
    virtual void _AddDgnOle(DgnOleDraw*) = 0;
    virtual void _AddPointCloud(int32_t numPoints, DPoint3dCR origin, FPoint3d const* points, ByteCP colors) = 0;
    virtual void _AddSubGraphic(GraphicR, TransformCR, GraphicParamsCR) = 0;
    virtual GraphicBuilderPtr _CreateSubGraphic(TransformCR) const = 0;
};

//=======================================================================================
//! Exposes methods for constructing a Graphic from graphic primitives.
// @bsistruct                                                   Paul.Connelly   05/16
//=======================================================================================
struct GraphicBuilder
{
    typedef IGraphicBuilder::TriMeshArgs TriMeshArgs;
private:
    friend struct GraphicBuilderPtr;

    GraphicPtr          m_graphic;
    IGraphicBuilderP    m_builder;

    GraphicBuilder() : m_builder(nullptr) {}
    GraphicBuilder(GraphicR graphic, IGraphicBuilderR builder) : m_graphic(&graphic), m_builder(&builder) {}
    template<typename T> GraphicBuilder(T* t) : m_graphic(t), m_builder(t) {}

    bool IsValid() const {return m_graphic.IsValid();}
public:
    GraphicBuilder(GraphicBuilderP p) : m_graphic(nullptr != p ? p->m_graphic : nullptr), m_builder(nullptr != p ? p->m_builder : nullptr) {}
    template<typename T> GraphicBuilder(T& t) : m_graphic(&t), m_builder(&t) {}

    DGNPLATFORM_EXPORT GraphicBuilderPtr CreateSubGraphic(TransformCR subToGraphic) const; // NOTE: subToGraphic is provided to allow stroking in world coords...

    operator Graphic&() {BeAssert(m_graphic.IsValid()); return *m_graphic;}
    DgnViewportCP GetViewport() const {return m_graphic->GetViewport();}
    TransformCR GetLocalToWorldTransform() const {return m_graphic->GetLocalToWorldTransform();}
    double GetPixelSize() const {return m_graphic->GetPixelSize();}
    void GetPixelSizeRange(double& min, double& max) const {m_graphic->GetPixelSizeRange(min, max);}
    void SetPixelSizeRange(double min, double max) {m_graphic->SetPixelSizeRange(min, max);}
    void UpdatePixelSizeRange(double newMin, double newMax) {m_graphic->UpdatePixelSizeRange(newMin, newMax);}
    bool IsForDisplay() const {return m_graphic->IsForDisplay();}

    StatusInt Close() {return IsOpen() ? m_builder->_Close() : SUCCESS;}
    bool IsOpen() const {return m_builder->_IsOpen();}

    //! Get the current GeometryStreamEntryId.
    //! @return A GeometryStream entry identifier for the graphics that are currently being drawn.
    GeometryStreamEntryIdCP GetGeometryStreamEntryId() const {return m_builder->_GetGeometryStreamEntryId();}

    //! Set the current GeometryStreamEntryId.
    void SetGeometryStreamEntryId(GeometryStreamEntryIdCP entry) {m_builder->_SetGeometryStreamEntryId(entry);}

    //! Set an GraphicParams to be the "active" GraphicParams for this Render::Graphic.
    //! @param[in]          graphicParams   The new active GraphicParams. All geometry drawn via calls to this Render::Graphic will
    //! @param[in]          geomParams      The source GeometryParams if graphicParams was created by cooking geomParams, nullptr otherwise.
    void ActivateGraphicParams(GraphicParamsCR graphicParams, GeometryParamsCP geomParams=nullptr) {m_builder->_ActivateGraphicParams(graphicParams, geomParams);}

    //! Draw a 3D line string.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the line string.
    void AddLineString(int numPoints, DPoint3dCP points) {m_builder->_AddLineString(numPoints, points);}

    //! Draw a 2D line string.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the line string.
    //! @param[in]          zDepth      Z depth value in local coordinates.
    void AddLineString2d(int numPoints, DPoint2dCP points, double zDepth) {m_builder->_AddLineString2d(numPoints, points, zDepth);}

    //! Draw a 3D point string. A point string is displayed as a series of points, one at each vertex in the array, with no vectors connecting the vertices.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the point string.
    void AddPointString(int numPoints, DPoint3dCP points) {m_builder->_AddPointString(numPoints, points);}

    //! Draw a 2D point string. A point string is displayed as a series of points, one at each vertex in the array, with no vectors connecting the vertices.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the point string.
    //! @param[in]          zDepth      Z depth value.
    void AddPointString2d(int numPoints, DPoint2dCP points, double zDepth) {m_builder->_AddPointString2d(numPoints, points, zDepth);}

    //! Draw a closed 3D shape.
    //! @param[in]          numPoints   Number of vertices in \c points array. If the last vertex in the array is not the same as the first vertex, an
    //!                                     additional vertex will be added to close the shape.
    //! @param[in]          points      Array of vertices of the shape.
    //! @param[in]          filled      If true, the shape will be drawn filled.
    void AddShape(int numPoints, DPoint3dCP points, bool filled) {m_builder->_AddShape(numPoints, points, filled);}

    //! Draw a 2D shape.
    //! @param[in]          numPoints   Number of vertices in \c points array. If the last vertex in the array is not the same as the first vertex, an
    //!                                     additional vertex will be added to close the shape.
    //! @param[in]          points      Array of vertices of the shape.
    //! @param[in]          zDepth      Z depth value.
    //! @param[in]          filled      If true, the shape will be drawn filled.
    void AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth) {m_builder->_AddShape2d(numPoints, points, filled, zDepth);}

    //! Draw a 3D elliptical arc or ellipse.
    //! @param[in]          ellipse     arc data.
    //! @param[in]          isEllipse   If true, and if full sweep, then draw as an ellipse instead of an arc.
    //! @param[in]          filled      If true, and isEllipse is also true, then draw ellipse filled.
    void AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) {m_builder->_AddArc(ellipse, isEllipse, filled);}

    //! Draw a 2D elliptical arc or ellipse.
    //! @param[in]          ellipse     arc data.
    //! @param[in]          isEllipse   If true, and if full sweep, then draw as an ellipse instead of an arc.
    //! @param[in]          filled      If true, and isEllipse is also true, then draw ellipse filled.
    //! @param[in]          zDepth      Z depth value
    void AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth) {m_builder->_AddArc2d(ellipse, isEllipse, filled, zDepth);}

    //! Draw a BSpline curve.
    void AddBSplineCurve(MSBsplineCurveCR curve, bool filled) {m_builder->_AddBSplineCurve(curve, filled);}

    //! Draw a BSpline curve as 2d geometry with display priority.
    //! @note Only necessary for non-ICachedDraw calls to support non-zero display priority.
    void AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) {m_builder->_AddBSplineCurve2d(curve, filled, zDepth);}

    //! Draw a curve vector.
    void AddCurveVector(CurveVectorCR curves, bool isFilled) {m_builder->_AddCurveVector(curves, isFilled);}

    //! Draw a curve vector as 2d geometry with display priority.
    //! @note Only necessary for non-ICachedDraw calls to support non-zero display priority.
    void AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) {m_builder->_AddCurveVector2d(curves, isFilled, zDepth);}

    //! Draw a light-weight surface or solid primitive.
    //! @remarks Solid primitives can be capped or uncapped, they include cones, torus, box, spheres, and sweeps.
    void AddSolidPrimitive(ISolidPrimitiveCR primitive) {m_builder->_AddSolidPrimitive(primitive);}

    //! Draw a BSpline surface.
    void AddBSplineSurface(MSBsplineSurfaceCR surface) {m_builder->_AddBSplineSurface(surface);}

    //! @remarks Wireframe fill display supported for non-illuminated meshes.
    void AddPolyface(PolyfaceQueryCR meshData, bool filled = false) {m_builder->_AddPolyface(meshData, filled);}

    void AddTriMesh(TriMeshArgs const& args) {m_builder->_AddTriMesh(args);}

    //! Draw a 3D point cloud.
    void AddPointCloud(int32_t numPoints, DPoint3dCR origin, FPoint3d const* points, ByteCP colors) {m_builder->_AddPointCloud(numPoints, origin, points, colors);}

    //! Draw a BRep surface/solid entity from the solids kernel.
    void AddBody(ISolidKernelEntityCR entity) {m_builder->_AddBody(entity);}

    //! Draw a series of Glyphs.
    //! @param[in]          text        Text drawing parameters
    void AddTextString(TextStringCR text) {m_builder->_AddTextString(text);}

    //! Draw a series of Glyphs with display priority.
    //! @param[in] text   Text drawing parameters
    //! @param[in] zDepth Priority value in 2d
    void AddTextString2d(TextStringCR text, double zDepth) {m_builder->_AddTextString2d(text, zDepth);}

    //! Draw a filled triangle strip from 3D points.
    //! @param[in] numPoints   Number of vertices in \c points array.
    //! @param[in] points      Array of vertices.
    //! @param[in] usageFlags  0 or 1 if tri-strip represents a thickened line.
    void AddTriStrip(int numPoints, DPoint3dCP points, int32_t usageFlags) {m_builder->_AddTriStrip(numPoints, points, usageFlags);}

    //! Draw a filled triangle strip from 2D points.
    //! @param[in] numPoints   Number of vertices in \c points array.
    //! @param[in] points      Array of vertices.
    //! @param[in] zDepth      Z depth value.
    //! @param[in] usageFlags  0 or 1 if tri-strip represents a thickened line.
    void AddTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth) {m_builder->_AddTriStrip2d(numPoints, points, usageFlags, zDepth);}

    //! @private
    void AddTile(TextureCR tile, DPoint3dCP corners) {m_builder->_AddTile(tile, corners);}

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
    void AddDgnOle(DgnOleDraw* ole) {m_builder->_AddDgnOle(ole);}

    void AddSubGraphic(GraphicR graphic, TransformCR subToGraphic, GraphicParamsCR params) {m_builder->_AddSubGraphic(graphic, subToGraphic, params);}

    //! Set symbology for decorations that are only used for display purposes. Pickable decorations require a category, must initialize
    //! a GeometryParams and cook it into a GraphicParams to have a locatable decoration.
    void SetSymbology(ColorDef lineColor, ColorDef fillColor, int lineWidth, GraphicParams::LinePixels linePixels=GraphicParams::LinePixels::Solid)
        {
        GraphicParams graphicParams;
        graphicParams.SetLineColor(lineColor);
        graphicParams.SetFillColor(fillColor);
        graphicParams.SetWidth(lineWidth);
        graphicParams.SetLinePixels(linePixels);
        ActivateGraphicParams(graphicParams);
        }

    //! Set blanking fill symbology for decorations that are only used for display purposes. Pickable decorations require a category, must initialize
    //! a GeometryParams and cook it into a GraphicParams to have a locatable decoration.
    void SetBlankingFill(ColorDef fillColor)
        {
        GraphicParams graphicParams;
        graphicParams.SetFillColor(fillColor);
        graphicParams.SetIsBlankingRegion(true);
        ActivateGraphicParams(graphicParams);
        }
};

//=======================================================================================
//! A smart-pointer to a GraphicBuilder object.
// @bsistruct                                                   Paul.Connelly   05/16
//=======================================================================================
struct GraphicBuilderPtr
{
private:
    GraphicBuilder  m_builder;
public:
    GraphicBuilderPtr() {}
    GraphicBuilderPtr(GraphicBuilderP builder) : m_builder(builder) {}

    template<typename T> GraphicBuilderPtr(T* impl) : m_builder(impl) {}
    operator GraphicPtr() {return m_builder.m_graphic;}

    bool IsValid() const {return m_builder.IsValid();}
    bool IsNull() const {return !IsValid();}
    GraphicBuilderP get() {return IsValid() ? &m_builder : nullptr;}
    GraphicBuilderP operator->() {return get();}
    GraphicBuilderR operator*() {BeAssert(IsValid()); return *get();}
    GraphicP GetGraphic() {return m_builder.m_graphic.get();}
};

//=======================================================================================
// An ordered list of RefCountedPtrs to a Render::Graphics, plus an override.
// @bsiclass
//=======================================================================================
struct GraphicList : RefCounted<NonCopyableClass>
{
    struct Node
    {
        GraphicPtr  m_ptr;
        void*       m_overrides;
        uint32_t    m_ovrFlags;
        Node(Graphic& graphic, void* ovr, uint32_t ovrFlags) : m_ptr(&graphic), m_overrides(ovr), m_ovrFlags(ovrFlags) {}
    };

    bvector<Node> m_list;

    uint32_t GetCount() const {return (uint32_t) m_list.size();}
    bool IsEmpty() const {return m_list.empty();}
    void Clear() {m_list.clear();}
    DGNPLATFORM_EXPORT void Drop(Graphic& graphic);
    DGNPLATFORM_EXPORT void Add(Graphic& graphic, void* ovr, uint32_t ovrFlags);
    DGNPLATFORM_EXPORT void ChangeOverride(Graphic& graphic, void* ovr, uint32_t ovrFlags);
};

//=======================================================================================
//! A set of GraphicLists of various types of Graphics that are "decorated" into the Render::Target,
//! in addition to the Scene.
// @bsiclass                                                    Keith.Bentley   12/15
//=======================================================================================
struct Decorations
{
    GraphicListPtr m_flashed;        // drawn with zbuffer, with scene lighting
    GraphicListPtr m_world;          // drawn with zbuffer, with default lighting, smooth shading
    GraphicListPtr m_worldOverlay;   // drawn in overlay mode, world units
    GraphicListPtr m_viewOverlay;    // drawn in overlay mode, view units
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct Redraws
{
    GraphicListPtr m_erase;
    GraphicListPtr m_draw;
    GraphicListPtr m_change;
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
    void Init(FrustumCR frustum);
    bool IsValid() const {return m_isValid;}
    enum struct Contained {Outside = 0, Partly = 1,Inside = 2,};
    Contained Contains(FrustumCR box) const {return Contains(box.m_pts, 8);}
    bool Intersects(FrustumCR box) const {return Contained::Outside != Contains(box);}
    bool ContainsPoint(DPoint3dCR pt, double tolerance=1.0e-8) const {return Contained::Outside != Contains(&pt, 1, tolerance);}
    DGNPLATFORM_EXPORT Contained Contains(DPoint3dCP, int nPts, double tolerance=1.0e-8) const;
    DGNPLATFORM_EXPORT bool IntersectsRay(DPoint3dCR origin, DVec3dCR direction);
};

//=======================================================================================
//! A Render::Plan holds a Frustum and the render settings for displaying
//! the current Render::Scene into a Render::Target.
// @bsiclass                                                    Keith.Bentley   12/15
//=======================================================================================
struct Plan
{
    enum class AntiAliasPref {Detect=0, On=1, Off=2};

    ViewFlags     m_viewFlags;
    bool          m_is3d;
    Frustum       m_frustum;
    double        m_fraction;
    ColorDef      m_bgColor;
    AntiAliasPref m_aaLines;
    AntiAliasPref m_aaText;
    ClipPrimitiveCPtr m_activeVolume;
    DGNPLATFORM_EXPORT Plan(DgnViewportCR);
};

//=======================================================================================
//! A Render::Window is a platform specific object that identifies a rectangular window on a screen.
//! On Windows, for example, the default Render::Window holds an "HWND"
// @bsiclass                                                    Keith.Bentley   11/15
//=======================================================================================
struct Window : RefCounted<NonCopyableClass>
{
    struct Rectangle {int left, top, right, bottom;};
protected:
    Window() {}
public:
    virtual Point2d _GetScreenOrigin() const = 0;
    virtual BSIRect _GetViewRect() const = 0;
    virtual void _OnPaint(Rectangle&) const = 0;
    virtual void* _GetNativeWindow() const = 0;

#if defined (BENTLEYCONFIG_DISPLAY_WIN32)
    virtual HWND__* _GetHWnd() const {return nullptr;} //!< Note this may return null even on Windows, depending on the associated Render::Target
    HWND__* GetHWnd() const {return _GetHWnd();}
#endif
};

//=======================================================================================
//! A Render::Device is the platform specific object that connects a render target to a rendering system.
//! It holds a reference to a Render::Window.
//! On Windows, for example, the default Render::Device maps to a "DC"
// @bsiclass                                                    Keith.Bentley   11/15
//=======================================================================================
struct Device : RefCounted<NonCopyableClass>
{
    struct PixelsPerInch {int width, height;};
protected:
    WindowPtr m_window;
    Device(Window* window) : m_window(window) {}
public:
    virtual PixelsPerInch _GetPixelsPerInch() const = 0;
    virtual DVec2d _GetDpiScale() const = 0;
    virtual void* _GetNativeDevice() const = 0;
#if defined (BENTLEYCONFIG_DISPLAY_WIN32)
    virtual HDC__* GetDC() const {return nullptr;} //!< Note this may return null even on Windows, depending on the associated Render::System
#endif
    virtual TargetPtr _CreateTarget(double frameRateGoal) = 0;
    double PixelsFromInches(double inches) const {PixelsPerInch ppi=_GetPixelsPerInch(); return inches * (ppi.height + ppi.width)/2;}
    Window const* GetWindow() const {return m_window.get();}
};

//=======================================================================================
//! An array of GraphicPtrs. 
//! @note All entries are closed (and therefore may never change) when they're added to this array.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct GraphicBranch
{
    ClipPrimitiveCP m_clip = nullptr;
    bool m_hasFlags = false;
    ViewFlags m_viewFlags;
    bvector<GraphicPtr> m_entries;

    void Add(Graphic& graphic) {graphic.EnsureClosed(); m_entries.push_back(&graphic);}
    void SetClip(ClipPrimitiveCP clip) {m_clip = clip;}
    void SetViewFlags(ViewFlags flags) {m_hasFlags=true; m_viewFlags=flags;}
};

//=======================================================================================
//! A Render::System is the renderer-specific factory for creating Render::Graphics, Render::Textures, and Render::Materials.
//! @note The methods of this class may be called from any thread.
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct System
{
    //! Get or create a material from a material element, by id
    virtual MaterialPtr _GetMaterial(DgnMaterialId, DgnDbR) const = 0;

    //! Create a Material from parameters
    virtual MaterialPtr _CreateMaterial(Material::CreateParams const&) const = 0;

    virtual GraphicBuilderPtr _CreateGraphic(Graphic::CreateParams const& params) const = 0;
    virtual GraphicPtr _CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency) const = 0;
    virtual GraphicPtr _CreateBranch(Graphic::CreateParams const& params, GraphicBranch& branch) const = 0;

    //! Get or create a Texture from a DgnTexture element. Note that there is a cache of textures stored on a DgnDb, so this may return a pointer to a previously-created texture.
    //! @param[in] textureId the DgnElementId of the texture element
    //! @param[in] db the DgnDb for textureId
    virtual TexturePtr _GetTexture(DgnTextureId textureId, DgnDbR db) const = 0;

    //! Create a new Texture from an Image.
    virtual TexturePtr _CreateTexture(ImageCR image, Texture::CreateParams const& params=Texture::CreateParams()) const = 0;

    //! Create a new Texture from an ImageSource.
    virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::Format targetFormat, Image::BottomUp bottomUp, Texture::CreateParams const& params=Texture::CreateParams()) const = 0;

    //! Create a Texture from a graphic.
    virtual TexturePtr _CreateGeometryTexture(GraphicCR graphic, DRange2dCR range, bool useGeometryColors, bool forAreaPattern) const = 0;
};

//=======================================================================================
//! Provides an algorithm for dynamically adjusting the frame rate goal of a
//! Render::Target based on the ratio of successfully drawn to aborted frames and other
//! factors.
// @bsistruct                                                   Paul.Connelly   06/16
//=======================================================================================
struct FrameRateAdjuster
{
private:
    uint32_t        m_drawCount = 0;
    uint32_t        m_abortCount = 0;
public:
    static uint32_t const FRAME_RATE_MIN = 1;
    static uint32_t const FRAME_RATE_MAX = 30;

    //! Computes an adjusted frame rate goal based on factors like draw/abort ratio, smallest attempted element size, etc
    //! @param[in]      target    The target who's frame rate goal is to be adjusted
    //! @param[in]      saesNpcSq The smallest attempted element size (NPC squared)
    //! @return The adjusted frame rate goal
    DGNPLATFORM_EXPORT double AdjustFrameRate(Render::TargetCR target, double saesNpcSq);

    void Reset() {m_drawCount = m_abortCount = 0;}    //!< Reset abort/draw counts
    void IncrementDrawCount() {++m_drawCount;}        //!< Increment the number of frames drawn
    void IncrementAbortCount() {++m_abortCount;}      //!< Increment the number of drawn frames aborted
};

//=======================================================================================
//! A Render:Target holds the current "scene", the current set of dynamic Graphics, and the current decorators.
//! When frames are composed, all of those Graphics are rendered, as appropriate.
//! A Render:Target holds a reference to a Render::Device, and a Render::System
//! Every DgnViewport holds a reference to a Render::Target.
// @bsiclass                                                    Keith.Bentley   11/15
//=======================================================================================
struct Target : RefCounted<NonCopyableClass>
{
    struct SceneParameters
        {
        double m_saesNpcSq;     // smallest attempted element size (NPC squared)
        SceneParameters(double n = 0) : m_saesNpcSq(n) {}
        };

protected:
    bool               m_abort;
    System&            m_system;
    DevicePtr          m_device;
    ClipPrimitiveCPtr  m_activeVolume;
    GraphicListPtr     m_currentScene;
    GraphicListPtr     m_terrain;
    GraphicListPtr     m_dynamics;
    Decorations        m_decorations;
    double             m_frameRateGoal; // frames per second
    uint32_t           m_minimumFrameRate;
    BeAtomic<uint32_t> m_graphicsPerSecondScene;
    BeAtomic<uint32_t> m_graphicsPerSecondNonScene;

    virtual void _OnResized() {}
    virtual void* _ResolveOverrides(OvrGraphicParamsCR) = 0;
    virtual Point2d _GetScreenOrigin() const = 0;
    virtual BSIRect _GetViewRect() const = 0;
    virtual DVec2d _GetDpiScale() const = 0;

    DGNVIEW_EXPORT Target(SystemR, double frameRateGoal);
    DGNVIEW_EXPORT ~Target();
    DGNPLATFORM_EXPORT static void VerifyRenderThread();

public:
    struct Debug
    {
        static void SaveGPS(int, double);
        DGNPLATFORM_EXPORT static void SaveSceneTarget(int);
        DGNPLATFORM_EXPORT static void SaveProgressiveTarget(int);
        static void Show();
    };
    virtual void _OnDestroy() {}
    virtual void _ChangeScene(GraphicListR scene, ClipPrimitiveCP activeVolume, SceneParameters const& parms = SceneParameters()) {VerifyRenderThread(); m_currentScene = &scene; m_activeVolume=activeVolume;}
    virtual void _ChangeTerrain(GraphicListR terrain) {VerifyRenderThread(); m_terrain = !terrain.IsEmpty() ? &terrain : nullptr;}
    virtual void _ChangeDynamics(GraphicListP dynamics) {VerifyRenderThread(); m_dynamics = dynamics;}
    virtual void _ChangeDecorations(Decorations& decorations) {VerifyRenderThread(); m_decorations = decorations;}
    virtual void _ChangeRenderPlan(PlanCR) = 0;
    virtual void _Redraw(Redraws&) = 0;
    virtual void _BeginHeal() = 0;
    virtual void _DrawHeal(GraphicListR healList) = 0;
    enum class HealAborted : bool {No=0, Yes=1};
    virtual void _FinishHeal(HealAborted) = 0;
    virtual bool _NeedsHeal(BSIRectR) const = 0;
    virtual void _DrawFrame(StopWatch&) = 0;
    virtual Image _ReadImage(BSIRectCR viewRect, Point2dCR targetSize) = 0;
    virtual void _DrawProgressive(GraphicListR progressiveList, StopWatch&) = 0;
    virtual bool _WantInvertBlackBackground() {return false;}
    virtual uint32_t _SetMinimumFrameRate(uint32_t minimumFrameRate){m_minimumFrameRate = minimumFrameRate; return m_minimumFrameRate;}
    virtual double _GetCameraFrustumNearScaleLimit() const = 0;
    virtual double _FindNearestZ(DRange2dCR) const = 0;

    void AbortProgressive() {m_abort=true;}
    Point2d GetScreenOrigin() const {return _GetScreenOrigin();}
    BSIRect GetViewRect() const {return _GetViewRect();}
    DVec2d GetDpiScale() const {return _GetDpiScale();}
    DeviceCP GetDevice() const {return m_device.get();}
    void OnResized() {_OnResized();}
    void* ResolveOverrides(OvrGraphicParamsCP ovr) {return ovr ? _ResolveOverrides(*ovr) : nullptr;}
    GraphicBuilderPtr CreateGraphic(Graphic::CreateParams const& params) {return m_system._CreateGraphic(params);}
    GraphicPtr CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency) {return m_system._CreateSprite(sprite, location, xVec, transparency);}
    MaterialPtr GetMaterial(DgnMaterialId id, DgnDbR dgndb) const {return m_system._GetMaterial(id, dgndb);}
    TexturePtr GetTexture(DgnTextureId id, DgnDbR dgndb) const {return m_system._GetTexture(id, dgndb);}
    TexturePtr CreateTexture(ImageCR image) const {return m_system._CreateTexture(image);}
    TexturePtr CreateTexture(ImageSourceCR source, Image::Format targetFormat=Image::Format::Rgb, Image::BottomUp bottomUp=Image::BottomUp::No) const {return m_system._CreateTexture(source, targetFormat, bottomUp);}
    TexturePtr CreateGeometryTexture(Render::GraphicCR graphic, DRange2dCR range, bool useGeometryColors, bool forAreaPattern) const {return m_system._CreateGeometryTexture(graphic, range, useGeometryColors, forAreaPattern);}
    SystemR GetSystem() {return m_system;}

    static double DefaultFrameRateGoal() 
        {
#ifdef BENTLEYCONFIG_GRAPHICS_DIRECTX // *** WIP - we are trying to predict the likely graphics performance of the box.
        return 20.0; // Plan for the best on Windows (desktop) computers.
#else
        return 10.0; // Plan for the worst on mobile devices
#endif
        }

    double GetFrameRateGoal() const {return m_frameRateGoal;}
    void SetFrameRateGoal(double goal) {m_frameRateGoal = goal;}
    static int const FRAME_RATE_MIN_DEFAULT = 5;
    uint32_t GetMinimumFrameRate() const {return m_minimumFrameRate;}
    uint32_t SetMinimumFrameRate(uint32_t minimumFrameRate) {return _SetMinimumFrameRate(minimumFrameRate);}
    uint32_t GetGraphicsPerSecondScene() const {return m_graphicsPerSecondScene.load();}
    uint32_t GetGraphicsPerSecondNonScene() const {return m_graphicsPerSecondNonScene.load();}
    void RecordFrameTime(GraphicList& scene, double seconds, bool isFromProgressiveDisplay) {RecordFrameTime(scene.GetCount(), seconds, isFromProgressiveDisplay);}
    DGNPLATFORM_EXPORT void RecordFrameTime(uint32_t numGraphicsInScene, double seconds, bool isFromProgressiveDisplay);

    //! Make the specified rectangle have the specified aspect ratio 
    //! @param[in] requestedRect    The rectangle within the view that the caller would like to capture
    //! @param[in] targetAspectRatio The desired aspect ratio 
    //! @return The adjusted rectangle that captures as much of the requested rectangle as possible 
    //!         with one of its dimensions adjusted to match the aspect ratio of targetSize.
    DGNPLATFORM_EXPORT static BSIRect SetAspectRatio(BSIRectCR requestedRect, double targetAspectRatio);
};

END_BENTLEY_RENDER_NAMESPACE
