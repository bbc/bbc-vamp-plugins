
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
    float peakRegularity(vector<int> peaks, int thisPeak);

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
