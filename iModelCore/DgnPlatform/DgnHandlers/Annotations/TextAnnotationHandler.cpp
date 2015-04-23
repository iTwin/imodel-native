//-------------------------------------------------------------------------------------- 
//     $Source: DgnHandlers/Annotations/TextAnnotationHandler.cpp $ 
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $ 
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatform/DgnHandlers/Annotations/TextAnnotationHandler.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationPersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/Annotations.fb.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

#if defined (NEEDSWORK_DGNITEM)
ELEMENTHANDLER_DEFINE_MEMBERS(TextAnnotationHandler);
#endif

#if defined (NEEDSWORK_DGNITEM)
static const uint16_t TEXT_ANNOTATION_SIGNATURE = 22854; // 0x5946; registered via http://toolsnet.bentley.com/Signature/Default.aspx on 2014-Apr-24.
#endif

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

#if defined (NEEDSWORK_DGNITEM)
static const uint8_t CURRENT_MAJOR_VERSION = 1;
static const uint8_t CURRENT_MINOR_VERSION = 0;
#endif

#if defined (NEEDS_WORK_DGNITEM)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
static BentleyStatus encodeElementDataAsFlatBuf(bvector<Byte>& buffer, TransformCR transform, DgnStyleId seedID)
    {
    FlatBufferBuilder encoder;

    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    FB::TextAnnotationElementBuilder fbElementData(encoder);
    fbElementData.add_majorVersion(CURRENT_MAJOR_VERSION);
    fbElementData.add_minorVersion(CURRENT_MINOR_VERSION);

    fbElementData.add_seedId(seedID.GetValue());
    fbElementData.add_transform(reinterpret_cast<FB::TextAnnotationElementTransform*>(const_cast<TransformP>(&transform)));

    auto fbElementDataOffset = fbElementData.Finish();

    //.............................................................................................
    encoder.Finish(fbElementDataOffset);
    buffer.resize(encoder.GetSize());
    memcpy(&buffer[0], encoder.GetBufferPointer(), encoder.GetSize());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
static BentleyStatus decodeElementDataFromFlatBuf(TransformP transform, DgnStyleId* seedID, ByteCP buffer, size_t numBytes)
    {
    if (NULL != transform)
        transform->InitIdentity();
    
    if (NULL != seedID)
        seedID->Invalidate();
    
    auto const& fbElementData = *GetRoot<FB::TextAnnotationElement>(buffer);
    
    PRECONDITION(fbElementData.has_majorVersion(), ERROR);
    if (fbElementData.majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    PRECONDITION(fbElementData.has_seedId(), ERROR);
    if (NULL != seedID)
        *seedID = DgnStyleId(fbElementData.seedId());

    PRECONDITION(fbElementData.has_transform(), ERROR);
    if (NULL != transform)
        *transform = *reinterpret_cast<TransformCP>(fbElementData.transform());

    return SUCCESS;
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
DgnClassId const& TextAnnotationHandler::GetHandlerId() { static const DgnClassId s_handlerID(TEXT_ANNOTATION_SIGNATURE, 0); return s_handlerID; }
static XAttributeHandlerId const& getElementDataXAID() { static const XAttributeHandlerId s_xaid(TEXT_ANNOTATION_SIGNATURE, 0); return s_xaid; }
static XAttributeHandlerId const& getAnnotationDataXAID() { static const XAttributeHandlerId s_xaid(TEXT_ANNOTATION_SIGNATURE, 1); return s_xaid; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2014
//---------------------------------------------------------------------------------------
void TextAnnotationHandler::_EditProperties(EditElementHandleR eh, PropertyContextR context)
    {
    // WIP
    T_Super::_EditProperties(eh, context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
void TextAnnotationHandler::_GetTypeName(Utf8StringR name, uint32_t desiredLength)
    {
    name = DgnHandlersMessage::GetString(DgnHandlersMessage::IDS_TYPENAMES_ANNOTATION_ELM);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
StatusInt TextAnnotationHandler::_OnTransform(EditElementHandleR eeh, TransformInfoCR tinfo)
    {
    PRECONDITION(NULL != eeh.GetDgnModelP(), ERROR);
    PRECONDITION(NULL != tinfo.GetTransform(), ERROR);

    auto& project = eeh.GetDgnModelP()->GetDgnDb();
    auto& transform = *tinfo.GetTransform();

    //.............................................................................................
    TextAnnotation annotation(project);
    Transform oldTransform;
    DgnStyleId seedID;
    POSTCONDITION(SUCCESS == TextAnnotationHandler::FromElement(&annotation, &oldTransform, &seedID, eeh), ERROR);

    //.............................................................................................
    // The goal here is to strip scaling from the transform, apply as a text scale, and apply the rest to the element.

    auto newTransform = Transform::FromProduct(transform, oldTransform);

    // TextString::TransformOrientationAndExtractScale will apply the Transform parameter to the Orientation parameter; so pass in the old orientation... it will become the new orientation.
    DPoint2d newScaleFactor;
    RotMatrix newOrientation; oldTransform.GetMatrix(newOrientation);
    TextString::TransformOrientationAndExtractScale(newScaleFactor, newOrientation, transform);

    newTransform.SetMatrix(newOrientation);

    if (NULL != annotation.GetTextP())
        {
        bool shouldScaleX = !DoubleOps::AlmostEqualFraction(newScaleFactor.x, 1.0);
        bool shouldScaleY = !DoubleOps::AlmostEqualFraction(newScaleFactor.y, 1.0);

        if (shouldScaleX || shouldScaleY)
            {
            auto& text = *annotation.GetTextP();
            for (auto& paragraph : text.GetParagraphs())
                {
                for (auto& run : paragraph->GetRuns())
                    {
                    auto runStyle = run->CreateEffectiveStyle();

                    if (shouldScaleY)
                        run->GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::Height, runStyle->GetHeight() * newScaleFactor.y);

                    if (shouldScaleX)
                        run->GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::WidthFactor, runStyle->GetWidthFactor() * (newScaleFactor.x / newScaleFactor.y));
                    }
                }

            if (text.GetDocumentWidth() > 0.0)
                text.SetDocumentWidth(text.GetDocumentWidth() * newScaleFactor.x);
            }
        }

    //.............................................................................................
    // By default, transform should also move the leader target points. Note that they are stored and drawn in world coordinates, so need to be transformed separately from the element's transform.
    for (auto& leader : annotation.GetLeaders())
        {
        if (AnnotationLeaderTargetAttachmentType::PhysicalPoint != leader->GetTargetAttachmentType())
            continue;

        auto const* worldPtP = leader->GetTargetAttachmentDataForPhysicalPoint();
        if (UNEXPECTED_CONDITION(NULL == worldPtP))
            continue;

        auto worldPt = *worldPtP;
        transform.Multiply(worldPt);

        leader->SetTargetAttachmentDataForPhysicalPoint(&worldPt);
        }

    //.............................................................................................
    return (StatusInt)UpdateElement(eeh, annotation, newTransform, seedID);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2014
//---------------------------------------------------------------------------------------
void TextAnnotationHandler::_QueryProperties(ElementHandleCR eh, PropertyContextR context)
    {
    // WIP
    T_Super::_QueryProperties(eh, context);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
bool TextAnnotationHandler::_IsPlanar(ElementHandleCR eh, DVec3dP normal, DPoint3dP pointOnPlane, DVec3dCP defaultNormal)
    {
    Transform transform;
    if (UNEXPECTED_CONDITION(SUCCESS != TextAnnotationHandler::FromElement(NULL, &transform, NULL, eh)))
        return T_Super::_IsPlanar(eh, normal, pointOnPlane, defaultNormal);

    if (NULL != normal)
        transform.GetMatrixColumn(*normal, 2);

    if (NULL != pointOnPlane)
        transform.GetTranslation(*pointOnPlane);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
void TextAnnotationHandler::_GetOrientation(ElementHandleCR eh, RotMatrixR orientation)
    {
    Transform transform;
    if (UNEXPECTED_CONDITION(SUCCESS != TextAnnotationHandler::FromElement(NULL, &transform, NULL, eh)))
        {
        T_Super::_GetOrientation(eh, orientation);
        return;
        }

    transform.GetMatrix(orientation);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2014
//---------------------------------------------------------------------------------------
void TextAnnotationHandler::_GetSnapOrigin(ElementHandleCR eh, DPoint3dR pt)
    {
    _GetTransformOrigin(eh, pt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
void TextAnnotationHandler::_GetTransformOrigin(ElementHandleCR eh, DPoint3dR pt)
    {
    Transform transform;
    if (UNEXPECTED_CONDITION(SUCCESS != TextAnnotationHandler::FromElement(NULL, &transform, NULL, eh)))
        {
        T_Super::_GetTransformOrigin(eh, pt);
        return;
        }
    
    transform.GetTranslation(pt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationHandler::UpdateElement(EditElementHandleR eeh, TextAnnotationCR annotation, TransformCR transform, DgnStyleId seedID)
    {
#if defined (NEEDS_WORK_DGNITEM)
    PRECONDITION(&eeh.GetHandler() == &ELEMENTHANDLER_INSTANCE(TextAnnotationHandler), ERROR);
    PRECONDITION(NULL != eeh.GetDgnModelP(), ERROR);

    auto& model = *eeh.GetDgnModelP();

    // Write the graphics as XGraphics.
    TextAnnotationDraw draw(annotation);
    draw.SetDocumentTransform(transform);

    XGraphicsRecorder graphics(&model);
    PRECONDITION(NULL != graphics.GetContext(), ERROR);
    
    // HACK
    // We have to do this because the first ActivateMatSymb call is normally ignored.
    // When drawing from an element, VisitElement normally sets up the ElemMatSymb from the element header, and thus the first activate is purposefully ignored because it's assumed to simply be "re"activating the same symbology.
    // However, we're not visiting an element here... so we need an initial do-nothing activate to trick the optimization, just so the real ones will work.
    graphics.GetContext()->GetIDrawGeom().ActivateMatSymb(graphics.GetContext()->GetElemMatSymb());

    POSTCONDITION(SUCCESS == draw.Draw(*graphics.GetContext()), ERROR);
    
    POSTCONDITION(SUCCESS == graphics.GetContainer().AddToElement(eeh), ERROR);
    POSTCONDITION(SUCCESS == eeh.GetDisplayHandler()->ValidateElementRange(eeh.GetGraphicsP(), &model), ERROR);

    // Write element data to an XAttribute.
    bvector<Byte> elementDataBuffer;
    POSTCONDITION(SUCCESS == encodeElementDataAsFlatBuf(elementDataBuffer, transform, seedID), ERROR);
    POSTCONDITION(SUCCESS == eeh.ScheduleWriteXAttribute(getElementDataXAID(), 0, elementDataBuffer.size(), &elementDataBuffer[0]), ERROR);

    // Write annotation data to an XAttribute.
    bvector<Byte> annotationBuffer;
    POSTCONDITION(SUCCESS == TextAnnotationPersistence::EncodeAsFlatBuf(annotationBuffer, annotation), ERROR);
    POSTCONDITION(SUCCESS == eeh.ScheduleWriteXAttribute(getAnnotationDataXAID(), 0, annotationBuffer.size(), &annotationBuffer[0]), ERROR);

    return SUCCESS;
#else
    return ERROR;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationHandler::CreateElement(EditElementHandleR eeh, ElementHandleCP templateEh, TextAnnotationCR annotation, DgnModelR model, TransformCR transform, DgnStyleId seedID)
    {
    T_Super::InitializeElement(eeh, model, templateEh->GetGraphicsCP()->GetCategory());
    eeh.GetWriteableElement()->SetElementHandler(&ELEMENTHANDLER_INSTANCE(TextAnnotationHandler));

    return UpdateElement(eeh, annotation, transform, seedID);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationHandler::FromElement(TextAnnotationP annotation, TransformP transform, DgnStyleId* seedID, ElementHandleCR eh)
    {
    // Read element data from the XAttribute.
    if ((NULL != transform) || (NULL != seedID))
        {
        ElementHandle::XAttributeIter elementDataXAttr(eh, getElementDataXAID(), 0);
        PRECONDITION(elementDataXAttr.IsValid(), ERROR);
        PRECONDITION(elementDataXAttr.GetSize() > 0, ERROR);
        POSTCONDITION(SUCCESS == decodeElementDataFromFlatBuf(transform, seedID, (ByteCP)elementDataXAttr.PeekData(), elementDataXAttr.GetSize()), ERROR);
        }

    // Read annotation data from the XAttribute.
    if (NULL != annotation)
        {
        ElementHandle::XAttributeIter annotationDataXAttr(eh, getAnnotationDataXAID(), 0);
        PRECONDITION(annotationDataXAttr.IsValid(), ERROR);
        PRECONDITION(annotationDataXAttr.GetSize() > 0, ERROR);
        POSTCONDITION(SUCCESS == TextAnnotationPersistence::DecodeFromFlatBuf(*annotation, (ByteCP)annotationDataXAttr.PeekData(), annotationDataXAttr.GetSize()), ERROR);
        }

    return SUCCESS;
    }
#endif

