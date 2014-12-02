/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementGraphics.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <BentleyGeomFlatBuffer/BentleyGeomFlatBufferApi.h>

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  06/09
+===============+===============+===============+===============+===============+======*/
struct ElementGraphicsDrawGeom : public SimplifyViewDrawGeom
{
    DEFINE_T_SUPER(SimplifyViewDrawGeom)
private:

IElementGraphicsProcessor*  m_dropObj;

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _DoClipping () const override {return m_dropObj->_WantClipping ();}
virtual bool _DoTextGeometry () const override {return true;}
virtual bool _DoSymbolGeometry () const override {return true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IFacetOptionsP _GetFacetOptions() override
    {
    IFacetOptionsP dropOptions = m_dropObj->_GetFacetOptionsP ();

    if (NULL == dropOptions)
        dropOptions = T_Super::_GetFacetOptions ();

    return dropOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessAsFacets (bool isPolyface) const override {return m_dropObj->_ProcessAsFacets (isPolyface);}
virtual bool _ProcessAsBody (bool isCurved) const override {return m_dropObj->_ProcessAsBody (isCurved);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnounceCurrentState ()
    {
    ElemMatSymb         currentMatSymb;
    m_dropObj->_AnnounceTransform (m_context->GetCurrLocalToFrustumTransformCP());
    GetCurrentMatSymb (currentMatSymb);
    m_dropObj->_AnnounceElemMatSymb (currentMatSymb);
    m_dropObj->_AnnounceElemDisplayParams (*m_context->GetCurrentDisplayParams ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessCurvePrimitive (ICurvePrimitiveCR curve, bool isClosed, bool isFilled) override
    {
    AnnounceCurrentState ();

    return m_dropObj->_ProcessCurvePrimitive (curve, isClosed, isFilled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override
    {
    AnnounceCurrentState ();

    return m_dropObj->_ProcessCurveVector (curves, isFilled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessSolidPrimitive (ISolidPrimitiveCR primitive) override
    {
    AnnounceCurrentState ();

    return m_dropObj->_ProcessSolidPrimitive (primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessSurface (MSBsplineSurfaceCR surface) override
    {
    AnnounceCurrentState ();

    return m_dropObj->_ProcessSurface (surface);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessFacetSet (PolyfaceQueryCR facets, bool filled) override
    {
    AnnounceCurrentState ();

    return m_dropObj->_ProcessFacets (facets, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP attachments) override
    {
    AnnounceCurrentState ();

    return m_dropObj->_ProcessBody (entity, attachments);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawTextString (TextStringCR text, double* zDepth) override
    {
    AnnounceCurrentState ();

    if (SUCCESS == m_dropObj->_ProcessTextString (text))
        return;

    T_Super::_DrawTextString (text, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ModifyDrawViewFlags (ViewFlagsR flags)
    {
    // Prefer "higher level" geometry from legacy types...
    flags.SetRenderMode (MSRenderMode::SmoothShade);

    // Make sure linestyles drawn for drop...esp. when dropping linestyles!
    flags.inhibitLineStyles = false;

    // Make sure to display fill so that fill/gradient can be added to output...
    flags.fill = true;

    // Make sure to display patterns so that patterns can be added to output...
    flags.patterns = true;

    // Don't draw text node number/cross when dropping text!
    flags.text_nodes = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _SetDrawViewFlags (ViewFlagsCP flags) override
    {
    T_Super::_SetDrawViewFlags (flags);

    ModifyDrawViewFlags (m_viewFlags);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void Init (ViewContextP context, IElementGraphicsProcessor* dropObj)
    {
    SetViewContext (context);
    m_dropObj = dropObj;
    ModifyDrawViewFlags (m_viewFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
IElementGraphicsProcessor* GetIElementGraphicsProcessor () {return m_dropObj;}

}; // ElementGraphicsDrawGeom

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  06/09
+===============+===============+===============+===============+===============+======*/
struct ElementGraphicsContext : public NullContext
{
    DEFINE_T_SUPER(NullContext)
protected:

ElementGraphicsDrawGeom&    m_output;
DgnModelP                m_targetModel;
bool                        m_wantUndisplayed;

virtual bool                _WantUndisplayed () { return m_wantUndisplayed; }      // Temporary for SmartFeatures - Add option to graphics processor to collect undisplayed??

virtual void _SetupOutputs () override {SetIViewDraw (m_output);}

public:

ElementGraphicsContext (IElementGraphicsProcessor* dropObj, ElementGraphicsDrawGeom& output, bool wantUndisplayed = false) : m_output (output), m_wantUndisplayed (wantUndisplayed)
    {
    m_purpose = dropObj->_GetDrawPurpose ();
    m_wantMaterials = true; // Setup material in ElemDisplayParams in case IElementGraphicsProcessor needs it...
    m_targetModel = NULL;

    SetBlockAsynchs (true);
    m_output.Init (this, dropObj);
    _SetupOutputs ();

    dropObj->_AnnounceContext (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawSymbol (IDisplaySymbolP symbolDef, TransformCP trans, ClipPlaneSetP clipPlanes, bool ignoreColor, bool ignoreWeight) override
    {
    // Pass along any symbol that is drawn from _ExpandPatterns/_ExpandLineStyles, etc.
    m_output.ClipAndProcessSymbol (symbolDef, trans, clipPlanes, ignoreColor, ignoreWeight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawAreaPattern (ElementHandleCR thisElm, ClipStencil& boundary, PatternParamSource& source) override
    {
    if (!m_output.GetIElementGraphicsProcessor ()->_ExpandPatterns ())
        return;

    T_Super::_DrawAreaPattern (thisElm, boundary, source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ILineStyleCP _GetCurrLineStyle (LineStyleSymbP* symb) override
    {
    ILineStyleCP  currStyle = T_Super::_GetCurrLineStyle (symb);

    if (!m_output.GetIElementGraphicsProcessor ()->_ExpandLineStyles (currStyle))
        return NULL;

    return currStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _CookDisplayParams (ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb) override
    {
    // Apply ignores, resolve effective, and cook ElemMatSymb...
    elParams.Resolve (*this);
    elMatSymb.FromResolvedElemDisplayParams (elParams, *this, m_startTangent, m_endTangent);
    }

}; // ElementGraphicsContext

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    John.Gooding    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphicsOutput::Process (IElementGraphicsProcessorR dropObj, IDisplaySymbolR dispSymbol, DgnProjectR project)
    {
    ElementGraphicsDrawGeom output;
    ElementGraphicsContext  context (&dropObj, output);

    context.GetCurrentDisplayParams()->Init ();
    context.SetDgnProject (project);

    dispSymbol._Draw (context);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphicsOutput::Process (IElementGraphicsProcessorR dropObj, ElementHandleCR eh)
    {
    ElementGraphicsDrawGeom output;
    ElementGraphicsContext  context (&dropObj, output);

    context.SetDgnProject (*eh.GetDgnProject ());

    context.VisitElemHandle (eh, false, false);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphicsOutput::Process (IElementGraphicsProcessorR dropObj, DgnProjectR project)
    {
    ElementGraphicsDrawGeom output;
    ElementGraphicsContext  context (&dropObj, output);

    context.GetCurrentDisplayParams()->Init ();
    context.SetDgnProject (project);

    dropObj._OutputGraphics (context);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  09/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphicsOutput::Process (IElementGraphicsProcessorR dropObj)
    {
    ElementGraphicsDrawGeom output;
    ElementGraphicsContext  context (&dropObj, output);

    context.GetCurrentDisplayParams()->Init ();
    dropObj._OutputGraphics (context); // Processor is expected to setup DgnProject in _OutputGraphics...
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphicsOutput::DebugXGraphics (ElementHandleCR eh)
    {
    XGraphicsContainer container;

    if (SUCCESS == container.ExtractFromElement (eh))
        {
        container.Dump (eh.GetDgnModelP ());
        }

#if defined (NEEDS_WORK_DGNITEM)
    ChildElemIter     childEh;
    ISharedCellQuery* scQuery;

    if (NULL != (scQuery = dynamic_cast <ISharedCellQuery*> (&eh.GetHandler ())) && scQuery->IsSharedCell (eh))
        childEh = ChildElemIter (ElementHandle (scQuery->GetDefinition (eh, *eh.GetDgnProject ())));
    else
        childEh = ChildElemIter (eh);

    for (; childEh.IsValid (); childEh = childEh.ToNext ())
        ElementGraphicsOutput::DebugXGraphics (childEh);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphics::Iterator::ToNext ()
    {
    if (m_dataOffset >= m_totalDataSize)
        {
        m_data = NULL;
        m_dataOffset = 0;

        return;
        }

    UInt32          opCode = *((UInt32 *) (m_data));
    UInt32          dataSize = *((UInt32 *) (m_data + sizeof (opCode)));
    UInt8 const*    data = (0 != dataSize ? (UInt8 const*) (m_data + sizeof (opCode) + sizeof (dataSize)) : NULL);
    size_t          xgOpSize = sizeof (opCode) + sizeof (dataSize) + dataSize;

    m_xgOp = Operation ((OpCode) (opCode), dataSize, data);
    m_data += xgOpSize;
    m_dataOffset += xgOpSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphics::Writer::Append (Operation const& xgOp)
    {
    UInt32  paddedDataSize = (xgOp.m_dataSize + 7) & ~7; // 8 byte aligned...
    size_t  xgOpSize = sizeof (xgOp.m_opCode) + sizeof (xgOp.m_dataSize) + paddedDataSize;
    size_t  currSize = m_buffer.size ();

    m_buffer.resize (currSize + xgOpSize);

    UInt8*  currOffset = &(m_buffer.at (currSize));

    memcpy (currOffset, &xgOp.m_opCode, sizeof (xgOp.m_opCode));
    currOffset += sizeof (xgOp.m_opCode);

    memcpy (currOffset, &paddedDataSize, sizeof (paddedDataSize));
    currOffset += sizeof (paddedDataSize);

    if (0 == xgOp.m_dataSize)
        return;
            
    memcpy (currOffset, xgOp.m_data, xgOp.m_dataSize);
    currOffset += xgOp.m_dataSize;

    if (paddedDataSize > xgOp.m_dataSize)
        memset (currOffset, 0, paddedDataSize - xgOp.m_dataSize); // Pad quietly or also assert?
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphics::Writer::Append (IGeometryCR geom)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes (geom, buffer);

    if (0 == buffer.size ())
        {
        BeAssert (false);
        return;
        }

    Append (Operation (OpCode::IGeometry, (UInt32) buffer.size (), &buffer.front ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ElementGraphics::Reader::Get (Operation const& xgOp)
    {
    if (OpCode::IGeometry != xgOp.m_opCode)
        return NULL;

    return BentleyGeometryFlatBuffer::BytesToGeometry (xgOp.m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphics::Collection::Draw (ViewContextR context) const
    {
    for (auto const& egOp : *this)
        {
        switch (egOp.m_opCode)
            {
            case ElementGraphics::OpCode::Header:
                {
                // Verify that this is first opcode or something...
                break;
                }

            case ElementGraphics::OpCode::PushTransform:
                {
                context.PushTransform (*ElementGraphics::Reader::GetTransform (egOp));
                break;
                }

            case ElementGraphics::OpCode::PopTransform:
                {
                context.PopTransformClip ();
                break;
                }

            case ElementGraphics::OpCode::IGeometry:
                {
                IGeometryPtr geomPtr = ElementGraphics::Reader::Get (egOp);

                if (!geomPtr.IsValid ())
                    break;

                switch (geomPtr->GetGeometryType ())
                    {
                    case IGeometry::GeometryType::CurveVector:
                        {
                        CurveVectorPtr curvePtr = geomPtr->GetAsCurveVector ();

                        context.GetIDrawGeom ().DrawCurveVector (*curvePtr, false);
                        break;
                        }

                    case IGeometry::GeometryType::SolidPrimitive:
                        {
                        ISolidPrimitivePtr solidPtr = geomPtr->GetAsISolidPrimitive ();

                        context.GetIDrawGeom ().DrawSolidPrimitive (*solidPtr);
                        break;
                        }

                    case IGeometry::GeometryType::BsplineSurface:
                        {
                        MSBsplineSurfacePtr surfacePtr = geomPtr->GetAsMSBsplineSurface ();

                        context.GetIDrawGeom ().DrawBSplineSurface (*surfacePtr);
                        break;
                        }

                    case IGeometry::GeometryType::Polyface:
                        {
                        PolyfaceHeaderPtr polyfacePtr = geomPtr->GetAsPolyfaceHeader ();

                        context.GetIDrawGeom ().DrawPolyface (*polyfacePtr, false);
                        break;
                        }
                    }

                break;
                }
            }
        }
    }

