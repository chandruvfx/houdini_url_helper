
#include "FS_HomeHelper.h"

#include <UT/UT_Assert.h>
#include <UT/UT_DirUtil.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_EnvControl.h>
#include <UT/UT_FileUtil.h>
#include <UT/UT_NTStreamUtil.h>
#include <UT/UT_OpUtils.h>
#include <UT/UT_SysClone.h>
#include <UT/UT_WorkArgs.h>
#include <UT/UT_WorkBuffer.h>
#include <PY/PY_Python.h>
#include <filesystem>
#include <string>
#include <iostream>
#include <fstream>


// Domain starting path specification 
#define HOME_SIGNATURE		"sRurl:"
#define HOME_SIGNATURE_LEN	6


void
installFSHelpers()
{
    new HDK_Sample::FS_HomeReadHelper();
    new HDK_Sample::FS_HomeWriteHelper();
    new HDK_Sample::FS_HomeInfoHelper();
}

using namespace HDK_Sample;

//
#define FS_HOMEREADER_HANDLE_OPTIONS

#ifdef FS_HOMEREADER_HANDLE_OPTIONS
static inline bool
fsFindLastOption(const char *option_name, const UT_WorkArgs &options,
		 UT_String *option_value = NULL, int *option_index = NULL)
{
    UT_String	option_tag;
    
    option_tag  = option_name;
    option_tag += "=";

    if( option_value )
	option_value->clear();
    if( option_index )
	*option_index = -1;
    for( int i = options.getArgc()-1; i >= 0; i--  )
    {
	UT_String	token( options(i) );
	if( token.startsWith( option_tag ))
	{
	    if( option_value )
		token.substr( *option_value, option_tag.length() );
	    if( option_index )
		*option_index = i;
	    return true;
	}
    }

    return false;
}

static inline void
fsGetFileAndOptions( const char *source, 
		     UT_String &file_str, UT_String &options_str )
{
    UT_String	    source_str( source );
    UT_WorkArgs	    list;
    int		    count;
    // domain declaration
    source_str.tokenize( list, "//" );
    count = list.getArgc();
    UT_ASSERT( 0 <= count && count <= 2 );
    file_str.harden( count > 0 ? list(0) : "" );
    options_str.harden( count > 1 ? list(1) : "" );
}

static inline void
fsAppendOptionsExceptForSkipped( UT_WorkBuffer &buff, 
				 const UT_WorkArgs &options, int skip_index )
{
    bool do_questionmark = true;
    bool do_ampersand    = false;
    for( int i = 0; i < options.getArgc(); i++ )
    {
	if( i == skip_index )
	    continue;

	if( do_questionmark )
	{
	    buff.append("//");
	    do_questionmark = false;
	}

	if( do_ampersand )
	    buff.append('#');

	buff.append( options(i) );
	do_ampersand = true;
    }
}

static inline bool
fsSplitPathIntoFileAndSection( const char *source, 
			       UT_String &file_name, UT_String &section_name)
{
    UT_String	    file_str, options_str;
    UT_WorkArgs	    options;
    int		    section_option_index;

    fsGetFileAndOptions( source, file_str, options_str );
    options_str.tokenize( options, '#' );
    fsFindLastOption( " ", options, &section_name, &section_option_index);

    UT_WorkBuffer buff;
    buff.append( file_str );
    fsAppendOptionsExceptForSkipped( buff, options, section_option_index );
    buff.copyIntoString( file_name );

    return section_name.isstring();
}
    
static inline void
fsCombineFileAndSectionIntoPath( UT_String &source, 
			     const char *file_name, const char *section_name)
{
    UT_WorkBuffer   buffer;
    UT_String	    file_name_str( file_name );
    char	    separator;

    // Figure out the separator: '?' if no options yet, or '&' if there are.
    UT_ASSERT( UTisstring( file_name ) && UTisstring( section_name ));
    separator =  file_name_str.findChar('?') ? '&' : '?';

    // Copy the base file name and append the section option.
    buffer.strcpy( file_name );
    buffer.append( separator );
    buffer.append( " " );
    buffer.append( section_name );

    buffer.copyIntoString( source );
}

static inline void
fsStripSectionOptions( UT_String &path, UT_StringArray &sections )
{
    UT_String	section_name;
    while( fsSplitPathIntoFileAndSection( path, path, section_name ))
	sections.insert( section_name, /*index=*/ 0 );
}

static inline void
fsAppendSectionNames( UT_String &path, UT_StringArray &sections )
{
    for( int i = 0; i < sections.entries(); i++ )
	UT_OpUtils::combineStandardIndexFileSectionPath( path, path, 
							 sections(i) );
}

static inline void
fsAppendFileSuffix( UT_String &str, 
		    const UT_WorkArgs &options, const char *option_name,
		    const char *separator )
{
    UT_String	option_value;

    if( fsFindLastOption( option_name, options, &option_value ))
    {
	str += separator;
	str += option_value;
    }
}

static inline void
fsProcessNonSectionOptions( UT_String &source )
{
    
    // Main protocol which save the Srurl path in the file 
    std::ofstream sRurlFile("C:\\Users\\chandrakanth\\Documents\\houdini19.5\\dso\\sRurl.txt");
    sRurlFile << source;
    sRurlFile.close();

    // Written file SRurl path readed and find the asset and shot subfolders and written
    // back into another file 
    PYrunPythonStatementsFromFile("C:\\Users\\chandrakanth\\Documents\\houdini19.5\\dso\\resolve_entity.py");

    // The parsed entity gonna read back 
    std::string entity;
    std::ifstream entityFile("C:\\Users\\chandrakanth\\Documents\\houdini19.5\\dso\\entity.txt");
    getline (entityFile, entity);
    entityFile.close();

    // The readed entity concatinated to the shot and asset subfolder text
    std::string shot = "/shots/" + entity +"/" ; 
    std::string assets = "/assets/" + entity +"/" ; 

    UT_String	    options_str;
    UT_WorkArgs	    options, options_1, options_2, options_3;

    fsGetFileAndOptions( source, source, options_str );

    options_str.tokenize( options, '#' );
    options_str.tokenize( options_3, '@' );
    options_str.tokenize( options_2, '?' );
    options_str.tokenize( options_1, ':' );

    UT_ASSERT( ! fsFindLastOption( " ", options ));
    
    // File hook handlers 
    //

    fsAppendFileSuffix( source, options_1, "show", "shows/" );
    fsAppendFileSuffix( source, options_1, "asset", assets.data());
    // fsAppendFileSuffix( source, options_1, "seq", "/shots/" );
    // fsAppendFileSuffix( source, options_1, "name", "/" );
    fsAppendFileSuffix( source, options_1, "shot", shot.data());
    fsAppendFileSuffix( source, options_1, "step", "/" );
    fsAppendFileSuffix( source, options_1, "work", "/work/" );
    fsAppendFileSuffix( source, options_1, "pub", "/publish/" );
    fsAppendFileSuffix( source, options_1, "file", "/" );
    fsAppendFileSuffix( source, options_2, "version", "_v" );
    fsAppendFileSuffix( source, options_3, "revision", "_r" );
    fsAppendFileSuffix( source, options, "ext", "." );
}
#endif // FS_HOMEREADER_HANDLE_OPTIONS

// ============================================================================
static inline void
fsPrefixPathWithHome(UT_String &path)
{
    UT_WorkBuffer   buff;

    // Substitute 'home:' prefix with home path.
    UT_ASSERT( path.length() >= HOME_SIGNATURE_LEN );
    buff.append( UT_EnvControl::getString(ENV_HOME));
    if( path(HOME_SIGNATURE_LEN) != '/' )
	buff.append('/');
    buff.append( &path.buffer()[HOME_SIGNATURE_LEN] );

    buff.copyIntoString(path);
}

static bool
fsConvertToStandardPathForWrite(UT_String &destpath, const char *srcpath)
{
    // Handle only the 'home:' protocol paths.
    if( strncmp(srcpath, HOME_SIGNATURE, HOME_SIGNATURE_LEN) != 0 )
	return false;

    destpath = srcpath;
#ifdef FS_HOMEREADER_HANDLE_OPTIONS
    UT_StringArray  sections;
    fsStripSectionOptions( destpath, sections );

    fsProcessNonSectionOptions( destpath );
    fsAppendSectionNames( destpath, sections );
#endif 

    fsPrefixPathWithHome(destpath);
    return true;
}

static bool
fsConvertToStandardPathForRead(UT_String &destpath, const char *srcpath)
{
    // Handle only the 'home:' protocol paths.
    if( strncmp(srcpath, HOME_SIGNATURE, HOME_SIGNATURE_LEN) != 0 )
	return false;

    destpath = srcpath;
#ifdef FS_HOMEREADER_HANDLE_OPTIONS
    fsProcessNonSectionOptions( destpath );
#endif 

    fsPrefixPathWithHome(destpath);
    return true;
}

static bool
fsConvertToStandardPathForInfo(UT_String &destpath, const char *srcpath)
{
#ifdef FS_HOMEREADER_HANDLE_OPTIONS
#endif 
    return fsConvertToStandardPathForRead(destpath, srcpath);
}

// ============================================================================
FS_HomeReadHelper::FS_HomeReadHelper()
{
    UTaddAbsolutePathPrefix(HOME_SIGNATURE);
}

FS_HomeReadHelper::~FS_HomeReadHelper()
{
}

FS_ReaderStream *
FS_HomeReadHelper::createStream(const char *source, const UT_Options *)
{
    FS_ReaderStream		*is = 0;
    UT_String			 homepath;

    if( fsConvertToStandardPathForRead(homepath, source) )
	is = new FS_ReaderStream(homepath);


    return is;
}

bool
FS_HomeReadHelper::splitIndexFileSectionPath(const char *source_section_path,
					UT_String &index_file_path,
					UT_String &section_name)
{
#ifdef FS_HOMEREADER_HANDLE_OPTIONS
    // We should be splitting only our own paths.
    if( !source_section_path ||
	strncmp(source_section_path, HOME_SIGNATURE, HOME_SIGNATURE_LEN) != 0 )
	return false;


    fsSplitPathIntoFileAndSection( source_section_path, 
				   index_file_path, section_name );

    return true;
#else
    return false;
#endif // FS_HOMEREADER_HANDLE_OPTIONS
}

bool
FS_HomeReadHelper::combineIndexFileSectionPath( UT_String &source_section_path,
					const char *index_file_path,
					const char *section_name)
{
#ifdef FS_HOMEREADER_HANDLE_OPTIONS
    if( strncmp(index_file_path, HOME_SIGNATURE, HOME_SIGNATURE_LEN) != 0 )
	return false;

    fsCombineFileAndSectionIntoPath( source_section_path, 
				     index_file_path, section_name );

    // Return true to indicate we are handling own gluing of section paths.
    return true;
#else
    return false;
#endif // FS_HOMEREADER_HANDLE_OPTIONS
}


// ============================================================================
FS_HomeWriteHelper::FS_HomeWriteHelper()
{
    UTaddAbsolutePathPrefix(HOME_SIGNATURE);
}

FS_HomeWriteHelper::~FS_HomeWriteHelper()
{
}

FS_WriterStream *
FS_HomeWriteHelper::createStream(const char *source)
{
    FS_WriterStream		*os = 0;
    UT_String			 homepath;

    if( fsConvertToStandardPathForWrite(homepath, source) )
	os = new FS_WriterStream(homepath);

    return os;
}

bool
FS_HomeWriteHelper::canMakeDirectory(const char *source)
{
    // Same as FS_HomeInfoHelper::canHandle()
    return (strncmp(source, HOME_SIGNATURE, HOME_SIGNATURE_LEN) == 0);
}

bool
FS_HomeWriteHelper::makeDirectory(
        const char *source, mode_t mode, bool ignore_umask)
{
    UT_String homepath;
    if (!fsConvertToStandardPathForWrite(homepath, source))
        return false;

    return UT_FileUtil::makeDirs(homepath, mode, ignore_umask);
}

// ============================================================================
FS_HomeInfoHelper::FS_HomeInfoHelper()
{
    UTaddAbsolutePathPrefix(HOME_SIGNATURE);
}

FS_HomeInfoHelper::~FS_HomeInfoHelper()
{
}

bool
FS_HomeInfoHelper::canHandle(const char *source)
{
    return (strncmp(source, HOME_SIGNATURE, HOME_SIGNATURE_LEN) == 0);
}

bool
FS_HomeInfoHelper::hasAccess(const char *source, int mode)
{
    UT_String			 homepath;

    if( fsConvertToStandardPathForInfo(homepath, source) )
    {
	FS_Info			 info(homepath);

	return info.hasAccess(mode);
    }

    return false;
}

bool
FS_HomeInfoHelper::getIsDirectory(const char *source)
{
    UT_String			 homepath;

    if( fsConvertToStandardPathForInfo(homepath, source) )
    {
	FS_Info			 info(homepath);

	return info.getIsDirectory();
    }

    return false;
}

time_t
FS_HomeInfoHelper::getModTime(const char *source)
{
    UT_String			 homepath;

    if( fsConvertToStandardPathForInfo(homepath, source) )
    {
	FS_Info			 info(homepath);

	return info.getModTime();
    }

    return 0;
}

int64
FS_HomeInfoHelper::getSize(const char *source)
{
    UT_String			 homepath;

    if( fsConvertToStandardPathForInfo(homepath, source) )
    {
	FS_Info			 info(homepath);

	return info.getFileDataSize();
    }

    return 0;
}

UT_String
FS_HomeInfoHelper::getExtension(const char *source)
{
    UT_String			 homepath;

    if( fsConvertToStandardPathForInfo(homepath, source) )
    {
#ifdef FS_HOMEREADER_HANDLE_OPTIONS
	UT_String	    filename_str;
	UT_String	    options_str;
	UT_String	    option_value;
	UT_WorkArgs	    options;

	fsGetFileAndOptions( source, filename_str, options_str );
	options_str.tokenize( options, '#' );
	if( fsFindLastOption( "ext", options, &option_value ))
	{
	    UT_String	     extension(UT_String::ALWAYS_DEEP, ".");
	    extension += option_value;
	    return extension;
	}
#endif
	return FS_InfoHelper::getExtension(homepath);
    }

    return FS_InfoHelper::getExtension(source);
}

bool
FS_HomeInfoHelper::getContents(const char *source,
			       UT_StringArray &contents,
			       UT_StringArray *dirs)
{
    UT_String			 homepath;

    if( fsConvertToStandardPathForInfo(homepath, source) )
    {
	FS_Info			 info(homepath);

	return info.getContents(contents, dirs);
    }

    return false;
}

