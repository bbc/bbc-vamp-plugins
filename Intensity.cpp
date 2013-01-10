/* Intensity.cpp
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
#include "Intensity.h"
/// @cond

Intensity::Intensity(float inputSampleRate):Plugin(inputSampleRate)
{
	m_sampleRate = inputSampleRate;
	numBands = 7;
	bandHighFreq = NULL;
	calculateBandFreqs();
}

Intensity::~Intensity()
{
  delete [] bandHighFreq;
}

string
Intensity::getIdentifier() const
{
    return "bbcrd-intensity";
}

string
Intensity::getName() const
{
    return "Intensity";
}

string
Intensity::getDescription() const
{
    return "";
}

string
Intensity::getMaker() const
{
    return "BBC Research and Development";
}

int
Intensity::getPluginVersion() const
{
    return 1;
}

string
Intensity::getCopyright() const
{
    return "(c) 2013 British Broadcasting Corporation";
}

Intensity::InputDomain
Intensity::getInputDomain() const
{
    return FrequencyDomain;
}

size_t
Intensity::getPreferredBlockSize() const
{
    return 1024;
}

size_t 
Intensity::getPreferredStepSize() const
{
    return 1024;
}

size_t
Intensity::getMinChannelCount() const
{
    return 1;
}

size_t
Intensity::getMaxChannelCount() const
{
    return 1;
}

Intensity::ParameterList
Intensity::getParameterDescriptors() const
{
    ParameterList list;

    ParameterDescriptor numBandsParam;
    numBandsParam.identifier = "numBands";
    numBandsParam.name = "Sub-bands";
    numBandsParam.description = "Number of sub-bands.";
    numBandsParam.unit = "";
    numBandsParam.minValue = 2;
    numBandsParam.maxValue = 50;
    numBandsParam.defaultValue = 7;
    numBandsParam.isQuantized = true;
    numBandsParam.quantizeStep = 1.0;
    list.push_back(numBandsParam);

    return list;
}

float
Intensity::getParameter(string identifier) const
{
    if (identifier == "numBands")
        return numBands;
    return 0;
}

void
Intensity::setParameter(string identifier, float value)
{
    if (identifier == "numBands") {
    	numBands = value;
    	calculateBandFreqs();
    }
}

Intensity::ProgramList
Intensity::getPrograms() const
{
    ProgramList list;

    return list;
}

string
Intensity::getCurrentProgram() const
{
    return "";
}

void
Intensity::selectProgram(string name)
{
}

Intensity::OutputList
Intensity::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor intensity;
    intensity.identifier = "intensity";
    intensity.name = "Intensity";
    intensity.description = "Sum of the FFT bin absolute values.";
    intensity.unit = "";
    intensity.hasFixedBinCount = true;
    intensity.binCount = 1;
    intensity.hasKnownExtents = false;
    intensity.isQuantized = false;
    intensity.sampleType = OutputDescriptor::OneSamplePerStep;
    intensity.hasDuration = false;
    list.push_back(intensity);

    OutputDescriptor intensityRatio;
    intensityRatio.identifier = "intensity-ratio";
    intensityRatio.name = "Intensity Ratio";
    intensityRatio.description = "Sum of each sub-band's absolute values.";
    intensityRatio.unit = "";
    intensityRatio.hasFixedBinCount = true;
    intensityRatio.binCount = numBands;
    intensityRatio.hasKnownExtents = false;
    intensityRatio.isQuantized = false;
    intensityRatio.sampleType = OutputDescriptor::OneSamplePerStep;
    intensityRatio.hasDuration = false;
    list.push_back(intensityRatio);

    return list;
}

bool
Intensity::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    m_blockSize = blockSize;
    m_stepSize = stepSize;
    reset();

    return true;
}

void
Intensity::reset()
{
}

Intensity::FeatureSet
Intensity::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
	FeatureSet output;
	float total = 0;
	int currentBand = 0;
	float *bandTotal = new float[numBands];

	// set band totals to zero
	for (int i=0; i<numBands; i++)
		bandTotal[i] = 0;

	// for each frequency bin
	for (int i=0; i<m_blockSize/2; i++)
	{
		// get absolute value
		float binVal = abs(complex<float>(inputBuffers[0][i*2], inputBuffers[0][i*2+1]));

		// add contents of this bin to total
		total += binVal;

		// find centre frequency of this bin
		float freq = (i+1)*m_sampleRate / (float)m_blockSize;

		// locate which band this bin belongs in
		while (freq > bandHighFreq[currentBand]) {
			currentBand++;
			if (currentBand >= numBands) break;
		}

		// add bin value to relevent band
		bandTotal[currentBand] += binVal;
	}

	// send intensity outputs
	Feature intensity;
	intensity.values.push_back(total);
	output[0].push_back(intensity);

	// send intensity ratio outputs
	Feature intensityRatio;
	for (int i=0; i<numBands; i++)
	{
		float bandResult;
		if (total == 0)
			bandResult = 0;
		else
			bandResult = bandTotal[i] / total;
		intensityRatio.values.push_back(bandResult);
	}
	output[1].push_back(intensityRatio);

	// clean up
	delete [] bandTotal;

  return output;
}

Intensity::FeatureSet
Intensity::getRemainingFeatures()
{

	FeatureSet output;
  return output;
}

/// @endcond

/*!
 * \brief Calculates the upper frequency for each of a given
 * number of sub-bands.
 */
void
Intensity::calculateBandFreqs()
{
	delete [] bandHighFreq;
	bandHighFreq = new float[numBands];

	for (int k=0; k<numBands; k++)
	{
		bandHighFreq[k] = m_sampleRate / pow(2.f,numBands-k);
	}
}
