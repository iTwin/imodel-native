/*-------------------------------------------------------------------------------------+
|
|     $Source: Formats/MXModelFile.cpp $
|    $RCSfile: modelfile.cpp,v $
|   $Revision: 1.10 $
|       $Date: 2012/08/16 16:17:17 $
|     $Author: Daryl.Holmwood $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*
* TR        DH  10/03/02 Fixed archiving of text strings.
*/
#include <Bentley\BeTimeUtilities.h>
#include "MXModelFile.h"
#include "mxtriangle.h"

#define asLong(a) *((long*)a)

static inline long ConvertStringName(const char* const stringName)
    {
    return (stringName[0] << 24) | (stringName[1] << 16) | (stringName[2] << 8) | stringName[3];
    }
static int CurTime = 1;

static inline double fabs2(const double& value)
    {
    if(value < 0)
        return -value;
    return value;
    }
#define fabs fabs2

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE


int addModelFile(MXModelFile* modelFile);
void removeModelFile(int number);
int addObject(int number, ModelObject* object);
void removeObject(int number, int objectNumber);

#ifdef MXEVENTS
ArrayClass<ModelFileEvents*> s_events;
void AddModelFileEventHandler(ModelFileEvents* mfeh)
    {
    int i;

    for(i = 0; i < s_events.size(); i++)
        if(s_events[i] == mfeh)
            return;
    s_events.append_entry(mfeh);
    }

void RemoveModelFileEventHandler(ModelFileEvents* mfeh)
    {
    int i;

    for(i = 0; i < s_events.size(); i++)
        {
        if(s_events[i] == mfeh)
            {
            s_events.delete_entry(i);
            return;
            }
        }
    }

static void StringModified(StringTableRecord* str)
    {
    int i;

    for(i = 0; i < s_events.size(); i++)
        s_events[i]->StringModified(str);
    }


static void StringAdded(ModelTableRecord* mtr, StringTableRecord* str)
    {
    int i;

    for(i = 0; i < s_events.size(); i++)
        s_events[i]->StringAdded(mtr, str);
    }

static void StringErased(ModelTableRecord* mtr, StringTableRecord* str)
    {
    int i;

    for(i = 0; i < s_events.size(); i++)
        s_events[i]->StringErased(mtr, str);
    }

static void StringRenaming(StringTableRecord* str, char* newName)
    {
    int i;

    for(i = 0; i < s_events.size(); i++)
        s_events[i]->StringRenaming(str, newName);
    }

static void ModelAdded(ModelTableRecord* mtr)
    {
    int i;

    for(i = 0; i < s_events.size(); i++)
        s_events[i]->ModelAdded(mtr);
    }

static void ModelErased(ModelTableRecord* mtr)
    {
    int i;

    for(i = 0; i < s_events.size(); i++)
        s_events[i]->ModelErased(mtr);
    }

static void ModelRenaming(ModelTableRecord* mtr, char* oldName)
    {
    int i;

    for(i = 0; i < s_events.size(); i++)
        s_events[i]->ModelRenaming(mtr, oldName);
    }
#endif
static inline bool compareTypes(int type1, int type2)
    {
    int dim1 = type1 % 100;

    if(dim1 != type2 % 100)
        return false;

    int mod = (1 >> (dim1 % 4)) * 100;
    if(!mod)
        mod = 100;
    dim1 /= 4;

    for(int i = 0; i < dim1; i++)
        mod *= 10;
    return (type1 % mod) == (type2 % mod);
    }

static inline void trim(char* string)
    {
    int len = (int)strlen(string) - 1;

    while(string[len] == ' ' && len >= 0)
        {
        string[len--] = 0;
        }
    }

static bool isValidModelName(const char* const modelName)
    {
    int len = (int)strlen(modelName);
    if(len == 0 || len > 28)
        return false;
    return true;
    }

static void fixModelName(char* modelName, const char* const inModelName)
    {
    strncpy(modelName, inModelName, 28);
    modelName[28] = 0;
    strupr(modelName);
    trim(modelName);
    }

MXRunTimeObject* MXRunTimeClass::create(void)
    {
    if(_makeFunction)
        return _makeFunction();
    return NULL;
    }
int MXRunTimeClass::isDerivedFrom(const MXRunTimeClass* classPtr)
    {
    if(classPtr == this)
        return 1;
    if(_parent)
        return _parent->isDerivedFrom(classPtr);
    return 0;
    }

MXRunTimeClass* ClassList::at(const char* const className)
    {
    int i;

    for(i = 0; i < _classes.size(); i++)
        {
        if(strcmp(_classes[i]._className, className) == 0)
            {
            return &_classes[i];
            }
        }
    return NULL;
    }

MXRunTimeClass* ClassList::getType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    int i;

    for(i = 0; i < _classes.size(); i++)
        {
        if(_classes[i]._isType && _classes[i]._isType(str, modelType))
            {
            return &_classes[i];
            }
        }
    return NULL;
    }

MXRunTimeClass* ClassList::newMXRunTimeClass(const char* const className, const char* const parentClass, IsTypeFunction isType, ClassMakeFunction makeFunction)
    {
    if(at(className))
        return NULL;

    MXRunTimeClass* rtClass = &_classes[_classes.size()];
    rtClass->_className = strdup(className);
    rtClass->_parent = at(parentClass);
    rtClass->_isType = isType;
    rtClass->_makeFunction = makeFunction;
    return rtClass;
    }

void ClassList::deleteClass(MXRunTimeClass* classPtr)
    {
    int i;
    for(i = 0; i < _classes.size(); i++)
        {
        if(classPtr == &_classes[i])
            {
            delete [] classPtr->_className;
            _classes.replace_delete(i);
            return;
            }
        }
    }
ClassList::ClassList()
    {
    cl_classList = this;
    // Register Standard classes.
    ModelTable::rtInit();
    ModelTableRecord::rtInit();
    StringTable::rtInit();
    StringTableRecord::rtInit();
    MXTriangleString::rtInit();
    MXTextString::rtInit();
    MXContourString::rtInit();
#ifdef MXALLSTRINGTYPES
    MXCadastreString::rtInit();
    MXAreaString::rtInit();
    MXEndAreaString::rtInit();
    MXVolumeString::rtInit();
    MXVisibilityString::rtInit();
    MX3dString::rtInit();
    MXSurveyStationString::rtInit();
    MXSurveyObservationString::rtInit();
    MXSectionString::rtInit();
    MXInterfaceString::rtInit();
    MXMasterString::rtInit();
    MXGeometryString::rtInit();
#endif
    MXProxyString::rtInit();
    }
ClassList* ClassList::cl_classList = 0;

MXRunTimeClass* newMXRunTimeClass(const char* const className, const char* const parentClass, IsTypeFunction type, ClassMakeFunction makeFunction)
    {
    return ClassList::Instance().newMXRunTimeClass(className, parentClass, type, makeFunction);
    }

MXRunTimeClass* MXRunTimeObject::isA() const
    {
    return NULL;
    }

int MXRunTimeObject::isKindOf(MXRunTimeClass* desc) const
    {
    MXRunTimeClass* c;
    return ((c = isA()) == NULL) ? 1: c->isDerivedFrom(desc);
    }

MXModelFile::MXModelFile()
    {
#ifdef FREESPACEMAPPER
    _noFreeSpaceMapper = false;
#endif
    _convertToWorld = FALSE;
    _swapXY = FALSE;
    _swapNeg = false;
    _scaleFactor = 1.0;
    _doScaleFactor = false;
    initalize();
    }

MXModelFile::~MXModelFile()
    {
    Close();
    }

ErrorStatus MXModelFile::Close(void)
    {
#ifdef FREESPACEMAPPER
    if(_freeSpaceMapper)
        _freeSpaceMapper->report();
    if(_freeSpaceMapper)
        {
        delete _freeSpaceMapper;
        _freeSpaceMapper = NULL;
        }
#endif
    if(_modelFileNumber)
        {
        removeModelFile(_modelFileNumber);
        _modelFileNumber = 0;
        }

    if(_modelTable)
        {
        delete _modelTable;
        _modelTable = NULL;
        }
    if(_fp)
        {
        fclose(_fp);
        _fp = NULL;
        }
    return eOk;
    }

void MXModelFile::initalize(void)
    {
    _fp = NULL;
    _modelTable = NULL;
    _nextRecord = 0;
    _nextDataRecord.set(0, 1);
    _modelFileNumber = 0;
#ifdef FREESPACEMAPPER
    _freeSpaceMapper = NULL;
#endif
    _modelShiftX    = 0.0;
    _modelShiftY    = 0.0;
    }

bool checkValidModel(ModelFileRecord& rec1, ModelFileRecord& rec2)
    {
//    long iflsiz = rec1.pad[0];
    long imvuse = rec1.pad[1];
    long ist502 = rec2.lastRecord;
    long ist503 = rec2.pad[0];
//    long ist504 = rec2.pad[1];

    // DECODE THE MOSS VERSION AND NUMBER OF USES SINCE MODELFILE CREATE
//    long numuse = imvuse % 100000;
    long mver = imvuse / 100000;

    //    DECODE LDATE AND LUSE.
    long ltemp = -ist502;
    long ntemp;
    int i;
    int idigs[7];

    for(i = 0; i < 7; i++)
        {
        ntemp = ltemp / 10;
        idigs[i] = ltemp - 10 * ntemp;
        ltemp = ntemp;
        }

    if(ltemp != 0)
        return false;

//    long ldate = (1000 * idigs[6] + 100 * idigs[5] + 10 * idigs[0] + idigs[3]) / 3;
    long luse = (100 * idigs[2] + 10 * idigs[1] + idigs[4]) - 121;

    // DECODE LDATED AND LUSED.

    ltemp = -ist503;
    for(i = 0; i < 7; i++)
        {
        ntemp = ltemp / 10;
        idigs[i] = ltemp - 10 * ntemp;
        ltemp = ntemp;
        }

    if(ltemp != 0)
        return false;

    long ldated = (1000 * idigs[6] + 100 * idigs[5] + 10 * idigs[0] + idigs[3]);
    if(mver < 92)
        ldated = ldated / 3;

    long lused = 100 * idigs[2] + 10 * idigs[1] + idigs[4] - 115;

    if(luse < 0 || luse > 850 || ldated < 0 || lused < 0 || lused > 850)
        return false;

    return true;
    }

ErrorStatus MXModelFile::UpgradeOpen()
    {
    if(!_readonly)
        return eOk;

    fclose(_fp);

    _fp = fopen(_filename.GetWCharCP(), "r+b");
    return eOk;
    }

ErrorStatus MXModelFile::Open(WCharCP filename, bool readonly, bool create)
    {
    Close();

    initalize();
    _readonly = readonly;
    if(readonly)
        _fp = fopen(filename, "rb");
    else
        {
        _fp = fopen(filename, "r+b");
        if(!_fp && create)
            {
            _fp = fopen(filename, "w+b");
            if(!_fp)
                return eCantOpenFile;
            ModelFileRecord mt;

            memset((void*)&mt, 0, RecordSize);
            mt.nextRecord = 1;
            mt.lastRecord = 2;
            mt.pad[0] = 0xfffb1e00;
            mt.pad[1] = 0x01406f41;
            fwrite((void*)&mt, RecordSize, 1, _fp);
            memset((void*)&mt, 0, RecordSize);
            mt.lastRecord = stringTableType;
            mt.pad[0] = 0xffdbed2d;
            fwrite((void*)&mt, RecordSize, 1, _fp);
            }
        }
    if(!_fp)
        return eCantOpenFile;

    _filename = filename;
    _modelFileNumber = addModelFile(this);
#ifdef FREESPACEMAPPER
    if(!_readonly && !_noFreeSpaceMapper)
        _freeSpaceMapper = new ModelFileFreeSpace;
#endif
    ModelFileRecord mt;
    ModelFileRecord mt2;
    readRecord(&mt, 1);
    readRecord(&mt2, 2);

    bool bIsValid = checkValidModel(mt, mt2);
    _nextRecord = mt.lastRecord;
    writeRecord(&mt,1);

    if(mt.pad[0] <= -60000000)
        _maxRecord = -(mt.pad[0] + 600000000);
    else if(mt.pad[0] <= -50000000)
        _maxRecord = -(mt.pad[0] + 50000000);
    else if(mt.pad[0] <= -4000000)
        _maxRecord = -(mt.pad[0] + 4000000);
    else if(mt.pad[0] <= -300000)
        _maxRecord = -(mt.pad[0] + 300000);
    else
        bIsValid = false;

    _nextDataRecord.set(mt.lastRecord, 1);

    // Validate that this is a valid model file.

    if(_maxRecord < _nextRecord)
        bIsValid = false;

    //    
    for(int i = 0; i < 50; i++)
        {
        if(mt.models[i].stringTablePos > _nextRecord || mt.models[i].stringTablePos < 0)
            bIsValid = false;
        }    


    if(!bIsValid)
        {
        fclose(_fp);
        return eCantOpenFile;
        }

    _modelTable = new ModelTable(this);
    _modelTable->close();

#ifdef FREESPACEMAPPER
    if(_freeSpaceMapper)
        {
        ModelFilePos pos;
        _freeSpaceMapper->getFreeData(1, pos, true);
        if(pos.getRecordNumber() == 2)
            AddUsedRecord(pos.getRecordNumber());

        }
#endif
    return eOk;
    }


void MXModelFile::updateNextRecord(void)
    {
    if(!_fp)
        return;
    fseek(_fp, 2004,SEEK_SET);
    fwrite(&_nextRecord, 4, 1, _fp);
    if(_maxRecord < _nextRecord)
        {
        _maxRecord = 1 + (_nextRecord / 10000);
        _maxRecord *= 10000;
        int tmp;
        if(_maxRecord > 9999999)
            tmp = -(600000000 + _maxRecord);
        else if(_maxRecord > 999999)
            tmp = -(4000000 + _maxRecord);
        else
            tmp = -(300000 + _maxRecord);
        fwrite(&tmp, 4, 1, _fp);
        }

    }

int MXModelFile::readRecord2(ModelFileRecord* buffer, int recordNumber)
    {
    if(!_fp)
        return 1;

    __int64 pos = (recordNumber - 1);
    pos *= RecordSize;
    if(ftell(_fp) != pos)
        fseek(_fp, pos, SEEK_SET);
    fread(buffer, sizeof(ElementTriangleStringDoubles), 10, _fp);
    return 0;
    }

int MXModelFile::readRecord(ModelFileRecord* buffer, int recordNumber)
    {
    if(!_fp)
        return 1;

    __int64 pos = (recordNumber - 1);
    pos *= RecordSize;
    if(ftell(_fp) != pos)
        fseek(_fp, pos, SEEK_SET);
    fread(buffer, RecordSize, 1, _fp);
    return 0;
    }

int MXModelFile::readRecords(ModelFileRecord* buffer, int recordNumber, int numRecords)
    {
    if(!_fp)
        return 1;

    __int64 pos = (recordNumber - 1);
    pos *= RecordSize;
    if(ftell(_fp) != pos)
        fseek(_fp, pos, SEEK_SET);
    fread(buffer, RecordSize, numRecords, _fp);
    return 0;
    }

int MXModelFile::readRecordOffsetAndSize(void* buffer, int recordNumber, int offset, int size)
    {
    if(!_fp)
        return 1;

    __int64 pos = (recordNumber - 1);
    pos *= RecordSize;
    pos += offset;
    if(ftell(_fp) != pos)
        fseek(_fp, pos, SEEK_SET);
    fread(buffer, size, 1, _fp);
    return 0;
    }

int MXModelFile::writeRecords(ModelFileRecord* buffer, int recordNumber, int numRecords)
    {
    if(!_fp)
        return 1;

    if(recordNumber == 1)
        buffer->lastRecord = _nextRecord;
    __int64 filePos = (recordNumber - 1);
    filePos *= RecordSize;
    if(ftell(_fp) != filePos)
        fseek(_fp, filePos, SEEK_SET);
    fwrite(buffer, RecordSize, numRecords, _fp);
    return 0;
    }

int MXModelFile::writeRecord(ModelFileRecord* buffer, int recordNumber)
    {
    if(!_fp)
        return 1;

    if(recordNumber == 1)
        buffer->lastRecord = _nextRecord;
    __int64 filePos = (recordNumber - 1);
    filePos *= RecordSize;
    if(ftell(_fp) != filePos)
        fseek(_fp, filePos, SEEK_SET);
    fwrite(buffer, RecordSize, 1, _fp);
    return 0;
    }

int MXModelFile::readData(void* data, const ModelFilePos& pos, const unsigned long dataSize)
    {
    ModelFileRecord rec;
    char* dataP = (char*)data;
    long recordNumber = pos.getRecordNumber();
    long offset = (pos.getRecordPos() - 1) * 4;
    long i = 0;

    while(1)
        {
        readRecord(&rec, recordNumber);
        if(dataSize - i + offset > 2000)
            {
            memcpy(&dataP[i], &rec.data[offset], 2000 - offset);
            i += 2000 - offset;
            recordNumber++;
            offset = 0;
            }
        else
            {
            memcpy(&dataP[i], &rec.data[offset], dataSize - i);
            break;
            }
        }
    return 0;
    }

int MXModelFile::writeData(void* data, const ModelFilePos& pos, const unsigned long dataSize)
    {
    ModelFileRecord rec;
    char* dataP = (char*)data;
    long recordNumber = pos.getRecordNumber();
    long offset = (pos.getRecordPos() - 1) * 4;
    long i = 0;

    while(1)
        {
        readRecord(&rec, recordNumber);
        if(dataSize - i + offset > 2000)
            {
            memcpy(&rec.data[offset], &dataP[i], 2000 - offset);
            i += 2000 - offset;
            writeRecord(&rec, recordNumber);
            recordNumber++;
            offset = 0;
            }
        else
            {
            memcpy(&rec.data[offset], &dataP[i], dataSize - i);
            writeRecord(&rec, recordNumber);
            offset += (((dataSize - i) + 3) / 4) * 4;
            break;
            }
        }
    if(_nextRecord <= recordNumber)
        {
        _nextRecord = recordNumber + 1;
        updateNextRecord();
        }

    if(_nextDataRecord.getRecordNumber() < recordNumber)
        {
        _nextDataRecord.set(recordNumber, (short)(1 + (offset / 4)));
        }
    else if(_nextDataRecord.getRecordNumber() == recordNumber && _nextDataRecord.getRecordPos() < offset)
        {
        _nextDataRecord.setRecordPos((short)(1 + (offset / 4)));
        }
    return 0;
    }

void MXModelFile::getFreeData(unsigned long dataSize, ModelFilePos& pos, bool startNewRecord)
    {
#ifdef FREESPACEMAPPER
    if(_freeSpaceMapper)
        _freeSpaceMapper->getFreeData(dataSize, pos, startNewRecord);
    else
#endif
        {
        if(startNewRecord)
            {
            pos.setRecordNumber(_nextRecord);
            pos.setRecordPos(1);
            _nextDataRecord.setRecordNumber(_nextRecord + 1);
            _nextDataRecord.setRecordPos(1);
            }
        else
            pos = _nextDataRecord;
        }
    }


ErrorStatus MXModelFile::getModelTable(ModelTable*& modelTable, ModelObject::OpenStatus mode)
    {
    ErrorStatus ret = _modelTable->open(mode);

    if(ret != eOk)
        {
        modelTable = NULL;
        return ret;
        }

    modelTable = _modelTable;
    return ret;
    }

ErrorStatus MXModelFile::getModelTable(ModelObjectId& modelTableId)
    {
    modelTableId = _modelTable->objectId();
    return eOk;
    }

ErrorStatus MXModelFile::SetModelToWorld(eCoordSystems modelCoord, eCoordSystems outputCoord, double scaleFactor)
    {
    bool modelXY = FALSE;
    bool outputXY = FALSE;

    if(outputCoord == eNoSystem)
        _convertToWorld = false;
    else
        _convertToWorld = true;

    setScaleFactor(scaleFactor);
    switch(modelCoord)
        {
        //        case eDane:
        case eCzech1:
        case eCzech2:
            //        case eJapanese:
            modelXY = TRUE;
            break;
        }
    if(outputCoord == eNoSystem)
        outputCoord = modelCoord;

    switch(outputCoord)
        {
        case eYX:
            //        case eDane:
        case eCzech1:
        case eCzech2:
            //        case eJapanese:
            outputXY = TRUE;
            break;
        }

    if(outputXY != modelXY)
        _swapXY = true;
    else
        _swapXY = false;

    if(outputCoord == eCAD)
        {
        _swapXY = false;
        _swapNeg = (modelCoord == eCzech2 || outputCoord == eCzech2) && (modelCoord != outputCoord);
        //						if(_swapNeg)
        //							_swapXY = !_swapXY;
        }
    // Need to sort out angles.
    return eOk;
    }


// ModelObject
ErrorStatus ModelObject::open(OpenStatus mode)
    {
    if(!_modelFile)
        return eOk;
    if(mode == Notify)
        return eOk;
    if(mode == Read)
        {
        if(opened < 0)
            return eOpenedForWrite;
        opened++;
        }
    else
        {
        if(_modelFile && _modelFile->isReadOnly())
            return eReadOnly;
        if(opened != 0)
            return eOpenedForWrite;
        opened = -1;
        }
    return eOk;
    }

ErrorStatus ModelObject::close()
    {
    if(!isInModelFile())
        return eNotInModelFile;

    if(isModified() || isNewRecord())
        {
        UpdateData();
        if(!_delayWrite)
            {
            UpdateFile();
            _fileNeedsUpdating = false;
            }
        else
            _fileNeedsUpdating = true;
        }

    if(opened < 0)
        {
        modified = 0;
        opened = 0;
        }
    else if(opened > 0)
        opened--;
    else
        return eNotOpened;

    if(!_id.objectNumber)
        _id.objectNumber = addObject(_id.modelFileNumber, this);
    _new = false;
    return eOk;
    }

void ModelObject::assertWriteEnabled()
    {
    if(opened >= 0)
        assert(1);
    if(_pos.getRecordNumber() != 0)
        {
        modified = 1;
        }
    }

ErrorStatus ModelObject::erase(int erase)
    {
    if(erase == erased) return eOk;
    assertWriteEnabled();
    if(erase)
        erased = 1;
    else
        {
        _new = true;
        erased = 0;
        }
    return eOk;
    }

void ModelObject::setDelayWrite(bool val)
    {
    assertReadEnabled();
    if(!val && _delayWrite == 1 && _fileNeedsUpdating)
        {
        UpdateFile();
        _fileNeedsUpdating = false;
        }
    if(!val)
        _delayWrite--;
    else
        _delayWrite++;
    }

ErrorStatus ModelObject::upgradeOpen()
    {
    if(_modelFile->isReadOnly())
        return eReadOnly;
    if(opened == 1 || opened < 0)
        {
        opened = -1;
        return eOk;
        }
    return eNotOpened;
    }

ErrorStatus ModelObject::downgradeOpen()
    {
    if(opened != 0)
        {
        if(opened < 0)
            opened = 1;
        return eOk;
        }
    return eNotOpened;
    }

ModelObject::ModelObject()
    {
    opened = -1;
    erased = 0;
    modified = 0;
    _modelFile = 0;
    _delayWrite = false;
    _fileNeedsUpdating = false;
#ifdef _MANAGED
    _notifyObj = new ObjectNotify;
#endif
    _new = true;
    }

void ModelObject::detach()
    {
    opened = -1;
    erased = 0;
    modified = 0;
    _modelFile = 0;
    _delayWrite = false;
    _fileNeedsUpdating = false;
#ifdef _MANAGED
    getObjectNotify()->Clear();
#endif
    }

void ModelObject::Release()
    {
    if(_fileNeedsUpdating)
        {
        opened = -1;
        UpdateFile();
        _fileNeedsUpdating = false;
        opened = 0;
        }
    }

ModelObject::~ModelObject()
    {
#ifdef _MANAGED
    if(_notifyObj)
        {
        delete _notifyObj;
        _notifyObj = 0;
        }
#endif

    if(_id.objectNumber && _modelFile->getModelFileNumber() != 0)
        removeObject(_id.modelFileNumber, _id.objectNumber);
    }

void ModelObject::UpdateFile()
    {
    }

void ModelObject::UpdateData()
    {
    }
// ModelTable


MXRTObject_DEFINE_MEMBERS(ModelTable, ModelObject);

ModelTable::ModelTable(MXModelFile* modelFile)
    {
    ModelFileRecord mt;
    int firstRecord = 1;
    int recordNumber = 1;
    int i;

    setModelFilePos(ModelFilePos(1, 1));
    setModelFile(modelFile);
    _modelFile->readRecord(&mt, recordNumber);
#ifdef FREESPACEMAPPER
    _modelFile->AddUsedRecord(recordNumber);
#endif
    do
        {
        for(i = 0; i < 50; i++)
            {
            ModelTableRecord* mr;

            mr = new ModelTableRecord;
            _models[_models.size()] = mr;
            mr->set(mt.models[i]);
            mr->setModelTable(this);
            mr->setModelFile(modelFile);

            if(!mt.models[i].stringTablePos)
                {
                mr->ModelObject::erase(1);
                }
            else
                {
                //  Enable This to automaticaly load in the stringTable for every MXModel
#ifdef FREESPACEMAPPER
                if(_modelFile->isReusingFreeSpace())
                    {
                    StringTable* st;
                    if(mr->getStringTable(st, ModelObject::Write) == eOk)
                        st->close();
                    }
#endif
                }
            mr->setModelFilePos(ModelFilePos(recordNumber, i + 1));
            mr->close();
            }

        if(mt.nextRecord == firstRecord)
            break;
        recordNumber = mt.nextRecord;
        _modelFile->readRecord(&mt, mt.nextRecord);
#ifdef FREESPACEMAPPER
        _modelFile->AddUsedRecord(recordNumber);
#endif
        } while(1);
    }

ModelTable::~ModelTable()
    {
    Release();
    int i;

    for(i = 0; i < (int)_models.size(); i++)
        {
        _models[i]->Release();
        delete _models[i];
        }
    _models.setLogicalLength(0);
    }

ModelTableIterator* ModelTable::newIterator()
    {
    return new ModelTableIterator(this);
    }

void ModelTable::DisposeU(ModelTableIterator *pModelTableIterator)
    {
    delete pModelTableIterator;
    }

int ModelTable::getModelIndex(const char* const modelName, bool getDeleted) const
    {
    int i;
    const ModelTableRecord* const* mt = &_models[0];
    const int numModels = (int)_models.size();

    for(i = 0; i < numModels; i++)
        {
        if(strcmp(modelName, mt[i]->_modelName) == 0)
            {
            if(getDeleted || !mt[i]->isErased())
                return i;
            }
        }
    return -1;
    }

ErrorStatus ModelTable::getModel(const char* const modelName, ModelTableRecord*& modelTableRecord, ModelObject::OpenStatus mode, bool getDeleted)
    {
    assertReadEnabled();
    if(strlen(modelName) == 0)
        return eKeyNotFound;

    int i = getModelIndex(modelName, getDeleted);
    if(i != -1)
        {
        ModelTableRecord** mt = &_models[0];
//        const int numModels = (int)_models.size();
        ErrorStatus ret = mt[i]->open(mode);

        if(ret != eOk)
            modelTableRecord = NULL;
        else
            modelTableRecord = mt[i];
        return ret;
        }
    return eKeyNotFound;
    }

ErrorStatus ModelTable::getModel(const char* const modelName, ModelObjectId& modelTableRecordId, bool getDeleted)
    {
    assertReadEnabled();
    if(strlen(modelName) == 0)
        return eKeyNotFound;

    int i = getModelIndex(modelName, getDeleted);
    if(i != -1)
        {
        const ModelTableRecord* const* mt = &_models[0];
//        const int numModels = (int)_models.size();
        modelTableRecordId = mt[i]->objectId();
        return eOk;
        }
    return eKeyNotFound;
    }

ErrorStatus ModelTable::addModel(const char* const inModelName, ModelTableRecord*& mr, const char* const modelType)
    {
    char modelName[33];
    int i;
    assertWriteEnabled();

    if(!isValidModelName(inModelName))
        return eInvalidData;

    if(modelType)
        {
        if(strlen(modelType) != 4)
            return eInvalidData;
        }

    fixModelName(modelName, inModelName);

    for(i = 0; i < (int)_models.size(); i++)
        {
        if(strcmp(_models[i]->_modelName, modelName) == 0)
            break;
        }

    if(i == _models.size())
        {
        for(i = 0; i < (int)_models.size(); i++)
            {
            if(_models[i]->isErased())
                break;
            }
        if(i == _models.size())
            {
            // Need to add a new modelFile Record into the modelfile and append new entries into the _models array.

            ModelFileRecord mt;
            int newRecordNumber = mt.nextRecord = _modelFile->addRecord();
#ifdef FREESPACEMAPPER
            _modelFile->AddUsedRecord(newRecordNumber);
#endif
            int recordNumber = _models[i - 1]->getModelFilePos().getRecordNumber();
            _modelFile->readRecord(&mt, recordNumber);
            int firstRecord = mt.nextRecord;
            mt.nextRecord = newRecordNumber;
            _modelFile->writeRecord(&mt, recordNumber);

            mt.nextRecord = firstRecord;

            for(int j = 0; j < 50; j++)
                {
                memset(mt.models[j].modelName, ' ', 32);
                mt.models[j].stringTablePos = 0;
                mt.models[j].date = 0;

                ModelTableRecord* mr;
                mr = new ModelTableRecord;
                _models[_models.size()] = mr;
                mr->set(mt.models[j]);
                mr->setModelTable(this);
                mr->setModelFile(_modelFile);
                mr->ModelObject::erase(1);
                mr->setModelFilePos(ModelFilePos(newRecordNumber, j + 1));
                mr->close();
                }
            _modelFile->writeRecord(&mt, newRecordNumber);
            }
        }
    else
        {
        if(!_models[i]->isErased())
            return eDuplicateName;
        _models[i]->erase(0);
        }

    int recordNumber = _models[i]->getModelFilePos().getRecordNumber();
    int recordEntry = _models[i]->getModelFilePos().getRecordPos() - 1;
    /*
    ModelFileRecord mt;

    // Find Next Free MXModel Record.
    _modelFile->readRecord(&mt, recordNumber);

    memset(mt.models[recordEntry].modelName, ' ', 32);
    strncpy(mt.models[recordEntry].modelName, modelName, strlen(modelName));
    mt.models[recordEntry].stringTablePos = _modelFile->addRecord();
    mt.models[recordEntry].date = 0;
    _modelFile->writeRecord(&mt, recordNumber);

    // Initalize stringTable
    ModelFileRecord str;

    memset(&str, 0, sizeof(str));
    for(int j = 0; j < 50; j++)
    memset(str.strings[j].stringName, ' ', 8);

    str.nextRecord = mt.models[recordEntry].stringTablePos;
    str.lastRecord = stringTableType;
    str.pad[0] = 0xffdbed2d;

    _modelFile->writeRecord(&str, mt.models[recordEntry].stringTablePos);
    */
    delete _models[i];

    _models[i] = mr;
    //    mr->set(/*mt.models[recordEntry].*/modelName, ModelFilePos(0/*mt.models[recordEntry].stringTablePos*/, 0), 0); //mt.models[recordEntry].date);
    mr->setModelFilePos(ModelFilePos(recordNumber, recordEntry + 1));
    strcpy(mr->_modelName, modelName);
    if(modelType)
        {
        strncpy(mr->_modelType, modelType, 4);
        }
    //    else
    //        strcpy(mr->_modelType, "    ");

    mr->setModelFile(_modelFile);
    mr->setModelTable(this);
    mr->assertWriteEnabled();
    mr->close();

    mr->open(ModelObject::Write);
    return eOk;
    }

#ifdef MXARCHIVE
    ErrorStatus ModelTable::Archive(char* modelName, unsigned long& size, void*& data)
    {
    assertReadEnabled();
    ModelTableRecord** mt = &_models[0];
    const int numModels = _models.size();
    unsigned char*& dataPtr = (unsigned char*&)data;
    long*& dataLongPtr = (long*&)data;
    int i;

    for(i = 0; i < numModels; i++)
        {
        if(!mt[i]->isErased() && (!modelName || stricmp(mt[i]->_modelName, modelName) == 0))
            {
            size += 28 + 4 + 4;
            if(data)
                {
                memcpy(dataPtr, mt[i]->_modelName, 28);
                dataPtr += 28;
                *dataLongPtr++ = *((long*)mt[i]->_modelType);
                *dataLongPtr++ = mt[i]->_date;
                }
            StringTable* st;

            if(mt[i]->open(Read) == eOk)
                {
                if(mt[i]->getStringTable(st, Read) == eOk)
                    {
                    st->Archive(NULL, size, data);
                    st->close();
                    }
                mt[i]->close();
                }
            }
        }

    size += 4;
    if(data)
        *dataLongPtr++ = 0;
    return eOk;
    }

ErrorStatus ModelTable::Retrieve(void*& data)
    {
    assertWriteEnabled();
    long*& dataLongPtr = (long*&)data;
    unsigned char*& dataPtr = (unsigned char*&)data;
    ModelTableRecord* mr;
    char modelName[29];
    char modelType[5];

    modelName[28] = 0;
    modelType[4] = 0;

    while(*dataPtr)
        {
        memcpy(modelName, dataPtr, 28);
        dataPtr += 28;

        *((long*)modelType) = *dataLongPtr++;
        long date = *dataLongPtr++;

        ErrorStatus err = getModel(modelName, mr, ModelObject::Write);
        if(err == eOk)
            {
            mr->erase();
            mr->close();
            }

        mr = new ModelTableRecord;
        addModel(modelName, mr, modelType);
        StringTable* st;

        if(mr->getStringTable(st, Read) == eOk)
            {
            st->Retrieve(data);
            st->close();
            }
        mr->close();
        mr->_date = date;
        }
    return eOk;
    }
#endif

// ModelTableIterator
ModelTableIterator::ModelTableIterator(ModelTable* modelTable)
    {
    _modelTable = modelTable;
    _includeDeleted = false;
    start();
    }

void ModelTableIterator::start()
    {
    _index = 0;

    while(_index < (int)_modelTable->_models.size() && (!_includeDeleted && _modelTable->_models[_index]->isErased()))
        _index++;
    }

void ModelTableIterator::next(void)
    {
    if(_index == _modelTable->_models.size())
        return;

    do
        {
        _index++;
        }
        while(_index < (int)_modelTable->_models.size() && (!_includeDeleted && _modelTable->_models[_index]->isErased()));
    }

int ModelTableIterator::done(void) const
    {
    return !(_index < (int)_modelTable->_models.size());
    }

ErrorStatus ModelTableIterator::getRecord(ModelTableRecord*& modelTableRecord, ModelObject::OpenStatus mode)
    {
    modelTableRecord = NULL;
    if(_index >= (int)_modelTable->_models.size())
        return eKeyNotFound;

    ErrorStatus ret = _modelTable->_models[_index]->open(mode);

    if(ret == eOk)
        modelTableRecord = _modelTable->_models[_index];

    return ret;
    }

ErrorStatus ModelTableIterator::getRecord(ModelObjectId& modelTableRecordId)
    {
    if(_index >= (int)_modelTable->_models.size())
        return eKeyNotFound;

    modelTableRecordId = _modelTable->_models[_index]->objectId();

    return eOk;
    }

// ModelTableRecord
MXRTObject_DEFINE_MEMBERS(ModelTableRecord, ModelObject);
ModelTableRecord::ModelTableRecord()
    {
    _stringTable = NULL;
    _modelTable = NULL;
    strncpy(_modelType, "    ", 4);
    _date = getCurrentDate();
    }

ModelTableRecord::~ModelTableRecord()
    {
    if(_stringTable)
        delete _stringTable;
    }

ErrorStatus ModelTableRecord::close()
    {
    bool bModified = false;
    bool bNew = false;
    if(!isInModelFile())
        return eOk;

    if(isModified() || isNewRecord() || isNew())
        {
        bModified = true;
        bNew = isNew();
        }

    ErrorStatus es = ModelObject::close();
#ifdef MXEVENTS
    if(bModified && !isErased())
        {
        if(bNew)
            {
            ModelAdded(this);
            }
        //        else
        //            ModelModified(this);
        }
#endif
    return es;
    }

ErrorStatus ModelTableRecord::erase(int erase)
    {
    ErrorStatus es = ModelObject::erase(erase);
#ifdef MXEVENTS
    if(erase)
        ModelErased(this);
#endif
    return es;
    }

void ModelTableRecord::set(const ModelTableFileRecord& modelRecord)
    {
    assertWriteEnabled();
    strncpy(_modelName, modelRecord.modelName, 28);
    _modelName[28] = 0;
    trim(_modelName);
    strncpy(_modelType, modelRecord.modelType, 4);
    _stringTablePos = ModelFilePos(modelRecord.stringTablePos, 1);
    _date = modelRecord.date;
    clearNew();
    }

ErrorStatus ModelTableRecord::getStringTable(StringTable*& stringTable, ModelObject::OpenStatus mode)
    {
    assertReadEnabled();
    if(!_stringTable)
        {
        _stringTable = new StringTable(_stringTablePos, _modelFile, this);
        _stringTable->close();
        }
    ErrorStatus ret = _stringTable->open(mode);
    if(ret != eOk)
        stringTable = NULL;
    else
        stringTable = _stringTable;
    return ret;
    }

ErrorStatus ModelTableRecord::getStringTable(ModelObjectId& stringTableId)
    {
    assertReadEnabled();
    if(!_stringTable)
        {
        _stringTable = new StringTable(_stringTablePos, _modelFile, this);
        _stringTable->close();
        }
    stringTableId = _stringTable->objectId();
    return eOk;
    }

void ModelTableRecord::UpdateData()
    {
    }

void ModelTableRecord::UpdateFile()
    {
    // Need to write out stringTable to ModelFile ???
    // Need to write out stringTable to ContourString ???
    ModelFileRecord mt;
    int recordNumber = _pos.getRecordNumber();
    int recordEntry = _pos.getRecordPos() - 1;

    // Find Next Free String Record.
    _modelFile->readRecord(&mt, recordNumber);

    memset(mt.models[recordEntry].modelName, ' ', 28);
    strncpy(mt.models[recordEntry].modelName, _modelName, strlen(_modelName));
    strncpy(mt.models[recordEntry].modelType, _modelType, 4);
    if(_date < 0)
        _date = mt.models[recordEntry].date = -ModelTableRecord::getCurrentDate();
    else
        _date = mt.models[recordEntry].date = ModelTableRecord::getCurrentDate();

    if(_stringTablePos.getRecordNumber() == 0)
        {
        mt.models[recordEntry].stringTablePos = _modelFile->addRecord();
        _stringTablePos.setRecordNumber(mt.models[recordEntry].stringTablePos);

        ModelFileRecord str;

        memset(&str, 0, sizeof(str));
        for(int j = 0; j < 50; j++)
            memset(str.strings[j].stringName, ' ', 8);

        str.nextRecord = mt.models[recordEntry].stringTablePos;
        str.lastRecord = stringTableType;
        str.pad[0] = 0xffdbed2d;

        _modelFile->writeRecord(&str, mt.models[recordEntry].stringTablePos);
        StringTable* st;
        if(getStringTable(st, ModelObject::Write) == eOk)
            st->close();
        }

    if(isErased())
        {
        mt.models[recordEntry].stringTablePos = 0;
        }
    else
        {
        mt.models[recordEntry].stringTablePos = _stringTablePos.getRecordNumber();
        }
    _modelFile->writeRecord(&mt, recordNumber);
    }

ErrorStatus ModelTableRecord::setModelName(const char* const modelName)
    {
    if(!isValidModelName(modelName))
        return eInvalidData;

    char modelNameUpr[29];
    fixModelName(modelNameUpr, modelName);

    if(strcmp(_modelName, modelNameUpr) == 0)
        return eOk;

    if(!_modelTable)
        {
        strcpy(_modelName, modelNameUpr);
        return eOk;
        }
    int i = _modelTable->getModelIndex(modelNameUpr, true);

    if(i != -1)
        {
        ModelTableRecord** mt = &_modelTable->_models[0];
        if(mt[i]->isErased())
            {
            if(mt[i]->open(ModelObject::Write) != eOk)
                return eDuplicateName;
            assertWriteEnabled();

            mt[i]->assertWriteEnabled();
            mt[i]->_modelName[0] = 0;
            mt[i]->close();
            }
        else
            return eDuplicateName;
        }
    assertWriteEnabled();
    char oldName[33];
    strcpy(oldName, _modelName);
    strcpy(_modelName, modelNameUpr);
#ifdef MXEVENTS
    if(strcmp(oldName, _modelName) != 0)
        ModelRenaming(this, oldName);
#endif
    return eOk;
    }

ErrorStatus ModelTableRecord::setModelType(const char* const modelType)
    {
    if(_stringTable)
        {
        StringTableIterator* iter = _stringTable->newIterator();

        bool hasStrings = !iter->done();
        delete iter;
        if(strncmp(_modelType, modelType, 4) != 0)
            {
            if(hasStrings)
                return eInvalidData;
            assertWriteEnabled();
            strncpy(_modelType, modelType, 4);
            }
        }
    else
        {
        assertWriteEnabled();
        strncpy(_modelType, modelType, 4);
        }
    return eOk;
    }

int ModelTableRecord::getCurrentDate(void)
    {
    struct tm lt;
    Bentley::BeTimeUtilities::ConvertUnixMillisToTm (lt, Bentley::BeTimeUtilities::GetCurrentTimeAsUnixMillis ());

    if(lt.tm_mon == 12)
        {
        lt.tm_mon = 0;
        lt.tm_year++;
        }

    int time = (((lt.tm_hour * 60) + lt.tm_min) * 6) + (lt.tm_sec / 10);
    int date = ((((lt.tm_year - 89) * 12) + lt.tm_mon + 1) * 100) + lt.tm_mday;
    return time + (date * 10000);
    }

char* ModelTableRecord::getDate(char* retString)
    {
    long date = abs(_date);
    int time = date % 10000;
    int second = (time % 6) * 10;
    time /= 6;
    int minute = time % 60;
    int hour = time / 60;

    date /= 10000;

    int day = date % 100;
    date /= 100;
    int month = date % 12;
    int year = 1989 + ((date - 1) / 12);
    if(month == 0)
        month = 12;

    static char string[100];
    if(!retString)
        retString = string;

    sprintf(retString, "%2d/%2d/%2d %2d:%02d:%02d", day, month, year % 100, hour, minute, second);
    return retString;
    }

//StringTable
MXRTObject_DEFINE_MEMBERS(StringTable, ModelObject);
StringTable::StringTable(ModelFilePos& stringRecordPos, MXModelFile* modelFile, ModelTableRecord* modelTableRecord)
    {
    setDelayWrite(true);    // Make this always delay write.

//    int firstRecord = stringRecordPos.getRecordNumber();
//    int recordNumber = firstRecord;

    setModelFilePos(stringRecordPos);
    setModelFile(modelFile);
    _modelTableRecord = modelTableRecord;
    DoLoad(false);
    }
void StringTable::DoLoad(bool reallyLoad)
    {
    int i;
    _isReallyLoaded = reallyLoad;

    ModelFileRecord mt;
    int firstRecord = getModelFilePos().getRecordNumber();
    int recordNumber = firstRecord;

    if(recordNumber != 0)
        {
        _modelFile->readRecord(&mt, recordNumber);
        if(!reallyLoad)
            {
#ifdef FREESPACEMAPPER
            _modelFile->AddUsedRecord(recordNumber);
#endif
            _recordNumbers.empty();
            }
        do
            {
            if(!reallyLoad)
                _recordNumbers.push_back(recordNumber);
            for(i = 0; i < 50; i++)
                {
                if(mt.strings[i].recordNumber)
                    {
                    StringTableRecord* mr = NULL;

                    if(mt.strings[i].recordNumber)
                        {
                        MXRunTimeClass* rtClass = ClassList::Instance().getType(&mt.strings[i], asLong(_modelTableRecord->_modelType)); //.type % 100, asLong(mt.strings[i].stringName));

                        if(rtClass)
                            mr = (StringTableRecord*)rtClass->create();
                        }
                    if(!mr)
                        mr = new MXProxyString;

                    mr->setModelFile(_modelFile);
                    mr->setModelTableRecord(_modelTableRecord);
                    mr->setStringTable(this);
                    mr->set(mt.strings[i], reallyLoad);
                    mr->setModelFilePos(ModelFilePos(recordNumber, i));
                    if(reallyLoad)
                        {
                        StringTableRecord** mrP;
                        mrP = &_strings[ConvertStringName(mt.strings[i].stringName)];
                        *mrP = mr;
                        mr->close();
                        }
                    else
                        delete mr;
                    }
                }

            if(mt.nextRecord == firstRecord)
                break;
            recordNumber = mt.nextRecord;
            _modelFile->readRecord(&mt, mt.nextRecord);
#ifdef FREESPACEMAPPER
            if(!reallyLoad)
                _modelFile->AddUsedRecord(recordNumber);
#endif
            } while(1);
        }
    }

StringMap::const_iterator StringTable::getStringIndex(const char* stringName) const
    {
    if(!_isReallyLoaded) const_cast<StringTable*>(this)->DoLoad(true);

    return _strings.find(ConvertStringName(stringName));
    }

StringTable::~StringTable()
    {
    Release();
    StringMap::iterator iter = _strings.begin();
    while(iter != _strings.end())
        {
        (*iter).second->Release();
        delete (*iter).second;
        iter++;
        }
    }

StringTableIterator* StringTable::newIterator()
    {
    if(!_isReallyLoaded) const_cast<StringTable*>(this)->DoLoad(true);
    return new StringTableIterator(this);
    }

void StringTable::DisposeU(StringTableIterator *pStringTableIterator)
    {
    delete pStringTableIterator;
    }

ErrorStatus StringTable::getString(const char* const inStringName, StringTableRecord*& stringTableRecord, ModelObject::OpenStatus mode, bool getDeleted)
    {
    assertReadEnabled();
    char stringName[5];

    strncpy(stringName, inStringName, 4);
    stringName[4] = 0;
    if(strlen(stringName) != 4)
        return eKeyNotFound;

    strupr(stringName);
    if(!_isReallyLoaded) const_cast<StringTable*>(this)->DoLoad(true);
    StringMap::iterator iter = _strings.find(ConvertStringName(stringName));
    if(iter == _strings.end())
        return eKeyNotFound;

    StringTableRecord* mr = (*iter).second;
    if(!getDeleted && mr->isErased())
        return eKeyNotFound;

    ErrorStatus ret = mr->open(mode);

    if(ret != eOk)
        stringTableRecord = NULL;
    else
        stringTableRecord = mr;
    return ret;
    }

ErrorStatus StringTable::getString(const char* const inStringName, ModelObjectId& stringTableRecordId, bool getDeleted)
    {
    assertReadEnabled();
    char stringName[5];

    strncpy(stringName, inStringName, 4);
    stringName[4] = 0;
    strupr(stringName);
    if(strlen(stringName) != 4)
        return eKeyNotFound;

    if(!_isReallyLoaded) const_cast<StringTable*>(this)->DoLoad(true);
    StringMap::iterator iter = _strings.find(ConvertStringName(stringName));
    //    int j = getStringIndex(stringName);
    if(iter == _strings.end())
        return eKeyNotFound;

    StringTableRecord* mr = (*iter).second;
    if(!getDeleted && mr->isErased())
        return eKeyNotFound;

    stringTableRecordId = mr->objectId();
    return eOk;
    }

inline bool isValidStringName(const char* name, bool isTextString = false)
    {
    int i = 0;
    if(!name)
        return false;
    if(isTextString)
        {
        if(*name != '*')
            return false;
        i++;
        name++;
        }
    while(*name)
        {
        i++;
        if((*name >= '0' && *name <= '9') || (*name >= 'A' && *name <= 'Z'))
            name++;
        else
            return false;
        }
    return i == 4;
    }
ErrorStatus StringTable::addString(const char* const inStringName, StringTableRecord* str)
    {
    assertWriteEnabled();
    char stringName[6];
    strncpy(stringName, inStringName, 5);
    stringName[5] = 0;
    strupr(stringName);
    if(!isValidStringName(stringName, (bool)(MXTextString::cast(str) != 0)))
        return eInvalidData;

    if(!_isReallyLoaded) const_cast<StringTable*>(this)->DoLoad(true);
    if(str->isInModelFile())
        {
        ErrorStatus es = str->open(ModelObject::Write);
        if(es != eOk) throw eReadOnly;

        if(!str->isErased())
            {
            if(str->_stringTable != this)
                {
                // Error
                str->close();
                throw eInvalidData;
                }
            return str->setStringName(inStringName);
            }
        else
            {
            if(str->_stringTable != this)
                {
                // Error
                str->close();
                throw eInvalidData;
                }
            es = str->setStringName(inStringName);
            if(es == eOk)
                str->erase(0);
            return es;
            }
        }
    assertWriteEnabled();

    StringMap::iterator iter = _strings.find(ConvertStringName(stringName));
    StringTableRecord** mrP = NULL;

    if(iter != _strings.end())
        {
        if(!(*iter).second->isErased())
            return eDuplicateName;
        mrP = &(*iter).second;
        (*mrP)->erase(0);
        }
    else
        {
        mrP = &_strings[ConvertStringName(stringName)];
        }

    if(*mrP)
        delete *mrP;
    *mrP = str;
    str->setModelFilePos(ModelFilePos(0, 1));
    str->setModelFile(_modelFile);
    str->setModelTableRecord(_modelTableRecord);
    str->setStringTable(this);
    strcpy(str->_stringName, stringName);

    this->close();
    this->open(ModelObject::Write);
    str->assertWriteEnabled();
#ifdef MXEVENTS
    StringAdded(str->_modelTableRecord, str);
#endif
    return eOk;
    }

void StringTable::UpdateData()
    {
    return; // This is no needed here changed to now be in the UpdateFile part.
    /*
    if(isModified() || isNewRecord())
    {
    int recNumIndex = 0;
    int recNum = -1;
    int recordEntry = 0;
    int i;

    StringTableRecord** stringsPtr = &_strings[0];
    int _strings_size = _strings.size();

    if(_recordNumbers.size() == 0)
    {
    recNum = _modelFile->addRecord();
#ifdef FREESPACEMAPPER
    _modelFile->AddUsedRecord(recNum);
#endif
    _recordNumbers.append_entry(recNum);
    }
    else
    recNum = _recordNumbers[recNumIndex];

    for(i = 0; i < _strings_size; i++)
    {
    if(!stringsPtr[i]->isErased() || stringsPtr[i]->_numPoints == 0)
    {
    stringsPtr[i]->_pos.setRecordNumber(recNum);
    stringsPtr[i]->_pos.setRecordPos(recordEntry);
    }
    else
    {
    stringsPtr[i]->_pos.setRecordNumber(0);
    stringsPtr[i]->_pos.setRecordPos(0);
    }

    recordEntry++;

    if(recordEntry == 50)
    {
    int nextRecNum;
    recNumIndex++;

    if(_recordNumbers.size() == recNumIndex)
    {
    nextRecNum = _modelFile->addRecord();
#ifdef FREESPACEMAPPER
    _modelFile->AddUsedRecord(nextRecNum);
#endif
    _recordNumbers.append_entry(nextRecNum);
    }
    else
    nextRecNum = _recordNumbers[recNumIndex];

    recNum = nextRecNum;
    recordEntry = 0;
    }
    }
    }
    */
    }

void StringTable::UpdateFile()
    {
    ModelFileRecord mt;
    int recNumIndex = 0;
    int recNum = -1;
    int recordEntry = 0;
    int i;

    if(!_isReallyLoaded) const_cast<StringTable*>(this)->DoLoad(true);
    StringMap::iterator stringsIter = _strings.begin();
    int _strings_size = (int)_strings.size();

    if(_recordNumbers.size() == 0)
        {
        recNum = _modelFile->addRecord();
#ifdef FREESPACEMAPPER
        _modelFile->AddUsedRecord(recNum);
#endif
        _recordNumbers.push_back(recNum);
        }
    else
        recNum = _recordNumbers[recNumIndex];

    recNumIndex = 0;
    recNum = _recordNumbers[recNumIndex];
    recordEntry = 0;
    mt.lastRecord = stringTableType;
    mt.pad[0] = 0xffdbed2d;
    for(i = 0; i < _strings_size; i++)
        {
        StringTableRecord* stringsPtr = (*stringsIter).second;
        stringsIter++;

        if(recordEntry == 50)
            {
            int nextRecNum;
            recNumIndex++;

            if(_recordNumbers.size() == recNumIndex)
                {
                nextRecNum = _modelFile->addRecord();
#ifdef FREESPACEMAPPER
                _modelFile->AddUsedRecord(nextRecNum);
#endif
                _recordNumbers.push_back(nextRecNum);
                }
            else
                nextRecNum = _recordNumbers[recNumIndex];

            mt.nextRecord = nextRecNum;

            _modelFile->writeRecord(&mt, recNum);

            recNum = nextRecNum;
            recordEntry = 0;
            }

        if(!stringsPtr->isErased() || stringsPtr->_numPoints == 0)
            {
            stringsPtr->_pos.setRecordNumber(recNum);
            stringsPtr->_pos.setRecordPos(recordEntry);
            }
        else
            {
            stringsPtr->_pos.setRecordNumber(0);
            stringsPtr->_pos.setRecordPos(0);
            }

        strncpy(mt.strings[recordEntry].stringName, stringsPtr->_stringName, 4);
        memcpy(mt.strings[recordEntry].subReference, stringsPtr->_subReference, 4);
        mt.strings[recordEntry].type = stringsPtr->_type;
        mt.strings[recordEntry].numPoints = stringsPtr->_numPoints;
        mt.strings[recordEntry].minx = stringsPtr->_minx;
        mt.strings[recordEntry].maxx = stringsPtr->_maxx;
        mt.strings[recordEntry].miny = stringsPtr->_miny;
        mt.strings[recordEntry].maxy = stringsPtr->_maxy;

        if(stringsPtr->isErased())
            {
            mt.strings[recordEntry].recordNumber = 0;
            mt.strings[recordEntry].offset = 0;
            }
        else
            {
            mt.strings[recordEntry].recordNumber = stringsPtr->_stringTableRecordDataPos.getRecordNumber();
            mt.strings[recordEntry].offset = stringsPtr->_stringTableRecordDataPos.getRecordPos();
            }
        recordEntry++;
        }

    if(recNumIndex == 0 || recordEntry != 0)
        {
        for(;recordEntry < 50; recordEntry++)
            {
            mt.strings[recordEntry].recordNumber = 0;
            mt.strings[recordEntry].offset = 0;
            }
        mt.nextRecord = _recordNumbers[0];
        _modelFile->writeRecord(&mt, recNum);
        }
    }
#ifdef MXACHIVE
ErrorStatus StringTable::Archive(char* stringMask, unsigned long& size, void*& data)
    {
    assertReadEnabled();
    if(!_isReallyLoaded) const_cast<StringTable*>(this)->DoLoad(true);
    StringMap::iterator stringsIter = _strings.begin();
    int _strings_size = (int)_strings.size();
    long*& dataPtr = (long*&)data;
    int i;

    for(i = 0; i < _strings_size; i++)
        {
        StringTableRecord* stringsPtr = (*stringsIter).second;
        stringsIter++;
        if(!stringsPtr->isErased() && (!stringMask || strnicmp(stringsPtr->_stringName, stringMask, strlen(stringMask)) == 0))
            {
            if(stringsPtr->open(Read) == eOk)
                {
                //                if(!MXTextString::cast(stringsPtr))
                    {
                    size += (9 * sizeof(long));
                    if(data)
                        {
                        *dataPtr++ = *((long*)stringsPtr->_stringName);
                        *dataPtr++ = *((long*)stringsPtr->_subReference);
                        *dataPtr++ = stringsPtr->_type;
                        *dataPtr++ = stringsPtr->_numPoints;
                        *dataPtr++ = stringsPtr->_minx;
                        *dataPtr++ = stringsPtr->_miny;
                        *dataPtr++ = stringsPtr->_maxx;
                        *dataPtr++ = stringsPtr->_maxy;
                        unsigned long* DataSize = (unsigned long*)dataPtr;
                        dataPtr++;
                        *DataSize = 0;
                        stringsPtr->Archive(*DataSize, data);
                        size += *DataSize;
                        }
                    else
                        stringsPtr->Archive(size, data);
                    stringsPtr->close();
                    }
                }
            }
        }
    if(data)
        {
        *dataPtr++ = 0;
        }
    size += 4;
    return eOk;
    }

ErrorStatus StringTable::Retrieve(void*& data)
    {
    assertWriteEnabled();
    long*& dataPtr = (long*&)data;
    char stringName[5];
    stringName[4] = 0;

    while(*dataPtr)
        {
        StringTableFileRecord stringInfo;
        StringTableRecord* mr = NULL;

        *((long*)stringName) = *dataPtr;
        memcpy(&stringInfo, dataPtr, sizeof(StringTableFileRecord) - 8);
        dataPtr += 9;
        stringInfo.recordNumber = 0;
        stringInfo.offset = 0;

        ErrorStatus err = getString(stringName, mr, ModelObject::Write);
        if(err == eOk)
            {
            mr->erase();
            mr->close();
            }
        mr = NULL;
        if(stringInfo.numPoints)
            {
            MXRunTimeClass* rtClass = ClassList::Instance().getType(&stringInfo, asLong(_modelTableRecord->_modelType));
            if(rtClass)
                mr = (StringTableRecord*)rtClass->create();

            if(!mr)
                mr = new MXProxyString;

            //        mr->set(stringInfo);
            strncpy(mr->_stringName, stringInfo.stringName, 4);
            mr->_stringName[4] = 0;
            memcpy(mr->_subReference, stringInfo.subReference, 4);
            mr->_subReference[4] = 0;
            mr->_type = stringInfo.type;
            mr->_numPoints = stringInfo.numPoints;

            addString(stringName, mr);
            mr->Retrieve((void*&)dataPtr);
            mr->close();
            mr->_minx = stringInfo.minx;
            mr->_miny = stringInfo.miny;
            mr->_maxx = stringInfo.maxx;
            mr->_maxy = stringInfo.maxy;
            }
        }
    dataPtr++;
    return eOk;
    }

ErrorStatus StringTable::CreateStringFromArchiveStringHeader(StringTableRecord*& mr, void* data)
    {
    assertWriteEnabled();
    long*& dataPtr = (long*&)data;
    char stringName[5];
    stringName[4] = 0;

    StringTableFileRecord stringInfo;

    *((long*)stringName) = *dataPtr;
    memcpy(&stringInfo, dataPtr, sizeof(StringTableFileRecord) - 8);
    dataPtr += 9;
    stringInfo.recordNumber = 0;
    stringInfo.offset = 0;

    ErrorStatus err = getString(stringName, mr, ModelObject::Write);
    if(err == eOk)
        {
        mr->erase();
        mr->close();
        }
    mr = NULL;
    if(stringInfo.numPoints)
        {
        MXRunTimeClass* rtClass = ClassList::Instance().getType(&stringInfo, asLong(_modelTableRecord->_modelType));
        if(rtClass)
            mr = (StringTableRecord*)rtClass->create();

        if(!mr)
            mr = new MXProxyString;

        strncpy(mr->_stringName, stringInfo.stringName, 4);
        mr->_stringName[4] = 0;
        memcpy(mr->_subReference, stringInfo.subReference, 4);
        mr->_subReference[4] = 0;
        mr->_type = stringInfo.type;
        mr->SetNumPoints(stringInfo.numPoints);
        mr->_numPoints = stringInfo.numPoints;

        addString(stringName, mr);
        mr->_minx = stringInfo.minx;
        mr->_miny = stringInfo.miny;
        mr->_maxx = stringInfo.maxx;
        mr->_maxy = stringInfo.maxy;
        }
    return eOk;
    }
#endif


// StringTableIterator
StringTableIterator::StringTableIterator(StringTable* stringTable)
    {
    _stringTable = stringTable;
    _includeDeleted = false;
    start();
    }

void StringTableIterator::start(void)
    {
    _index = _stringTable->_strings.begin();

    while(_index != _stringTable->_strings.end() && (!_includeDeleted && (*_index).second->isErased()))
        _index++;
    }

void StringTableIterator::next(void)
    {
    if(_index == _stringTable->_strings.end())
        return;

    do
        {
        _index++;
        }
        while(_index != _stringTable->_strings.end() && (!_includeDeleted && (*_index).second->isErased()));
    }

int StringTableIterator::done(void) const
    {
    return _index == _stringTable->_strings.end();
    }

ErrorStatus StringTableIterator::getRecord(StringTableRecord*& stringTableRecord, ModelObject::OpenStatus mode)
    {
    stringTableRecord = NULL;
    if(_index == _stringTable->_strings.end())
        return eKeyNotFound;
    ErrorStatus ret = (*_index).second->open(mode);

    if(ret == eOk)
        stringTableRecord = (*_index).second;
    return ret;
    }

ErrorStatus StringTableIterator::getRecord(ModelObjectId& stringTableRecordId)
    {
    if(_index == _stringTable->_strings.end())
        return eKeyNotFound;
    stringTableRecordId = (*_index).second->objectId();
    return eOk;
    }

//StringTableRecord
MXRTObject_DEFINE_MEMBERS(StringTableRecord, ModelObject);
StringTableRecord::StringTableRecord()
    {
    _stringTable = 0;
    strcpy(_subReference, "    ");
    _stringName[0] = 0;
    _originalDataSize = 0;
    _numPoints = 0;
    _stringData = NULL;
    _loadedData = true;
    _type = 0;
    _modelTableRecord = NULL;
    _modelTable = NULL;
    _modelFile = NULL;
    }

StringTableRecord::~StringTableRecord()
    {
    }

void StringTableRecord::detach()
    {
    if(!_loadedData)
        {
        loadData();
        _loadedData = true;
        }

    _stringTable = 0;
    _originalDataSize = 0;
    _modelFile = 0;
    ModelObject::detach();
    }

ErrorStatus StringTableRecord::erase(int erase)
    {
    ErrorStatus es = ModelObject::erase(erase);
#ifdef MXEVENTS
    if(erase)
        StringErased(_modelTableRecord, this);
#endif
    return es;
    }

void StringTableRecord::set(
    const StringTableFileRecord& data, bool reallyLoad)
    {
    assertWriteEnabled();
    strncpy(_stringName, data.stringName, 4);
    _stringName[4] = 0;
    memcpy(_subReference, data.subReference, 4);
    _subReference[4] = 0;
    _type = data.type;
    _numPoints = data.numPoints;
    _minx = data.minx;
    _miny = data.miny;
    _maxx = data.maxx;
    _maxy = data.maxy;

    _stringTableRecordDataPos = ModelFilePos(data.recordNumber, (short)data.offset);

    _originalDataSize = dataSize();
#ifdef FREESPACEMAPPER
    if(!reallyLoad)
        _modelFile->AddUsedData(_stringTableRecordDataPos, _originalDataSize / 4);
#endif
    _loadedData = false;
    clearNew();
    }

void StringTableRecord::UpdateFile()
    {
    if(_stringData)
        _modelFile->writeData(_stringData, _stringTableRecordDataPos, dataSize());
    }

void StringTableRecord::UpdateData()
    {
    if(_stringTable && (isModified() || isNewRecord()))
        {
        unsigned long _newDataSize = recordSize() * _numPoints;

        resetFilePos(_newDataSize);
        _stringTable->setNeedsUpdating();
        }
    }

void StringTableRecord::emptyData()
    {
    if(NeedsUpdating())
        {
        UpdateFile();
        resetNeedsUpdating();
        }
    _stringData = NULL;
    _loadedData = false;
    }

ErrorStatus StringTableRecord::close()
    {
    bool bModified = false;
    bool bNew = false;
    if(!isInModelFile())
        return eOk;

    if(isModified() || isNewRecord() || isNew())
        {
        bModified = true;
        bNew = isNew();
        }

    ErrorStatus es = ModelObject::close();

#ifdef MXEVENTS
    if(bModified && !isErased())
        {
        if(bNew)
            {
            StringAdded(this->_modelTableRecord, this);
            }
        else
            StringModified(this);
        }
#endif
    if(es == eOk)
        {
        if(_loadedData && !isDelayWrite())
            emptyData();
        }
    return es;
    }

#ifdef MXACHIVE
ErrorStatus StringTableRecord::Archive(unsigned long& size, void*& data)
    {
    assertReadEnabled();
    unsigned char*& dataPtr = (unsigned char*&)data;
    size += dataSize();
    if(data)
        {
        if(!_loadedData)
            loadData();
        memcpy(dataPtr, _stringData, dataSize());
        dataPtr += dataSize();
        }
    return eOk;
    }

ErrorStatus StringTableRecord::Retrieve(void*& data)
    {
    assertWriteEnabled();
    unsigned char*& dataPtr = (unsigned char*&)data;

    emptyData();
    int numPoints = _numPoints;
    _numPoints = 0;
    _loadedData = true;
    setNumPoints(numPoints);
    _numPoints = numPoints;

    memcpy(_stringData, dataPtr, dataSize());
    dataPtr += dataSize();
    return eOk;
    }
#endif

ErrorStatus StringTableRecord::put(__int64 offset, unsigned long size, void* data)
    {
    assertWriteEnabled();
    if(!_loadedData)
        loadData();

    memcpy(((char*)_stringData) + offset, data, size);
    return eOk;
    }

ErrorStatus StringTableRecord::get(__int64 offset, unsigned long size, void* data)
    {
    assertReadEnabled();

    if(!_loadedData)
        loadData();
    memcpy(data, ((char*)_stringData) + offset, size);
    return eOk;
    }

#ifdef MXACHIVE
ErrorStatus StringTableRecord::getArchiveStringHeader(void* data)
    {
    long* dataPtr = (long*)data;
    *dataPtr++ = *((long*)_stringName);
    *dataPtr++ = *((long*)_subReference);
    *dataPtr++ = _type;
    *dataPtr++ = _numPoints;
    *dataPtr++ = _minx;
    *dataPtr++ = _miny;
    *dataPtr++ = _maxx;
    *dataPtr++ = _maxy;
    *dataPtr++ = dataSize();
    return eOk;
    }
#endif

int StringTableRecord::recordSize(void) const
    {
    int numDoubles = 0;
    int type = _type / 100;
    int temp;
    int i;

    for(i = 0; i < _type % 100; i++)
        {
        if((i % 3) == 0)
            {
            temp = type % 10;
            type /= 10;
            }
        if(temp & 1)
            numDoubles++;
        temp = temp >> 1;
        }
    return (_type % 100) * 4 + (numDoubles * 4);
    }


ErrorStatus StringTableRecord::setRecord(const int i, void* data)
    {
    assertWriteEnabled();
    if(i >= _numPoints || i < 0)
        return eInvalidData;

    if(!_loadedData)
        loadData();

    if(!_stringData)
        return eInvalidData;

    long recSize = recordSize();
    memcpy(((char*)_stringData) + (i * recSize), data, recSize);
    return eOk;
    }

ErrorStatus StringTableRecord::getRecord(const int i, const void*& data)
    {
    assertReadEnabled();
    if(i >= _numPoints || i < 0)
        return eInvalidData;

    if(!_loadedData)
        loadData();

    if(!_stringData)
        return eInvalidData;

    data = ((char*)_stringData) + (i * recordSize());
    return eOk;
    }

int StringTableRecord::addRecord(void)
    {
    assertWriteEnabled();

    if(_numPoints)
        {
        if(!_loadedData)
            loadData();

        if(!_stringData)
            return -1;
        }
    else
        _loadedData = true;

    _numPoints++;
    SetNumPoints(_numPoints);
    return _numPoints - 1;
    }

ErrorStatus StringTableRecord::removeRecord(const int i)
    {
    assertWriteEnabled();
    if(i >= _numPoints || i < 0)
        return eInvalidData;

    if(!_loadedData)
        loadData();

    if(!_stringData)
        return eInvalidData;

    _numPoints--;
    if(i != _numPoints)
        {
        long recSize = recordSize();
        memcpy(((char*)_stringData) + ((i + 1) * recSize), ((char*)_stringData) + (i * recSize), recSize * (_numPoints - i + 1));
        }
    return eOk;
    }

ErrorStatus StringTableRecord::insertRecord(const int i)
    {
    assertWriteEnabled();
    if(i > _numPoints || i < 0)
        return eInvalidData;

    if(!_loadedData)
        loadData();

    if(!_stringData)
        return eInvalidData;

    SetNumPoints(_numPoints + 1);
    if(i != _numPoints - 1)
        {
        long recSize = recordSize();
        for(int j = _numPoints - 1; j > i; j--)
            memcpy(((char*)_stringData) + (j - 1 * recSize), ((char*)_stringData) + (j * recSize), recSize);
        }
    return eOk;
    }

void StringTableRecord::SetNumPoints(const int i)
    {
    }


MXStringDimData* StringTableRecord::getDimData(void)
    {
    assertReadEnabled();
    if(!_loadedData)
        loadData();

    if(!_stringData)
        {
        return new MXStringDimData(_type, 0);
        }

    unsigned char* data = new unsigned char[dataSize()];
    memcpy(data, _stringData, dataSize());
    MXStringDimData* dd = new MXStringDimData(data, _type, _numPoints);
    return dd;
    }

void StringTableRecord::DisposeU(MXStringDimData *pMXStringDimData)
    {
    delete pMXStringDimData;
    }

ErrorStatus StringTableRecord::setDimData(const MXStringDimData* dd)
    {
    const unsigned char* const data = dd->data();
    _type = dd->type();
    _numPoints = dd->numPoints();

    assertWriteEnabled();

    SetNumPoints(_numPoints);

    if(!_stringData)
        return eInvalidData;

    int _newDataSize = dataSize();

    memcpy(_stringData, data, _newDataSize);
    _loadedData = true;
    return eOk;
    }

ErrorStatus StringTableRecord::setStringName(const char* const stringName)
    {
    assertWriteEnabled();
    char stringNameUpr[6];
    strncpy(stringNameUpr, stringName, 5);
    stringNameUpr[5] = 0;
    strupr(stringNameUpr);
    if(!isValidStringName(stringName, MXTextString::cast(this) != 0))
        return eInvalidData;
    if(_stringTable)
        {
        StringMap::const_iterator i = _stringTable->getStringIndex(stringNameUpr);
        StringMap::const_iterator oldJ = _stringTable->getStringIndex(_stringName);

        if(asLong(stringNameUpr) == asLong(_stringName))
            return eOk;

        if(i != _stringTable->_strings.end())
            {
            if((*i).second->isErased())
                {
                if((*i).second->open(ModelObject::Write) != eOk)
                    {
                    return eDuplicateName;
                    }
                assertWriteEnabled();
                delete (*i).second;
                _stringTable->_strings.erase((*i).first);
                }
            else
                return eDuplicateName;
            }

        if(oldJ != _stringTable->_strings.end())
            {
            _stringTable->_strings.erase((*oldJ).first);
            }
        _stringTable->_strings[ConvertStringName(stringNameUpr)] = this;

#ifdef MXEVENTS
        StringRenaming(this, stringNameUpr);
#endif
        }
    strcpy(_stringName, stringNameUpr);
    return eOk;
    }

ErrorStatus StringTableRecord::copy(StringTableRecord*& clonedString) const
    {
    assertReadEnabled();
    StringTableRecord* newString = StringTableRecord::cast(this->isA()->create());
    if(!newString)
        newString = new StringTableRecord;
    strcpy(newString->_stringName, "    ");
    memcpy(newString->_subReference, _subReference, sizeof(_subReference));
    newString->_type = _type;
    newString->setNumPoints(_numPoints);
    newString->_minx = _minx;
    newString->_miny = _miny;
    newString->_maxx = _maxx;
    newString->_maxy = _maxy;

    if(_loadedData)
        {
        if(MXTriangleString::cast(newString))
            memcpy(newString->_stringData, _stringData, (sizeof(ElementTriangleString) * _numPoints));
        else
            memcpy(newString->_stringData, _stringData, dataSize());
        }
    else
        {
        newString->_stringTableRecordDataPos = _stringTableRecordDataPos;
        newString->setModelFile(_modelFile);
        newString->loadData();

        newString->_stringTableRecordDataPos.setRecordNumber(0);
        newString->_stringTableRecordDataPos.setRecordPos(0);
        newString->_modelTableRecord = 0;
        newString->_modelTable = 0;
        newString->_stringTable = 0;
        newString->_originalDataSize = 0;
        newString->_modelFile = NULL;
        }
    clonedString = newString;
    return eOk;
    }

//MXTriangleString
MXRT_STRING_DEFINE_MEMBERS(MXTriangleString, StringTableRecord); // 9

bool MXTriangleString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return (str->type % 100) == 9; // && modelType == asLong("TRIA");
    }

ErrorStatus MXTriangleString::loadData(void)
    {
    assertReadEnabled();
    if(_loadedData)
        return eOk;
    ErrorStatus es = loadData(_data);
    if(es == eOk)
        {
        _loadedData = true;
        _stringData = &_data[0];
        }
    return es;
    };

ErrorStatus MXTriangleString::loadData(ElementTriangleString* data)
    {
    int i;
    int j;
    ModelFileRecord record;
    ElementTriangleStringRecord& rec = (ElementTriangleStringRecord&)record;

    long recordNumber = _stringTableRecordDataPos.getRecordNumber();
    for(i = 0; i < _numPoints; i++)
        {
        if(i % 10 == 0)
            {
            _modelFile->readRecord(&record, recordNumber++);
            j = 0;
            }
        data->doubles = rec.doubles[j];
        data->ints = rec.ints[j];
        data++;
        j++;
        }
    return eOk;
    }

ErrorStatus MXTriangleString::loadData(ArrayClass<ElementTriangleString>& data)
    {
    data.setPhysicalLength(_numPoints);
    data.setLogicalLength(_numPoints);

    int i;
    int j;
    ModelFileRecord record;
    ElementTriangleStringRecord& rec = (ElementTriangleStringRecord&)record;

    long recordNumber = _stringTableRecordDataPos.getRecordNumber();
    ElementTriangleString* dataPtr = &data[0];
    for(i = 0; i < _numPoints; i++, dataPtr++)
        {
        if(i % 10 == 0)
            {
            _modelFile->readRecord(&record, recordNumber++);
            j = 0;
            }
        dataPtr->doubles = rec.doubles[j];
        dataPtr->ints = rec.ints[j];
        j++;
        }
    return eOk;
    }


struct MFTrianglePoints
    {
    double pts[3][3];
    double a[8];
    };
inline int FixSide(int value)
    {
    if(value >= 3)
        return value - 3;
    return value;
    }

static int fixTriangle(
    MXTriangle::triangle** triangles,
    int ptNum,
    double& curZ,
    int triNum,
    int fixTriNum,
    int origTriNum)
    {
    int i;

    do
        {
        MXTriangle::triangle* triangleP = triangles[fixTriNum];
        for(i = 0; i < 3; i++)
            {
            if(triangleP->adjTri[i] == triNum)
                {
                i = FixSide(i + 1);
                if(triangleP->ptNum[i] != -1)
                    {
                    // Error Probably left handed triangle.
                    throw eLeftHandedTriangle;
                    }

                triangleP->ptNum[i] = ptNum;
                triNum = triangleP->adjTri[i];

                if(triNum == origTriNum)
                    return 0;
                else if(triNum == -1)
                    return 1;

                int j = fixTriNum;
                fixTriNum = triNum;
                triNum = j;
                break;
                }
            }
        } while(i != 3);
        return 0;
    }

static void fixTriangleReverse(
    MXTriangle::triangle** triangles,
    int ptNum,
    double& curZ,
    int triNum,
    int fixTriNum,
    int origTriNum)
    {
    int i;

    do
        {
        MXTriangle::triangle* triangleP = triangles[fixTriNum];
        for(i = 0; i < 3; i++)
            {
            if(triangleP->adjTri[i] == triNum)
                {

                if(triangleP->ptNum[i] != -1)
                    {
                    // Error Probably left handed triangle.
                    throw eLeftHandedTriangle;
                    return;
                    }
                triangleP->ptNum[i] = ptNum;
                i = FixSide(i + 2);
                triNum = triangleP->adjTri[i];
                if(triNum != origTriNum && triNum != -1)
                    {
                    int j = fixTriNum;
                    fixTriNum = triNum;
                    triNum = j;
                    break;
                    //                    fixTriangleReverse(data, triangles, ptNum, curZ, fixTriNum, triNum, origTriNum);
                    }
                }
            }
        }
        while(i != 3);
    }



static int fixTriangle2(
    MXTriangle::triangle** triangles,
    int& ptNum,
    double& curZ,
    int triNum,
    int fixTriNum,
    int origTriNum,
    ArrayBulkPtrClass<long>& reUsePointNum)
    {
    int i;

    do
        {
        if(fixTriNum > triNum)
            return 1;

        MXTriangle::triangle* triangleP = triangles[fixTriNum];
        for(i = 0; i < 3; i++)
            {
            if(triangleP->adjTri[i] == triNum)
                {
                i = FixSide(i + 1);
                ptNum = triangleP->ptNum[i];
                return 1;
                }
            }
        } while(i != 3);
        return 0;
    }

static void fixTriangleReverse2(
    MXTriangle::triangle** triangles,
    int& ptNum,
    double& curZ,
    int triNum,
    int fixTriNum,
    int origTriNum,
    int& reUseNumber)
    {
    int i;
    int lastPtNum = -1;

    reUseNumber = -1;
    do
        {
        if(fixTriNum > origTriNum)
            return;

        MXTriangle::triangle* triangleP = triangles[fixTriNum];
        for(i = 0; i < 3; i++)
            {
            if(triangleP->adjTri[i] == triNum)
                {

                if(ptNum == -1)
                    {
                    ptNum = triangleP->ptNum[i];
                    return;
                    }
                else if(ptNum == triangleP->ptNum[i])
                    return;
                else if(lastPtNum == -1)
                    {
                    lastPtNum = triangleP->ptNum[i];
                    //for(int k = 0; k  <reUsePointNum.size(); k++)
                    //    {
                    //    if(reUsePointNum[k] == lastPtNum)
                    //        lastPtNum = -2;
                    //    }
                    if(lastPtNum != -2)
                        {
                        reUseNumber = lastPtNum;
                        }
                    }

                triangleP->ptNum[i] = ptNum;
                i = FixSide(i + 2);
                triNum = triangleP->adjTri[i];
                if(triNum != origTriNum && triNum != -1)
                    {
                    int j = fixTriNum;
                    fixTriNum = triNum;
                    triNum = j;
                    break;
                    //                    fixTriangleReverse(data, triangles, ptNum, curZ, fixTriNum, triNum, origTriNum);
                    }
                }
            }
        }
        while(i != 3);
    }




inline bool isRight(
    const MXTriangle::point& pt1,
    const MXTriangle::point& pt2,
    const MXTriangle::point& pt3)
    {
    double x;
    double y;
    //    isRight2(pt1, pt2, pt3);
    x = (pt1.x + pt2.x + pt3.x) / 3;
    y = (pt1.y + pt2.y + pt3.y) / 3;
    double a = pt2.y - pt1.y;
    double b = pt1.x - pt2.x;
    double offset = a * (x - pt1.x) + b * (y - pt1.y);
    const double tola = -0.00000001L;
    if(offset < tola)
        {
        //                        MessageBox(NULL, "Created a Wrong Handed Triangle", "MXMF", MB_OK);
        return false;
        }
    return true;
    }

ErrorStatus MXTriangleString::loadData(MXTriangle* tri)
    {
    if(!tri)
        return eInvalidData;

    tri->empty();

    MXTriangle::PointArray* points;
    MXTriangle::TriangleArray* triangles;

    //    tri->getPtrs(triangles2, points2);
    tri->getPtrs(triangles, points);
    //loadData2(tri);

    //MXTriangle::PointArray pa;
    //MXTriangle::TriangleArray ta;
    //MXTriangle::PointArray* points = &pa;
    //MXTriangle::TriangleArray* triangles = &ta;

    MXTriangle::triangle** triangleP;
    //    MFTrianglePoints* dataP;
    ElementTriangleStringInts* dataIP = NULL;
    ElementTriangleStringDoubles* dataDP = NULL;
    MXTriangle::point* pt = nullptr;

    points->setPhysicalLength(_numPoints / 2);
    points->setBlockSize(1000);
    triangles->setPhysicalLength(_numPoints);
    triangles->setLogicalLength(_numPoints);
    ArrayBulkPtrClass<long> reUsePointNum;
    MXTriangle::triangle** trianglesPtr = triangles->getArrayPtr();
    int pNum = 4;  // The first four are special points.
    int numReused = 0;
    int numNew = 0;
    int numExisting = 0;

    try
        {
        int i;
        int j;
        int k = 0;
        //        ModelFileRecord record;
        ModelFileRecord records[10];
        //                        ElementTriangleStringRecord& rec = (ElementTriangleStringRecord&)record;

        triangleP = triangles->getArrayPtr();
        j = 0;
        long recordNumber = _stringTableRecordDataPos.getRecordNumber();

        for(i = 0; i < _numPoints; i++)
            {
            if(!j)
                {
                if(!k)
                    {
                    _modelFile->readRecords(records, recordNumber, 10);
                    recordNumber += 10;
                    k = 10;
                    }
                k--;
                j = 10;
                dataIP = &records[9-k].triangle.ints[0];
                dataDP = &records[9-k].triangle.doubles[0];
                }
            j--;

            int* tp = &(*triangleP)->ptNum[0];

            *tp++ = -1; *tp++ = -1; *tp++ = -1;
            *tp++ = dataIP->adjTri[0] - 1; *tp++ = dataIP->adjTri[1] - 1; *tp++ = dataIP->adjTri[2] - 1;
            *tp++ = dataIP->label[0]; *tp++ = dataIP->label[1]; *tp++ = dataIP->label[2];
            if(dataDP->pts[0][2] <= -998)
                *tp = asLong("NULL");
            else
                *tp = dataIP->groupCode;

            for(int j = 0; j < 3; j++)
                {
                int ptNum = -1;

                int mix = dataDP->pts[j][0] < (_minx - 400) ? 1 : 0;
                int max = !mix && dataDP->pts[j][0] > (_maxx + 400) ? 1 : 0;
                int miy = dataDP->pts[j][1] < (_miny - 400) ? 1 : 0;
                int may = !miy && dataDP->pts[j][1] > (_maxy + 400) ? 1 : 0;

                if((mix + max + miy + may) > 1)
                    {
                    if(mix && miy)
                        ptNum = 0;
                    else if(mix && may)
                        ptNum = 1;
                    else if(max && may)
                        ptNum = 2;
                    else if(max && miy)
                        ptNum = 3;

                    (*triangleP)->ptNum[j] = ptNum;
                    pt = &(*points)[ptNum];
                    *pt = *((MXTriangle::point*)&dataDP->pts[j][0]);
                    }
                else
                    {

                    int triNum = (*triangleP)->adjTri[j];
                    if(triNum != -1)
                        {
                        if(fixTriangle2(trianglesPtr, ptNum, pt->z, i, triNum, i, reUsePointNum))
                            {
                            triNum = (*triangleP)->adjTri[FixSide(j + 2)];
                            if(triNum != -1)
                                {
                                int reUseNumber;
                                fixTriangleReverse2(trianglesPtr, ptNum, pt->z, i, triNum, i, reUseNumber);
                                if(ptNum != -1 && reUseNumber != -1)
                                    {
                                    reUsePointNum.append_entry(reUseNumber);
                                    pt = &(*points)[ptNum];
                                    if(pt->z < -998)
                                        {
                                        MXTriangle::point* pt2 = &(*points)[reUseNumber];
                                        pt->z = pt2->z;
                                        }
                                    }

                                }
                            }
                        }
                    else
                        {
                        triNum = (*triangleP)->adjTri[FixSide(j + 2)];
                        if(triNum != -1)
                            {
                            int reUseNumber;
                            fixTriangleReverse2(trianglesPtr, ptNum, pt->z, i, triNum, i, reUseNumber);
                            if(ptNum != -1 && reUseNumber != -1)
                                {
                                reUsePointNum.append_entry(reUseNumber);
                                pt = &(*points)[ptNum];
                                if(pt->z < -998)
                                    {
                                    MXTriangle::point* pt2 = &(*points)[reUseNumber];
                                    pt->z = pt2->z;
                                    }
                                }
                            }
                        }

                    // Check

                    /*                   for(int t = 0; t < i; t++)
                    {
                    for(int j = 0; j < 3; j++)
                    {
                    for(int k = 0; k < reUsePointNum.size(); k++)
                    {
                    if(triangles->getArrayPtr()[t]->ptNum[j] == reUsePointNum[k])
                    k = k;
                    }
                    }
                    }*/
                    if(ptNum == -1)
                        {
                        if(reUsePointNum.size())
                            {
                            numReused++;
                            ptNum = reUsePointNum[reUsePointNum.size() - 1];
                            reUsePointNum.delete_entry(reUsePointNum.size() - 1);
                            }
                        else
                            {
                            numNew++;
                            ptNum = pNum++;
                            }
                        (*triangleP)->ptNum[j] = ptNum;
                        pt = &(*points)[ptNum];
                        *pt = *((MXTriangle::point*)&dataDP->pts[j][0]);
                        }
                    else
                        {
                        numExisting++;
                        (*triangleP)->ptNum[j] = ptNum;
                        pt = &(*points)[ptNum];
                        if(pt->z < -998)
                            pt->z = dataDP->pts[j][2];
                        }
                    }
                }
            triangleP++;
            dataIP++;
            dataDP++;
            }
        triangleP = triangles->getArrayPtr();
        pNum = points->size();
        long* ptNums = new long[pNum];
        for(i = 0; i < pNum; i++)
            {
            ptNums[i] = 0;
            }

        for(i = 0; i < reUsePointNum.size(); i++)
            {
            ptNums[reUsePointNum[i]] = -1;
            }

        int pNum = 0;
        for(i = 0; i < points->size(); i++)
            {
            if(ptNums[i] != -1)
                {
                ptNums[i] = pNum;
                if(i != pNum)
                    (*points)[pNum] = (*points)[i];
                pNum++;
                }
            }

        for(i = 0; i < _numPoints; i++, triangleP++)
            {
            for(j = 0; j < 3; j++)
                {
                (*triangleP)->ptNum[j] = ptNums[(*triangleP)->ptNum[j]];
                //if((*triangleP)->ptNum[j] == -1)
                //    j = j;
                }
            }
        delete [] ptNums;
        points->setBlockSize(0);
        points->setLogicalLength(pNum);
        points->setPhysicalLength(pNum);

        //for(int i = 0; i < _numPoints; i++)
        //    {
        //    for(int j = 0; j < 3; j++)
        //        {
        //        if(triangles2->getArrayPtr()[i]->adjTri[j] != triangles->getArrayPtr()[i]->adjTri[j])
        //            j = j;

        //        int pt = triangles->getArrayPtr()[i]->ptNum[j];
        //        int pt2 = triangles2->getArrayPtr()[i]->ptNum[j];
        //        if(points->getArrayPtr()[pt]->x != points2->getArrayPtr()[pt2]->x)
        //            j = j;
        //        if(points->getArrayPtr()[pt]->y != points2->getArrayPtr()[pt2]->y)
        //            j = j;
        //        if(points->getArrayPtr()[pt]->z != points2->getArrayPtr()[pt2]->z)
        //            j = j;
        //        }
        //    }

        }
    catch(ErrorStatus es)
        {
        return es;
        }
    // ToDo - Need to find the NULL triangles.
    // It may be better to set the NULLs to be _VOD and set them to NULL if they are on the edge.
    tri->updateLastTri();

    return eOk;
    }

ErrorStatus MXTriangleString::loadData2(MXTriangle* tri)
    {
    if(!tri)
        return eInvalidData;

    tri->empty();

    MXTriangle::PointArray* points;
    MXTriangle::TriangleArray* triangles;

    tri->getPtrs(triangles, points);

    MXTriangle::triangle** triangleP;
    MFTrianglePoints* dataP = nullptr;
    ElementTriangleStringInts* dataIP = nullptr;
    ElementTriangleStringDoubles* dataDP = nullptr;

    points->setPhysicalLength(_numPoints / 2);
    points->setBlockSize(1000);
    triangles->setPhysicalLength(_numPoints);
    triangles->setLogicalLength(_numPoints);

    MXTriangle::triangle** trianglesPtr = triangles->getArrayPtr();

    try
        {
        int i;
        int j;
        int k = 0;
        ModelFileRecord record;
        ModelFileRecord records[10];
        //                        ElementTriangleStringRecord& rec = (ElementTriangleStringRecord&)record;

        triangleP = triangles->getArrayPtr();
        j = 0;
        long recordNumber = _stringTableRecordDataPos.getRecordNumber();

        for(i = 0; i < _numPoints; i++)
            {
            if(!j)
                {


                if(!k)
                    {
                    _modelFile->readRecords(records, recordNumber, 10);
                    recordNumber += 10;
                    k = 10;
                    }
                k--;
                //                                _modelFile->readRecord(&record, recordNumber++);

                j = 10;
                dataIP = &records[9-k].triangle.ints[0];
                dataDP = &records[9-k].triangle.doubles[0];
                }
            j--;

            //        isRight(*(MXTriangle::point*)dataDP->pts[0], *(MXTriangle::point*)dataDP->pts[1], *(MXTriangle::point*)dataDP->pts[2]);
            int* tp = &(*triangleP)->ptNum[0];

            *tp++ = -1; *tp++ = -1; *tp++ = -1;
            *tp++ = dataIP->adjTri[0] - 1; *tp++ = dataIP->adjTri[1] - 1; *tp++ = dataIP->adjTri[2] - 1;
            *tp++ = dataIP->label[0]; *tp++ = dataIP->label[1]; *tp++ = dataIP->label[2];
            if(dataDP->pts[0][2] <= -998)
                *tp = asLong("NULL");
            else
                *tp = dataIP->groupCode;

            triangleP++;
            dataIP++;
            dataDP++;
            }

        recordNumber = _stringTableRecordDataPos.getRecordNumber();
        int triNum;
        int pNum = 3;  // The first four are special points.
        MXTriangle::point* pt;

        triangleP = triangles->getArrayPtr();
        int numPoints = triangles->size();
        k = 10;
        long loadedRecNum = -1;
        dataP = (MFTrianglePoints*)&record.triangle.doubles[0];
        for(i = 0; i < numPoints; i++, triangleP++, dataP++)
            {
            if(!k)
                {
                recordNumber++;
                //                                _modelFile->readRecord(&record, recordNumber++);
                //                                _modelFile->readRecordOffsetAndSize(&record, recordNumber++, 0, sizeof(rec.doubles));
                dataP = (MFTrianglePoints*)&record.triangle.doubles[0];
                k = 10;
                }
            k--;
            for(j = 0; j < 3; j++)
                {
                int ptNum = -1;
                if((*triangleP)->ptNum[j] == -1)
                    {
                    if(loadedRecNum != recordNumber)
                        {
                        _modelFile->readRecord2(&record, recordNumber);
                        loadedRecNum = recordNumber;
                        }

                    int mix = dataP->pts[j][0] < (_minx - 400) ? 1 : 0;
                    int max = !mix && dataP->pts[j][0] > (_maxx + 400) ? 1 : 0;
                    int miy = dataP->pts[j][1] < (_miny - 400) ? 1 : 0;
                    int may = !miy && dataP->pts[j][1] > (_maxy + 400) ? 1 : 0;

                    if((mix + max + miy + may) > 1)
                        {
                        if(mix && miy)
                            ptNum = 0;
                        else if(mix && may)
                            ptNum = 1;
                        else if(max && may)
                            ptNum = 2;
                        else if(max && miy)
                            ptNum = 3;
                        }
                    else
                        {
                        pNum++;
                        ptNum = pNum;
                        }
                    pt = &(*points)[ptNum];
                    *pt = *((MXTriangle::point*)&dataP->pts[j][0]);
                    (*triangleP)->ptNum[j] = ptNum;

                    triNum = (*triangleP)->adjTri[j];
                    if(triNum != -1)
                        {
                        if(fixTriangle(trianglesPtr, ptNum, pt->z, i, triNum, i))
                            {
                            triNum = (*triangleP)->adjTri[FixSide(j + 2)];
                            if(triNum != -1)
                                {
                                fixTriangleReverse(trianglesPtr, ptNum, pt->z, i, triNum, i);
                                }
                            }
                        }
                    else
                        {
                        triNum = (*triangleP)->adjTri[FixSide(j + 2)];
                        if(triNum != -1)
                            {
                            fixTriangleReverse(trianglesPtr, ptNum, pt->z, i, triNum, i);
                            }
                        }
                    }
                else
                    {
                    int ptNum = (*triangleP)->ptNum[j];
                    if(points->getArrayPtr()[ptNum]->z < -998)
                        {
                        if(loadedRecNum != recordNumber)
                            {
                            _modelFile->readRecord2(&record, recordNumber);
                            loadedRecNum = recordNumber;
                            }

                        if(dataP->pts[j][2] > -998)
                            {
                            points->getArrayPtr()[ptNum]->z = dataP->pts[j][2];
                            }
                        }
                    }
                }
            }
        points->setBlockSize(0);
        points->setPhysicalLength(points->size());

        }
    catch(ErrorStatus es)
        {
        return es;
        }
    tri->updateLastTri();

    return eOk;
    }

ErrorStatus MXTriangleString::close()
    {
    if((isModified() || isNewRecord()) && !isErased())
        {
        if(_loadedData)
            {
            ErrorStatus ret = saveData(_data);
            if(!isDelayWrite())
                {
                _loadedData = false;
                _data.empty();
                }
            return ret;
            }
        }
    return StringTableRecord::close();
    }

ErrorStatus MXTriangleString::saveData(ArrayClass<ElementTriangleString>& _data)
    {
    assertWriteEnabled();
    if((isModified() || isNewRecord()) && !isErased())
        {
        _numPoints = (int)_data.size();
        return saveData(&_data[0]);
        }
    return eOk; //StringTableRecord::close();
    //                    return StringTableRecord::close();
    }

ErrorStatus MXTriangleString::saveData(ElementTriangleString* _data)
    {
    assertWriteEnabled();
    if((isModified() || isNewRecord()) && !isErased())
        {

        flushRecords(false);
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;
            minx = miny = 99999999;
            maxx = maxy = -99999999;
            //            _data[0].doubles.pts[0][0];
            //            _miny = _maxy = (long)_data[0].doubles.pts[0][1];

            for(int i = 0; i < _numPoints; i++)
                for(int j = 0; j < 3; j++)
                    {
                    double x = _data[i].doubles.pts[j][0];
                    double y = _data[i].doubles.pts[j][1];

                    if(minx > x)
                        minx = x;
                    else if(maxx < x)
                        maxx = x;

                    if(miny > y)
                        miny = y;
                    else if(maxy < y)
                        maxy = y;
                    }

                double xs = 0;    // Triangulations don't have the negative shift.
                double ys = 0;
                //            _modelFile->getModelShift(xs, ys);
                _minx = (long)floor(minx - xs) + 500;
                _maxx = (long)ceil(maxx - xs) - 500;
                _miny = (long)floor(miny - ys) + 500;
                _maxy = (long)ceil(maxy - ys) - 500;
                if(_minx == _maxx)
                    _minx--;
                if(_miny == _maxy)
                    _miny--;

                unsigned long _newDataSize = sizeof(ElementTriangleString) * 10 * ((_numPoints + 9) / 10);

                resetFilePos(_newDataSize, true);

                int recordNumber = _stringTableRecordDataPos.getRecordNumber();
                int j = 0;
                int i;
                ModelFileRecord rec;

                _modelFile->updateNextRecord(recordNumber + ((_numPoints + 9) / 10));

                for(i = 0; i < _numPoints; i++)
                    {
                    rec.triangle.doubles[j] = _data[i].doubles;
                    rec.triangle.ints[j] = _data[i].ints;
                    j++;

                    if(i % 10 == 9)
                        {
                        _modelFile->writeRecord(&rec, recordNumber);
                        recordNumber++;
                        j = 0;
                        }
                    }
                _modelFile->writeRecord(&rec, recordNumber);
            }
        }
    return eOk; //StringTableRecord::close();
    //                    return StringTableRecord::close();
    }


ErrorStatus MXTriangleString::saveData(MXTriangle* tri)
    {
    assertWriteEnabled();
    if((1/*isModified()*/ || isNewRecord()) && !isErased())
        {
        MXTriangle::triangle** triPtr = tri->getTriangles().getArrayPtr();
        MXTriangle::point** const pointsPtr = tri->getPoints().getArrayPtr();
        const int numTriangles = tri->getTriangles().size();
        const int numPoints = tri->getPoints().size();

        _numPoints = numTriangles;

        flushRecords(false);
        if(numPoints > 0)
            {
            double minx,miny,maxx,maxy;
            minx = miny = 99999999;
            maxx = maxy = -99999999;

            for(int i = 0; i < 4; i++)
                {
                double& x = pointsPtr[i]->x;
                double& y = pointsPtr[i]->y;

                if(x < minx)
                    minx = x;
                else if(x > maxx)
                    maxx = x;

                if(y < miny)
                    miny = y;
                else if(y > maxy)
                    maxy = y;
                }
            double xs = 0;    // MXMF Triangles don't use the negative Shift.
            double ys = 0;
            //            _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs) + 500;
            _maxx = (long)ceil(maxx - xs) - 500;
            _miny = (long)floor(miny - ys) + 500;
            _maxy = (long)ceil(maxy - ys) - 500;
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            unsigned long _newDataSize = sizeof(ElementTriangleString) * 10 * ((_numPoints + 9) / 10);

            resetFilePos(_newDataSize, true);

            int recordNumber = _stringTableRecordDataPos.getRecordNumber();
            int j = 0;
            //        ModelFilePos pos = _stringTableRecordDataPos;
            ModelFileRecord rec[10] = {0};
            int k = 0;

            _modelFile->updateNextRecord(recordNumber + ((_numPoints + 9) / 10));

            //            _modelFile->writeRecord(&rec[0], recordNumber + ((_numPoints + 9) / 10) - 1);
            //            memset(&rec.triangle, 0, sizeof(rec));
            int i;
            for(i = 0; i < _numPoints; i++)
                {
                const MXTriangle::triangle* t = *triPtr++;
                int* ptr = &rec[k].triangle.ints[j].adjTri[0];
                const int* sptr = &t->ptNum[0];
                double* dPtr = &rec[k].triangle.doubles[j].pts[0][0];
                int pn1 = *sptr++;
                int pn2 = *sptr++;
                int pn3 = *sptr++;
                const /*MXTriangle::point*/ double* p1 = (const double*)pointsPtr[pn1]; //t->ptNum[0]];
                const /*MXTriangle::point*/ double* p2 = (const double*)pointsPtr[pn2]; //t->ptNum[1]];
                const /*MXTriangle::point*/ double* p3 = (const double*)pointsPtr[pn3]; //t->ptNum[2]];

                /*rec.triangle.ints[j].adjTri[0]*/ *ptr++ = *sptr++ + 1; /*t->adjTri[0] + 1;*/
                /*rec.triangle.ints[j].adjTri[1]*/ *ptr++ = *sptr++ + 1; /*t->adjTri[1] + 1;*/
                /*rec.triangle.ints[j].adjTri[2]*/ *ptr++ = *sptr++ + 1; /*t->adjTri[2] + 1;*/

                /*rec.triangle.ints[j].pts[0]*/ *ptr++ = *sptr++ ? -(pn1 + 1) : pn1 + 1; /*t->pts[0];*/
                /*rec.triangle.ints[j].pts[1]*/ *ptr++ = *sptr++ ? -(pn2 + 1) : pn2 + 1; /*t->pts[1];*/
                /*rec.triangle.ints[j].pts[2]*/ *ptr++ = *sptr++ ? -(pn3 + 1) : pn3 + 1; /*t->pts[2];*/
                ptr += 3;
                sptr -= 3;
                /*rec.triangle.ints[j].label[0]*/ *ptr++ = *sptr++; /*t->strings[0];*/
                /*rec.triangle.ints[j].label[1]*/ *ptr++ = *sptr++; /*t->strings[1];*/
                /*rec.triangle.ints[j].label[2]*/ *ptr++ = *sptr++; /*t->strings[2];*/
                /*rec.triangle.ints[j].groupCode*/ *ptr++ = *sptr; /*t->groupCode;*/

                /*rec.triangle.doubles[j].pts[0][0]*/ *dPtr++ = *p1++; //p1->x;
                /*rec.triangle.doubles[j].pts[0][1]*/ *dPtr++ = *p1++; //p1->y;
                dPtr++;
                /*rec.triangle.doubles[j].pts[1][0]*/ *dPtr++ = *p2++; //p2->x;
                /*rec.triangle.doubles[j].pts[1][1]*/ *dPtr++ = *p2++; //p2->y;
                dPtr++;
                /*rec.triangle.doubles[j].pts[2][0]*/ *dPtr++ = *p3++; //p3->x;
                /*rec.triangle.doubles[j].pts[2][1]*/ *dPtr++ = *p3++; //p3->y;

                dPtr -= 6;
                if(*p1 < -998 || *p2 < -998 || *p3 < -998 || *sptr == asLong("NULL") || *sptr == asLong("_VOD"))
                    {
                    ptr[-1] = 0; // GroupCode
                    /*rec.triangle.doubles[j].pts[0][2]*/ *dPtr = -999.0;
                    dPtr += 3;
                    /*rec.triangle.doubles[j].pts[1][2]*/ *dPtr = -999.0;
                    dPtr += 3;
                    /*rec.triangle.doubles[j].pts[2][2]*/ *dPtr = -999.0;
                    }
                else
                    {
                    /*rec.triangle.doubles[j].pts[0][2]*/ *dPtr = *p1; //p1->z;
                    dPtr += 3;
                    /*rec.triangle.doubles[j].pts[1][2]*/ *dPtr = *p2; //p2->z;
                    dPtr += 3;
                    /*rec.triangle.doubles[j].pts[2][2]*/ *dPtr = *p3; //p3->z;
                    }

                j++;

                if(j == 10)
                    {
                    k++;
                    if(k == 10)
                        {
                        _modelFile->writeRecords(&rec[0], recordNumber, 10);
                        recordNumber += 10;
                        k = 0;
                        }

                    j = 0;
                    }
                }
            if(j != 0)
                _modelFile->writeRecords(&rec[0], recordNumber, k + 1);
            else if(k != 0)
                _modelFile->writeRecords(&rec[0], recordNumber, k);
            }
        }
    return eOk; //StringTableRecord::close();
    }

void MXTriangleString::UpdateData()
    {
    _stringTable->setNeedsUpdating();
    }

void MXTriangleString::UpdateFile()
    {
    flushRecords(true);
    }

#ifdef MXACHIVE
ErrorStatus MXTriangleString::getArchiveStringHeader(void* data)
    {
    long* dataPtr = (long*)data;
    *dataPtr++ = *((long*)_stringName);
    *dataPtr++ = *((long*)_subReference);
    *dataPtr++ = _type;
    *dataPtr++ = _numPoints;
    *dataPtr++ = _minx;
    *dataPtr++ = _miny;
    *dataPtr++ = _maxx;
    *dataPtr++ = _maxy;
    *dataPtr++ = _numPoints * sizeof(ElementTriangleString);
    return eOk;
    }

ErrorStatus MXTriangleString::Archive(unsigned long& size, void*& data)
    {
    assertReadEnabled();
    unsigned char*& dataPtr = (unsigned char*&)data;
    size += _numPoints * sizeof(ElementTriangleString);
    if(data)
        {
        if(!_loadedData)
            loadData((ElementTriangleString*)dataPtr);
        else
            memcpy(dataPtr, _stringData, _numPoints * sizeof(ElementTriangleString));
        dataPtr += _numPoints * sizeof(ElementTriangleString);
        }
    return eOk;
    }

ErrorStatus MXTriangleString::Retrieve(void*& data)
    {
    assertWriteEnabled();
    unsigned char*& dataPtr = (unsigned char*&)data;

    emptyData();
    int numPoints = _numPoints;
    //    _numPoints = 0;
    //    setNumPoints(numPoints);
    _numPoints = numPoints;

    if(_stringTable)
        {
        saveData((ElementTriangleString*)dataPtr);
        close();
        open(ModelObject::Write);
        }
    dataPtr += _numPoints * sizeof(ElementTriangleString);
    return eOk;
    }
#endif

ErrorStatus MXTriangleString::put(__int64 offset, unsigned long size, void* dataP)
    {
    unsigned char* data = (unsigned char*)dataP;
    long recordNumber = (long)(offset / sizeof(ElementTriangleString));
    ElementTriangleString rec;

    assertWriteEnabled();

    if(offset % sizeof(ElementTriangleString))
        {
        int pOff = (long)(offset % sizeof(ElementTriangleString));
        int pSize= sizeof(ElementTriangleString) - pOff;
        // Partial Record.
        getTriangle(recordNumber, rec);
        memcpy(&((char*)&rec)[pOff], data, size);
        setTriangle(recordNumber, rec);
        data += pSize;
        size -= pSize;
        recordNumber++;
        }
    while(size >= sizeof(ElementTriangleString))
        {
        getTriangle(recordNumber, rec);
        memcpy(&rec, data, sizeof(ElementTriangleString));
        setTriangle(recordNumber, rec);
        data += sizeof(ElementTriangleString);
        size -= sizeof(ElementTriangleString);
        recordNumber++;
        }
    if(size)
        {
        getTriangle(recordNumber, rec);
        memcpy(&rec, data, size);
        setTriangle(recordNumber, rec);
        }
    return eOk;
    }

ErrorStatus MXTriangleString::get(__int64 offset, unsigned long size, void* dataP)
    {
    unsigned char* data = (unsigned char*)dataP;
    long recordNumber = (long)(offset / sizeof(ElementTriangleString));
    ElementTriangleString rec;

    assertReadEnabled();

    if(offset % sizeof(ElementTriangleString))
        {
        int pOff = (long)(offset % sizeof(ElementTriangleString));
        int pSize= sizeof(ElementTriangleString) - pOff;
        // Partial Record.
        getTriangle(recordNumber, rec);
        memcpy(data, &((char*)&rec)[pOff], size);
        data += pSize;
        size -= pSize;
        recordNumber++;
        }
    while(size >= sizeof(ElementTriangleString))
        {
        getTriangle(recordNumber, rec);
        memcpy(data, &rec, sizeof(ElementTriangleString));
        data += sizeof(ElementTriangleString);
        size -= sizeof(ElementTriangleString);
        recordNumber++;
        }
    if(size)
        {
        getTriangle(recordNumber, rec);
        memcpy(data, &rec, size);
        }
    return eOk;
    }
void MXTriangleString::flushRecords(bool updateFile)
    {
    int i;

    if(_recordNumbers)
        {
        if(updateFile)
            {
            for(i = 0; i < _numberRecords; i++)
                {
                if(_recordNumbers[i] < 0)
                    {
                    if(_autoConvert && _modelFile->ConvertToWorld() && _modelFile->swapXY())
                        {
                        for(int j = 0; j < 10; j++)
                            {
                            for(int k = 0; k < 3; k++)
                                {
                                double bak;
                                bak = _records[i].triangle.doubles[j].pts[k][0];
                                _records[i].triangle.doubles[j].pts[k][1] = _records[i].triangle.doubles[j].pts[k][0];
                                _records[i].triangle.doubles[j].pts[k][1] = bak;
                                }
                            }
                        }
                    _modelFile->writeRecord(&_records[i], _stringTableRecordDataPos.getRecordNumber() + abs(_recordNumbers[i]));
                    }
                }
            }
        delete [] _recordMapper;
        delete [] _timeNumbers;
        delete [] _recordNumbers;
        delete [] _records;
        _recordMapper = NULL;
        _timeNumbers = NULL;
        _recordNumbers = NULL;
        _records = NULL;
        }
    }

int MXTriangleString::loadTriangle(int recordNumber)
    {
    int i;
    int oldestRec;
    int oldestTime = CurTime;
    for(i = 0; i < _numberRecords; i++)
        {
        if(oldestTime > _timeNumbers[i])
            {
            oldestTime = _timeNumbers[i];
            oldestRec = i;
            }
        if(_recordNumbers[i] == 0)
            {
            oldestRec = i;
            break;
            }
        }
    i = oldestRec;
    _recordMapper[abs(_recordNumbers[i])] = -1;
    if(_recordNumbers[i] < 0)
        {
        if(_autoConvert && _modelFile->ConvertToWorld() && _modelFile->swapXY())
            {
            for(int j = 0; j < 10; j++)
                {
                for(int k = 0; k < 3; k++)
                    {
                    double bak;
                    bak = _records[i].triangle.doubles[j].pts[k][0];
                    _records[i].triangle.doubles[j].pts[k][1] = _records[i].triangle.doubles[j].pts[k][0];
                    _records[i].triangle.doubles[j].pts[k][1] = bak;
                    }
                }
            }
        _modelFile->writeRecord(&_records[i], _stringTableRecordDataPos.getRecordNumber() + abs(_recordNumbers[i]) - 1);
        }

    _modelFile->readRecord(&_records[i], _stringTableRecordDataPos.getRecordNumber() + recordNumber - 1);
    if(_autoConvert && _modelFile->ConvertToWorld() && _modelFile->swapXY())
        {
        for(int j = 0; j < 10; j++)
            {
            for(int k = 0; k < 3; k++)
                {
                double bak;
                bak = _records[i].triangle.doubles[j].pts[k][0];
                _records[i].triangle.doubles[j].pts[k][1] = _records[i].triangle.doubles[j].pts[k][0];
                _records[i].triangle.doubles[j].pts[k][1] = bak;
                }
            }
        }
    _recordMapper[recordNumber] = i;
    _recordNumbers[i] = recordNumber;

    return i;
    }
ErrorStatus MXTriangleString::getTriangle(const int triangleNumber, ElementTriangleStringDoubles*& doubles, ElementTriangleStringInts*& ints, bool setWrite)

    {
    if(setWrite)
        assertWriteEnabled();
    else
        assertReadEnabled();

    if(triangleNumber >= _numPoints)
        return eInvalidNumber;

    if(_loadedData)
        {
        doubles = &_data[triangleNumber].doubles;
        ints = &_data[triangleNumber].ints;
        }
    else
        {
        int i;

        if(!_records)
            {
            _records = new ModelFileRecord[_numberRecords];
            _recordNumbers = new int[_numberRecords];
            _timeNumbers = new int[_numberRecords];
            for(i = 0; i < _numberRecords; i++)
                {
                _recordNumbers[i] = 0;
                _timeNumbers[i] = 0;
                }
            _recordMapper = new int[(_numPoints + 20) / 10];
            for(i = 0; i < (_numPoints + 20) / 10; i++)
                {
                _recordMapper[i] = -1;
                }
            setDelayWrite(true);
            }

        long recordNumber = (triangleNumber / 10) + 1;
        i = _recordMapper[recordNumber];

        if(i == -1)
            {
            i = loadTriangle(recordNumber);
            }
        _timeNumbers[i] = CurTime++;
        ElementTriangleStringRecord* rec = (ElementTriangleStringRecord*)&_records[i];
        doubles = &rec->doubles[triangleNumber % 10];
        ints = &rec->ints[triangleNumber % 10];
        if(setWrite)
            _recordNumbers[i] = -recordNumber;
        }
    return eOk;
    }

ErrorStatus MXTriangleString::getTriangle(const int triangleNumber, ElementTriangleString& data)
    {
    if(triangleNumber >= _numPoints)
        return eInvalidNumber;

    if(_loadedData)
        {
        data = _data[triangleNumber];
        }
    else
        {
        int i;

        if(!_records)
            {
            _records = new ModelFileRecord[_numberRecords];
            _recordNumbers = new int[_numberRecords];
            _timeNumbers = new int[_numberRecords];
            for(i = 0; i < _numberRecords; i++)
                {
                _recordNumbers[i] = 0;
                _timeNumbers[i] = 0;
                }
            _recordMapper = new int[(_numPoints + 20) / 10];
            for(i = 0; i < (_numPoints + 20) / 10; i++)
                {
                _recordMapper[i] = -1;
                }
            setDelayWrite(true);
            }

        long recordNumber = (triangleNumber / 10) + 1;
        i = _recordMapper[recordNumber];

        if(i == -1)
            {
            i = loadTriangle(recordNumber);
            }
        _timeNumbers[i] = CurTime++;
        ElementTriangleStringRecord& rec = (ElementTriangleStringRecord&)_records[i];
        data.doubles = rec.doubles[triangleNumber % 10];
        data.ints = rec.ints[triangleNumber % 10];
        }
    return eOk;
    }

ErrorStatus MXTriangleString::setTriangle(const int triangleNumber, const ElementTriangleString& data)
    {
    assertWriteEnabled();
    if(triangleNumber >= _numPoints)
        return eInvalidNumber;

    if(_loadedData)
        {
        _data[triangleNumber] = data;
        }
    else
        {
        int i;

        if(!_records)
            {
            _records = new ModelFileRecord[_numberRecords];
            _recordNumbers = new int[_numberRecords];
            for(i = 0; i < _numberRecords; i++)
                {
                _recordNumbers[i] = 0;
                _timeNumbers[i] = 0;
                }
            _recordMapper = new int[(_numPoints + 20) / 10];
            for(i = 0; i < (_numPoints + 20) / 10; i++)
                {
                _recordMapper[i] = -1;
                }
            setDelayWrite(true);
            }

        long recordNumber = (triangleNumber / 10) + 1;
        i = _recordMapper[recordNumber];

        if(i == -1)
            {
            i = loadTriangle(recordNumber);
            }
        _timeNumbers[i] = CurTime++;
        ElementTriangleStringRecord& rec = (ElementTriangleStringRecord&)_records[i];
        rec.doubles[triangleNumber % 10] = data.doubles;
        rec.ints[triangleNumber % 10] = data.ints;
        _recordNumbers[i] = -recordNumber;
        }
    return eOk;
    }

ErrorStatus MXTriangleString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

#ifdef MXALLSTRINGTYPES
struct GetBoundaryStruct
    {
    double x1;
    double y1;
    double x2;
    double y2;
    };
ErrorStatus MXTriangleString::getBoundaryString(ArrayClass<ElementContourString>& PrimaryRegion)
    {
    // Now Find Outer Boundary Sides
    // 1. Search Thru looking for Null Triangles <= Null_Level
    // 2. Ignore 0 adjacency
    // 3. Null Triangle Found Then - Check its Adjacency Triangles
    //    One of these triangles (eventually) will be live.
    //    The common side (1-2 or 2-3 or 3-1) will be a boundary vector.
    // 4. Store This Vector
    // 5. Nose-tail joint vectors
    int j;
    int TriVectCount = 0;
    ArrayPtrClass<GetBoundaryStruct> TriVect;

    for(j = 0; j < _numPoints; j++)
        {
        ElementTriangleString MXTria;
        getTriangle(j, MXTria);

        if(MXTria.doubles.pts[0][2] <= -998)
            {
            // Look at adjacent triangles
            // 1st Adj
            ElementTriangleString MXTria2;
            getTriangle(MXTria.ints.adjTri[0] - 1, MXTria2);
            if(MXTria.ints.adjTri[0] > 0 && MXTria2.doubles.pts[0][2] > -999)
                {
                // Link is 1 - 2
                TriVect[TriVectCount].x1 = MXTria.doubles.pts[0][0];
                TriVect[TriVectCount].y1 = MXTria.doubles.pts[0][1];
                TriVect[TriVectCount].x2 = MXTria.doubles.pts[1][0];
                TriVect[TriVectCount].y2 = MXTria.doubles.pts[1][1];
                TriVectCount++;
                }
            // 2nd Adj
            getTriangle(MXTria.ints.adjTri[1] - 1, MXTria2);
            if(MXTria.ints.adjTri[1] > 0 && MXTria2.doubles.pts[0][2] > -999)
                {
                // Link is 2 - 3
                TriVect[TriVectCount].x1 = MXTria.doubles.pts[1][0];
                TriVect[TriVectCount].y1 = MXTria.doubles.pts[1][1];
                TriVect[TriVectCount].x2 = MXTria.doubles.pts[2][0];
                TriVect[TriVectCount].y2 = MXTria.doubles.pts[2][1];
                TriVectCount++;
                }
            // 3rd Adj
            getTriangle(MXTria.ints.adjTri[2] - 1, MXTria2);
            if(MXTria.ints.adjTri[2] > 0 && MXTria2.doubles.pts[0][2] > -999)
                {
                // Link is 3 - 1
                TriVect[TriVectCount].x1 = MXTria.doubles.pts[2][0];
                TriVect[TriVectCount].y1 = MXTria.doubles.pts[2][1];
                TriVect[TriVectCount].x2 = MXTria.doubles.pts[0][0];
                TriVect[TriVectCount].y2 = MXTria.doubles.pts[0][1];
                TriVectCount++;
                }
            }
        }

    // Now Link Vectors Nose To Tail & Put into Drain Project Coords
    // whilst making Region Number 1
    int PrimaryCount = 2;
    GetBoundaryStruct** TriVectArray = TriVect.getArrayPtr();

    PrimaryRegion[0].x = TriVectArray[0]->x1;
    PrimaryRegion[0].y = TriVectArray[0]->y1;
    double NoseX = PrimaryRegion[1].x = TriVectArray[0]->x2;
    double NoseY = PrimaryRegion[1].y = TriVectArray[0]->y2;
    int found;

    TriVect.replace_delete(0);
    TriVectCount--;
    do
        {
        found = 0;
        // Loop Thru For As Many Vectors
        for(j = TriVectCount - 1; j >= 0 ; j--)
            {
            if(fabs(NoseX - TriVectArray[j]->x1) < 0.0015 && fabs(NoseY - TriVectArray[j]->y1) < 0.0015)
                {
                NoseX = PrimaryRegion[PrimaryCount].x = TriVectArray[j]->x2;
                NoseY = PrimaryRegion[PrimaryCount].y = TriVectArray[j]->y2;
                PrimaryCount++;
                TriVect.replace_delete(j);
                TriVectCount--;
                found = 1;
                }
            else if(fabs(NoseX - TriVect[j].x2) < 0.0015 && fabs(NoseY - TriVect[j].y2) < 0.0015)
                {
                NoseX = PrimaryRegion[PrimaryCount].x = TriVectArray[j]->x1;
                NoseY = PrimaryRegion[PrimaryCount].y = TriVectArray[j]->y1;
                PrimaryCount++;
                TriVect.replace_delete(j);
                TriVectCount--;
                found = 1;
                }
            }
        } while(found);
        return eOk;
    }
#endif
//MXContourString
MXRT_STRING_DEFINE_MEMBERS(MXContourString, StringTableRecord); // 2

bool MXContourString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return compareTypes(str->type, 7702) && modelType == 0x20202020;
    }

ErrorStatus MXContourString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(&_data[0], _stringTableRecordDataPos, sizeof(ElementContourString) * _numPoints);
    _stringData = &_data[0];
    _loadedData = true;
    return eOk;
    }

void MXContourString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;

            ElementContourString* _dataP = &_data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }
            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXContourString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

#ifdef MXALLSTRINGTYPES
//MXCadastreString
MXRT_STRING_DEFINE_MEMBERS(MXCadastreString, StringTableRecord); // 2

bool MXCadastreString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return compareTypes(str->type, 7710) && modelType == 0x20202020 && str->stringName[0] == 'P' && (asLong(str->subReference) == asLong("SHEE") || asLong(str->subReference) == asLong("NORT"));
    }

ErrorStatus MXCadastreString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementCadastreString) * _numPoints);
    ElementCadastreString* _dataP = _data[0];
    for(int i = 0; i < _numPoints;i++, _dataP++)
        {
        _dataP->pad1 = 0;
        }
    _stringData = _data[0];
    _loadedData = true;
    return eOk;
    }

void MXCadastreString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;

            ElementCadastreString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP->pad1 = 0;
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                _dataP->pad1 = 0;
                }

            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXCadastreString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }
#endif
//MXTextString
MXRT_STRING_DEFINE_MEMBERS(MXTextString, StringTableRecord); // 2

bool MXTextString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    char* stringName = (char*)&str->stringName;
    return stringName[0] == '*' && modelType == 0x20202020;
    }

ErrorStatus MXTextString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    ModelFilePos pos = _stringTableRecordDataPos;

    int size = recordSize();
    for(int i = 0; i < _numPoints; i++)
        {
        _modelFile->readData(&_data[i], pos, size);
        pos += (size / 4);
        }
    _loadedData = true;
    _stringData = &_data[0];

//    int numDims = _type % 100;
    int type = _type / 1000;
    int mask;
    int i;

    for(i = 0; i < _numPoints; i++)
        {
        long* pt = (long*)_data[i].text;
        long* pt2 = (long*)_data[i].text;

        int ttype = type;
        mask = 2;
        for(int j = 4; j < _type % 100; j++)
            {
            if((ttype % 10) & mask)
                {
                *pt2++ = *pt++;
                pt++;
                }
            else
                *pt2++ = *pt++;
            mask <<= 1;
            if(mask == 8)
                {
                ttype /= 10;
                mask = 1;
                }
            }
        *((char*)pt2) = 0;
        }
    return eOk;
    }

void MXTextString::UpdateFile()
    {
    if(_stringData)
        {
        int recSize = recordSize();
        unsigned char* _fileData = new unsigned char[dataSize()];
        ElementTextString* _fileDataPtr;

//        int numDims = _type % 100;
        int type = _type / 1000;
        int mask;
        int i;

        for(i = 0; i < _numPoints; i++)
            {
            _fileDataPtr = (ElementTextString*)(_fileData + (i * recSize));
            _fileDataPtr->x = _data[i].x;
            _fileDataPtr->y = _data[i].y;
            _fileDataPtr->height = _data[i].height;
            _fileDataPtr->angle = _data[i].angle;

            long* pt = (long*)_data[i].text;
            long* pt2 = (long*)_fileDataPtr->text;

            int ttype = type;
            mask = 2;
            for(int j = 4; j < _type % 100; j++)
                {
                if((ttype % 10) & mask)
                    {
                    *pt2++ = *pt++;
                    *pt2++ = asLong("    ");
                    }
                else
                    *pt2++ = *pt++;
                mask <<= 1;
                if(mask == 8)
                    {
                    ttype /= 10;
                    mask = 1;
                    }
                }
            }

        _modelFile->writeData(_fileData, _stringTableRecordDataPos, dataSize());
        delete [] _fileData;
        }
    }

void MXTextString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        int maxlen = 1;
        if(_numPoints > 0)
            {
            double minx, miny,maxx,maxy;

            ElementTextString* _dataP = &_data[0];
            double x = fabs(_dataP->x);
            double y = fabs(_dataP->y);
            minx = maxx = x;
            miny = maxy = y;
            maxlen = (int)strlen(_dataP->text);
            _dataP++;
            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                x = fabs(_dataP->x);
                y = fabs(_dataP->y);

                if(minx > x)
                    minx = x;
                else if(maxx < x)
                    maxx = x;

                if(miny > y)
                    miny = y;
                else if(maxy < y)
                    maxy = y;

                int len = (int)strlen(_dataP->text);
                if(len > maxlen)
                    maxlen = len;
                }
            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            maxlen = ((maxlen + 3) / 4) + 4;
            if((_type % 100) < maxlen)
                {
                _type = ((_type / 100) * 100) + maxlen;
                }
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXTextString::copy(StringTableRecord*& clonedString) const
    {
    assertReadEnabled();
    MXTextString* newString = MXTextString::cast(this->isA()->create());
    if(!newString)
        newString = new MXTextString;
    strcpy(newString->_stringName, "    ");
    memcpy(newString->_subReference, _subReference, sizeof(_subReference));
    newString->_type = _type;
    newString->setNumPoints(_numPoints);
    newString->_minx = _minx;
    newString->_miny = _miny;
    newString->_maxx = _maxx;
    newString->_maxy = _maxy;

    if(_loadedData)
        {
        memcpy(newString->_stringData, _stringData, _numPoints * sizeof(ElementTextString));
        }
    else
        {
        newString->_stringTableRecordDataPos = _stringTableRecordDataPos;
        newString->setModelFile(_modelFile);
        newString->loadData();

        newString->_stringTableRecordDataPos.setRecordNumber(0);
        newString->_stringTableRecordDataPos.setRecordPos(0);
        newString->_modelTableRecord = 0;
        newString->_modelTable = 0;
        newString->_stringTable = 0;
        newString->_originalDataSize = 0;
        newString->_modelFile = NULL;
        }
    clonedString = newString;
    return eOk;
    }

#ifdef MXACHIVE
ErrorStatus MXTextString::getArchiveStringHeader(void* data)
    {
    long* dataPtr = (long*)data;
    *dataPtr++ = *((long*)_stringName);
    *dataPtr++ = *((long*)_subReference);
    *dataPtr++ = _type;
    *dataPtr++ = _numPoints;
    *dataPtr++ = _minx;
    *dataPtr++ = _miny;
    *dataPtr++ = _maxx;
    *dataPtr++ = _maxy;
    *dataPtr++ = sizeof(ElementTextString) * _numPoints;
    return eOk;
    }
ErrorStatus MXTextString::Archive(unsigned long& size, void*& data)
    {
    assertReadEnabled();
    unsigned char*& dataPtr = (unsigned char*&)data;
    unsigned long dataSize = sizeof(ElementTextString) * _numPoints;
    size += dataSize;
    if(data)
        {
        if(!_loadedData)
            loadData();
        for(int i = 0; i < _data.size(); i++)
            {
            int len = (int)strlen(_data[i].text);
            memset(&_data[i].text[len], 0, 56 - len);
            }
        memcpy(dataPtr, _stringData, dataSize);
        dataPtr += dataSize;
        }
    return eOk;
    }

ErrorStatus MXTextString::Retrieve(void*& data)
    {
    assertWriteEnabled();
    unsigned char*& dataPtr = (unsigned char*&)data;
    unsigned long dataSize = sizeof(ElementTextString) * _numPoints;

    emptyData();
    int numPoints = _numPoints;
    _numPoints = 0;
    setNumPoints(numPoints);
    _numPoints = numPoints;

    memcpy(_stringData, dataPtr, dataSize);
    dataPtr += dataSize;
    return eOk;
    }
#endif

ErrorStatus MXTextString::put(__int64 offset, unsigned long size, void* data)
    {
    assertWriteEnabled();
    if(!_loadedData)
        {
        loadData();
        }

    memcpy(((char*)_stringData) + offset, data, size);
    return eOk;
    }

ErrorStatus MXTextString::get(__int64 offset, unsigned long size, void* data)
    {
    assertReadEnabled();

    if(!_loadedData)
        {
        loadData();
        }
    for(int i = 0; i < (int)_data.size(); i++)
        {
        int len = (int)strlen(_data[i].text);
        memset(&_data[i].text[len], 0, 56 - len);
        }
    memcpy(data, ((char*)_stringData) + offset, size);
    return eOk;
    }

#ifdef MXALLSTRINGTYPES
//MX3dString
MXRT_STRING_DEFINE_MEMBERS(MX3dString, StringTableRecord);  // 3

bool MX3dString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return compareTypes(str->type, 703) && modelType == 0x20202020;
    }

ErrorStatus MX3dString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(Element3dString) * _numPoints);
    _loadedData = true;
    _stringData = _data[0];
    return eOk;
    }

void MX3dString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;
            Element3dString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }
            double xs = 0;
            double ys = 0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MX3dString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

//MXSurveyStationString
MXRT_STRING_DEFINE_MEMBERS(MXSurveyStationString, StringTableRecord);  // 4

bool MXSurveyStationString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    if(str->stringName[0] == 'P' && asLong(str->subReference) == asLong("SSTA") && modelType == 0x20202020)
        {
        if(compareTypes(str->type, 1704))
            return true;
        }
    return false;
    }

ErrorStatus MXSurveyStationString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementSurveyStationString) * _numPoints);
    _loadedData = true;
    _stringData = _data[0];
    return eOk;
    }

void MXSurveyStationString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;
            ElementSurveyStationString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }
            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXSurveyStationString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

//MXSurveyObservationString
MXRT_STRING_DEFINE_MEMBERS(MXSurveyObservationString, StringTableRecord);  // 4

bool MXSurveyObservationString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    if(modelType == 0x20202020)
        {
        if(compareTypes(str->type, 704))
            return true;
        }
    return false;
    }

ErrorStatus MXSurveyObservationString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementSurveyObservationString) * _numPoints);
    ElementSurveyObservationString* _dataP = _data[0];
    _loadedData = true;
    _stringData = _data[0];
    return eOk;
    }

void MXSurveyObservationString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;

            ElementSurveyObservationString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }

            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXSurveyObservationString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

//MXSectionString
MXRT_STRING_DEFINE_MEMBERS(MXSectionString, StringTableRecord);  // 3

bool MXSectionString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return compareTypes(str->type, 1705) && (asLong(str->subReference) != asLong("INTC")) && (asLong(str->subReference) != asLong("INTF")) && modelType == 0x20202020;
    }

ErrorStatus MXSectionString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementSectionString) * _numPoints);
    ElementSectionString* _dataP = _data[0];
    for(int i = 0; i < _numPoints;i++, _dataP++)
        {
        _dataP->pad = 0;
        }
    _loadedData = true;
    _stringData = _data[0];
    return eOk;
    }

void MXSectionString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            ElementSectionString* _dataP = _data[0];
            double minx,miny,maxx,maxy;
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP->pad = 0;
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                _dataP->pad = 0;
                }
            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXSectionString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

//MXInterfaceString
MXRT_STRING_DEFINE_MEMBERS(MXInterfaceString, StringTableRecord);  // 3

bool MXInterfaceString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return compareTypes(str->type, 2705) && (asLong(str->subReference) == asLong("INTC")) || (asLong(str->subReference) == asLong("INTF")) && modelType == 0x20202020;
    }

ErrorStatus MXInterfaceString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementInterfaceString) * _numPoints);
    _loadedData = true;
    _stringData = _data[0];
    return eOk;
    }

void MXInterfaceString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;
            ElementInterfaceString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }

            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXInterfaceString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

//MXMasterString
MXRT_STRING_DEFINE_MEMBERS(MXMasterString, StringTableRecord);  // 3

bool MXMasterString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return compareTypes(str->type, 7706) && modelType == 0x20202020;
    }

ErrorStatus MXMasterString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementMasterString) * _numPoints);
    _loadedData = true;
    _stringData = _data[0];
    return eOk;
    }

void MXMasterString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;
            ElementMasterString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }

            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXMasterString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

//MXGeometryString
MXRT_STRING_DEFINE_MEMBERS(MXGeometryString, StringTableRecord);  // 3

bool MXGeometryString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return (str->type % 100) == 12 && modelType == 0x20202020;
    }

ErrorStatus MXGeometryString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementGeometryString) * _numPoints);
    _loadedData = true;
    _stringData = _data[0];
    return eOk;
    }

void MXGeometryString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;
            ElementGeometryString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }

            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXGeometryString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }
#endif


MXStringDimData::MXStringDimData()
    {
    _data = NULL;
    }

MXStringDimData::MXStringDimData(unsigned char* data, const int type, const int numPoints)
    {
    InitArray(type, numPoints);
    _data = data;
    }

MXStringDimData::MXStringDimData(const int type, const int numPoints)
    {
    _data = NULL;
    Init(type, numPoints);
    }

void MXStringDimData::Init(const int type, const int numPoints)
    {
    if(_data)
        delete [] _data;
    InitArray(type, numPoints);
    _data = new unsigned char[_recSize * _numPoints];
    }

void MXStringDimData::InitArray(const int type, const int numPoints)
    {
//    int numDoubles = 0;
    int temp;
    int i;
    int pos = 0;
    int ttype = type / 100;

    _dimData.empty();
    for(i = 0; i < type % 100; i++)
        {
        if((i % 3) == 0)
            {
            temp = ttype % 10;
            ttype /= 10;
            }
        _dimData[i].offset = pos;
        if(temp & 1)
            {
            _dimData[i].size = 8;
            pos += 8;
            }
        else
            {
            _dimData[i].size = 4;
            pos += 4;
            }
        temp = temp >> 1;
        }
    _numPoints = numPoints;
    _recSize = pos;
    }

ErrorStatus MXStringDimData::getDouble(const int pointNum, const int dim, double& value) const
    {
    if(pointNum > _numPoints || pointNum <= 0)
        return eInvalidData;
    if(dim <= 0 || dim > (int)_dimData.size())
        return eInvalidData;

    if(_dimData[dim - 1].size != 8)
        return eInvalidData;

    int pos = ((pointNum - 1) * _recSize) + _dimData[dim - 1].offset;
    value = *((double*)(_data + pos));
    return eOk;
    }

ErrorStatus MXStringDimData::setDouble(const int pointNum, const int dim, double value)
    {
    if(pointNum > _numPoints || pointNum <= 0)
        return eInvalidData;
    if(dim <= 0 || dim > (int)_dimData.size())
        return eInvalidData;

    if(_dimData[dim - 1].size != 8)
        return eInvalidData;

    int pos = ((pointNum - 1) * _recSize) + _dimData[dim - 1].offset;
    *((double*)(_data + pos)) = value;
    return eOk;
    }

ErrorStatus MXStringDimData::getInt(const int pointNum, const int dim, int& value) const
    {
    if(pointNum > _numPoints || pointNum <= 0)
        return eInvalidData;
    if(dim <= 0 || dim > (int)_dimData.size())
        return eInvalidData;

    if(_dimData[dim - 1].size != 4)
        return eInvalidData;

    int pos = ((pointNum - 1) * _recSize) + _dimData[dim - 1].offset;
    value = *((int*)(_data + pos));
    return eOk;
    }

ErrorStatus MXStringDimData::setInt(const int pointNum, const int dim, int value)
    {
    if(pointNum > _numPoints || pointNum <= 0)
        return eInvalidData;
    if(dim <= 0 || dim > (int)_dimData.size())
        return eInvalidData;

    if(_dimData[dim - 1].size != 4)
        return eInvalidData;

    int pos = ((pointNum - 1) * _recSize) + _dimData[dim - 1].offset;
    *((int*)(_data + pos)) = value;
    return eOk;
    }

ErrorStatus MXStringDimData::getText(const int pointNum, const int dim, const char* & data) const
    {
    if(pointNum > _numPoints || pointNum <= 0)
        return eInvalidData;
    if(dim <= 0 || dim > (int)_dimData.size())
        return eInvalidData;

    /*
    if(_dimData[dim - 1].size != 4)
    return eInvalidData;
    */
    int pos = ((pointNum - 1) * _recSize) + _dimData[dim - 1].offset;
    data = (char*)(_data + pos);
    return eOk;
    }

ErrorStatus MXStringDimData::setText(const int pointNum, const int dim, const char* const data)
    {
    if(pointNum > _numPoints || pointNum <= 0)
        return eInvalidData;
    if(dim <= 0 || dim > (int)_dimData.size())
        return eInvalidData;

    /*
    if(_dimData[dim - 1].size != 4)
    return eInvalidData;
    */
    int pos = ((pointNum - 1) * _recSize) + _dimData[dim - 1].offset;
    strncpy((char*)(_data + pos), (char*)data, _dimData[dim - 1].size);
    return eOk;
    }

int MXStringDimData::type(void) const
    {
    int type = (int)_dimData.size();
//    int numDoubles = 0;
    int i;
    int multiply = 100;

    for(i = 0; i < (int)_dimData.size(); i++)
        {
        if((i % 3) == 0)
            {
            multiply = (int)pow(10.0, 2.0 + (i / 3));
            }

        if(_dimData[i].size == 8)
            {
            type += multiply;
            }
        multiply *= 2;
        }
    return type;
    }

void MXStringDimData::setNumPoints(const int value)
    {
    if(value == _numPoints)
        return;

    unsigned char* data = _data;
    int size = _recSize * _numPoints;

    _numPoints = value;
    _data = new unsigned char[_recSize * _numPoints];
    if(size > _recSize * _numPoints)
        size = _recSize * _numPoints;

    memcpy(_data, data, size);
    delete [] data;
    }

void MXStringDimData::getMinMax(long& minx, long& miny, long& maxx, long& maxy) const
    {
    if(_numPoints == 0)
        {
        minx = -1;
        maxx = -1;
        miny = -1;
        maxy = -1;
        return;
        }
    int i;
    int pos1 = _dimData[0].offset;
    int pos2 = _dimData[1].offset;

    int dbl1= _dimData[0].size == 8;
    int dbl2 = _dimData[1].size == 8;

    if(dbl1)
        minx = maxx = (long)*((double*)(_data + pos1));
    else
        minx = maxx = (long)*((int*)(_data + pos1));

    if(dbl2)
        miny = maxy = (long)*((double*)(_data + pos2));
    else
        miny = maxy = (long)*((int*)(_data + pos2));

    long vx;
    long vy;

    for(i = 1; i < _numPoints; i++)
        {
        if(dbl1)
            vx = (long)*((double*)(_data + pos1));
        else
            vx = (long)*((int*)(_data + pos1));

        if(dbl2)
            vy = (long)*((double*)(_data + pos2));
        else
            vy = (long)*((int*)(_data + pos2));

        if(minx > vx)
            minx = vx;
        else if(maxx < vx)
            maxx = vx;

        if(miny > vy)
            miny = vy;
        else if(maxy < vy)
            maxy = vy;

        pos1 += _recSize;
        pos2 += _recSize;
        }
    }

MXRT_STRING_DEFINE_MEMBERS(MXProxyString, StringTableRecord); // 2

bool MXProxyString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return false;
    }

MXProxyString::~MXProxyString()
    {
    void* stringData = _stringData;
    StringTableRecord::emptyData();
    if(stringData)
        delete [] stringData;
    }

void MXProxyString::emptyData(void)
    {
    if(_stringData)
        delete [] _stringData;
    _stringData = NULL;
    _loadedData = false;
    }

ErrorStatus MXProxyString::loadData(void)
    {
    assertReadEnabled();
    long dataSize = recordSize() * _numPoints;
    _stringData = new unsigned char[dataSize];
    _numPointsInData = _numPoints;
    _modelFile->readData(_stringData, _stringTableRecordDataPos, dataSize);
    _loadedData = true;
    return eOk;
    }

void MXProxyString::UpdateData(void)
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            long recSize = recordSize();

            double minx,miny,maxx,maxy;

            if(((_type / 100) & 3) == 0)
                {
                long* data = (long*)_stringData;
                minx = maxx = abs(data[0]);
                miny = maxy = abs(data[1]);
                for(int i = 1; i < _numPoints; i++)
                    {
                    data = (long*)(((unsigned char*)_stringData) + (i * recSize));
                    if(minx > abs(data[0]))
                        minx = abs(data[0]);
                    else if(maxx < abs(data[0]))
                        maxx = abs(data[0]);

                    if(miny > abs(data[1]))
                        miny = abs(data[1]);
                    else if(maxy < abs(data[1]))
                        maxy = abs(data[1]);
                    }
                minx /= 1000;
                maxx /= 1000;
                miny /= 1000;
                maxy /= 1000;
                }
            else
                {
                double* data = (double*)_stringData;
                minx = maxx = fabs(data[0]);
                miny = maxy = fabs(data[1]);
                for(int i = 1; i < _numPoints; i++)
                    {
                    data = (double*)(((unsigned char*)_stringData) + (i * recSize));
                    if(minx > fabs(data[0]))
                        minx = fabs(data[0]);
                    else if(maxx < fabs(data[0]))
                        maxx = fabs(data[0]);

                    if(miny > fabs(data[1]))
                        miny = fabs(data[1]);
                    else if(maxy < fabs(data[1]))
                        maxy = fabs(data[1]);
                    }
                }
            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

void MXProxyString::SetNumPoints(const int i)
    {
    if(!_loadedData)
        loadData();
    if(i <= _numPointsInData)
        return;

    long recSize = recordSize();
    void* data = new char[recSize * i];

    if(_stringData)
        {
        memcpy(data, _stringData, (recSize * _numPointsInData));
        delete [] _stringData;
        }
    _numPointsInData = i;
    _stringData = data;
    }

#ifdef MXALLSTRINGTYPES
//MXAreaString
MXRT_STRING_DEFINE_MEMBERS(MXAreaString, StringTableRecord); // 2

bool MXAreaString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return compareTypes(str->type, 177710) && modelType == 0x20202020 && asLong(str->subReference) == asLong("AREA");
    }

ErrorStatus MXAreaString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementAreaString) * _numPoints);
    _stringData = _data[0];
    _loadedData = true;
    return eOk;
    }

void MXAreaString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;

            ElementAreaString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }

            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXAreaString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

//MXEndAreaString
MXRT_STRING_DEFINE_MEMBERS(MXEndAreaString, StringTableRecord); // 2

bool MXEndAreaString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return compareTypes(str->type, 177710) && modelType == 0x20202020 && asLong(str->subReference) == asLong("ENDA");
    }

ErrorStatus MXEndAreaString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementEndAreaString) * _numPoints);
    _stringData = _data[0];
    _loadedData = true;
    return eOk;
    }

void MXEndAreaString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;

            ElementEndAreaString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }
            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXEndAreaString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

//MXVolumeString
MXRT_STRING_DEFINE_MEMBERS(MXVolumeString, StringTableRecord); // 2

bool MXVolumeString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return compareTypes(str->type, 177710) && modelType == 0x20202020 && asLong(str->subReference) == asLong("VOLM");
    }

ErrorStatus MXVolumeString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementVolumeString) * _numPoints);
    _stringData = _data[0];
    _loadedData = true;
    return eOk;
    }

void MXVolumeString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;

            ElementVolumeString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }

            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXVolumeString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }

//MXVisibilityString
MXRT_STRING_DEFINE_MEMBERS(MXVisibilityString, StringTableRecord); // 2

bool MXVisibilityString::isType(const StringTableFileRecord* str, const long modelType) //const int type, const long name, const long subref)
    {
    return compareTypes(str->type, 177710) && modelType == 0x20202020 && asLong(str->subReference) == asLong("VISI");
    }

ErrorStatus MXVisibilityString::loadData()
    {
    assertReadEnabled();
    _data.setPhysicalLength(_numPoints);
    _data.setLogicalLength(_numPoints);

    _modelFile->readData(_data[0], _stringTableRecordDataPos, sizeof(ElementVisibilityString) * _numPoints);
    _stringData = _data[0];
    _loadedData = true;
    return eOk;
    }

void MXVisibilityString::UpdateData()
    {
    if((isModified() || isNewRecord()) && !isErased() && _loadedData)
        {
        if(_numPoints > 0)
            {
            double minx,miny,maxx,maxy;

            ElementVisibilityString* _dataP = _data[0];
            minx = maxx = fabs(_dataP->x);
            miny = maxy = fabs(_dataP->y);
            _dataP++;

            for(int i = 1; i < _numPoints;i++, _dataP++)
                {
                if(minx > fabs(_dataP->x))
                    minx = fabs(_dataP->x);
                else if(maxx < fabs(_dataP->x))
                    maxx = fabs(_dataP->x);

                if(miny > fabs(_dataP->y))
                    miny = fabs(_dataP->y);
                else if(maxy < fabs(_dataP->y))
                    maxy = fabs(_dataP->y);
                }

            double xs =0;
            double ys =0;
            if(_modelFile)
                _modelFile->getModelShift(xs, ys);
            _minx = (long)floor(minx - xs);
            _maxx = (long)ceil(maxx - xs);
            _miny = (long)floor(miny - ys);
            _maxy = (long)ceil(maxy - ys);
            if(_minx == _maxx)
                _minx--;
            if(_miny == _maxy)
                _miny--;
            }
        }
    StringTableRecord::UpdateData();
    }

ErrorStatus MXVisibilityString::copy(StringTableRecord*& clonedString) const
    {
    return StringTableRecord::copy(clonedString);
    }
#endif


// From File filemapper.cpp
#ifdef FREESPACEMAPPER
int ModelFileFreeSpace::findPos(ModelFilePos& start)
    {
    int startPos = 0;
    int endPos = _usedSpace.size();
    MFUsedSpace** usedSpace = _usedSpace.getArrayPtr();
    int pos = endPos / 2;

    if(endPos == 0)
        return 0;
    if(endPos == 1)
        {
        if(usedSpace[0]->end > start)
            return 0;
        return 1;
        }
    while(1)
        {
        if(usedSpace[pos]->end == start)
            return pos + 1;
        else if(usedSpace[pos]->end < start)
            {
            startPos = pos;
            }
        else
            {
            endPos = pos;
            }
        if(endPos <= startPos)
            {
            if(usedSpace[pos]->end <= start)
                return pos + 1;
            else
                return pos;
            }
        pos = startPos + ((endPos - startPos) / 2);
        if(pos == startPos)
            {
            if(usedSpace[pos]->end <= start)
                return pos + 1;
            else
                return pos;
            }
        }
    }

bool ModelFileFreeSpace::addBlock(ModelFilePos& start, ModelFilePos& end)
    {
    int i;
    int num = _usedSpace.size();

    i = findPos(start);
    if(i > 0)
        {
        if(_usedSpace[i - 1].end == start)
            {
            if(i < num && _usedSpace[i].start == end)
                {
                _usedSpace[i - 1].end = _usedSpace[i].end;
                _usedSpace.delete_entry(i);
                return true;
                }
            _usedSpace[i - 1].end = end;

            return true;
            }
        }

    if(i < num)
        {
        if(_usedSpace[i].start == end)
            {
            _usedSpace[i].start = start;
            return true;
            }
        }

    _usedSpace.insert_entry(i);
    _usedSpace[i].start = start;
    _usedSpace[i].end = end;
    if(_usedSpace.size() <= (i + 1))
        return true;
    else
        return _usedSpace[i + 1].start > end;
    };

bool ModelFileFreeSpace::getFreeData(long size, ModelFilePos& pos, bool startNewRecord) const
    {
    int num = _usedSpace.size();
    for(int i = 0; i < num - 1; i++)
        {
        ModelFilePos startPos = _usedSpace[i].end;
        if(startNewRecord && startPos.getRecordPos() != 1)
            startPos += 501 - startPos.getRecordPos();
        if(size <= (_usedSpace[i + 1].start - startPos))
            {
            pos = startPos;
            return true;
            }
        }
    pos = _usedSpace[num].end;
    if(startNewRecord && pos.getRecordPos() != 1)
        pos+= 501 - pos.getRecordPos();
    return false;
    }

void ModelFileFreeSpace::removeBlock(ModelFilePos& start, ModelFilePos& end)
    {
    int i;
    int num = _usedSpace.size();
    if(start.getRecordNumber() == 2)
        return;
    i = findPos(start);
    if(_usedSpace[i].start < start)
        {
        if(_usedSpace[i].end == end)
            {
            _usedSpace[i].end = start;
            }
        else
            {
            _usedSpace.insert_entry(i);
            _usedSpace[i].start = _usedSpace[i + 1].start;
            _usedSpace[i].end = start;
            _usedSpace[i + 1].start = end;
            }
        }
    else if(_usedSpace[i].start == start)
        {
        if(_usedSpace[i].end == end)
            _usedSpace.delete_entry(i);
        else
            _usedSpace[i].start = end;
        }
    };

void ModelFileFreeSpace::report(void) const
    {
    return;
    }
#endif

// From File modelid.cpp
class ModelFileMap
    {
    private:
        long nextNum;
        bmap<long, ModelObject*> _ObjectMapper;
    public:
        MXModelFile* _modelFile;
        ModelFileMap()
            {
            nextNum = 1;
            }

        ModelObject* GetObject(long id)
            {
            ModelObject* ret = _ObjectMapper[id];
            if(!ret)
                _ObjectMapper.erase(id);
            return ret;
            }

        long addObject(ModelObject* object)
            {
            _ObjectMapper[nextNum] = object; //.insert(nextNum, object);
            return nextNum++;
            }
        void removeObject(long id)
            {
            _ObjectMapper.erase(id);
            }
    };

class ModelFileMapper
    {
    private:
        int modelFileNum;
        Bentley::bmap<short, ModelFileMap*> _modelMapper;
    public:

        ModelFileMapper()
            {
            modelFileNum = 1;
            }
        ~ModelFileMapper()
            {
            Bentley::bmap<short, ModelFileMap*>::iterator it = _modelMapper.begin();
            while(it != _modelMapper.end())
                {
                (*it).second->_modelFile->ReleaseModelFileNumber();
                it++;
                }
            }
        ModelFileMap* getModelFile(short modelFileNumber)
            {
            Bentley::bmap<short, ModelFileMap*>::const_iterator it = _modelMapper.find(modelFileNumber);

            if(it != _modelMapper.end())
                return (*it).second;
            return NULL;
            }

        int addModelFile(MXModelFile* modelFile)
            {
            ModelFileMap* mfm = new ModelFileMap;
            mfm->_modelFile = modelFile;
            while(_modelMapper[modelFileNum])
                modelFileNum++;
            _modelMapper[modelFileNum] = mfm; //.insert(modelFileNum, mfm);
            return modelFileNum++;
            }
        void removeModelFile(short modelFileNumber)
            {
            ModelFileMap* mfm = _modelMapper[modelFileNumber];
            delete mfm;
            _modelMapper.erase(modelFileNumber);
            }
    };

ModelFileMapper mapper;
ErrorStatus MXOpenModelObject(ModelObjectId& id, ModelObject*& object, ModelObject::OpenStatus status)
    {
    ModelFileMap* mfm = mapper.getModelFile(id.modelFileNumber);
    if(!mfm)
        return eInvalidId;
    object = mfm->GetObject(id.objectNumber);

    if(!object)
        return eInvalidId;

    return object->open(status);
    }

int addModelFile(MXModelFile* modelFile)
    {
    return mapper.addModelFile(modelFile);
    }

void removeModelFile(int number)
    {
    mapper.removeModelFile(number);
    }

int addObject(int number, ModelObject* object)
    {
    ModelFileMap* mfm = mapper.getModelFile(number);
    if(mfm)
        return mfm->addObject(object);
    return 0;
    }

void removeObject(int number, int objectNumber)
    {
    ModelFileMap* mfm = mapper.getModelFile(number);
    if(mfm)
        mfm->removeObject(objectNumber);
    }










END_BENTLEY_TERRAINMODEL_NAMESPACE












    /*
    ToDo

    Implement an Open/Close System. For Read and Write.
    Do all writes need to be done striaght away or on close/commit?

    ModelFile
    setData(ModelFilePos, length, buffer);
    getData(ModelFilePos, length, buffer);

    If the ModelTableRecord is closed then delete the stringTable. // ??
    ModelTable AddModel.
    ModelTable DeleteModel.

    StringTable
    newIterator

    StringTableIterator
    -- See ModelTableIterator

    StringTableRecord
    StringName
    Subreference
    Type
    NumPoints
    MinX
    MinY
    MaxX
    MaxY
    stringDataPos

    AddModel(const char* const ModelName)

    Find next free model space, Search through (recordNumber, offset) for gap.
    Insert in modelTableRecord into the correct place. Insert a blank string Record at the end of the model file.
    Write out the modeltablerecord include date, if(record wasn't 1 then Modify the next free record.)

    GetStringData
    This will load the stringData into data.

    AddString(StringTableRecord& stringTable)


    Should I use ArrayPtrClasses????
    */



//Change arrays and maps to use the bmap/blist etc
//Create unmanaged classes to get models and get files. all this is interested in is Models and triangulation strings.