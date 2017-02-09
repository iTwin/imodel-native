#pragma once

#include "DataSourceDefs.h"
#include "DataSourceURL.h"
#include "DataSourceMode.h"

class DataSourceService;
class DataSourceAccount;


class DataSourceLocator
{

protected:

    DataSourceService                    *  m_service;
    DataSourceAccount                    *  m_account;

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

    void                                    getURL                  (DataSourceURL &url);

    void                                    setService              (DataSourceService *newService);
    DataSourceService                  *    getService              (void);

    void                                    setAccount              (DataSourceAccount *sourceAccount);
    DataSourceAccount                  *    getAccount              (void) const;


    void                                    setPrefixPath           (const DataSourceURL &path);
    const DataSourceURL                &    getPrefixPath           (void);

    void                                    setSubPath              (const DataSourceURL &path);
    const DataSourceURL                &    getSubPath              (void);

    void                                    setSegmentName          (DataSourceURL &segmentName);
    const DataSourceURL                &    getSegmentName          (void) const;

    void                                    setMode                 (DataSourceMode sourceMode);
    DataSourceMode                          getMode                 (void) const;

};


