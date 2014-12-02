/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Raster/RasterAttachment.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

static Utf8CP RASTERSOURCEXAttr_Url       = "url";        

/*---------------------------------------------------------------------------------------
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/_/_/_/_/_/_/_/_/_/ RasterXAttrStringFacilityerElem  _/_/_/_/_/_/_/_/_/_/
/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// Helper class for XAttribute string.
---------------------------------------------------------------------------------------*/
struct RasterXAttrStringFacility
    {
    /*---------------------------------------------------------------------------------**//**
    * Extract a raster XAttribute string in is native format(WideString)
    * @bsimethod                                                    MarcBedard  12/2006
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BentleyStatus GetString(WStringR outString, ElementHandleCR elemHandle, RasterFrameXAttributesMinorId stringID)
        {
        // Ensure we have a valid XAttribute iterator.
        ElementHandle::XAttributeIter xAttrItr(elemHandle, XAttributeHandlerId(XATTRIBUTEID_RasterFrame, stringID));
        if (!xAttrItr.IsValid() || !xAttrItr.GetSize ())
            {
            outString.clear();
            return ERROR;
            }

        // Extract WString directly.
        DataInternalizer dataInternalizer((byte*)xAttrItr.PeekData(), xAttrItr.GetSize());
        dataInternalizer.get(outString);

        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * Set a raster XAttribute string in is native format(WideString)
    * @bsimethod                                                    MarcBedard  12/2006
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BentleyStatus SetString(WCharCP inString, EditElementHandleR editElemHandle, RasterFrameXAttributesMinorId stringID)
        {
        DataExternalizer sink;
        sink.put(inString);

        // Tell the element handle to add this XAttribute when rewrite is called.
        return (SUCCESS == editElemHandle.ScheduleWriteXAttribute (XAttributeHandlerId(XATTRIBUTEID_RasterFrame, stringID), 0, sink.getBytesWritten(), sink.getBuf())) ? SUCCESS : ERROR;
        }
    };  // End class RasterXAttrStringFacility

/*---------------------------------------------------------------------------------------
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/_/_/_/_/_/_/_/_/_/ RasterXAttrSourceURLFacility  _/_/_/_/_/_/_/_/_/_/
/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// Helper class for raster source url XAttribute.
// Format:
//      url=[WChar*]
---------------------------------------------------------------------------------------*/
struct RasterXAttrSourceURLFacility
    {

    /*---------------------------------------------------------------------------------**//**
    * Extract a raster XAttribute source url.
    * @bsimethod                                                    MarcBedard  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    static WString GetRasterSourceUrl(ElementHandleCR elemHandle)
        {
        // Ensure we have a valid XAttribute iterator.
        ElementHandle::XAttributeIter xAttrItr(elemHandle, XAttributeHandlerId(XATTRIBUTEID_RasterFrame, SOURCEMONIKER_XATTR_ID));
        if (!xAttrItr.IsValid() || !xAttrItr.GetSize ())
            return WString();

        DataInternalizer dataInternalizer((byte*)xAttrItr.PeekData(), xAttrItr.GetSize());


        WString externalizedState;
        dataInternalizer.get (externalizedState);

        WString url;
        Json::Value         jsonObj (Json::objectValue);
        if (!Json::Reader::Parse(Utf8String(externalizedState), jsonObj))
            return url; //empty string

        Utf8String  urlUtf8 = jsonObj[RASTERSOURCEXAttr_Url].asString ();

        url.AssignUtf8(urlUtf8.c_str());

        return url;
        }

    /*---------------------------------------------------------------------------------**//**
    * Store a raster XAttribute source url.
    * @bsimethod                                                    MarcBedard  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BentleyStatus SetRasterSourceUrl(EditElementHandleR elemHandle, WCharCP url)
        {
        Json::Value outValue;

        outValue[RASTERSOURCEXAttr_Url] = Utf8String(url);
        Utf8String  externalizedStateUtf8 = Json::FastWriter::ToString(outValue);  

        WString externalizedState;
        externalizedState.AssignUtf8(externalizedStateUtf8.c_str());
        DataExternalizer sink;
        sink.put(externalizedState.c_str());

        // Tell the element handle to add this XAttribute when rewrite is called.
        return (SUCCESS == elemHandle.ScheduleWriteXAttribute (XAttributeHandlerId(XATTRIBUTEID_RasterFrame, SOURCEMONIKER_XATTR_ID), 0, sink.getBytesWritten(), sink.getBuf())) ? BSISUCCESS : BSIERROR;
        }

    };  // End class RasterXAttrSourceURLFacility



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  12/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::IsValidTransform(TransformCR matrix)
    {
    return RasterTransformFacility::IsValidRasterTransform(matrix);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  9/2002
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::IsTransform3D (TransformCR matrix)
    {
    return (RasterTransformFacility::Has3dRotation(matrix));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  RasterFrameHandler::ColorIndexFromRgbInModel (UInt32& index, DgnModelR modelRef, RgbColorDef const& rgbColor)
    {
    DgnColorMapCP colorMap = DgnColorMap::Get (modelRef.GetDgnProject());

    if (!colorMap)
        return ERROR;

    IntColorDef inputColor(rgbColor);
    index = colorMap->FindClosestMatch(inputColor,NULL);

    UInt32      elementColor;
    if (INVALID_COLOR == (elementColor = modelRef.GetDgnProject().Colors().CreateElementColor (inputColor, NULL, NULL)))
        return ERROR;

    index |= (ColorUtil::GetExtendedIndexFromRawColor (elementColor) << 8);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  RasterFrameHandler::RgbFromColorIndexInModel (RgbColorDef& color, DgnProjectR project, UInt32 elementColor)
    {
    IntColorDef colorDef;

    // Continue to support wacky behavior of passing -1 to return background color...
    if (-1 == elementColor)
        elementColor = DgnColorMap::INDEX_Background;

    if (SUCCESS != project.Colors().Extract (&colorDef, NULL, NULL, NULL, NULL, elementColor))
        return ERROR;

    memcpy (&color, &colorDef.m_rgb, sizeof (colorDef.m_rgb));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RasterFrameHandler::CreateRasterAttachment 
(
EditElementHandleR      eeh, 
ElementHandleCP         templateEh, 
WCharCP                 sourceURL,
DPoint3dCR              origin, 
DVec3dCR                Uvect, 
DVec3dCR                Vvect,
DgnModelR            modelRef
)
    {
    DgnElementCP     in = (NULL != templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank       out;
    RasterFrameElm* rasterElmP = reinterpret_cast<RasterFrameElm*>(&out);

    if (NULL != in && RASTER_FRAME_ELM == in->GetLegacyType() && reinterpret_cast<RasterFrameElm const*>(in)->version >= V10_RASTER_FRAME_FIRSTVERSION)
        {
        memcpy (&out, in, in->Size());
        ElementUtil::SetRequiredFields (out, RASTER_FRAME_ELM, in->GetLevel(), false, modelRef.Is3d() ? ElementUtil::ELEMDIM_3d : ElementUtil::ELEMDIM_2d);
        }
    else
        {
        LevelId levelId = LEVEL_DEFAULT_LEVEL_ID;

        memset (&out, 0, sizeof(RasterFrameElm));
        
        // If the template was of the wrong type, only take some select data from it.
        if(NULL != in)
            {
            levelId = in->GetLevel();
            if (in->IsGraphic())
                memcpy (&out, in, sizeof(DgnElement));
            }
        ElementUtil::SetRequiredFields (out, RASTER_FRAME_ELM, levelId, false, modelRef.Is3d() ? ElementUtil::ELEMDIM_3d : ElementUtil::ELEMDIM_2d);

        RgbColorDef       white = {255,255,255};
        RgbColorDef       black = {0,0,0};
        UInt32            rawWhite(0);
        UInt32            rawBlack(DgnColorMap::INDEX_Background);
        ColorIndexFromRgbInModel(rawWhite,modelRef,white);
        ColorIndexFromRgbInModel(rawBlack,modelRef,black);

        // Init non zero default values.
        rasterElmP->version = V10_RASTER_FRAME_FIRSTVERSION;
        rasterElmP->views = 0xFF; //All on by default
        rasterElmP->flags.plot = true;
        rasterElmP->foregroundColor = rawWhite;
        rasterElmP->backgroundColor = rawBlack;
        rasterElmP->uVector.init(1,0,0);
        rasterElmP->vVector.init(0,1,0);
        rasterElmP->origin.x = 0;
        rasterElmP->origin.y = 0;
        rasterElmP->origin.z = 0;
        }

    rasterElmP->version = V10_RASTER_FRAME_FIRSTVERSION;
    out.SetSizeWordsNoAttributes(sizeof(RasterFrameElm)/2);

    // Copy linkages.
    if(NULL != in)
        ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr(new MSElementDescr(out,modelRef), false);

    if(NULL == in)
        T_HOST.GetRasterAttachmentAdmin()._ApplyDefaultSettings(eeh);

    RasterFrameHandler* pRasterFrameHandler = dynamic_cast<RasterFrameHandler*>(&eeh.GetHandler());

    pRasterFrameHandler->SetSourceUrl(eeh,sourceURL);

    pRasterFrameHandler->SetOrigin(eeh,origin,false);
    pRasterFrameHandler->SetU(eeh,Uvect,false);
    if (pRasterFrameHandler->SetV(eeh,Vvect,true))
        return BSISUCCESS;

   
    eeh.Invalidate();
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetDisplayBorderState(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->flags.displayBorder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetSnappableState(ElementHandleCR eh) const
    {
    return !GetFrameElmCP(eh)->IsSnappable();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetLockedState(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->IsLocked();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetViewIndependentState(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->IsViewIndependent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 RasterFrameHandler::GetVersion(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->version;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetOpenReadWrite(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->flags.openedReadWrite;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d   RasterFrameHandler::GetU(ElementHandleCR eh) const
    {
    return DVec3d::From(GetFrameElmCP(eh)->uVector);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d   RasterFrameHandler::GetV(ElementHandleCR eh) const
    {
    return DVec3d::From(GetFrameElmCP(eh)->vVector);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d RasterFrameHandler::GetOrigin(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->origin;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d RasterFrameHandler::GetExtent(ElementHandleCR eh) const
    {
    // Raster size in UORs
    DPoint2d extent;

    DVec3d uVector, vVector;
    uVector  = DVec3d::From(GetFrameElmCP(eh)->uVector); 
    vVector  = DVec3d::From(GetFrameElmCP(eh)->vVector);

    extent.x = uVector.Magnitude();
    extent.y = vVector.Magnitude();
    return extent;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d RasterFrameHandler::GetScanningResolution(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->scanningResolution;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetViewState(ElementHandleCR eh,int viewNumber) const
    {
    if (viewNumber < 0 || viewNumber>8)
        return true;
    return TO_BOOL(GetFrameElmCP(eh)->views & (0x01 << viewNumber));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetInvertState(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->flags.invert;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetPrintState(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->flags.plot;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetClipState(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->flags.clipping;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetTransparencyState(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->flags.transparency;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::GetBinaryPrintInvertState(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->flags.binaryPlotInvert;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
long RasterFrameHandler::GetDisplayOrder(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->displayOrder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 RasterFrameHandler::GetForegroundColor(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->foregroundColor;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 RasterFrameHandler::GetBackgroundColor(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->backgroundColor;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt8 RasterFrameHandler::GetForegroundTransparencyLevel(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->foregroundTransparency;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt8 RasterFrameHandler::GetBackgroundTransparencyLevel(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->backgroundTransparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt8 RasterFrameHandler::GetImageTransparencyLevel(ElementHandleCR eh) const
    {
    return GetFrameElmCP(eh)->imageTransparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BarryBentley                    05/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt getOriginalFileName (WString& fileName, DgnModelP modelRef)
    {
    fileName = modelRef->GetDgnProject().GetFileName ();
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString RasterFrameHandler::GetSearchPath (DgnModelP modelRef)
    {
    WString result;

    // *** Try in the dgn folder
    if (modelRef!= NULL)
        {
        WString     parentFileName;

        // NOTE: We use getOriginalFileName here rather than mdlDgnModel_getFileName, because the two
        // are different (only) during a Save As. We're trying to find the references relative to the original
        // file name.
        if (SUCCESS == getOriginalFileName (parentFileName, modelRef))
            {
            BeFileName path (BeFileName::DevAndDir, parentFileName.c_str());
            result += path.GetName();
            result += L";";
            }
        }

    // *** Try in MS_RFDIR
    result += WString(L"MS_RFDIR");
    result += L";";

    return result;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString RasterFrameHandler::GetSourceUrl(ElementHandleCR eh) const
    {
    return RasterXAttrSourceURLFacility::GetRasterSourceUrl(eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
WString RasterFrameHandler::GetAttachDescription(ElementHandleCR eh) const
    {
    Bentley::WString description;
    RasterXAttrStringFacility::GetString(description, eh, DESCRIPTION_XATTR_ID);
    return description;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
WString RasterFrameHandler::GetLogicalName(ElementHandleCR eh) const
    {
    Bentley::WString LogicalName;
    RasterXAttrStringFacility::GetString(LogicalName, eh, LOGICALNAME_XATTR_ID);
    return LogicalName;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetDisplayBorderState(EditElementHandleR eeh, bool state)
    {
    GetFrameElmP(eeh)->flags.displayBorder = state;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetSnappableState(EditElementHandleR eeh, bool state)
    {
    GetFrameElmP(eeh)->SetSnappable(!state);// props.b.s == false >>> snappable
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetLockedState(EditElementHandleR eeh, bool state)
    {
    GetFrameElmP(eeh)->SetLocked(state);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetViewIndependentState(EditElementHandleR eeh, bool state, bool reValidateRange)
    {
    GetFrameElmP(eeh)->SetViewIndependent(state);

    if (reValidateRange)
        {
        return (SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh));
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetOpenReadWrite(EditElementHandleR eeh, bool isWritable)
    {
    GetFrameElmP(eeh)->flags.openedReadWrite=isWritable;
    return true;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetU(EditElementHandleR eeh, DVec3dCR uVector, bool reValidateRange)
    {
    GetFrameElmP(eeh)->uVector=uVector;
    if (reValidateRange)
        {
        return (SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh));
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetV(EditElementHandleR eeh, DVec3dCR vVectov, bool reValidateRange)
    {
    GetFrameElmP(eeh)->vVector=vVectov;
    if (reValidateRange)
        {
        return (SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh));
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetOrigin(EditElementHandleR eeh, DPoint3dCR origin, bool reValidateRange) 
    {
    GetFrameElmP(eeh)->origin=origin;
    if (reValidateRange)
        {
        return (SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh));
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetExtent(EditElementHandleR eeh,DPoint2dCR extentInUOR, bool reValidateRange)
    {
    DVec3d uVector, vVector;
    uVector = DVec3d::From(GetFrameElmCP(eeh)->uVector);
    vVector = DVec3d::From(GetFrameElmCP(eeh)->vVector);
    uVector.scaleToLength(extentInUOR.x);
    vVector.scaleToLength(extentInUOR.y);

    GetFrameElmP(eeh)->uVector = uVector;
    GetFrameElmP(eeh)->vVector = vVector;
    if (reValidateRange)
        {
        return (SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange(eeh));
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetScanningResolution(EditElementHandleR eeh,DPoint2dCR scanningResolutionDPI)
    {
    GetFrameElmP(eeh)->scanningResolution=scanningResolutionDPI;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetViewState(EditElementHandleR eeh, int viewNumber, bool state)
    {
    if (state)
        GetFrameElmP(eeh)->views |= (0x01 << viewNumber);
    else
        GetFrameElmP(eeh)->views &= ~(0x01 << viewNumber);

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetInvertState(EditElementHandleR eeh, bool state)
    {
    GetFrameElmP(eeh)->flags.invert=state;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetPrintState(EditElementHandleR eeh, bool state)
    {
    GetFrameElmP(eeh)->flags.plot=state;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetClipState(EditElementHandleR eeh, bool state)
    {
    GetFrameElmP(eeh)->flags.clipping=state;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetTransparencyState(EditElementHandleR eeh, bool state)
    {
    GetFrameElmP(eeh)->flags.transparency=state;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetBinaryPrintInvertState(EditElementHandleR eeh, bool state)
    {
    GetFrameElmP(eeh)->flags.binaryPlotInvert=state;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetDisplayOrder(EditElementHandleR eeh, long displayOrder)
    {
    GetFrameElmP(eeh)->displayOrder=displayOrder;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetForegroundColor(EditElementHandleR eeh, UInt32 rawColorIndex)
    {
    GetFrameElmP(eeh)->foregroundColor=rawColorIndex;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetBackgroundColor(EditElementHandleR eeh, UInt32 rawColorIndex)
    {
    GetFrameElmP(eeh)->backgroundColor=rawColorIndex;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetForegroundTransparencyLevel(EditElementHandleR eeh,UInt8 transparency)
    {
    GetFrameElmP(eeh)->foregroundTransparency=transparency;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetBackgroundTransparencyLevel(EditElementHandleR eeh,UInt8 transparency)
    {
    GetFrameElmP(eeh)->backgroundTransparency=transparency;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetImageTransparencyLevel(EditElementHandleR eeh,UInt8 transparency)
    {
    GetFrameElmP(eeh)->imageTransparency=transparency;
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetSourceUrl (EditElementHandleR eeh, WCharCP sourceURL)
    {
    return (BSISUCCESS == RasterXAttrSourceURLFacility::SetRasterSourceUrl(eeh,sourceURL));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetAttachDescription(EditElementHandleR eeh, WCharCP description)
    {
    if (SUCCESS != RasterXAttrStringFacility::SetString(description, eeh, DESCRIPTION_XATTR_ID))
        return false;

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::SetLogicalName(EditElementHandleR eeh, WCharCP logicalName)
    {
    if (SUCCESS!=  RasterXAttrStringFacility::SetString(logicalName, eeh, LOGICALNAME_XATTR_ID))
        return false;

    return true;
    }


