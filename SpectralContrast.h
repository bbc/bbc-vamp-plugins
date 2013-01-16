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
#ifndef _CONTRAST_H_
#define _CONTRAST_H_

#include <cmath>
#include <complex>
#include <algorithm>
#include <vector>
#include <vamp-sdk/Plugin.h>

using std::string;
using std::vector;
using std::complex;
using std::abs;

/*!
 * \brief Calculates the peak and valleys of the spectral contrast feature
 *
 * \section Outputs
 * \par Valleys
 * The valley of each frequency sub-band
 * \par Peaks
 * The peak of each frequency sub-band
 * \par Mean
 * The mean of each frequency sub-band
 *
 * \section Parameters
 * \par Alpha
 * Ratio of FFT bins used to find the peak/valley in each sub-band (default = 0.02)
 * \par Sub-bands
 * The number of sub-bands to use. (default = 7)
 *
 * \section Description
 *
 * This simple algorithm, taken from [1], divides a signal into N sub-bands and
 * sorts the FFT bins in each sub-band by magnitude. The peak and valley are
 * found by taking a proportion (defined as alpha) of FFT bins from the
 * top/bottom of the sorted bins and finding the mean of those. The mean of all
 * the FFT bins in each sub-band are also calculated. The 'spectral contrast'
 * can be found by subtracting the valley from the peak in each sub-band,
 * although this isn't calculated in the plugin.
 *
 * [1] Jiang, D.-N., Lu, L., & Zhang, H.-J. (2002). Music type classification
 * by spectral contrast feature. IEEE International Conference on Multimedia
 * and Expo (pp. 113–116).
 *
 * Thanks to Erik Schmidt at Drexel for providing a reference MATLAB implementation.
 */
class SpectralContrast : public Vamp::Plugin
{
public:
    /// @cond
    SpectralContrast(float inputSampleRate);
    virtual ~SpectralContrast();
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
    void setParameter(string identifier,
    		            float value);
    ProgramList getPrograms() const;
    string getCurrentProgram() const;
    void selectProgram(string name);
    OutputList getOutputDescriptors() const;
    bool initialise(size_t channels,
    		         size_t stepSize,
    		         size_t blockSize);
    void reset();
    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);
    FeatureSet getRemainingFeatures();
    void calculateBandFreqs();
    /// @endcond

protected:
    /// @cond
    int m_blockSize, m_stepSize;
    float m_sampleRate;
    /// @endcond

    float alpha;          /*!< Alpha parameter of spectral contrast algorithm*/
    int numBands;         /*!< Number of sub-bands to use */
    float *bandHighFreq;  /*!< Upper frequency range of each sub-band */
};

#endif
