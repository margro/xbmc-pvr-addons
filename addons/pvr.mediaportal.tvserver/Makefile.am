#
# Makefile for the Mediaportal add-on for XBMC PVR
#
# See the README for copyright information and
# how to reach the author.
#

ADDONBINNAME    = XBMC_Mediaportal
ADDONNAME       = pvr.mediaportal.tvserver
LIBNAME         = libmediaportal-addon
lib_LTLIBRARIES = libmediaportal-addon.la

LIBS            = @abs_top_builddir@/lib/tinyxml/libtinyxml.la

include ../Makefile.include.am


INCLUDES_LIVE555=-I${srcdir}/src/lib/live555 \
                 -I${srcdir}/src/lib/live555/liveMedia/include \
                 -I${srcdir}/src/lib/live555/BasicUsageEnvironment/include \
                 -I${srcdir}/src/lib/live555/UsageEnvironment/include \
                 -I${srcdir}/src/lib/live555/groupsock/include

DEFINES_LIVE555=-DLIVE555 -DSOCKLEN_T=socklen_t -DBSD=1

INCLUDES+=-I${srcdir}/src $(INCLUDES_LIVE555)
DEFINES+=$(DEFINES_LIVE555)

libmediaportal_addon_la_SOURCES = src/Cards.cpp \
                                  src/channels.cpp \
                                  src/client.cpp \
                                  src/epg.cpp \
                                  src/GenreTable.cpp \
                                  src/pvrclient-mediaportal.cpp \
                                  src/recordings.cpp \
                                  src/Socket.cpp \
                                  src/timers.cpp \
                                  src/uri.cpp \
                                  src/utils.cpp \
                                  src/lib/tsreader/ChannelInfo.cpp \
                                  src/lib/tsreader/DeMultiplexer.cpp \
                                  src/lib/tsreader/DvbUtil.cpp \
                                  src/lib/tsreader/FileReader.cpp \
                                  src/lib/tsreader/MemoryBuffer.cpp \
                                  src/lib/tsreader/MemoryReader.cpp \
                                  src/lib/tsreader/MemorySink.cpp \
                                  src/lib/tsreader/MepoRTSPClient.cpp \
                                  src/lib/tsreader/MultiFileReader.cpp \
                                  src/lib/tsreader/PacketSync.cpp \
                                  src/lib/tsreader/PatParser.cpp \
                                  src/lib/tsreader/PidTable.cpp \
                                  src/lib/tsreader/PmtParser.cpp \
                                  src/lib/tsreader/Section.cpp \
                                  src/lib/tsreader/SectionDecoder.cpp \
                                  src/lib/tsreader/TSHeader.cpp \
                                  src/lib/tsreader/TSReader.cpp \
                                  src/lib/live555/BasicUsageEnvironment/BasicHashTable.cpp \
                                  src/lib/live555/BasicUsageEnvironment/BasicTaskScheduler.cpp \
                                  src/lib/live555/BasicUsageEnvironment/BasicTaskScheduler0.cpp \
                                  src/lib/live555/BasicUsageEnvironment/BasicUsageEnvironment.cpp \
                                  src/lib/live555/BasicUsageEnvironment/BasicUsageEnvironment0.cpp \
                                  src/lib/live555/BasicUsageEnvironment/DelayQueue.cpp \
                                  src/lib/live555/UsageEnvironment/HashTable.cpp \
                                  src/lib/live555/UsageEnvironment/UsageEnvironment.cpp \
                                  src/lib/live555/UsageEnvironment/strDup.cpp \
                                  src/lib/live555/groupsock/GroupEId.cpp \
                                  src/lib/live555/groupsock/Groupsock.cpp \
                                  src/lib/live555/groupsock/GroupsockHelper.cpp \
                                  src/lib/live555/groupsock/IOHandlers.cpp \
                                  src/lib/live555/groupsock/NetAddress.cpp \
                                  src/lib/live555/groupsock/NetInterface.cpp \
                                  src/lib/live555/groupsock/inet.c \
                                  src/lib/live555/liveMedia/Base64.cpp \
                                  src/lib/live555/liveMedia/BasicUDPSource.cpp \
                                  src/lib/live555/liveMedia/DigestAuthentication.cpp \
                                  src/lib/live555/liveMedia/FramedFilter.cpp \
                                  src/lib/live555/liveMedia/FramedSource.cpp \
                                  src/lib/live555/liveMedia/Locale.cpp \
                                  src/lib/live555/liveMedia/MPEG2TransportStreamFramer.cpp \
                                  src/lib/live555/liveMedia/Media.cpp \
                                  src/lib/live555/liveMedia/MediaSession.cpp \
                                  src/lib/live555/liveMedia/MediaSink.cpp \
                                  src/lib/live555/liveMedia/MediaSource.cpp \
                                  src/lib/live555/liveMedia/MultiFramedRTPSink.cpp \
                                  src/lib/live555/liveMedia/MultiFramedRTPSource.cpp \
                                  src/lib/live555/liveMedia/RTCP.cpp \
                                  src/lib/live555/liveMedia/RTPInterface.cpp \
                                  src/lib/live555/liveMedia/RTPSink.cpp \
                                  src/lib/live555/liveMedia/RTPSource.cpp \
                                  src/lib/live555/liveMedia/RTSPClient.cpp \
                                  src/lib/live555/liveMedia/RTSPCommon.cpp \
                                  src/lib/live555/liveMedia/SimpleRTPSink.cpp \
                                  src/lib/live555/liveMedia/SimpleRTPSource.cpp \
                                  src/lib/live555/liveMedia/our_md5.c \
                                  src/lib/live555/liveMedia/our_md5hl.c \
                                  src/lib/live555/liveMedia/rtcp_from_spec.c
libmediaportal_addon_la_LDFLAGS = @TARGET_LDFLAGS@

