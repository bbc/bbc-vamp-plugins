# Edit this to the location of the Vamp plugin SDK, relative to your project directory
VAMP_SDK_DIR := ../vamp-plugin-sdk-2.4

PLUGIN_LIBRARY_NAME := bbc-vamp-plugins

SOURCES := src/Energy.cpp \
           src/Intensity.cpp \
           src/SpectralFlux.cpp \
           src/Rhythm.cpp \
           src/SpectralContrast.cpp \
           src/SpeechMusicSegmenter.cpp \
           src/Peaks.cpp \
           src/plugins.cpp

HEADERS := src/Energy.h \
           src/Intensity.h \
           src/SpectralFlux.h \
           src/Rhythm.h \
           src/SpectralContrast.h \
           src/SpeechMusicSegmenter.h \
           src/Peaks.h
