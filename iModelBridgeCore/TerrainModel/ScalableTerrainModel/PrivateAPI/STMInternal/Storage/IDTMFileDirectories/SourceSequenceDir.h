//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/SourceSequenceDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once


#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HTGFFPacketIter.h>

#include <STMInternal/Storage/HPUArray.h>
#include <STMInternal/Storage/HTGFFSubDirHelpers.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeaturePacketHandler.h>

namespace IDTMFile {


enum SourceType
    {
    SOURCE_TYPE_QTY,
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SourceSequenceDir : public HTGFF::Directory
    {
public:
    typedef time_t                      TimeType;
private:
    friend class                        HTGFF::Directory;
    template <typename T>
    friend struct                       AttributeHelper;

    class                               SourceEditor;

    typedef HPU::Array<TimeType>        LastModifiedArray;
    LastModifiedArray                   m_lastModifiedTimeStamps;

    typedef HTGFF::PacketIndexSubDir    SerializedSourcesDir;
    SerializedSourcesDir*               m_pSerializedSources;

    typedef HTGFF::PacketIndexSubDir    ContentConfigsDir;
    ContentConfigsDir*                  m_pContentConfigs;

    typedef HTGFF::PacketIndexSubDir    ImportSequencesDir;
    ImportSequencesDir*                 m_pImportSequences;

    typedef HTGFF::PacketIndexSubDir    ImportConfigsDir;
    ImportConfigsDir*                   m_pImportConfigs;

    virtual bool                        _Create                            (const CreateConfig&             pi_rCreateConfig,
                                                                            const UserOptions*              pi_pUserOptions) override;
    virtual bool                        _Load                              (const UserOptions*              pi_pUserOptions) override;
    virtual bool                        _Save                              () override;

public:
    explicit                            SourceSequenceDir                  ();  // Should be private, Android problem.

    typedef PacketID                    SourceID;

    typedef Packet                      SerializedSourcePacket;
    typedef Packet                      ContentConfigPacket;
    typedef Packet                      ImportSequencePacket;
    typedef Packet                      ImportConfigPacket;

    typedef const SourceEditor          CSource;
    typedef SourceEditor                Source;

    typedef HTGFF::PacketIter<CSource>  SourceCIter;
    typedef HTGFF::PacketIter<Source>      
                                        SourceIter;

     static uint32_t              s_GetVersion                       ();

    virtual                             ~SourceSequenceDir                 ();


     SourceCIter                  SourcesBegin                       () const;
     SourceCIter                  SourcesEnd                         () const;

     SourceIter                   SourcesBegin                       ();
     SourceIter                   SourcesEnd                         ();


     bool                         AddSource                          (TimeType                        pi_lastModified,
                                                                            const SerializedSourcePacket&   pi_rSerializedSource,
                                                                            const ContentConfigPacket&      pi_rContentConfig,
                                                                            const ImportSequencePacket&     pi_rImportSequence,
                                                                            const ImportConfigPacket&       pi_rImportConfig);

     bool                         AddSource                          (TimeType                        pi_lastModified,
                                                                            const SerializedSourcePacket&   pi_rSerializedSource,
                                                                            const ContentConfigPacket&      pi_rContentConfig,
                                                                            const ImportSequencePacket&     pi_rImportSequence);

    // Deprecated.
     bool                         AddSource                          (const SerializedSourcePacket&   pi_rSerializedSource,
                                                                            const WString&                  pi_rGCSWKTOverride,
                                                                            TimeType                        pi_lastModified);

     bool                         ClearAll                           ();

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SourceSequenceDir::SourceEditor : public HTGFF::PacketEditorBase<SourceSequenceDir>
    {
    friend class                        SourceSequenceDir;
public:
    typedef Packet                      SerializedSourcePacket;
    typedef Packet                      ContentConfigPacket;
    typedef Packet                      ImportSequencePacket;
    typedef Packet                      ImportConfigPacket;

    typedef PacketID                    SourceID;
   

     SourceID                     GetID                              () const;

     bool                         GetSerializedSource                (SerializedSourcePacket&         po_rSerializedSource) const;
     bool                         GetContentConfig                   (ContentConfigPacket&            po_rContentConfig) const;
     bool                         GetImportSequence                  (ImportSequencePacket&           po_rImportSequence) const;
     bool                         GetImportConfig                    (ImportConfigPacket&             po_rImportConfig) const;

    // Deprecated.
     bool                         GetGCSWKTOverride                  (wstring&                        po_rGCSOverride) const;




     TimeType                     GetLastModified                    () const;

    };


} //End namespace IDTMFile