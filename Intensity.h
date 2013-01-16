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
#ifndef _INTENSITY_H_
#define _INTENSITY_H_

#include <cmath>
#include <complex>
#include <vector>
#include <vamp-sdk/Plugin.h>

using std::string;
using std::vector;
using std::abs;
using std::complex;

/*!
 * \brief Calculates the intensity of a signal and the intensity ratio for a number of sub-bands
 *
 * \section Outputs
 * \par Intensity
 * The sum of the magnitude of the FFT bins.
 * \par Intensity ratio
 * The ratio between the intensity of each sub-band to the overall intensity.
 *
 * \section Parameters
 * \par Sub-bands
 * The number of sub-bands to use. (default = 7)
 *
 * \section Description
 *
 * The intensity features are based on those published in [1], section 3A.
 *
 * Firstly the signal is divided into \f$i\f$ sub-bands with the following frequency ranges.
 * \f[ \left(0,\frac{F_s}{2^i}\right) , \left(\frac{F_s}{2^i},\frac{F_s}{2^{i-1}}\right)
 * , \ldots \left(\frac{F_s}{2^2},\frac{F_s}{2^1}\right) \f]
 *
 * The intensity of each frame \f$n\f$ is calculated by summing the magnitude \f$A\f$ of each
 * frequency bin \f$k\f$.
 * \f[ I(n) = \displaystyle\sum\limits_{k=0}^{F_s/2} A(n,k) \f]
 *
 * For each sub-band \f$i\f$ with a frequency range from \f$L_i\f$ to \f$H_i\f$, the intensity
 * ratio is the ratio of that sub-band's intensity to the overall intensity.
 * \f[ D_i(n) = \frac{1}{I(n)} \displaystyle\sum\limits_{k=L_i}^{H_i} A(n,k) \f]
 *
 * ﻿[1] <i>Lu, L., Liu, D., & Zhang, H.-J. (2006). Automatic Mood Detection and Tracking of Music
 * Audio Signals. IEEE Transactions on Audio, Speech and Language Processing (Vol. 14, pp. 5-18).﻿</i>
 */
class Intensity : public Vamp::Plugin
{
public:
    /// @cond
    Intensity(float inputSampleRate);
    virtual ~Intensity();
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
    /// @endcond

protected:
    void calculateBandFreqs();

    /// @cond
    int m_blockSize, m_stepSize;
    float m_sampleRate;
    /// @endcond

    int numBands;			/*!< Number of sub-bands to use */
    float *bandHighFreq;	/*!< Upper frequency range of each sub-band */
};

#endif
