#include "VampDynamicsSpectral.h"

VampDynamicsSpectral::VampDynamicsSpectral(float inputSampleRate):Plugin(inputSampleRate)
{
	m_sampleRate = inputSampleRate;
	numBands = 7;
	bandHighFreq = NULL;
	calculateBandFreqs();
}

VampDynamicsSpectral::~VampDynamicsSpectral()
{
}

string
VampDynamicsSpectral::getIdentifier() const
{
    return "dynamics-spectral";
}

string
VampDynamicsSpectral::getName() const
{
    return "Dynamics";
}

string
VampDynamicsSpectral::getDescription() const
{
    return "";
}

string
VampDynamicsSpectral::getMaker() const
{
    return "BBC Research and Development";
}

int
VampDynamicsSpectral::getPluginVersion() const
{
    return 1;
}

string
VampDynamicsSpectral::getCopyright() const
{
    return "(c) 2012 British Broadcasting Corporation";
}

VampDynamicsSpectral::InputDomain
VampDynamicsSpectral::getInputDomain() const
{
    return FrequencyDomain;
}

size_t
VampDynamicsSpectral::getPreferredBlockSize() const
{
    return 1024;
}

size_t 
VampDynamicsSpectral::getPreferredStepSize() const
{
    return 1024;
}

size_t
VampDynamicsSpectral::getMinChannelCount() const
{
    return 1;
}

size_t
VampDynamicsSpectral::getMaxChannelCount() const
{
    return 1;
}

VampDynamicsSpectral::ParameterList
VampDynamicsSpectral::getParameterDescriptors() const
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
VampDynamicsSpectral::getParameter(string identifier) const
{
    if (identifier == "numBands")
        return numBands;
    return 0;
}

void
VampDynamicsSpectral::setParameter(string identifier, float value)
{
    if (identifier == "numBands") {
    	numBands = value;
    	calculateBandFreqs();
    }
}

VampDynamicsSpectral::ProgramList
VampDynamicsSpectral::getPrograms() const
{
    ProgramList list;

    return list;
}

string
VampDynamicsSpectral::getCurrentProgram() const
{
    return "";
}

void
VampDynamicsSpectral::selectProgram(string name)
{
}

VampDynamicsSpectral::OutputList
VampDynamicsSpectral::getOutputDescriptors() const
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
VampDynamicsSpectral::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    m_blockSize = blockSize;
    m_stepSize = stepSize;
    reset();

    return true;
}

void
VampDynamicsSpectral::reset()
{
}

void
VampDynamicsSpectral::calculateBandFreqs()
{
	delete [] bandHighFreq;
	bandHighFreq = new float[numBands];

	for (int k=0; k<numBands; k++)
	{
		bandHighFreq[k] = m_sampleRate / pow(2.f,numBands-k);
		cout << bandHighFreq[k] << "\n";
	}
}

VampDynamicsSpectral::FeatureSet
VampDynamicsSpectral::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
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

VampDynamicsSpectral::FeatureSet
VampDynamicsSpectral::getRemainingFeatures()
{

	FeatureSet output;
//	Feature f;
//	f.hasTimestamp = true;
//	f.timestamp = Vamp::RealTime::fromSeconds(0);
//	f.values.push_back(lowEnergyRatio);
//	output[1].push_back(f);
    return output;
}

