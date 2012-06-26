#include "Rhythm.h"

Rhythm::Rhythm(float inputSampleRate):Plugin(inputSampleRate)
{
	m_sampleRate = inputSampleRate;
	numBands = 7;
	bandHighFreq = NULL;
	calculateBandFreqs();

	// calculate and save half-hanny window
	halfHannLength = 12;
	halfHannWindow = new float[halfHannLength];
	for (int i=0; i<halfHannLength; i++)
		halfHannWindow[i] = halfHanning((float)i);

	// calculate and save canny window
	cannyLength = 12;
	cannyShape = 4.f;
	cannyWindow = new float[cannyLength*2 + 1];
	for (int i=cannyLength*-1; i<cannyLength+1; i++)
		cannyWindow[i+cannyLength] = canny((float)i);
}

Rhythm::~Rhythm()
{
	delete [] halfHannWindow;
}

string
Rhythm::getIdentifier() const
{
    return "bbcrd-rhythm";
}

string
Rhythm::getName() const
{
    return "Rhythm";
}

string
Rhythm::getDescription() const
{
    return "";
}

string
Rhythm::getMaker() const
{
    return "BBC Research and Development";
}

int
Rhythm::getPluginVersion() const
{
    return 1;
}

string
Rhythm::getCopyright() const
{
    return "(c) 2012 British Broadcasting Corporation";
}

Rhythm::InputDomain
Rhythm::getInputDomain() const
{
    return FrequencyDomain;
}

size_t
Rhythm::getPreferredBlockSize() const
{
    return 1024;
}

size_t 
Rhythm::getPreferredStepSize() const
{
    return 1024;
}

size_t
Rhythm::getMinChannelCount() const
{
    return 1;
}

size_t
Rhythm::getMaxChannelCount() const
{
    return 1;
}

Rhythm::ParameterList
Rhythm::getParameterDescriptors() const
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
Rhythm::getParameter(string identifier) const
{
    if (identifier == "numBands")
        return numBands;
    return 0;
}

void
Rhythm::setParameter(string identifier, float value)
{
    if (identifier == "numBands") {
    	numBands = value;
    	calculateBandFreqs();
    }
}

Rhythm::ProgramList
Rhythm::getPrograms() const
{
    ProgramList list;

    return list;
}

string
Rhythm::getCurrentProgram() const
{
    return "";
}

void
Rhythm::selectProgram(string name)
{
}

Rhythm::OutputList
Rhythm::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor onset;
    onset.identifier = "onset";
    onset.name = "Onset curve";
    onset.description = "Onset detection curve.";
    onset.unit = "";
    onset.hasFixedBinCount = true;
    onset.binCount = 1;
    onset.hasKnownExtents = false;
    onset.isQuantized = false;
    onset.sampleType = OutputDescriptor::VariableSampleRate;
    onset.hasDuration = false;
    list.push_back(onset);

    return list;
}

bool
Rhythm::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    m_blockSize = blockSize;
    m_stepSize = stepSize;
    reset();

    return true;
}

void
Rhythm::reset()
{
	intensity.clear();
}

void
Rhythm::calculateBandFreqs()
{
	delete [] bandHighFreq;
	bandHighFreq = new float[numBands];

	for (int k=0; k<numBands; k++)
	{
		bandHighFreq[k] = m_sampleRate / pow(2.f,numBands-k);
	}
}


float
Rhythm::halfHanning(float n)
{
	return 0.5f + 0.5f * cos( 2.f * M_PI * ( n / (2.f * (float)halfHannLength - 1.f)));
}

float
Rhythm::canny(float n)
{
	return n / (cannyShape*cannyShape) * exp(-1 * (n*n) / (2*cannyShape*cannyShape));
}

Rhythm::FeatureSet
Rhythm::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
	FeatureSet output;
	float total = 0;
	int currentBand = 0;
	vector<float> bandTotal;

	// set band totals to zero
	for (int i=0; i<numBands; i++)
		bandTotal.push_back(0.f);

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
		bandTotal.at(currentBand) += binVal;
	}

	intensity.push_back(bandTotal);

    return output;
}

Rhythm::FeatureSet
Rhythm::getRemainingFeatures()
{
	FeatureSet output;
	int frames = intensity.size();

	// find envelope by convolving each subband with half-hanning window
	vector< vector<float> > envelope;
	for (int frame=0; frame<frames; frame++)
	{
		vector<float> frameResult;
		for (int subBand=0; subBand<numBands; subBand++)
		{
			float result = 0;
			for (int shift=0; shift<halfHannLength; shift++)
			{
				if (frame+shift < frames) result += intensity.at(frame+shift).at(subBand) * halfHannWindow[shift];
			}
			frameResult.push_back(result);
		}
		envelope.push_back(frameResult);
	}

	// find onset curve by convolving each subband of envelope with canny window
	Feature f;
	f.hasTimestamp = true;
	f.hasDuration = false;

	// for each frame
	for (int frame=0; frame<frames; frame++)
	{
		// reset feature details
		f.timestamp = Vamp::RealTime::frame2RealTime(frame*m_stepSize,m_sampleRate);
		f.values.clear();
		float sum = 0;

		// for each sub-band
		for (int subBand=0; subBand<numBands; subBand++)
		{
			// convolve the canny window with the envelope of that sub-band
			float result = 0;
			for (int shift=cannyLength*-1; shift<cannyLength; shift++)
			{
				if (frame+shift < frames && frame+shift >= 0)
					result += envelope.at(frame+shift).at(subBand) * cannyWindow[shift+cannyLength];
			}
			sum += result;
		}

		// save result
		f.values.push_back(sum);
		output[0].push_back(f);
	}

    return output;
}

