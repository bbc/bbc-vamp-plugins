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
#include "Energy.h"
/// @cond

Energy::Energy(float inputSampleRate):Plugin(inputSampleRate)
{
	threshRatio = 1;
	useRoot = true;
}

Energy::~Energy()
{
}

string
Energy::getIdentifier() const
{
    return "bbc-energy";
}

string
Energy::getName() const
{
    return "Energy";
}

string
Energy::getDescription() const
{
    return "";
}

string
Energy::getMaker() const
{
    return "BBC";
}

int
Energy::getPluginVersion() const
{
    return 2;
}

string
Energy::getCopyright() const
{
    return "(c) 2013 British Broadcasting Corporation";
}

Energy::InputDomain
Energy::getInputDomain() const
{
    return TimeDomain;
}

size_t
Energy::getPreferredBlockSize() const
{
    return 1024;
}

size_t 
Energy::getPreferredStepSize() const
{
    return 1024;
}

size_t
Energy::getMinChannelCount() const
{
    return 1;
}

size_t
Energy::getMaxChannelCount() const
{
    return 1;
}

Energy::ParameterList
Energy::getParameterDescriptors() const
{
    ParameterList list;

    ParameterDescriptor threshold;
    threshold.identifier = "threshold";
    threshold.name = "Low energy threshold";
    threshold.description = "Ratio of threshold to average energy.";
    threshold.unit = "";
    threshold.minValue = 0;
    threshold.maxValue = 10;
    threshold.defaultValue = 1;
    threshold.isQuantized = false;
    list.push_back(threshold);

    ParameterDescriptor root;
    root.identifier = "root";
    root.name = "Use root";
    root.description = "Whether to apply root to energy calc.";
    root.unit = "";
    root.minValue = 0;
    root.maxValue = 1;
    root.defaultValue = 1;
    root.isQuantized = true;
    root.quantizeStep = 1;
    list.push_back(root);

    return list;
}

float
Energy::getParameter(string identifier) const
{
    if (identifier == "threshold") {
        return threshRatio;
    }
    else if (identifier == "root")
    {
    	return useRoot;
    }
    return 0;
}

void
Energy::setParameter(string identifier, float value)
{
    if (identifier == "threshold") {
    	threshRatio = value;
    }
    else if (identifier == "root")
    {
    	if (value == 1)
    		useRoot = true;
    	else
    		useRoot = false;
    }
}

Energy::ProgramList
Energy::getPrograms() const
{
    ProgramList list;

    return list;
}

string
Energy::getCurrentProgram() const
{
    return "";
}

void
Energy::selectProgram(string name)
{
}

Energy::OutputList
Energy::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor rmsenergy;
    rmsenergy.identifier = "rmsenergy";
    rmsenergy.name = "RMS Energy";
    rmsenergy.description = "RMS of the signal.";
    rmsenergy.unit = "";
    rmsenergy.hasFixedBinCount = true;
    rmsenergy.binCount = 1;
    rmsenergy.hasKnownExtents = false;
    rmsenergy.isQuantized = false;
    rmsenergy.sampleType = OutputDescriptor::OneSamplePerStep;
    rmsenergy.hasDuration = false;
    list.push_back(rmsenergy);

    OutputDescriptor lowenergy;
    lowenergy.identifier = "lowenergy";
    lowenergy.name = "Low Energy";
    lowenergy.description = "Percentage of track which is below the low energy threshold.";
    lowenergy.unit = "";
    lowenergy.hasFixedBinCount = true;
    lowenergy.binCount = 1;
    lowenergy.hasKnownExtents = false;
    lowenergy.isQuantized = false;
    lowenergy.sampleType = OutputDescriptor::VariableSampleRate;
    lowenergy.hasDuration = false;
    list.push_back(lowenergy);

    return list;
}

bool
Energy::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    m_blockSize = blockSize;
    m_stepSize = stepSize;
    reset();

    return true;
}

void
Energy::reset()
{
	rmsEnergy.clear();
}

Energy::FeatureSet
Energy::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
	float totalEnergy;
	for (int i=0; i<m_blockSize; i++)
	{
		totalEnergy += inputBuffers[0][i]*inputBuffers[0][i];
	}

	float rms;
	if (useRoot)
		rms = sqrt(totalEnergy / (float)m_blockSize);
	else
		rms = totalEnergy / (float)m_blockSize;
	rmsEnergy.push_back(rms);

	FeatureSet output;
	Feature f;
	f.values.push_back(rms);
	output[0].push_back(f);
    return output;
}

Energy::FeatureSet
Energy::getRemainingFeatures()
{
	// find average of RMS energy values
	float total;
	for (unsigned i=0; i<rmsEnergy.size(); i++)
	{
		total += rmsEnergy.at(i);
	}
	float average = total / (float)rmsEnergy.size();

	// find threshold value
	float threshold = average * threshRatio;

	// find number of frames above/below threshold
	float lowEnergy, highEnergy = 0;
	for (unsigned i=0; i<rmsEnergy.size(); i++)
	{
		if (rmsEnergy.at(i) < threshold)
			lowEnergy++;
		else
			highEnergy++;
	}

	// calculate low energy ratio
	float lowEnergyRatio = (100.f * lowEnergy) / (lowEnergy + highEnergy);

	FeatureSet output;
	Feature f;
	f.hasTimestamp = true;
	f.timestamp = Vamp::RealTime::fromSeconds(0);
	f.values.push_back(lowEnergyRatio);
	output[1].push_back(f);
    return output;
}

/// @endcond
