
-- --------------------------------------------------
-- Entity Designer DDL Script for SQL Server 2005, 2008, 2012 and Azure
-- --------------------------------------------------
-- Date Created: 07/09/2015 11:54:39
-- Generated from EDMX file: D:\Sources\dgndb06_commit\RealityModFramework\RealityCrawler\RealityCrawlerDB.edmx
-- --------------------------------------------------

SET QUOTED_IDENTIFIER OFF;
GO
USE [RealityCrawlerDB];
GO
IF SCHEMA_ID(N'dbo') IS NULL EXECUTE(N'CREATE SCHEMA [dbo]');
GO

-- --------------------------------------------------
-- Dropping existing FOREIGN KEY constraints
-- --------------------------------------------------

IF OBJECT_ID(N'[dbo].[FK_ServerWMSServerExtendedProperties]', 'F') IS NOT NULL
    ALTER TABLE [dbo].[WMSServerExtendedPropertiesSet] DROP CONSTRAINT [FK_ServerWMSServerExtendedProperties];
GO
IF OBJECT_ID(N'[dbo].[FK_WMSServerExtendedPropertiesWMSData]', 'F') IS NOT NULL
    ALTER TABLE [dbo].[WMSServerDataSet] DROP CONSTRAINT [FK_WMSServerExtendedPropertiesWMSData];
GO
IF OBJECT_ID(N'[dbo].[FK_WMSDataWMSData]', 'F') IS NOT NULL
    ALTER TABLE [dbo].[WMSServerDataSet] DROP CONSTRAINT [FK_WMSDataWMSData];
GO

-- --------------------------------------------------
-- Dropping existing tables
-- --------------------------------------------------

IF OBJECT_ID(N'[dbo].[ServerSet]', 'U') IS NOT NULL
    DROP TABLE [dbo].[ServerSet];
GO
IF OBJECT_ID(N'[dbo].[WMSServerExtendedPropertiesSet]', 'U') IS NOT NULL
    DROP TABLE [dbo].[WMSServerExtendedPropertiesSet];
GO
IF OBJECT_ID(N'[dbo].[WMSServerDataSet]', 'U') IS NOT NULL
    DROP TABLE [dbo].[WMSServerDataSet];
GO

-- --------------------------------------------------
-- Creating all tables
-- --------------------------------------------------

-- Creating table 'ServerSet'
CREATE TABLE [dbo].[ServerSet] (
    [ServerId] int IDENTITY(1,1) NOT NULL,
    [Name] nvarchar(max)  NULL,
    [URL] nvarchar(max)  NOT NULL,
    [Legal] nvarchar(max)  NULL,
    [Online] bit  NOT NULL,
    [LastCheck] datetime  NOT NULL,
    [LastTimeOnline] datetime  NOT NULL,
    [Latency] float  NOT NULL,
    [State] int  NOT NULL,
    [Type] int  NOT NULL,
    [CoordinateSystem] nvarchar(max)  NULL,
    [OldServerId] int  NULL,
    [LockedBy] nvarchar(max)  NULL,
    [LastLockUpdate] datetime  NULL
);
GO

-- Creating table 'WMSServerExtendedPropertiesSet'
CREATE TABLE [dbo].[WMSServerExtendedPropertiesSet] (
    [WMSId] int IDENTITY(1,1) NOT NULL,
    [Name] nvarchar(max)  NOT NULL,
    [Title] nvarchar(max)  NOT NULL,
    [Description] nvarchar(max)  NULL,
    [LayerNumber] int  NULL,
    [Version] nvarchar(max)  NOT NULL,
    [ContactPerson] nvarchar(max)  NULL,
    [ContactOrganization] nvarchar(max)  NULL,
    [ContactPosition] nvarchar(max)  NULL,
    [ContactAddressType] nvarchar(max)  NULL,
    [ContactAddress] nvarchar(max)  NULL,
    [ContactCity] nvarchar(max)  NULL,
    [ContactStateOrProvince] nvarchar(max)  NULL,
    [ContactPostCode] nvarchar(max)  NULL,
    [ContactCountry] nvarchar(max)  NULL,
    [ContactVoiceTelephone] nvarchar(max)  NULL,
    [ContactFacsimileTelephone] nvarchar(max)  NULL,
    [ContactElectronicMailAddress] nvarchar(max)  NULL,
    [Fees] nvarchar(max)  NULL,
    [AccessConstraints] nvarchar(max)  NULL,
    [GetCapabilitiesData] nvarchar(max)  NULL,
    [SupportedFormats] nvarchar(max)  NOT NULL,
    [GetMapURL] nvarchar(max)  NOT NULL,
    [GetMapURLQuery] nvarchar(max)  NULL,
    [Server_ServerId] int  NOT NULL
);
GO

-- Creating table 'WMSServerDataSet'
CREATE TABLE [dbo].[WMSServerDataSet] (
    [WMSDataId] int IDENTITY(1,1) NOT NULL,
    [Name] nvarchar(max)  NOT NULL,
    [Title] nvarchar(max)  NOT NULL,
    [Description] nvarchar(max)  NULL,
    [BoundingBox] geometry  NOT NULL,
    [WMSServerExtendedPropertiesWMSId] int  NOT NULL,
    [WMSDataParentId] int  NULL,
    [Selected] bit  NOT NULL,
    [CoordinateSystems] nvarchar(max)  NULL,
    [Style] nvarchar(max)  NULL
);
GO

-- --------------------------------------------------
-- Creating all PRIMARY KEY constraints
-- --------------------------------------------------

-- Creating primary key on [ServerId] in table 'ServerSet'
ALTER TABLE [dbo].[ServerSet]
ADD CONSTRAINT [PK_ServerSet]
    PRIMARY KEY CLUSTERED ([ServerId] ASC);
GO

-- Creating primary key on [WMSId] in table 'WMSServerExtendedPropertiesSet'
ALTER TABLE [dbo].[WMSServerExtendedPropertiesSet]
ADD CONSTRAINT [PK_WMSServerExtendedPropertiesSet]
    PRIMARY KEY CLUSTERED ([WMSId] ASC);
GO

-- Creating primary key on [WMSDataId] in table 'WMSServerDataSet'
ALTER TABLE [dbo].[WMSServerDataSet]
ADD CONSTRAINT [PK_WMSServerDataSet]
    PRIMARY KEY CLUSTERED ([WMSDataId] ASC);
GO

-- --------------------------------------------------
-- Creating all FOREIGN KEY constraints
-- --------------------------------------------------

-- Creating foreign key on [Server_ServerId] in table 'WMSServerExtendedPropertiesSet'
ALTER TABLE [dbo].[WMSServerExtendedPropertiesSet]
ADD CONSTRAINT [FK_ServerWMSServerExtendedProperties]
    FOREIGN KEY ([Server_ServerId])
    REFERENCES [dbo].[ServerSet]
        ([ServerId])
    ON DELETE CASCADE ON UPDATE NO ACTION;
GO

-- Creating non-clustered index for FOREIGN KEY 'FK_ServerWMSServerExtendedProperties'
CREATE INDEX [IX_FK_ServerWMSServerExtendedProperties]
ON [dbo].[WMSServerExtendedPropertiesSet]
    ([Server_ServerId]);
GO

-- Creating foreign key on [WMSServerExtendedPropertiesWMSId] in table 'WMSServerDataSet'
ALTER TABLE [dbo].[WMSServerDataSet]
ADD CONSTRAINT [FK_WMSServerExtendedPropertiesWMSData]
    FOREIGN KEY ([WMSServerExtendedPropertiesWMSId])
    REFERENCES [dbo].[WMSServerExtendedPropertiesSet]
        ([WMSId])
    ON DELETE CASCADE ON UPDATE NO ACTION;
GO

-- Creating non-clustered index for FOREIGN KEY 'FK_WMSServerExtendedPropertiesWMSData'
CREATE INDEX [IX_FK_WMSServerExtendedPropertiesWMSData]
ON [dbo].[WMSServerDataSet]
    ([WMSServerExtendedPropertiesWMSId]);
GO

-- Creating foreign key on [WMSDataParentId] in table 'WMSServerDataSet'
ALTER TABLE [dbo].[WMSServerDataSet]
ADD CONSTRAINT [FK_WMSDataWMSData]
    FOREIGN KEY ([WMSDataParentId])
    REFERENCES [dbo].[WMSServerDataSet]
        ([WMSDataId])
    ON DELETE NO ACTION ON UPDATE NO ACTION;
GO

-- Creating non-clustered index for FOREIGN KEY 'FK_WMSDataWMSData'
CREATE INDEX [IX_FK_WMSDataWMSData]
ON [dbo].[WMSServerDataSet]
    ([WMSDataParentId]);
GO

-- --------------------------------------------------
-- Script has ended
-- --------------------------------------------------