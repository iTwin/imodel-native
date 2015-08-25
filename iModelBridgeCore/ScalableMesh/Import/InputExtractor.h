/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/InputExtractor.h $
|    $RCSfile: InputExtractor.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/08/26 18:46:46 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once


#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Memory/Packet.h>
#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/Import/ExtractionConfig.h> // TDORAY: Remove... Only for query


BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
struct Log;
END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct InputExtractorBase;
struct InputExtractorCreatorBase;
END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE



BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct Source;
struct InputExtractor;
struct InputExtractorCreator;
struct ExtractionConfig;
struct ExtractionQuery; // TDORAY: Include once in its own header


typedef SharedPtrTypeTrait<InputExtractor>::type
                                                InputExtractorPtr;






/*---------------------------------------------------------------------------------**//**
* @description  Base class for implementing specific importer plug-ins. 
*   
* @see          Source
* @see          InputExtractorPluginCreator
*
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct InputExtractor : private Uncopyable, public ShareableObjectTypeTrait<InputExtractor>::type
    {
private:
    friend struct                               InputExtractorCreator;
    typedef Plugin::V0::InputExtractorBase      Base;
    
    std::auto_ptr<Base>                         m_baseP;

    explicit                                    InputExtractor                     (Base*                           implP);

public:
                                                ~InputExtractor                    ();

    void                                        Read                               ();
    bool                                        Next                               ();
    };


/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct InputExtractorCreator
    {
    typedef const void*                         SourceClassID;
private:
    typedef Plugin::V0::InputExtractorCreatorBase   
                                                Base;
    const Base*                                 m_baseP;
    SourceClassID                               m_sourceClassID;

public:
    typedef const Base*                         ID;

    explicit                                    InputExtractorCreator              (const Base&                     impl);
                                                ~InputExtractorCreator             ();

    // Using default copy
   
    ID                                          GetID                              () const { return m_baseP; }

    bool                                        Supports                           (const DataType&                 type) const;

    void                                        AdaptOutputType                    (DataType&                       type) const;

    RawCapacities                               GetOutputCapacities                (Source&                         source,
                                                                                    const ExtractionQuery&          query) const;


    SourceClassID                               GetSourceClassID                   () const { return m_sourceClassID; }

    InputExtractorPtr                           Create                             (Source&                         source,
                                                                                    const ExtractionQuery&          query,
                                                                                    PacketGroup&                    rawEntities,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const;

    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
