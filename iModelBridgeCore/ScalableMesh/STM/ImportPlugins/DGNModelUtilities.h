/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/DGNModelUtilities.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh\ScalableMeshLib.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct DGNFileHolder;
struct DGNModelRefHolder;


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                    FindDGNLevelIDFromName                 (DgnModelRefP            modelRefP,
                                                                                const wchar_t*          levelName,
                                                                                LevelId&                levelID);


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNFileHolder                           OpenDGNFile                            (const wchar_t*          pName,
                                                                                StatusInt&              status);



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier    11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNModelRefHolder                       FindDGNModel                           (const DGNFileHolder&    dgnFile,
                                                                                ModelId                 modelID,
                                                                                StatusInt&              status);


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNModelRefHolder                       FindDGNModel                           (const DGNFileHolder&    dgnFile,
                                                                                const wchar_t*          modelName,
                                                                                StatusInt&              status);


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNModelRefHolder                       FindDGNReferenceFromRootModel          (const DGNModelRefHolder&
                                                                                                        rootModel,
                                                                                const WChar*             rootToRefPersitentPath,
                                                                                StatusInt&              status);

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNFileHolder
    {
private:
    DgnFileP                            m_dgnFileP;

    explicit                            DGNFileHolder                          (DgnFileP                dgnFileP);

public:
    static DGNFileHolder                CreateFromWorking                      (DgnFileP                dgnFileP);
    static DGNFileHolder                CreateFromWorking                      (DgnFilePtr              dgnFilePtr);
    static DGNFileHolder                CreateFromActive                       (DgnFileP                dgnFileP);
    
    explicit                            DGNFileHolder                          ();
                                        DGNFileHolder                          (const DGNFileHolder&    rhs);

                                        ~DGNFileHolder                         ();

    DGNFileHolder&                      operator=                              (const DGNFileHolder&    rhs);


    DgnFileP                            GetP                                   () const;

    void                                Reset                                  ();
    };




/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Jean-Francois.Cote   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNModelRefHolder
    {
private:
    struct Impl;
    struct NullImpl;
    struct WorkingImpl;
    struct ActiveImpl;
    struct ReferenceImpl;


    HFCPtr<Impl>                 m_implP;

    explicit                            DGNModelRefHolder                      (Impl*                       implP);
public:

    static DGNModelRefHolder            CreateFromWorking                      (const DGNFileHolder&        dgnFile,
                                                                                DgnModelRefP                modelRefP);

    static DGNModelRefHolder            CreateFromWorking                      (DgnModelRefP                modelRefP);

    static DGNModelRefHolder            CreateFromActive                       (const DGNFileHolder&        dgnFile,
                                                                                DgnModelRefP                modelRefP);

    static DGNModelRefHolder            CreateFromActive                       (DgnModelRefP                modelRefP);

    static DGNModelRefHolder            CreateFromReference                    (const DGNModelRefHolder&    rootModel,
                                                                                DgnModelRefP                referenceModelRefP);

    explicit                            DGNModelRefHolder                      ();

                                        DGNModelRefHolder                      (const DGNModelRefHolder&    rhs);

    DGNModelRefHolder&                  operator=                              (const DGNModelRefHolder&    rhs);


    DgnModelRefP                        GetP                                   () const;

    void                                Reset                                  ();

    };


#include "DGNModelUtilities.hpp"

END_BENTLEY_SCALABLEMESH_NAMESPACE
