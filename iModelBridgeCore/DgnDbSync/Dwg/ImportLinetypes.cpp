/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportLinetypes.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

#define PREFIX_LineCode         "LC-"
#define PREFIX_LinePoint        "LP-"
#define PREFIX_PointSymbol      "Symbol"


BEGIN_DWG_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          08/16
+===============+===============+===============+===============+===============+======*/
struct LineStyleFactory
    {
private:
    DwgImporter&                    m_importer;
    DwgDbLinetypeTableRecordPtr&    m_linetype;
    bool                            m_hasSymbols;
    uint32_t                        m_numberOfDashes;
    Utf8String                      m_linetypeName;
    Utf8String                      m_linecodeName;

    LineStyleStatus     CreateContinuousStyle (LsComponentId& outId);
    LineStyleStatus     CreateLineCode (LsComponentId& outId);
    LineStyleStatus     CreateLinePoint (LsComponentId& outId, LsComponentId const& linecodeId);
    LineStyleStatus     CreateCompound (LsComponentId& outId, LsComponentId const& linecodeId);
    LineStyleStatus     CreatePointSymbol (LsComponentId& outId, uint32_t symbolNo, uint32_t segNo);
    BentleyStatus       CreateTextString (TextStringPtr& dgnText, uint32_t segNo);
    BentleyStatus       BuildGeometryFromText (GeometryBuilderPtr& builder, DRange3dR outRange, uint32_t segNo);
    BentleyStatus       BuildGeometryFromShape (GeometryBuilderPtr& builder, DRange3dR outRange, uint32_t segNo, uint32_t shapeNumber);
    size_t              GetString (Utf8StringR out, uint32_t segNo) const;
    DgnFontCP           GetFont (DwgDbTextStyleTableRecordCR textStyle) const;
    bool                IsSymbolSegment (uint32_t segNo) const;

public:
    LineStyleFactory (DwgImporter& importer, DwgDbLinetypeTableRecordPtr& ltype) : m_importer(importer), m_linetype(ltype)
        {
        m_hasSymbols = false;
        m_numberOfDashes = ltype->GetNumberOfDashes ();
        m_linetypeName.Assign (ltype->GetName().c_str());
        m_linecodeName.clear ();
        }

    LineStyleStatus     Create (DgnStyleId& outId);
    };  // LineStyleFactory

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LineStyleFactory::IsSymbolSegment (uint32_t segNo) const
    {
    // check if the segment is either a text character or a shape symbol:
    if (m_linetype->GetShapeNumberAt(segNo) > 0)
        return  true;

    DwgString   text;
    if (DwgDbStatus::Success == m_linetype->GetTextAt(text, segNo) && text.c_str()[0] != 0)
        return  true;

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus LineStyleFactory::CreateContinuousStyle (LsComponentId& outId)
    {
    Json::Value continuous(Json::objectValue);
    continuous["descr"] = "CONTINUOUS";

    return LsComponent::AddComponentAsJsonProperty(outId, m_importer.GetDgnDb(), LsComponentType::LineCode, continuous);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus LineStyleFactory::CreateLineCode (LsComponentId& outId)
    {
    Json::Value linecodeDef(Json::objectValue);

    // set options: a center-stretched single segment
    linecodeDef["options"] = LCOPT_SEGMENT | LCOPT_CENTERSTRETCH;

    m_hasSymbols = false;

    // set strokes
    Json::Value strokes(Json::arrayValue);

    for (uint32_t i = 0; i < m_numberOfDashes; i++)
        {
        // dash/space length
        double      strokeLength = m_linetype->GetDashLengthAt (i);

        // dash or space
        uint32_t    strokeFlags = strokeLength >= 0.0 ? LCSTROKE_DASH : LCSTROKE_GAP;
        
        // set the new linecode entry in Json value:
        Json::Value     linecodeEntry(Json::objectValue);
        linecodeEntry["length"] = fabs (strokeLength);
        linecodeEntry["strokeMode"] = strokeFlags;
        linecodeEntry["widthMode"] = 3;

        strokes[i] = linecodeEntry;

        if (this->IsSymbolSegment(i))
            m_hasSymbols = true;
        }

    linecodeDef["strokes"] = strokes;

    // set LineCode name based on whether or not a symbol exists in the linetype
    if (m_hasSymbols)
        m_linecodeName = Utf8String(PREFIX_LineCode) + m_linetypeName;
    else
        m_linecodeName = m_linetypeName;

    linecodeDef["descr"] = m_linecodeName.c_str ();

    outId = LsComponentId ();

    return LsComponent::AddComponentAsJsonProperty(outId, m_importer.GetDgnDb(), LsComponentType::LineCode, linecodeDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          LineStyleFactory::GetString (Utf8StringR outString, uint32_t segNo) const
    {
    DwgString   dwgString;
    uint32_t    shapeNumber = 0;

    if (DwgDbStatus::Success == m_linetype->GetTextAt(dwgString, segNo) && !dwgString.IsEmpty())
        {
        // this is an ASCII string - use it as is
        outString.Assign (dwgString.c_str());
        }
    else if (0 != (shapeNumber = m_linetype->GetShapeNumberAt(segNo)))
        {
        // this is a shape code - make it as an ASCII character plus an ending terminator.
        Utf8Char    shapeCodes[] = { static_cast<Utf8Char>(0xFF & shapeNumber), 0 };
        outString.assign (shapeCodes);
        }
    else
        {
        m_importer.ReportError (IssueCategory::UnexpectedData(), Issue::LinetypeError(), Utf8PrintfString("unsupported symbol type in linetype %s!", m_linetypeName.c_str()).c_str());
        return  BSIERROR;
        }

    return  outString.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontCP       LineStyleFactory::GetFont (DwgDbTextStyleTableRecordCR textStyle) const
    {
    DgnFontCP       dgnFont = nullptr;
    DwgFontInfo     fontInfo;

    if (DwgDbStatus::Success != textStyle.GetFontInfo(fontInfo))
        {
        m_importer.ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::InconsistentData(), Issue::LinetypeError(), m_linetypeName.c_str(), "can't find DWG font!");
        dgnFont = m_importer.GetDefaultFont();
        if (nullptr == dgnFont)
            dgnFont = &DgnFontManager::GetAnyLastResortFont ();
        }
    else
        {
        dgnFont = m_importer.GetDgnFontFor (fontInfo);
        if (nullptr == dgnFont)
            {
            m_importer.ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::InconsistentData(), Issue::LinetypeError(), m_linetypeName.c_str(), "can't find font!");
            dgnFont = fontInfo.GetTypeFace().empty() ? &DgnFontManager::GetLastResortTrueTypeFont() : &DgnFontManager::GetLastResortShxFont();
            }
        }

    return  dgnFont;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LineStyleFactory::CreateTextString (TextStringPtr& dgnText, uint32_t segNo)
    {
    dgnText = TextString::Create ();
    if (!dgnText.IsValid())
        return  BSIERROR;

    DwgDbTextStyleTableRecordPtr    dwgStyle(m_linetype->GetShapeStyleAt(segNo), DwgDbOpenMode::ForRead);
    if (dwgStyle.IsNull())
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::CantOpenObject(), Utf8PrintfString("failed opening a textstyle used in linetype[%ls]!", m_linetypeName.c_str()).c_str());
        return  BSIERROR;
        }

    // get the ascii string at this segment
    Utf8String  asciiString;
    if (this->GetString(asciiString, segNo) < 1)
        return  BSIERROR;
    
    // try to get the working font of the symbol:
    DgnFontCP   dgnFont = this->GetFont (*dwgStyle.get());
    if (nullptr == dgnFont)
        return  BSIERROR;
    
    // size for either a text or a shape:
    DPoint2d    textSize = DPoint2d::From (1.0, 1.0);
    if (!dwgStyle->IsShapeFile())
        {
        double  styleHeight = dwgStyle->GetTextSize ();
        if (styleHeight > 0.0)
            textSize.y = styleHeight;
        }

    textSize.x = textSize.y * dwgStyle->GetWidthFactor();
    if (textSize.x < 0.0)
        {
        BeDataAssert (false && "Unexpected text style size!");
        textSize.x = 1.0;
        }

    double      scale = m_linetype->GetShapeScaleAt (segNo);
    DPoint3d    origin = DPoint3d::FromZero ();
    if (m_linetype->IsShapeUprightAt(segNo) && fabs(scale) > 1.0e-4)
        {
        // text must be kept upright - move its origin to ensure correct rotation of the text:
        DPoint2d    shapeOffset = m_linetype->GetShapeOffsetAt (segNo);

        origin.x += shapeOffset.x / scale;
        origin.y += shapeOffset.y / scale;
        }

    dgnText->SetOrigin (origin);
    dgnText->SetText (asciiString.c_str());

    TextStringStyleR    style = dgnText->GetStyleR ();
    style.SetFont (*dgnFont);
    style.SetSize (textSize.x, textSize.y);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LineStyleFactory::BuildGeometryFromText (GeometryBuilderPtr& builder, DRange3dR outRange, uint32_t segNo)
    {
    // create a TextString from either a text or a shape code:
    TextStringPtr   dgnText;
    if (BSISUCCESS != this->CreateTextString(dgnText, segNo))
        return BSIERROR;

    // create a geoemtry for TextString
    GeometricPrimitivePtr   geometry = GeometricPrimitive::Create (dgnText);
    if (!geometry.IsValid())
        return BSIERROR;

    outRange.Init ();
    geometry->GetRange (outRange, nullptr);

    // build a geometry part
    builder = GeometryBuilder::CreateGeometryPart (m_importer.GetDgnDb(), true);
    if (!builder.IsValid())
        return BSIERROR;

    return builder->Append(*geometry.get()) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LineStyleFactory::BuildGeometryFromShape (GeometryBuilderPtr& builder, DRange3dR outRange, uint32_t segNo, uint32_t shapeNumber)
    {
    // this is a shape code - make it as an ASCII character plus an ending terminator.
    DwgString   shapeCode;
    shapeCode.Append (static_cast<WChar>(shapeNumber));

    DwgDbTextStyleTableRecordPtr    dbStyle(m_linetype->GetShapeStyleAt(segNo), DwgDbOpenMode::ForRead);
    if (dbStyle.IsNull())
        return  BSIERROR;

    DwgGiTextStyle      giTextstyle;
    if (DwgDbStatus::Success != giTextstyle.CopyFrom(*dbStyle.get()))
        return  BSIERROR;

    // try dropping the text string to a collection of linestrings
    bvector<DPoint3dArray>  linestrings;
    ShapeTextProcessor      shapeProcessor (giTextstyle, shapeCode);
    if (DwgDbStatus::Success != shapeProcessor.Drop(linestrings, 0.0))
        return  BSIERROR;

    // create a geometry part for dropped shape
    builder = GeometryBuilder::CreateGeometryPart (m_importer.GetDgnDb(), true);
    if (!builder.IsValid())
        return BSIERROR;

    // add linestrings into the part
    outRange.Init ();
    for (auto& linestring : linestrings)
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLineString (linestring);
        if (primitive.IsValid())
            {
            DRange3d    range;
            primitive->GetRange (range);
            outRange.Extend (range);

            builder->Append (*primitive.get());
            }
        }
    
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus LineStyleFactory::CreatePointSymbol (LsComponentId& outId, uint32_t symbolNo, uint32_t segNo)
    {
    // check if this is a text or a shape and build geometry accordingly:
    uint32_t            shapeNumber = m_linetype->GetShapeNumberAt (segNo);
    BentleyStatus       status;
    DRange3d            range;
    GeometryBuilderPtr  builder;

    if (0 != shapeNumber)
        status = this->BuildGeometryFromShape (builder, range, segNo, shapeNumber);
    else
        status = this->BuildGeometryFromText (builder, range, segNo);

    if (BSISUCCESS != status)
        return LINESTYLE_STATUS_ConvertingComponent;

    DgnDbR      dgndb = m_importer.GetDgnDb ();

    // place parts in our partitioned model
    auto partsModel = m_importer.GetGeometryPartsModel ();
    if (!partsModel.IsValid())  // should not occur!
        partsModel = &dgndb.GetDictionaryModel ();

    auto ltypeId = m_linetype->GetObjectId().ToUInt64 ();
    auto fileId = m_importer.GetDwgFileId (m_importer.GetDwgDb(), false);
    Utf8PrintfString codevalue("%s-%d:%x-%d", m_linetypeName.c_str(), fileId, ltypeId, segNo);

    // build geometry part
    DgnGeometryPartPtr  geometryPart = DgnGeometryPart::Create (*partsModel, codevalue);
    if (BSISUCCESS != builder->Finish(*geometryPart))
        return LINESTYLE_STATUS_ConvertingComponent;

    // add the geometry part to DB
    dgndb.Elements().Insert <DgnGeometryPart> (*geometryPart);

    // calc geometry base & size:
    range.high.Subtract (range.low);

    ColorDef    color = ColorDef::White ();
    double      scale = m_linetype->GetShapeScaleAt (segNo);
    if (scale != 0.0)
        scale = 1.0 / scale;

    // set symbol flags
    uint32_t    symbolFlags = LSSYM_3D;
    if (fabs(scale) < 1.0e-4 || fabs(scale - 1.0) < 1.0e-4)
        symbolFlags |= LSSYM_NOSCALE;

    // create symbol definition in Json
    Json::Value symbolDef (Json::objectValue);
    symbolDef["descr"] = Utf8PrintfString("%s%d-%s", PREFIX_PointSymbol, symbolNo, m_linetypeName.c_str()).c_str ();

    // set Json values
    LsSymbolComponent::SaveSymbolDataToJson (symbolDef, range.low, range.high, geometryPart->GetId(), symbolFlags, scale);

    // create a PointSymbol component
    return LsComponent::AddComponentAsJsonProperty(outId, dgndb, LsComponentType::PointSymbol, symbolDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus LineStyleFactory::CreateLinePoint (LsComponentId& outId, LsComponentId const& linecodeId)
    {
    Json::Value linepointDef (Json::objectValue);

    // add linecode to the linepoint def
    LsPointComponent::SaveLineCodeIdToJson(linepointDef, linecodeId);

    Json::Value symbols (Json::arrayValue);
    uint32_t    symbolIndex = 0;

    // collect symbols
    for (uint32_t i = 0; i < m_numberOfDashes; i++)
        {
        if (!this->IsSymbolSegment(i))
            continue;

        LsComponentId   symbolId;
        if (LINESTYLE_STATUS_Success != this->CreatePointSymbol(symbolId, symbolIndex, i))
            continue;

        Json::Value     linepointEntry(Json::objectValue);
        LsPointComponent::SaveSymbolIdToJson (linepointEntry, symbolId);

        linepointEntry["strokeNum"] = i;

        // adjust symbol location should it be aligned on the line:
        if (!m_linetype->IsShapeUprightAt(i))
            {
            DPoint2d    shapeOffset = m_linetype->GetShapeOffsetAt(i);
            linepointEntry["xOffset"] = shapeOffset.x;
            linepointEntry["yOffset"] = shapeOffset.y;
            }

        double  angle = m_linetype->GetShapeRotationAt (i);
        if (fabs(angle) > 1.0e-3)
            linepointEntry["angle"] = angle;
        
        // set modifiers for the point symbol entry
        uint32_t    modifier = LCPOINT_END | LCPOINT_NOSCALE;
        if (m_linetype->IsShapeUprightAt(i))
            modifier |= LCPOINT_ADJROT;
        linepointEntry["mod1"] = modifier;

        symbols.append (linepointEntry);
        symbolIndex++;
        }

    // if can't create a line point for any reason, error out now:
    if (symbolIndex < 1)
        return  LINESTYLE_STATUS_ComponentNotFound;

    linepointDef["symbols"] = symbols;

    // set LinePoint name
    Utf8String  name = Utf8String(PREFIX_LinePoint) + m_linetypeName;
    linepointDef["descr"] = name.c_str();

    outId = LsComponentId ();
    return LsComponent::AddComponentAsJsonProperty(outId, m_importer.GetDgnDb(), LsComponentType::LinePoint, linepointDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus LineStyleFactory::CreateCompound (LsComponentId& outId, LsComponentId const& linecodeId)
    {
    // DWG compound linestyle contains a LineCode and a LinePoint - first try creating a LinePoint:
    LsComponentId   linepointId;
    LineStyleStatus status = this->CreateLinePoint (linepointId, linecodeId);
    if (LINESTYLE_STATUS_Success != status)
        return  status;

    // set LineCode component
    Json::Value linecodeEntry (Json::objectValue);
    linecodeEntry["type"] = static_cast<uint32_t> (linecodeId.GetType());
    linecodeEntry["id"] = linecodeId.GetValue ();

    // set LinePoint component
    Json::Value linepointEntry (Json::objectValue);
    linepointEntry["type"] = static_cast<uint32_t> (linepointId.GetType());
    linepointEntry["id"] = linepointId.GetValue ();

    // put them into a Json array
    Json::Value components (Json::arrayValue);
    components.append (linecodeEntry);
    components.append (linepointEntry);

    // create compound definition in Json:
    Json::Value compoundDef (Json::objectValue);
    compoundDef["descr"] = m_linetypeName.c_str();
    compoundDef["comps"] = components;

    outId = LsComponentId ();
    return LsComponent::AddComponentAsJsonProperty(outId, m_importer.GetDgnDb(), LsComponentType::Compound, compoundDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus LineStyleFactory::Create (DgnStyleId& styleId)
    {
    if (m_linetype.IsNull())
        {
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingLinetype(), "??");
        return  LINESTYLE_STATUS_Error;
        }

    // set attributes (LSATTR_SHAREDCELL_SCALE_INDEPENDENT deprecated in DgnDb??)
    uint32_t        lsAttribs = LSATTR_UNITMETERS | LSATTR_NOSNAP;
    LsComponentId   componentId;
    LineStyleStatus status = LINESTYLE_STATUS_Success;

    if (m_linetype->IsByLayer())
        {
        styleId = DgnStyleId((uint64_t)LsKnownStyleNumber::STYLE_ByLevel);
        return  status;
        }
    else if (m_linetype->IsByBlock())
        {
        styleId = DgnStyleId((uint64_t)LsKnownStyleNumber::STYLE_ByCell);
        return  status;
        }
    else if (m_linetype->IsContinuous())
        {
        status = this->CreateContinuousStyle (componentId);
        lsAttribs |= LSATTR_CONTINUOUS;
        }
    else
        {
        // always create a LineCode
        status = this->CreateLineCode (componentId);

        if (LINESTYLE_STATUS_Success == status && m_hasSymbols)
            {
            // create a compound component by appending above LineCode + a new LinePoint, and get back the compound ID:
            LsComponentId   linecodeId = componentId;
            this->CreateCompound (componentId, linecodeId);
            }
        }

    if (LINESTYLE_STATUS_Success == status)
        {
        if (!m_hasSymbols)
            lsAttribs |= LSATTR_NORANGE;

        /*-------------------------------------------------------------------------------
        Scale the whole linestyle to Meters and compound it by LTSCALE. A scaled line style
        may be created from:

            1) Modelspace annotation scale when MSLTSCALE=1
            2) Paperspace annotation scale when PSLTSCALE=1
            3) Entity linetype scale, possibly compounded by 1) or 2).

        but they are all compounded by LTSCALE, except for obviously linetype continuous.
        -------------------------------------------------------------------------------*/
        double  scale = 1.0;
        if (0 == (lsAttribs & LSATTR_CONTINUOUS))
            scale = m_importer.GetScaleToMeters() * m_importer.GetDwgDb().GetLTSCALE();

        // add the new style to DgnDb:
        if (BSISUCCESS != m_importer.GetDgnDb().LineStyles().Insert(styleId, m_linetypeName.c_str(), componentId, lsAttribs, scale))
            {
            m_importer.ReportError (IssueCategory::DiskIO(), Issue::SaveError(), Utf8PrintfString("line style [%s]", m_linetypeName.c_str()).c_str());
            status = LINESTYLE_STATUS_SQLITE_Error;
            }
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus DwgImporter::_ImportLineType (DwgDbLinetypeTableRecordPtr& linetype)
    {
    DgnStyleId          styleId;
    LineStyleFactory    factory(*this, linetype);

    LineStyleStatus     status = factory.Create (styleId);

    if (LINESTYLE_STATUS_Success == status)
        {
        // save the imported DWG-DGN linestyle into the syncInfo
        this->GetSyncInfo().InsertLinetype (styleId, *linetype.get());
        // save it to our linestyle map
        m_importedLinestyles.insert (T_DwgDgnLineStyleId(linetype->GetObjectId(), styleId));
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyleId      DwgImporter::GetDgnLineStyleFor (DwgDbObjectIdCR ltypeId)
    {
    auto        found = m_importedLinestyles.find (ltypeId);
    if (found != m_importedLinestyles.end())
        return  found->second;

    return  DgnStyleId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportLineTypeSection ()
    {
    DwgDbLinetypeTablePtr  linetypeTable(m_dwgdb->GetLinetypeTableId(), DwgDbOpenMode::ForRead);
    if (linetypeTable.IsNull())
        return  BSIERROR;

    DwgDbSymbolTableIteratorPtr iter = linetypeTable->NewIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        return  BSIERROR;

    this->SetStepName (ProgressMessage::STEP_IMPORTING_LINETYPES());

    uint32_t    count = 0;
    for (iter->Start(); !iter->Done(); iter->Step())
        {
        DwgDbLinetypeTableRecordPtr    linetype(iter->GetRecordId(), DwgDbOpenMode::ForRead);
        if (linetype.IsNull())
            {
            this->ReportError (IssueCategory::Unknown(), Issue::CantOpenObject(), Utf8PrintfString("linetype ID=%ld", iter->GetRecordId().ToAscii()).c_str());
            continue;
            }

        // ByLayer & ByBlock may not be needed at all??
        if (linetype->IsByLayer() || linetype->IsByBlock())
            continue;

        DwgString       name = linetype->GetName ();
        if (name.IsEmpty())
            this->ReportIssue (IssueSeverity::Warning, IssueCategory::UnexpectedData(), Issue::Error(), "empty linetype name!");
        else
            LOG_LINETYPE.tracev ("Processinging DWG Linetype %ls", name.c_str());

        if (this->IsUpdating())
            {
            // this is an unscaled linestyle
            DgnStyleId  lstyleId = m_syncInfo.FindLineStyle (linetype->GetObjectId());
            if (lstyleId.IsValid())
                {
                // update syncInfo
                this->_OnUpdateLineType (lstyleId, *linetype.get());
                // save to our linestyle map
                m_importedLinestyles.insert (T_DwgDgnLineStyleId(linetype->GetObjectId(), lstyleId));
                continue;
                }
            }

        if ((count++ % 100) == 0)
            this->Progress ();

        this->_ImportLineType (linetype);
        }
    
    return  BSISUCCESS;
    }

END_DWG_NAMESPACE
