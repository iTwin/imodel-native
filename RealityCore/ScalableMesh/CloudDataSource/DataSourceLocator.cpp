/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "DataSourceLocator.h"
#include "include/DataSourceLocator.h"
#include "DataSourceAccount.h"

DataSourceSession::SessionInstance DataSourceSession::instanceCounter = 0;


DataSourceSession::DataSourceSession(void)
    {
    sessionInstance = 0;
    }

DataSourceSession::DataSourceSession(const SessionKey &key)
    {
    setSessionKey(key);
    }

void DataSourceSession::initializeInstance(void)
    {
    setInstanceCounter(getInstanceCounter() + 1);

    setSessionInstance(getInstanceCounter());
    }

void DataSourceSession::setSessionInstance(SessionInstance instance)
    {
    sessionInstance = instance;
    }

DataSourceSession::SessionInstance DataSourceSession::getSessionInstance(void) const
    {
    return sessionInstance;
    }

CLOUD_EXPORT void DataSourceSession::setKeyRemapFunction(const KeyRemapFunction &f)
    {
    keyRemapFunction = f;
    }

CLOUD_EXPORT const DataSourceSession::KeyRemapFunction &DataSourceSession::getKeyRemapFunction(void) const
    {
    return keyRemapFunction;
    }

void DataSourceSession::setSessionKey(const SessionKey &key)
    {
    initializeInstance();

    sessionKey = key;
    }

const DataSourceSession::SessionKey &DataSourceSession::getSessionKey(void) const
    {
    return sessionKey;
    }

void DataSourceSession::setInstanceCounter(SessionInstance value)
    {
    instanceCounter = value;
    }

DataSourceSession::SessionInstance DataSourceSession::getInstanceCounter(void)
    {
    return instanceCounter;
    }

DataSourceSession &DataSourceSession::operator=(const DataSourceSession &other)
    {
    sessionKey = other.getSessionKey();

    setSessionInstance(other.getSessionInstance());

    setKeyRemapFunction(other.getKeyRemapFunction());

    return *this;
    }

DataSourceSession &DataSourceSession::operator=(const SessionKey &key)
    {
    setSessionKey(key);

    return *this;
    }

DataSourceSession &DataSourceSession::operator=(const wchar_t *key)
    {
    if (key)
        {
        SessionKey k(key);

        setSessionKey(k);
        }

    return *this;
    }

bool DataSourceSession::operator==(const DataSourceSession &other) const
    {
    return (getSessionInstance() == other.getSessionInstance() && getSessionKey() == other.getSessionKey());
    }



// *************************************************************************************************


DataSourceLocator::DataSourceLocator(void)
{
    setService(nullptr);
    setAccount(nullptr);
    m_mode = DataSourceMode::DataSourceMode_Null;
}

DataSourceLocator::DataSourceLocator(const DataSourceLocator & locator)
{
    *this = locator;
}


DataSourceLocator &DataSourceLocator::operator=(const DataSourceLocator &locator)
    {
    setService(const_cast<DataSourceLocator &>(locator).getService());
    setAccount(locator.getAccount());
    setSessionName(locator.getSessionName());

    setPrefixPath(locator.getPrefixPath());
    setSubPath(locator.getSubPath());
    setSegmentName(locator.getSegmentName());

    setMode(locator.getMode());

    return *this;
    }

void DataSourceLocator::getURL(DataSourceURL &url)
{
    DataSourceAccount * account;

    if (account = getAccount())
        {
        switch (account->getPrefixPathType())
            {
            case PrefixPathSession:
                url = getSessionName().getSessionKey();
                break;

            case PrefixPathAccount:
            default:
                url = getPrefixPath();
                break;
            }
        }
    else
        {
                                                            // Use account prefix. This shouldn't happen (because Account should always be set)
        url = getPrefixPath();
        }

    url.append(getSubPath());
}

void DataSourceLocator::setName(const DataSourceName & name)
    {
    m_name = name;
    }

const DataSourceLocator::DataSourceName & DataSourceLocator::getName(void)
    {
    return m_name;
    }

void DataSourceLocator::setService(DataSourceService * newService)
{
    m_service = newService;
}

DataSourceService * DataSourceLocator::getService(void)
{
    return m_service;
}

void DataSourceLocator::setAccount(DataSourceAccount * sourceAccount)
{
    m_account = sourceAccount;
}

DataSourceAccount * DataSourceLocator::getAccount(void) const
{
    return m_account;
}

void DataSourceLocator::setSessionName(const SessionName &session)
    {
    m_session = session;
    }

const DataSourceLocator::SessionName &DataSourceLocator::getSessionName(void) const
    {
    return m_session;
    }

void DataSourceLocator::setPrefixPath(const DataSourceURL & path)
{
    m_prefixPath = path;
}

const DataSourceURL & DataSourceLocator::getPrefixPath(void) const
{
    return m_prefixPath;
}

void DataSourceLocator::setSubPath(const DataSourceURL & path)
{
    m_subPath = path;
}

const DataSourceURL & DataSourceLocator::getSubPath(void) const
{
    return m_subPath;
}

void DataSourceLocator::setSegmentName(const DataSourceURL & name)
{
    m_segmentName = name;
}


const DataSourceURL &DataSourceLocator::getSegmentName(void) const
{
    return m_segmentName;
}

void DataSourceLocator::setMode(DataSourceMode sourceMode)
{
    m_mode = sourceMode;
}

DataSourceMode DataSourceLocator::getMode(void) const
{
    return m_mode;
}