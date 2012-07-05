
// This is a skeleton file for use in creating your own plugin
// libraries.  Replace MyPlugin and myPlugin throughout with the name
// of your first plugin class, and fill in the gaps as appropriate.


// Remember to use a different guard symbol in each header!
#ifndef _RHYTHM_H_
#define _RHYTHM_H_

#define _USE_MATH_DEFINES

#include <iostream>
#include <cmath>
#include <complex>
#include <vector>
#include <algorithm>
#include <vamp-sdk/Plugin.h>

using std::string;
using std::vector;
using std::complex;
using std::abs;
using std::cout;
using std::cos;

class Rhythm : public Vamp::Plugin
{
public:
    Rhythm(float inputSampleRate);
    virtual ~Rhythm();

    string getIdentifier() const;
    string getName() const;
    string getDescription() const;
    string getMaker() const;
    int getPluginVersion() const;
    string getCopyright() const;

    InputDomain getInputDomain() const;
    size_t getPreferredBlockSize() const;
    size_t getPreferredStepSize() const;
    size_t getMinChannelCount() const;
    size_t getMaxChannelCount() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(string identifier) const;
    void setParameter(string identifier, float value);

    ProgramList getPrograms() const;
    string getCurrentProgram() const;
    void selectProgram(string name);

    OutputList getOutputDescriptors() const;

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();
    void calculateBandFreqs();
    float halfHanning(float n);
    float canny(float n);
    float findRemainder(vector<int> peaks,
                          int thisPeak);
    float findTempo(vector<int> peaks);
    float findMeanPeak(vector<float> signal,
    		             vector<int> peaks,
    		             int shift);
    void findCorrelationPeaks(vector<float> autocor_in,
    		                     float percentile_in,
    		                     int windowLength_in,
    		                     int shift_in,
    		                     vector<int>& peaks_out,
    		                     vector<int>& valleys_out);
    void autocorrelation(vector<float> signal_in,
    		                int startShift_in,
    		                int endShift_in,
    		                vector<float>& autocor_out);
    void findOnsetPeaks(vector<float> onset_in,
    		           int windowLength_in,
    		           vector<int>& peaks_out);
    void movingAverage(vector<float> signal_in,
                         int windowLength_in,
                         float threshold_in,
                         vector<float>& average_out,
                         vector<float>& difference_out);
    void normalise(vector<float> signal_in,
                     vector<float>& normalised_out);
    void halfHannConvolve(vector< vector<float> >& envelope_out);
    void cannyConvolve(vector< vector<float> > envelope_in, vector<float>& onset_out);
    FeatureSet process(const float *const *inputBuffers,
                        Vamp::RealTime timestamp);
    FeatureSet getRemainingFeatures();

protected:
    int m_blockSize, m_stepSize;
    float m_sampleRate;
    int numBands;
    float *bandHighFreq;
    int halfHannLength;
    float *halfHannWindow;
    int cannyLength;
    float cannyShape;
    float *cannyWindow;
    vector< vector<float> > intensity;
    float threshold;
    int average_window;
    int peak_window;
	int max_bpm;
	int min_bpm;
};



#endif
