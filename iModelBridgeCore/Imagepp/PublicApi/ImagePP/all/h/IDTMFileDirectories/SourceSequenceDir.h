//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/SourceSequenceDir.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once


#include <ImagePP/all/h/HTGFFDirectory.h>
#include <ImagePP/all/h/HTGFFPacketIter.h>

#include <ImagePP/all/h/HPUArray.h>
#include <ImagePP/all/h/HTGFFSubDirHelpers.h>
#include <Imagepp/all/h/IDTMFileDirectories/IDTMFeaturePacketHandler.h>

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

    _HDLLg static uint32_t              s_GetVersion                       ();

    virtual                             ~SourceSequenceDir                 ();


    _HDLLg SourceCIter                  SourcesBegin                       () const;
    _HDLLg SourceCIter                  SourcesEnd                         () const;

    _HDLLg SourceIter                   SourcesBegin                       ();
    _HDLLg SourceIter                   SourcesEnd                         ();


    _HDLLg bool                         AddSource                          (TimeType                        pi_lastModified,
                                                                            const SerializedSourcePacket&   pi_rSerializedSource,
                                                                            const ContentConfigPacket&      pi_rContentConfig,
                                                                            const ImportSequencePacket&     pi_rImportSequence,
                                                                            const ImportConfigPacket&       pi_rImportConfig);

    _HDLLg bool                         AddSource                          (TimeType                        pi_lastModified,
                                                                            const SerializedSourcePacket&   pi_rSerializedSource,
                                                                            const ContentConfigPacket&      pi_rContentConfig,
                                                                            const ImportSequencePacket&     pi_rImportSequence);

    // Deprecated.
    _HDLLg bool                         AddSource                          (const SerializedSourcePacket&   pi_rSerializedSource,
                                                                            const WString&                  pi_rGCSWKTOverride,
                                                                            TimeType                        pi_lastModified);

    _HDLLg bool                         ClearAll                           ();

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
   

    _HDLLg SourceID                     GetID                              () const;

    _HDLLg bool                         GetSerializedSource                (SerializedSourcePacket&         po_rSerializedSource) const;
    _HDLLg bool                         GetContentConfig                   (ContentConfigPacket&            po_rContentConfig) const;
    _HDLLg bool                         GetImportSequence                  (ImportSequencePacket&           po_rImportSequence) const;
    _HDLLg bool                         GetImportConfig                    (ImportConfigPacket&             po_rImportConfig) const;

    // Deprecated.
    _HDLLg bool                         GetGCSWKTOverride                  (wstring&                        po_rGCSOverride) const;




    _HDLLg TimeType                     GetLastModified                    () const;

    };


} //End namespace IDTMFile