/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/ImportPlugins/XYZFileImporter.cpp $
|    $RCSfile: XYZFileImporter.cpp,v $
|   $Revision: 1.42 $
|       $Date: 2012/01/17 16:06:21 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableTerrainModelPCH.h>

#include <ScalableTerrainModel/Import/Plugin/InputExtractorV0.h>
#include <ScalableTerrainModel/Import/Plugin/SourceV0.h>

#include <ScalableTerrainModel/Plugin/IMrDTMPolicy.h>
#include <ScalableTerrainModel/Plugin/IMrDTMFileUtilities.h>

#include <ScalableTerrainModel/Type/IMrDTMPoint.h>

#include <STMInternal/Foundations/PrivateStringTools.h>


USING_NAMESPACE_BENTLEY_MRDTM_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY_MRDTM

using namespace Bentley::MrDTM::Plugin;

namespace { //BEGIN UNAMED NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                IsAsciiFile                            (const WChar*  filePath, 
                                                                            StatusInt&      status)
    {   
    struct IsKindOf
        {
        bool operator () (bool isKindOf, const FileRange& sampleRange) const
            {
            const WChar* sampleIt = sampleRange.begin;
            while ((sampleIt < sampleRange.end) && isascii(*sampleIt)) 
                { ++sampleIt; }

            return isKindOf && sampleIt == sampleRange.end;
            }
        };

    // TDORAY: Consider the possibility of computing separator quantity/nature and validating whether this quantity/nature 
    //         remains equal to what was computed in previous samples.


    return FileAnalysisTool<20000, 5>::Run(filePath, IsKindOf(), true, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                IsBinaryFile                           (const WChar*  filePath, 
                                                                            StatusInt&      status)
    {
    // TDORAY: This is not sufficient analysis. It is not because the file is not ASCII that it is 
    // binary. File can be in any other encodings such as Unicode, MB, etc. In these cases,
    // the driver will try to load the file when it shouldn't. Investigate implementing
    // a custom IsKindOfFileTool for this purpose. This may bloat the code a little more
    // but allow more flexibility. We should check if checking for invalid double values
    // (e.g. QNAN and others) should be is sufficient validation. Checking for file size 
    // multiple of 3 *(sizeof double) would also be a nice addition.
    return !IsAsciiFile(filePath, status) &&
           BSISUCCESS == status;
    }



/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class XYZSource : public SourceMixinBase<XYZSource>
    {
public:
    FILE*                           GetFile                () {return m_pFile;}

private:
    friend class                    XYZSourceCreator;

    FILE*                           m_pFile;

    
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        XYZSource        (const WString&   pi_rFilePath) 
        : m_pFile(_wfopen (pi_rFilePath.c_str(), L"rb")) 
        {
        if (0 == m_pFile)
            throw FileIOException();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                 () override
        {
        if (0 != m_pFile)
            {
            fclose (m_pFile);
            m_pFile = 0;
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor          _CreateDescriptor  () const override
        {
        // TDORAY: Should we compute an extent??
        return ContentDescriptor
            (
            L"",
            LayerDescriptor(L"",
                            PointType3d64fCreator().Create(), 
                            GCS::GetNull(), 
                            0)
            );
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*         _GetType                   () const override
        {
        return L"Binary XYZ";
        }


    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class XYZSourceCreator : public LocalFileSourceCreatorBase
    {
    virtual ExtensionFilter         _GetExtensions                         () const override
        {
        return L"*.xyz";
        }    

    virtual bool                    _Supports                              (const LocalFileSourceRef&   pi_rSourceRef) const override
        {
        if(!DefaultSupports(pi_rSourceRef))
            return false;


        StatusInt status;
        return IsBinaryFile(pi_rSourceRef.GetPathCStr(), status) && 
                            BSISUCCESS == status;
        }

    virtual SourceBase*             _Create                                (const LocalFileSourceRef&   pi_rSourceRef,
                                                                            Log&                 pi_warningLog) const override
        {
        return new XYZSource(pi_rSourceRef.GetPath());
        }
    };

const SourceRegistry::AutoRegister<XYZSourceCreator> s_RegisterXYZBinaryFile;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class XYZPointExtractor : public InputExtractorBase
    {
private:
    friend class                    XYZPointExtractorCreator;

    // Dimension groups definition
    enum 
        {
        DG_XYZ,
        DG_QTY,
        };


    static const size_t             MAX_POINT_QTY = 500000;
    static const size_t             POINT_TYPE_SIZE = sizeof(DPoint3d);

    XYZSource&                      m_rFile;

    PODPacketProxy<DPoint3d>        m_pointPacket;

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        XYZPointExtractor        (XYZSource&        pi_rFile) 
        :   m_rFile(pi_rFile)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Assign                        (PacketGroup&     pi_rRawEntities) override
        {
        m_pointPacket.AssignTo(pi_rRawEntities[DG_XYZ]);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Read                          () override
        {
        if (feof(m_rFile.GetFile()))
            {
            m_pointPacket.SetSize(0);
            return;
            }

        const size_t readPtsQty = fread (m_pointPacket.Edit(), POINT_TYPE_SIZE, MAX_POINT_QTY, m_rFile.GetFile());
        m_pointPacket.SetSize(readPtsQty);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        // File pointer was already moved forward.
        return !feof(m_rFile.GetFile());
        }

    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class XYZPointExtractorCreator : public InputExtractorCreatorMixinBase<XYZSource>
    {
    virtual bool                                _Supports                          (const DataType&                 type) const override
        {
        return type.GetFamily() == PointTypeFamilyCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (XYZSource&                      sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        return RawCapacities (XYZPointExtractor::POINT_TYPE_SIZE*XYZPointExtractor::MAX_POINT_QTY);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (XYZSource&                      sourceBase,
                                                                                    const Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                     log) const override
        {
        // TDORAY: Ensure file is reset at begin
        return new XYZPointExtractor(sourceBase);
        }
    };

const ExtractorRegistry::AutoRegister<XYZPointExtractorCreator> s_RegisterXYZImporter;

} //END UNAMED NAMESPACE