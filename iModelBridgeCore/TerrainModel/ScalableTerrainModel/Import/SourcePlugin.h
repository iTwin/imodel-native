/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/SourcePlugin.h $
|    $RCSfile: SourcePlugin.h,v $
|   $Revision: 1.10 $
|       $Date: 2012/02/16 00:36:46 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE
struct Log;
END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct SourceCreatorBase;
struct LocalFileSourceCreatorBase;
struct DGNElementSourceCreatorBase;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct Source;
struct SourceRef;
struct LocalFileSourceRef;
struct DGNElementSourceRef;

typedef SharedPtrTypeTrait<Source>::type        SourcePtr; 

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceCreator
    {
private:
    typedef Plugin::V0::SourceCreatorBase       Base;

    const Base*                                 m_baseP;
public:
    typedef const Base*                         ID;

    explicit                                    SourceCreator                      (const Base&                         impl);
                                                ~SourceCreator                     ();

    // Using default copy

    ID                                          GetID                              () const { return m_baseP; }

    bool                                        Supports                           (const SourceRef&                    sourceRef) const;
    SourcePtr                                   Create                             (const SourceRef&                    sourceRef,
                                                                                    Log&                         log) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalFileSourceCreator
    {
private:
    typedef Plugin::V0::LocalFileSourceCreatorBase  
                                                Base;

    const Base*                                 m_baseP;
public:
    typedef bvector<WString>::const_iterator    ExtensionCIter;

    typedef const Base*                         ID;

    explicit                                    LocalFileSourceCreator             (const Base&                         impl);
                                                ~LocalFileSourceCreator            ();

    // Using default copy

    ID                                          GetID                              () const { return m_baseP; }

    bool                                        Supports                           (const LocalFileSourceRef&           sourceRef) const;
    SourcePtr                                   Create                             (const LocalFileSourceRef&           sourceRef,
                                                                                    Log&                         log) const;

    ExtensionCIter                              ExtensionsBegin                    () const;
    ExtensionCIter                              ExtensionsEnd                      () const;
    };




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNElementID
    {
private:
    UInt                                    m_type;
    UInt                                    m_handlerID;
public:
    explicit                                DGNElementID                       (UInt                                type,
                                                                                UInt                                handlerID);

    friend bool                             operator<                          (const DGNElementID&                 lhs,
                                                                                const DGNElementID&                 rhs);

    friend bool                             operator==                         (const DGNElementID&                 lhs,
                                                                                const DGNElementID&                 rhs);

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNElementSourceCreator
    {


private:
    typedef Plugin::V0::DGNElementSourceCreatorBase  
                                                Base;

    const Base*                                 m_baseP;
    DGNElementID                                m_elementID;
public:
    typedef const Base*                         ID;


    explicit                                    DGNElementSourceCreator            (const Base&                         base);
                                                ~DGNElementSourceCreator           ();


    // Using default copy

    ID                                          GetID                              () const { return m_baseP; }

    UInt                                        GetElementType                     () const;
    UInt                                        GetElementHandlerID                () const;

    DGNElementID                                GetElementID                       () const { return m_elementID; }

    bool                                        Supports                           (const DGNElementSourceRef&          sourceRef) const;
    SourcePtr                                   Create                             (const DGNElementSourceRef&          sourceRef,
                                                                                    Log&                                log) const;

    };



END_BENTLEY_MRDTM_IMPORT_NAMESPACE