//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFileFactory.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFRasterFileFactory
//-----------------------------------------------------------------------------
// This class describes the basic interface of a raster file format
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HRFRasterFile.h"

class  HFCURL;
class  HRFCallback;
class  IHRFPWFileHandler;

class HRFRasterFileFactory
    {
public:

    // The type of the registry of raster file creators
    typedef map<HCLASS_ID, HRFRasterFileCreator*>
    CreatorsMap;
    typedef vector<HRFRasterFileCreator*, allocator<HRFRasterFileCreator* > >
    Creators;

    HRFRasterFileFactory ();
    virtual ~HRFRasterFileFactory();

    // Raster File object instanciation

    _HDLLg HFCPtr<HRFRasterFile>  OpenFile     (const HFCPtr<HFCURL>&           pi_rpURL,
                                                bool                           pi_AsReadOnly = false,
                                                uint64_t                       pi_Offset = 0) const;

    HFCPtr<HRFRasterFile>         OpenFileAs   (const HFCPtr<HFCURL>&           pi_rpURL,
                                                const HRFRasterFileCreator*     pi_pCreator,
                                                bool                           pi_AsReadOnly = false,
                                                uint64_t                       pi_Offset = 0) const;

    _HDLLg HFCPtr<HRFRasterFile>  NewFile      (const HFCPtr<HFCURL>&           pi_rpURL,
                                                uint64_t                       pi_Offset = 0) const;

    _HDLLg HFCPtr<HRFRasterFile>  NewFileAs    (const HFCPtr<HFCURL>&           pi_rpURL,
                                                const HRFRasterFileCreator*     pi_pCreator,
                                                uint64_t                       pi_Offset = 0) const;

    // function like C style fopen()
    // These open function can reopen or create a new file with the specified access mode
    _HDLLg HFCPtr<HRFRasterFile>  OpenFile     (const HFCPtr<HFCURL>&           pi_rpURL,
                                                HFCAccessMode                   pi_AccessMode,
                                                uint64_t                       pi_Offset = 0) const;

    HFCPtr<HRFRasterFile>         OpenFileAs   (const HFCPtr<HFCURL>&           pi_rpURL,
                                                const HRFRasterFileCreator*     pi_pCreator,
                                                HFCAccessMode                   pi_AccessMode,
                                                uint64_t                       pi_Offset = 0) const;

    // Add the creators to the registry
    _HDLLg void                   RegisterCreator (const HRFRasterFileCreator*   pi_pCreator);

    _HDLLg void                   SetRasterDllDirectory(HCLASS_ID         pi_ClassID,
                                                  const WString&    pi_rDir);

    // Search with the URL, AccessMode and offset the appropriate creator
    void                          SetFactoryScanOnOpen(bool pi_FactoryScanOnOpen);
    bool                         GetFactoryScanOnOpen() const;

    _HDLLg const HRFRasterFileCreator*   FindCreator(const HFCPtr<HFCURL>& pi_rpURL,
                                                     HFCAccessMode            pi_AccessMode = HFC_READ_WRITE,
                                                     uint64_t             pi_Offset = 0) const;

    // Obtains the creators list - logic const function
    // HFC_READ_ONLY  : obtains the formats that can read data
    // HFC_WRITE_ONLY : obtains the formats that can write data
    // HFC_CREATE_ONLY : obtains the formats that can create data
    // Any other combinaison : obtains all formats that can read data, write data or create data.
    _HDLLg const Creators&               GetCreators(HFCAccessMode pi_AccessMode = HFC_READ_WRITE) const;

    // Obtains the creators that include the specified access mode.
    _HDLLg void                          GetCreators(Creators& creators, HFCAccessMode pi_AccessMode = HFC_READ_ONLY) const;

    _HDLLg HRFRasterFileCreator*        GetCreator(HCLASS_ID pi_ClassID) const;

    // Same as above, but it is returned as a map of class_ids to creators.  The associated
    // class id is the one for the raster file.
    _HDLLg const CreatorsMap&            GetCreatorsMap(HFCAccessMode pi_AccessMode = HFC_READ_WRITE) const;

    _HDLLg HFCAccessMode                 DetectAccessMode(const HFCPtr<HFCURL>& pi_rpURL) const;

    _HDLLg void                     RegisterCallback(HRFCallback* pi_pCallback);
    _HDLLg void                     UnregisterCallback(HRFCallback* pi_pCallback);

    _HDLLg void                     RegisterPWHandler(IHRFPWFileHandler* pi_pHandler);
    _HDLLg IHRFPWFileHandler*       GetPWHandler() const;


protected:

private:
    // Raster File Creators registry
    Creators    m_Creators;
    Creators    m_ReadCreators;
    Creators    m_WriteCreators;
    Creators    m_CreateCreators;
    CreatorsMap m_CreatorsMap;
    CreatorsMap m_ReadCreatorsMap;
    CreatorsMap m_WriteCreatorsMap;
    CreatorsMap m_CreateCreatorsMap;

    bool       m_FactoryScanOnOpen;

    typedef map<HCLASS_ID, WString> DllDirMap;
    DllDirMap   m_DllDir;

    // ProjectWise Handler
    IHRFPWFileHandler*  m_pPWHandler;

    // Disabled methods
    HRFRasterFileFactory(const HRFRasterFileFactory&);
    HRFRasterFileFactory& operator=(const HRFRasterFileFactory&);

    // Singleton
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFRasterFileFactory)
    };

