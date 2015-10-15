//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderTileHandler.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


template <typename HeaderT>
struct FeatureHeaderTileHandler<HeaderT>::Impl
    {
private:
    friend class            FeatureHeaderTileHandler<HeaderT>;

    virtual bool            _IsPointOnly                       () const = 0;

    virtual size_t          _GetTileMaxHeaderCount             () const = 0;
    virtual uint64_t       _CountHeaders                      () const = 0;
    virtual size_t          _CountHeaders                      (TileID                      pi_ID) const = 0;

    virtual bool            _GetHeaders                        (TileID                      pi_ID,
                                                                HeaderArray&                pi_rTileHeaders) = 0;

    virtual bool            _SetHeaders                        (TileID                      pi_ID,
                                                                const HeaderArray&          pi_rTileHeaders) = 0;

    virtual bool            _AddHeaders                        (TileID&                     po_rID,
                                                                const HeaderArray&          pi_rTileHeaders) = 0;

    virtual bool            _RemoveHeaders                     (TileID                      pi_ID) = 0;
    };


template <typename HeaderT>
struct FeatureHeaderTileHandler<HeaderT>::LinearFeatureImpl : public Impl
    {
    explicit                LinearFeatureImpl                  (FeatureHeaderDir*           pi_rpDir)
        :   m_pDir(pi_rpDir)
        {

        }
private:
    virtual bool            _IsPointOnly                       () const override
    {
        return false;
    }

    virtual size_t          _GetTileMaxHeaderCount             () const override
    {
        return m_pDir->GetTileMaxHeaderCount();
    }

    virtual uint64_t       _CountHeaders                      () const override
    {
        return m_pDir->CountHeaders();
    }

    virtual size_t          _CountHeaders                      (TileID                      pi_ID) const override
    {
        return m_pDir->CountHeaders(pi_ID);
    }

    virtual bool            _GetHeaders                        (TileID                      pi_ID,
                                                                HeaderArray&                po_rTileHeaders) override
        {
        return m_pDir->GetHeaders(pi_ID, po_rTileHeaders.EditPacket());
        }

    virtual bool            _SetHeaders                        (TileID                      pi_ID,
                                                                const HeaderArray&          pi_rTileHeaders) override
        {
        return m_pDir->SetHeaders(pi_ID, pi_rTileHeaders.GetPacket());
        }

    virtual bool            _AddHeaders                        (TileID&                     po_rID,
                                                                const HeaderArray&          pi_rTileHeaders) override
        {
        return m_pDir->AddHeaders(po_rID, pi_rTileHeaders.GetPacket());
        }

    virtual bool            _RemoveHeaders                     (TileID                      pi_ID) override
        {
        return m_pDir->RemoveHeaders(pi_ID);
        }


    FeatureHeaderDir*       m_pDir;
    };

template <typename HeaderT>
struct FeatureHeaderTileHandler<HeaderT>::PointOnlyImpl : public Impl
    {
    explicit                PointOnlyImpl                      (FeatureType             pi_FeatureType,
                                                                PointDir*               pi_pPointDir)
        :   m_pPointDir(pi_pPointDir),
            m_FeatureType(pi_FeatureType)
        {
        HPRECONDITION(0 != m_pPointDir);
        }

private:
    virtual bool            _IsPointOnly                       () const override
    {
        return true;
    }

    virtual size_t          _GetTileMaxHeaderCount             () const override
    {
        return 1;
    }

    virtual uint64_t       _CountHeaders                      () const override
    {
        return m_pPointDir->CountTiles();
    }

    virtual size_t          _CountHeaders                      (TileID                  pi_ID) const override
    {
        HPRECONDITION(pi_ID != GetNullTileID());
        return 1;
    }

    virtual bool            _GetHeaders                        (TileID                  pi_ID,
                                                                HeaderArray&            po_rTileHeaders) override
        {
        HPRECONDITION(pi_ID != GetNullTileID());


        if (po_rTileHeaders.IsReadOnly())
            return false;

        if (!po_rTileHeaders.IsBufferOwner() && po_rTileHeaders.GetCapacity() < 1)
            return false;

        if (1 != po_rTileHeaders.GetSize())
            po_rTileHeaders.Resize(1);

        // TDORAY: maybe store group id in a tag?
        po_rTileHeaders.Edit()[0].groupId = IDTrait<GroupID>::GetNullID();
        po_rTileHeaders.Edit()[0].type = m_FeatureType;
        po_rTileHeaders.Edit()[0].offset = 0;
        po_rTileHeaders.Edit()[0].size = static_cast<uint32_t>(m_pPointDir->CountPoints(pi_ID));

        return true;
        }

    virtual bool            _SetHeaders                        (TileID                  pi_ID,
                                                                const HeaderArray&      pi_rTileHeaders) override
        {
        HPRECONDITION(pi_ID != GetNullTileID());
        HPRECONDITION(1 == pi_rTileHeaders.GetSize()); // Point feature tiles are expected to be composed of only 1 header
        return true;
        }

    virtual bool            _AddHeaders                        (TileID&                 po_rID,
                                                                const HeaderArray&      pi_rTileHeaders) override
        {
        HPRECONDITION(1 == pi_rTileHeaders.GetSize()); // Point feature tiles are expected to be composed of only 1 header
        return true;
        }

    virtual bool            _RemoveHeaders                     (TileID                  pi_ID) override
        {
        HPRECONDITION(pi_ID != GetNullTileID());
        return true;
        }

    FeatureType             m_FeatureType;
    const PointDir*         m_pPointDir;
    };






template <typename HeaderT>
bool FeatureHeaderTileHandler<HeaderT>::IsCompatibleWith (const FeatureDir& pi_rFeatureDir)
    {
    const FeatureHeaderTypeID DirHeaderType = pi_rFeatureDir.HasHeaderDir() ? pi_rFeatureDir.GetHeaderDir().GetHeaderType() :
                                              FeatureHeaderTypeIDTrait<HeaderT>::value;

    return IsSupportedHeaderTypeTrait<HeaderT>::value &&
           (FeatureHeaderTypeIDTrait<HeaderT>::value == DirHeaderType);
    }

template <typename HeaderT>
FeatureHeaderTileHandler<HeaderT>* FeatureHeaderTileHandler<HeaderT>::CreateFromImpl (UniformFeatureDir* pi_pDir)
    {
    if (0 == pi_pDir || !IsCompatibleWith(*pi_pDir))
        return 0;

    return new FeatureHeaderTileHandler(pi_pDir->HasHeaderDir() ?
                                        static_cast<Impl*>(new LinearFeatureImpl(pi_pDir->GetHeaderDirP())) :
                                        static_cast<Impl*>(new PointOnlyImpl(pi_pDir->GetFeatureType(),
                                                                             pi_pDir->GetPointDirP())));
    }

template <typename HeaderT>
FeatureHeaderTileHandler<HeaderT>* FeatureHeaderTileHandler<HeaderT>::CreateFromImpl (MixedFeatureDir* pi_pDir)
    {
    if (0 == pi_pDir || !IsCompatibleWith(*pi_pDir))
        return 0;

    return new FeatureHeaderTileHandler(new LinearFeatureImpl(pi_pDir->GetHeaderDirP()));
    }

template <typename HeaderT>
typename FeatureHeaderTileHandler<HeaderT>::Ptr
FeatureHeaderTileHandler<HeaderT>::CreateFrom (UniformFeatureDir* pi_pDir)
    {
    return CreateFromImpl(pi_pDir);
    }

template <typename HeaderT>
typename FeatureHeaderTileHandler<HeaderT>::Ptr
FeatureHeaderTileHandler<HeaderT>::CreateFrom (MixedFeatureDir* pi_pDir)
    {
    return CreateFromImpl(pi_pDir);
    }

template <typename HeaderT>
typename FeatureHeaderTileHandler<HeaderT>::CPtr
FeatureHeaderTileHandler<HeaderT>::CreateFrom (const UniformFeatureDir* pi_pDir)
    {
    return CreateFromImpl(const_cast<UniformFeatureDir*>(pi_pDir));
    }

template <typename HeaderT>
typename FeatureHeaderTileHandler<HeaderT>::CPtr
FeatureHeaderTileHandler<HeaderT>::CreateFrom (const MixedFeatureDir* pi_pDir)
    {
    return CreateFromImpl(const_cast<MixedFeatureDir*>(pi_pDir));
    }


template <typename HeaderT>
FeatureHeaderTileHandler<HeaderT>::FeatureHeaderTileHandler (Impl* pi_pImpl)
    :   m_pImpl(pi_pImpl)
    {
    HPRECONDITION(0 != m_pImpl.get());
    }

template <typename HeaderT>
FeatureHeaderTileHandler<HeaderT>::~FeatureHeaderTileHandler ()
    {
    }

template <typename HeaderT>
bool FeatureHeaderTileHandler<HeaderT>::IsPointOnly () const
    {
    return m_pImpl->_IsPointOnly();
    }

template <typename HeaderT>
size_t FeatureHeaderTileHandler<HeaderT>::GetTileMaxHeaderCount () const
    {
    return m_pImpl->_GetTileMaxHeaderCount();
    }

template <typename HeaderT>
uint64_t FeatureHeaderTileHandler<HeaderT>::CountHeaders () const
    {
    return m_pImpl->_CountHeaders();
    }

template <typename HeaderT>
size_t FeatureHeaderTileHandler<HeaderT>::CountHeaders (TileID pi_ID) const
    {
    return m_pImpl->_CountHeaders(pi_ID);
    }

template <typename HeaderT>
bool FeatureHeaderTileHandler<HeaderT>::GetHeaders (TileID                  pi_ID,
                                                    HeaderArray&            pi_rTileHeaders)
    {
    return m_pImpl->_GetHeaders(pi_ID, pi_rTileHeaders);
    }

template <typename HeaderT>
bool FeatureHeaderTileHandler<HeaderT>::SetHeaders (TileID                      pi_ID,
                                                    const HeaderArray&          pi_rTileHeaders)
    {
    return m_pImpl->_SetHeaders(pi_ID, pi_rTileHeaders);
    }

template <typename HeaderT>
bool FeatureHeaderTileHandler<HeaderT>::AddHeaders (TileID&                     po_rID,
                                                    const HeaderArray&          pi_rTileHeaders)
    {
    return m_pImpl->_AddHeaders(po_rID, pi_rTileHeaders);
    }

template <typename HeaderT>
bool FeatureHeaderTileHandler<HeaderT>::RemoveHeaders (TileID pi_ID)
    {
    return m_pImpl->_RemoveHeaders(pi_ID);
    }

template <typename HeaderT>
bool FeatureHeaderTileHandler<HeaderT>::_Save ()
    {
    return true;
    }

template <typename HeaderT>
bool FeatureHeaderTileHandler<HeaderT>::_Load ()
    {
    return true;
    }