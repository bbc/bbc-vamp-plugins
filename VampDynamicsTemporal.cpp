#include "VampDynamicsTemporal.h"

VampDynamicsTemporal::VampDynamicsTemporal(float inputSampleRate):Plugin(inputSampleRate)
{
	threshRatio = 1;
	useRoot = true;
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
    return 2;
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
VampDynamicsTemporal::getParameter(string identifier) const
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
VampDynamicsTemporal::setParameter(string identifier, float value)
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

