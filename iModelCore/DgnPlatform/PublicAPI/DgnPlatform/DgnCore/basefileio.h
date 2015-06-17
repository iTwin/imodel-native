/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/basefileio.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <Geom/msgeomstructs.hpp>
#include <DgnPlatform/DgnCore/DgnFileIO.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! @bsiclass                                                     Ray.Bentley   10/05
//=======================================================================================
struct BaseFileType : DgnFileType
{
    BaseFileType (int format) : DgnFileType (format){}

    virtual void GetCapabilities (DgnFileCapabilities *cap) override;
    virtual bool                ValidateFile    (int *pFormat, int *pMajorVersion, int *pMinorVersion, bool *pDefaultModelIs3D, IThumbnailPropertyValuePtr*, const WChar *pName) = 0;
    virtual DgnFileIOP Factory (DgnDbP pFile) = 0;
    virtual int _GetFormat () = 0;
    virtual IThumbnailPropertyValuePtr ReadThumbnail (WCharCP pName);

};

//=======================================================================================
//! @bsiclass                                                     RayBentley  01/05
//=======================================================================================
class BaseFileIO : public DgnFileIO
{
private:
    double     m_scaleToDGN;
    Transform  m_transformToDGN;

protected:
    // If the file type can be written, you must override _CanExclusiveLock to return true, and you must override DgnFileIO's _OnCreateFile and _WriteChanges methods.
    // If _CanExclusiveLock is not overridden, all file writing is prevented and you do not need to override the DgnFileIO Write-oriented methods to return ERROR - they'll never be called.
    virtual bool _CanExclusiveLock () override { return false; }
    virtual StatusInt _OnCreateFile (DesignFileHeader* pHdr, WChar const* pFileName, DgnFileIO* pDonor) override { return (ERROR); }
    virtual StatusInt _LoadModelIndex (IStorage* pRootStore) override;

public:
    BaseFileIO (DgnFile* pFile) : DgnFileIO (pFile){}
    virtual ~BaseFileIO () {}

    // methods that are concerned with reading files, overriding methods in DgnFileIO.
    virtual int _GetFormat () override = 0;
    virtual StatusInt _GetModelReader (DgnModelReader** ppModelReader, DgnModel* pCache) = 0;
    virtual StatusInt _CreateModelAndLoadHeader (DgnModelP& cache, ModelId modelIDtoRead) override;
    virtual StatusInt _LoadFile (DesignFileHeader* pHdr, bool* openedReadonly, StatusInt* rwStatus, DgnFileOpenMode openMode) override;
    virtual StatusInt _CloseDgnFileIO () override;
    virtual void* GetReadHandle () override { return 0; }
    virtual int GetReadIStorage (void**) override { return ERROR; }

    virtual double GetFileTime () override;

    // these methods are not in DgnFileIO. They can't be called from DgnFileObj.
    virtual bool ValidateFile () = 0;
    virtual void GetBackgroundColor (RgbColorDef* pColor) { pColor->red = pColor->green = pColor->blue = 150; }
    virtual bool UseSeedLevels () { return true; }
    Transform const*         GetTransformToDGN       ()                         { return &m_transformToDGN; }
    double GetScaleToDGN () { return m_scaleToDGN; }
    virtual StatusInt WriteDictionaryElements (bool doFullSave) override { return ERROR; }
    virtual StatusInt GetUnits (UnitInfo* pStorageUnits, UnitInfo* pMasterUnits, UnitInfo* pSubUnits) { return ERROR; }
    static StatusInt GetSeedFileName (WChar*);
    static WChar*             s_defaultModelName;

protected:
    StatusInt GetModelInfoFromSeed (ModelInfo* pModelInfo);
};

//=======================================================================================
//! @bsiclass                                                     RayBentley  01/05
//=======================================================================================
class BaseModelReader : public DgnModelReader
{
public:
    BaseModelReader         (DgnModel* pCache, BaseFileIO* pBaseFileIO);
    ~BaseModelReader        ();
    virtual StatusInt _ReadSectionsIntoCache (uint32_t* majorVersion, uint32_t* minorVersion, DgnModelSections sections) override;

protected:

    virtual StatusInt LoadSeedFileElements ();
    StatusInt LoadElementsIntoCache (DgnElementDescrP pDescrChain);
    virtual StatusInt LoadGeometry (Transform const* pTranformToDGN) = 0;
    virtual StatusInt LoadNonModelElements (const Transform* pTransformToDGN, double scaleToDGN) { return SUCCESS; }
    virtual StatusInt LoadSavedViews (const Transform* pTransformToDGN, double scaleToDGN, const DRange3d * pRange) { return SUCCESS; }
    virtual StatusInt LoadDefaultViewports ();
    virtual StatusInt GetDefaultView (RotMatrixP pRMatrix, DPoint3dP pOrigin, DPoint3dP pDelta, double* pActiveZ, ViewFlags* pFlags, DPoint3dP pCamera, double* pFocalLength, const Transform* pTransformToDGN, double scaleToDGN, DRange3d const*) { return ERROR; }
    virtual void LoadHeaderData (TcbP);
    virtual bool UseSeedLevels () { return true; }
    virtual void InitializeViewElementDefaults (ViewElement* pViewElement);

    BaseFileIO*                 m_pBaseFileIO;
    DgnModelP                m_modelRef;
    DRange3d                    m_range;
};

END_BENTLEY_NAMESPACE
