INSERT [dbo].[SpatialDataSources]([MainURL], [DataSourceType])
VALUES ('http://www.openstreetmap.org/', 'OSM')

declare @SDSID int

set @SDSID = SCOPE_IDENTITY()

INSERT [dbo].[Metadatas]([Legal])
VALUES ('© OpenStreetMap contributors')

declare @METAID int

set @METAID = SCOPE_IDENTITY()

INSERT [dbo].[SpatialEntityBases]([Footprint], [Name], [DataSourceTypesAvailable], [Metadata_Id])
VALUES (geometry::STGeomFromText('POLYGON((-180 -90, -180 90, 180 90, 180 -90, -180 -90))', 4326), 'OpenStreetMap', 'OSM', @METAID)

declare @SEBID int

set @SEBID = SCOPE_IDENTITY()



INSERT [dbo].[SpatialEntities]([Id])
VALUES (@SEBID)

INSERT [dbo].[SpatialEntitySpatialDataSources]([SpatialDataSource_Id], [SpatialEntity_Id])
VALUES (@SDSID, @SEBID)