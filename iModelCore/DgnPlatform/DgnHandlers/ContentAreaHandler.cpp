/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/ContentAreaHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

// The purpose of the  structures in this file is to extract data from the persisted form stored
// on XAttributes where it is more important to have a compact structure than have effcient memory
// access. Force the compiler use 1 byte aligment for the structures in this header file.  This prevents
// the compiler from padding the structures with empty space to align members.
#pragma pack(push)
#pragma pack(1)

/*=================================================================================**//**
* @bsiclass                                                     Mark.Dane       10/2010
+===============+===============+===============+===============+===============+======*/
struct VersionData
    {
protected:
    UInt8   m_version;
public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    VersionData () : m_version ((UInt8) CurrentVersion()) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    UInt8 GetVersion () const  {return m_version;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    static int CurrentVersion() {return 1;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BentleyApi::DgnPlatform::XAttributeHandlerId XAttributeId()
        {
        return BentleyApi::DgnPlatform::XAttributeHandlerId (DgnPlatform::XATTRIBUTEID_Markup, DgnPlatform::MINORID_ContentAreaVersion);
        }
    }; // VersionData

/*=================================================================================**//**
* @bsiclass                                                     Mark.Dane       10/2010
+===============+===============+===============+===============+===============+======*/
struct AreaData
    {
protected:
    double  m_originX;
    double  m_originY;
    double  m_width;
    double  m_height;

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    AreaData (DRange2dCR range)
    :
    m_originX (range.low.x),
    m_originY (range.low.y),
    m_width (range.high.x - range.low.x),
    m_height (range.high.y - range.low.y)
        {
        // Verify assumtions about size since we are using this to read/write persistent bytes
        BeAssert (8 == sizeof (double));
        BeAssert (4 * sizeof(double) == sizeof(AreaData));
        BeAssert (range.low.x < range.high.x);
        BeAssert (range.low.y < range.high.y);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    DRange2d GetArea () const
        {
        DRange2d range;
        range.low.x = m_originX;
        range.low.y = m_originY;
        range.high.x = m_originX + m_width;
        range.high.y = m_originY + m_height;
        return range;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BentleyApi::DgnPlatform::XAttributeHandlerId XAttributeId()
        {
        return BentleyApi::DgnPlatform::XAttributeHandlerId (DgnPlatform::XATTRIBUTEID_Markup, DgnPlatform::MINORID_ContentAreaArea);
        }

    }; // AreaData

typedef struct AreaData const*     AreaDataCP;
typedef struct VersionData const*  VersionDataCP;

#pragma pack(pop)


#define DOUBLE_TOLERANCE 1e-12


ELEMENTHANDLER_DEFINE_MEMBERS (ContentAreaHandler);

/*---------------------------------------------------------------------------------**//**
* Get points in counter clockwise order starting from bottom left
* @bsimethod                                    Mark.Dane                       08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void get4Corners (DPoint2d points[], DRange2dCR range)
    {
    range.get4Corners (points);
    std::swap (points[2], points[3]);
    }

/*---------------------------------------------------------------------------------**//**
* Get points in counter clockwise order starting from bottom left
* @bsimethod                                    Mark.Dane                       08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void get4Corners3d (DPoint3d points[], DRange2dCR range)
    {
    DPoint2d    localBuf2d[4];

    get4Corners (localBuf2d, range);

    for (int i=0; i<4; i++)
        points[i].Init (localBuf2d[i]);
    }

#if defined (NEEDSWORK_DOESNT_GO_HERE)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void getLocalizedString (WStringR string, int messageId, int desiredLength = -1)
    {
    WChar buffer[256] = {'\0'};
    if (SUCCESS != mdlResource_loadFromStringList (buffer, s_rscFile, MSGLISTID_ContentAreaHandler, messageId))
        string.assign(L"");
    else
        string.assign(buffer);
    }

/*=================================================================================**//**
* @bsiclass                                                     Mark.Dane       08/2010
+===============+===============+===============+===============+===============+======*/
struct ContentAreaDragManipulator : RefCounted <IDragManipulator>
    {
private:
    static const int      s_controlCount = 4;
    DPoint3d              m_controlPoints[s_controlCount];
    ControlsSelectState   m_controlStates[s_controlCount];
    int                   m_anchorIndex;
    int                   m_selectedCount;

protected:
    virtual bool        _OnCreateControls (ElementHandleCR elHandle) override
        {
        DRange2d range = ContentAreaHandler::GetArea(elHandle);
        DPoint2d box2d[4];
        get4Corners (box2d, range);

        for (int i = 0; i < 4; i++)
            {
            m_controlPoints[i].init (&box2d[i]);
            m_controlStates[i] = CONTROL_SELECT_None;
            }
        m_anchorIndex = 0;

        return true;
        }

    virtual void        _OnCleanupControls (ElementHandleCR elHandle) override {};
    virtual bool        _OnSelectControl (ElementHandleCR elHandle, DgnButtonEventCP ev) override
        {
        return ManipulatorUtils::DoSelectControl (&m_selectedCount, m_controlStates, m_controlPoints, s_controlCount, NULL, ev, NULL, false);
        }

    virtual bool        _OnSelectControl (ElementHandleCR elHandle, HitPathCP path) override 
        {
        return ManipulatorUtils::DoSelectControl (&m_selectedCount, m_controlStates, m_controlPoints, s_controlCount, NULL, path, NULL, false);
        }
    virtual bool        _OnMultiSelectControls (ElementHandleCR elHandle, DgnButtonEventCP ev, SelectionMode mode) override {return ManipulatorUtils::DoMultiSelectControls (&m_selectedCount, m_controlStates, m_controlPoints, s_controlCount, NULL, ev, mode, NULL, false);}
    virtual bool        _OnMultiSelectControls (ElementHandleCR elHandle, FenceParamsP fp, SelectionMode mode)override {return ManipulatorUtils::DoMultiSelectControls (&m_selectedCount, m_controlStates, m_controlPoints, s_controlCount, NULL, fp, mode, NULL, false);}
    virtual bool        _HasSelectedControls (ElementHandleCR elHandle) override {return m_selectedCount > 0;}
    virtual void        _OnDraw (ElementHandleCR elHandle, ViewportP vp)override {ManipulatorUtils::DoDrawControls (vp, m_controlPoints, m_controlStates, s_controlCount, NULL, NULL, NULL);}
    virtual WString     _OnGetDescription (ElementHandleCR elHandle) override
        {
        WString description;
        getLocalizedString (description, MSG_DragToResize);
        return description;
        }
    virtual bool        _OnSetupDrag (DgnButtonEventR ev, EditElementHandleR elHandle) override
        {
        m_anchorIndex =ManipulatorUtils::DoSetupDrag (ev, m_controlPoints, s_controlCount, NULL);
        return true;
        }
    virtual void        _OnStartDrag (ElementHandleCR elHandle, DgnButtonEventCP ev) override
        {

        }
    virtual void        _OnCancelDrag (ElementHandleCR elHandle, DgnButtonEventCP ev) override {}
    virtual StatusInt   _OnDrag (EditElementHandleR elHandle, DgnButtonEventCP ev) override {return ManipulatorUtils::DoDrag (elHandle, ev, this);}
    virtual StatusInt   _OnEndDrag (EditElementHandleR elHandle, DgnButtonEventCP ev) override {return ManipulatorUtils::DoEndDrag (elHandle, ev, this);}
    virtual StatusInt   _DoDragControls (EditElementHandleR elHandle, DgnButtonEventCP ev, bool isDynamics) override
        {
        int oppositeCorner =  (m_anchorIndex + 2) % s_controlCount;

        DPoint3d pointArray[2] = {m_controlPoints[oppositeCorner], *ev->GetPoint()};
        DRange2d range;
        range.initFrom (pointArray, 2);

        ContentAreaHandler::SetArea (elHandle, range);
        return SUCCESS;
        }

    ContentAreaDragManipulator () : m_selectedCount (0), m_anchorIndex (0) {}

public:
    static ContentAreaDragManipulator* Create() {return new ContentAreaDragManipulator();}
    };

/*=================================================================================**//**
* @bsiclass                                                     Mark.Dane       09/2010
+===============+===============+===============+===============+===============+======*/
struct ContentAreaDragExtension : DgnPlatform::IDragManipulatorExtension
    {
protected:
    virtual IDragManipulatorP       _GetIDragManipulator (ElementHandleCR elHandle, DisplayPathCP path)
        {
        return ContentAreaDragManipulator::Create();
        }
    };


#endif // NEEDSWORK_DOESNT_GO_HERE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentAreaHandler::_GetTypeName (WStringR name, UInt32 desiredLength)
    {
    name.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_ContentArea));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentAreaHandler::_GetDescription
(
ElementHandleCR     el,
WStringR            descr,
UInt32              desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_ContentArea));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DRange2d ContentAreaHandler::GetArea (ElementHandleCR el)
    {
    DRange2d range;

    // default to the scan range since this is what the unpublished version 0 used
    DRange3dCP scanRange = &el.GetElementCP()->GetRange();
    range.low.x   = scanRange->low.x;
    range.low.y   = scanRange->low.y;
    range.high.x  = scanRange->high.x;
    range.high.y  = scanRange->high.y;

    ElementHandle::XAttributeIter xAttribute (el, AreaData::XAttributeId());
    if (!xAttribute.IsValid() || xAttribute.GetSize() < sizeof (AreaData))
        {
        BeAssert (false && "ContentAreaHandler could not find area data");
        return range;
        }

    AreaDataCP data = (AreaDataCP) xAttribute.PeekData();
    return data->GetArea();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt updateXGraphics (EditElementHandleR eeh)
    {
    XGraphicsContainer::RemoveFromElement(eeh); // if existing xgraphics are not removed the
                                                // call to CreateFromElement simply optimizes
                                                // the existing xgraphics and does not redraw

    XGraphicsContainer  xgContainer;
    if (SUCCESS != xgContainer.CreateFromElement (eeh))
        return ERROR;

    return xgContainer.AddToElement (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ContentAreaHandler::SetArea (EditElementHandleR eeh, DRange2dCR area)
    {
    AreaData data (area);
    if (SUCCESS != eeh.ScheduleWriteXAttribute (AreaData::XAttributeId(), 0, sizeof (AreaData), &data))
        return ERROR;

    // update scan range
    DRange3d range3d;

    range3d.low.x = area.low.x;
    range3d.low.y = area.low.y;
    range3d.low.z = 0;
    range3d.high.x = area.high.x;
    range3d.high.y = area.high.y;
    range3d.high.z = 0;

    eeh.GetElementP()->GetRangeR() = range3d;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentAreaHandler::IsValidTransform (TransformInfoCR transformInfo) const
    {
    for (int i = 0; i < 2; i++)
        {
        for (int j = 0; j < 2; j++)
            {
            if (i == j)
                continue;
            if (fabs (transformInfo.GetTransform()->getFromMatrixByRowAndColumn (i, j)) > DOUBLE_TOLERANCE)
                return false;
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ContentAreaHandler::_OnTransform (EditElementHandleR element, TransformInfoCR transformInfo)
    {
    if (!IsValidTransform (transformInfo))
        return ERROR;

    DRange2d range2d = GetArea (element);
    DPoint2d range2dArray[] = {range2d.low, range2d.high};

    DRange3d range3d;
    range3d.initFrom (range2dArray, 2, 0);
    transformInfo.GetTransform()->multiply (&range3d, &range3d);

    DPoint3d range3dArray[] ={range3d.low, range3d.high};
    range2d.initFrom (range3dArray, 2);

    SetArea (element, range2d);
    updateXGraphics (element);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       08/2010
+---------------+--------------+---------------+---------------+---------------+------*/
bool ContentAreaHandler::_IsTransformGraphics (ElementHandleCR element, TransformInfoCR transformInfo)
    {
    return IsValidTransform (transformInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void drawShape (DRange2dCR range, bool filled, ViewContextR context)
    {
    DPoint2d points[5];
    get4Corners (points, range);
    points[4] = points[0];

    context.GetIDrawGeom().DrawShape2d(5, points, filled, context.GetDisplayPriority(), NULL);
    }

/*=================================================================================**//**
* @bsiclass                                                     Mark.Dane       08/2010
+===============+===============+===============+===============+===============+======*/
struct ContentAreaFillStroker : IStrokeForCache 
    {
    DRange2d m_outer;
    DRange2d m_inner;

    ContentAreaFillStroker (DRange2dCR outerBorder, DRange2dCR textBorder) : m_outer (outerBorder), m_inner(textBorder) {}

    virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize = 0.0) override
        {
        DPoint3d    points[5];

        CurveVectorPtr region = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
        CurveVectorPtr outerLoop = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
        CurveVectorPtr innerLoop = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Inner);

        get4Corners3d (points, m_outer); points[4] = points[0];
        outerLoop->push_back (ICurvePrimitive::CreateLineString (points, 5));

        get4Corners3d (points, m_inner); points[4] = points[0];
        innerLoop->push_back (ICurvePrimitive::CreateLineString (points, 5));

        region->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*outerLoop.get ()));
        region->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*innerLoop.get ()));

        context.GetIDrawGeom().DrawCurveVector (*region, true);
        }
    }; // ContentAreaFillSTroker

/*=================================================================================**//**
* @bsiclass                                                     Mark.Dane       08/2010
+===============+===============+===============+===============+===============+======*/
struct ContentAreaPickStroker : IStrokeForCache
    {
    virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
        {
        BeAssert(NULL != dh.GetElementHandleCP());
        ElementHandleCR el = *dh.GetElementHandleCP();
        DRange2d range = ContentAreaHandler::GetArea (el);
        drawShape (range, true, context);
        }

    virtual bool _WantLocateByStroker () override {return false;} // Don't call _StrokeForCache, locate interior by QvElem...

    }; // ContentAreaPickStroker

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentAreaHandler::_Draw (ElementHandleCR el, ViewContextR context)
    {
    DrawPurpose purpose = context.GetDrawPurpose();
    if (DrawPurpose::Pick == purpose)
        {
        // Picking matches the interior of a shape only works the solid area is drawn as cached element.
        // Snapping to the corners only works if the element is not cached.  Draw the element twice, once
        // cached and once as a regular draw to get them to work both at the same time.
        ContentAreaPickStroker pickStroker;
        CachedDrawHandle dh(&el);
        context.DrawCached (dh, pickStroker,QV_CACHE_INDEX_Pick);
        pickStroker._StrokeForCache (dh, context, 0);
        return;
        }

    XGraphicsContainer container;
    if (DrawPurpose::Dynamics != purpose && DrawPurpose::XGraphicsCreate != purpose && DrawPurpose::CutXGraphicsCreate != purpose &&
        SUCCESS == container.ExtractFromElement (el))
        {
        container.Draw (context);
        return;
        }

    ViewContext::ContextMark saveContext(&context);

    DRange2d range = ContentAreaHandler::GetArea (el);

    // Draw the border
    drawShape (range, false, context);

    // Draw the "Markup Content" text in the middle
    WString label;
    label.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_ContentArea));

    if (0 == label.size ())
        return;

    double width = range.high.x - range.low.x;
    double height = range.high.y - range.low.y;

    DPoint2d charSize;
    charSize.y = charSize.x = width / label.size ();

    TextStringProperties textStrProp;
    textStrProp.SetFont (DgnFontManager::GetDecoratorFont());
    textStrProp.ApplyScale (charSize);

    RotMatrix rot;
    rot.initIdentity();

    DPoint3d location;
    location.x = range.low.x + (range.high.x - range.low.x) * 0.5;
    location.y = range.low.y + (range.high.y - range.low.y) * 0.5;
    location.z = context.GetDisplayPriority();

    TextString textStr (label.GetWCharCP(), &location, &rot, textStrProp);
    textStr.MakeExtentsValid();
    location.x -= textStr.GetWidth() / 2;
    location.y += textStr.GetHeight() / 2;
    textStr.SetOriginFromUserOrigin (location);
    context.GetIDrawGeom().DrawTextString (textStr); // txtStr.Draw (*context);

    if (DrawPurpose::Dynamics == purpose)
        return;

    // Draw the hatch at 50% opacity
    DRange2d textRange;
    textRange.low.init (&location);
    textRange.high.init (&location);
    textRange.high.x += textStr.GetWidth ();
    textRange.low.y -= textStr.GetHeight ();
    textRange.extend (charSize.x / 4.0);

    PatternParams hatch;
    hatch.Init();
    hatch.angle1 = PI / 4.0;
    hatch.space1 = MAX(width, height) / 10;
    hatch.rMatrix.initIdentity ();
    hatch.modifiers =  PatternParamsModifierFlags::Space1 | PatternParamsModifierFlags::Angle1 | PatternParamsModifierFlags::RotMatrix;

    ContentAreaFillStroker hatchAreaStroke (range, textRange);
    ViewContext::PatternParamSource patternParamSource (&hatch, NULL);
    ViewContext::ClipStencil stencil(hatchAreaStroke, 0, false);

    ElemDisplayParamsP edParams = context.GetCurrentDisplayParams();
    double opacity = 1.0 - edParams->GetTransparency ();
    opacity = opacity * 0.3;
    edParams->SetTransparency (1.0 - opacity);
    context.CookDisplayParams ();

    // Insure that the hatch is always draw even if patterns are not requested.
    // Patterns tends to be off in XGraphics creation.
    ViewFlags patternDisplayFlags;
    if (NULL == context.GetViewFlags())
        memset (&patternDisplayFlags, 0, sizeof(ViewFlags));
    else
        patternDisplayFlags = *context.GetViewFlags();

    patternDisplayFlags.patterns = true;
    patternDisplayFlags.patternDynamics = true;
    context.SetViewFlags(&patternDisplayFlags);

    context.DrawAreaPattern (el, stencil, patternParamSource);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//ITransactionHandler::PreActionStatus ContentAreaHandler::_OnAdd (EditElementHandleR eeh)
//    {
//    DgnModelP model = eeh.GetDgnModel();
//    BeAssert (NULL != model);
//
//    if (ContentAreaHandler::FindElement (NULL, *model))
//        return PRE_ACTION_Block;
//
//    updateXGraphics (eeh); // Add XGraphics to new element...
//    return PRE_ACTION_Ok;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------*/
//ITransactionHandler::PreActionStatus ContentAreaHandler::_OnReplace (EditElementHandleR eeh, ElementHandleCR eh)
//    {
//    updateXGraphics (eeh); // Update XGraphics for modified element...
//    return PRE_ACTION_Ok;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerId ContentAreaHandler::GetHandlerId ()
    {
    return ElementHandlerId (XATTRIBUTEID_Markup, MINORID_ContentArea);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentAreaHandler::CreateContentArea (EditElementHandleR eeh, DRange2dCR range2d, DgnModelP model)
    {
    ExtendedElementHandler::InitializeElement (eeh, NULL /* no template */, *model, false, false);

    DgnElementP elementP = eeh.GetElementP();
    elementP->GetSymbologyR().color  = 0;
    elementP->GetSymbologyR().weight = 0;
    elementP->GetSymbologyR().style  = 0;

    SetArea (eeh, range2d);

    VersionData versionData;
    eeh.ScheduleWriteXAttribute (VersionData::XAttributeId(), 0, sizeof (VersionData), &versionData);

#ifdef DGNV10FORMAT_CHANGES_WIP
    ElementHandlerXAttribute    handlerXAttr (GetHandlerId ());
    ElementHandlerManager::AddHandlerToElement (eeh, handlerXAttr);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentAreaHandler::IsContentArea (ElementHandleCR element)
    {
    return NULL != dynamic_cast<ContentAreaHandler*> (&element.GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentAreaHandler::FindElement (ElementHandleP outElem, DgnModelR model)
    {
    if (NULL != outElem)
        *outElem = ElementHandle();

    PersistentElementRefListIterator  elementIterator = model.GetGraphicElementsP()->begin();
    for (ElementRefP testElemRef = elementIterator.GetCurrentElementRef(); NULL != testElemRef; testElemRef = elementIterator.GetNextElementRef())
        {
        if (EXTENDED_ELM != testElemRef->GetLegacyType())
            continue;

        ElementHandle eh (testElemRef);
        if (NULL != dynamic_cast<ContentAreaHandler*> (&eh.GetHandler()))
            {
            if (NULL != outElem)
                *outElem = eh;
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentAreaHandler::GetVersion (ElementHandleCR el)
    {
    ElementHandle::XAttributeIter xAttribute(el, VersionData::XAttributeId());
    if (!xAttribute.IsValid() || xAttribute.GetSize() < sizeof (VersionData))
        return 0;

    VersionDataCP data = (VersionDataCP) xAttribute.PeekData();
    return data->GetVersion();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mark.Dane                       10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentAreaHandler::GetCompiledVersion()
    {
    return VersionData::CurrentVersion();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentAreaHandler::Register()
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    // register built-in handlers
    ElementHandlerManager::RegisterHandler (ContentAreaHandler::GetHandlerId(), ELEMENTHANDLER_INSTANCE (ContentAreaHandler));
#endif
    }

#if defined (NOT_HERE)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Wil.Maier       10/08
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" __declspec (dllexport) int MdlMain
(
int     argc,
char    *argv[]
)
    {
    mdlSystem_setMdlAppClass (NULL, APPLICATION_MSREQD);    // Can't let handler unload once loaded
    mdlResource_openFile (&s_rscFile, NULL, RSC_READ);

    ElementHandlerManager::RegisterHandler (ContentAreaHandler::GetHandlerId (), ContentAreaHandler::GetInstance());
    IDragManipulatorExtension::RegisterExtension (ContentAreaHandler::GetInstance(), *new ContentAreaDragExtension());
    return 0;
    }
#endif


