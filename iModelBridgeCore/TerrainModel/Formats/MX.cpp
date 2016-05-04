/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/MX.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TerrainModel/TerrainModel.h>
#include "TerrainModel/Formats/Formats.h"
#include "MXModelFile.h"
#include "mxtriangle.h"
#include "TerrainModel\Formats\mx.h"

#define asLong(a) *((long*)a)

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

MXFilImporter::MXFilImporter (WCharCP filename) : m_filename (filename)
    {
    }

bool MXFilImporter::IsFileSupported (WCharCP filename)
    {
    if (BeFileName::GetExtension (filename).CompareToI (L"fil") == 0)
        {
        MXModelFile modelFile;

        if (modelFile.Open (filename) == eOk)
            {
            modelFile.Close ();
            return true;
            }
        modelFile.Close ();
        }
    return false;
    }

MXFilImporterPtr MXFilImporter::Create (WCharCP filename)
    {
    if (MXFilImporter::IsFileSupported (filename))
        return new MXFilImporter (filename);
    return nullptr;
    }

const TerrainInfoList& MXFilImporter::_GetTerrains () const
    {
    if (m_surfaces.size() == 0)
        {
        MXModelFile modelFile;

        if (modelFile.Open (m_filename.GetWCharCP()) == eOk)
            {
            MXModelObjectPtr<ModelTable> modelTable;
            ErrorStatus es = modelFile.getModelTable(modelTable.GetR(), ModelObject::Read);
            if (es == eOk)
                {
                std::unique_ptr<ModelTableIterator> iter(modelTable->newIterator());
                while(!iter->done())
                    {
                    MXModelObjectPtr<ModelTableRecord> modelTableRecord;
                    if (iter->getRecord(modelTableRecord.GetR(), ModelObject::Read) == eOk)
                        {
                        WString modelName (modelTableRecord->modelName());
                        if (modelTableRecord->modelType() == 0 || asLong(modelTableRecord->modelType()) == asLong ("    "))
                            {
                            // normal string model add this name.
                            m_surfaces.push_back (TerrainInfo (modelName.GetWCharCP(), nullptr, false));
                            }
                        if (asLong(modelTableRecord->modelType()) == asLong ("TRIA"))
                            {
                            // triangulation model search for the strings.
                            MXModelObjectPtr<StringTable> stringTable;
                            ErrorStatus es = modelTableRecord->getStringTable (stringTable.GetR(), ModelObject::Read);
                            if (es == eOk)
                                {
                                std::unique_ptr<StringTableIterator> iter(stringTable->newIterator());
                                while(!iter->done())
                                    {
                                    MXModelObjectPtr<StringTableRecord> stringTableRecord;
                                    if (iter->getRecord(stringTableRecord.GetR(), ModelObject::Read) == eOk)
                                        {
                                        // triangulation string
                                        WString stringName (stringTableRecord->stringName());
                                        WString name = stringName + modelName;
                                        m_surfaces.push_back (TerrainInfo (name.GetWCharCP(), nullptr, true));
                                        }
                                    iter->next();
                                    }
                                }
                            }
                        }
                    iter->next();
                    }
                }
            }
        }
    return m_surfaces;
    }


void MXFilImporter::DoImport (bmap <WString, Bentley::TerrainModel::BcDTMPtr>& nameDtms, bool importAll) const
    {
    MXModelFile modelFile;

    if (modelFile.Open (m_filename.GetWCharCP()) == eOk)
        {
        MXModelObjectPtr<ModelTable> modelTable;
        ErrorStatus es = modelFile.getModelTable(modelTable.GetR(), ModelObject::Read);
        if (es == eOk)
            {
            std::unique_ptr<ModelTableIterator> iter(modelTable->newIterator());
            while(!iter->done())
                {
                MXModelObjectPtr<ModelTableRecord> modelTableRecord;
                if (iter->getRecord(modelTableRecord.GetR(), ModelObject::Read) == eOk)
                    {
                    WString modelName (modelTableRecord->modelName());
                    if (modelTableRecord->modelType() == 0 || asLong(modelTableRecord->modelType()) == asLong ("    "))
                        {
                        // normal string model add this name.
                        if (importAll || nameDtms.find (modelName) != nameDtms.end())
                            nameDtms[modelName] = ImportStringModel (modelTableRecord.get());
                        }
                    if (asLong(modelTableRecord->modelType()) == asLong ("TRIA"))
                        {
                        // triangulation model search for the strings.
                        MXModelObjectPtr<StringTable> stringTable;
                        ErrorStatus es = modelTableRecord->getStringTable (stringTable.GetR(), ModelObject::Read);
                        if (es == eOk)
                            {
                            std::unique_ptr<StringTableIterator> iter(stringTable->newIterator());
                            while(!iter->done())
                                {
                                MXModelObjectPtr<StringTableRecord> stringTableRecord;
                                if (iter->getRecord(stringTableRecord.GetR(), ModelObject::Read) == eOk)
                                    {
                                    // triangulation string
                                    WString stringName (stringTableRecord->stringName());
                                    WString name = stringName + modelName;
                                    if (importAll || nameDtms.find (name) != nameDtms.end())
                                        nameDtms[name] = ImportTriangulation (stringTableRecord.get(), name.GetWCharCP());
                                    }
                                iter->next();
                                }
                            }
                        }
                    }
                iter->next();
                }
            }
        }
    }

ImportedTerrain MXFilImporter::_ImportTerrain (WCharCP name) const
    {
    bmap <WString, Bentley::TerrainModel::BcDTMPtr> nameDtms;
    nameDtms[name] = nullptr;
    DoImport (nameDtms, false);

    return ImportedTerrain (nameDtms[name].get(), name, nullptr, false);    // ToDo set if it was from a string model or a triangulated string
    }

ImportedTerrainList MXFilImporter::_ImportTerrains () const
    {
    bmap <WString, Bentley::TerrainModel::BcDTMPtr> nameDtms;
    DoImport (nameDtms, true);
    ImportedTerrainList list;
    for (bmap <WString, Bentley::TerrainModel::BcDTMPtr>::const_iterator iter = nameDtms.begin(); iter != nameDtms.end(); iter++)
        list.push_back (ImportedTerrain (iter->second.get(), iter->first.GetWCharCP(), nullptr, false));    // ToDo set if it was from a string model or a triangulated string

    return list;
    }

ImportedTerrainList MXFilImporter::_ImportTerrains (bvector<WString>& names) const
    {
    bmap <WString, Bentley::TerrainModel::BcDTMPtr> nameDtms;

    for (bvector<WString>::const_iterator iter = names.begin(); iter != names.end(); iter++)
        nameDtms[*iter] = nullptr;

    DoImport (nameDtms, false);
    ImportedTerrainList list;
    for (bvector<WString>::const_iterator iter = names.begin(); iter != names.end(); iter++)
        list.push_back (ImportedTerrain (nameDtms[*iter].get(), iter->GetWCharCP(), nullptr, false));    // ToDo set if it was from a string model or a triangulated string

    return list;
    }




BcDTMPtr MXFilImporter::ImportStringModel (ModelTableRecord* modelTableRecord) const
    {
    WString modelName (modelTableRecord->modelName ());

    BcDTMPtr dtm;
    if (m_callback && !m_callback->StartTerrain (modelName.GetWCharCP (), L"", dtm))
        return nullptr;
    if (dtm.IsNull())
        dtm = BcDTM::Create ();
    DTMFeatureId featureId;
    bvector<DPoint3d> pointList;

    MXModelFile* modelFile = modelTableRecord->getModelFile();
    // triangulation model search for the strings.
    MXModelObjectPtr<StringTable> stringTable;
    ErrorStatus es = modelTableRecord->getStringTable (stringTable.GetR(), ModelObject::Read);
    if (es == eOk)
        {
        std::unique_ptr<StringTableIterator> iter(stringTable->newIterator());
        while(!iter->done())
            {
            MXModelObjectPtr<StringTableRecord> stringTableRecord;
            if (iter->getRecord(stringTableRecord.GetR(), ModelObject::Read) == eOk)
                {
                WString stringName(stringTableRecord->stringName());
                int numDims = stringTableRecord->type() % 100;
                bool isPointString = (((numDims == 3) || (numDims == 4)) && (stringTableRecord->stringName()[0] == 'P'));
                if (stringTableRecord->stringName()[0] != '*') // If we aren't a text straing
                    {
                    // Add String Masking here! and/or callback!
                    DTMFeatureType featureType = isPointString ? DTMFeatureType::FeatureSpot : DTMFeatureType::Breakline;
                    // Need to interpret the string type and do we want to import it?
                    unsigned char* data = (unsigned char*)stringTableRecord->stringData();
                    bool isContour = false;
                    double z = 0;
                    if (stringTableRecord->isA() == MXContourString::desc())
                        {
                        MXContourString* contourString = MXContourString::cast(stringTableRecord.get());
                        isContour = true;
                        z = contourString->getContourHeight();
                        }

                    std::unique_ptr<DPoint3d[]> pointsP(new DPoint3d[stringTableRecord->numPoints()]);
                    std::unique_ptr<int[]> discosP(new int[stringTableRecord->numPoints()]);
                    DPoint3d* points = pointsP.get();
                    int* discos = discosP.get();
                    int j = 0;
                    if (((stringTableRecord->type() / 100) % 10) == 0)
                        {
                        long d[2];
                        DPoint3d pt;
                        for (int i = 1; i <= stringTableRecord->numPoints(); i++, j++)
                            {
                            int offset = ((i - 1) * stringTableRecord->dataRecordSize());

                            memcpy(&d[0], data + offset, sizeof(long) * 2);
                            pt.x = ((double)d[0]) / 1000;
                            pt.y = ((double)d[1]) / 1000;

                            if (isContour)
                                {
                                pt.z = z;
                                }
                            else
                                {
                                memcpy(&d[0], data + offset + sizeof(long) * 2, sizeof(long));
                                pt.z = ((double)d[0]) / 1000;
                                }
                            discos[j] = modelFile->ConvertXYZToWorld(pt.x, pt.y, pt.z);
                            points[j] = pt;
                            }
                        }
                    else
                        {
                        for (int i = 1; i <= stringTableRecord->numPoints(); i++, j++)
                            {
                            DPoint3d pt;
                            int offset = ((i - 1) * stringTableRecord->dataRecordSize());

                            memcpy(&pt, data + offset, sizeof(double) * 2);

                            if (isContour)
                                {
                                pt.z = z;
                                }
                            else
                                memcpy(&pt.z, data + offset + sizeof(double) * 2, sizeof(double));
                            discos[j] = modelFile->ConvertXYZToWorld(pt.x, pt.y, pt.z);
                            points[j] = pt;
                            }
                        }

                    // Add the points.
                    bool includeNULLSegments = false;
                    //bool nullInValidLevels = false;

                    int numPoints = stringTableRecord->numPoints();
                    int pointNum = 0;
                    bool inNullSegment = points[0].z < -998;
                    bool inDisco = discos[0] == eStart;
                    pointList.clear();

                    for (pointNum = 0; pointNum < numPoints; pointNum++)
                        {
                        if (inDisco)
                            {
                            if (discos[pointNum] != eEnd)
                                continue;

                            inDisco = false;
                            pointList.push_back(points[pointNum]);
                            inNullSegment = points[pointNum].z < -998;
                            continue;
                            }

                        // Skip over discos.
                        bool isPointNull = points[pointNum].z < -998;

                        if (isPointNull == inNullSegment)
                            pointList.push_back(points[pointNum]);
                        else
                            {
                            if (inNullSegment)
                                pointList.push_back(points[pointNum]);

                            if (includeNULLSegments || !inNullSegment)
                                {
                                if (inNullSegment)
                                    pointList.push_back(points[pointNum]);
                                if (DTMFeatureType::FeatureSpot == featureType)
                                    dtm->AddPointFeature(pointList.data(), (int)pointList.size(), asLong(stringTableRecord->stringName()), &featureId);
                                else
                                    dtm->AddLinearFeature(featureType, pointList.data(), (int)pointList.size(), asLong(stringTableRecord->stringName()), &featureId);
                                if (m_callback)
                                    m_callback->AddFeature(featureId, L"", L"", stringName.GetWCharCP(), L"", featureType, pointList.data(), (int)pointList.size());
                                }
                            pointList.clear();
                            if (inNullSegment)
                                pointList.push_back(points[pointNum]);
                            inNullSegment = isPointNull;
                            }

                        if (discos[pointNum] == eStart)
                            {
                            if (!pointList.empty())
                                {
                                if (includeNULLSegments || !inNullSegment)
                                    {
                                    if (DTMFeatureType::FeatureSpot == featureType)
                                        dtm->AddPointFeature(pointList.data(), (int)pointList.size(), asLong(stringTableRecord->stringName()), &featureId);
                                    else
                                        dtm->AddLinearFeature(featureType, pointList.data(), (int)pointList.size(), asLong(stringTableRecord->stringName()), &featureId);
                                    if (m_callback)
                                        m_callback->AddFeature(featureId, L"", L"", stringName.GetWCharCP(), L"", featureType, pointList.data(), (int)pointList.size());
                                    }
                                pointList.clear();
                                }
                            inDisco = true;
                            }
                        }
                    if (includeNULLSegments || !inNullSegment)
                        {
                        if (DTMFeatureType::FeatureSpot == featureType)
                            dtm->AddPointFeature(pointList.data(), (int)pointList.size(), asLong(stringTableRecord->stringName()), &featureId);
                        else
                            dtm->AddLinearFeature(featureType, pointList.data(), (int)pointList.size(), asLong(stringTableRecord->stringName()), &featureId);
                        if (m_callback)
                            m_callback->AddFeature(featureId, L"", L"", stringName.GetWCharCP(), L"", featureType, pointList.data(), (int)pointList.size());
                        }
                    pointList.clear();
                    }
                }
            iter->next();
            }
        }
        if (m_callback)
            m_callback->EndTerrain (modelName.GetWCharCP (), dtm.get ());
    return dtm;
    }

BcDTMPtr MXFilImporter::ImportTriangulation (StringTableRecord* stringTableRecord, WCharCP name) const
    {
    BcDTMPtr dtm;
    MXTriangleString* triangulationString = MXTriangleString::cast (stringTableRecord);

    if (triangulationString)
        {
        MXTriangle triangulation;

        triangulationString->loadData (&triangulation);
        MXTriangle::TriangleArray* triPtr;
        MXTriangle::PointArray* pointsPtr;
        triangulation.getPtrs (triPtr, pointsPtr);
        if (m_callback && !m_callback->StartTerrain (name, L"", dtm))
            return nullptr;
        if (dtm.IsNull ())
            dtm = BcDTM::Create ();

        BC_DTM_OBJ* dtmObjP = dtm->GetTinHandle ();
        bcdtmImport_MXTriangulationToDtmObject (dtmObjP, triPtr, pointsPtr);
        if (m_callback)
            m_callback->EndTerrain (name, dtm.get ());
        }
    return dtm;
    }


MXFilExporterPtr MXFilExporter::Create()
    {
    return new MXFilExporter();
    }

bool IsValidModelName(WCharCP name)
    {
    int i = 0;
    if (!name)
        return false;
    while (*name)
        {
        i++;
        if (*name == L' ' || *name == L'_' || (*name >= L'0' && *name <= L'9') || (*name >= L'A' && *name <= L'Z'))
            name++;
        else
            return false;
        }
    return i <= 28;
    }

bool IsValidStringName(WCharCP name)
    {
    int i = 0;
    if (!name)
        return false;
    while (*name)
        {
        i++;
        if ((*name >= L'0' && *name <= L'9') || (*name >= L'A' && *name <= L'Z'))
            name++;
        else
            return false;
        }
    return i == 4;
    }

MXFilExporter::MXExportError MXFilExporter::Export(WCharCP filename, WCharCP inModelName, WCharCP inStringName, NamedDTM const& namedDtm, bool allowOverwrite) //AddToCurrentModelFile?
    {
    BcDTMP dtm = namedDtm.GetBcDTMP();
    if (WString::IsNullOrEmpty(filename) || WString::IsNullOrEmpty(inModelName) || WString::IsNullOrEmpty(inStringName) || nullptr == dtm)
        return MXExportError::Error;

    Transform transform;
    bool hasTransform = !dtm->GetTransformation(transform);
    if (hasTransform)
        {
        DPoint3d fixedPoint;
        DPoint3d directionVector;
        double scale;
        double aspectFix;

        if (!transform.isUniformScaleAndRotateAroundLine(&fixedPoint, &directionVector, &aspectFix, &scale))
            {
            return MXExportError::Error;
            }
        else if (0 != aspectFix)
            {
            return MXExportError::Error;
            }
        }

    WString wModelName(inModelName);
    
    wModelName.ToUpper();
    if (!IsValidModelName(wModelName.c_str()))
        return MXExportError::Error;

    WString wStringName(inStringName);
    wStringName.ToUpper();
    if (!IsValidStringName(wStringName.c_str()))
        return MXExportError::Error;

    AString modelName(wModelName.c_str());
    AString stringName(wStringName.c_str());
    // Create modelFile.
    MXModelFile modelFile;

    if (modelFile.Open(filename, false, true) != eOk)
        return MXExportError::CantOpenFile;

    MXModelObjectPtr<ModelTable> modelTable;
    if (eOk != modelFile.getModelTable(modelTable.GetR(), ModelObject::Read))
        return MXExportError::Error;

    // Create Model
    MXModelObjectPtr<ModelTableRecord> modelTableRecord;
    if (eOk != modelTable->getModel(modelName.c_str(), modelTableRecord.GetR(), ModelObject::Write, false))
        {
        std::unique_ptr<ModelTableRecord> newModelTableRecord = std::make_unique<ModelTableRecord>();
        if (eOk != modelTable->addModel(modelName.c_str(), newModelTableRecord.get(), "TRIA"))
            return MXExportError::Error;

        modelTableRecord = newModelTableRecord.release();
        }
    
    if (asLong(modelTableRecord->modelType()) != asLong("TRIA"))
        return MXExportError::Error;

    MXModelObjectPtr<StringTable> stringTable;
    if (eOk != modelTableRecord->getStringTable(stringTable.GetR(), ModelObject::Write))
        return MXExportError::Error;

    MXModelObjectPtr<StringTableRecord> stringTableRecord;
    if (eOk != stringTable->getString(stringName.c_str(), stringTableRecord.GetR(), ModelObject::Write, false))
        {
        std::unique_ptr<MXTriangleString> triangleStringTableRecord = std::make_unique<MXTriangleString>();
        if (eOk != stringTable->addString(stringName.c_str(), triangleStringTableRecord.get()))
            return MXExportError::Error;

        stringTableRecord = triangleStringTableRecord.release();
        }
    else if (!allowOverwrite)
        return MXExportError::StringExists;

    MXTriangleString* triangleStringTableRecord = MXTriangleString::cast(stringTableRecord.get());

    if (nullptr == triangleStringTableRecord)
        return MXExportError::Error;

    MXTriangle mxTriangulation;
    MXTriangle::TriangleArray* triPtrP;
    MXTriangle::PointArray* pointsPtrP;
    mxTriangulation.getPtrs(triPtrP, pointsPtrP);

    if (hasTransform) // Here we need to sort out unit conversions.
        {
        for (int i = 0; i < pointsPtrP->size(); i++)
            {
            auto& point = (*pointsPtrP)[i];
            transform.Multiply(point);
            //point.x *= 3.28084;
            //point.y *= 3.28084;
            //point.z *= 3.28084;
            }
        }
    bcdtmExport_MXTriangulationFromDtmObject(dtm->GetTinHandle(), triPtrP, pointsPtrP);

    if (eOk != triangleStringTableRecord->saveData(&mxTriangulation))
        return MXExportError::Error;

    return MXExportError::Success;
    }

END_BENTLEY_TERRAINMODEL_NAMESPACE

