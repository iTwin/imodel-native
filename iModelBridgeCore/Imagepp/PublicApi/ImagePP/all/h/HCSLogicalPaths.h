//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSLogicalPaths.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSLogicalPaths
//-----------------------------------------------------------------------------
// HCSLogicalPaths.h : header file
//-----------------------------------------------------------------------------

#pragma once

#include "HCSLogicalPath.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements an intelligent collection of logical paths. A set of
    logical paths is built, searched into based on physical path name of
    logical path name.
    -----------------------------------------------------------------------------
*/
template<class Properties> class HCSLogicalPaths
    {
public:
    //--------------------------------------
    // types
    //--------------------------------------

    typedef vector<HCSLogicalPath<Properties> >
    PathList;

    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSLogicalPaths();
    ~HCSLogicalPaths();
    HCSLogicalPaths(const HCSLogicalPaths& pi_rObj);
    HCSLogicalPaths&    operator= (const HCSLogicalPaths& pi_rObj);


    //--------------------------------------
    // Logical path methods
    //--------------------------------------

    // Returns a reference to the logical path lock to permit
    // usage in a monitor. (Safer)
    HFCExclusiveKey&    GetLogicalPathsKey() const;

    // Set the logical path with a path list
    void                SetLogicalPaths(const PathList& pi_pList);

    // Finds a physical path for a given logical path
    WString              FindPhysicalFor(const WString& pi_rLogical,
                                         Properties*     po_pProperties = 0) const;

    // Gives the logical path count and values
    uint32_t            CountLogicalPaths() const;
    void                GetLogicalPath(uint32_t     pi_pIndex,
                                       WString*     po_pLogical,
                                       WString*     po_pPhysical,
                                       Properties*  po_pProperties = 0) const;

    // Remove a logical path from the list
    void                RemoveLogicalPath(uint32_t pi_Index);
    void                RemoveLogicalPath(const WString& pi_rLogical);

    // Clear the path list.
    void                ClearLogicalPaths();

    // Add a new logical path
    void                AddLogicalPath(const WString&       pi_pLogical,
                                       const WString&       pi_pPhysical,
                                       const Properties&    pi_rProperties = Properties());


    //-----------------------------------------------------------------------------
    // Function used to map a logical name to a physical name.  From the file name,
    // the logical and physical parameters are created.
    //
    // The result string is given by the caller to avoid confusion between threads.
    //-----------------------------------------------------------------------------
    bool               GetPhysicalPath(const WString&  pi_rLogical,
                                        WString*        po_pPhysical,
                                        WString*        po_pLogicalPath = 0,
                                        Properties*     po_pProperties = 0) const;


    //-----------------------------------------------------------------------------
    //  Builds a physical file name based on a file name with a logical path
    //
    //-----------------------------------------------------------------------------
    bool               GetPhysicalFileName(const WString&  pi_rImageName,
                                            WString*        po_pPhysical,
                                            Properties*     po_pProperties = 0) const;


    //-----------------------------------------------------------------------------
    //  Splits a file name in a directory and file name
    //
    //-----------------------------------------------------------------------------
    void                SplitPath(const WString&    pi_rPath,
                                  WString*          po_pDirectory,
                                  WString*          po_pFileName) const;


protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // To make the class thread-safe
    mutable HFCExclusiveKey
    m_Key;

    // Logical path
    PathList            m_LogicalPathList;
    };

#include "HCSLogicalPaths.hpp"

