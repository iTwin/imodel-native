#pragma once
using namespace std;
#include <list>
#include <BeXml/BeXml.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <TerrainModel/TerrainModel.h>


#include <ScalableMesh/ScalableMeshDefs.h>
//#include <ScalableMesh\IScalableMeshMoniker.h>
#include <ScalableMesh/IScalableMeshStream.h>
#include <ScalableMesh/IScalableMeshURL.h>
#include <ScalableMesh/Import/ScalableMeshData.h>
#include <ScalableMesh/IScalableMeshSources.h>
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshSourceCollection.h>

#include <ScalableTerrainModel/IMrDTMSourceCollection.h>


USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
namespace ScalableMeshSDKexe
    {

    class FileFinder
        {
        public:

            explicit               FileFinder();
            virtual                ~FileFinder();

            void                   FindFiles(const WString&          pi_rSourceFolderPath,
                                                      WString&                pi_FilePaths,
                                                      bool                    pi_SearchSubFolders) const;

           bool                   ParseFilePaths(WString&                pio_FilePaths,
                                                 WString&                pio_FirstPath) const;
        };
    
    
    BENTLEY_NAMESPACE_NAME::MrDTM::IDTMSourcePtr CreateSourceFor(const WString&                    sourcePath,
                                                  BENTLEY_NAMESPACE_NAME::MrDTM::DTMSourceDataType importedType);   
                                                       
    bool ParseSourceSubNodes(BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMSourceCollection& sourceCollection,                              
                             bvector<DPoint3d>&                           importClipShape,                             
                             DRange3d&                                    importRange, 
                             BeXmlNodeP                                   pTestNode);
    
    };
