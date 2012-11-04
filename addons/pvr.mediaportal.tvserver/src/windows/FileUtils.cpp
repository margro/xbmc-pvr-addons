#include "../FileUtils.h"
#include "platform/os.h"
#include <string>
#include "platform/util/StdString.h"
#include "../utils.h"

namespace OS
{
  bool CFile::Exists(const std::string& strFileName)
  {
    CStdStringW strWFile = UTF8Util::ConvertUTF8ToUTF16(strFileName.c_str());
    DWORD dwAttr = GetFileAttributesW(strWFile.c_str());

    if(dwAttr != 0xffffffff)
    {
      return true;
    }
    return false;
  }
}