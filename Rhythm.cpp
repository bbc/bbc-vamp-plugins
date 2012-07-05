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
	max_bpm = 300;
	min_bpm = 12;
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

    ParameterDescriptor min_bpmParam;
    min_bpmParam.identifier = "min_bpm";
    min_bpmParam.name = "Minimum calculated BPM";
    min_bpmParam.description = "Minimum BPM calculated for autocorrelation.";
    min_bpmParam.unit = "bpm";
    min_bpmParam.minValue = 5;
    min_bpmParam.maxValue = 300;
    min_bpmParam.defaultValue = 12;
    min_bpmParam.isQuantized = true;
    min_bpmParam.quantizeStep = 1.0;
    list.push_back(min_bpmParam);

    ParameterDescriptor max_bpmParam;
    max_bpmParam.identifier = "max_bpm";
    max_bpmParam.name = "Maximum calculated BPM";
    max_bpmParam.description = "Maximum BPM calculated for autocorrelation.";
    max_bpmParam.unit = "bpm";
    max_bpmParam.minValue = 50;
    max_bpmParam.maxValue = 400;
    max_bpmParam.defaultValue = 300;
    max_bpmParam.isQuantized = true;
    max_bpmParam.quantizeStep = 1.0;
    list.push_back(max_bpmParam);

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
    else if (identifier == "min_bpm")
        return min_bpm;
    else if (identifier == "max_bpm")
        return max_bpm;
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
    else if (identifier == "min_bpm")
    {
        min_bpm = (int)value;
    }
    else if (identifier == "max_bpm")
    {
        max_bpm = (int)value;
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

    OutputDescriptor onset_curve;
    onset_curve.identifier = "onset_curve";
    onset_curve.name = "Onset curve";
    onset_curve.description = "Onset detection curve.";
    onset_curve.unit = "";
    onset_curve.hasFixedBinCount = true;
    onset_curve.binCount = 1;
    onset_curve.hasKnownExtents = false;
    onset_curve.isQuantized = false;
    onset_curve.sampleType = OutputDescriptor::VariableSampleRate;
    onset_curve.hasDuration = false;
    list.push_back(onset_curve);

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

    OutputDescriptor onset;
    onset.identifier = "onset";
    onset.name = "Onset";
    onset.description = "Point of onsets.";
    onset.unit = "";
    onset.hasFixedBinCount = true;
    onset.binCount = 0;
    onset.sampleType = OutputDescriptor::VariableSampleRate;
    list.push_back(onset);

    OutputDescriptor avg_onset_freq;
    avg_onset_freq.identifier = "avg-onset-freq";
    avg_onset_freq.name = "Average Onset Frequency";
    avg_onset_freq.description = "Rate of onsets per minute.";
    avg_onset_freq.unit = "";
    avg_onset_freq.hasFixedBinCount = true;
    avg_onset_freq.binCount = 1;
    avg_onset_freq.sampleType = OutputDescriptor::VariableSampleRate;
    avg_onset_freq.hasKnownExtents = false;
    avg_onset_freq.isQuantized = false;
    avg_onset_freq.hasDuration = false;
    list.push_back(avg_onset_freq);

    OutputDescriptor rhythm_strength;
    rhythm_strength.identifier = "rhythm-strength";
    rhythm_strength.name = "Rhythm Strength";
    rhythm_strength.description = "Average value of peaks in onset curve.";
    rhythm_strength.unit = "";
    rhythm_strength.hasFixedBinCount = true;
    rhythm_strength.binCount = 1;
    rhythm_strength.sampleType = OutputDescriptor::VariableSampleRate;
    rhythm_strength.hasKnownExtents = false;
    rhythm_strength.isQuantized = false;
    rhythm_strength.hasDuration = false;
    list.push_back(rhythm_strength);

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

    OutputDescriptor mean_correlation_peak;
    mean_correlation_peak.identifier = "mean-correlation-peak";
    mean_correlation_peak.name = "Mean Correlation Peak";
    mean_correlation_peak.description = "Mean of the peak autocorrelation values.";
    mean_correlation_peak.unit = "";
    mean_correlation_peak.hasFixedBinCount = true;
    mean_correlation_peak.binCount = 1;
    mean_correlation_peak.hasKnownExtents = false;
    mean_correlation_peak.isQuantized = false;
    mean_correlation_peak.sampleType = OutputDescriptor::VariableSampleRate;
    mean_correlation_peak.hasDuration = false;
    list.push_back(mean_correlation_peak);

    OutputDescriptor peak_valley_ratio;
    peak_valley_ratio.identifier = "peak-valley-ratio";
    peak_valley_ratio.name = "Peak-Valley Ratio";
    peak_valley_ratio.description = "Ratio of the mean correlation peak to the mean correlation valley.";
    peak_valley_ratio.unit = "";
    peak_valley_ratio.hasFixedBinCount = true;
    peak_valley_ratio.binCount = 1;
    peak_valley_ratio.hasKnownExtents = false;
    peak_valley_ratio.isQuantized = false;
    peak_valley_ratio.sampleType = OutputDescriptor::VariableSampleRate;
    peak_valley_ratio.hasDuration = false;
    list.push_back(peak_valley_ratio);

    OutputDescriptor tempo;
    tempo.identifier = "tempo";
    tempo.name = "Tempo";
    tempo.description = "Overall tempo of the track in BPM.";
    tempo.unit = "bpm";
    tempo.hasFixedBinCount = true;
    tempo.binCount = 1;
    tempo.hasKnownExtents = false;
    tempo.isQuantized = false;
    tempo.sampleType = OutputDescriptor::VariableSampleRate;
    tempo.hasDuration = false;
    list.push_back(tempo);

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

float
Rhythm::peakRegularity(vector<int> peaks, int thisPeak)
{
	float total = 0;
	for (unsigned i=0; i<peaks.size(); i++)
	{
		float ratio = (float)peaks.at(i) / (float)thisPeak;
		total += ratio - round(ratio);
	}
	return total;
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
	vector<float> onsetNorm;
	f_onset.hasTimestamp = true;
	f_onset.hasDuration = false;
	for (int frame=0; frame<frames; frame++)
	{
		// find timestamp and reset feature
		f_onset.timestamp = Vamp::RealTime::frame2RealTime(frame*m_stepSize,m_sampleRate);
		f_onset.values.clear();

		// normalise value
		onsetNorm.push_back((onset.at(frame) - onsetMean) / onsetStdDev);

		// half-wave rectification
		if (onsetNorm.at(frame) < 0) onsetNorm.at(frame) = 0;

		// push result out
		f_onset.values.push_back(onsetNorm.at(frame));
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
				result += abs(onsetNorm.at(frame+i));
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
		float result = onsetNorm.at(frame)-onsetAverage.at(frame);
		if (result < 0) result = 0;

		// push result out
		onsetPeak.push_back(result);
		f_diff.values.push_back(result);
		output[2].push_back(f_diff);
	}

	// choose peaks
	Feature f_peak;
	vector<int> peaks;
	f_peak.hasTimestamp = true;
	int onsetCount = 0;
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
			peaks.push_back(frame);
			output[3].push_back(f_peak);
			onsetCount++;
		}
	}

	// calculate average onset frequency
	Feature f_avgOnsetFreq;
	f_avgOnsetFreq.hasTimestamp = true;
	f_avgOnsetFreq.timestamp = Vamp::RealTime::fromSeconds(0.0);
	f_avgOnsetFreq.values.push_back((float)onsetCount / (float)(frames*m_stepSize/m_sampleRate));
	output[4].push_back(f_avgOnsetFreq);

	// calculate rhythm strength
	Feature f_rhythmStrength;
	f_rhythmStrength.hasTimestamp = true;
	f_rhythmStrength.timestamp = Vamp::RealTime::fromSeconds(0.0);
	float total = 0;
	for (unsigned i=0; i<peaks.size(); i++)
	{
		total += onset.at(peaks.at(i));
	}
	f_rhythmStrength.values.push_back(total/(float)peaks.size());
	output[5].push_back(f_rhythmStrength);

	// find shift range for autocor
	float firstShift = (int)round(60.f/max_bpm*m_sampleRate/m_stepSize);
	float lastShift = (int)round(60.f/min_bpm*m_sampleRate/m_stepSize);

	// autocorrelation
	vector<float> autocor;
	Feature f_autoCor;
	f_autoCor.hasTimestamp = true;
	for (float shift = firstShift; shift < lastShift; shift++)
	{
		float result = 0;
		f_autoCor.timestamp = Vamp::RealTime::frame2RealTime(shift*m_stepSize,m_sampleRate);
		f_autoCor.values.clear();
		for (int frame=0; frame<frames; frame++)
		{
			if (frame+shift < frames) result += onsetPeak.at(frame) * onsetPeak.at(frame+shift);
		}
		autocor.push_back(result/frames);
		f_autoCor.values.push_back(result/frames);
		output[6].push_back(f_autoCor);
	}

	// find peaks in autocor
	float percentile = 95;
	int autocorValleyPos = 0;
	int autocorWindowLength = 3;
	vector<float> autocorSorted (autocor);
	vector<int> autocorPeaks;
	vector<int> autocorValleys;
	std::sort (autocorSorted.begin(), autocorSorted.end());
	float autocorThreshold = autocorSorted.at(percentile / 100.f * (autocorSorted.size() - 1));
	float autocorValleyValue = autocorThreshold;
//	std::cout << "Threshold is: " << autocorThreshold << "\n";
	for (unsigned i=0; i<autocor.size(); i++)
	{
		bool success = true;

		// check for valley
		if (autocor.at(i) < autocorValleyValue)
		{
			autocorValleyPos = i;
			autocorValleyValue = autocor.at(i);
		}

		// if below the threshold, move onto next element
		if (autocor.at(i) < autocorThreshold) continue;

		// check for other peaks in the area
		for (int j=autocorWindowLength*-1; j<autocorWindowLength+1; j++)
		{
			if (i+j >= 0 && i+j < autocor.size())
			{
				if (autocor.at(i+j) > autocor.at(i)) success = false;
			}
		}

		// save peak and valley
		if (success)
		{
			autocorPeaks.push_back(firstShift + i);
			autocorValleys.push_back(firstShift + autocorValleyPos);
			autocorValleyValue = autocor.at(i);
		}
	}

	// find average corrolation peak
	total = 0;
	for (unsigned i=0; i<autocorPeaks.size(); i++)
	{
		total += autocor.at(autocorPeaks.at(i)-firstShift);
//		std::cout << "Peak at " << autocorPeaks.at(i) << ": " << autocor.at(autocorPeaks.at(i)-firstShift) << "\n";
	}
	float meanCorrelationPeak = total / autocorPeaks.size();
	Feature f_meanCorrelationPeak;
	f_meanCorrelationPeak.hasTimestamp = true;
	f_meanCorrelationPeak.timestamp = Vamp::RealTime::fromSeconds(0.0);
	f_meanCorrelationPeak.values.push_back(meanCorrelationPeak);
	output[7].push_back(f_meanCorrelationPeak);

	// find peak/valley ratio
	total = 0;
	for (unsigned i=0; i<autocorValleys.size(); i++)
	{
		total += autocor.at(autocorValleys.at(i)-firstShift);
//		std::cout << "Valley at " << autocorValleys.at(i) << ": " << autocor.at(autocorValleys.at(i)-firstShift) << "\n";
	}
	Feature f_peakValleyRatio;
	f_peakValleyRatio.hasTimestamp = true;
	f_peakValleyRatio.timestamp = Vamp::RealTime::fromSeconds(0.0);
	f_peakValleyRatio.values.push_back(meanCorrelationPeak / (total / autocorValleys.size() + 0.0001));
	output[8].push_back(f_peakValleyRatio);

	// find largest divisor
	float min = peakRegularity(autocorPeaks, autocorPeaks.at(0));
	int minPos = 0;
	for (unsigned i=1; i<autocorPeaks.size(); i++)
	{
		float result = peakRegularity(autocorPeaks, autocorPeaks.at(i));
		if (result < min)
		{
			min = result;
			minPos = i;
		}
	}
	float bpm = 60.f/(autocorPeaks.at(minPos)*m_stepSize/m_sampleRate);
	Feature f_tempo;
	f_tempo.hasTimestamp = true;
	f_tempo.timestamp = Vamp::RealTime::fromSeconds(0.0);
	f_tempo.values.push_back(bpm);
	output[9].push_back(f_tempo);

    return output;
}

