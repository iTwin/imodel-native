/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DataSourceDefs.h"
#include "DataSourceURL.h"
#include "DataSourceMode.h"

#include <functional>
#include <Bentley/WString.h>

class DataSourceService;
class DataSourceAccount;


class DataSourceSession
    {
    public:

        typedef int64_t                         SessionInstance;
        typedef std::wstring                    SessionKey;

        typedef std::function<std::string()> KeyRemapFunction;

    protected:

        static SessionInstance                  instanceCounter;

        SessionInstance                         sessionInstance;
        SessionKey                              sessionKey;

        KeyRemapFunction                        keyRemapFunction;

    protected:

        void                                    setSessionInstance          (SessionInstance instance);

        void                                    setInstanceCounter          (SessionInstance value);
        SessionInstance                         getInstanceCounter          (void);

        void                                    initializeInstance          (void);

    public:

        CLOUD_EXPORT                            DataSourceSession           (void);
        CLOUD_EXPORT                            DataSourceSession           (const SessionKey &key);

        CLOUD_EXPORT    void                    setSessionKey               (const SessionKey &key);
        CLOUD_EXPORT    const SessionKey &      getSessionKey               (void) const;

        CLOUD_EXPORT    SessionInstance         getSessionInstance          (void) const;

        CLOUD_EXPORT    void                    setKeyRemapFunction         (const KeyRemapFunction &f);
        CLOUD_EXPORT    const KeyRemapFunction& getKeyRemapFunction         (void) const;

        CLOUD_EXPORT    DataSourceSession &     operator=                   (const DataSourceSession &other);
        CLOUD_EXPORT    DataSourceSession &     operator=                   (const SessionKey &key);
        CLOUD_EXPORT    DataSourceSession &     operator=                   (const wchar_t *key);

        CLOUD_EXPORT    bool                    operator==                  (const DataSourceSession &other) const;
    };


class DataSourceTypes
    {
    public:

    typedef std::wstring                ServiceName;
    typedef std::wstring                AccountName;
    typedef DataSourceSession           SessionName;
    typedef std::wstring                DataSourceName;
    typedef std::wstring                AccountIdentifier;
    typedef std::wstring                AccountKey;
    typedef std::string                 AccountSSLCertificatePath;

    enum PrefixPathType
        {
        PrefixPathAccount,
        PrefixPathSession
        };

    };

class DataSourceLocator : public DataSourceTypes
{

protected:

    DataSourceName                          m_name;
    DataSourceService                    *  m_service;
    DataSourceAccount                    *  m_account;
    SessionName                             m_session;

    DataSourceURL                           m_prefixPath;
    DataSourceURL                           m_subPath;
    DataSourceURL                           m_segmentName;

    DataSourceMode                          m_mode;


protected:

    void                                    setURL                  (const DataSourceURL &newURL);
    void                                    setURL                  (const DataSourceURL &prefix, DataSourceURL &subPath);

public:

                                            DataSourceLocator       (void);
                                            DataSourceLocator       (DataSourceLocator &locator);

                                           ~DataSourceLocator       () = default;

    CLOUD_EXPORT void                       getURL                  (DataSourceURL &url);

    void                                    setName                 (const DataSourceName &name);
    const DataSourceName                &   getName                 (void);

    void                                    setService              (DataSourceService *newService);
    DataSourceService                  *    getService              (void);

    void                                    setAccount              (DataSourceAccount *sourceAccount);
    DataSourceAccount                  *    getAccount              (void) const;

    void                                    setSessionName          (const SessionName &session);
    const SessionName                   &   getSessionName          (void) const;

    void                                    setPrefixPath           (const DataSourceURL &path);
    const DataSourceURL                &    getPrefixPath           (void) const;

    void                                    setSubPath              (const DataSourceURL &path);
    const DataSourceURL                &    getSubPath              (void) const;

    void                                    setSegmentName          (const DataSourceURL &segmentName);
    const DataSourceURL                &    getSegmentName          (void) const;

    void                                    setMode                 (DataSourceMode sourceMode);
    DataSourceMode                          getMode                 (void) const;

    DataSourceLocator &                     operator=               (const DataSourceLocator &other);

};


