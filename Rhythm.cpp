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
Rhythm::findRemainder(vector<int> peaks,
		                 int thisPeak)
{
	float total = 0;
	for (unsigned i=0; i<peaks.size(); i++)
	{
		float ratio = (float)peaks.at(i) / (float)thisPeak;
		total += ratio - round(ratio);
	}
	return total;
}

float
Rhythm::findTempo(vector<int> peaks)
{
	float min = findRemainder(peaks, peaks.at(0));
	int minPos = 0;
	for (unsigned i=1; i<peaks.size(); i++)
	{
		float result = findRemainder(peaks, peaks.at(i));
		if (result < min)
		{
			min = result;
			minPos = i;
		}
	}
	return 60.f / (peaks.at(minPos) * m_stepSize / m_sampleRate);
}

float
Rhythm::findMeanPeak(vector<float> signal,
		                vector<int> peaks,
		                int shift)
{
	float total = 0;
	for (unsigned i=0; i<peaks.size(); i++)
		total += signal.at(peaks.at(i)-shift);
	return total / peaks.size();
}

void
Rhythm::findCorrelationPeaks(vector<float> autocor_in,
                                 float percentile_in,
                                 int windowLength_in,
                                 int shift_in,
                                 vector<int>& peaks_out,
                                 vector<int>& valleys_out)
{
	vector<float> autocorSorted (autocor_in);
	std::sort (autocorSorted.begin(), autocorSorted.end());
	float autocorThreshold = autocorSorted.at(percentile_in / 100.f * (autocorSorted.size() - 1));

	int autocorValleyPos = 0;
	float autocorValleyValue = autocorThreshold;

	for (unsigned i=0; i<autocor_in.size(); i++)
	{
		bool success = true;

		// check for valley
		if (autocor_in.at(i) < autocorValleyValue)
		{
			autocorValleyPos = i;
			autocorValleyValue = autocor_in.at(i);
		}

		// if below the threshold, move onto next element
		if (autocor_in.at(i) < autocorThreshold) continue;

		// check for other peaks in the area
		for (int j=windowLength_in*-1; j<windowLength_in+1; j++)
		{
			if (i+j >= 0 && i+j < autocor_in.size())
			{
				if (autocor_in.at(i+j) > autocor_in.at(i)) success = false;
			}
		}

		// save peak and valley
		if (success)
		{
			peaks_out.push_back(shift_in + i);
			valleys_out.push_back(shift_in + autocorValleyPos);
			autocorValleyValue = autocor_in.at(i);
		}
	}
}

void
Rhythm::autocorrelation(vector<float> signal_in,
		                   int startShift_in,
		                   int endShift_in,
		                   vector<float>& autocor_out)
{
	for (float shift = startShift_in; shift < endShift_in; shift++)
	{
		float result = 0;
		for (unsigned frame=0; frame<signal_in.size(); frame++)
		{
			if (frame+shift < signal_in.size()) result += signal_in.at(frame) * signal_in.at(frame+shift);
		}
		autocor_out.push_back(result/signal_in.size());
	}
}

void
Rhythm::findOnsetPeaks(vector<float> onset_in,
		                  int windowLength_in,
		                  vector<int>& peaks_out)
{
	for (unsigned frame=0; frame<onset_in.size(); frame++)
	{
		bool success = true;

		// ignore 0 values
		if (onset_in.at(frame) <= 0) continue;

		// if any frames within windowSize have a bigger value, this is not the peak
		for (int i=windowLength_in*-1; i<windowLength_in+1; i++)
		{
			if (frame+i >= 0 && frame+i < onset_in.size())
			{
				if (onset_in.at(frame+i) > onset_in.at(frame)) success = false;
			}
		}

		// push result out
		if (success)
		{
			peaks_out.push_back(frame);
		}
	}
}

void
Rhythm::movingAverage(vector<float> signal_in,
		                 int windowLength_in,
		                 float threshold_in,
		                 vector<float>& average_out,
		                 vector<float>& difference_out)
{
	float avgWindowLength = (windowLength_in*2)+1;
	for (unsigned frame=0; frame<signal_in.size(); frame++)
	{
		float result = 0;
		for (int i=windowLength_in*-1; i<windowLength_in+1; i++)
		{
			if (frame+i >= 0 && frame+i < signal_in.size())
				result += abs(signal_in.at(frame+i));
		}

		// calculate average and difference results
		float average = result/avgWindowLength + threshold_in;
		float difference = signal_in.at(frame) - average;
		if (difference < 0) difference = 0;

		average_out.push_back(average);
		difference_out.push_back(difference);
	}
}

void
Rhythm::normalise(vector<float> signal_in,
                    vector<float>& normalised_out)
{
	// find mean
	float total = 0;
	for (unsigned i=0; i<signal_in.size(); i++)
		total += signal_in.at(i);
	float mean = total / signal_in.size();

	// find std dev
	float std = 0;
	for (unsigned i=0; i<signal_in.size(); i++)
		std += pow(signal_in.at(i) - mean, 2);
	std = sqrt(std / signal_in.size());

	// normalise and rectify
	for (unsigned i=0; i<signal_in.size(); i++)
	{
		normalised_out.push_back((signal_in.at(i) - mean) / std);
		if (normalised_out.at(i) < 0) normalised_out.at(i) = 0;
	}
}


void
Rhythm::halfHannConvolve(vector< vector<float> >& envelope_out)
{
	for (unsigned frame=0; frame<intensity.size(); frame++)
	{
		vector<float> frameResult;
		for (int subBand=0; subBand<numBands; subBand++)
		{
			float result = 0;
			for (int shift=0; shift<halfHannLength; shift++)
			{
				if (frame+shift < intensity.size()) result += intensity.at(frame+shift).at(subBand) * halfHannWindow[shift];
			}
			frameResult.push_back(result);
		}
		envelope_out.push_back(frameResult);
	}
}

void
Rhythm::cannyConvolve(vector< vector<float> > envelope_in,
		                 vector<float>& onset_out)
{
	for (unsigned frame=0; frame<envelope_in.size(); frame++)
	{
		// reset feature details
		float sum = 0;

		// for each sub-band
		for (int subBand=0; subBand<numBands; subBand++)
		{
			// convolve the canny window with the envelope of that sub-band
			for (int shift=cannyLength*-1; shift<cannyLength; shift++)
			{
				if (frame+shift >= 0 && frame+shift < envelope_in.size())
					sum += envelope_in.at(frame+shift).at(subBand) * cannyWindow[shift+cannyLength];
			}
		}

		// save result
		onset_out.push_back(sum);
	}
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
	halfHannConvolve(envelope);

	// find onset curve by convolving each subband of envelope with canny window
	vector<float> onset;
	cannyConvolve(envelope, onset);

	// normalise onset curve
	vector<float> onsetNorm;
	normalise(onset, onsetNorm);

	// push normalised onset curve
	Feature f_onset;
	f_onset.hasTimestamp = true;
	for (unsigned i=0; i<onsetNorm.size(); i++)
	{
		f_onset.timestamp = Vamp::RealTime::frame2RealTime(i*m_stepSize,m_sampleRate);
		f_onset.values.clear();
		f_onset.values.push_back(onsetNorm.at(i));
		output[0].push_back(f_onset);
	}

	// find moving average of onset curve and difference
	vector<float> onsetAverage;
	vector<float> onsetDiff;
	movingAverage(onsetNorm, average_window, threshold, onsetAverage, onsetDiff);

	// push moving average
	Feature f_avg;
	f_avg.hasTimestamp = true;
	for (unsigned i=0; i<onsetAverage.size(); i++)
	{
		f_avg.timestamp = Vamp::RealTime::frame2RealTime(i*m_stepSize,m_sampleRate);
		f_avg.values.clear();
		f_avg.values.push_back(onsetAverage.at(i));
		output[1].push_back(f_avg);
	}

	// push difference from average
	Feature f_diff;
	f_diff.hasTimestamp = true;
	for (unsigned i=0; i<onsetDiff.size(); i++)
	{
		f_diff.timestamp = Vamp::RealTime::frame2RealTime(i*m_stepSize,m_sampleRate);
		f_diff.values.clear();
		f_diff.values.push_back(onsetDiff.at(i));
		output[2].push_back(f_diff);
	}

	// choose peaks
	vector<int> peaks;
	findOnsetPeaks(onsetDiff, peak_window, peaks);
	int onsetCount = (int)peaks.size();

	// push peaks
	Feature f_peak;
	f_peak.hasTimestamp = true;
	for (unsigned i=0; i<peaks.size(); i++)
	{
		f_peak.timestamp = Vamp::RealTime::frame2RealTime(peaks.at(i)*m_stepSize, m_sampleRate);
		output[3].push_back(f_peak);
	}

	// calculate average onset frequency
	float averageOnsetFreq = (float)onsetCount / (float)(frames*m_stepSize/m_sampleRate);
	Feature f_avgOnsetFreq;
	f_avgOnsetFreq.hasTimestamp = true;
	f_avgOnsetFreq.timestamp = Vamp::RealTime::fromSeconds(0.0);
	f_avgOnsetFreq.values.push_back(averageOnsetFreq);
	output[4].push_back(f_avgOnsetFreq);

	// calculate rhythm strength
	float rhythmStrength = findMeanPeak(onset, peaks, 0);
	Feature f_rhythmStrength;
	f_rhythmStrength.hasTimestamp = true;
	f_rhythmStrength.timestamp = Vamp::RealTime::fromSeconds(0.0);
	f_rhythmStrength.values.push_back(rhythmStrength);
	output[5].push_back(f_rhythmStrength);

	// find shift range for autocor
	float firstShift = (int)round(60.f/max_bpm*m_sampleRate/m_stepSize);
	float lastShift = (int)round(60.f/min_bpm*m_sampleRate/m_stepSize);

	// autocorrelation
	vector<float> autocor;
	autocorrelation(onsetDiff, firstShift, lastShift, autocor);
	Feature f_autoCor;
	f_autoCor.hasTimestamp = true;
	for (float shift = firstShift; shift < lastShift; shift++)
	{
		f_autoCor.timestamp = Vamp::RealTime::frame2RealTime(shift*m_stepSize,m_sampleRate);
		f_autoCor.values.clear();
		f_autoCor.values.push_back(autocor.at(shift-firstShift));
		output[6].push_back(f_autoCor);
	}

	// find peaks in autocor
	float percentile = 95;
	int autocorWindowLength = 3;
	vector<int> autocorPeaks;
	vector<int> autocorValleys;
	findCorrelationPeaks(autocor, percentile, autocorWindowLength, firstShift, autocorPeaks, autocorValleys);

	// find average corrolation peak
	float meanCorrelationPeak = findMeanPeak(autocor, autocorPeaks, firstShift);
	Feature f_meanCorrelationPeak;
	f_meanCorrelationPeak.hasTimestamp = true;
	f_meanCorrelationPeak.timestamp = Vamp::RealTime::fromSeconds(0.0);
	f_meanCorrelationPeak.values.push_back(meanCorrelationPeak);
	output[7].push_back(f_meanCorrelationPeak);

	// find peak/valley ratio
	float meanCorrelationValley = findMeanPeak(autocor, autocorValleys, firstShift) + 0.0001;
	Feature f_peakValleyRatio;
	f_peakValleyRatio.hasTimestamp = true;
	f_peakValleyRatio.timestamp = Vamp::RealTime::fromSeconds(0.0);
	f_peakValleyRatio.values.push_back(meanCorrelationPeak / meanCorrelationValley);
	output[8].push_back(f_peakValleyRatio);

	// find tempo from peaks
	float tempo = findTempo(autocorPeaks);
	Feature f_tempo;
	f_tempo.hasTimestamp = true;
	f_tempo.timestamp = Vamp::RealTime::fromSeconds(0.0);
	f_tempo.values.push_back(tempo);
	output[9].push_back(f_tempo);

    return output;
}

