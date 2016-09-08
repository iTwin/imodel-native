namespace HttpTraversalEngineNet
{
    using System;
    using System.Data.Entity;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Linq;

    public partial class RealityDB : DbContext
    {
        public RealityDB()
            : base("name=RealityDB")
        {
        }

        public virtual DbSet<Metadata> Metadatas { get; set; }
        public virtual DbSet<Server> Servers { get; set; }
        public virtual DbSet<SpatialDataSource> SpatialDataSources { get; set; }
        public virtual DbSet<SpatialEntity> SpatialEntities { get; set; }
        public virtual DbSet<SpatialEntityBas> SpatialEntityBases { get; set; }
        public virtual DbSet<SpatialEntityDataset> SpatialEntityDatasets { get; set; }
        public virtual DbSet<Thumbnail> Thumbnails { get; set; }

        protected override void OnModelCreating(DbModelBuilder modelBuilder)
        {
            modelBuilder.Entity<Metadata>()
                .HasMany(e => e.SpatialEntityBases)
                .WithOptional(e => e.Metadata)
                .HasForeignKey(e => e.Metadata_Id);

            modelBuilder.Entity<Server>()
                .HasMany(e => e.SpatialDataSources)
                .WithOptional(e => e.Server)
                .HasForeignKey(e => e.Server_Id);

            modelBuilder.Entity<SpatialDataSource>()
                .HasMany(e => e.SpatialEntities)
                .WithMany(e => e.SpatialDataSources)
                .Map(m => m.ToTable("SpatialEntitySpatialDataSources"));

            modelBuilder.Entity<SpatialEntityBas>()
                .HasOptional(e => e.SpatialEntity)
                .WithRequired(e => e.SpatialEntityBas);

            modelBuilder.Entity<SpatialEntityBas>()
                .HasOptional(e => e.SpatialEntityDataset1)
                .WithRequired(e => e.SpatialEntityBas);

            modelBuilder.Entity<SpatialEntityDataset>()
                .HasMany(e => e.SpatialEntityBases)
                .WithOptional(e => e.SpatialEntityDataset)
                .HasForeignKey(e => e.ParentDataset_Id);

            modelBuilder.Entity<SpatialEntityDataset>()
                .HasMany(e => e.SpatialEntityDatasets1)
                .WithOptional(e => e.SpatialEntityDataset1)
                .HasForeignKey(e => e.AlternateDataset_Id);

            modelBuilder.Entity<Thumbnail>()
                .HasMany(e => e.SpatialEntityBases)
                .WithOptional(e => e.Thumbnail)
                .HasForeignKey(e => e.Thumbnail_Id)
                .WillCascadeOnDelete();
        }
    }
}
