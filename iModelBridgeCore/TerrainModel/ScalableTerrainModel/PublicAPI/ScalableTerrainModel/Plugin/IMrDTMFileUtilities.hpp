/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Plugin/IMrDTMFileUtilities.hpp $
|    $RCSfile: IMrDTMFileUtilities.hpp,v $
|   $Revision: 1.6 $
|       $Date: 2012/02/22 14:21:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/

#include <io.h>

template <typename FileType>
struct FileAnalysisStreamTrait
    {
    // Should not compile
    };
template <>
struct FileAnalysisStreamTrait<int>
    {
    typedef Int64                       SizeType;
    typedef Int64                       OffsetType;
    typedef Int64                       PositionType;
    typedef UInt                        ReadCountType;
    typedef int                         ReadSizeType;

    static bool                         GetFileSize                                (int fileHandle, SizeType& size)
        {
        assert(0 == _telli64(fileHandle));

        if (0 > _lseeki64(fileHandle, 0, SEEK_END))
            {
            size = 0;
            return false;
            }

        size = _telli64(fileHandle);

        return 0 <= _lseeki64(fileHandle, 0, SEEK_SET);
        }

    static bool                         SeekBegin                                  (int fileHandle)
        {
        return 0 <= _lseeki64(fileHandle, 0, SEEK_SET);
        }

    static bool                         SeekByOffset                               (int fileHandle, OffsetType offset)
        {
        return 0 <= _lseeki64(fileHandle, offset, SEEK_CUR);
        }

    static ReadSizeType                 Read                                       (int fileHandle, WChar buffer[], ReadCountType count)
        {
        static vector<char> s_tempBuffer(count);

        if (s_tempBuffer.capacity() < count) s_tempBuffer.resize(count);

        ReadSizeType readSize = _read(fileHandle, &s_tempBuffer[0], count);

        if (readSize > 0)
            {
            mbstowcs(buffer, &s_tempBuffer[0], readSize);            
            }        
        
        return readSize;
        }

    static bool                         Eof                                        (int fileHandle)
        {
        return 1 == _eof(fileHandle);
        }

    };
template <>
struct FileAnalysisStreamTrait<istream>
    {
    typedef streamoff                   SizeType;
    typedef streamoff                   OffsetType;
    typedef streampos                   PositionType;
    typedef streamsize                  ReadCountType;
    typedef streamsize                  ReadSizeType;

    static bool                         GetFileSize                                (istream& fileStream, SizeType& size)
        {
        assert(0 == fileStream.tellg());
        
        fileStream.clear();
        if (!fileStream.seekg(0, std::ios::end).good())
            {
            size = 0;
            return false;
            }

        const streampos fileEndPos = fileStream.tellg();
        size = fileEndPos;

        return fileStream.seekg(0).good();
        }
    
    static bool                         SeekBegin                                  (istream& fileStream)
        {
        fileStream.clear();
        return fileStream.seekg(0).good();
        }

    static bool                         SeekByOffset                               (istream& fileStream, OffsetType offset)
        {
        fileStream.clear();
        return fileStream.seekg(offset, ios_base::cur).good();
        }

    static ReadSizeType                 Read                                       (istream& fileStream, WChar buffer[], ReadCountType count)
        {
        unique_ptr<char> tmpBuffer(new char[count]);
        fileStream.read(tmpBuffer.get(), count);
        const ReadSizeType readSize = (fileStream.good() || fileStream.eof()) ? fileStream.gcount() : ReadSizeType(-1);

        if (readSize > 0)
            {            
            BeStringUtilities::CurrentLocaleCharToWChar(buffer, tmpBuffer.get(), readSize);
            }

        return readSize;
        }

    static bool                         Eof                                        (const istream& fileStream)
        {
        return fileStream.eof();
        }
    };
template <>
struct FileAnalysisStreamTrait<ifstream> : public FileAnalysisStreamTrait<istream>
    { };
template <>
struct FileAnalysisStreamTrait<iostream> : public FileAnalysisStreamTrait<istream>
    { };


// TDORAY: Specialization for standard C FILE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const UInt MAX_SAMPLE_QTY, const bool RANDOM_SAMPLES> 
template <typename AccumulateFnT, typename StatT>
StatT FileAnalysisTool<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES>::Run       (const WChar*      filePath, 
                                                                                    AccumulateFnT       accumulateFn,
                                                                                    const StatT&        initStats,
                                                                                    FileStatistics&     fileStatistics,
                                                                                    StatusInt&          status)
    {
    return Run(filePath, accumulateFn, accumulateFn, initStats, fileStatistics, status);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const UInt MAX_SAMPLE_QTY, const bool RANDOM_SAMPLES> 
template <typename AccumulateFnT, typename StatT>
StatT FileAnalysisTool<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES>::Run       (const WChar*      filePath, 
                                                                                    AccumulateFnT       accumulateFn,
                                                                                    const StatT&        initStats,
                                                                                    StatusInt&          status)
    {
    return Run(filePath, accumulateFn, accumulateFn, initStats, status);
    }




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const UInt MAX_SAMPLE_QTY, const bool RANDOM_SAMPLES> 
template <typename InitAccumulateFnT, typename AccumulateFnT, typename StatT>
StatT FileAnalysisTool<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES>::Run       (const WChar*      filePath, 
                                                                                    InitAccumulateFnT   initAccumulateFn,
                                                                                    AccumulateFnT       accumulateFn,
                                                                                    const StatT&        initStats,
                                                                                    FileStatistics&     fileStatistics,
                                                                                    StatusInt&          status)
    {
    typedef int StreamType;

    typedef FileAnalysisStrategy<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES, InitAccumulateFnT, AccumulateFnT, StatT, StreamType>
                    Strategy;

    static_assert(MAX_SAMPLE_SIZE <= INT_MAX, "");

    // Get file size
    StreamType      fileHandle;
    struct _stat64  fileStat;

    if (0 != _wsopen_s(&fileHandle, filePath, _O_RDONLY | _O_BINARY, _SH_DENYWR, _S_IREAD) ||
        0 != _fstat64(fileHandle, &fileStat))
        {
        status = BSIERROR;
        return StatT();
        }

    fileStatistics.size = fileStat.st_size;

    WChar fileSampleBuffer[MAX_SAMPLE_SIZE];	

    bool success;
    const StatT fileStats = Strategy::Run(fileHandle, fileStatistics, fileSampleBuffer, initAccumulateFn, accumulateFn, initStats, success);


    _close(fileHandle);

    status = success ? BSISUCCESS : BSIERROR;
    return fileStats;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const UInt MAX_SAMPLE_QTY, const bool RANDOM_SAMPLES> 
template <typename InitAccumulateFnT, typename AccumulateFnT, typename StatT>
StatT FileAnalysisTool<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES>::Run       (const WChar*      filePath, 
                                                                                    InitAccumulateFnT   initAccumulateFn,
                                                                                    AccumulateFnT       accumulateFn,
                                                                                    const StatT&        initStats,
                                                                                    StatusInt&          status)
    {
    FileStatistics fileStats;
    return Run(filePath, initAccumulateFn, accumulateFn, initStats, fileStats, status);
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const UInt MAX_SAMPLE_QTY, const bool RANDOM_SAMPLES> 
template <typename StreamT, typename AccumulateFnT, typename StatT>
StatT FileAnalysisTool<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES>::Run       (StreamT&            fileStream,
                                                                                    AccumulateFnT       accumulateFn,
                                                                                    const StatT&        initStats,
                                                                                    FileStatistics&     fileStatistics,
                                                                                    StatusInt&          status)
    {
    return Run(fileStream, accumulateFn, accumulateFn, initStats, fileStatistics, status);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const UInt MAX_SAMPLE_QTY, const bool RANDOM_SAMPLES> 
template <typename StreamT, typename AccumulateFnT, typename StatT>
StatT FileAnalysisTool<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES>::Run       (StreamT&            fileStream,
                                                                                    AccumulateFnT       accumulateFn,
                                                                                    const StatT&        initStats,
                                                                                    StatusInt&          status)
    {
    return Run(fileStream, accumulateFn, accumulateFn, initStats, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const UInt MAX_SAMPLE_QTY, const bool RANDOM_SAMPLES> 
template <typename StreamT, typename InitAccumulateFnT, typename AccumulateFnT, typename StatT>
StatT FileAnalysisTool<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES>::Run       (StreamT&            fileStream, 
                                                                                    InitAccumulateFnT   initAccumulateFn,
                                                                                    AccumulateFnT       accumulateFn,
                                                                                    const StatT&        initStats,
                                                                                    FileStatistics&     fileStatistics,
                                                                                    StatusInt&          status)
    {
    typedef StreamT StreamType;

    typedef FileAnalysisStrategy<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES, InitAccumulateFnT, AccumulateFnT, StatT, StreamType>
                    Strategy;
    typedef FileAnalysisStreamTrait<StreamType>
                    StreamTraitType;
    typedef typename StreamTraitType::SizeType
                    StreamSize;

    static_assert(MAX_SAMPLE_SIZE <= INT_MAX, "");

    StreamSize fileSize;

    if (!StreamTraitType::GetFileSize(fileStream, fileSize))
        {
        status = BSIERROR;
        fileStatistics.size = 0;
        return StatT();
        }

    fileStatistics.size = fileSize;

    WChar fileSampleBuffer[MAX_SAMPLE_SIZE];	

    bool success;
    const StatT fileStats = Strategy::Run(fileStream, fileStatistics, fileSampleBuffer, initAccumulateFn, accumulateFn, initStats, success);

    success = success && StreamTraitType::SeekBegin(fileStream);

    status = success ? BSISUCCESS : BSIERROR;
    return fileStats;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const UInt MAX_SAMPLE_QTY, const bool RANDOM_SAMPLES> 
template <typename StreamT, typename InitAccumulateFnT, typename AccumulateFnT, typename StatT>
StatT FileAnalysisTool<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES>::Run       (StreamT&            fileStream, 
                                                                                    InitAccumulateFnT   initAccumulateFn,
                                                                                    AccumulateFnT       accumulateFn,
                                                                                    const StatT&        initStats,
                                                                                    StatusInt&          status)
    {
    FileStatistics fileStats;
    return Run(fileStream, initAccumulateFn, accumulateFn, initStats, fileStats, status);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const UInt MAX_SAMPLE_QTY, const bool RANDOM_SAMPLES, typename StatT, typename StreamT>
struct FileAnalysisStrategyBase
    {
protected:
    typedef FileAnalysisStreamTrait<StreamT>
                                        StreamTraitType;
    typedef typename StreamTraitType::OffsetType   
                                        OffsetType;
    typedef typename StreamTraitType::ReadSizeType   
                                        ReadSizeType;

    template <typename AccumulateFnT>
    static StatT                        RunOnSingleSample                          (StreamT&                fileStream, 
                                                                                    const FileStatistics&   fileStatistics,
                                                                                    WChar                    fileSampleBuffer[],
                                                                                    AccumulateFnT           accumulateFn,
                                                                                    const StatT&            stats,
                                                                                    bool&                   success)
        {
        const ReadSizeType readSampleSize = StreamTraitType::Read(fileStream, fileSampleBuffer, MAX_SAMPLE_SIZE);
        if(readSampleSize < 0)
            {
            success = false;
            return StatT();
            }

        success = true;
        return accumulateFn(stats, FileRange(fileSampleBuffer, fileSampleBuffer + readSampleSize));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* NOTE: See specialization for case 1 == MAX_SAMPLE_QTY
* TDORAY: It may become interesting to generate random offsets instead of linearly spaced ones
* @bsiclass                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const UInt MAX_SAMPLE_QTY, const bool RANDOM_SAMPLES, 
          typename InitAccumulateFnT, typename AccumulateFnT, typename StatT, typename StreamT>
struct FileAnalysisStrategy : private FileAnalysisStrategyBase<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES, StatT, StreamT>
    {
private:
    friend struct FileAnalysisTool<MAX_SAMPLE_SIZE, MAX_SAMPLE_QTY, RANDOM_SAMPLES>;

    static StatT                        Run                                    (StreamT&                fileStream, 
                                                                                const FileStatistics&   fileStatistics,
                                                                                WChar                    fileSampleBuffer[],
                                                                                InitAccumulateFnT       initAccumulateFn,
                                                                                AccumulateFnT           accumulateFn,
                                                                                const StatT&            stats,
                                                                                bool&                   success)
        {
        static_assert(1 < MAX_SAMPLE_QTY, "");

        StatT cummuledStats = RunOnSingleSample(fileStream, fileStatistics, fileSampleBuffer, initAccumulateFn, stats, success);

        if (!cummuledStats)
            return cummuledStats;
        assert(success);
    
        const UInt remainingSampleQty = max<UInt>(1, static_cast<UInt>(min<UInt64>(MAX_SAMPLE_QTY, fileStatistics.size / MAX_SAMPLE_SIZE))) - 1;
        if (0 == remainingSampleQty) 
            return cummuledStats; // First sample was representative enough
    
        assert(fileStatistics.size <= static_cast<UInt64>((numeric_limits<OffsetType>::max)()));
        const OffsetType sampleOffsetDelta = static_cast<OffsetType>(fileStatistics.size / (remainingSampleQty + 1));

        static_assert(!RANDOM_SAMPLES, "Implement the shuffled offsets generation strategy");
        assert(MAX_SAMPLE_SIZE < sampleOffsetDelta);

        OffsetType samplesOffsets[MAX_SAMPLE_QTY - 1];
        std::fill_n(samplesOffsets, remainingSampleQty, sampleOffsetDelta - MAX_SAMPLE_SIZE);

        assert(samplesOffsets[remainingSampleQty - 1] < static_cast<OffsetType>(fileStatistics.size));

        // We assume ASCII file until proven otherwise
        UInt sampleIdx = 0;
        for (; (sampleIdx < remainingSampleQty) && !StreamTraitType::Eof(fileStream); ++sampleIdx)
            {
            if (!StreamTraitType::SeekByOffset(fileStream, samplesOffsets[sampleIdx]))
                {
                success = false;
                break;
                }

            cummuledStats = RunOnSingleSample(fileStream, fileStatistics, fileSampleBuffer, accumulateFn, cummuledStats, success);

            if (!cummuledStats)
                break;
            };
    
        return success ? cummuledStats : StatT();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const UInt MAX_SAMPLE_SIZE, const bool RANDOM_SAMPLES, 
          typename InitAccumulateFnT, typename AccumulateFnT, typename StatT, typename StreamT>
struct FileAnalysisStrategy<MAX_SAMPLE_SIZE, 1, RANDOM_SAMPLES, InitAccumulateFnT, AccumulateFnT, StatT, StreamT> 
    : private FileAnalysisStrategyBase<MAX_SAMPLE_SIZE, 1, RANDOM_SAMPLES, StatT, StreamT>
    {
private:
    friend struct FileAnalysisTool<MAX_SAMPLE_SIZE, 1, RANDOM_SAMPLES>;
    
    static StatT                        Run                                        (StreamT&                fileStream, 
                                                                                    const FileStatistics&   fileStatistics,
                                                                                    WChar                    fileSampleBuffer[],
                                                                                    InitAccumulateFnT       initAccumulateFn,
                                                                                    AccumulateFnT           accumulateFn,
                                                                                    const StatT&            stats,
                                                                                    bool&                   success)
        {
        return RunOnSingleSample(fileStream, fileStatistics, fileSampleBuffer, initAccumulateFn, stats, success);
        }
    };
