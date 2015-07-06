/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>
#include <RasterSchema/RasterFileHandler.h>
#include "RasterSource.h"
#include "RasterQuadTree.h"
#include "RasterFileSource.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_RASTERSCHEMA

HANDLER_DEFINE_MEMBERS(RasterFileModelHandler)

//----------------------------------------------------------------------------------------
//-------------------------------  RasterFileProperties  ---------------------------------
//----------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DPoint2dFromJson (DPoint2dR point, JsonValueCR inValue)
    {
    point.x = inValue[0].asDouble();
    point.y = inValue[1].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DPoint2dToJson (JsonValueR outValue, DPoint2dCR point)
    {
    outValue[0] = point.x;
    outValue[1] = point.y;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
RasterFileProperties::RasterFileProperties()
    {
    m_fileMonikerPtr = FileMoniker::Create("", "");
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
static void DRange2dFromJson (DRange2dR range, JsonValueCR inValue)
    {
    DPoint2dFromJson (range.low, inValue["low"]);
    DPoint2dFromJson (range.high, inValue["high"]);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
static void DRange2dToJson (JsonValueR outValue, DRange2dCR range)
    {
    DPoint2dToJson (outValue["low"], range.low);
    DPoint2dToJson (outValue["high"], range.high);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void RasterFileProperties::ToJson(Json::Value& v) const
    {
    m_fileMonikerPtr->ToJson(v["fileMoniker"]);
    DRange2dToJson(v["bbox"], m_boundingBox);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void RasterFileProperties::FromJson(Json::Value const& v)
    {
    m_fileMonikerPtr->FromJson(v["fileMoniker"]);
    DRange2dFromJson(m_boundingBox, v["bbox"]);
    }

//----------------------------------------------------------------------------------------
//------------------------------  RasterFileModelHandler  --------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
DgnModelId RasterFileModelHandler::CreateRasterFileModel(DgnDbR db, FileMonikerPtr fileMoniker)
    {
    DgnClassId classId(db.Schemas().GetECClassId(BENTLEY_RASTER_SCHEMA_NAME, RASTER_CLASSNAME_RasterFileModel));
    BeAssert(classId.IsValid());

    // Find resolved file name for the raster
    BeFileName basePath(db.GetDbFileName());
    Utf8String basePathUtf8(basePath);
    Utf8String resolvedName;
    fileMoniker->ResolveFileName(resolvedName, basePathUtf8);

    // Create model name (just use the file name without extension)
    BeFileName fileName(resolvedName);
    WString modelName ( fileName.GetFileNameWithoutExtension().c_str() );
    Utf8String utf8Name(modelName);

    // Set RasterFileProperties
    RasterFileProperties props;
    props.m_fileMonikerPtr = fileMoniker;

    //&&ep - find real raster range
    DRange2d range2d = DRange2d::From(1, 0);
    props.m_boundingBox = range2d;

    // Open raster
    RasterFilePtr rasterFilePtr = RasterFile::Create(resolvedName);
    if (rasterFilePtr == nullptr)
        {
        // Can't create model; probably that file name is invalid. Return an invalid model id.
        return DgnModelId();
        }

    // Create model in DgnDb
    RasterFileModelPtr model = new RasterFileModel(DgnModel::CreateParams(db, classId, utf8Name.c_str()), props);
    model->Insert();
    return model->GetModelId();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::RasterFileModel(CreateParams const& params) 
:T_Super (params)
    {

    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::RasterFileModel(CreateParams const& params, RasterFileProperties const& properties) 
:T_Super (params),
 m_fileProperties(properties)
    {

    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::~RasterFileModel()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
BentleyStatus RasterFileModel::_LoadQuadTree()
    {
    m_rasterTreeP = nullptr;

    // Resolve raster name
    BeFileName basePath(GetDgnDb().GetDbFileName());
    Utf8String basePathUtf8(basePath);
    Utf8String resolvedName;
    m_fileProperties.m_fileMonikerPtr->ResolveFileName(resolvedName, basePathUtf8);

    // Create RasterQuadTree
    RasterSourcePtr pSource = RasterFileSource::Create(resolvedName);
    if(pSource.IsValid())
        m_rasterTreeP = RasterQuadTree::Create(*pSource, GetDgnDb());

    return m_rasterTreeP.IsValid() ? BSISUCCESS : BSIERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d RasterFileModel::_QueryModelRange() const
    {
    return AxisAlignedBox3d(m_fileProperties.m_boundingBox);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterFileModel::_ToPropertiesJson(Json::Value& v) const
    {
    T_Super::_ToPropertiesJson(v);
    m_fileProperties.ToJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterFileModel::_FromPropertiesJson(Json::Value const& v)
    {
    T_Super::_FromPropertiesJson(v);
    m_fileProperties.FromJson(v);
    }

