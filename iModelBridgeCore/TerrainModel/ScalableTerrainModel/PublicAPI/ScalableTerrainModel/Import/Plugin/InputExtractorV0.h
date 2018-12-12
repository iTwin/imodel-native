/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Plugin/InputExtractorV0.h $
|    $RCSfile: InputExtractorV0.h,v $
|   $Revision: 1.18 $
|       $Date: 2011/11/22 16:26:59 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Foundations/Log.h>
#include <ScalableTerrainModel/Import/Exceptions.h>
#include <ScalableTerrainModel/Import/Warnings.h>

#include <ScalableTerrainModel/Memory/Packet.h>
#include <ScalableTerrainModel/Memory/PacketAccess.h>
#include <ScalableTerrainModel/Import/Source.h>
#include <ScalableTerrainModel/Import/ExtractionConfig.h> // TDORAY: Remove, Only there for query

#include <ScalableTerrainModel/Import/Plugin/ExtractorRegistry.h>


BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct Source;
struct InputExtractor;
struct InputExtractorCreator;

struct ExtractionConfig;
struct DataType;
END_BENTLEY_MRDTM_IMPORT_NAMESPACE


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)

struct SourceBase;

/*---------------------------------------------------------------------------------**//**
* @description  Base class for implementing specific InputExtractor plug-ins. This base is
*               not designed to be used directly but under the hood of Importer.
*
*               Implementations of this class need to be instantiated via specialization
*               of InputExtractorCreator.
*           
*               Requisites:
*                   - Simple implementations of InputExtractorCreator expect an importer
*                     plug-in's constructor to take its associated file plug-in as it sole 
*                     argument. Specializations of InputExtractor are guaranteed that their
*                     associated source plug-in instances will outlive them. 
*
* @see          InputExtractorCreator
*
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct InputExtractorBase : private Uncopyable
    {
private:
    friend struct                               InputExtractor;
    friend struct                               InputExtractorCreator;
    friend struct                               InputExtractorCreatorBase;

    struct                                      Impl;   
    std::auto_ptr<Impl>                         m_pImpl;

    IMPORT_DLLE virtual bool                    _Finalize                          ();

    virtual void                                _Assign                            (PacketGroup&                    packets) = 0;

    virtual void                                _Read                              () = 0;
    virtual bool                                _Next                              () = 0;

protected:
    struct                                      Handler;

    typedef DataType                            DataType; // Help avoid name conflicts when global namespace pollution occurs

    IMPORT_DLLE explicit                        InputExtractorBase                 ();

public:
    IMPORT_DLLE virtual                         ~InputExtractorBase                () = 0;

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct InputExtractorBase::Handler
    {
    static bool                                 Finalize                           (InputExtractorBase&             instance);

    static void                                 Assign                             (InputExtractorBase&             instance,
                                                                                    PacketGroup&                    packets);

    static void                                 Read                               (InputExtractorBase&             instance);
    static bool                                 Next                               (InputExtractorBase&             instance);
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @see InputExtractorCreatorBase
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct InputExtractorCreatorBase : private Uncopyable
    {
protected:
    typedef const void*                         SourceClassID;
private:
    friend struct                               InputExtractorCreator;

    struct                                      Impl; 
    std::auto_ptr<Impl>                         m_pImpl;

    virtual SourceClassID                       _GetSourceClassID                  () const = 0;


    // NTERAY: It would maybe be a nice add if this one also took Source/LayerID as supplemental parameters?
    virtual bool                                _Supports                          (const DataType&                 type) const = 0;

    IMPORT_DLLE virtual void                    _AdaptOutputType                   (DataType&                       type) const;

    virtual RawCapacities                       _GetOutputCapacities               (SourceBase&                     sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          query) const = 0;

    virtual InputExtractorBase*                 _Create                            (SourceBase&                     sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          query,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                            log) const = 0;

protected:
    typedef DataType                            DataType; // Help avoid name conflicts when global namespace pollution occurs

    struct                                      ExtractorHandler;

    IMPORT_DLLE explicit                        InputExtractorCreatorBase          ();

public:
    typedef const InputExtractorCreatorBase*    ID;

    IMPORT_DLLE virtual                         ~InputExtractorCreatorBase         () = 0;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct InputExtractorCreatorBase::ExtractorHandler : public InputExtractorBase::Handler
    {
    };


/*---------------------------------------------------------------------------------**//**
* @description  Helper template that facilitate implementation of InputExtractorCreator
*               by providing basic functionalities. Inherit your custom 
*               InputExtractor creator class from this helper class.
*
*    
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SourceT>
struct InputExtractorCreatorMixinBase : public InputExtractorCreatorBase
    {
private:
    virtual SourceClassID                       _GetSourceClassID                  () const override
        {
        return typename SourceT::s_GetClassID();
        }

    virtual RawCapacities                       _GetOutputCapacities               (SourceBase&                     sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          query) const override
        {
        assert(0 != dynamic_cast<SourceT*>(&sourceBase));
        return _GetOutputCapacities(static_cast<SourceT&>(sourceBase), source, query);
        }

    virtual RawCapacities                       _GetOutputCapacities               (SourceT&                        sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          query) const = 0;

    virtual InputExtractorBase*                 _Create                            (SourceBase&                     sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          query,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                            log) const override
        {
        assert(0 != dynamic_cast<SourceT*>(&sourceBase));
        return _Create(static_cast<SourceT&>(sourceBase), source, query, config, log);
        }

    virtual InputExtractorBase*                 _Create                            (SourceT&                        sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          query,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                            log) const = 0;
   
    };


// TDORAY: Move to hpp?
inline bool InputExtractorBase::Handler::Finalize (InputExtractorBase& instance)
    { return instance._Finalize(); }

inline void InputExtractorBase::Handler::Assign(InputExtractorBase& instance,
                                                PacketGroup&        packets)
    { instance._Assign(packets); }

inline void InputExtractorBase::Handler::Read (InputExtractorBase& instance)
    { instance._Read(); }

inline bool InputExtractorBase::Handler::Next (InputExtractorBase& instance)
    { return instance._Next(); }

END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE