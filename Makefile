
##  Skeleton Makefile for Vamp plugin builds using command-line tools.
##  This requires GNU make, which is what you get with OS/X, Linux, or
##  MinGW/Cygwin on Windows.
##
##  Rename this to Makefile, and edit as appropriate.
##  This Makefile WILL NOT WORK until you have edited it as described
##  below -- the Makefile as supplied does nothing useful at all!
##
##  Various sets of options are provided, commented out -- just uncomment
##  (remove the '#' characters for) the set that most closely resembles
##  your own situation, and adjust to taste.  Then run "gmake".
##
##  (For Windows builds using MS Visual Studio, start instead with the
##  VampExamplePlugins project found in the build directory of the SDK.)


# Edit this to the base name of your plugin library
#
PLUGIN_LIBRARY_NAME := vampdynamics

# Edit this to list the .cpp or .c files in your plugin project
#
PLUGIN_SOURCES := VampDynamics.cpp plugins.cpp

# Edit this to list the .h files in your plugin project
#
PLUGIN_HEADERS := VampDynamics.h

# Edit this to the location of the Vamp plugin SDK, relative to your
# project directory
#
VAMP_SDK_DIR := /home/chrisbau/builds/vamp-plugin-sdk-v2.2.1


## Uncomment these for an OS/X universal binary (PPC and 32- and
## 64-bit Intel) supporting 10.4 or newer (if you have the 10.4 SDK):

# CXX := g++-4.0
# CXXFLAGS := -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -arch i386 -arch x86_64 -arch ppc -I$(VAMP_SDK_DIR) -Wall -fPIC
# PLUGIN_EXT := .dylib
# LDFLAGS := $(CXXFLAGS) -dynamiclib -install_name $(PLUGIN) $(VAMP_SDK_DIR)/libvamp-sdk.a -exported_symbols_list vamp-plugin.list


## Uncomment these for an OS/X universal binary (PPC and 32- and
## 64-bit Intel) supporting 10.5 or newer (use this if you have the
## default 10.6 developer tools):

# CXXFLAGS := -isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -arch i386 -arch x86_64 -arch ppc -I$(VAMP_SDK_DIR) -Wall -fPIC
# PLUGIN_EXT := .dylib
# LDFLAGS := $(CXXFLAGS) -dynamiclib -install_name $(PLUGIN) $(VAMP_SDK_DIR)/libvamp-sdk.a -exported_symbols_list vamp-plugin.list


##  Uncomment these for Linux using the standard tools:

CXXFLAGS := -I$(VAMP_SDK_DIR) -Wall -fPIC
PLUGIN_EXT := .so
LDFLAGS := -shared -Wl,-soname=$(PLUGIN) $(VAMP_SDK_DIR)/libvamp-sdk.a -Wl,--version-script=vamp-plugin.map


##  Uncomment these for a cross-compile from Linux to Windows using MinGW:

# CXX := i586-mingw32msvc-g++
# CXXFLAGS := -I$(VAMP_SDK_DIR) -Wall 
# PLUGIN_EXT := .dll
# LDFLAGS := --static-libgcc -Wl,-soname=$(PLUGIN) -shared $(VAMP_SDK_DIR)/libvamp-sdk.a


##  Uncomment these for OpenSolaris using SunStudio compiler and GNU make:

# CXX := CC
# CXXFLAGS := -G -I$(VAMP_SDK_DIR) +w -KPIC
# PLUGIN_EXT := .so
# LDFLAGS := -G -h$(PLUGIN) $(VAMP_SDK_DIR)/libvamp-sdk.a -Qoption ld -Mvamp-plugin.map



##  All of the above

PLUGIN := $(PLUGIN_LIBRARY_NAME)$(PLUGIN_EXT)

PLUGIN_OBJECTS := $(PLUGIN_SOURCES:.cpp=.o)
PLUGIN_OBJECTS := $(PLUGIN_OBJECTS:.c=.o)

$(PLUGIN): $(PLUGIN_OBJECTS)
	   $(CXX) -o $@ $^ $(LDFLAGS)

$(PLUGIN_OBJECTS): $(PLUGIN_HEADERS)

clean:
	rm -f *.o

