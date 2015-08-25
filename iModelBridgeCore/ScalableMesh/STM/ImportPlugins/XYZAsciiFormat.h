/*----------------------------------------------------------------------+
|
|   $Source: STM/ImportPlugins/XYZAsciiFormat.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once
    
#include <algorithm>

#include <ScalableMesh\Plugin\IScalableMeshFileUtilities.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct XYZFormat
    {
    static const UInt           MAX_FIELD_COUNT = 20;
    static const UInt           MINIMUM_FIELD_COUNT = 3;
    static const WChar           INVALID_DELIMITER = 0;

    static const WChar           NEW_LINE = '\n';
    static const WChar           COMMENT = '"';

    static const size_t         MAX_FORMATED_LINE_SIZE = 250;

    struct                      IsWhiteSpace;
    struct                      IsWhiteSpaceOr;
    struct                      IsNumeric;
    struct                      IsNewLineOrComment;

    //struct                      CType;
    typedef Bentley::ScalableMesh::Plugin::FileRange
                                FileRange;

private:
    static WChar                 FindDelimiterIn                    (FileRange::const_iterator   delimZoneBegin,
                                                                    FileRange::const_iterator   delimZoneEnd);

    static bool                 IsWS                               (WChar                        c);
    static bool                 IsNum                              (WChar                        c);

public:
    static UInt                 SplitLine                          (FileRange                   fieldsRanges[],
                                                                    const FileRange             lineRange);

    static bool                 IsCommentLine                      (const FileRange&            lineRange);
    static bool                 IsCommentOrEmptyLine               (const FileRange&            lineRange);

    static bool                 GetNextLineRange                   (FileRange&                  nextLineRange,
                                                                    FileRange::const_iterator   rangeBegin,
                                                                    FileRange::const_iterator   rangeEnd);

    static bool                 GetFirstLineRange                  (FileRange&                  firstLineRange,
                                                                    FileRange::const_iterator   rangeBegin,
                                                                    FileRange::const_iterator   rangeEnd);

    static WChar                 GetDelimiterFor                    (FileRange::const_iterator   delimZoneBegin,
                                                                    FileRange::const_iterator   delimZoneEnd);
    static WChar                 GetDelimiterFor                    (const FileRange             fields[],
                                                                    UInt                        fieldsCount);

    template <typename OutIt, typename IsWSPred>
    static UInt                 ReadLine                           (FILE*       file, 
                                                                    OutIt&      outBeginIt,
                                                                    OutIt       outEndIt,
                                                                    IsWSPred    isWSPred,
                                                                    WChar       newDelimiter);


    template <typename OutIt>
    static bool                 ReadFields                         (const WChar*                 lineBegin,
                                                                    const WChar*                 lineEnd,
                                                                    OutIt                       fieldOutBeginIt,
                                                                    OutIt                       fieldOutEndIt);

    };


#include "XYZAsciiFormat.hpp"

END_BENTLEY_SCALABLEMESH_NAMESPACE
