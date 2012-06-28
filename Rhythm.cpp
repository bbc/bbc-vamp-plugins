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

	// set up parameters
	threshold = 1;
	average_window = 200;
	peak_window = 6;
	autocor_max = 5.f;
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
    return 256;
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

    ParameterDescriptor thresholdParam;
    thresholdParam.identifier = "threshold";
    thresholdParam.name = "Threshold";
    thresholdParam.description = "For peak picker.";
    thresholdParam.unit = "";
    thresholdParam.minValue = 0;
    thresholdParam.maxValue = 10;
    thresholdParam.defaultValue = 1;
    thresholdParam.isQuantized = false;
    list.push_back(thresholdParam);

    ParameterDescriptor average_windowParam;
    average_windowParam.identifier = "average_window";
    average_windowParam.name = "Average window length";
    average_windowParam.description = "Length of window used for moving average.";
    average_windowParam.unit = "frames";
    average_windowParam.minValue = 1;
    average_windowParam.maxValue = 500;
    average_windowParam.defaultValue = 200;
    average_windowParam.isQuantized = true;
    average_windowParam.quantizeStep = 1.0;
    list.push_back(average_windowParam);

    ParameterDescriptor peak_windowParam;
    peak_windowParam.identifier = "peak_window";
    peak_windowParam.name = "Peak-picking window length";
    peak_windowParam.description = "Length of window used for peak picking.";
    peak_windowParam.unit = "frames";
    peak_windowParam.minValue = 1;
    peak_windowParam.maxValue = 20;
    peak_windowParam.defaultValue = 6;
    peak_windowParam.isQuantized = true;
    peak_windowParam.quantizeStep = 1.0;
    list.push_back(peak_windowParam);

    ParameterDescriptor autocor_maxParam;
    autocor_maxParam.identifier = "autocor_max";
    autocor_maxParam.name = "Max autocorrelation period";
    autocor_maxParam.description = "Maximum period rendered for autocorrelation.";
    autocor_maxParam.unit = "seconds";
    autocor_maxParam.minValue = 0.1;
    autocor_maxParam.maxValue = 60;
    autocor_maxParam.defaultValue = 5;
    autocor_maxParam.isQuantized = false;
    list.push_back(autocor_maxParam);

    return list;
}

float
Rhythm::getParameter(string identifier) const
{
    if (identifier == "numBands")
        return numBands;
    else if (identifier == "threshold")
        return threshold;
    else if (identifier == "average_window")
        return average_window;
    else if (identifier == "peak_window")
        return peak_window;
    else if (identifier == "autocor_max")
        return autocor_max;
    return 0;
}

void
Rhythm::setParameter(string identifier, float value)
{
    if (identifier == "numBands") {
    	numBands = value;
    	calculateBandFreqs();
    }
    else if (identifier == "threshold")
    {
    	threshold = value;
    }
    else if (identifier == "average_window")
    {
        average_window = (int)value;
    }
    else if (identifier == "peak_window")
    {
        peak_window = (int)value;
    }
    else if (identifier == "autocor_max")
    {
        autocor_max = value;
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

    OutputDescriptor average;
    average.identifier = "average";
    average.name = "Average";
    average.description = "Moving average of onset curve.";
    average.unit = "";
    average.hasFixedBinCount = true;
    average.binCount = 1;
    average.hasKnownExtents = false;
    average.isQuantized = false;
    average.sampleType = OutputDescriptor::VariableSampleRate;
    average.hasDuration = false;
    list.push_back(average);

    OutputDescriptor diff;
    diff.identifier = "diff";
    diff.name = "Difference";
    diff.description = "Difference between onset and average.";
    diff.unit = "";
    diff.hasFixedBinCount = true;
    diff.binCount = 1;
    diff.hasKnownExtents = false;
    diff.isQuantized = false;
    diff.sampleType = OutputDescriptor::VariableSampleRate;
    diff.hasDuration = false;
    list.push_back(diff);

    OutputDescriptor peak;
    peak.identifier = "peak";
    peak.name = "Onset event";
    peak.description = "Peak of onset curve.";
    peak.unit = "";
    peak.hasFixedBinCount = true;
    peak.binCount = 0;
    peak.sampleType = OutputDescriptor::VariableSampleRate;
    list.push_back(peak);

    OutputDescriptor autocor;
    autocor.identifier = "autocor";
    autocor.name = "Autocorrelation";
    autocor.description = "Autocorrelation of onset detection curve.";
    autocor.unit = "";
    autocor.hasFixedBinCount = true;
    autocor.binCount = 1;
    autocor.hasKnownExtents = false;
    autocor.isQuantized = false;
    autocor.sampleType = OutputDescriptor::VariableSampleRate;
    autocor.hasDuration = false;
    list.push_back(autocor);

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
	vector<float> onset;
	float onsetMean = 0;
	for (int frame=0; frame<frames; frame++)
	{
		// reset feature details
		float sum = 0;

		// for each sub-band
		for (int subBand=0; subBand<numBands; subBand++)
		{
			// convolve the canny window with the envelope of that sub-band
			for (int shift=cannyLength*-1; shift<cannyLength; shift++)
			{
				if (frame+shift >= 0 && frame+shift < frames)
					sum += envelope.at(frame+shift).at(subBand) * cannyWindow[shift+cannyLength];
			}
		}

		// save result
		onset.push_back(sum);
		onsetMean += sum;
	}
	onsetMean = onsetMean / frames;

	// find std dev
	float onsetStdDev = 0;
	for (int frame=0; frame<frames; frame++)
	{
		onsetStdDev += pow(onset.at(frame) - onsetMean, 2);
	}
	onsetStdDev = sqrt(onsetStdDev / frames);

	// normalise and export onset curve
	Feature f_onset;
	f_onset.hasTimestamp = true;
	f_onset.hasDuration = false;
	for (int frame=0; frame<frames; frame++)
	{
		// find timestamp and reset feature
		f_onset.timestamp = Vamp::RealTime::frame2RealTime(frame*m_stepSize,m_sampleRate);
		f_onset.values.clear();

		// normalise value
		onset.at(frame) = (onset.at(frame) - onsetMean) / onsetStdDev;

		// half-wave rectification
		if (onset.at(frame) < 0) onset.at(frame) = 0;

		// push result out
		f_onset.values.push_back(onset.at(frame));
		output[0].push_back(f_onset);
	}

	// SIMPLE THRESHOLD
//	Feature f_peak;
//	f_peak.hasTimestamp = true;
//	for (int frame=0; frame<frames; frame++)
//	{
//		if (onset.at(frame) > 1)
//		{
//			f_peak.timestamp = Vamp::RealTime::frame2RealTime(frame*m_stepSize,m_sampleRate);
//			output[2].push_back(f_peak);
//		}
//	}

	// DIXON METHOD
//	int window = 3;
//	int meanMultiplier = 3;
////	float threshold = 0.1;
//	Feature f_peak;
//	f_peak.hasTimestamp = true;
//	for (int frame=0; frame<frames; frame++)
//	{
//		bool success = true;
//
//		// highest value within window
//		for (int i=window*-1; i<window+1; i++)
//		{
//			if (frame+i >= 0 && frame+i < frames)
//				if (onset.at(frame+i) > onset.at(frame)) success = false;
//		}
//
//		// higher than mean + threshold within asymmetric window
//		float mean = 0;
//		for (int i=meanMultiplier*window*-1; i<window+1; i++)
//		{
//			if (frame+i >= 0 && frame+i < frames) mean += onset.at(frame+i);
//		}
//		mean = (mean / ((meanMultiplier * window) + window + 1)) + threshold;
//		if (onset.at(frame) < mean) success = false;
//
//		if (success)
//		{
//			f_peak.timestamp = Vamp::RealTime::frame2RealTime(frame*m_stepSize,m_sampleRate);
//			output[2].push_back(f_peak);
//		}
//	}

	// find moving average of onset
	vector<float> onsetAverage;
	Feature f_avg;
	f_avg.hasTimestamp = true;
	float avgWindowLength = (average_window*2)+1;
	for (int frame=0; frame<frames; frame++)
	{
		f_avg.timestamp = Vamp::RealTime::frame2RealTime(frame*m_stepSize,m_sampleRate);
		f_avg.values.clear();

		float result = 0;
		for (int i=average_window*-1; i<average_window+1; i++)
		{
			if (frame+i >= 0 && frame+i < frames)
				result += abs(onset.at(frame+i));
		}

		// add threshold value and push result
		onsetAverage.push_back(result/avgWindowLength + threshold);
		f_avg.values.push_back(result/avgWindowLength + threshold);
		output[1].push_back(f_avg);
	}

	// find peak picking curve by removing moving average from onset curve
	vector<float> onsetPeak;
	Feature f_diff;
	f_diff.hasTimestamp = true;
	for (int frame=0; frame<frames; frame++)
	{
		f_diff.timestamp = Vamp::RealTime::frame2RealTime(frame*m_stepSize,m_sampleRate);
		f_diff.values.clear();

		// calculate result and apply half-wave rectification
		float result = onset.at(frame)-onsetAverage.at(frame);
		if (result < 0) result = 0;

		// push result out
		onsetPeak.push_back(result);
		f_diff.values.push_back(result);
		output[2].push_back(f_diff);
	}

	// choose peaks
	Feature f_peak;
	f_peak.hasTimestamp = true;
	for (int frame=0; frame<frames; frame++)
	{
		bool success = true;

		// ignore 0 values
		if (onsetPeak.at(frame) <= 0) continue;

		// if any frames within windowSize have a bigger value, this is not the peak
		for (int i=peak_window*-1; i<peak_window+1; i++)
		{
			if (frame+i >= 0 && frame+i < frames)
			{
				if (onsetPeak.at(frame+i) > onsetPeak.at(frame)) success = false;
			}
		}

		// push result out
		if (success)
		{
			f_peak.timestamp = Vamp::RealTime::frame2RealTime(frame*m_stepSize,m_sampleRate);
			output[3].push_back(f_peak);
		}
	}

	// autocorrelation
	vector<float> autocor;
	Feature f_autoCor;
	f_autoCor.hasTimestamp = true;
	for (float shift=1; shift < autocor_max*m_sampleRate/m_stepSize; shift++)
	{
		float result = 0;
		f_autoCor.timestamp = Vamp::RealTime::frame2RealTime(shift*m_stepSize,m_sampleRate);
		f_autoCor.values.clear();
		for (int frame=0; frame<frames; frame++)
		{
			if (frame+shift < frames) result += onsetPeak.at(frame) * onsetPeak.at(frame+shift) / frames;
		}
		autocor.push_back(result);
		f_autoCor.values.push_back(result);
		output[4].push_back(f_autoCor);
	}

    return output;
}

