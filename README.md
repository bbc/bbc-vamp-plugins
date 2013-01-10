BBC Vamp plugin collection
===

## Introduction
This is a collection of audio feature extraction algorithms written in the [Vamp
plugin format](http://vamp-plugins.org). They were written by BBC Research and
Development as part of the collaborative Making Musical Moods Metadata project.

Below is a list of plugins and their outputs. Detailed information about each of
the features and the algorithms used is available in the full documentation
(see below).
#####Energy
1. RMS energy
1. Low energy ratio
#####Intensity
1. Intensity
1. Intensity ratio
#####Rhythm
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
#####Spectral Contrast
1. Valleys
1. Peaks
1. Mean
#####Spectral flux
1. Spectral flux

## Installation
The following instructions are written for Linux and OS X systems. For installing
on Windows, please follow the instructions where possible, but refer to
[this document](http://code.soundsoftware.ac.uk/projects/vamp-plugin-sdk/wiki/mtp2)
when compiling.
####Dependencies
You will need a working C++ compiler. On OS X this is done by installing
[XCode](http://developer.apple.com/xcode/). On Debian/Ubuntu, this can be done
with the following command:

    sudo apt-get install build-essential

The Vamp SDK is required to compile the plugins. Download the main SDK from
<http://vamp-plugins.org/develop.html> and extract the contents. Follow the
installation instructions in build/README.linux or build/README.osx.

####Configuration
Edit Makefile and set `VAMP_SDK_DIR` to the SDK installation directory.

####Compile
With the plugin's folder as your working directory, run the following command:

    make

then install it by moving the plugin to the Vamp plugin folder (see
[here](http://vamp-plugins.org/download.html#install) for defaults):

    mv bbcrd-vamp-plugins.so /usr/local/lib/vamp

####Documentation
To generate the documentation, install [Doxygen](http://www.doxygen.org) and
run the following command. The documents will appear in doc/html/index.html

    doxygen bbcrd-vamp-plugins.doxyfile

## Usage
The two primary programs which use Vamp plugins are
[sonic annotator](http://www.omras2.org/sonicannotator) and
[sonic visualiser](http://www.sonicvisualiser.org/).

Below is an example of how to extract the tempo of an audio file using sonic
annotator and default settings:

    sonic-annotator -d vamp:bbcrd-vamp-plugins:bbcrd-rhythm:tempo audio.wav -w csv --csv-stdout

## Further reading
* [Vamp plugins](http://vamp-plugins.org)
* [BBC R&D](http://www.bbc.co.uk/rd)
* [QMUL Centre for Digital Music](http://www.elec.qmul.ac.uk/digitalmusic/)

## Legal
This code is released under the
[GNU GPLv3](http://www.gnu.org/licenses/gpl-3.0.html) licence.

## Contact
Please direct all queries to chris.baume@bbc.co.uk.
