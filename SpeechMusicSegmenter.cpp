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
#include "SpeechMusicSegmenter.h"
/// @cond

SpeechMusicSegmenter::SpeechMusicSegmenter(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_blockSize(0),
    m_nframes(0),
    resolution(256),
    margin(14),
    change_threshold(0.0781),
    decision_threshold(0.2734),
    min_music_length(0)
    // Also be sure to set your plugin parameters (presumably stored
    // in member variables) to their default values here -- the host
    // will not do that for you
{
}

SpeechMusicSegmenter::~SpeechMusicSegmenter()
{
}

string
SpeechMusicSegmenter::getIdentifier() const
{
    return "bbc-speechmusic-segmenter";
}

string
SpeechMusicSegmenter::getName() const
{
    return "Speech/Music segmenter";
}

string
SpeechMusicSegmenter::getDescription() const
{
    return "A simple speech/music segmenter";
}

string
SpeechMusicSegmenter::getMaker() const
{
    return "BBC";
}

int
SpeechMusicSegmenter::getPluginVersion() const
{
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string
SpeechMusicSegmenter::getCopyright() const
{
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "(c) 2011 British Broadcasting Corporation";
}

SpeechMusicSegmenter::InputDomain
SpeechMusicSegmenter::getInputDomain() const
{
    return TimeDomain;
}

size_t
SpeechMusicSegmenter::getPreferredBlockSize() const
{
    return 0; // 0 means "I can handle any block size"
}

size_t 
SpeechMusicSegmenter::getPreferredStepSize() const
{
    return 0; // 0 means "anything sensible"; in practice this
              // means the same as the block size for TimeDomain
              // plugins, or half of it for FrequencyDomain plugins
}

size_t
SpeechMusicSegmenter::getMinChannelCount() const
{
    return 1;
}

size_t
SpeechMusicSegmenter::getMaxChannelCount() const
{
    return 1;
}

SpeechMusicSegmenter::ParameterList
SpeechMusicSegmenter::getParameterDescriptors() const
{
    ParameterList list;

    // If the plugin has no adjustable parameters, return an empty
    // list here (and there's no need to provide implementations of
    // getParameter and setParameter in that case either).

    // Note that it is your responsibility to make sure the parameters
    // start off having their default values (e.g. in the constructor
    // above).  The host needs to know the default value so it can do
    // things like provide a "reset to default" function, but it will
    // not explicitly set your parameters to their defaults for you if
    // they have not changed in the mean time.

    ParameterDescriptor d;
    d.identifier = "resolution";
    d.name = "Resolution";
    d.description = "Resolution (in number of frames) at which segment boundaries can be found";
    d.unit = "";
    d.minValue = 1;
    d.maxValue = 1024;
    d.defaultValue = 256;
    d.isQuantized = true;
    d.quantizeStep = 1;
    list.push_back(d);

    ParameterDescriptor d21;
    d21.identifier = "change_threshold";
    d21.name = "Change threshold";
    d21.description = "Threshold the detection function needs to exceed for a corresponding segment change to be taken into account";
    d21.unit = "";
    d21.minValue = 0;
    d21.maxValue = 1;
    d21.defaultValue = 0.0781;
    d21.isQuantized = false;
    list.push_back(d21);

    ParameterDescriptor d22;
    d22.identifier = "decision_threshold";
    d22.name = "Decision threshold";
    d22.description = "Mean of detection function above threshold: speech; Mean of detection function below threshold: music";
    d22.unit = "";
    d22.minValue = 0;
    d22.maxValue = 1;
    d22.defaultValue = 0.2734;
    d22.isQuantized = false;
    list.push_back(d22);

    ParameterDescriptor d23;
    d23.identifier = "min_music_length";
    d23.name = "Minimum music segment length";
    d23.description = "The minimum length of a music segment";
    d23.unit = "";
    d23.minValue = 0;
    d23.maxValue = 100;
    d23.defaultValue = 0;
    d23.isQuantized = false;
    list.push_back(d23);

    ParameterDescriptor d3;
    d3.identifier = "margin";
    d3.name = "Margin";
    d3.description = "Margin around mean ZCR under which no value is taken into account in the detection function";
    d3.unit = "";
    d3.minValue = 0;
    d3.defaultValue = 14;
    d3.maxValue = 50;
    d3.isQuantized = false;
    list.push_back(d3);

    return list;
}

float
SpeechMusicSegmenter::getParameter(string identifier) const
{
    if (identifier == "resolution") {
        return resolution;
    }

    if (identifier == "change_threshold") {
        return change_threshold;
    }

    if (identifier == "decision_threshold") {
        return decision_threshold;
    }

    if (identifier == "min_music_length") {
        return min_music_length;
    }

    if (identifier == "margin") {
        return margin;
    }

    std::cerr << "WARNING: SegmenterPlugin::getParameter: unknown parameter \""
              << identifier << "\"" << std::endl;
    return 0.0;
}

void
SpeechMusicSegmenter::setParameter(string identifier, float value) 
{
    if (identifier == "resolution") {
        resolution = value;
        return;
    }

    if (identifier == "change_threshold") {
        change_threshold = value;
        return;
    }

    if (identifier == "decision_threshold") {
        decision_threshold = value;
        return;
    }

    if (identifier == "min_music_length") {
        min_music_length = value;
        return;
    }

    if (identifier == "margin") {
        margin = value;
        return;
    }

    std::cerr << "WARNING: SegmenterPlugin::setParameter: unknown parameter \""
              << identifier << "\"" << std::endl;
}

SpeechMusicSegmenter::ProgramList
SpeechMusicSegmenter::getPrograms() const
{
    ProgramList list;

    // If you have no programs, return an empty list (or simply don't
    // implement this function or getCurrentProgram/selectProgram)

    return list;
}

string
SpeechMusicSegmenter::getCurrentProgram() const
{
    return ""; // no programs
}

void
SpeechMusicSegmenter::selectProgram(string name)
{
}

SpeechMusicSegmenter::OutputList
SpeechMusicSegmenter::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor segmentation;
    segmentation.identifier = "segmentation";
    segmentation.name = "Segmentation";
    segmentation.description = "Segmentation";
    segmentation.unit = "segment-type";
    segmentation.hasFixedBinCount = true;
    segmentation.binCount = 1;
    segmentation.hasKnownExtents = true;
    segmentation.minValue = 0;
    segmentation.maxValue = 2;
    segmentation.isQuantized = true;
    segmentation.quantizeStep = 1;
    segmentation.sampleType = OutputDescriptor::VariableSampleRate;
    segmentation.sampleRate = m_inputSampleRate / getPreferredStepSize();

    OutputDescriptor skewness;
    skewness.identifier = "skewness";
    skewness.name = "Detection function";
    skewness.description = "Detection function";
    skewness.unit = "segment-type";
    skewness.hasFixedBinCount = true;
    skewness.binCount = 1;
    skewness.hasKnownExtents = true;
    skewness.minValue = 0;
    skewness.maxValue = 2;
    skewness.isQuantized = true;
    skewness.quantizeStep = 1;
    skewness.sampleType = OutputDescriptor::VariableSampleRate;
    skewness.sampleRate = m_inputSampleRate / getPreferredStepSize();

    list.push_back(segmentation);
    list.push_back(skewness);

    return list;
}

bool
SpeechMusicSegmenter::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    // Real initialisation work goes here!
    m_blockSize = blockSize;

    return true;
}

void
SpeechMusicSegmenter::reset()
{
    // Clear buffers, reset stored values, etc
    m_zcr.erase(m_zcr.begin(), m_zcr.end());
}

SpeechMusicSegmenter::FeatureSet
SpeechMusicSegmenter::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    // Extracting ZCR per frame
    size_t i = 1;
    double zc = 0.0;

    while (i < m_blockSize) {
        if ((inputBuffers[0][i] * inputBuffers[0][i - 1]) < 0) zc += 1;
        i += 1;
    }
    zc /= (m_blockSize - 1);
    m_zcr.push_back(zc);

    m_nframes += 1;

    return FeatureSet();
}

SpeechMusicSegmenter::FeatureSet
SpeechMusicSegmenter::getRemainingFeatures()
{
    FeatureSet features;
    vector<double> skewness = getSkewnessFunction();
    double old_mean = 0.0;
    int feature_size = 0;
    for (int n = 0; n < m_nframes / resolution; n++) {
        double mean = 0.0;
        for (int i = 0; i < resolution; i++) {
            mean += skewness[n * resolution + i];
        }
        mean /= resolution;
        if ((n > 0 && std::abs(mean - old_mean) > change_threshold) || n == 0) {
            Feature feature; feature.hasTimestamp = true;
            feature.timestamp = Vamp::RealTime::frame2RealTime((n * resolution + resolution / 2.0) * m_blockSize, static_cast<unsigned int>(m_inputSampleRate));
            vector<float> floatval;
            floatval.push_back(mean);
            if (mean < decision_threshold) {
                feature.label = "Music";
            } else {
                feature.label = "Speech";
            }
            feature.values = floatval;
            if (feature_size == 0 || (feature_size > 0 && feature.label != features[0].back().label)) {
                if (feature_size > 0 && features[0].back().label == "Music" && 
                    (feature.timestamp - features[0].back().timestamp < Vamp::RealTime::fromSeconds(min_music_length))
                ) {
                    features[0].pop_back();
                    feature_size -= 1;
                } else {
                    if (feature_size == 0) feature.timestamp = Vamp::RealTime::fromSeconds(0);
                    features[0].push_back(feature);
                    feature_size += 1;
                }
            }
        }
        old_mean = mean;
    }

    for (unsigned int n = 1; n < skewness.size(); n++) {
        Feature feature;
        feature.hasTimestamp = true;
        feature.timestamp = Vamp::RealTime::frame2RealTime(n * m_blockSize, static_cast<unsigned int>(m_inputSampleRate));
        vector<float> floatval;
        floatval.push_back(skewness[n]);
        feature.values = floatval;
        features[1].push_back(feature);
    }

    return features;
}

vector<double>
SpeechMusicSegmenter::getSkewnessFunction()
{
    double threshold_d = margin / 1000;
    vector<double> skewness;

    double mean_zcr;
    for (int n = 0; n < m_nframes; n++) {
        int i = 0;
        mean_zcr = 0.0;
        while (i < resolution) {
            mean_zcr += m_zcr[n + i]; 
            i += 1;
        }
        mean_zcr /= resolution;
        i = 0;
        int above = 0;
        int below = 0;
        while (i < resolution) {
            if (m_zcr[n + i] > (mean_zcr + threshold_d)) above += 1;
            if (m_zcr[n + i] < (mean_zcr - threshold_d)) below += 1;
            i += 1;
        } 
        double skewness_value = below - above;
        skewness_value /= resolution;
        skewness.push_back(skewness_value);
    }
    return skewness;
}
/// @endcond
