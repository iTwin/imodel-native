/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/Sink.h $
|    $RCSfile: Sink.h,v $
|   $Revision: 1.6 $
|       $Date: 2011/09/01 14:06:58 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/Exceptions.h>
#include <ScalableMesh/Import/Warnings.h>

#include <ScalableMesh/Memory/Packet.h>
#include <ScalableMesh/Import/DataType.h>

#include <ScalableMesh/Import/ContentDescriptor.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

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

    virtual BackInserter*                   _CreateBackInserterFor (uint32_t                        layerID,
                                                                    const DataType&             type,
                                                                    Log&                 log) const = 0;

protected:
     explicit                    Sink                   ();

public:
     virtual                     ~Sink                  () = 0;

     const ContentDescriptor&    GetDescriptor          () const;

    // TDORAY: Add insertion config replace layerID and type for insertion query.

     BackInserterPtr             CreateBackInserterFor  (const PacketGroup&          packets,
                                                                    uint32_t                        layerID,
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
    
    virtual void                            _Assign                (const PacketGroup&          packets) = 0;
        
    virtual void                            _Write                 () = 0;

    virtual void                            _NotifySourceImported() {};

protected:

     bool m_is3dData;
     bool m_isGridData;

     explicit                    BackInserter           ();
public:
     virtual                     ~BackInserter          () = 0;

     void                        SetIs3dData(bool is3dData);

     void                        SetIsGridData(bool isGridData);

     void                        Write                  () { _Write(); }     

     void                        NotifySourceImported() { _NotifySourceImported(); }
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
