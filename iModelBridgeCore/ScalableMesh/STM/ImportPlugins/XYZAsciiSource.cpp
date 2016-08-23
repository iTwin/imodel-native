/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/XYZAsciiSource.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"
#include <ScalableMesh\Import\DataType.h>
#include <ScalableMesh\Import\DataTypeDescription.h>
#include <ScalableMesh\Import\Metadata.h>
#include <ScalableMesh\Import\Plugin\InputExtractorV0.h>
#include <ScalableMesh\Import\Plugin\SourceV0.h>
#include <ScalableMesh\Type\IScalableMeshPoint.h>

/*
 * Header that were originally brought by the PCH.
 */

/*
 * TDORAY:  these should not be required. The need of including these means that
 *          some headers are not self contained or contains errors when compiled
 *          with default options. See my notes.
 */
#pragma warning( disable:4800 )
using std::pair;

#undef F1
#undef F2
     
#include "XYZAsciiFormat.h"
#include "PluginUtils.h"

                    
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY_SCALABLEMESH

using namespace BENTLEY_NAMESPACE_NAME::ScalableMesh::Plugin;
using namespace std;

namespace { //BEGIN_UNAMED_NAMESPACE

//#define XYZASCII_PLUGIN_USES_STD_STREAM_BASED_IMPLEMENTATION

typedef FILE* XYZAsciiStream;
                                        
} //END_UNNAMED_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  Specialize FileAnalysisStreamTrait with new file stream implementation
*               used to allow very large file to be supported. Will allow us to
*               use FileAnalysisTool with this new file stream type.
* @bsiclass                                                 Raymond.Gauthier   01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template <>
struct FileAnalysisStreamTrait<XYZAsciiStream>
    {
    typedef __int64                     SizeType;
    typedef __int64                     OffsetType;
    typedef streampos                   PositionType;
    typedef size_t                      ReadCountType;
    typedef __int64                     ReadSizeType;

    static const OffsetType             ZERO_OFFSET = 0;

    static bool                         GetFileSize                                (XYZAsciiStream fileStream, SizeType& size)
        {                                                
        if (ferror (fileStream))
            {
            size = 0;
            return false;
            }            

        _fseeki64(fileStream, ZERO_OFFSET, SEEK_END);
        size = _ftelli64(fileStream);
        _fseeki64(fileStream, ZERO_OFFSET, SEEK_SET);                        
        
        return (ferror (fileStream) == 0);
        }

    static bool                         SeekBegin                                  (XYZAsciiStream fileStream)
        {
        _fseeki64(fileStream, ZERO_OFFSET, SEEK_SET);                        
        
        return (ferror (fileStream) == 0);
        }

    static bool                         SeekByOffset                               (XYZAsciiStream fileStream, OffsetType offset)
        {        
        _fseeki64(fileStream, offset, SEEK_SET);                                        

        return (ferror (fileStream) == 0);
        }

    static ReadSizeType                 Read                                       (XYZAsciiStream fileStream, WChar buffer[], ReadCountType count)
        {
        static vector<char> s_tempBuffer(count); //WIP_CHAR_OK 
        
        if (s_tempBuffer.capacity() < count) s_tempBuffer.resize(count);
            
        size_t size = fread (&s_tempBuffer[0], 1, count, fileStream);
           
        const ReadSizeType readSize = (ferror (fileStream) == 0) ? size : ReadSizeType(-1);

        if (readSize != -1)
            {
            mbstowcs(buffer, &s_tempBuffer[0], readSize);            
            }

        return readSize;
        }

    static bool                         Eof                                        (const XYZAsciiStream fileStream)
        {            
        return feof (fileStream) != 0;
        }
    };
END_BENTLEY_SCALABLEMESH_NAMESPACE

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
* @bsiclass                                                 Raymond.Gauthier   01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LineStats
    {
    enum ErrorFlag
        {
        EF_INVALID_FORMAT = 0x01,
        };

    uint32_t            errorFlags;
    uint32_t            validCount;
    uint32_t            commentedCount;
    uint32_t            fieldCount;
    WChar            fieldDelimiter;


    explicit LineStats (bool valid = false)
        :   errorFlags(valid ? 0 : EF_INVALID_FORMAT),
            validCount(0),
            commentedCount(0),
            fieldCount(0),
            fieldDelimiter(XYZFormat::INVALID_DELIMITER) {}

    operator bool () const
        {
        return 0 == errorFlags && XYZFormat::MINIMUM_FIELD_COUNT <= fieldCount && XYZFormat::INVALID_DELIMITER != fieldDelimiter;
        }

    void SetInvalidFormat () { errorFlags |= EF_INVALID_FORMAT; }

    bool IsInvalidFormat () const { return 0 != (errorFlags & EF_INVALID_FORMAT); }

    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier   01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LineStats ComputeFileProfile(XYZAsciiStream file, StatusInt& status)
    {
    struct ComputeSampleProfileBase : protected XYZFormat
        {
    protected:
        static void  ComputeRemainingStats (LineStats& stats, FileRange fieldsRanges[], FileRange& lineRange, const FileRange& range)
            {
            bool insideRange;
            while (insideRange = GetNextLineRange(lineRange, lineRange.end, range.end))
                {
                if (IsCommentOrEmptyLine(lineRange))
                    {
                    ++stats.commentedCount;
                    continue;
                    }

                const uint32_t fieldCount = SplitLine(fieldsRanges, lineRange);
                const WChar delimiter = GetDelimiterFor(fieldsRanges, fieldCount);

                if (fieldCount != stats.fieldCount ||
                    delimiter != stats.fieldDelimiter)
                    {
                    stats.SetInvalidFormat();
                    break;
                    }

                ++stats.validCount;
                }
            }

        };

    struct ComputeInitialSampleProfile : private ComputeSampleProfileBase
        {
        LineStats operator () (const LineStats& previousStats, const FileRange& range) const
            {
            LineStats stats(previousStats);

            FileRange fieldsRanges[MAX_FIELD_COUNT];
            FileRange lineRange(range.begin, range.begin);

            assert(0 == stats.validCount);

            bool insideRange = GetFirstLineRange (lineRange, lineRange.end, range.end);
            while (insideRange && IsCommentOrEmptyLine(lineRange))
                {
                ++stats.commentedCount;
                insideRange = GetNextLineRange(lineRange, lineRange.end, range.end);
                }

            uint32_t fieldCount;
            WChar delimiter;

            if (!insideRange ||
                MINIMUM_FIELD_COUNT > (fieldCount = SplitLine(fieldsRanges, lineRange)) ||
                INVALID_DELIMITER == (delimiter = GetDelimiterFor(fieldsRanges, fieldCount)))
                {
                stats.SetInvalidFormat();
                return stats;
                }

            stats.fieldCount = fieldCount;
            stats.fieldDelimiter = delimiter;
            ++stats.validCount;

            ComputeRemainingStats(stats, fieldsRanges, lineRange, range);

            return stats;
            }
        };


    struct ComputeRemainingSamplesProfile : private ComputeSampleProfileBase
        {
        LineStats operator () (const LineStats& previousStats, const FileRange& range) const
            {
            LineStats stats(previousStats);

            FileRange fieldsRanges[MAX_FIELD_COUNT];
            FileRange lineRange(range.begin, range.begin);

            ComputeRemainingStats(stats, fieldsRanges, lineRange, range);

            return stats;
            }

        };


    return FileAnalysisTool<1024, 8>::Run(file,
                                          ComputeInitialSampleProfile(),
                                          ComputeRemainingSamplesProfile(),
                                          LineStats(true),
                                          status);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Jean-Francois.Cote   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class XYZAsciiSource : public SourceMixinBase<XYZAsciiSource>
    {
public:
    XYZAsciiStream          GetFile                    ()   { return m_inFile; }
    size_t                  GetFieldCount              ()   { return m_fieldCount; }
    WChar                    GetFieldDelimiter          ()   { return m_fieldDelimiter; }

private:
    friend class XYZAsciiSourceFileCreator;

    static const uint32_t       MAX_PREVIEW_LINE_COUNT = 5;
    static const int        READ_BUFFER_SIZE = 1048576; // Default was 4096 (-1). Try optimize using greater values
    static const int        PBACK_BUFFER_SIZE = -1; // Default was 4 (-1). No need to change this as we don't write.
    
    XYZAsciiStream          m_inFile;

    size_t                  m_fieldCount;
    WChar                    m_fieldDelimiter;

    WChar                    m_preview[MAX_PREVIEW_LINE_COUNT][XYZFormat::MAX_FORMATED_LINE_SIZE + 1];
    size_t                  m_previewLineCount;


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   01/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                    XYZAsciiSource                 (const WString&       filePath)
        :                                       
            m_fieldCount(0),
            m_fieldDelimiter(0),
            m_previewLineCount(0)
        {        
        // Null terminate all our preview lines
        for (uint32_t previewLineIdx = 0; previewLineIdx < MAX_PREVIEW_LINE_COUNT; ++previewLineIdx)
            m_preview[previewLineIdx][XYZFormat::MAX_FORMATED_LINE_SIZE] = L'\0';

        m_inFile = _wfopen(filePath.c_str(), L"r");
            
        if (m_inFile == 0)
            throw FileIOException(SMStatus::S_ERROR_COULD_NOT_OPEN);

        // Compute column count and get data preview
        StatusInt status;
        const LineStats xyzProfile(ComputeFileProfile(m_inFile, status));
        if (!xyzProfile || BSISUCCESS != status)
            throw CustomException(L"Invalid xyz ascii format!");

        m_fieldCount = xyzProfile.fieldCount;
        m_fieldDelimiter = xyzProfile.fieldDelimiter;

        if (!CreatePreview())
            throw CustomException(L"Error creating file preview!");
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   01/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                     () override
        {
        fclose(m_inFile);        
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor       _CreateDescriptor          () const override
        {
        // TDORAY: Should we compute an extent??

        DimensionOrg org;
        for(size_t i = 0; i < m_fieldCount; ++i)
            org.push_back(DimensionDef(BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::GetFloat64()));
        ScalableMeshData data = ScalableMeshData::GetNull();

        RefCountedPtr<ILayerDescriptor> desc = ILayerDescriptor::CreateLayerDescriptor(L"",
                             DataTypeFactory().Create(PointTypeFamilyCreator().Create(), org),
                             GCS::GetNull(),
                             0,
                             data);

        // Set all lines for preview
        FieldComposed lines;
        for(size_t previewLineIdx = 0; previewLineIdx < m_previewLineCount; ++previewLineIdx)                            
            {            
            FieldString fieldString(m_preview[previewLineIdx]);  
            lines.push_back(fieldString);            
            }

        MetadataRecord record;
        record.push_back(MetadataEntry(L"CONTENT_PREVIEW", lines));
        desc->SetMetadataRecord(record);

        return ContentDescriptor(L"",
                                 desc, 
                                 true);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   08/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual const WChar*         _GetType                   () const override
        {
        return L"ASCII XYZ";
        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsiclass                                                 Jean-Francois.Cote   04/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool                            CreatePreview              ()
        {
        assert(0 == _ftelli64(m_inFile));
        
        XYZFormat::IsWhiteSpaceOr isWhitespacePred(m_fieldDelimiter);

        bool success = true;
        size_t readLinesCount = 0;
        
        while (readLinesCount < MAX_PREVIEW_LINE_COUNT && feof(m_inFile) == 0)
            {
            WChar* const readLineBegin = m_preview[readLinesCount];
            WChar* const readLineBufferEnd = readLineBegin + XYZFormat::MAX_FORMATED_LINE_SIZE;

            WChar* readLineEnd = readLineBegin;
            const uint32_t fieldCount = XYZFormat::ReadLine(m_inFile,
                                                        readLineEnd, readLineBufferEnd,
                                                        isWhitespacePred, L' ');

            if (0 == fieldCount)
                continue; // Commented out line
            if (fieldCount != m_fieldCount ||
                readLineBufferEnd == readLineEnd)
                {
                success = false;
                break;
                }

            *readLineEnd = L'\0'; // Null terminate our preview line
            ++readLinesCount;
            }

        m_previewLineCount = readLinesCount;

        // Nullify remaining preview lines (which are not used). This is an optional step
        // but one that will greatly enhance debugging.
        for (; readLinesCount < MAX_PREVIEW_LINE_COUNT; ++readLinesCount)
            m_preview[readLinesCount][0] = L'\0';

        // Move the get pointer to the beginning of the file        
        _fseeki64(m_inFile, 0, SEEK_SET);                        
        return success;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Jean-Francois.Cote   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class XYZAsciiSourceFileCreator : public LocalFileSourceCreatorBase
    {
    virtual ExtensionFilter _GetExtensions() const override
        {
        return L"*.xyz;*.txt";
        }

    virtual bool _Supports(const LocalFileSourceRef& sourceRef) const override
        {
        if(!DefaultSupports(sourceRef))
            return false;

        StatusInt status;
        return IsAsciiFile(sourceRef.GetPathCStr(), status) &&
               BSISUCCESS == status;
        }

    virtual SourceBase* _Create(const LocalFileSourceRef&   sourceRef,
                                Log&                        warningLog) const override
        {
        try
            {
            IDTMSourcePtr sourcePtr = sourceRef.GetDtmSource();

            auto_ptr<XYZAsciiSource> sourceP(new XYZAsciiSource(sourceRef.GetPath()));
            return sourceP.release();
            }
        catch (const std::ios_base::failure&)
            {
            // Rethrow as "could not open" (this kind of exception
            // is expected to be thrown by device/streambuf constructors
            // when used directly.
            throw FileIOException(SMStatus::S_ERROR_COULD_NOT_OPEN);
            }

        //return 0;
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Jean-Francois.Cote   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class XYZAsciiPointExtractor : public InputExtractorBase
{
private:
    friend class XYZAsciiPointExtractorCreator;

    // Dimension groups definition
    enum
        {
        DG_XYZ,
        DG_QTY,
        };

    static const size_t         MAX_POINT_QTY   = 500000;
    static const size_t         POINT_TYPE_SIZE = sizeof(DPoint3d);
    
    XYZAsciiSource&             m_source; 

    PODPacketProxy<DPoint3d>    m_pointPacket;
    WChar                       m_lineBuffer[XYZFormat::MAX_FORMATED_LINE_SIZE + 1];

    XYZFormat::IsWhiteSpaceOr   m_isWhitespacePred;

    double                      m_fields[XYZFormat::MAX_FIELD_COUNT];
    const size_t                m_expectedFieldCount;

    const double*               m_xP;
    const double*               m_yP;
    const double*               m_zP;

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   01/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                    XYZAsciiPointExtractor                     (XYZAsciiSource&         file,
                                                                            const DimensionOrg&     dimensionOrg)
        :   m_source(file),            
            m_isWhitespacePred(file.GetFieldDelimiter()),
            m_expectedFieldCount(file.GetFieldCount()),
            m_xP(0),
            m_yP(0),
            m_zP(0)
        {
        // Make sure our read line buffer is null ended
        m_lineBuffer[XYZFormat::MAX_FORMATED_LINE_SIZE] = '\0';

        if (file.GetFieldCount() != dimensionOrg.GetSize())
            throw CustomException(L"Point type does not define appropriate dimension count!");

        for(size_t i = 0; i < dimensionOrg.GetSize(); i++)
            {
            DimensionRole role = dimensionOrg[i].GetRole();
            switch(role)
                {
                case PointDimensionDef::ROLE_XCOORDINATE:
                    m_xP = &m_fields[i];
                    break;
                case PointDimensionDef::ROLE_YCOORDINATE:
                    m_yP = &m_fields[i];
                    break;
                case PointDimensionDef::ROLE_ZCOORDINATE:
                    m_zP = &m_fields[i];
                    break;
                default:
                    break;
                }
            }

        //NEEDS_WORK_SM : Set default dimension to allow ScalableMesh ATP to run.
        for(size_t i = 0; i < dimensionOrg.GetSize(); i++)
            {
            DimensionRole role = dimensionOrg[i].GetRole();

            if (role == PointDimensionDef::ROLE_UNKNOWN)
                {
                if (m_xP == 0)
                    {
                    m_xP = &m_fields[i];
                    }
                else
                if (m_yP == 0)
                    {
                    m_yP = &m_fields[i];
                    }
                else
                if (m_zP == 0)
                    {
                    m_zP = &m_fields[i];
                    }
                }    
            }        

        if (0 == m_xP || 0 == m_yP || 0 == m_zP)
            throw CustomException(L"Point type does not define all required x, y and z dimensions!");
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _Assign(PacketGroup& pi_rRawEntities) override
        {
        m_pointPacket.AssignTo(pi_rRawEntities[DG_XYZ]);
        }



    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                Jean-Francois.Cote   01/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _Read() override
        {
        PODPacketProxy<DPoint3d>::iterator const ptsBegin = m_pointPacket.begin();
        PODPacketProxy<DPoint3d>::iterator ptIt = ptsBegin;

        uint32_t readLineCount = 0;
        bool success = true;

        WChar* const lineBegin = m_lineBuffer;
        WChar* const lineBufferEnd = lineBegin + XYZFormat::MAX_FORMATED_LINE_SIZE;

        double* fieldOutBegin = m_fields;
        double* fieldOutEnd = fieldOutBegin + m_expectedFieldCount;

        for (; readLineCount < MAX_POINT_QTY && feof(m_source.GetFile()) == 0; ++readLineCount)
            {
            WChar* lineEndIt = lineBegin;
            const uint32_t fieldCount = XYZFormat::ReadLine(m_source.GetFile(),
                                                        lineEndIt, lineBufferEnd,
                                                        m_isWhitespacePred, L'\0');

            if (0 == fieldCount)
                continue; // This was a commented out line
            if (m_expectedFieldCount != fieldCount ||
                lineBufferEnd == lineEndIt)
                {
                success = false;
                break;
                }

            *lineEndIt = '\0'; // Null terminate line

            if (!XYZFormat::ReadFields(lineBegin, lineEndIt, fieldOutBegin, fieldOutEnd))
                {
                success = false;
                break;
                }

            ptIt->x = *m_xP;
            ptIt->y = *m_yP;
            ptIt->z = *m_zP;

            ++ptIt;
            } 

        if (!success)
            throw CustomException(L"Incorrect line format"); // TDORAY: Create a new exception taken erroneous line number as parameter

        m_pointPacket.SetEnd(ptIt);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool _Next() override
        {
        // File pointer was already moved forward.
        return feof(m_source.GetFile()) == 0;
        }
};

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Jean-Francois.Cote   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class XYZAsciiPointExtractorCreator : public InputExtractorCreatorMixinBase<XYZAsciiSource>
{
    virtual bool _Supports(const DataType& type) const override
        {
        //if (!type.IsComplete())
        //    return false;

        // Validate number of channels (at least 3 (XYZ)) and TypeSize
        if(type.GetDimensionCount() < 3 || type.GetSize() == 0)
            return false;

        // Validate that at least X Y Z channel can be found in the type
        bool hasXChannel = false;
        bool hasYChannel = false;
        bool hasZChannel = false;                

        DimensionOrg  dimOrg = type.GetOrgGroup()[0];
        for(DimensionOrg::iterator dimDefIt = dimOrg.begin(), dimDefEnd = dimOrg.end();
            dimDefIt != dimDefEnd;
            ++dimDefIt)
            {            
            if(PointDimensionDef::ROLE_XCOORDINATE == dimDefIt->GetRole()) hasXChannel = true;
            else if(PointDimensionDef::ROLE_YCOORDINATE == dimDefIt->GetRole()) hasYChannel = true;
            else if(PointDimensionDef::ROLE_ZCOORDINATE == dimDefIt->GetRole()) hasZChannel = true;
            }
            
        //NEEDS_WORK_SM : If not set use three first channels as default X Y and Z. 
        return true; //(hasXChannel && hasYChannel && hasZChannel);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   05/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _AdaptOutputType (DataType& type) const
        {
        type = PointType3d64fCreator().Create();
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities               (XYZAsciiSource&                       sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source& source,
                                                                                    const ExtractionQuery&                selection) const override
        {
        return RawCapacities (XYZAsciiPointExtractor::POINT_TYPE_SIZE*XYZAsciiPointExtractor::MAX_POINT_QTY);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Raymond.Gauthier   07/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create                            (XYZAsciiSource&                       sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source& source,
                                                                                    const ExtractionQuery&                selection,
                                                                                    const ExtractionConfig&               config,
                                                                                    Log&                                  log) const override
        {
        //assert(selection.GetType().IsComplete());
        return new XYZAsciiPointExtractor(sourceBase, selection.GetType().GetOrgGroup()[0]);
        }
};

} //END UNAMED NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const SourceRegistry::AutoRegister<XYZAsciiSourceFileCreator> s_RegisterXYZAsciiSourceFile;
const ExtractorRegistry::AutoRegister<XYZAsciiPointExtractorCreator> s_RegisterXYZAsciiPointExtractorCreator;

END_BENTLEY_SCALABLEMESH_NAMESPACE
