/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Source.h $
|    $RCSfile: Source.h,v $
|   $Revision: 1.21 $
|       $Date: 2011/12/20 16:23:59 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/SourceReference.h>
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct SourceBase;
END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE
struct SourceRegistry;
END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Forward declarations
+---------------+---------------+---------------+---------------+---------------+------*/
struct Source;
struct ContentDescriptor;
struct ContentConfig;
struct ContentConfigPolicy;

typedef SharedPtrTypeTrait<Source>::type    SourcePtr; 
typedef SharedPtrTypeTrait<const Source>::type  
                                            SourceCPtr;    

/*---------------------------------------------------------------------------------**//**
* @description  This class is responsible for the creation of new sources from a 
*               specified source references. This process may also be viewed as
*               the opening of a source.
*               
*    
* @see SourceRegistry
* @see SourceRef
* @see Source
* @see ContentConfig
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceFactory : private Unassignable
    {

private:
    struct                                  Impl;
    SharedPtrTypeTrait<Impl>::type          m_pImpl;
public:

    typedef Plugin::SourceRegistry          Registry;

    IMPORT_DLLE explicit                    SourceFactory                  (Log&                    log = GetDefaultLog());
    IMPORT_DLLE explicit                    SourceFactory                  (const Registry&         registry,
                                                                            Log&                    log = GetDefaultLog());

    IMPORT_DLLE                             SourceFactory                  (const SourceFactory&    rhs);

    IMPORT_DLLE                             ~SourceFactory                 ();

    IMPORT_DLLE SourcePtr                   Create                         (const SourceRef&        sourceRef) const;

    IMPORT_DLLE SourcePtr                   Create                         (const SourceRef&        sourceRef,
                                                                            SMStatus&                 status) const;

    IMPORT_DLLE SourcePtr                   Create                         (const SourceRef&        sourceRef,
                                                                            SMStatus&                 status,
                                                                            StatusInt&              statusEx) const;

    // TDORAY:  Add a FindCreatorFor interface once Creator interface specified just below becomes available. Will
    //          have to decide what will be the creator not found semantic. I personally think that the creator should 
    //          not be help by a Ptr but have a copy semantic instead. Maybe the null object pattern would be appropriate?
    };


// TDORAY:  It would be good to add a creator interface here so that user could get a hold of it once 
//          and not have to duplicate Supports search. This would greatly enhance performances for 
//          some applications. Consider the null object pattern for not found or made invalid 
//          creator (e.g.: would be good for supporting cases were user unregisters his source).
//          It these cases, an event trapping pattern should be implemented (probably listener pattern) 
//          in order to invalidate (make null) the instance once unregister calls are trapped.
//
//          Another element that would be interesting to add in this interface would be a "locate" or
//          "reach" or "is reachable" method so that it becomes possible for the user to only validate
//          whether the source could be found without necessarily (in most cases) opening it. For this,
//          we would only be required to implement one locate per SourceRef type and maybe provide
//          a default which would uses the already existing opening functionality to perform the 
//          action. This approach would imply another plug-in interface specifically for per source ref
//          actions. Not certain if there should be one plug-in per action or if we should combine these...


/*---------------------------------------------------------------------------------**//**
* @description  This class represents an open data source and has the following 
*               responsibilities:
*               - providing accurate description of the content of this source.
*               - managing resources required in view of its extraction
*
*               User may access the content description of this source in order 
*               to be able to know what content can be extracted from this source.
*
*               If early disposal of the source is required, user may trigger this
*               disposal by invoking the Close method. If not invoked, source will
*               be disposed of automatically once out of scope.
*
* @see Plugin::V0::Source
* @see InputExtractor
* @see SourceFactory
* @see ContentDescriptor
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Source : private Uncopyable, public ShareableObjectTypeTrait<Source>::type
    {

private:
    friend struct                           InternalSourceHandler;

    typedef Plugin::V0::SourceBase          Base;
    typedef const std::type_info*           ClassID;

    std::auto_ptr<Base>                     m_baseP;
    ClassID                                 m_classID;
    Base&                                   m_originalBase;
    SourceImportConfig*                     m_sourceImportConf;

    explicit                                Source                         (Base*                   baseP);
public:
    IMPORT_DLLE                             ~Source                        ();

    ClassID                                 GetClassID                     () const { return m_classID; }

    IMPORT_DLLE const ContentDescriptor&    GetDescriptor                  () const;    

    IMPORT_DLLE const WChar*             GetTypeCStr                    () const;

    IMPORT_DLLE SMStatus                      Close                          ();

    IMPORT_DLLE SourceImportConfig*         GetSourceImportConfig()                {return m_sourceImportConf;}

    IMPORT_DLLE SourceImportConfig*         GetSourceImportConfigC() const               {return m_sourceImportConf;}

    IMPORT_DLLE void                       SetImportConfig(SourceImportConfig* sourceImportConf) {m_sourceImportConf = sourceImportConf;}
    };


IMPORT_DLLE SourcePtr                       Configure                      (const SourcePtr&                sourcePtr,
                                                                            const ContentConfig&            config,
                                                                            Log&                            log = GetDefaultLog());


IMPORT_DLLE SourcePtr                       Configure                      (const SourcePtr&                sourcePtr,
                                                                            const ContentConfig&            config,
                                                                            const ContentConfigPolicy&      policy, 
                                                                            Log&                            log = GetDefaultLog());

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
