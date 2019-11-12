/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    virtual                                 ~EditListener              () {}

public: 
    void                                    NotifyOfPublicEdit         ();

    void                                    NotifyOfLastEditUpdate     (Time            updatedLastEditTime);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
