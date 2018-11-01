//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMFilteringDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/IDTMTypes.h>
#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HTGFFDirectoryHandler.h>
#include <STMInternal/Storage/HTGFFSubDirHelpers.h>

#include <STMInternal/Storage/HPUArray.h>

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description  Listing of all supported IDTM filter types.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
enum FilterType
    {
    FILTER_TYPE_DUMB,
    FILTER_TYPE_TILE,
    FILTER_TYPE_TIN,
    FILTER_TYPE_QTY,
    };


/*---------------------------------------------------------------------------------**//**
* @description  Directory that stores information about the filtering process. Filter
*               type used and whether a specific tile has been filtered can be queried
*               via this interface.
*
*
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class FilteringDir : public HTGFF::Directory
    {
public:
    struct Options : public UserOptions
        {
         virtual          ~Options                           () = 0;

         FilterType       GetType                            () const;

    protected:
         explicit         Options                            (FilterType              pi_Type);
    private:
        friend class            FilteringDir;

        virtual bool            _SaveTo                            (FilteringDir&               pi_rDir) const = 0;

        FilterType              m_Type;
        };

    // TDORAY: Add missing options classes here


     static uint32_t      s_GetVersion                       ();

    virtual                     ~FilteringDir                      ();

    /*---------------------------------------------------------------------------------**//**
    * Filtering accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
     FilterType           GetType                            () const;

    // TDORAY: Is it really necessary to store this information?
     bool                 IsFiltered                         (TileID                  pi_ID) const;
     void                 SetFiltered                        (TileID                  pi_ID,
                                                                    bool                        pi_Filtered);

    //TDORAY: Template method on filter type here that return specific parameters class for the specified filter type

    explicit                    FilteringDir                       ();      // Should be private, Android problem.

private:
    friend class                HTGFF::Directory;

    // TDORAY: This is overkill friendship. See IDTMDirectory note...
    friend class                DumbFilteringHandler;

    bool                        SetType                            (FilterType              pi_Type);

    virtual bool                _Create                            (const CreateConfig&         pi_rCreateConfig,
                                                                    const UserOptions*          pi_pUserOptions) override;
    virtual bool                _Load                              (const UserOptions*          pi_pUserOptions) override;
    virtual bool                _Save                              () override;

    typedef HTGFF::AttributeSubDir<Byte>
                                NodesFilteringStatusDir;

    NodesFilteringStatusDir*    m_pNodesFilteringStatusDir;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Handler that wrap on a spatial index directory to offer quad tree
*               specific accessors.
*
* @see SpatialIndexDir
* @see SpatialIndexHandler
*
* @bsiclass                                                  Raymond.Gauthier   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class DumbFilteringHandler : public HTGFF::SingleDirHandler<DumbFilteringHandler, FilteringDir>
    {
public:
    struct Options : public FilteringDir::Options
        {
         explicit         Options                            ();
    private:
        virtual bool            _SaveTo                            (FilteringDir&               pi_rDir) const override;
        };

     virtual              ~DumbFilteringHandler              ();

     static bool          IsCompatibleWith                   (const FilteringDir&         pi_rDir);

     static Ptr           CreateFrom                         (FilteringDir*               pi_pDir);

private:
    friend                      super_class;

    explicit                    DumbFilteringHandler               (FilteringDir*               pi_rpDir);


    virtual bool                _Save                              () override;
    virtual bool                _Load                              () override;
    bool                        Create                             (const Options&              pi_rOptions);

    FilteringDir*               m_pDir;
    };








} //End namespace IDTMFile