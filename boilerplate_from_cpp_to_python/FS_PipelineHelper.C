/*  HOW TO COMPILE
hcustom FS_PipelineHelper.C -i C:\Users\chandrakanth\Documents\houdini19.5\dso\fs -I "D:\Program Files\Side Effects Software\Houdini 19.5.368\toolkit\include" -I "D:\Program Files\Side Effects Software\Houdini 19.5.368\python39\include" -l "D:\Program Files\Side Effects Software\Houdini 19.5.368\python39\libs\python39.lib"
*/

#include <UT/UT_Assert.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_DirUtil.h>
#include <UT/UT_EnvControl.h>
#include <UT/UT_NTStreamUtil.h>
#include <UT/UT_OpUtils.h>
#include <UT/UT_SysClone.h>
#include <UT/UT_WorkArgs.h>
#include <UT/UT_WorkBuffer.h>
#include <iostream>

#include <cstdlib>

#include "FS_PipelineHelper.h"
#include "pxr/pxr.h"
#include "pxr/base/tf/api.h"

#include "hboost/python/extract.hpp"
#include "pxr/base/tf/pyUtils.h"
#include "pxr/base/tf/pyInterpreter.h"

#define PROJECT_SIGNATURE "prj:"
#define PROJECT_SIGNATURE_LEN 4

void installFSHelpers() {
  new HDK_notawhale::FS_ProjectReaderHelper();
  new HDK_notawhale::FS_ProjectWriterHelper();
  new HDK_notawhale::FS_ProjectInfoHelper();
}

using namespace HDK_notawhale;
using namespace PXR_INTERNAL_NS;
using namespace std;


inline bool fsConvertToStandardPath(UT_String &destpath, const char *srcpath) {
  if (strncmp(srcpath, PROJECT_SIGNATURE, PROJECT_SIGNATURE_LEN) != 0)
    return false;

  TfPyRunSimpleString("result = r'C:\\Users\\chandrakanth\\Documents\\shows\\MHZ\\assets\\char\\mike1\\mod\\publish\\abc\\mike1_pos_v001.abc'");
  hboost::python::extract<std::string> getString(TfPyRunString("result", Py_eval_input).get());
  string ret = getString();
  destpath = ret;
  return true;
}

////// FS_ProjectReaderHelper
FS_ProjectReaderHelper::FS_ProjectReaderHelper() { UTaddAbsolutePathPrefix(PROJECT_SIGNATURE); }

FS_ProjectReaderHelper::~FS_ProjectReaderHelper() {}

FS_ReaderStream *FS_ProjectReaderHelper::createStream(const char *source, const UT_Options *) {
  FS_ReaderStream *is = 0;
  UT_String path;
  if (strncmp(source, PROJECT_SIGNATURE, PROJECT_SIGNATURE_LEN) != 0)
    return is;

  if (fsConvertToStandardPath(path, source))
    is = new FS_ReaderStream(path);

  return is;
}

bool FS_ProjectReaderHelper::splitIndexFileSectionPath(const char *source_section_path,
                                                       UT_String &index_file_path,
                                                       UT_String &section_name) {
  return false;
}

bool FS_ProjectReaderHelper::combineIndexFileSectionPath(UT_String &source_section_path,
                                                         const char *index_file_path,
                                                         const char *section_name) {
  return false;
}

////// FS_ProjectWriterHelper
FS_ProjectWriterHelper::FS_ProjectWriterHelper() { UTaddAbsolutePathPrefix(PROJECT_SIGNATURE); }

FS_ProjectWriterHelper::~FS_ProjectWriterHelper() {}

FS_WriterStream *FS_ProjectWriterHelper::createStream(const char *source) {
  FS_WriterStream *os = 0;
  UT_String homepath;
  if (fsConvertToStandardPath(homepath, source))
    os = new FS_WriterStream(homepath);
  return os;
}

////// FS_ProjectInfoHelper
FS_ProjectInfoHelper::FS_ProjectInfoHelper() { UTaddAbsolutePathPrefix(PROJECT_SIGNATURE); }

FS_ProjectInfoHelper::~FS_ProjectInfoHelper() {}

bool FS_ProjectInfoHelper::canHandle(const char *source) {
  return (strncmp(source, PROJECT_SIGNATURE, PROJECT_SIGNATURE_LEN) == 0);
}

bool FS_ProjectInfoHelper::hasAccess(const char *source, int mode) {
  UT_String path;
  if (fsConvertToStandardPath(path, source)) {
    FS_Info info(path);
    return info.hasAccess(mode);
  }
  return false;
}

bool FS_ProjectInfoHelper::getIsDirectory(const char *source) {
  UT_String path;
  if (fsConvertToStandardPath(path, source)) {
    FS_Info info(path);
    return info.getIsDirectory();
  }
  return false;
}

time_t FS_ProjectInfoHelper::getModTime(const char *source) {
  UT_String path;
  if (fsConvertToStandardPath(path, source)) {
    FS_Info info(path);
    return info.getModTime();
  }
  return 0;
}

int64 FS_ProjectInfoHelper::getSize(const char *source) {
  UT_String path;
  if (fsConvertToStandardPath(path, source)) {
    FS_Info info(path);
    return info.getFileDataSize();
  }
  return 0;
}

UT_String FS_ProjectInfoHelper::getExtension(const char *source) {
  UT_String path;
  if (fsConvertToStandardPath(path, source)) {
    return FS_InfoHelper::getExtension(path);
  }
  return FS_InfoHelper::getExtension(source);
}

bool FS_ProjectInfoHelper::getContents(const char *source, UT_StringArray &contents,
                                       UT_StringArray *dirs) {
  UT_String path;
  if (fsConvertToStandardPath(path, source)) {
    FS_Info info(path);
    return info.getContents(contents, dirs);
  }
  return false;
}

bool FS_ProjectInfoHelper::getPathOnDisk(UT_String &path, const char *file) {
  UT_String tmp_path;
  if (fsConvertToStandardPath(tmp_path, file)) {
    path = std::move(tmp_path);
    return true;
  }
  return false;
}