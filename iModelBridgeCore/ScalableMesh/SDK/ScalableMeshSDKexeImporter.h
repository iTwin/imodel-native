#pragma once
using namespace std;
#include <list>
#include <BeXml/BeXml.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <ScalableMesh\IScalableMeshMoniker.h>
#include <ScalableMesh\IScalableMeshStream.h>
#include <ScalableMesh\IScalableMeshURL.h>
#include <ScalableMesh\Import\ScalableMeshData.h>
#include <ScalableMesh\IScalableMeshSources.h>
#include <ScalableMesh\IScalableMeshSourceImportConfig.h>
#include <ScalableMesh\IScalableMeshSourceCollection.h>

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

    bool ParseSourceSubNodes(IDTMSourceCollection& sourceCollection, BeXmlNodeP pTestNode);
    IDTMSourcePtr CreateSourceFor(const WString&          sourcePath,
                                  DTMSourceDataType importedType,
                                  BeXmlNodeP        pTestChildNode = 0);
    bool AddOptionToSource(IDTMSourcePtr srcPtr, BeXmlNodeP pTestChildNode);
    void GetSourceDataType(DTMSourceDataType& dataType, BeXmlNodeP pSourceNode);
    };