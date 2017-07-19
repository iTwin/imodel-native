/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Samples/XData/ImportXData.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "ImportXData.h"

//---------------------------------------------------------------------------------------
//  This is a helper macro that adds a unique Adhoc property in AdhocPropertiesBuilder(below):
//
//  1. Create an Adhoc property name, "XDxx", where xx is a 2-digit count of total entries
//  2. Get AdHocJsonPropertyValue by name, from the input DgnElement
//  3. Set the Adhoc value by type
//  4. Increase the property counter by 1 for next property
//
//---------------------------------------------------------------------------------------
#define ADDADHOCPROPERTY(_Type_, _Value_)                                               \
    Utf8PrintfString    uniqueName("XD%02d", m_count++);                                \
    AdHocJsonValue      adhocProp;                                                      \
    adhocProp.SetValue##_Type_ (#_Type_, _Value_);                                      \
    m_element.SetUserProperties (uniqueName.c_str(), adhocProp);


BEGIN_DGNDBSYNC_DWG_NAMESPACE

//=======================================================================================
// A helper class that adds an Adhoc property at a time to an input DgnElement. It tracks
// number of the properties added to ensure unique property names being added on the same 
// element.
//
// @bsiclass
//=======================================================================================
struct  AdhocPropertiesBuilder
    {
private:
    DgnElementR     m_element;
    uint32_t        m_count;

public:
    // Constructor
    AdhocPropertiesBuilder (DgnElementR el) : m_element(el) { m_count = 0; }

    // Add properties by type
    void AddInt (int32_t i)         { ADDADHOCPROPERTY(Int, i) }
    void AddInt64 (int64_t i)       { ADDADHOCPROPERTY(Int64, i) }
    void AddDouble (double d)       { ADDADHOCPROPERTY(Double, d) }
    void AddText (Utf8CP t)         { ADDADHOCPROPERTY(Text, t) }
    void AddPoint3d (DPoint3dCR p)  { ADDADHOCPROPERTY(Point3d, p) }
    };  // AdhocPropertiesBuilder

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ImportXData::ConvertXData (DgnElementR element, DwgDbEntityCR entity)
    {
    // Get xdata from the entity - an empty regapp name extracts all xdata
    DwgString           allRegApps;
    DwgResBufIterator   xdata = entity.GetXData (allRegApps);

    BentleyStatus       status = BSISUCCESS;

    if (xdata.IsValid())
        {
        LOG.tracev ("Entity %ls has XDATA, adding Adhoc properties to DgnElement...", entity.GetDxfName().c_str());

        AdhocPropertiesBuilder  builder(element);

        // Iterate through all entries, prints them and add them to DgnElement as Adhoc properties:
        for (DwgResBufP curr = xdata->Start(); curr != xdata->End(); curr = curr->Next())
            {
            switch (curr->GetDataType())
                {
                case DwgResBuf::DataType::Integer8:
                    LOG.tracev ("XDATA Int8= %d", curr->GetInteger8());
                    builder.AddInt (curr->GetInteger8());
                    break;
                case DwgResBuf::DataType::Integer16:
                    LOG.tracev ("XDATA Int16= %d", curr->GetInteger16());
                    builder.AddInt (curr->GetInteger16());
                    break;
                case DwgResBuf::DataType::Integer32:
                    LOG.tracev ("XDATA Int32= %d", curr->GetInteger32());
                    builder.AddInt (curr->GetInteger32());
                    break;
                case DwgResBuf::DataType::Integer64:
                    LOG.tracev ("XDATA Int64= %I64d", curr->GetInteger64());
                    builder.AddInt64 (curr->GetInteger64());
                    break;
                case DwgResBuf::DataType::Double:
                    LOG.tracev ("XDATA Double= %g", curr->GetDouble());
                    builder.AddDouble (curr->GetDouble());
                    break;
                case DwgResBuf::DataType::Text:
                    LOG.tracev ("XDATA String= %ls", curr->GetString().c_str());
                    builder.AddText (DwgHelper::ToUtf8CP(curr->GetString()));
                    break;
                case DwgResBuf::DataType::BinaryChunk:
                    {
                    // Binary is not currently supported as an Adhoc property - just show size in a text
                    DwgBinaryData   data;
                    if (DwgDbStatus::Success == curr->GetBinaryData(data))
                        {
                        LOG.tracev ("XDATA Binary data size = %lld", data.GetSize());
                        builder.AddText (Utf8PrintfString("binary data in %lld bytes", data.GetSize()).c_str());
                        }
                    else
                        {
                        BeDataAssert(false && "failed extracting binary xdata!");
                        }
                    break;
                    }
                case DwgResBuf::DataType::Handle:
                    LOG.tracev ("XDATA Handle= %ls", curr->GetHandle().AsAscii().c_str());
                    builder.AddText (Utf8PrintfString("Handle=%ls", curr->GetHandle().AsAscii().c_str()).c_str());
                    break;
                case DwgResBuf::DataType::HardOwnershipId:
                case DwgResBuf::DataType::SoftOwnershipId:
                case DwgResBuf::DataType::HardPointerId:
                case DwgResBuf::DataType::SoftPointerId:
                    LOG.tracev ("XDATA ObjectId= %ls", curr->GetObjectId().ToAscii().c_str());
                    builder.AddText (Utf8PrintfString("ObjectID=%ls", curr->GetObjectId().ToAscii().c_str()).c_str());
                    break;
                case DwgResBuf::DataType::Point3d:
                    {
                    DPoint3d    point;
                    if (DwgDbStatus::Success == curr->GetPoint3d(point))
                        {
                        LOG.tracev ("XDATA Point3d= %g, %g, %g", point.x, point.y, point.z);
                        builder.AddPoint3d (point);
                        }
                    else
                        {
                        BeDataAssert (false && "failed extracting Point3d xdata!");
                        }
                    break;
                    }
                case DwgResBuf::DataType::None:
                case DwgResBuf::DataType::NotRecognized:
                default:
                    LOG.warning ("Unexpected XDATA type!!!");
                }
            }
        }

    return  status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ImportXData::_ImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    // Create a DgnElement from DWG entity by default, and get the results back:
    DgnElementP     dgnElement = nullptr;
    BentleyStatus   status = T_Super::_ImportEntity (results, inputs);
    if (BSISUCCESS != status || nullptr == (dgnElement = results.GetImportedElement()))
        {
        LOG.errorv ("Failed creating DgnElement from DWG entity %lld", inputs.GetEntity().GetObjectId().ToUInt64());
        return  status;
        }

    LOG.tracev ("DgnElement %s(ID=%lld) has been created, checking XDATA...", dgnElement->GetDisplayLabel().c_str(), dgnElement->GetElementId());

    status = ConvertXData (*dgnElement, inputs.GetEntity());

    if (BSISUCCESS != status)
        LOG.error ("Failed adding xdata to DgnElement as Adhoc properties!");

    return  status;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ImportXDataSample::ImportDwgFile (int argc, WCharCP argv[])
    {
    return T_Super::RunAsStandaloneExe(argc, argv);
    }

END_DGNDBSYNC_DWG_NAMESPACE


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int wmain (int argc, wchar_t const* argv[])
    {
    /*-----------------------------------------------------------------------------------
    Expected command line arguments:

    ImportXData <input DWG full file name> <output DgnDb folder name>
    -----------------------------------------------------------------------------------*/
    if (argc < 3)
        return  1;

    ImportXDataSample     sampleImporter;

    // Begin importing DWG file into DgnDb
    BentleyStatus   status = sampleImporter.ImportDwgFile (argc, argv);

    return (int)status;
    }
