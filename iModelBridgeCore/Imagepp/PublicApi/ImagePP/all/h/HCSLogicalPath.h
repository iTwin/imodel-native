//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSLogicalPath.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSLogicalPath
//-----------------------------------------------------------------------------
// HCSLogicalPath.h : header file
//-----------------------------------------------------------------------------

#pragma once

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a logical paths. Logical paths are used by servers to
    represent a physical local path as a internet wide logicaly named path.

    The template class binds the physical path to the logical path name as well
    as properties that are of the template argument type of any kind.
    -----------------------------------------------------------------------------
*/
template<class Properties>
class HCSLogicalPath
    {
public:
    //--------------------------------------
    // Constructor / Destructor
    //--------------------------------------

    HCSLogicalPath();
    HCSLogicalPath(const WString&    pi_rLogical,
                   const WString&    pi_rPhysical,
                   const Properties& pi_rProps = Properties());
    HCSLogicalPath(const HCSLogicalPath& pi_rObj);
    ~HCSLogicalPath();


    //--------------------------------------
    // Operators
    //--------------------------------------

    // Copy operator
    HCSLogicalPath& operator=(const HCSLogicalPath& pi_rObj);

    // Operators (defined for DEC STL)
    bool           operator< (const HCSLogicalPath& pi_rObj) const;
    bool           operator> (const HCSLogicalPath& pi_rObj) const;
    bool           operator==(const HCSLogicalPath& pi_rObj) const;
    bool           operator!=(const HCSLogicalPath& pi_rObj) const;


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Returns the name of the path
    const WString&  GetName() const;

    // Returns the mapped physical path
    const WString&  GetPhysicalPath() const;

    // Return the properties of the logical path
    const Properties&
    GetProperties() const;

    //-----------------------------------------------------------------------------
    // This function formats a logical path to the required format, which is:
    //
    //  format      =    "/" | logicalpath
    //  logicalpath =    *["/" directory ] directory
    //
    //  e.g:    "/", "/Quebec/Satellite/Resolution"
    //-----------------------------------------------------------------------------
    static void     FormatLogicalPath(WString* po_pLogical);


    //-----------------------------------------------------------------------------
    // This function formats a Physical path to the required format, which is:
    //
    //  format       =  scheme:// physicalpath
    //  physicalpath =  [(\\server\share) | (host ":" "\")] *["/" directory ] directory
    //
    //  e.g:    file://\\alpha\cert, file://d:\images
    //-----------------------------------------------------------------------------
    static void     FormatPhysicalPath(WString* po_pPhysical);



private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    Properties      m_Properties;
    WString         m_Logical;
    WString         m_Physical;
    };

#include "HCSLogicalPath.hpp"

