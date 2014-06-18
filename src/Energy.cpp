/**
 * BBC Vamp plugin collection
 *
 * Copyright (c) 2011-2014 British Broadcasting Corporation
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
  sampleRate = inputSampleRate;
	threshRatio = 1;
	useRoot = true;
  prevRMS=0;
  avgWindowLength=1;
  avgPercentile=3;
  dipThresh=3;
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
    return 3;
}

string
Energy::getCopyright() const
{
    return "(c) 2014 British Broadcasting Corporation";
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

    ParameterDescriptor avgwindow;
    avgwindow.identifier = "avgwindow";
    avgwindow.name = "Moving average window size";
    avgwindow.description = "Size of moving averagw window, in seconds.";
    avgwindow.unit = "seconds";
    avgwindow.minValue = 0.001;
    avgwindow.maxValue = 10;
    avgwindow.defaultValue = 1;
    avgwindow.isQuantized = false;
    list.push_back(avgwindow);

    ParameterDescriptor avgpercentile;
    avgpercentile.identifier = "avgpercentile";
    avgpercentile.name = "Moving average percentile";
    avgpercentile.description = "Percentile to use when calculating moving average.";
    avgpercentile.unit = "";
    avgpercentile.minValue = 0;
    avgpercentile.maxValue = 100;
    avgpercentile.defaultValue = 3;
    avgpercentile.isQuantized = false;
    list.push_back(avgpercentile);

    ParameterDescriptor dipthresh;
    dipthresh.identifier = "dipthresh";
    dipthresh.name = "Dip threshold";
    dipthresh.description = "Threshold for calculating dips, as multiple of the moving average.";
    dipthresh.unit = "";
    dipthresh.minValue = 0;
    dipthresh.maxValue = 10;
    dipthresh.defaultValue = 3;
    dipthresh.isQuantized = false;
    list.push_back(dipthresh);

    ParameterDescriptor threshold;
    threshold.identifier = "threshold";
    threshold.name = "Low energy threshold";
    threshold.description = "Threshold to use for low energy, as a multiple of mean energy.";
    threshold.unit = "";
    threshold.minValue = 0;
    threshold.maxValue = 10;
    threshold.defaultValue = 1;
    threshold.isQuantized = false;
    list.push_back(threshold);

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
    else if (identifier == "avgwindow")
    {
    	return avgWindowLength;
    }
    else if (identifier == "avgpercentile")
    {
    	return avgPercentile;
    }
    else if (identifier == "dipthresh")
    {
      return dipThresh;
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
    else if (identifier == "avgwindow")
    {
      avgWindowLength = value;
    }
    else if (identifier == "avgpercentile")
    {
      avgPercentile = value;
    }
    else if (identifier == "dipthresh")
    {
      dipThresh = value;
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

    OutputDescriptor rmsdelta;
    rmsdelta.identifier = "rmsdelta";
    rmsdelta.name = "RMS Energy Delta";
    rmsdelta.description = "Difference between RMS of previous and current blocks.";
    rmsdelta.unit = "";
    rmsdelta.hasFixedBinCount = true;
    rmsdelta.binCount = 1;
    rmsdelta.hasKnownExtents = false;
    rmsdelta.isQuantized = false;
    rmsdelta.sampleType = OutputDescriptor::OneSamplePerStep;
    rmsdelta.hasDuration = false;
    list.push_back(rmsdelta);

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
    lowenergy.sampleRate = 0;
    lowenergy.hasDuration = false;
    list.push_back(lowenergy);

    OutputDescriptor average;
    average.identifier = "average";
    average.name = "Moving Average";
    average.description = "Mean of RMS values over moving average window.";
    average.unit = "";
    average.hasFixedBinCount = true;
    average.binCount = 1;
    average.hasKnownExtents = false;
    average.isQuantized = false;
    average.sampleType = OutputDescriptor::FixedSampleRate;
    average.sampleRate = (float)sampleRate/(float)m_stepSize;
    average.hasDuration = false;
    list.push_back(average);

    OutputDescriptor pdip;
    pdip.identifier = "pdip";
    pdip.name = "Dip probability";
    pdip.description = "Probability of the RMS energy dipping below the threshold.";
    pdip.unit = "";
    pdip.hasFixedBinCount = true;
    pdip.binCount = 1;
    pdip.hasKnownExtents = false;
    pdip.isQuantized = false;
    pdip.sampleType = OutputDescriptor::FixedSampleRate;
    pdip.sampleRate = (float)sampleRate/(float)m_stepSize;
    pdip.hasDuration = false;
    list.push_back(pdip);

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
  prevRMS=0;
}

Energy::FeatureSet
Energy::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
	FeatureSet output;
	Feature fRMS, fDelta;
	float totalEnergy = 0.f;
	float rms;

  // find total energy for frame
	for (int i=0; i<m_blockSize; i++)
	{
		totalEnergy += inputBuffers[0][i]*inputBuffers[0][i];
	}

  // apply square root
	if (useRoot)
		rms = sqrt(totalEnergy / (float)m_blockSize);
	else
		rms = totalEnergy / (float)m_blockSize;
	rmsEnergy.push_back(rms);

  // return RMS and delta
	fRMS.values.push_back(rms);
	output[0].push_back(fRMS);
  fDelta.values.push_back(std::abs(rms-prevRMS));
  output[1].push_back(fDelta);
  
  // save RMS of current frame
  prevRMS=rms;
  
  return output;
}

Energy::FeatureSet
Energy::getRemainingFeatures()
{
	FeatureSet output;
  vector<float> rmsAvg;
	float total = 0.f, average = 0.f;
	float lowEnergy = 0.f, highEnergy = 0.f;

  // set window size
  float avgWindowSize = avgWindowLength*sampleRate/(float)m_blockSize;
  int avgWindowOffsetL = (int)floor(avgWindowSize/2.0);
  int avgWindowOffsetR = (int)ceil(avgWindowSize/2.0);

	for (unsigned i=0; i<rmsEnergy.size(); i++)
	{
	  // find total of RMS energy values
		total += rmsEnergy.at(i);

    // get start and end of window
    int start = i-avgWindowOffsetL;
    if (start<0) start=0;
    int end = i+avgWindowOffsetR-1;
    if (end>=rmsEnergy.size()) end = rmsEnergy.size()-1;

    // copy window
    vector<float>::const_iterator first = rmsEnergy.begin() + start;
    vector<float>::const_iterator last = rmsEnergy.begin() + end + 1;
    vector<float> window(first, last);

    // sort window
    std::sort(window.begin(), window.end());

    // find Xth percentile of window
    int pos = (int)((float)(window.size()-1) / 100.0 * avgPercentile);
    rmsAvg.push_back(window[pos]);

    // return moving average
    Feature fAvg;
    fAvg.values.push_back(rmsAvg[i]);
    output[3].push_back(fAvg);
	}

  // find mean of all RMS values
	if (rmsEnergy.size() != 0)
		average = total / (float)rmsEnergy.size();

	// find threshold value
	float threshLowEnergy = average * threshRatio;

	for (unsigned i=0; i<rmsEnergy.size(); i++)
	{
	  // find number of frames above/below low energy threshold
		if (rmsEnergy.at(i) < threshLowEnergy)
			lowEnergy++;
		else
			highEnergy++;

    // get start and end of window
    int start = i-avgWindowOffsetL;
    if (start<0) start=0;
    int end = i+avgWindowOffsetR-1;
    if (end>=rmsEnergy.size()) end = rmsEnergy.size()-1;

    // count dips below moving average * dipThresh
    float dipCount = 0;
    float threshDip = rmsAvg[i]*dipThresh;
    for (unsigned int j=start; j<=end; j++)
    {
      if (rmsEnergy[j] < threshDip)
        dipCount++;
    }

    // return dip probability 
    Feature fProb;
    fProb.values.push_back(dipCount/(float)(end-start));
    output[4].push_back(fProb);
	}

	// calculate low energy ratio
	float lowEnergyRatio = 0.f;
	if (lowEnergy + highEnergy != 0)
		lowEnergyRatio = (100.f * lowEnergy) / (lowEnergy + highEnergy);

  // return low energy
	Feature fLowEnergy;
	fLowEnergy.hasTimestamp = true;
	fLowEnergy.timestamp = Vamp::RealTime::fromSeconds(0);
	fLowEnergy.values.push_back(lowEnergyRatio);
	output[2].push_back(fLowEnergy);

  return output;
}

/// @endcond
