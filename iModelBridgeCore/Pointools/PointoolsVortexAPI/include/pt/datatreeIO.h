#pragma once

#include <ptds/DataSource.h>
#include <pt/datatreeBranch.h>

namespace pt
{
namespace datatree
{
	bool writeBinaryDatatree( const Branch *dtree, ptds::DataSource * dataSrc );
	bool readBinaryDatatree( Branch *root, ptds::DataSource * dataSrc );

}	// namespace pt
}	// namespace datatree