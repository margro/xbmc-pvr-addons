#include "DvbData.h"

#include "client.h" 

using namespace ADDON;
using namespace PLATFORM;

void Dvb::TimerUpdates()
{
}

Dvb::Dvb() 
{
  m_bIsConnected = false;
  m_strServerName = "Dvb";
  CStdString strURL = "", strURLStream = "", strURLRecording = "";

  // simply add user@pass in front of the URL if username/password is set
  if ((g_strUsername.length() > 0) && (g_strPassword.length() > 0))
    strURL.Format("%s:%s@", g_strUsername.c_str(), g_strPassword.c_str());
  strURL.Format("http://%s%s:%u/", strURL.c_str(), g_strHostname.c_str(), g_iPortWeb);
  m_strURL = strURL.c_str();

  if ((g_strUsername.length() > 0) && (g_strPassword.length() > 0))
    strURLRecording.Format("%s:%s@", g_strUsername.c_str(), g_strPassword.c_str());
  strURLRecording.Format("http://%s%s:%u/", strURLRecording.c_str(), g_strHostname.c_str(), g_iPortRecording);
  m_strURLRecording = strURLRecording.c_str();

  if ((g_strUsername.length() > 0) && (g_strPassword.length() > 0))
    strURLStream.Format("%s:%s@", g_strUsername.c_str(), g_strPassword.c_str());
  strURLStream.Format("http://%s%s:%u/", strURLStream.c_str(), g_strHostname.c_str(), g_iPortStream);
  m_strURLStream = strURLStream.c_str();

  m_iNumRecordings = 0;
  m_iNumChannelGroups = 0;
  m_iCurrentChannel = -1;
  m_iClientIndexCounter = 1;
  m_bInitial = false;

  m_iUpdateTimer = 0;
}

bool Dvb::Open()
{
  CLockObject lock(m_mutex);

  XBMC->Log(LOG_NOTICE, "%s - DVBViewer Addon Configuration options", __FUNCTION__);
  XBMC->Log(LOG_NOTICE, "%s - Hostname: '%s'", __FUNCTION__, g_strHostname.c_str());
  XBMC->Log(LOG_NOTICE, "%s - WebPort: '%d'", __FUNCTION__, g_iPortWeb);
  XBMC->Log(LOG_NOTICE, "%s - StreamPort: '%d'", __FUNCTION__, g_iPortStream);
  
  m_bIsConnected = GetDeviceInfo();

  if (m_channels.size() == 0) {
    XBMC->Log(LOG_DEBUG, "%s No stored channels found, fetch from webapi", __FUNCTION__);

    if (!LoadChannels())
      return false;

    m_bInitial = true;
  }
  TimerUpdates();

  XBMC->Log(LOG_INFO, "%s Starting separate client update thread...", __FUNCTION__);
  CreateThread(); 
  
  return IsRunning(); 
}

void  *Dvb::Process()
{
  XBMC->Log(LOG_DEBUG, "%s - starting", __FUNCTION__);

  while(!IsStopped())
  {

    Sleep(5 * 1000);
    m_iUpdateTimer += 5;

    if (m_bInitial == false)
    {
      m_iUpdateTimer = 0;
 
      // Trigger Timer and Recording updates acording to the addon settings
      CLockObject lock(m_mutex);
      XBMC->Log(LOG_INFO, "%s Perform Updates!", __FUNCTION__);

      TimerUpdates();
      PVR->TriggerRecordingUpdate();
    }

  }

  CLockObject lock(m_mutex);
  m_started.Broadcast();
  //XBMC->Log(LOG_DEBUG, "%s - exiting", __FUNCTION__);

  return NULL;
}


bool Dvb::LoadChannels() 
{
  channelsdat *channel;
  int num_channels, channel_pos;
  char channel_group[26]="", channel_name[26]="";
  CStdString strBIN, url;
  CStdString strTmp;

  m_groups.clear();
  m_iNumChannelGroups = 0;
  DvbChannelGroup newGroup;
  DvbChannel newChannel;

  url.Format("%sapi/getchannelsdat.html", m_strURL.c_str()); 
  strBIN = GetHttpBIN(url);
  if (strBIN.size() == 0)
  {
    XBMC->Log(LOG_ERROR, "%s Unable to load channels.dat", __FUNCTION__);
    return false;
  }
  strBIN.erase(0, CHANNELDAT_HEADER);
  channel = (channelsdat*)strBIN.data();
  num_channels = strBIN.size() / sizeof(channelsdat);

  channel_pos = 0;
  for (int i=0; i<num_channels; i++)
  {
    if (i) channel++;

    if (channel->Flags & (1 << 7)) continue;
    if ((strcmp(channel_group, channel->Category) != 0) && (channel->Flags & (1 << 3)))
      {
        newGroup.strGroupName.assign(strncpy(channel_group, channel->Category, 25));
        newGroup.strServiceReference = "";
        m_groups.push_back(newGroup);
        m_iNumChannelGroups++; 
      }

    channel_pos++;

    /* EPGChannelID = (TunerType + 1)*2^48 + NID*2^32 + TID*2^16 + SID */
    newChannel.llEpgId = ((uint64_t)(channel->TunerType + 1)<<48) + ((uint64_t)channel->OriginalNetwork_ID<<32) + ((uint64_t)channel->TransportStream_ID<<16) + channel->Service_ID;
    newChannel.Encrypted = channel->Flags & (1 << 0);

    /* ChannelID = (tunertype + 1) * 536870912 + APID * 65536 + SID */
    newChannel.iChannelId = (channel->TunerType + 1)*536870912 + channel->Audio_PID*65536 + channel->Service_ID;
    newChannel.bRadio = (channel->Flags & (1 << 3)) == 0;

    newChannel.strGroupName = newGroup.strGroupName;
    newChannel.iUniqueId = m_channels.size()+1;
    newChannel.iChannelNumber = m_channels.size()+1;
    newChannel.strServiceReference = "";
    newChannel.strChannelName.assign(strncpy(channel_name, channel->ChannelName, 25));
  
    strTmp.Format("%supnp/channelstream/%d.ts", m_strURLStream.c_str(), channel_pos-1);
    newChannel.strStreamURL = strTmp;

    strTmp.Format("%sLogos/%s.png", m_strURL.c_str(), URLEncodeInline(newChannel.strChannelName.c_str())); 
    newChannel.strIconPath = strTmp;

    m_channels.push_back(newChannel);
  }

  XBMC->Log(LOG_INFO, "%s Loaded %d Channelsgroups", __FUNCTION__, m_iNumChannelGroups);
  return true;
}

bool Dvb::IsConnected() 
{
  return m_bIsConnected;
}

CStdString Dvb::GetHttpXML(CStdString& url)
{
  CStdString strResult;
  void* fileHandle = XBMC->OpenFile(url.c_str(), 0);
  if (fileHandle)
  {
    char buffer[1024];
    while (XBMC->ReadFileString(fileHandle, buffer, 1024))
      strResult.append(buffer);
    XBMC->CloseFile(fileHandle);
  }
  return strResult;
}

CStdString Dvb::GetHttpBIN(CStdString& url)
{
  CStdString strResult;
  void* fileHandle = XBMC->OpenFile(url.c_str(), 0);
  if (fileHandle)
  {
    char buffer[1024];
    while (int bytesRead = XBMC->ReadFile(fileHandle, buffer, 1024))
      strResult.append(buffer, bytesRead);
    XBMC->CloseFile(fileHandle);
  }
  return strResult;
}

const char * Dvb::GetServerName() 
{
  return m_strServerName.c_str();  
}

int Dvb::GetChannelsAmount()
{
  return m_channels.size();
}

int Dvb::GetTimersAmount()
{
  return m_timers.size();
}

unsigned int Dvb::GetRecordingsAmount() {
  return m_iNumRecordings;
}

PVR_ERROR Dvb::GetChannels(ADDON_HANDLE handle, bool bRadio)
{
    for (unsigned int iChannelPtr = 0; iChannelPtr < m_channels.size(); iChannelPtr++)
  {
    DvbChannel &channel = m_channels.at(iChannelPtr);
    if (channel.bRadio == bRadio)
    {
      PVR_CHANNEL xbmcChannel;
      memset(&xbmcChannel, 0, sizeof(PVR_CHANNEL));

      xbmcChannel.iUniqueId         = channel.iUniqueId;
      xbmcChannel.bIsRadio          = channel.bRadio;
      xbmcChannel.iChannelNumber    = channel.iChannelNumber;
      strncpy(xbmcChannel.strChannelName, channel.strChannelName.c_str(), sizeof(xbmcChannel.strChannelName));
      strncpy(xbmcChannel.strInputFormat, "", sizeof(xbmcChannel.strInputFormat)); // unused

      CStdString strStream;
      strStream.Format("pvr://stream/tv/%i.ts", channel.iUniqueId);
      strncpy(xbmcChannel.strStreamURL, strStream.c_str(), sizeof(xbmcChannel.strStreamURL)); //channel.strStreamURL.c_str();
	  xbmcChannel.iEncryptionSystem = channel.Encrypted;
      
      strncpy(xbmcChannel.strIconPath, channel.strIconPath.c_str(), sizeof(xbmcChannel.strIconPath));
      xbmcChannel.bIsHidden         = false;

      PVR->TransferChannelEntry(handle, &xbmcChannel);
    }
  }

  return PVR_ERROR_NO_ERROR;
}

Dvb::~Dvb() 
{
  StopThread();
  
  m_channels.clear();  
  m_timers.clear();
  m_recordings.clear();
  m_groups.clear();
  m_bIsConnected = false;
}

int Dvb::ParseDate(CStdString Date)
{
  struct tm timeinfo;

  memset(&timeinfo, 0, sizeof(tm));
  sscanf(Date, "%04d%02d%02d%02d%02d%02d", &timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
  timeinfo.tm_mon  -= 1;
  timeinfo.tm_year -= 1900;
  timeinfo.tm_isdst = -1;

  return mktime(&timeinfo);
}

PVR_ERROR Dvb::GetEPGForChannel(ADDON_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd)
{
  DvbChannel &myChannel = m_channels.at(channel.iUniqueId-1);

  CStdString url;
  url.Format("%sapi/epg.html?lvl=2&channel=%lld&start=%f&end=%f",  m_strURL.c_str(), myChannel.llEpgId, iStart/86400.0 + 25569, iEnd/86400.0 + 25569); 
 
  CStdString strXML;
  strXML = GetHttpXML(url);

  int iNumEPG = 0;

  XMLResults xe;
  XMLNode xMainNode = XMLNode::parseString(strXML.c_str(), NULL, &xe);

  if(xe.error != 0)  {
    XBMC->Log(LOG_ERROR, "%s Unable to parse XML. Error: '%s' ", __FUNCTION__, XMLNode::getError(xe.error));
    return PVR_ERROR_SERVER_ERROR;
  }

  XMLNode xNode = xMainNode.getChildNode("epg");
  int n = xNode.nChildNode("programme");

  XBMC->Log(LOG_INFO, "%s Number of elements: '%d'", __FUNCTION__, n);

  for (int i = 0; i<n; i++)
  {
    CStdString strTmp;
    int iTmpStart;
    int iTmpEnd;

    XMLNode xTmp = xNode.getChildNode("programme", i);
    iTmpStart = ParseDate(xNode.getChildNode("programme", i).getAttribute("start"));
    iTmpEnd = ParseDate(xNode.getChildNode("programme", i).getAttribute("stop"));

    if ((iEnd > 1) && (iEnd < iTmpEnd))
       continue;
    
    DvbEPGEntry entry;
    entry.startTime = iTmpStart;
    entry.endTime = iTmpEnd;

    if (!GetInt(xTmp, "eventid", entry.iEventId))  
      continue;

    entry.iChannelId = channel.iUniqueId;
    
    if(!GetString(xTmp.getChildNode("titles"), "title", strTmp))
      continue;

    entry.strTitle = strTmp;
    
    entry.strServiceReference = myChannel.strServiceReference;

    if (GetString(xTmp.getChildNode("events"), "event", strTmp))
       entry.strPlotOutline = strTmp;

    if (GetString(xTmp.getChildNode("descriptions"), "description", strTmp))
      entry.strPlot = strTmp;

    EPG_TAG broadcast;
    memset(&broadcast, 0, sizeof(EPG_TAG));

    broadcast.iUniqueBroadcastId  = entry.iEventId;
    broadcast.strTitle            = entry.strTitle.c_str();
    broadcast.iChannelNumber      = channel.iChannelNumber;
    broadcast.startTime           = entry.startTime;
    broadcast.endTime             = entry.endTime;
    broadcast.strPlotOutline      = entry.strPlotOutline.c_str();
    broadcast.strPlot             = entry.strPlot.c_str();
    broadcast.strIconPath         = ""; // unused
    broadcast.iGenreType          = 0; // unused
    broadcast.iGenreSubType       = 0; // unused
    broadcast.strGenreDescription = "";
    broadcast.firstAired          = 0;  // unused
    broadcast.iParentalRating     = 0;  // unused
    broadcast.iStarRating         = 0;  // unused
    broadcast.bNotify             = false;
    broadcast.iSeriesNumber       = 0;  // unused
    broadcast.iEpisodeNumber      = 0;  // unused
    broadcast.iEpisodePartNumber  = 0;  // unused
    broadcast.strEpisodeName      = ""; // unused

    PVR->TransferEpgEntry(handle, &broadcast);

    iNumEPG++; 

    XBMC->Log(LOG_INFO, "%s loaded EPG entry '%d:%s' channel '%d' start '%d' end '%d'", __FUNCTION__, broadcast.iUniqueBroadcastId, broadcast.strTitle, entry.iChannelId, entry.startTime, entry.endTime);
  }

  XBMC->Log(LOG_INFO, "%s Loaded %u EPG Entries for channel '%s'", __FUNCTION__, iNumEPG, channel.strChannelName);
  return PVR_ERROR_NO_ERROR;
}

int Dvb::GetChannelPos(int iChannelId)
{
  for (unsigned int i = 0;i<m_channels.size();  i++) 
  {
	  if (m_channels[i].iChannelId == iChannelId)
      return i+1;
  }
  return -1;
}

int Dvb::GetChannelNumber(CStdString strServiceReference)  
{
  for (unsigned int i = 0;i<m_channels.size();  i++) 
  {
    if (!strServiceReference.compare(m_channels[i].strServiceReference))
      return i+1;
  }
  return -1;
}

PVR_ERROR Dvb::GetTimers(ADDON_HANDLE handle)
{
  return PVR_ERROR_NOT_IMPLEMENTED;
}

std::vector<DvbTimer> Dvb::LoadTimers()
{
  CStdString url; 
  url.Format("%s%s", m_strURL.c_str(), "web/timerlist"); 

  CStdString strXML;
  strXML = GetHttpXML(url);

  XMLResults xe;
  XMLNode xMainNode = XMLNode::parseString(strXML.c_str(), NULL, &xe);
  
  std::vector<DvbTimer> timers;

  if(xe.error != 0)  {
    XBMC->Log(LOG_ERROR, "%s Unable to parse XML. Error: '%s' ", __FUNCTION__, XMLNode::getError(xe.error));
    return timers;
  }

  XMLNode xNode = xMainNode.getChildNode("e2timerlist");
  int n = xNode.nChildNode("e2timer");

  XBMC->Log(LOG_INFO, "%s Number of elements: '%d'", __FUNCTION__, n);
  

  while(n>0)
  {
    int i = n-1;
    n--;
    XMLNode xTmp = xNode.getChildNode("e2timer", i);

    CStdString strTmp;
    int iTmp;
    bool bTmp;
    int iDisabled;
    
    if (GetString(xTmp, "e2name", strTmp)) 
      XBMC->Log(LOG_DEBUG, "%s Processing timer '%s'", __FUNCTION__, strTmp.c_str());
 
    if (!GetInt(xTmp, "e2state", iTmp)) 
      continue;

    if (!GetInt(xTmp, "e2disabled", iDisabled))
      continue;

    DvbTimer timer;
    
    timer.strTitle          = strTmp;

    if (GetString(xTmp, "e2servicereference", strTmp))
      timer.iChannelId = GetChannelNumber(strTmp.c_str());

    if (!GetInt(xTmp, "e2timebegin", iTmp)) 
      continue; 
 
    timer.startTime         = iTmp;
    
    if (!GetInt(xTmp, "e2timeend", iTmp)) 
      continue; 
 
    timer.endTime           = iTmp;
    
    if (GetString(xTmp, "e2description", strTmp))
      timer.strPlot        = strTmp.c_str();
 
    if (GetInt(xTmp, "e2repeated", iTmp))
      timer.iWeekdays         = iTmp;
    else 
      timer.iWeekdays = 0;

    if (timer.iWeekdays != 0)
      timer.bRepeating      = true; 
    else
      timer.bRepeating = false;
    
    if (GetInt(xTmp, "e2eit", iTmp))
      timer.iEpgID = iTmp;
    else 
      timer.iEpgID = 0;

    timer.state = PVR_TIMER_STATE_NEW;

    if (!GetInt(xTmp, "e2state", iTmp))
      continue;

    XBMC->Log(LOG_DEBUG, "%s e2state is: %d ", __FUNCTION__, iTmp);
  
    if (iTmp == 0) {
      timer.state = PVR_TIMER_STATE_SCHEDULED;
      XBMC->Log(LOG_DEBUG, "%s Timer state is: SCHEDULED", __FUNCTION__);
    }
    
    if (iTmp == 2) {
      timer.state = PVR_TIMER_STATE_RECORDING;
      XBMC->Log(LOG_DEBUG, "%s Timer state is: RECORDING", __FUNCTION__);
    }
    
    if (iTmp == 3 && iDisabled == 0) {
      timer.state = PVR_TIMER_STATE_COMPLETED;
      XBMC->Log(LOG_DEBUG, "%s Timer state is: COMPLETED", __FUNCTION__);
    }

    if (GetBoolean(xTmp, "e2cancled", bTmp)) {
      if (bTmp)  {
        timer.state = PVR_TIMER_STATE_ABORTED;
        XBMC->Log(LOG_DEBUG, "%s Timer state is: ABORTED", __FUNCTION__);
      }
    }

	if (iDisabled == 1) {
		timer.state = PVR_TIMER_STATE_CANCELLED;
		XBMC->Log(LOG_DEBUG, "%s Timer state is: Cancelled", __FUNCTION__);
	}

    if (timer.state == PVR_TIMER_STATE_NEW)
      XBMC->Log(LOG_DEBUG, "%s Timer state is: NEW", __FUNCTION__);

    timers.push_back(timer);

    XBMC->Log(LOG_INFO, "%s fetched Timer entry '%s', begin '%d', end '%d'", __FUNCTION__, timer.strTitle.c_str(), timer.startTime, timer.endTime);
  }

  XBMC->Log(LOG_INFO, "%s fetched %u Timer Entries", __FUNCTION__, timers.size());
  return timers; 
}

CStdString Dvb::URLEncodeInline(const CStdString& strData)
{
  /* Copied from xbmc/URL.cpp */

  CStdString strResult;

  /* wonder what a good value is here is, depends on how often it occurs */
  strResult.reserve( strData.length() * 2 );

  for (int i = 0; i < (int)strData.size(); ++i)
  {
    int kar = (unsigned char)strData[i];
    //if (kar == ' ') strResult += '+'; // obsolete
    if (isalnum(kar) || strchr("-_.!()" , kar) ) // Don't URL encode these according to RFC1738
    {
      strResult += kar;
    }
    else
    {
      CStdString strTmp;
      strTmp.Format("%%%02.2x", kar);
      strResult += strTmp;
    }
  }
  return strResult;
}


PVR_ERROR Dvb::AddTimer(const PVR_TIMER &timer)
{
  return PVR_ERROR_NOT_IMPLEMENTED;
}

PVR_ERROR Dvb::DeleteTimer(const PVR_TIMER &timer) 
{
  return PVR_ERROR_NOT_IMPLEMENTED;
}

PVR_ERROR Dvb::GetRecordings(ADDON_HANDLE handle)
{
  m_iNumRecordings = 0;
  m_recordings.clear();

  CStdString url;
  url.Format("%s%s", m_strURL.c_str(), "api/recordings.html");
 
  CStdString strXML;
  strXML = GetHttpXML(url);

  XMLResults xe;
  XMLNode xMainNode = XMLNode::parseString(strXML.c_str(), NULL, &xe);

  if(xe.error != 0)  {
    XBMC->Log(LOG_ERROR, "%s Unable to parse XML. Error: '%s' ", __FUNCTION__, XMLNode::getError(xe.error));
    return PVR_ERROR_SERVER_ERROR;
  }

  XMLNode xNode = xMainNode.getChildNode("recordings");
  int n = xNode.nChildNode("recording");

  XBMC->Log(LOG_INFO, "%s Number of elements: '%d'", __FUNCTION__, n);
 
  int iNumRecording = 0; 

  while(n>0)
  {
    int i = n-1;
    n--;
    XMLNode xTmp = xNode.getChildNode("recording", i);
    CStdString strTmp;

    DvbRecording recording;
    if (xNode.getChildNode("recording", i).getAttribute("id"))
      recording.strRecordingId = xNode.getChildNode("recording", i).getAttribute("id");

    if (GetString(xTmp, "title", strTmp))
      recording.strTitle = strTmp;
    
    if (GetString(xTmp, "info", strTmp))
      recording.strPlotOutline = strTmp;

    if (GetString(xTmp, "desc", strTmp))
      recording.strPlot = strTmp;
    
    if (GetString(xTmp, "channel", strTmp))
      recording.strChannelName = strTmp;

	strTmp.Format("%supnp/recordings/%d.ts", m_strURLRecording.c_str(), atoi(recording.strRecordingId.c_str()));
    recording.strStreamURL = strTmp;

    recording.startTime = ParseDate(xNode.getChildNode("recording", i).getAttribute("start"));

    int hours, mins, secs;
    sscanf(xNode.getChildNode("recording", i).getAttribute("duration"), "%02d%02d%02d", &hours, &mins, &secs);
    recording.iDuration = hours*60*60 + mins*60 + secs;


    PVR_RECORDING tag;
    memset(&tag, 0, sizeof(PVR_RECORDING));
    strncpy(tag.strRecordingId, recording.strRecordingId.c_str(), sizeof(tag.strRecordingId));
    strncpy(tag.strTitle, recording.strTitle.c_str(), sizeof(tag.strTitle));
    strncpy(tag.strStreamURL, recording.strStreamURL.c_str(), sizeof(tag.strStreamURL));
    strncpy(tag.strPlotOutline, recording.strPlotOutline.c_str(), sizeof(tag.strPlotOutline));
    strncpy(tag.strPlot, recording.strPlot.c_str(), sizeof(tag.strPlot));
    strncpy(tag.strChannelName, recording.strChannelName.c_str(), sizeof(tag.strChannelName));
    tag.recordingTime     = recording.startTime;
    tag.iDuration         = recording.iDuration;
    strncpy(tag.strDirectory, "/", sizeof(tag.strDirectory));   // unused

    PVR->TransferRecordingEntry(handle, &tag);

    m_iNumRecordings++; 
    iNumRecording++;
    m_recordings.push_back(recording);

    XBMC->Log(LOG_INFO, "%s loaded Recording entry '%s', start '%d', length '%d'", __FUNCTION__, tag.strTitle, recording.startTime, recording.iDuration);
  }

  XBMC->Log(LOG_INFO, "%s Loaded %u Recording Entries", __FUNCTION__, iNumRecording);

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR Dvb::DeleteRecording(const PVR_RECORDING &recinfo) 
{
  CStdString url;

  url.Format("%srec_list.html?aktion=delete_rec&recid=%s", m_strURL.c_str(), recinfo.strRecordingId);
  GetHttpXML(url);
  PVR->TriggerRecordingUpdate();

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR Dvb::UpdateTimer(const PVR_TIMER &timer)
{

  XBMC->Log(LOG_DEBUG, "%s timer channelid '%d'", __FUNCTION__, timer.iClientChannelUid);

  CStdString strTmp;
  CStdString strServiceReference = m_channels.at(timer.iClientChannelUid-1).strServiceReference.c_str();  

  unsigned int i=0;

  while (i<m_timers.size())
  {
	  if (m_timers.at(i).iClientIndex == timer.iClientIndex)
		  break;
	  else
		  i++;
  }

  DvbTimer &oldTimer = m_timers.at(i);
  CStdString strOldServiceReference = m_channels.at(oldTimer.iChannelId-1).strServiceReference.c_str();  
  XBMC->Log(LOG_DEBUG, "%s old timer channelid '%d'", __FUNCTION__, oldTimer.iChannelId);

  int iDisabled = 0;
  if (timer.state == PVR_TIMER_STATE_CANCELLED)
    iDisabled = 1;

  strTmp.Format("web/timerchange?sRef=%s&begin=%d&end=%d&name=%s&eventID=&description=%s&tags=&afterevent=3&eit=0&disabled=%d&justplay=0&repeated=%d&channelOld=%s&beginOld=%d&endOld=%d&deleteOldOnSave=1", URLEncodeInline(strServiceReference.c_str()), timer.startTime, timer.endTime, URLEncodeInline(timer.strTitle), URLEncodeInline(timer.strSummary), iDisabled, timer.iWeekdays, URLEncodeInline(strOldServiceReference.c_str()), oldTimer.startTime, oldTimer.endTime  );
  
  TimerUpdates();

  return PVR_ERROR_NO_ERROR;
}

bool Dvb::GetInt(XMLNode xRootNode, const char* strTag, int& iIntValue)
{
  XMLNode xNode = xRootNode.getChildNode(strTag );
  if (xNode.isEmpty())
     return false;
  iIntValue = atoi(xNode.getText());
  return true;
}

bool Dvb::GetBoolean(XMLNode xRootNode, const char* strTag, bool& bBoolValue)
{
  XMLNode xNode = xRootNode.getChildNode(strTag );
  if (xNode.isEmpty()) 
    return false;

  CStdString strEnabled = xNode.getText();

  strEnabled.ToLower();
  if (strEnabled == "off" || strEnabled == "no" || strEnabled == "disabled" || strEnabled == "false" || strEnabled == "0" )
    bBoolValue = false;
  else
  {
    bBoolValue = true;
    if (strEnabled != "on" && strEnabled != "yes" && strEnabled != "enabled" && strEnabled != "true")
      return false; // invalid bool switch - it's probably some other string.
  }
  return true;
}

bool Dvb::GetString(XMLNode xRootNode, const char* strTag, CStdString& strStringValue)
{
  XMLNode xNode = xRootNode.getChildNode(strTag );
  if (!xNode.isEmpty())
  {
    strStringValue = xNode.getText();
    return true;
  }
  strStringValue.Empty();
  return false;
}


PVR_ERROR Dvb::GetChannelGroups(ADDON_HANDLE handle)
{
  for(unsigned int iTagPtr = 0; iTagPtr < m_groups.size(); iTagPtr++)
  {
    PVR_CHANNEL_GROUP tag;
    memset(&tag, 0 , sizeof(PVR_CHANNEL_GROUP));

    tag.bIsRadio     = false;
    strncpy(tag.strGroupName, m_groups[iTagPtr].strGroupName.c_str(), sizeof(tag.strGroupName));

    PVR->TransferChannelGroup(handle, &tag);
  }

  return PVR_ERROR_NO_ERROR;
}


unsigned int Dvb::GetNumChannelGroups() {
  return m_iNumChannelGroups;
}

PVR_ERROR Dvb::GetChannelGroupMembers(ADDON_HANDLE handle, const PVR_CHANNEL_GROUP &group)
{
  XBMC->Log(LOG_DEBUG, "%s - group '%s'", __FUNCTION__, group.strGroupName);
  CStdString strTmp = group.strGroupName;
  for (unsigned int i = 0;i<m_channels.size();  i++) 
  {
    DvbChannel &myChannel = m_channels.at(i);
    if (!strTmp.compare(myChannel.strGroupName)) 
    {
      PVR_CHANNEL_GROUP_MEMBER tag;
      memset(&tag,0 , sizeof(PVR_CHANNEL_GROUP_MEMBER));

      strncpy(tag.strGroupName, group.strGroupName, sizeof(tag.strGroupName));
      tag.iChannelUniqueId = myChannel.iUniqueId;
      tag.iChannelNumber   = myChannel.iChannelNumber;

      XBMC->Log(LOG_DEBUG, "%s - add channel %s (%d) to group '%s' channel number %d",
          __FUNCTION__, myChannel.strChannelName.c_str(), tag.iChannelUniqueId, group.strGroupName, myChannel.iChannelNumber);

      PVR->TransferChannelGroupMember(handle, &tag);
    }
  }
  return PVR_ERROR_NO_ERROR;
}

int Dvb::GetCurrentClientChannel(void) 
{
  return m_iCurrentChannel;
}

const char* Dvb::GetLiveStreamURL(const PVR_CHANNEL &channelinfo)
{
  SwitchChannel(channelinfo);

  return m_channels.at(channelinfo.iUniqueId-1).strStreamURL.c_str();
}

bool Dvb::OpenLiveStream(const PVR_CHANNEL &channelinfo)
{
  XBMC->Log(LOG_INFO, "%s channel '%u'", __FUNCTION__, channelinfo.iUniqueId);

  if ((int)channelinfo.iUniqueId == m_iCurrentChannel)
    return true;

  return SwitchChannel(channelinfo);
}

void Dvb::CloseLiveStream(void) 
{
  m_iCurrentChannel = -1;
}

bool Dvb::SwitchChannel(const PVR_CHANNEL &channel)
{
  return true;
}

bool Dvb::GetDeviceInfo()
{
  CStdString url; 
  url.Format("%s%s", m_strURL.c_str(), "api/version.html"); 

  CStdString strXML;
  strXML = GetHttpXML(url);

  XMLResults xe;
  XMLNode xMainNode = XMLNode::parseString(strXML.c_str(), NULL, &xe);
  
  if(xe.error != 0)  {
    XBMC->Log(LOG_ERROR, "%s Unable to parse XML. Error: '%s' ", __FUNCTION__, XMLNode::getError(xe.error));
    return false;
  }

  XMLNode xNode = xMainNode.getChildNode("version");

  CStdString strTmp;;

  XBMC->Log(LOG_NOTICE, "%s - DeviceInfo", __FUNCTION__);

  // Get Version
  if (!xNode.getText()) {
    XBMC->Log(LOG_ERROR, "%s Could not parse version from result!", __FUNCTION__);
    return false;
  }
  m_strDVBViewerVersion = xNode.getText();
  XBMC->Log(LOG_NOTICE, "%s - Version: %s", __FUNCTION__, m_strDVBViewerVersion.c_str());

  return true;
}

PVR_ERROR Dvb::SignalStatus(PVR_SIGNAL_STATUS &signalStatus)
{
  unsigned short iSignalPos;
  CStdString strXML, url, signal="%</th>";

  snprintf(signalStatus.strAdapterStatus, sizeof(signalStatus.strAdapterStatus), "OK");
  url.Format("%s%s",  m_strURL.c_str(), "status.html?aktion=status"); 
  strXML = GetHttpXML(url);
  iSignalPos=strXML.find(signal);
  if (iSignalPos < strXML.size())
    signalStatus.iSignal = (int)(atoi(strXML.substr(iSignalPos-2, 2).c_str()) * 655.35);
  return PVR_ERROR_NO_ERROR;
}
