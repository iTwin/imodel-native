#include "PointoolsVortexAPIInternal.h"

#include <ptcloud2/metadata.h>
#include <ptcloud2/datachannel.h>

#include <stdio.h>

using namespace ptds;
using namespace pcloud;

enum ImportFlags
{
	Meta_ImportOptionsCombine = 1,
	Meta_ScanLineFiltered = 2,
	Meta_GenNormals = 4
};

MetaImportSettings::MetaImportSettings()
{
	channels_in_scan = 0;
	channels_imported = 0;
	import_options = 0;
	upper_scan_row_skip = 0;
	lower_scan_row_skip = 0;
	compression_tolerance = 0;
	spatial_filtering = 0;
	scan_point_max_dist = 0;
	min_range = 0;
	max_range = 0;
}
//---------------------------------------------------------------------------
// Write the meta data to a mem block
//---------------------------------------------------------------------------
int		MetaImportSettings::writeToBlock( WriteBlock &wb ) const
{
	int version = 1;
	wb.write(version);
	wb.write(channels_in_scan);
	wb.write(channels_imported);
	wb.write(import_options);
	wb.write(upper_scan_row_skip);
	wb.write(lower_scan_row_skip);
	wb.write(compression_tolerance);
	wb.write(spatial_filtering);
	wb.write(scan_point_max_dist);
	wb.write(min_range);
	wb.write(max_range);	

	return 1;
}
//---------------------------------------------------------------------------
// Read the meta data from a mem block
//---------------------------------------------------------------------------
void	MetaImportSettings::readFromBlock( ReadBlock &rb )
{
	int version = 1;
	rb.read(version);
	rb.read(channels_in_scan);
	rb.read(channels_imported);
	rb.read(import_options);
	rb.read(upper_scan_row_skip);
	rb.read(lower_scan_row_skip);
	rb.read(compression_tolerance);
	rb.read(spatial_filtering);
	rb.read(scan_point_max_dist);
	rb.read(min_range);
	rb.read(max_range);			
}
//---------------------------------------------------------------------------
// Get the meta data as a string
//---------------------------------------------------------------------------
bool	MetaImportSettings::getMetaDataString(const pt::String &item, pt::String &value ) const
{
	if (item.compare(L"ChannelsInSource")==0)
	{
		pt::String channels = pt::String("Position");
		if (channels_in_scan & pcloud::PCloud_Intensity)
			channels += pt::String(";Intensity");
		if (channels_in_scan & pcloud::PCloud_RGB)
			channels += pt::String(";RGB");
		if (channels_in_scan & pcloud::PCloud_Normal)
			channels += pt::String(";Normal");
		value = channels;
	}
	else if (item.compare(L"ChannelsImported")==0)
	{
		pt::String channels = pt::String("Position");
		if (channels_imported & pcloud::PCloud_Intensity)
			channels += pt::String(";Intensity");
		if (channels_imported & pcloud::PCloud_RGB)
			channels += pt::String(";RGB");
		if (channels_imported & pcloud::PCloud_Normal)
			channels += pt::String(";Normal");
		value = channels;
	}
	else if (item.compare(L"CombinedOnImport")==0)
	{
		value = (import_options & Meta_ImportOptionsCombine)
			? pt::String("true") : pt::String("false");
	}
	else if (item.compare(L"ScanLineFiltered")==0)
	{
		value = (import_options & Meta_ScanLineFiltered)
			? pt::String("true") : pt::String("false");
	}		
	else if (item.compare(L"UpperScanRowRemoval")==0)
	{
		value.format("%d", upper_scan_row_skip );
	}	
	else if (item.compare(L"LowerScanRowRemoval")==0)
	{
		value.format("%d", lower_scan_row_skip );
	}
	else if (item.compare(L"CompressionTolerance")==0)
	{
		value.format("%0.6f", compression_tolerance );
	}
	else if (item.compare(L"SpatialFiltering")==0)
	{
		value.format("%0.6f", spatial_filtering );
	}
	else if (item.compare(L"MaxScanLinePointDist")==0)
	{
		value.format("%0.6f", scan_point_max_dist );
	}
	else if (item.compare(L"ScanMinRange")==0)
	{
		value.format("%0.6f", min_range );
	}
	else if (item.compare(L"ScanMaxRange")==0)
	{
		value.format("%0.6f", max_range );
	}
	return value.length() ? true : false;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int			MetaImportSettings::numItems()
{
	return 11;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
pt::String	MetaImportSettings::itemName(int index)
{
	switch (index)
	{	
	case 0: return pt::String(L"ChannelsInSource"); 
	case 1: return pt::String(L"ChannelsImported"); 
	case 2: return pt::String(L"CombinedOnImport"); 
	case 3: return pt::String(L"ScanLineFiltered"); 
	case 4: return pt::String(L"UpperScanRowRemoval"); 
	case 5: return pt::String(L"LowerScanRowRemoval"); 
	case 6: return pt::String(L"CompressionTolerance"); 
	case 7: return pt::String(L"SpatialFiltering"); 
	case 8: return pt::String(L"MaxScanLinePointDist"); 
	case 9: return pt::String(L"ScanMinRange"); 
	case 10: return pt::String(L"ScanMaxRange"); 
	}
	return pt::String("");
}
//---------------------------------------------------------------------------
// Get the meta data as a string
//---------------------------------------------------------------------------
MetaHardware::MetaHardware()
{
	scanner_manufacturer = pt::String("");
	scanner_serial= pt::String("");
	scanner_model= pt::String("");
	camera_model= pt::String("");
	camera_serial= pt::String("");
	camera_lens= pt::String("");
}
//---------------------------------------------------------------------------
// Get the meta data as a string
//---------------------------------------------------------------------------
int		MetaHardware::writeToBlock( WriteBlock &wb ) const
{
	int version = 1;
	wb.write(version);

	wb.write( scanner_manufacturer );
	wb.write( scanner_serial );
	wb.write( scanner_model );
	wb.write( camera_model );
	wb.write( camera_serial );
	wb.write( camera_lens );

	return 1;
}
//---------------------------------------------------------------------------
// Get the meta data as a string
//---------------------------------------------------------------------------
void	MetaHardware::readFromBlock( ReadBlock &rb )
{
	int version = 1;
	rb.read(version);

	rb.read( scanner_manufacturer );
	rb.read( scanner_serial );
	rb.read( scanner_model );
	rb.read( camera_model );
	rb.read( camera_serial );
	rb.read( camera_lens );
}
//---------------------------------------------------------------------------
// Get the meta data as a string
//---------------------------------------------------------------------------
bool	MetaHardware::getMetaDataString(const pt::String &item, pt::String &value ) const
{
	value = L"";
	if (item.compare( L"ScannerManufacturer" ) == 0 || item.compare( L"Manufacturer" )==0)
		value = scanner_manufacturer;

	else if (item.compare( L"ScannerModel" ) == 0 || item.compare( L"Model" ) == 0)
		value = scanner_model;

	else if (item.compare( L"ScannerSerial" ) == 0 || item.compare( L"Serial" ) == 0)
		value = scanner_serial;

	else if (item.compare( L"CameraModel" ) == 0)
		value = camera_model;

	else if (item.compare( L"CameraSerial" ) == 0)
		value = camera_serial;

	else if (item.compare( L"CameraLens" ) == 0)
		value = camera_lens;

	return value.length() ? true : false;
}
//---------------------------------------------------------------------------
// Get the meta data as a string
//---------------------------------------------------------------------------
bool	MetaHardware::setMetaDataString(const pt::String &item, const pt::String &value )
{
	if (item.compare( L"ScannerManufacturer" ) == 0 || item.compare( L"Manufacturer" )==0)
		scanner_manufacturer = value;

	else if (item.compare( L"ScannerModel" ) == 0 || item.compare( L"Model" ) == 0)
		scanner_model = value;

	else if (item.compare( L"ScannerSerial" ) == 0 || item.compare( L"Serial" ) == 0)
		scanner_serial = value;

	else if (item.compare( L"CameraModel" ) == 0)
		camera_model = value;

	else if (item.compare( L"CameraSerial" ) == 0)
		camera_serial = value;

	else if (item.compare( L"CameraLens" ) == 0)
		camera_lens = value;

	else return false;

	return true;
}
//---------------------------------------------------------------------------
// Get the meta data as a string
//---------------------------------------------------------------------------
int			MetaHardware::numItems()
{
	return 6;
}
//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
pt::String	MetaHardware::itemName(int index)
{
	switch (index)
	{	
	case 0: return pt::String(L"ScannerManufacturer"); 
	case 1: return pt::String(L"ScannerModel"); 
	case 2: return pt::String(L"ScannerSerial"); 
	case 3: return pt::String(L"CameraModel"); 
	case 4: return pt::String(L"CameraSerial"); 
	case 5: return pt::String(L"CameraLens"); 
	}
	return pt::String("");
}
MetaSurvey::MetaSurvey()
{
	site_location = ""; 
	site_lat = 0;
	site_long =0;
	post_code = "";
	capture_date = 0;
	georef = "";

	per_company = "";
	per_operator ="";
	project_name ="";
	project_code ="";
}
int		MetaSurvey::writeToBlock( WriteBlock &wb ) const
{
	int version = 2;
	wb.write( version );

	wb.write( site_location );
	wb.write( site_lat );
	wb.write( site_long );
	wb.write( post_code );
	wb.write( capture_date );

	wb.write( per_company );
	wb.write( per_operator );
	wb.write( project_name );
	wb.write( project_code );

	wb.write( georef );
	return 1;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void	MetaSurvey::readFromBlock( ReadBlock &rb )
{
	int version = 1;
	rb.read( version );

	rb.read( site_location );
	rb.read( site_lat );
	rb.read( site_long );
	rb.read( post_code );
	rb.read( capture_date );

	rb.read( per_company );
	rb.read( per_operator );
	rb.read( project_name );
	rb.read( project_code );

	if (version > 1)
		rb.read( georef );
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool	MetaSurvey::getMetaDataString(const pt::String &item, pt::String &value ) const
{
	if (item.compare( L"Site" ) == 0)
		value = site_location;
	
	else if (item.compare( L"SiteLat" ) == 0 && fabs(site_lat) > 0)
		value.format( "%0.2f", site_lat );

	else if (item.compare( L"SiteLong" ) == 0 && fabs(site_long) > 0 )
		value.format( "%0.2f", site_long );

	else if (item.compare( L"PostCode" ) == 0)
		value = post_code;

	else if (item.compare( L"ZipCode" ) == 0)
		value = post_code;

	else if (item.compare( L"GeoReference" ) == 0)
		value = georef;

	else if (item.compare( L"DateOfCapture" ) == 0 && capture_date!= 0)
	{
		char buf[256];
		
		try
		{
			if (capture_date < 10e9)
			{
                struct tm*  ts;
                ts = localtime(&capture_date);

                if (ts->tm_year == 70) return false;
                strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);

				value = buf;

			}
			else
			{
				strcpy(buf, "unknown");
			}
		}
		catch (...)
		{
			strcpy(buf, "unknown");
		}
	}

	else if (item.compare( L"Company" ) == 0)
		value = per_company;

	else if (item.compare( L"Operator" ) == 0)
		value = per_operator;

	else if (item.compare( L"ProjectName" ) == 0)
		value = project_name;

	else if (item.compare( L"ProjectCode" ) == 0)
		value = project_code;

	return value.length() ? true : false;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool	MetaSurvey::setMetaDataString(const pt::String &item, const pt::String &value )
{
	if (item.compare( L"Site" ) == 0)
		site_location = value;
	
	else if (item.compare( L"SiteLat" ) == 0)
	{
		if (value.length())
			site_lat = atof( value.c_str() );		
		else site_lat = 0;
	}
	else if (item.compare( L"SiteLong" ) == 0)
	{
		if (value.length())
			site_long = atof( value.c_str() );		
		else site_long = 0;
	}
	else if (item.compare( L"PostCode" ) == 0)
		post_code = value;

	else if (item.compare( L"GeoReference" ) == 0)
		georef = value;

	else if (item.compare( L"ZipCode" ) == 0)
		post_code = value;

	else if (item.compare( L"DateOfCapture" ) == 0)
	{
		char buf[16];
		strncpy(buf, value.c_str(), 8);	
		buf[4] = 0;
		buf[7] = 0;
		buf[10] = 0;

        struct tm* ts;
        time(&capture_date);
        ts = localtime(&capture_date);

        ts->tm_year = atoi(buf);
        ts->tm_mon = atoi(&buf[5]);
        ts->tm_mday = atoi(&buf[8]);

        capture_date = mktime(ts);
	}

	else if (item.compare( L"Company" ) == 0)
		per_company = value;

	else if (item.compare( L"Operator" ) == 0)
		per_operator = value;

	else if (item.compare( L"ProjectName" ) == 0)
		project_name = value;

	else if (item.compare( L"ProjectCode" ) == 0)
		project_code = value;

	else return false;

	return true;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int			MetaSurvey::numItems()
{
	return 10;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
pt::String	MetaSurvey::itemName(int index)
{
	switch (index)
	{	
	case 0: return pt::String(L"Site"); 
	case 1: return pt::String(L"SiteLat"); 
	case 2: return pt::String(L"SiteLong"); 
	case 3: return pt::String(L"PostCode"); 
	case 4: return pt::String(L"DateOfCapture"); 
	case 5: return pt::String(L"Company"); 
	case 6: return pt::String(L"Operator"); 
	case 7: return pt::String(L"ProjectName"); 
	case 8: return pt::String(L"ProjectCode"); 
	case 9: return pt::String(L"GeoReference"); 
	}
	return pt::String("");
}
MetaDesc::MetaDesc()
{
	/* nothing to do, strings are initialised anyway */ 
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int		MetaDesc::writeToBlock( WriteBlock &wb ) const
{
	int version = 1;
	wb.write(version);

	wb.write( description );
	wb.write( keywords );
	wb.write( category );

	return 1;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void	MetaDesc::readFromBlock( ReadBlock &rb )
{
	int version = 1;
	rb.read(version);

	rb.read( description );
	rb.read( keywords );
	rb.read( category );
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool	MetaDesc::getMetaDataString(const pt::String &item, pt::String &value ) const
{
	if (item.compare( L"Description" ) == 0)
		value = description;

	else if (item.compare( L"Keywords" ) == 0)
		value = keywords;

	else if (item.compare( L"Category" ) == 0)
		value = category;

	return value.length() ? true : false;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool	MetaDesc::setMetaDataString(const pt::String &item, const pt::String &value )
{
	if (item.compare( L"Description" ) == 0)
		description = value;

	else if (item.compare( L"Keywords" ) == 0)
		keywords = value;

	else if (item.compare( L"Category" ) == 0)
		category = value;

	else return false;

	return true;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int			MetaDesc::numItems()
{
	return 3;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
pt::String	MetaDesc::itemName(int index)
{
	switch (index)
	{	
	case 0: return pt::String(L"Description"); 
	case 1: return pt::String(L"Keywords"); 
	case 2: return pt::String(L"Category"); 
	}
	return pt::String("");
}
//
//
//
MetaAudit::MetaAudit()
{
	original_num_scans = -1;
	file_iterator = 0;
	application = pt::String("Unknown");
	import_date = 0;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int		MetaAudit::writeToBlock( WriteBlock &wb ) const
{
	int version = 1;
	wb.write( version ); 
	
	wb.write( (int)scan_file_paths.size() );

	int i;

	time_t nullDate = 0;

	for ( i=0; i<scan_file_paths.size();i++)
	{
		wb.write( scan_file_paths[i] );
		if (scan_file_datemod.size() > i)
			wb.write( scan_file_datemod[i] );
		else wb.write( nullDate );
	}
	import_settings.writeToBlock( wb );

	/* edits */ 
	wb.write( (int)edits.size() );
	for (i=0;i<edits.size();i++)
	{
		wb.write( edits[i].generation );
		wb.write( edits[i].timestamp );
		wb.write( edits[i].editor );
	}	

	wb.write( original_num_scans );
	wb.write( import_date );
	wb.write( application );

	return 1;
}
//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void	MetaAudit::readFromBlock( ReadBlock &rb )
{
	int version = 1;
	int num=0;
	int i;

	rb.read( version );	
	rb.read( num );
	
	pt::String path;
	time_t t;

	for ( i=0; i<num;i++)
	{
		rb.read( path );
		rb.read( t );

		scan_file_paths.push_back(path);
		scan_file_datemod.push_back(t);
	}
	import_settings.readFromBlock( rb );

	/* edits */ 
	rb.read( num );
	for (i=0;i<num;i++)
	{
		Edit e;

		rb.read( e.generation );
		rb.read( e.timestamp );
		rb.read( e.editor );

		edits.push_back( e );
	}	

	rb.read( original_num_scans );
	rb.read( import_date );
	rb.read( application );
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool	MetaAudit::getMetaDataString(const pt::String &item, pt::String &value ) const
{
	if (item.compare( L"ScanPaths" ) == 0)
	{
		value = "";

		/* have to hack around the const */ 
		int *mutable_file_iterator_ptr = const_cast<int*>(&file_iterator);
		int &mutable_file_iterator = *mutable_file_iterator_ptr;

		if (file_iterator < scan_file_paths.size())
		{
			value = scan_file_paths[file_iterator];
			++mutable_file_iterator;
		}
		else 
		{
			mutable_file_iterator = 0;
			value = "";
			return true;
		}
	}
	else if (item.compare( "OriginalNumScans" ) == 0)
	{
		value.format( "%d", original_num_scans );
	}	
	else if (item.compare( "CreatorApp" ) == 0 || item.compare( "Application" ) == 0)
	{
		value = application;
	}
	else if (item.compare( "Generation" ) == 0)
	{
		value.format("%d", edits.size());
	}
	else if (item.compare( "DateCreated" ) == 0)
	{
		char buf[256];

		try
		{
			if (import_date < 10e9)
			{
				time_t now = time(0);

                struct tm*  ts = localtime(&import_date);

                if (ts->tm_year == 70) return false;

                strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
				value = buf;
			}
			else
			{
				strcpy(buf, "unknown");
			}
		}
		catch (...)
		{
			strcpy(buf, "unknown");
		}
	}
	else return import_settings.getMetaDataString(item, value);

	return value.length() ? true : false;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool	MetaAudit::setMetaDataString(const pt::String &item, const pt::String &value )
{
	if (item.compare( "ScanPaths" ) == 0)
	{
		scan_file_paths.push_back( value );
	}
	else if (item.compare( "OriginalNumScans" ) == 0)
	{
		original_num_scans = atoi(value.c_str());
	}	
	else if (item.compare( "CreatorApp" ) == 0 || item.compare( "Application" ) == 0)
	{
		application = value;
	}
	else return false;	
	
	return true;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int			MetaAudit::numItems()
{
	return 4 + import_settings.numItems();
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
pt::String	MetaAudit::itemName(int index)
{
	switch (index)
	{	
	case 0: return pt::String(L"Description"); 
	case 1: return pt::String(L"Keywords"); 
	case 2: return pt::String(L"Category"); 
	case 3: return pt::String(L"Category"); 
	default:
		index -= numItems();
		return import_settings.itemName(index);
	}
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int		MetaData::writeToBlock( WriteBlock &wb ) const
{
	int version = 1;
	wb.write( version );
	hardware.writeToBlock( wb );
	survey.writeToBlock( wb );
	desc.writeToBlock( wb );
	audit.writeToBlock( wb );

	user.writeToBlock( wb );
	return 1;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void	MetaData::readFromBlock( ReadBlock &rb )
{
	int version = 1;
	rb.read( version );
	hardware.readFromBlock( rb );
	survey.readFromBlock( rb );
	desc.readFromBlock( rb );
	audit.readFromBlock( rb );

	user.readFromBlock( rb );
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool	MetaData::getMetaDataString( const pt::String &section, const pt::String &item, pt::String &value ) const
{
	if (!section.length() || section.compare("Any")==0)
	{
		if (hardware.getMetaDataString(item, value)) return true;
		if (survey.getMetaDataString(item, value))	return true;
		if (desc.getMetaDataString(item, value))	return true;
		if (audit.getMetaDataString(item, value))	return true;
	}
	else
	{
		if (section.compare(L"Instrument")==0)	return hardware.getMetaDataString(item, value);
		if (section.compare(L"Survey")==0)		return survey.getMetaDataString(item, value);
		if (section.compare(L"Description")==0)	return desc.getMetaDataString(item, value);
		if (section.compare(L"Process")==0 || section.compare(L"Audit")==0) 
			return audit.getMetaDataString(item, value);
	}
	return user.getMetaTag(section, item, value);
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool	MetaData::setMetaDataString( const pt::String &section, const pt::String &item, const pt::String &value )
{
	if (!section.length() || section.compare("Any")==0)
	{
		if (hardware.setMetaDataString(item, value))return true;
		if (survey.setMetaDataString(item, value))	return true;
		if (desc.setMetaDataString(item, value))	return true;
		if (audit.setMetaDataString(item, value))	return true;
	}
	else
	{
		if (section.compare("Instrument")==0)	return hardware.setMetaDataString(item, value);
		if (section.compare("Survey")==0)		return survey.setMetaDataString(item, value);
		if (section.compare("Description")==0)	return desc.setMetaDataString(item, value);
		if (section.compare("Process")==0 || section.compare("Audit")==0) 
			return audit.setMetaDataString(item, value);
	}
//	user.setMetaTag(section, item, value); // always succeeds, creates group and tag if not exists
	return false;
}

/*****************************************************************************/

/*****************************************************************************/
void MetaData::operator=( const MetaData &m )
{
	hardware = m.hardware;
	survey = m.survey;
	desc = m.desc;
	audit = m.audit;
	user = m.user;
}

void MetaAudit::operator=( const MetaAudit &m )
{

	application = m.application;
	file_iterator = m.file_iterator;
	import_date = m.import_date;
	import_settings = m.import_settings;
	original_num_scans = m.original_num_scans;

	scan_file_datemod.clear();
	scan_file_datemod.insert(scan_file_datemod.begin(), m.scan_file_datemod.begin(), m.scan_file_datemod.end());

	scan_file_paths.clear();
	scan_file_paths.insert(scan_file_paths.begin(), m.scan_file_paths.begin(), m.scan_file_paths.end());

	edits.clear();
	edits.insert(edits.begin(), m.edits.begin(), m.edits.end());
}

void MetaDesc::operator=( const MetaDesc &m )
{
	category = m.category;
	description = m.description;
	keywords = m.keywords;
}

void MetaSurvey::operator=( const MetaSurvey &m )
{
	capture_date = m.capture_date;
	site_location = m.site_location;

	site_lat = m.site_lat;
	site_long = m.site_long;
	post_code = m.post_code;
	georef = m.georef;

	/* time */ 
	capture_date = m.capture_date;

	/* personnel */
	per_company = m.per_company;
	per_operator = m.per_operator;
	project_name = m.project_name;
	project_code = m.project_code;
}

void MetaHardware::operator=( const MetaHardware &m )
{
	scanner_manufacturer = m.scanner_manufacturer;
	scanner_serial =  m.scanner_serial;
	scanner_model = m.scanner_model;
	camera_model = m.camera_model;
	camera_serial = m.camera_serial;
	camera_lens = m.camera_lens;
}

void MetaImportSettings::operator=( const MetaImportSettings &m )
{
	channels_in_scan = m.channels_in_scan;
	channels_imported = m.channels_imported;
	import_options = m.import_options;
	compression_tolerance = m.compression_tolerance;
	spatial_filtering = m.spatial_filtering;
	scan_point_max_dist = m.scan_point_max_dist;
	upper_scan_row_skip = m.upper_scan_row_skip;
	lower_scan_row_skip = m.lower_scan_row_skip;
	min_range = m.min_range;
	max_range = m.max_range;

	metatags = m.metatags;
}