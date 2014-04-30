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

## Binary installation
Download the correct plugin for your platform from the [releases
page](https://github.com/bbcrd/bbc-vamp-plugins/releases) and extract the
contents into the [Vamp system plugin
folder](http://vamp-plugins.org/download.html#install).

## Installation from source

The following instructions are written for Linux and OS X systems. For
installing on Windows, please follow the instructions where possible, but refer
to [this
document](http://code.soundsoftware.ac.uk/projects/vamp-plugin-sdk/wiki/mtp2)
when compiling.

#### Dependencies
You will need a working C++ compiler. On OS X this is done by installing
[XCode](http://developer.apple.com/xcode/). On Debian/Ubuntu, this can be done
with the following command:

    sudo apt-get install build-essential

The Vamp SDK is required to compile the plugins. Download the main SDK from
<http://vamp-plugins.org/develop.html> and extract the contents. Follow the
installation instructions in build/README.linux or build/README.osx.

#### Configuration

Simply edit 'Makefile.inc' and set `VAMP_SDK_DIR` to the directory containing
the extracted and compiled Vamp SDK.

#### Compile

With the project's folder as your working directory, compile the plugin using
the following command for linux:

    make -f Makefile.linux

or the following command for OSX:

    make -f Makefile.osx

then install it by moving the plugin, category and RDF files to the system Vamp
plugin folder (see [here](http://vamp-plugins.org/download.html#install) for
defaults). Use the following command for linux:

    mv bbc-vamp-plugins.so bbc-vamp-plugins.cat bbc-vamp-plugins.n3 /usr/local/lib/vamp/

or the following command for OSX:

    mv bbc-vamp-plugins.dylib bbc-vamp-plugins.cat bbc-vamp-plugins.n3 /Library/Audio/Plug-Ins/Vamp/

#### Documentation

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
