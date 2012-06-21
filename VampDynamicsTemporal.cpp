#include "VampDynamicsTemporal.h"

VampDynamicsTemporal::VampDynamicsTemporal(float inputSampleRate):Plugin(inputSampleRate)
{
	threshRatio = 1;
}

VampDynamicsTemporal::~VampDynamicsTemporal()
{
}

string
VampDynamicsTemporal::getIdentifier() const
{
    return "dynamics-temporal";
}

string
VampDynamicsTemporal::getName() const
{
    return "Dynamics";
}

string
VampDynamicsTemporal::getDescription() const
{
    return "";
}

string
VampDynamicsTemporal::getMaker() const
{
    return "BBC Research and Development";
}

int
VampDynamicsTemporal::getPluginVersion() const
{
    return 1;
}

string
VampDynamicsTemporal::getCopyright() const
{
    return "(c) 2012 British Broadcasting Corporation";
}

VampDynamicsTemporal::InputDomain
VampDynamicsTemporal::getInputDomain() const
{
    return TimeDomain;
}

size_t
VampDynamicsTemporal::getPreferredBlockSize() const
{
    return 1024;
}

size_t 
VampDynamicsTemporal::getPreferredStepSize() const
{
    return 1024;
}

size_t
VampDynamicsTemporal::getMinChannelCount() const
{
    return 1;
}

size_t
VampDynamicsTemporal::getMaxChannelCount() const
{
    return 1;
}

VampDynamicsTemporal::ParameterList
VampDynamicsTemporal::getParameterDescriptors() const
{
    ParameterList list;

    ParameterDescriptor d;
    d.identifier = "threshold";
    d.name = "Low energy threshold";
    d.description = "Ratio of threshold to average energy.";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 10;
    d.defaultValue = 1;
    d.isQuantized = false;
    list.push_back(d);

    return list;
}

float
VampDynamicsTemporal::getParameter(string identifier) const
{
    if (identifier == "threshold") {
        return threshRatio;
    }
    return 0;
}

void
VampDynamicsTemporal::setParameter(string identifier, float value)
{
    if (identifier == "threshold") {
    	threshRatio = value;
    }
}

VampDynamicsTemporal::ProgramList
VampDynamicsTemporal::getPrograms() const
{
    ProgramList list;

    return list;
}

string
VampDynamicsTemporal::getCurrentProgram() const
{
    return "";
}

void
VampDynamicsTemporal::selectProgram(string name)
{
}

VampDynamicsTemporal::OutputList
VampDynamicsTemporal::getOutputDescriptors() const
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
VampDynamicsTemporal::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    m_blockSize = blockSize;
    m_stepSize = stepSize;
    reset();

    return true;
}

void
VampDynamicsTemporal::reset()
{
	rmsEnergy.clear();
}

VampDynamicsTemporal::FeatureSet
VampDynamicsTemporal::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
	float totalEnergy;
	for (int i=0; i<m_blockSize; i++)
	{
		totalEnergy += inputBuffers[0][i]*inputBuffers[0][i];
	}

	float rms = sqrt(totalEnergy / (float)m_blockSize);
	rmsEnergy.push_back(rms);

	FeatureSet output;
	Feature f;
	f.values.push_back(rms);
	output[0].push_back(f);
    return output;
}

VampDynamicsTemporal::FeatureSet
VampDynamicsTemporal::getRemainingFeatures()
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

