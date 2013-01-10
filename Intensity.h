/* Intensity.h
 *
 * Copyright (c) 2013 British Broadcasting Corporation
 *
 * This file is part of the BBC Vamp plugin collection.
 *
 * BBC Vamp plugin collection is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BBC Vamp plugin collection is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the BBC Vamp plugin collection.
 * If not, see <http://www.gnu.org/licenses/>.
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
