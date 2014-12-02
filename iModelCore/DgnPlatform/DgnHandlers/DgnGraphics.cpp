/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnGraphics.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

#define LOG (*Bentley::NativeLogging::LoggingManager::GetLogger (L"DgnGraphics"))

static const Symbology s_defaultSymbology = { STYLE_BYLEVEL, WEIGHT_BYLEVEL, COLOR_BYLEVEL };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
SymbologyCR DgnGraphics::GetDefaultSymbology() { return s_defaultSymbology; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
UInt32 DgnGraphics::GetSymbologyColorFromRgb (RgbColorDefCR rgb)
    {
    DgnModelP model = m_sourceElement ? m_sourceElement->GetDgnModelP() : m_element.GetDgnModelP();
    return model->GetDgnProject().Colors().CreateElementColor (IntColorDef (rgb), NULL, NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
PhysicalGraphicsPtr PhysicalModel::CreatePhysicalGraphics (LevelId level)
    {
    PhysicalGraphicsPtr result = new PhysicalGraphics();
    result->Init (*this, level);
    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DrawingGraphicsPtr DgnModel2d::Create2dGraphics (LevelId level)
    {
    DrawingGraphicsPtr result = new DrawingGraphics();
    result->Init (*this, level);
    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
PhysicalGraphicsPtr PhysicalModel::ReadPhysicalGraphics (ElementId id)
    {
    PersistentElementRefPtr element = GetDgnProject().Models().GetElementById (id);
    if (!element.IsValid())
        return NULL;

    PhysicalGraphicsPtr result = new PhysicalGraphics();
    result->Load (element.get());
    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DrawingGraphicsPtr DgnModel2d::Read2dGraphics (ElementId id)
    {
    PersistentElementRefPtr element = GetDgnProject().Models().GetElementById (id);
    if (!element.IsValid())
        return NULL;

    DrawingGraphicsPtr result = new DrawingGraphics();
    result->Load (element.get());
    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DgnGraphics::Entry::Entry (IGeometryPtr& geometry, SymbologyCR symbology) : m_geometry (geometry), m_symbology (symbology)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DgnGraphics::Entry::Entry (TextStringP text, SymbologyCR symbology)
    :
    m_text (text),
    m_symbology (symbology)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  06/14
//---------------------------------------------------------------------------------------
bool DgnGraphics::Entry::TransformInPlace (TransformCR transform)
    {
    if (m_text.IsValid ()) // Is this the correct way to transform a TextString?!? NEEDSWORK - Add TextStringPtr TextString::CopyTransformed (TextStringCR) method...
        {
        DPoint2d                scaleFactor;
        DPoint3d                lowerLeft = m_text->GetOrigin ();
        RotMatrix               rMatrix = m_text->GetRotMatrix ();
        TextStringPropertiesPtr props = m_text->GetProperties ().Clone ();

        transform.Multiply (lowerLeft);
        TextString::TransformOrientationAndGetScale (scaleFactor, rMatrix, NULL, transform, false);
        props->ApplyScale (scaleFactor);
        props->SetIs3d (false);

        m_text = TextString::Create (m_text->GetString ().c_str (), &lowerLeft, &rMatrix, *props);

        return true;
        }

    return (m_geometry.IsValid () ? m_geometry->TryTransformInPlace (transform) : true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TextStringP             DgnGraphics::Entry::GetAsTextStringP()          { return m_text.get(); }
TextStringCP            DgnGraphics::Entry::GetAsTextStringCP() const   { return m_text.get(); }

SymbologyR              DgnGraphics::Entry::GetSymbologyR()             { return m_symbology; }
SymbologyCR             DgnGraphics::Entry::GetSymbology() const        { return m_symbology; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
ICurvePrimitiveCP DgnGraphics::Entry::GetAsICurvePrimitiveCP() const
    {
    ICurvePrimitivePtr curvePrimitive = m_geometry.IsValid () ? m_geometry->GetAsICurvePrimitive () : NULL;

    return curvePrimitive.get ();
    }

ICurvePrimitiveP DgnGraphics::Entry::GetAsICurvePrimitiveP() { return const_cast <ICurvePrimitiveP> (GetAsICurvePrimitiveCP()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
CurveVectorCP DgnGraphics::Entry::GetAsCurveVectorCP() const
    {
    CurveVectorPtr curveVector = m_geometry.IsValid () ? m_geometry->GetAsCurveVector () : NULL;

    return curveVector.get ();
    }

CurveVectorP DgnGraphics::Entry::GetAsCurveVectorP() { return const_cast <CurveVectorP> (GetAsCurveVectorCP()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
PolyfaceHeaderCP DgnGraphics::Entry::GetAsPolyfaceHeaderCP() const
    {
    PolyfaceHeaderPtr meshData = m_geometry.IsValid () ? m_geometry->GetAsPolyfaceHeader () : NULL;

    return meshData.get ();
    }

PolyfaceHeaderP DgnGraphics::Entry::GetAsPolyfaceHeaderP() { return const_cast <PolyfaceHeaderP> (GetAsPolyfaceHeaderCP()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
MSBsplineSurfaceCP DgnGraphics::Entry::GetAsMSBsplineSurfaceCP() const
    {
    MSBsplineSurfacePtr bSurface = m_geometry.IsValid () ? m_geometry->GetAsMSBsplineSurface () : NULL;

    return bSurface.get ();
    }

MSBsplineSurfaceP DgnGraphics::Entry::GetAsMSBsplineSurfaceP() { return const_cast <MSBsplineSurfaceP> (GetAsMSBsplineSurfaceCP()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
ISolidPrimitiveCP DgnGraphics::Entry::GetAsISolidPrimitiveCP() const
    {
    ISolidPrimitivePtr solidPrimitive = m_geometry.IsValid () ? m_geometry->GetAsISolidPrimitive () : NULL;

    return solidPrimitive.get ();
    }

ISolidPrimitiveP DgnGraphics::Entry::GetAsISolidPrimitiveP() { return const_cast <ISolidPrimitiveP> (GetAsISolidPrimitiveCP()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DgnGraphics::Entry::Type DgnGraphics::Entry::GetType() const
    {
    if (m_text.IsValid ())
        return Type::Text;
        
    if (!m_geometry.IsValid ())
        return Type::None;

    switch (m_geometry->GetGeometryType ())
        {
        case IGeometry::GeometryType::CurvePrimitive:
            return Type::CurvePrimitive;

        case IGeometry::GeometryType::CurveVector:
            return Type::CurveVector;

        case IGeometry::GeometryType::SolidPrimitive:
            return Type::SolidPrimitive;

        case IGeometry::GeometryType::BsplineSurface:
            return Type::MSBsplineSurface;

        case IGeometry::GeometryType::Polyface:
            return Type::Polyface;
        }

    BeAssert (false); 
    return Type::None;          
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
void DgnGraphics::Init (DgnModelR model, LevelId level)
    {
    ExtendedElementHandler::InitializeElement (GetElementHandleR(), NULL, model, model.Is3d());
    GetElementHandleR().GetElementDescrP()->ElementR().SetLevel(level.GetValueUnchecked());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
void DgnGraphics::DrawEntry (IDrawGeomR drawGeom, Entry const& entry)
    {
    switch (entry.GetType())
        {
        case Entry::Type::Text:
            {
            drawGeom.DrawTextString (*entry.GetAsTextStringCP());
            break;
            }

        case Entry::Type::CurvePrimitive:
            {
            CurveVectorPtr curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, const_cast <Entry*> (&entry)->GetAsICurvePrimitiveP());
            drawGeom.DrawCurveVector (*curve, false);
            break;
            }

        case Entry::Type::CurveVector:
            {
            CurveVectorPtr curve = const_cast <Entry*> (&entry)->GetAsCurveVectorP();
            drawGeom.DrawCurveVector (*curve, curve->IsClosedPath());
            break;
            }

        case Entry::Type::Polyface:
            {
            drawGeom.DrawPolyface (*const_cast <Entry*> (&entry)->GetAsPolyfaceHeaderP());
            break;
            }

        case Entry::Type::MSBsplineSurface:
            {
            drawGeom.DrawBSplineSurface (*entry.GetAsMSBsplineSurfaceCP());
            break;
            }

        case Entry::Type::SolidPrimitive:
            {
            drawGeom.DrawSolidPrimitive (*entry.GetAsISolidPrimitiveCP());
            break;
            }

        case Entry::Type::None:
        default:
            {
            BeAssert (false);
            break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
void DgnGraphics::DrawSymbology (ViewContextR context, SymbologyCR symb)
    {
    ElemDisplayParamsP dp = context.GetCurrentDisplayParams();
    dp->SetLineColor(symb.color);
    dp->SetWeight(symb.weight);
    dp->SetLineStyle(symb.style, NULL);

    context.CookDisplayParams();
    }

#define COMPARE_VALUES(val0, val1)  if (val0 < val1) { return true; } if (val0 > val1) { return false; }
//=======================================================================================
// @bsiclass                                                    MattGooding     08/13
//=======================================================================================
struct SymbologyCompare
    {
    bool operator() (SymbologyCR symb0, SymbologyCR symb1) const
        {
        COMPARE_VALUES (symb0.color, symb1.color);
        COMPARE_VALUES (symb0.weight, symb1.weight);
        COMPARE_VALUES (symb0.style, symb1.style);

        return false;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/14
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphics::SaveEntries(EditElementHandleR eh)
    {
    if (m_entries.empty()) // disallow elements without graphics
        return ERROR;

    // Separate by symbology...
    bmap <Symbology, bvector <Entry const*>, SymbologyCompare> symbMap;
    for (EntryPtr const& entry : m_entries)
        symbMap[entry->GetSymbology()].push_back (entry.get());

    // ...and determine the most common symbology to store it off in the element header.
    auto mostCommonSymb = symbMap.begin();
    for (auto symbMapIter = mostCommonSymb; symbMap.end() != symbMapIter; ++symbMapIter)
        {
        if (mostCommonSymb->second.size() < symbMapIter->second.size())
            mostCommonSymb = symbMapIter;
        }

    eh.GetElementDescrP()->ElementR().GetSymbologyR() = mostCommonSymb->first;

    XGraphicsContainer container;
    container.BeginDraw();

    ViewContextP xgContext = XGraphicsPublish::CreateXGraphicsContext (container, XGRAPHIC_CreateOptions_None, XGRAPHIC_OptimizeOptions_None, *eh.GetDgnModelP());

    // Won't actually be written to XG stream, but will set baseline for future symbology changes.
    DrawSymbology (*xgContext, mostCommonSymb->first);

    for (Entry const* symbGraphics : mostCommonSymb->second)
        DrawEntry (xgContext->GetIDrawGeom(), *symbGraphics);

    for (auto symbMapIter = symbMap.begin(); symbMap.end() != symbMapIter; ++symbMapIter)
        {
        if (symbMapIter == mostCommonSymb)
            continue;

        DrawSymbology (*xgContext, symbMapIter->first);

        for (Entry const* symbGraphics : symbMapIter->second)
            DrawEntry (xgContext->GetIDrawGeom(), *symbGraphics);
        }

    XGraphicsPublish::DeleteXGraphicsContext (xgContext);

    if (SUCCESS != container.AddToElement(eh))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    04/14
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphics::Finish()
    {
    if (m_isFinished)
        return SUCCESS;

    if (SUCCESS != SaveEntriesToElement())
        return ERROR;

    if (SUCCESS != m_element.GetDisplayHandler()->ValidateElementRange (m_element))
        return ERROR;

    m_isFinished = true;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
ElementId DgnGraphics::Save()
    {
    if (SUCCESS != Finish())
        return ElementId();

    ElementRefP             elRef;
    PersistentElementRefPtr persistentElRef;

    if (NULL == (elRef = m_element.GetElementRef()))
        {
        persistentElRef = m_element.GetDgnProject()->Models().GetElementById (m_element.GetElementId());
        elRef = persistentElRef.get();
        }

    StatusInt status = (NULL != elRef) ? m_element.ReplaceInModel (elRef) : m_element.AddToModel();

    return (SUCCESS == status) ? m_element.GetElementId() : ElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DgnGraphics::DgnGraphics() : m_sourceElement (NULL), m_isFinished (false) { }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
EditElementHandleR DgnGraphics::GetElementHandleR()
    {
    if (!m_element.IsValid() && m_sourceElement)
        {
        ElementHandle original (m_sourceElement);
        m_element.Duplicate (original);
        ExtendedElementHandler::InitializeElement (m_element, &original, *original.GetDgnModelP(), original.GetDgnModelP()->Is3d());
        m_element.GetElementDescrP()->ElementR().SetElementId(original.GetElementId());
        }

    return m_element;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus PhysicalGraphics::AddSolidPrimitive (ISolidPrimitiveCR solid)
    {
    return AddSolidPrimitive (solid, GetDefaultSymbology());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus PhysicalGraphics::AddSolidPrimitive (ISolidPrimitiveCR solid, SymbologyCR symb)
    {
    if (SolidPrimitiveType_None == solid.GetSolidPrimitiveType())
        return ERROR;

    IGeometryPtr geometry = IGeometry::Create (solid.Clone());
    m_entries.push_back (new DgnGraphics::Entry (geometry, symb));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus PhysicalGraphics::AddPolyface (PolyfaceQueryCR polyface)
    {
    return AddPolyface (polyface, GetDefaultSymbology());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus PhysicalGraphics::AddPolyface (PolyfaceQueryCR polyface, SymbologyCR symb)
    {
    PolyfaceHeaderPtr clone = PolyfaceHeader::New();
    clone->CopyFrom (polyface);
    IGeometryPtr geometry = IGeometry::Create (clone);
    m_entries.push_back (new DgnGraphics::Entry (geometry, symb));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus PhysicalGraphics::AddMSBsplineSurface (MSBsplineSurfaceCR surface)
    {
    return AddMSBsplineSurface (surface, GetDefaultSymbology());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus PhysicalGraphics::AddMSBsplineSurface (MSBsplineSurfaceCR surface, SymbologyCR symb)
    {
    MSBsplineSurfacePtr clone = MSBsplineSurface::CreatePtr();
    clone->CopyFrom (surface);
    IGeometryPtr geometry = IGeometry::Create (clone);
    m_entries.push_back (new DgnGraphics::Entry (geometry, symb));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphics::AddCurvePrimitive (ICurvePrimitiveCR curve)
    {
    return AddCurvePrimitive (curve, GetDefaultSymbology());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphics::AddCurvePrimitive (ICurvePrimitiveCR curve, SymbologyCR symb)
    {
    IGeometryPtr geometry = IGeometry::Create (curve.Clone());
    m_entries.push_back (new DgnGraphics::Entry (geometry, symb));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphics::AddCurveVector (CurveVectorCR curves)
    {
    return AddCurveVector (curves, GetDefaultSymbology());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphics::AddCurveVector (CurveVectorCR curves, SymbologyCR symb)
    {
    IGeometryPtr geometry = IGeometry::Create (curves.Clone());
    m_entries.push_back (new DgnGraphics::Entry (geometry, symb));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphics::AddTextString (TextStringCR text)
    {
    return AddTextString (text, GetDefaultSymbology());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphics::AddTextString (TextStringCR text, SymbologyCR symb)
    {
    TextStringPtr textPtr = TextString::Create (text.GetString().c_str(), &text.GetOrigin(), &text.GetRotMatrix(), text.GetProperties());
    m_entries.push_back (new DgnGraphics::Entry (textPtr.get(), symb));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DgnGraphics::const_iterator     DgnGraphics::begin() const     {return m_entries.begin();}
DgnGraphics::iterator           DgnGraphics::begin()           {return m_entries.begin();}
DgnGraphics::const_iterator     DgnGraphics::end() const       {return m_entries.end();}
DgnGraphics::iterator           DgnGraphics::end()             {return m_entries.end();}

size_t                          DgnGraphics::GetSize() const   {return m_entries.size();}
bool                            DgnGraphics::IsEmpty() const   {return m_entries.empty();}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DgnGraphics::iterator DgnGraphics::Erase (DgnGraphics::iterator pos)
    {
    return m_entries.erase (pos);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  06/14
//---------------------------------------------------------------------------------------
void DgnGraphics::TransformInPlace (TransformCR transform)
    {
    if (transform.IsIdentity ())
        return;

    for (EntryPtr& entry : m_entries)
        {
        if (!entry.IsValid ())
            continue;

        entry->TransformInPlace (transform);
        }
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    MattGooding     08/13
//=======================================================================================
struct DgnGraphicsProcessor : public IElementGraphicsProcessor
{
private:
    DgnGraphicsR            m_graphics;
    PhysicalGraphicsP       m_graphicsAsPhysical;   // same target - NULL for drawing graphics
    Symbology               m_currentSymbology;
    Transform               m_activeTransform;

    virtual bool            _ProcessAsBody (bool isCurved) const override {return false;} // NEEDSWORK: Have to support BReps!

    virtual BentleyStatus   _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override;
    virtual BentleyStatus   _ProcessSolidPrimitive (ISolidPrimitiveCR solid) override;
    virtual BentleyStatus   _ProcessSurface (MSBsplineSurfaceCR surface) override;
    virtual BentleyStatus   _ProcessFacets (PolyfaceQueryCR meshData, bool isFilled) override;
    virtual BentleyStatus   _ProcessTextString (TextStringCR text) override;

    virtual void            _AnnounceElemDisplayParams (ElemDisplayParamsCR displayParams) override;
    virtual void            _AnnounceTransform (TransformCP t) override {if (t) { m_activeTransform = *t; }}

public:
    DgnGraphicsProcessor (DgnGraphicsR graphics)
        :
        m_graphics (graphics),
        m_graphicsAsPhysical (dynamic_cast <PhysicalGraphics*> (&graphics)),
        m_currentSymbology (DgnGraphics::GetDefaultSymbology())
        {
        m_activeTransform.InitIdentity();
        }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
void DgnGraphicsProcessor::_AnnounceElemDisplayParams (ElemDisplayParamsCR displayParams)
    {
    m_currentSymbology = displayParams.m_symbology;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphicsProcessor::_ProcessTextString (TextStringCR text)
    {
    TextStringCP effectiveText = &text;
    TextStringPtr clone;
    if (!m_activeTransform.IsIdentity())
        {
        Transform textTransform = Transform::FromIdentity();
        text.GetDrawTransform (textTransform, false);
        textTransform.productOf (&textTransform, &m_activeTransform);

        DPoint3d textOrigin;
        RotMatrix textRotation;
        textTransform.GetTranslation (textOrigin);
        textTransform.GetMatrix (textRotation);
        clone = TextString::Create (text.GetString().c_str(), &textOrigin,
                                    &textRotation, text.GetProperties());
        effectiveText = clone.get();
        }

    m_graphics.AddTextString (*effectiveText, m_currentSymbology);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphicsProcessor::_ProcessCurveVector (CurveVectorCR curves, bool isFilled)
    {
    CurveVectorCP effectiveCurves = &curves;
    CurveVectorPtr clone;
    if (!m_activeTransform.IsIdentity())
        {
        clone = curves.Clone();
        clone->TransformInPlace (m_activeTransform);
        effectiveCurves = clone.get();
        }

    m_graphics.AddCurveVector (*effectiveCurves, m_currentSymbology);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphicsProcessor::_ProcessSolidPrimitive (ISolidPrimitiveCR solid)
    {
    if (!m_graphicsAsPhysical || SolidPrimitiveType_None == solid.GetSolidPrimitiveType())
        return SUCCESS;

    ISolidPrimitiveCP effectiveSolid = &solid;
    ISolidPrimitivePtr clone;
    if (!m_activeTransform.IsIdentity())
        {
        clone = solid.Clone();
        clone->TransformInPlace (m_activeTransform);
        effectiveSolid = clone.get();
        }

    m_graphicsAsPhysical->AddSolidPrimitive (*effectiveSolid, m_currentSymbology);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphicsProcessor::_ProcessSurface (MSBsplineSurfaceCR surface)
    {
    if (!m_graphicsAsPhysical)
        return SUCCESS;

    MSBsplineSurfaceCP effectiveSurface = &surface;
    MSBsplineSurfacePtr clone;
    if (!m_activeTransform.IsIdentity())
        {
        clone = MSBsplineSurface::CreatePtr();
        clone->CopyFrom (surface);
        clone->TransformSurface (m_activeTransform);
        effectiveSurface = clone.get();
        }

    m_graphicsAsPhysical->AddMSBsplineSurface (*effectiveSurface, m_currentSymbology);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphicsProcessor::_ProcessFacets (PolyfaceQueryCR polyface, bool isFilled)
    {
    if (!m_graphicsAsPhysical)
        return SUCCESS;

    PolyfaceQueryCP effectivePolyface = &polyface;
    PolyfaceHeaderPtr clone;
    if (!m_activeTransform.IsIdentity())
        {
        clone = PolyfaceHeader::New();
        clone->CopyFrom (polyface);
        clone->Transform (m_activeTransform);
        effectivePolyface = clone.get();
        }

    m_graphicsAsPhysical->AddPolyface (*effectivePolyface, m_currentSymbology);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
void DgnGraphics::Load (ElementHandleCR element)
    {
    DgnGraphicsProcessor processor (*this);
    ElementGraphicsOutput::Process (processor, element);

    m_element.Duplicate (element);
    ExtendedElementHandler::InitializeElement (m_element, &element, *element.GetDgnModelP(), element.GetDgnModelP()->Is3d());
    m_element.GetElementDescrP()->ElementR().SetElementId(element.GetElementId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     01/14
//---------------------------------------------------------------------------------------
void DgnGraphics::Load (ElementRefP element)
    {
    m_sourceElement = element;

    DgnGraphicsProcessor processor (*this);
    ElementGraphicsOutput::Process (processor, ElementHandle (element));
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
static BentleyStatus createXGraphicsElement (EditElementHandleR newElement, ElementHandleCR originalElement, XGraphicsContainerR container)
    {
    ExtendedElementHandler::InitializeElement (newElement, &originalElement, *originalElement.GetDgnModelP(), true);
    if (SUCCESS != container.AddToElement (newElement) || SUCCESS != newElement.GetDisplayHandler()->ValidateElementRange (newElement))
        return ERROR;

    // Some XGraphics streams contains operations with invalid ranges (2009_1182A-3DFP02I in Hospital).
    // These end up not being drawn in MicroStation - just throw them away when optimizing.
    if (!newElement.GetElementCP()->IsRangeValid3d())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/14
//---------------------------------------------------------------------------------------
static void splitElementsByRange (ElementAgendaR elements)
    {
    bool wasModified = false;

    ElementAgenda results;

    for (auto& iter : elements)
        {
        XGraphicsContainer container;

        if (SUCCESS != container.ExtractFromElement (iter))
            {
            results.Insert (iter);
            continue;
            }

        std::list <XGraphicsContainer> splitContainers;
        if (SUCCESS != container.SplitByRange (splitContainers, *iter.GetDgnModelP()))
            {
            results.Insert (iter);
            continue;
            }

        wasModified = true;

        for (auto& splitIter : splitContainers)
            {
            EditElementHandle eeh;
            if (SUCCESS == createXGraphicsElement (eeh, iter, splitIter))
                results.Insert (eeh);
            }
        }

    elements = results;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/13
//---------------------------------------------------------------------------------------
static void splitLargeMeshes (ElementAgendaR elements)
    {
    bool wasModified = false;

    ElementAgenda results;

    for (auto& iter : elements)
        {
        XGraphicsContainer container;
        if (SUCCESS != container.ExtractFromElement (iter))
            {
            results.Insert (iter);
            continue;
            }

        std::list <XGraphicsContainer> splitMeshes;
        if (SUCCESS != container.SplitMeshesByFaceCount (splitMeshes, 300))
            {
            results.Insert (iter);
            continue;
            }

        wasModified = true;

        for (auto& meshIter : splitMeshes)
            {
            EditElementHandle eeh;
            if (SUCCESS == createXGraphicsElement (eeh, iter, meshIter))
                results.Insert (eeh);
            }

        // These are the XGraphics left over when subdivided meshes are removed.
        if (!container.IsEmpty())
            {
            // Create new element with same symbology and new XGraphics - do this instead of
            // just replacing XGraphics to avoid carrying over XAttributes, EC properties, etc.
            // that should be processed differently.
            EditElementHandle eeh;
            if (SUCCESS == createXGraphicsElement (eeh, iter, container))
                results.Insert (eeh);
            }
        }

    elements = results;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/13
//---------------------------------------------------------------------------------------
static void splitLargeSurfaces (ElementAgendaR elements)
    {
    bool wasModified = false;

    ElementAgenda results;

    for (auto& iter : elements)
        {
        XGraphicsContainer container;
        if (SUCCESS != container.ExtractFromElement (iter))
            {
            results.Insert (iter);
            continue;
            }

        std::list <XGraphicsContainer> splitSurfaces;
        if (SUCCESS != container.SplitSurfacesByPoleCount (splitSurfaces, 10))
            {
            results.Insert (iter);
            continue;
            }

        wasModified = true;

        for (auto& surfaceIter : splitSurfaces)
            {
            EditElementHandle eeh;
            if (SUCCESS == createXGraphicsElement (eeh, iter, surfaceIter))
                results.Insert (eeh);
            }

        // These are the XGraphics left over when subdivided surfaces are removed.
        if (!container.IsEmpty())
            {
            // Create new element with same symbology and new XGraphics - do this instead of
            // just replacing XGraphics to avoid carrying over XAttributes, EC properties, etc.
            // that should be processed differently.
            EditElementHandle eeh;
            if (SUCCESS == createXGraphicsElement (eeh, iter, container))
                results.Insert (eeh);
            }
        }

    elements = results;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     05/13
//---------------------------------------------------------------------------------------
static void flattenSymbology (ElementAgendaR elements)
    {
    for (auto& iter : elements)
        {
        XGraphicsContainer container;

        if (SUCCESS != container.ExtractFromElement (iter) || SUCCESS != container.FlattenSymbology (iter))
            continue;
        
        container.ReplaceOnElement (iter);
        }
    }

#if defined (NEEDS_WORK_DGNITEM)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/14
//---------------------------------------------------------------------------------------
static DgnItemId getItemFromElement (ElementHandleCR eh)
    {
    if (NULL == eh.GetElementDescrCP())
        return DgnItemId();

    return eh.GetElementDescrCP()->GetItemId();
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/14
//---------------------------------------------------------------------------------------
BentleyStatus DgnGraphics::OptimizeAndSave (bvector <ElementId>& elementIds)
    {
#if defined (NEEDS_WORK_DGNITEM)
    DgnItemId itemId = getItemFromElement (m_element);
#endif

    // Convert DgnGraphics entries to XGraphics stored on header. This will also consolidate different
    // symbologies and place the most common symbology on the header.
    if (SUCCESS != SaveEntriesToElement())
        return ERROR;

    // Our list of optimized elements starts with the original element. Each split function may
    // modify this list.
    ElementAgenda elements;
    elements.Insert (m_element);

    splitElementsByRange (elements);
    splitLargeMeshes (elements);
    splitLargeSurfaces (elements);
    flattenSymbology (elements);

#if defined (NEEDS_WORK_DGNITEM)
    // We want to use our optimized element even if we didn't end up splitting it because we
    // may still have streamlined its symbology. No point in setting the OGRE handler or assembly
    // IDs on standalone elements, though.
    bool setOgreHandlers = elements.size() > 1;

    ElementId ogreLeaderId;

    for (auto& iter : elements)
        {
        bool isLeader = false;

        if (ogreLeaderId.IsValid())
            {
            OgreFollowerHandler::AttachToElement (iter);
            }

        if (setOgreHandlers && !ogreLeaderId.IsValid())
            {
            OgreLeadHandler::AttachToElement (iter);
            isLeader = true;
            }

        iter.GetElementDescrP()->SetItemId(itemId);

        if (SUCCESS != iter.GetDisplayHandler()->ValidateElementRange (iter) || SUCCESS != iter.AddToModel())
            {
            LOG.error (L"DgnGraphics::OptimizeAndSave resulted in invalid sub-element - some graphics may be missing.");
            continue;
            }

        elementIds.push_back (iter.GetElementId());

        if (setOgreHandlers && isLeader)
            ogreLeaderId = iter.GetElementId();
        }
#endif

    return elementIds.empty() ? ERROR : SUCCESS;
    }
