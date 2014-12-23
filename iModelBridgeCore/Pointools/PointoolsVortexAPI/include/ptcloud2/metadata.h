// File Metadata structure
#pragma once
#include <ptcloud2/pcloud.h>
#include <ptcloud2/metatags.h>
#include <pt/datatree.h>
#include <pt/ptstring.h>
#include <time.h>

namespace pcloud
{
struct PCLOUD_API MetaImportSettings
{
	enum ImportSettings
	{
		ScaleIntensities = 1,
		CombineClouds = 2,
		GenerateNormals = 4
	};
	MetaImportSettings();

	unsigned int channels_in_scan;
	unsigned int channels_imported;
	unsigned int import_options;
	float compression_tolerance;
	float spatial_filtering;
	float scan_point_max_dist;
	int upper_scan_row_skip;
	int lower_scan_row_skip;
	float min_range;
	float max_range;

	MetaTags	metatags;

	int			numItems();
	pt::String	itemName(int index);

	int		writeToBlock( ptds::WriteBlock &wb ) const;
	void	readFromBlock( ptds::ReadBlock &rb );
	bool	getMetaDataString(const pt::String &item, pt::String &value ) const;

	void operator = (const MetaImportSettings &m);
};

struct PCLOUD_API MetaHardware
{
	MetaHardware();

	pt::String scanner_manufacturer;
	pt::String scanner_serial;
	pt::String scanner_model;
	pt::String camera_model;
	pt::String camera_serial;
	pt::String camera_lens;

	int			numItems();
	pt::String	itemName(int index);

	int		writeToBlock( ptds::WriteBlock &wb ) const;
	void	readFromBlock( ptds::ReadBlock &rb );
	bool	getMetaDataString(const pt::String &item, pt::String &value ) const;
	bool	setMetaDataString(const pt::String &item, const pt::String &value );

	void operator = (const MetaHardware &m);
};

struct PCLOUD_API MetaSurvey
{
	MetaSurvey();

	/* site */ 
	pt::String	site_location;
	double		site_lat;
	double		site_long;
	pt::String	post_code;
	pt::String	georef;

	/* time */ 
	time_t		capture_date;

	/* personnel */
	pt::String per_company;
	pt::String per_operator;
	pt::String project_name;
	pt::String project_code;

	int			numItems();
	pt::String	itemName(int index);

	int		writeToBlock( ptds::WriteBlock &wb ) const;
	void	readFromBlock( ptds::ReadBlock &rb );
	bool	getMetaDataString(const pt::String &item, pt::String &value ) const;
	bool	setMetaDataString(const pt::String &item, const pt::String &value );

	void operator = (const MetaSurvey &m);
};

struct PCLOUD_API MetaDesc
{
	MetaDesc();

	pt::String description;
	pt::String keywords;
	pt::String category;

	int			numItems();
	pt::String	itemName(int index);

	int		writeToBlock( ptds::WriteBlock &wb ) const;
	void	readFromBlock( ptds::ReadBlock &rb );
	bool	getMetaDataString(const pt::String &item, pt::String &value ) const;
	bool	setMetaDataString(const pt::String &item, const pt::String &value );

	void operator = (const MetaDesc &m);
};

struct PCLOUD_API MetaAudit
{
	MetaAudit();

	struct Edit
	{
		int generation;
		time_t timestamp;
		pt::String editor;

		void operator =(const Edit &e) 
		{ 
			generation = e.generation; 
			timestamp = e.timestamp; 
			editor = e.editor; 
		}
	};

	std::vector< pt::String > scan_file_paths;
	std::vector< time_t >	scan_file_datemod;
	MetaImportSettings		import_settings;
	std::vector<Edit>		edits;
	int						original_num_scans;
	time_t					import_date;
	pt::String				application;
	int						file_iterator;

	int			numItems();
	pt::String	itemName(int index);

	int			writeToBlock( ptds::WriteBlock &wb ) const;
	void		readFromBlock( ptds::ReadBlock &rb );
	bool		getMetaDataString(const pt::String &item, pt::String &value ) const;
	bool		setMetaDataString(const pt::String &item, const pt::String &value );

	void operator = (const MetaAudit &m);
};

struct PCLOUD_API MetaData
{
	MetaHardware	hardware;
	MetaSurvey		survey;
	MetaDesc		desc;
	MetaAudit		audit;	
	
	MetaTags		user;

	int		writeToBlock( ptds::WriteBlock &wb ) const;
	void	readFromBlock( ptds::ReadBlock &rb );
	bool	getMetaDataString( const pt::String &section, const pt::String &item, pt::String &value ) const;
	bool	setMetaDataString( const pt::String &section, const pt::String &item, const pt::String &value );

	void operator = (const MetaData &m);
};
}