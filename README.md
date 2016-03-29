BBC Vamp plugin collection
===

## Introduction

This is a collection of audio feature extraction algorithms written in the
[Vamp plugin format](http://vamp-plugins.org) by BBC Research and Development.

Below is a list of plugins and their outputs. Detailed information about each
of the features and the algorithms used is contained in the full documentation,
which is available to download from the [releases
page](https://github.com/bbcrd/bbc-vamp-plugins/releases).

* __Peaks__
  1. Peak/trough
* __Energy__
  1. RMS energy
  1. RMS energy delta
  1. Moving average
  1. Dip probability
  1. Low energy ratio
* __Intensity__
  1. Intensity
  1. Intensity ratio
* __Rhythm__
  1. Onset detection curve
  1. Moving average of the onset detection curve
  1. Difference between 1 and 2
  1. Onsets
  1. Average onset frequency
  1. Rhythm strength
  1. Autocorrelation
  1. Mean correlation peak
  1. Peak valley ratio
  1. Tempo
* __Spectral Contrast__
  1. Valleys
  1. Peaks
  1. Mean
* __Spectral Flux__
  1. Spectral flux
* __Speech/music segmenter__
  1. Segmentation
  1. Detection function

## Binary installation (recommended)
Download the correct plugin for your platform from the [releases
page](https://github.com/bbcrd/bbc-vamp-plugins/releases) and extract the
contents into the [Vamp system plugin
folder](http://vamp-plugins.org/download.html#install).

## Installation from source

### Linux (Ubuntu/Debian)

Firstly you will need a C++ compiler:

    sudo apt-get install build-essential

Download the Vamp SDK

    wget https://code.soundsoftware.ac.uk/attachments/download/1514/vamp-plugin-sdk-2.6.tar.gz
    tar xvf vamp-plugin-sdk-2.6.tar.gz
    cd vamp-plugin-sdk-2.6

Compile the SDK

    ./configure
    make sdk

In Makefile.inc, set VAMP\_SDK\_DIR to the SDK path

    cd /path/to/bbc-vamp-plugins
    nano Makefile.inc

Build the plugin

    make -f Makefile.linux

Install the plugin

    mv bbc-vamp-plugins.so bbc-vamp-plugins.cat bbc-vamp-plugins.n3 /usr/local/lib/vamp/

### OS/X

Install [XCode](http://developer.apple.com/xcode/) if you haven't already.

Download the Vamp SDK

    wget https://code.soundsoftware.ac.uk/attachments/download/1514/vamp-plugin-sdk-2.6.tar.gz
    tar xvf vamp-plugin-sdk-2.6.tar.gz
    cd vamp-plugin-sdk-2.6

Compile the SDK

    make -f build/Makefile.osx sdk

In Makefile.inc, set VAMP\_SDK\_DIR to the SDK path

    cd /path/to/bbc-vamp-plugins
    nano Makefile.inc

Build the plugin

    make -f Makefile.osx

Install the plugin

    mv bbc-vamp-plugins.dylib bbc-vamp-plugins.cat bbc-vamp-plugins.n3 /Library/Audio/Plug-Ins/Vamp/

### Windows (cross-compiled)

To compile a Windows binary from a Linux environment, install MinGW:

    sudo apt-get install mingw-w64

Download the Vamp SDK

    wget https://code.soundsoftware.ac.uk/attachments/download/1514/vamp-plugin-sdk-2.6.tar.gz
    tar xvf vamp-plugin-sdk-2.6.tar.gz
    cd vamp-plugin-sdk-2.6

Compile the SDK

    make -f build/Makefile.mingw32 sdk

In Makefile.inc, set VAMP\_SDK\_DIR to the SDK path

    cd /path/to/bbc-vamp-plugins
    nano Makefile.inc

Build the plugin

    make -f Makefile.mingw32

Install the plugin by putting bbc-vamp-plugins.dll, bbc-vamp-plugins.cat and
bbc-vamp-plugins.n3 in the [Vamp system plugin
folder](http://vamp-plugins.org/download.html#install).

## Documentation

To generate the documentation, install [Doxygen](http://www.doxygen.org) and
run the following command from the src folder. The documents will appear in
doc/html/index.html

    doxygen ../bbc-vamp-plugins.doxyfile

## Usage

The two primary programs which use Vamp plugins are
[sonic annotator](http://www.omras2.org/sonicannotator) and
[sonic visualiser](http://www.sonicvisualiser.org/).

Below is an example of how to extract the tempo of an audio file using sonic
annotator and default settings:

    sonic-annotator -d vamp:bbc-vamp-plugins:bbc-rhythm:tempo audio.wav -w csv --csv-stdout

## Further reading

* [Vamp plugins](http://vamp-plugins.org)
* [BBC R&D](http://www.bbc.co.uk/rd)
* [QMUL Centre for Digital Music](http://www.elec.qmul.ac.uk/digitalmusic/)

## Licensing terms and authorship

Please refer to the 'COPYING' and 'AUTHORS' files.
