#pragma once

#ifdef TSREADER_DEBUG
#define TSDEBUG XBMC->Log
#else
#ifdef _MSC_VER
#define TSDEBUG
#else
#define TSDEBUG(...)
#endif
#endif
