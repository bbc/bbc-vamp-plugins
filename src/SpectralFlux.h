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
