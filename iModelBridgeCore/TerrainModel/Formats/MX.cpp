/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/MX.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
            ModelTable* modelTable;
            ErrorStatus es = modelFile.getModelTable(modelTable, ModelObject::Read);
            if (es == eOk)
                {
                std::auto_ptr<ModelTableIterator> iter(modelTable->newIterator());
                while(!iter->done())
                    {
                    ModelTableRecord* modelTableRecord;
                    if (iter->getRecord(modelTableRecord, ModelObject::Read) == eOk)
                        {
                        WString modelName;
                        modelName.AssignA (modelTableRecord->modelName ());
                        if (modelTableRecord->modelType() == 0 || asLong(modelTableRecord->modelType()) == asLong ("    "))
                            {
                            // normal string model add this name.
                            m_surfaces.push_back (TerrainInfo (modelName.GetWCharCP(), nullptr, false));
                            }
                        if (asLong(modelTableRecord->modelType()) == asLong ("TRIA"))
                            {
                            // triangulation model search for the strings.
                            StringTable* stringTable;
                            ErrorStatus es = modelTableRecord->getStringTable (stringTable, ModelObject::Read);
                            if (es == eOk)
                                {
                                std::auto_ptr<StringTableIterator> iter(stringTable->newIterator());
                                while(!iter->done())
                                    {
                                    StringTableRecord* stringTableRecord;
                                    if (iter->getRecord(stringTableRecord, ModelObject::Read) == eOk)
                                        {
                                        // triangulation string
                                        WString stringName;
                                        stringName.AssignA (stringTableRecord->stringName ());
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
                modelTable->close();
                }
            modelFile.Close();
            }
        }
    return m_surfaces;
    }


void MXFilImporter::DoImport (bmap <WString, BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr>& nameDtms, bool importAll) const
    {
    MXModelFile modelFile;

    if (modelFile.Open (m_filename.GetWCharCP()) == eOk)
        {
        ModelTable* modelTable;
        ErrorStatus es = modelFile.getModelTable(modelTable, ModelObject::Read);
        if (es == eOk)
            {
            std::auto_ptr<ModelTableIterator> iter(modelTable->newIterator());
            while(!iter->done())
                {
                ModelTableRecord* modelTableRecord;
                if (iter->getRecord(modelTableRecord, ModelObject::Read) == eOk)
                    {
                    WString modelName;
                    modelName.AssignA (modelTableRecord->modelName ());
                    if (modelTableRecord->modelType() == 0 || asLong(modelTableRecord->modelType()) == asLong ("    "))
                        {
                        // normal string model add this name.
                        if (importAll || nameDtms.find (modelName) != nameDtms.end())
                            nameDtms[modelName] = ImportStringModel (modelTableRecord);
                        }
                    if (asLong(modelTableRecord->modelType()) == asLong ("TRIA"))
                        {
                        // triangulation model search for the strings.
                        StringTable* stringTable;
                        ErrorStatus es = modelTableRecord->getStringTable (stringTable, ModelObject::Read);
                        if (es == eOk)
                            {
                            std::auto_ptr<StringTableIterator> iter(stringTable->newIterator());
                            while(!iter->done())
                                {
                                StringTableRecord* stringTableRecord;
                                if (iter->getRecord(stringTableRecord, ModelObject::Read) == eOk)
                                    {
                                    // triangulation string
                                    WString stringName;
                                    stringName.AssignA (stringTableRecord->stringName ());
                                    WString name = stringName + modelName;
                                    if (importAll || nameDtms.find (name) != nameDtms.end())
                                        nameDtms[name] = ImportTriangulation (stringTableRecord, name.GetWCharCP());
                                    }
                                iter->next();
                                }
                            }
                        }
                    }
                iter->next();
                }
            modelTable->close();
            }
        modelFile.Close();
        }
    }

ImportedTerrain MXFilImporter::_ImportTerrain (WCharCP name) const
    {
    bmap <WString, BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr> nameDtms;
    nameDtms[name] = nullptr;
    DoImport (nameDtms, false);

    return ImportedTerrain (nameDtms[name].get(), name, nullptr, false);    // ToDo set if it was from a string model or a triangulated string
    }

ImportedTerrainList MXFilImporter::_ImportTerrains () const
    {
    bmap <WString, BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr> nameDtms;
    DoImport (nameDtms, true);
    ImportedTerrainList list;
    for (bmap <WString, BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr>::const_iterator iter = nameDtms.begin(); iter != nameDtms.end(); iter++)
        list.push_back (ImportedTerrain (iter->second.get(), iter->first.GetWCharCP(), nullptr, false));    // ToDo set if it was from a string model or a triangulated string

    return list;
    }

ImportedTerrainList MXFilImporter::_ImportTerrains (bvector<WString>& names) const
    {
    bmap <WString, BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr> nameDtms;

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
    WString modelName;
    modelName.AssignA (modelTableRecord->modelName ());

    BcDTMPtr dtm;
    if (m_callback && !m_callback->StartTerrain (modelName.GetWCharCP (), L"", dtm))
        return nullptr;
    if (dtm.IsNull())
        dtm = BcDTM::Create ();
    DTMFeatureId featureId;
    MXModelFile* modelFile = modelTableRecord->getModelFile();
    // triangulation model search for the strings.
    StringTable* stringTable;
    ErrorStatus es = modelTableRecord->getStringTable (stringTable, ModelObject::Read);
    if (es == eOk)
        {
        std::auto_ptr<StringTableIterator> iter(stringTable->newIterator());
        while(!iter->done())
            {
            StringTableRecord* stringTableRecord;
            if (iter->getRecord(stringTableRecord, ModelObject::Read) == eOk)
                {
                WString stringName;
                stringName.AssignA (stringTableRecord->stringName ());
                int numDims = stringTableRecord->type() % 100;
                bool isPointString = (((numDims == 3) || (numDims == 4)) && (stringTableRecord->stringName()[0] == 'P'));
                // Add String Masking here! and/or callback!
                DTMFeatureType featureType = isPointString ? DTMFeatureType::FeatureSpot : DTMFeatureType::Breakline;
                // Need to interpret the string type and do we want to import it?
                unsigned char* data = (unsigned char*)stringTableRecord->stringData();
                bool isContour = false;
                double z = 0;
                if(stringTableRecord->isA() == MXContourString::desc())
                    {
                    MXContourString* contourString = MXContourString::cast(stringTableRecord);
                    isContour = true;
                    z = contourString->getContourHeight();
                    }

                std::auto_ptr<DPoint3d> pointsP (new DPoint3d [stringTableRecord->numPoints()]);
                std::auto_ptr<int> discosP (new int[stringTableRecord->numPoints()]);
                DPoint3d* points = pointsP.get();
                int* discos = discosP.get();
                int j = 0;
                if (((stringTableRecord->type() / 100) % 10) == 0)
                    {
                    long d[2];
                    DPoint3d pt;
                    for(int i = 1; i <= stringTableRecord->numPoints(); i++, j++)
                        {
                        int offset = ((i - 1) * stringTableRecord->dataRecordSize());

                        memcpy (&d[0], data + offset, sizeof(long) * 2);
                        pt.x = ((double)d[0]) / 1000;
                        pt.y = ((double)d[1]) / 1000;

                        if(isContour)
                            {
                            pt.z = z;
                            }
                        else
                            {
                            memcpy (&d[0], data + offset + sizeof(long) * 2, sizeof(long));
                            pt.z = ((double)d[0]) / 1000;
                            }
                        discos[j] = modelFile->ConvertXYZToWorld (pt.x, pt.y, pt.z);
                        points[j] = pt;
                    }
                }
                else
                    {
                    for(int i = 1; i <= stringTableRecord->numPoints(); i++, j++)
                        {
                        DPoint3d pt;
                        int offset = ((i - 1) * stringTableRecord->dataRecordSize());

                        memcpy (&pt, data + offset, sizeof(double) * 2);

                        if(isContour)
                            {
                            pt.z = z;
                            }
                        else
                            memcpy (&pt.z, data + offset + sizeof(double) * 2, sizeof(double));
                        discos[j] = modelFile->ConvertXYZToWorld (pt.x, pt.y, pt.z);
                        points[j] = pt;
                        }
                    }

                // Add the points.

                int numPoints = stringTableRecord->numPoints();
                int pointNum = 0;
                int startPt = -1;
                bool inNullSegment = false;
                bool inDisco = discos[0] == eStart;
                bool includeNULLSegments = false;
                //bool nullInValidLevels = false;

                while (pointNum < numPoints)
                    {
                    // If start disco detected, read forward until end disco detected
                    while ((pointNum < numPoints)
                        && (inDisco))
                        {            

                        // Report a disco at the start of string as a point
                        if (pointNum == 0)
                            {
                            if (startPt == -1)
                                startPt = pointNum;

                            // Add callback...
                            dtm->AddLinearFeature (featureType, &points[startPt], pointNum - startPt + 1, asLong (stringTableRecord->stringName()), &featureId);
                            if (m_callback)
                                m_callback->AddFeature (featureId, L"", L"", stringName.GetWCharCP (), L"", featureType, &points[startPt], pointNum - startPt + 1);
                            startPt = -1;
                            }

                        if (discos[pointNum] == eEnd)
                            {
                            inDisco = false;
                            break;
                            }

                        pointNum++;
                        }             

                    // If null detected and no disco, read forward till valid level or disco detected
                    while ((pointNum < numPoints) 
                        && (points[pointNum].z < -998) 
                        && (discos[pointNum] != eStart)
                        && (discos[pointNum] != eBearing))
                        {
                        inNullSegment = true;
                        if (startPt == -1)
                            startPt = pointNum;
                        pointNum++;
                        }

                    // End of null segment - include the point the finishes the null segement (valid level, start disco or bearing disco
                    if (inNullSegment)
                        {
                        if (includeNULLSegments)
                            {

                            if (pointNum < numPoints)
                                {
                                if (startPt == -1)
                                    startPt = pointNum;
                                if (discos[pointNum] == eStart)
                                    inDisco = true;
                                }

                            //if (nullInValidLevels)
                            //    {
                            //    pointList[0].Z = -999;
                            //    pointList[pointList->Count - 1].Z = -999;
                            //    }

                            if (startPt == -1)
                                startPt = pointNum;
                            // Add callback...
                            dtm->AddLinearFeature (featureType, &points[startPt], pointNum - startPt + 1, asLong (stringTableRecord->stringName()), &featureId);
                            if (m_callback)
                                m_callback->AddFeature (featureId, L"", L"", stringName.GetWCharCP (), L"", featureType, &points[startPt], pointNum - startPt + 1);
                            }
                        startPt = -1;

                        inNullSegment = false;
                        }

                    // Start of disco detected - back top main loop
                    if (inDisco)
                        {
                        pointNum++;
                        continue;              
                        }

                    // Normal level must be detected - read forward till null level, bearing or start disco
                    while (pointNum < numPoints)
                        {
                        // Null level detected - break back to start of loop
                        if (points[pointNum].z < -998)
                            break;

                        // Point is ok to add - might be bearing disco or start disco
                        if (startPt == -1)
                            startPt = pointNum;

                        if (discos[pointNum] == eStart)
                            {
                            inDisco = true;
                            break;
                            }

                        pointNum++;

                        }
        
                    // Add the level points to results
                    if (startPt != -1)
                        {
                        // Add callback...
                        dtm->AddLinearFeature (featureType, &points[startPt], pointNum - startPt, asLong (stringTableRecord->stringName()), &featureId);
                        if (m_callback)
                            m_callback->AddFeature (featureId, L"", L"", stringName.GetWCharCP (), L"", featureType, &points[startPt], pointNum - startPt);
                        startPt = -1;
                        }
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

END_BENTLEY_TERRAINMODEL_NAMESPACE

