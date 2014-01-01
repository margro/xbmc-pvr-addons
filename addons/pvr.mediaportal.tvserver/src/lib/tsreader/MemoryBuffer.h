#pragma once
/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *************************************************************************
 *  This file is a modified version from Team MediaPortal's
 *  TsReader DirectShow filter
 *  MediaPortal is a GPL'ed HTPC-Application
 *  Copyright (C) 2005-2012 Team MediaPortal
 *  http://www.team-mediaportal.com
 *
 * Changes compared to Team MediaPortal's version:
 * - Code cleanup for PVR addon usage
 * - Code refactoring for cross platform usage
 */

#ifdef LIVE555

#include "platform/threads/mutex.h"
#include <vector>

class CMemoryBuffer
{
  public:
    CMemoryBuffer(void);
    virtual ~CMemoryBuffer(void);

    unsigned long ReadFromBuffer(unsigned char *pbData, long lDataLength);
    long PutBuffer(unsigned char *pbData, long lDataLength);
    void Clear();
    unsigned long Size();
    void Run(bool onOff);
    bool IsRunning();

    typedef struct
    {
      unsigned char* data;
      int   nDataLength;
      int   nOffset;
    } BufferItem;

  protected:
    std::vector<BufferItem *> m_Array;
    PLATFORM::CMutex m_BufferLock;
    unsigned long    m_BytesInBuffer;
    PLATFORM::CEvent m_event;
    bool m_bRunning;
};
#endif //LIVE555
