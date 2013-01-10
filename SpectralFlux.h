/* SpectralFlux.h
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
#ifndef _FLUX_H_
#define _FLUX_H_

#include <cmath>
#include <complex>
#include <vector>
#include <vamp-sdk/Plugin.h>

using std::string;
using std::vector;
using std::complex;
using std::abs;

/*!
 * \brief Calculates the spectral flux
 *
 * \section Outputs
 * \par Spectral flux
 * The spectral difference between successive frames.
 *
 * \section Parameters
 * \par Use L2 norm
 * Whether to use L2 normalisation over L1 (default = 0)
 *
 * \section Description
 *
 * The algorithm is defined in [1], section 2.1:
 * "Spectral flux measures the change in magnitude in each frequency bin. It is
 * restricted to the positive changes and summed across all frequency bins."
 *
 * When using L1 norm, the algorithm is as follows:
 * \f[ SF(n) = \sum_{k=0}^{F_s/2} H\left( | X(n,k)| - |X(n-1,k)| \right) \f]
 *
 * When L2 norm is selected, the following is used:
 * \f[ SF(n) = \sqrt{ \sum_{k=0}^{F_s/2} H\left( | X(n,k)| - |X(n-1,k)| \right)^2 } \f]
 *
 * In both cases, \f$ H(x) = \frac{x+|x|}{2} \f$
 *
 * [1] Dixon, S. (2006). Onset Detection Revisited. International Conference on
 * Digital Audio Effects (DAFx) (pp. 133–137).
 */
class SpectralFlux : public Vamp::Plugin
{
public:
	/// @cond
    SpectralFlux(float inputSampleRate);
    virtual ~SpectralFlux();
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
    /// @cond
    int m_blockSize, m_stepSize;
    vector<float> prevBin;
    /// @endcond

    bool l2norm;	/*!< Flag to indicate use of L2 normalisation */
};

#endif
