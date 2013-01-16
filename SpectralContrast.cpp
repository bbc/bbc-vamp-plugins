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
#include "SpectralContrast.h"
/// @cond

SpectralContrast::SpectralContrast(float inputSampleRate):Plugin(inputSampleRate)
{
  m_sampleRate = inputSampleRate;
  alpha = 0.02;
  numBands = 7;
  bandHighFreq = NULL;
  calculateBandFreqs();
}

SpectralContrast::~SpectralContrast()
{
  delete[] bandHighFreq;
}

string
SpectralContrast::getIdentifier() const
{
    return "bbc-spectral-contrast";
}

string
SpectralContrast::getName() const
{
    return "Spectral Contrast";
}

string
SpectralContrast::getDescription() const
{
    return "";
}

string
SpectralContrast::getMaker() const
{
    return "BBC";
}

int
SpectralContrast::getPluginVersion() const
{
    return 1;
}

string
SpectralContrast::getCopyright() const
{
    return "(c) 2013 British Broadcasting Corporation";
}

SpectralContrast::InputDomain
SpectralContrast::getInputDomain() const
{
    return FrequencyDomain;
}

size_t
SpectralContrast::getPreferredBlockSize() const
{
    return 1024;
}

size_t 
SpectralContrast::getPreferredStepSize() const
{
    return 512;
}

size_t
SpectralContrast::getMinChannelCount() const
{
    return 1;
}

size_t
SpectralContrast::getMaxChannelCount() const
{
    return 1;
}

SpectralContrast::ParameterList
SpectralContrast::getParameterDescriptors() const
{
    ParameterList list;

    ParameterDescriptor paramAlpha;
    paramAlpha.identifier = "alpha";
    paramAlpha.name = "Alpha";
    paramAlpha.description = "Ratio of FFT bins used to find average";
    paramAlpha.unit = "";
    paramAlpha.minValue = 0;
    paramAlpha.maxValue = 1;
    paramAlpha.defaultValue = 0.02;
    paramAlpha.isQuantized = false;
    list.push_back(paramAlpha);

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
SpectralContrast::getParameter(string identifier) const
{
    if (identifier == "alpha")
        return alpha;
    if (identifier == "numBands")
        return numBands;
    return 0;
}

void
SpectralContrast::setParameter(string identifier, float value)
{
    if (identifier == "alpha") {
    	alpha = value;
    }
    if (identifier == "numBands") {
      numBands = value;
      calculateBandFreqs();
    }
}

SpectralContrast::ProgramList
SpectralContrast::getPrograms() const
{
    ProgramList list;

    return list;
}

string
SpectralContrast::getCurrentProgram() const
{
    return "";
}

void
SpectralContrast::selectProgram(string name)
{
}

SpectralContrast::OutputList
SpectralContrast::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor SpectralValleys;
    SpectralValleys.identifier = "valleys";
    SpectralValleys.name = "Spectral Valleys";
    SpectralValleys.description = "Valley of the spectrum.";
    SpectralValleys.unit = "";
    SpectralValleys.hasFixedBinCount = true;
    SpectralValleys.binCount = numBands;
    SpectralValleys.hasKnownExtents = false;
    SpectralValleys.isQuantized = false;
    SpectralValleys.sampleType = OutputDescriptor::OneSamplePerStep;
    SpectralValleys.hasDuration = false;
    list.push_back(SpectralValleys);

    OutputDescriptor SpectralPeaks;
    SpectralPeaks.identifier = "peaks";
    SpectralPeaks.name = "Spectral Peaks";
    SpectralPeaks.description = "Peak of the spectrum.";
    SpectralPeaks.unit = "";
    SpectralPeaks.hasFixedBinCount = true;
    SpectralPeaks.binCount = numBands;
    SpectralPeaks.hasKnownExtents = false;
    SpectralPeaks.isQuantized = false;
    SpectralPeaks.sampleType = OutputDescriptor::OneSamplePerStep;
    SpectralPeaks.hasDuration = false;
    list.push_back(SpectralPeaks);

    OutputDescriptor SpectralMean;
    SpectralMean.identifier = "mean";
    SpectralMean.name = "Spectral Mean";
    SpectralMean.description = "Mean of the spectrum.";
    SpectralMean.unit = "";
    SpectralMean.hasFixedBinCount = true;
    SpectralMean.binCount = numBands;
    SpectralMean.hasKnownExtents = false;
    SpectralMean.isQuantized = false;
    SpectralMean.sampleType = OutputDescriptor::OneSamplePerStep;
    SpectralMean.hasDuration = false;
    list.push_back(SpectralMean);

    return list;
}

bool
SpectralContrast::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() || channels > getMaxChannelCount()) return false;

    m_blockSize = blockSize;
    m_stepSize = stepSize;
    reset();

    return true;
}

void
SpectralContrast::reset()
{
}

SpectralContrast::FeatureSet
SpectralContrast::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
	FeatureSet output;
  Feature valleysOut;
  Feature peaksOut;
  Feature meanOut;
  int currentBand = 0;

  // create vector of vectors
  vector<float> empty;
	vector< vector<float> > bins;
	bins.push_back(empty);

  // for each frequency bin
  for (int i=0; i<m_blockSize/2; i++)
  {
    // get absolute value
    float binVal = abs(complex<float>(inputBuffers[0][i*2], inputBuffers[0][i*2+1]));

    // find centre frequency of this bin
    float freq = (i+1)*m_sampleRate / (float)m_blockSize;

    // locate which band this bin belongs in
    while (freq > bandHighFreq[currentBand]) {
      currentBand++;
      if (currentBand >= numBands) break;
      bins.push_back(empty);
    }

    // add the bin to the relevent band vector
    bins.at(currentBand).push_back(binVal);
  }

  // for each band
  for (int band=0; band<numBands; band++)
  {
    // sort the bins by magnitude
    std::sort(bins.at(band).begin(), bins.at(band).end());

    // find the start and end of valley bins
    int start = 0;
    int end = 1;
    if (bins.at(band).size() >= (1/alpha)) end = round(bins.at(band).size()*alpha);
    float valleySum = 0;

    // find average of those bins
    for (int i=start; i<end; i++)
    {
      valleySum += bins.at(band).at(i);
    }
    valleysOut.values.push_back(valleySum / (float)(end - start));

    // find the start and end of peak bins
    start = bins.at(band).size()-1;
    if (bins.at(band).size() >= (1/alpha)) start = bins.at(band).size() - round(bins.at(band).size()*alpha);
    end = bins.at(band).size();
    float peakSum = 0;

    // find average of those bins
    for (int i=start; i<end; i++)
    {
      peakSum += bins.at(band).at(i);
    }
    peaksOut.values.push_back(peakSum / (float)(end - start));

    // find average of all bins in band
    start = 0;
    end = bins.at(band).size();
    float meanSum = 0;
    for (int i=start; i<end; i++)
    {
      meanSum += bins.at(band).at(i);
    }
    meanOut.values.push_back(meanSum / (float)end);
  }

  // save outputs
  output[0].push_back(valleysOut);
  output[1].push_back(peaksOut);
  output[2].push_back(meanOut);

  return output;
}

SpectralContrast::FeatureSet
SpectralContrast::getRemainingFeatures()
{
    return FeatureSet();
}

/// @endcond

/*!
 * \brief Calculates the upper frequency for each of a given
 * number of sub-bands.
 */
void
SpectralContrast::calculateBandFreqs()
{
  delete [] bandHighFreq;
  bandHighFreq = new float[numBands];

  for (int k=0; k<numBands; k++)
  {
    bandHighFreq[k] = m_sampleRate / pow(2.f,numBands-k);
  }
}
