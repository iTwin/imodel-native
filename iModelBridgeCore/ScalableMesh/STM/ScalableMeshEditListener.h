/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshEditListener.h $
|    $RCSfile: ScalableMeshEditListener.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/09/13 13:30:57 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/IScalableMeshTime.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct EditListener
    {
private:
    virtual void                            _NotifyOfPublicEdit        () = 0;
    virtual void                            _NotifyOfLastEditUpdate    (Time            updatedLastEditTime) = 0;

protected:
    virtual                                 ~EditListener              () = 0 {}

public: 
    void                                    NotifyOfPublicEdit         ();

    void                                    NotifyOfLastEditUpdate     (Time            updatedLastEditTime);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
