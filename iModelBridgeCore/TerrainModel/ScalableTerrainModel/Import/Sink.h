/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/Sink.h $
|    $RCSfile: Sink.h,v $
|   $Revision: 1.6 $
|       $Date: 2011/09/01 14:06:58 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/Exceptions.h>
#include <ScalableTerrainModel/Import/Warnings.h>

#include <ScalableTerrainModel/Memory/Packet.h>
#include <ScalableTerrainModel/Import/DataType.h>

#include <ScalableTerrainModel/Import/ContentDescriptor.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct Sink;
struct BackInserter;

typedef SharedPtrTypeTrait<BackInserter>::type  
                                            BackInserterPtr;
typedef SharedPtrTypeTrait<Sink>::type      SinkPtr;


/*---------------------------------------------------------------------------------**//**
* @description  
* // TDORAY: Make it a plugin
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Sink : private Uncopyable, public ShareableObjectTypeTrait<Sink>::type
    {
private:
    struct                                  Impl;
    std::auto_ptr<Impl>                     m_pImpl;

    virtual ContentDescriptor               _CreateDescriptor      () const = 0;

    virtual BackInserter*                   _CreateBackInserterFor (UInt                        layerID,
                                                                    const DataType&             type,
                                                                    Log&                 log) const = 0;

protected:
    IMPORT_DLLE explicit                    Sink                   ();

public:
    IMPORT_DLLE virtual                     ~Sink                  () = 0;

    IMPORT_DLLE const ContentDescriptor&    GetDescriptor          () const;

    // TDORAY: Add insertion config replace layerID and type for insertion query.

    IMPORT_DLLE BackInserterPtr             CreateBackInserterFor  (const PacketGroup&          packets,
                                                                    UInt                        layerID,
                                                                    const DataType&             type,
                                                                    Log&                        log) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* // TDORAY: Make it a plug-in. 
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct BackInserter : private Uncopyable, public ShareableObjectTypeTrait<BackInserter>::type
    {
private:
    friend struct                           Sink;

    struct                                  Impl;
    std::auto_ptr<Impl>                     m_pImpl;

    virtual void                            _Assign                (const PacketGroup&          packets) = 0;
        
    virtual void                            _Write                 () = 0;

protected:
    IMPORT_DLLE explicit                    BackInserter           ();
public:
    IMPORT_DLLE virtual                     ~BackInserter          () = 0;

    IMPORT_DLLE void                        Write                  () { _Write(); }
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE