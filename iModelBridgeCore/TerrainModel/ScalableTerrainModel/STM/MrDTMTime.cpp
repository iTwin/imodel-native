/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMTime.cpp $
|    $RCSfile: MrDTMTime.cpp,v $
|   $Revision: 1.4 $
|       $Date: 2012/01/19 20:04:54 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include "MrDTMTime.h"

using namespace std;

BEGIN_BENTLEY_MRDTM_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time Time::CreateActual ()
    {
    TimeType cTime;
    time( &cTime );

    return Time(cTime);
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time Time::CreateSmallestPossible ()
    {
    TimeType cTime = TimeType();
    return Time(cTime);
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time Time::CreateGreathestPossible ()
    {
    TimeType cTime = (numeric_limits<TimeType>::max)();
    return Time(cTime);
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time::Time ()
    :   m_cTime(TimeType())
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time::Time (TimeType time)
    :   m_cTime(time)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time::Time (const Time& time)
    :   m_cTime(time.m_cTime)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time& Time::operator= (const Time& time)
    {
    m_cTime = time.m_cTime;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time::~Time ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool Time::IsSmallestPossible () const
    {
    return TimeType() == m_cTime;
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool Time::IsGreathestPossible () const
    {
    return (numeric_limits<TimeType>::max)() == m_cTime;
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool Time::operator< (const Time& time) const
    {
    return m_cTime < time.m_cTime;
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time::TimeType GetCTimeFor (const Time& time)
    {
    return time.m_cTime;
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time CreateTimeFrom (Time::TimeType time)
    {
    return Time(time);
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time CreateUnknownModificationTime ()
    {
    return Time::CreateActual();
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time CreateUndefinedModificationTime ()
    {
        return Time::CreateSmallestPossible();
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsUndefinedModificationTime (const Time& time)
    {
    return time.IsSmallestPossible();
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Time GetFileLastModificationTimeFor(const WChar*  filePath)
    {
    struct _stat64 fileStats;
    int status = _wstat64(filePath, &fileStats);
    
    if (0 != status)
        return CreateUnknownModificationTime();
    
    return CreateTimeFrom(fileStats.st_mtime);
    }

END_BENTLEY_MRDTM_NAMESPACE