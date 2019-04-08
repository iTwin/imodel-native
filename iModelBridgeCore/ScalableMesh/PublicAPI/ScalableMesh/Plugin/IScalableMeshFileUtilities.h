/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Plugin/IScalableMeshFileUtilities.h $
|    $RCSfile: IScalableMeshFileUtilities.h,v $
|   $Revision: 1.7 $
|       $Date: 2012/01/17 16:06:28 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Foundations/Algorithm.h>
#include <fcntl.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
namespace Plugin {

/*---------------------------------------------------------------------------------**//**
* @description  Simple file statistic class used to return most important file 
*               statistics.
* @see IsKindOfFileTool
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileStatistics
    {
    uint64_t                      size;

    explicit                    FileStatistics                         () : size(0) {}    
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileRange
    {
    typedef const WChar*        const_iterator;

    const_iterator              begin;
    const_iterator              end;

    explicit                    FileRange                              ()
        :   begin(0),
            end(0) {}

    explicit                    FileRange                              (const_iterator              rangeBegin,
                                                                        const_iterator              rangeEnd)
        :   begin(rangeBegin),
            end(rangeEnd) {}
    };


/*---------------------------------------------------------------------------------**//**
* @description  Generic algorithm for evaluating statistical information on a file by
*               accumulation over distributed samples taken from the specified file.
*
*               File could be specified via a file path, an already open std stream,
*               an OS file handle (int) or a standard C file handle (FILE). In the 
*               case of the file path,the algorithm will take care of the file's life 
*               cycle. Otherwise, user is responsible for provided file's life cycle. 
*               Moreover, file provided by the user is expected to be opened, in binary 
*               mode and have its read pointer at file begin. Algorithm will be responsible 
*               for reseting read pointer to file begin once it returns control to caller.
* 
*               For the algorithm to work, user must also provide an accumulation function 
*               (AccumulateFnT) of the following form: 
*
*               struct AccumulateSomeStatistics : public std::binary_function<SomeStatsType, 
*                                                                             FileRange, 
*                                                                             SomeStatsType>
*                   {
*                   SomeStatsType operator ()  (const SomeStatsType&    cummuledStats, 
*                                               const FileRange&        sampleRange) const
*                       {
*                       // do something computations here over file range...
*
*                       // combine "stats" with newly computed stats in a new SomeStatsType
*                       // object
*                       return SomeStatsType(...);
*                       }
*                   };
*
*               NOTE: The binary_function base is optional but usually recommended when 
*                     designing standard functors.
*
*               1st argument is the cumulated stats for previous samples. 2th argument is
*               the range of the sample user's predicate is expected to test. User has to
*               return a StatT object that is a combination of the 1st and 2th argument. 
*               
&               StatT objects must be convertible to bool. Conversion operator should return  
*               "false" for early return and "true" if algorithm is to continue with next
*               sample. StatT objects must be default constructible and default constructed
*               StatT objects must convert to "false". Initial StatT object passed to this
*               algorithm must convert to "true". Most simple StatT object can be of bool type.
*
*               e.g.:
*               class SomeStatsType
*                   {
*                   bool        isKindOf;
*               public:    
*                   explicit    SomeStatsType      (bool isKindOf = true) : isKindOf(isKindOf) {}
*
*                               operator bool      () const { return isKindOf; }
*                   };
*
*               which could also be expressed in its simplest form:
*               typedef bool SomeStatsType;
*                          
*
*               User's AccumulateFnT will be called MAX_SAMPLE_QTY time with next sample but
*               will return early if end-of-file is reached of if StatT object converts to
*               false.
*
*               User can also optionally provide a InitAccumulateFnT functor of the same form as 
*               AccumulateFnT in order to differentiate first sample computations from remaining
*               sample ones. This functor will be called only for the first sample (which
*               is always taken at file begin). Algorithm will then call AccumulateFnT for remaining
*               samples if required (so that it will be called for a maximum of [MAX_SAMPLE_QTY - 1] 
*               times).
*
*               MAX_SAMPLE_SIZE: the size in bytes each sample is to be made of.
*               MAX_SAMPLE_QTY: how many sample of MAX_SAMPLE_SIZE are to be taken throughout 
*                               the file. 
*               RANDOM_SAMPLES: for 1 == MAX_SAMPLE_QTY, this parameter is unnecessary as 
*                               first sample is always taken at file begin. When true, 
*                               will take remaining samples at random positions
*                               over the whole file. When false, samples are taken at 
*                               linearly distributed positions over the whole file. In both 
*                               cases, samples are not allowed to overlap each other.
*                               
*
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <const uint32_t MAX_SAMPLE_SIZE, const uint32_t MAX_SAMPLE_QTY = 1, const bool RANDOM_SAMPLES = false>
struct FileAnalysisTool
    {
public:
    template <typename AccumulateFnT, typename StatT>
    static StatT                    Run                                    (const WChar*      filePath, 
                                                                            AccumulateFnT       accumulateFn,
                                                                            const StatT&        initStats,
                                                                            FileStatistics&     fileStatistics,
                                                                            StatusInt&          status);

    template <typename AccumulateFnT, typename StatT>
    static StatT                    Run                                    (const WChar*      filePath, 
                                                                            AccumulateFnT       accumulateFn,
                                                                            const StatT&        initStats,
                                                                            StatusInt&          status);

    template <typename InitAccumulateFnT, typename AccumulateFnT, typename StatT>
    static StatT                    Run                                    (const WChar*      filePath, 
                                                                            InitAccumulateFnT   initAccumulateFn,
                                                                            AccumulateFnT       accumulateFn,
                                                                            const StatT&        initStats,
                                                                            FileStatistics&     fileStatistics,
                                                                            StatusInt&          status);

    template <typename InitAccumulateFnT, typename AccumulateFnT, typename StatT>
    static StatT                    Run                                    (const WChar*      filePath, 
                                                                            InitAccumulateFnT   initAccumulateFn,
                                                                            AccumulateFnT       accumulateFn,
                                                                            const StatT&        initStats,
                                                                            StatusInt&          status);


    template <typename StreamT, typename AccumulateFnT, typename StatT>
    static StatT                    Run                                    (StreamT&            fileStream, 
                                                                            AccumulateFnT       accumulateFn,
                                                                            const StatT&        initStats,
                                                                            FileStatistics&     fileStatistics,
                                                                            StatusInt&          status);

    template <typename StreamT, typename AccumulateFnT, typename StatT>
    static StatT                    Run                                    (StreamT&            fileStream, 
                                                                            AccumulateFnT       accumulateFn,
                                                                            const StatT&        initStats,
                                                                            StatusInt&          status);


    template <typename StreamT, typename InitAccumulateFnT, typename AccumulateFnT, typename StatT>
    static StatT                    Run                                    (StreamT&            fileStream, 
                                                                            InitAccumulateFnT   initAccumulateFn,
                                                                            AccumulateFnT       accumulateFn,
                                                                            const StatT&        initStats,
                                                                            FileStatistics&     fileStatistics,
                                                                            StatusInt&          status);

    template <typename StreamT, typename InitAccumulateFnT, typename AccumulateFnT, typename StatT>
    static StatT                    Run                                    (StreamT&            fileStream, 
                                                                            InitAccumulateFnT   initAccumulateFn,
                                                                            AccumulateFnT       accumulateFn,
                                                                            const StatT&        initStats,
                                                                            StatusInt&          status);


    // TDORAY: Variants for OS (int) and standard C (FILE) handles. Can be generalized using an additional 
    // template argument for stream type.
    };


#include <ScalableMesh/Plugin/IScalableMeshFileUtilities.hpp>

} // END namespace Plugin
END_BENTLEY_SCALABLEMESH_NAMESPACE
