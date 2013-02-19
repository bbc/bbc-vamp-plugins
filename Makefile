
# Edit this to the location of the Vamp plugin SDK, relative to your project directory
VAMP_SDK_DIR := ../vamp-plugin-sdk-2.4

PLUGIN_LIBRARY_NAME := bbc-vamp-plugins

SOURCES := Energy.cpp \
           Intensity.cpp \
           SpectralFlux.cpp \
           Rhythm.cpp \
           SpectralContrast.cpp \
           SpeechMusicSegmenter.cpp \
           plugins.cpp

HEADERS := Energy.h \
           Intensity.h \
           SpectralFlux.h \
           Rhythm.h \
           SpectralContrast.h \
           SpeechMusicSegmenter.h

## Uncomment these for an OS/X universal binary

# OSX_SDK    := /Developer/SDKs/MacOSX10.6.sdk
# CFLAGS     := -O3 -isysroot $(OSX_SDK) -arch i386 -arch x86_64 -I$(VAMP_SDK_DIR)
# CXXFLAGS   := $(CFLAGS)
# PLUGIN_EXT := .dylib
# LDFLAGS    := -isysroot $(OSX_SDK) -arch i386 -arch x86_64 -dynamiclib $(VAMP_SDK_DIR)/libvamp-sdk.a -exported_symbols_list vamp-plugin.list -install_name $(PLUGIN_LIBRARY_NAME)$(PLUGIN_EXT)

##  Uncomment these for Linux using the standard tools:

# CXXFLAGS   := -I$(VAMP_SDK_DIR) -fPIC
# PLUGIN_EXT := .so
# LDFLAGS    := -shared -Wl,-soname=$(PLUGIN) $(VAMP_SDK_DIR)/libvamp-sdk.a -Wl,--version-script=vamp-plugin.map

##  All of the above

PLUGIN_EXT  ?= .so
PLUGIN      ?= $(PLUGIN_LIBRARY_NAME)$(PLUGIN_EXT)
CXX         ?= g++
CC          ?= gcc

OBJECTS := $(SOURCES:.cpp=.o)
OBJECTS := $(OBJECTS:.c=.o)

$(PLUGIN):	$(OBJECTS)
		$(CXX) -o $@ $^ $(LDFLAGS)

clean:		
		rm $(OBJECTS)

distclean:	clean
		rm $(PLUGIN)
