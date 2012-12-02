#
# Makefile for the VDR XVDR add-on for XBMC PVR
#
# See the README for copyright information and
# how to reach the author.
#

ADDONBINNAME    = XBMC_VDR_xvdr
ADDONNAME       = pvr.vdr.xvdr
LIBNAME         = libvdrxvdr-addon
lib_LTLIBRARIES = libvdrxvdr-addon.la

include ../Makefile.include.am

INCLUDES+=-Ilibxvdr/include

libvdrxvdr_addon_la_SOURCES = xvdr/XBMCAddon.cpp \
                              xvdr/XBMCCallbacks.cpp \
                              xvdr/XBMCChannelScan.cpp \
                              xvdr/XBMCSettings.cpp \
                              libxvdr/src/clientinterface.cpp \
                              libxvdr/src/connection.cpp \
                              libxvdr/src/dataset.cpp \
                              libxvdr/demux.cpp \
                              libxvdr/src/iso639.cpp \
                              libxvdr/src/msgtpacket.cpp \
                              libxvdr/src/os-config.cpp \
                              libxvdr/src/session.cpp \
                              libxvdr/src/thread.cpp

