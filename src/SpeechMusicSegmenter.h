/**
 * BBC Vamp plugin collection
 *
 * Copyright (c) 2011-2013 British Broadcasting Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _SPEECHMUSIC_PLUGIN_H_
#define _SPEECHMUSIC_PLUGIN_H_

#include <vector>
#include <vamp-sdk/Plugin.h>
#include <math.h>
#include <cmath>

using std::string;
using std::vector;

/*!
 * \brief Calculates boundaries between speech and music
 *
 * \section Outputs
 * \par Segmentation
 * Impulses at the boundary points.
 * \par Detection function
 * Function used to find boundaries.
 *
 * \section Parameters
 * \par Resolution
 * The number of frames defining the window at which candidate changes might
 * be found (default = 256)
 * \par Change threshold
 * The threshold of skewness difference at which a candidate change will be marked
 * (default = 0.0781)
 * \par Decision threshold
 * The threshold used to classify segments as speech or music (default = 0.2734)
 * \par Margin
 * A parameter for the generation of the ZCR skewness (margin around mean ZCR where
 * no ZCR samples will be taken into account) (default = 14)
 * \par Minimum music segment length
 * Music segments that are shorter than this minimum length will be dismissed
 * (default = 0)
 *
 * \section Description
 *
 * This Vamp plugin is heavily inspired by the approach described in [1].
 *
 * The algorithm works as follows:
 *
 * -# Measure the skewness of the distribution of zero-crossing rate across the audio file;
 * -# Find points at which this distribution changes drastically;
 * -# For each candidate change point found, classify the corresponding segment as follows:
 *     - Mean skewness > threshold: speech
 *     - Mean skewness < threshold: music
 * -# If the segment has the same type with the previous one, merge it with
 * the previous one.
 *
 * This is a very early prototype, so not very accurate. It is relatively fast
 * (around 1s to process a 20 minute file).
 *
 * \section References
 * [1] <i>J. Saunders, "Real-time discrimination of broadcast speech/music,"
 * IEEE International Conference on Acoustics, Speech, and Signal Processing,
 * vol.2, pp.993-999, 7-10 May 1996</i>
 */
class SpeechMusicSegmenter : public Vamp::Plugin
{
public:
    /// @cond
    SpeechMusicSegmenter(float inputSampleRate);
    virtual ~SpeechMusicSegmenter();

    string getIdentifier() const;
    string getName() const;
    string getDescription() const;
    string getMaker() const;
    int getPluginVersion() const;
    string getCopyright() const;

    InputDomain getInputDomain() const;
    size_t getPreferredBlockSize() const;
    size_t getPreferredStepSize() const;
    size_t getMinChannelCount() const;
    size_t getMaxChannelCount() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(string identifier) const;
    void setParameter(string identifier, float value);

    ProgramList getPrograms() const;
    string getCurrentProgram() const;
    void selectProgram(string name);

    OutputList getOutputDescriptors() const;

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();
    vector<double> getSkewnessFunction();
    /// @endcond

protected:
    /// @cond
    size_t m_blockSize;
    /// @endcond
    vector<double> m_zcr;
    int m_nframes;
    int resolution;
    double margin;
    double change_threshold;
    double decision_threshold;
    double min_music_length;
};



#endif
