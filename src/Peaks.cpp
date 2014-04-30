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
#include "Peaks.h"
/// @cond

Peaks::Peaks(float inputSampleRate):Plugin(inputSampleRate)
{
}

Peaks::~Peaks()
{
}

string
Peaks::getIdentifier() const
{
    return "bbc-peaks";
}

string
Peaks::getName() const
{
    return "Peaks";
}

string
Peaks::getDescription() const
{
    return "";
}

string
Peaks::getMaker() const
{
    return "BBC";
}

int
Peaks::getPluginVersion() const
{
    return 1;
}

string
Peaks::getCopyright() const
{
    return "(c) 2014 British Broadcasting Corporation";
}

Peaks::InputDomain
Peaks::getInputDomain() const
{
    return TimeDomain;
}

size_t
Peaks::getPreferredBlockSize() const
{
    return 256;
}

size_t 
Peaks::getPreferredStepSize() const
{
    return 256;
}

size_t
Peaks::getMinChannelCount() const
{
    return 1;
}

size_t
Peaks::getMaxChannelCount() const
{
    return 1;
}

Peaks::ParameterList
Peaks::getParameterDescriptors() const
{
    ParameterList list;
    return list;
}

float
Peaks::getParameter(string identifier) const
{
    return 0;
}

void
Peaks::setParameter(string identifier, float value)
{
}

Peaks::ProgramList
Peaks::getPrograms() const
{
    ProgramList list;
    return list;
}

string
Peaks::getCurrentProgram() const
{
    return "";
}

void
Peaks::selectProgram(string name)
{
}

Peaks::OutputList
Peaks::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor peaks;
    peaks.identifier = "peaks";
    peaks.name = "Peaks";
    peaks.description = "Peak and trough, in order of occurance.";
    peaks.unit = "";
    peaks.hasFixedBinCount = true;
    peaks.binCount = 1;
    peaks.hasKnownExtents = false;
    peaks.isQuantized = false;
    peaks.sampleType = OutputDescriptor::OneSamplePerStep;
    peaks.hasDuration = false;
    list.push_back(peaks);

    return list;
}

bool
Peaks::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    m_blockSize = blockSize;
    m_stepSize = stepSize;
    reset();

    return true;
}

void
Peaks::reset()
{
}

Peaks::FeatureSet
Peaks::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
  float min=1.f;
  int minPoint=0;
  float max=-1.f;
  int maxPoint=0;
	for (int i=0; i<m_blockSize; i++)
	{
    if (inputBuffers[0][i] < min)
    {
      min=inputBuffers[0][i];
      minPoint=i;
    }
    else if (inputBuffers[0][i] > max)
    {
      max=inputBuffers[0][i];
      maxPoint=i;
    }
	}

	FeatureSet output;
	Feature f;
  if (minPoint<maxPoint)
  {
    f.values.push_back(min);
    f.values.push_back(max);
  } else {
    f.values.push_back(max);
    f.values.push_back(min);
  }
	output[0].push_back(f);
  return output;
}

Peaks::FeatureSet
Peaks::getRemainingFeatures()
{
  return FeatureSet();
}

/// @endcond
