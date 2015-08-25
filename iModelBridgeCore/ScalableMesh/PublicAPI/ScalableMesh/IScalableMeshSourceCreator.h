/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceCreator.h $
|    $RCSfile: IScalableMeshSourceCreator.h,v $
|   $Revision: 1.39 $
|       $Date: 2015/07/15 10:35:02 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMeshCreator.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshSourceCreator;
typedef RefCountedPtr<IScalableMeshSourceCreator>            IScalableMeshSourceCreatorPtr;

//This is the creator interface to use when providing a series of source files to import data to the Scalable Mesh. All details of indexing, etc are handled
//automatically. At the moment, it is not possible to import data from source files and also manually create nodes in the index.
struct IScalableMeshSourceCreator : public IScalableMeshCreator
    {
    private:
        /*__PUBLISH_SECTION_END__*/
        friend struct                       IScalableMeshCreator;
        struct                              Impl;
        //std::auto_ptr<Impl>                 m_implP;

        explicit                            IScalableMeshSourceCreator(Impl*                       implP);

        /*__PUBLISH_SECTION_START__*/

    public:
        BENTLEYSTM_EXPORT virtual                 ~IScalableMeshSourceCreator();

        BENTLEYSTM_EXPORT static IScalableMeshSourceCreatorPtr GetFor(const WChar*              filePath,
                                                                    StatusInt&                status);

        BENTLEYSTM_EXPORT static IScalableMeshSourceCreatorPtr GetFor(const IScalableMeshPtr&     scmPtr,
                                                                    StatusInt&                  status);
        // TDORAY: For next versions: Add overloads taking as parameters a working dir path and maybe a listing 
        //         of environment variables. This supplementary information will enable STM relocation without 
        //         sources relocation (by specifying previous STM dir as working dir). Another solution could
        //         be to provide a Relocate functionality.

        BENTLEYSTM_EXPORT const IDTMSourceCollection& GetSources() const;
        BENTLEYSTM_EXPORT IDTMSourceCollection&       EditSources();

        BENTLEYSTM_EXPORT void                    SetSourcesDirty();
        BENTLEYSTM_EXPORT bool                    HasDirtySources() const;

        BENTLEYSTM_EXPORT bool                    AreAllSourcesReachable() const;

        BENTLEYSTM_EXPORT StatusInt               UpdateLastModified();

        BENTLEYSTM_EXPORT void                    ResetLastModified();

#ifdef SCALABLE_MESH_ATP

        BENTLEYSTM_EXPORT static unsigned __int64 GetNbImportedPoints();

        BENTLEYSTM_EXPORT static double GetImportPointsDuration();

        BENTLEYSTM_EXPORT static double GetLastBalancingDuration();

        BENTLEYSTM_EXPORT static double GetLastMeshingDuration();

        BENTLEYSTM_EXPORT static double GetLastFilteringDuration();

        BENTLEYSTM_EXPORT static double GetLastStitchingDuration();
#endif   


    };

END_BENTLEY_SCALABLEMESH_NAMESPACE