/* Energy.h
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
#ifndef _ENERGY_H_
#define _ENERGY_H_

#include <cmath>
#include <vector>
#include <vamp-sdk/Plugin.h>

using std::string;
using std::vector;

/*!
 * \brief Calculates the RMS energy and low energy ratio of a signal
 *
 * \section Outputs
 * \par RMS energy
 * The root mean square energy of the signal.
 * \par Low energy ratio
 * Percentage of blocks/frames whose energy falls below a threshold.
 *
 * \section Parameters
 * \par Low energy threshold
 * Ratio of threshold to the RMS energy (default = 1.0)
 * \par Use root
 * Whether to apply the square root in RMS calculation. (default = 1)
 *
 * \section Description
 *
 * <b>RMS energy</b> for each block is calculated as follows. The square root
 * can be removed using the 'Use root' parameter (default = true)
 * \f[ RMS = \sqrt{\displaystyle\sum\limits_{i=0}^n x_i^2} \f]
 *
 * The <b>low energy ratio</b> is the percentage of blocks which fall below
 * a certain RMS energy threshold. The threshold is set using the 'Low energy
 * threshold' parameter which is a ratio of the overall mean RMS energy (default = 1).
 */
class Energy : public Vamp::Plugin
{
public:
	/// @cond
    Energy(float inputSampleRate);
    virtual ~Energy();
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
    /// @endcond

    bool useRoot;				/*!< Flag to indicate whether to find root of mean energy */
    float threshRatio;			/*!< Ratio of threshold to average energy */
    vector<float> rmsEnergy;	/*!< Variable to store RMS values from previous blocks in order to calculate mean */
};



#endif
