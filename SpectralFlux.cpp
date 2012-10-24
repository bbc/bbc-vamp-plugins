#include "SpectralFlux.h"
/// @cond

SpectralFlux::SpectralFlux(float inputSampleRate):Plugin(inputSampleRate)
{
	l2norm = false;
}

SpectralFlux::~SpectralFlux()
{
}

string
SpectralFlux::getIdentifier() const
{
    return "bbcrd-spectral-flux";
}

string
SpectralFlux::getName() const
{
    return "Spectral Flux";
}

string
SpectralFlux::getDescription() const
{
    return "";
}

string
SpectralFlux::getMaker() const
{
    return "BBC Research and Development";
}

int
SpectralFlux::getPluginVersion() const
{
    return 1;
}

string
SpectralFlux::getCopyright() const
{
    return "(c) 2012 British Broadcasting Corporation";
}

SpectralFlux::InputDomain
SpectralFlux::getInputDomain() const
{
    return FrequencyDomain;
}

size_t
SpectralFlux::getPreferredBlockSize() const
{
    return 1024;
}

size_t 
SpectralFlux::getPreferredStepSize() const
{
    return 1024;
}

size_t
SpectralFlux::getMinChannelCount() const
{
    return 1;
}

size_t
SpectralFlux::getMaxChannelCount() const
{
    return 1;
}

SpectralFlux::ParameterList
SpectralFlux::getParameterDescriptors() const
{
    ParameterList list;

    ParameterDescriptor usel2;
    usel2.identifier = "usel2";
    usel2.name = "Use L2 norm over L1";
    usel2.description = "Replaces L1 normalisation with L2.";
    usel2.unit = "";
    usel2.minValue = 0;
    usel2.maxValue = 1;
    usel2.defaultValue = 0;
    usel2.isQuantized = true;
    usel2.quantizeStep = 1.0;
    list.push_back(usel2);

    return list;
}

float
SpectralFlux::getParameter(string identifier) const
{
    if (identifier == "usel2")
        return l2norm;
    return 0;
}

void
SpectralFlux::setParameter(string identifier, float value)
{
    if (identifier == "usel2") {
    	l2norm = value;
    }
}

SpectralFlux::ProgramList
SpectralFlux::getPrograms() const
{
    ProgramList list;

    return list;
}

string
SpectralFlux::getCurrentProgram() const
{
    return "";
}

void
SpectralFlux::selectProgram(string name)
{
}

SpectralFlux::OutputList
SpectralFlux::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor spectralflux;
    spectralflux.identifier = "spectral-flux";
    spectralflux.name = "Spectral Flux";
    spectralflux.description = "Difference between FFT bin values.";
    spectralflux.unit = "";
    spectralflux.hasFixedBinCount = true;
    spectralflux.binCount = 1;
    spectralflux.hasKnownExtents = false;
    spectralflux.isQuantized = false;
    spectralflux.sampleType = OutputDescriptor::OneSamplePerStep;
    spectralflux.hasDuration = false;
    list.push_back(spectralflux);

    return list;
}

bool
SpectralFlux::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    m_blockSize = blockSize;
    m_stepSize = stepSize;
    reset();

    return true;
}

void
SpectralFlux::reset()
{
  prevBin.clear();
}

SpectralFlux::FeatureSet
SpectralFlux::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
	FeatureSet output;
	float total = 0;

	// for each frequency bin
	for (int i=0; i<m_blockSize/2; i++)
	{
	  // make up previous value if none exists
	  while (i >= prevBin.size())
	  {
	    prevBin.push_back(0.f);
	  }

		// get absolute value
		float bin = abs(complex<float>(inputBuffers[0][i*2], inputBuffers[0][i*2+1]));

		// find difference from prev frame
		float diff = bin - prevBin.at(i);

		// save current frame
		prevBin.at(i) = bin;

		// have-wave rectify
		if (diff < 0) diff = diff * -1;

		// square if L2 norm
		if (l2norm) diff = diff*diff;

		// add to total
		total += diff;
	}

	// find root of total if L2 norm
	if (l2norm) total = sqrt(total);

	// send SpectralFlux outputs
	Feature flux;
	flux.values.push_back(total);
	output[0].push_back(flux);

  return output;
}

SpectralFlux::FeatureSet
SpectralFlux::getRemainingFeatures()
{
    return FeatureSet();
}

/// @endcond
