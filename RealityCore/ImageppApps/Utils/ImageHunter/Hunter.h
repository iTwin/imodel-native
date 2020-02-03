/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/Hunter.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/Hunter.h,v 1.4 2010/06/02 13:16:45 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : Hunter
//-----------------------------------------------------------------------------
// Loads supported capabilities and executes search using supplied criterias.
//-----------------------------------------------------------------------------

#pragma once

#include "ICriteria.h"

//-----------------------------------------------------------------------------
// Hunter class
//-----------------------------------------------------------------------------
ref class Hunter
{
public:
    static Hunter^                      GetInstance();
    void                                AddCriteria(SupportedCapability capability, ICriteria^ criteria);
    void                                RemoveCriteria(SupportedCapability capability, ICriteria^ criteria);
    void                                RemoveAllCriterias();
    void                                RegisterBuilder(ICriteriaBuilder^ builder);
    ICriteriaBuilder^                   GetBuilder(System::String^ builderName);
    System::Collections::ArrayList^     GetCriteriasList(FilterType filter);
    System::Collections::ArrayList^     GetCriteriaBuilders();
    System::Collections::ArrayList^     GetValidImages();
    System::Collections::Hashtable^     GetSupportedCapabilities(SupportedCapability index);
    void                                Hunt(array<System::String^>^ paths, bool isRecursive, System::Collections::Hashtable^ excludedExtensions,
                                             System::ComponentModel::ProgressChangedEventHandler^ progressCallback, 
                                             System::ComponentModel::RunWorkerCompletedEventHandler^ completedCallback);
    void                                Search(array<System::String^>^ paths, bool isRecursive);
    int                                 GetCurrentFileNo();
    int                                 GetTotalNumberOfFiles();
    System::String^                     GetCurrentScannedImage();
    void                                CancelHunt();

private:
                                        Hunter();
    void                                Init();
    void                                DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);
    int                                 CountNumberOfFiles(array<System::String^>^ paths);

    System::ComponentModel::BackgroundWorker^  m_bw;
    static Hunter^                      m_instance;
    bool                                m_isRecursiveSearch;
    int                                 m_numberOfFiles;
    int                                 m_currentFileNo;
    System::String^                     m_currentScannedFile;
    System::Collections::Hashtable^     m_criteriasList;
    System::Collections::ArrayList^     m_criteriasBuilder;
    System::Collections::ArrayList^     m_validImages;
    System::Collections::Hashtable^     m_supportedCapabilities;
    System::Collections::Hashtable^     m_ExcludedExtensions;
};
