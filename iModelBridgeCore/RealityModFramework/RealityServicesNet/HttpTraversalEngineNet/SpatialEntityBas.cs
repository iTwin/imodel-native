namespace HttpTraversalEngineNet
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    [Table("SpatialEntityBases")]
    public partial class SpatialEntityBas
    {
        public int Id { get; set; }

        [Required]
        public DbGeometry Footprint { get; set; }

        [Required]
        public string Name { get; set; }

        public string Keywords { get; set; }

        public string AssociateFile { get; set; }

        public string ProcessingDescription { get; set; }

        public string DataSourceTypesAvailable { get; set; }

        public string AccuracyResolutionDensity { get; set; }

        public string ResolutionInMeters { get; set; }

        public string DataProvider { get; set; }

        public string DataProviderName { get; set; }

        public int? ParentDataset_Id { get; set; }

        public int? Thumbnail_Id { get; set; }

        public int? Metadata_Id { get; set; }

        [DatabaseGenerated(DatabaseGeneratedOption.Computed)]
        [StringLength(50)]
        public string IdStr { get; set; }

        [Column(TypeName = "date")]
        public DateTime? Date { get; set; }

        [StringLength(100)]
        public string Classification { get; set; }

        public virtual Metadata Metadata { get; set; }

        public virtual SpatialEntity SpatialEntity { get; set; }

        public virtual SpatialEntityDataset SpatialEntityDataset { get; set; }

        public virtual Thumbnail Thumbnail { get; set; }

        public virtual SpatialEntityDataset SpatialEntityDataset1 { get; set; }
    }
}
