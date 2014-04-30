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
#ifndef _ENERGY_H_
#define _ENERGY_H_

#include <cmath>
#include <vector>
#include <algorithm>
#include <vamp-sdk/Plugin.h>

using std::string;
using std::vector;

/*!
 * \brief Calculates the RMS energy and related features
 *
 * \section Outputs
 * \par RMS energy
 * The root mean square energy of the signal.
 * \par RMS energy delta
 * The difference between the RMS energy of the current frame and the last.
 * \par Moving average
 * The Xth percentile of the RMS energy within a window.
 * \par Dip probability
 * The ratio of frames that have dipped below the dip threshold within the
 * averaging window, where the threshold is a product of the moving average.
 * \par Low energy ratio
 * Percentage of frames in the file whose energy falls below a threshold, which
 * is a product of the overall mean energy.
 *
 * \section Parameters
 * \par Use root
 * Whether to apply the square root in RMS calculation. (default = 1)
 * \par Moving average window size
 * The size of the averaging window, in seconds. (default = 1.0)
 * \par Moving average percentile
 * The percentile used to calculate the average. (default = 3.0)
 * \par Dip threshold
 * The threshold for calculating the dip, which is multiplied by the moving
 * average. (default = 3.0)
 * \par Low energy threshold
 * The threshold for calculating low energy, which is multiplied by the overall
 * mean RMS energy (default = 1.0)
 *
 * \section Description
 *
 * <b>RMS energy</b> for each block is calculated as follows. The square root
 * can be removed using the 'Use root' parameter (default = true)
 * \f[ RMS = \sqrt{\displaystyle\sum\limits_{i=0}^n x_i^2} \f]
 *
 * The <b>dip threshold</b> is a simple but effective speech/music
 * discriminator. It is defined as the ratio of frames in a moving window which
 * fall below a threshold, where the threshold is a product of the moving
 * average.
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

    float sampleRate;   /*!< Variable to store input sample rate, used for calculating window sizes */
    bool useRoot;				/*!< Flag to indicate whether to find root of mean energy */
    float threshRatio;			/*!< Ratio of threshold to average energy */
    vector<float> rmsEnergy;	/*!< Variable to store RMS values from previous blocks in order to calculate mean */
    float prevRMS; /*!< Variable to store RMS value of previous block */
    float avgWindowLength; /*!< Length of window to use for averaging, in seconds */
    float avgPercentile; /*!< Percentile to calculate as average. */
    float dipThresh; /*!< Threshold to use for calculating dips, as a multiple of the moving average. */
};



#endif
