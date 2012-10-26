#ifndef _CONTRAST_H_
#define _CONTRAST_H_

#include <cmath>
#include <complex>
#include <algorithm>
#include <vector>
#include <vamp-sdk/Plugin.h>

using std::string;
using std::vector;
using std::complex;
using std::abs;

/*!
 * \brief Calculates the spectral flux
 */
class SpectralContrast : public Vamp::Plugin
{
public:
	/// @cond
    SpectralContrast(float inputSampleRate);
    virtual ~SpectralContrast();
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
    void setParameter(string identifier,
    		            float value);
    ProgramList getPrograms() const;
    string getCurrentProgram() const;
    void selectProgram(string name);
    OutputList getOutputDescriptors() const;
    bool initialise(size_t channels,
    		         size_t stepSize,
    		         size_t blockSize);
    void reset();
    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);
    FeatureSet getRemainingFeatures();
    void calculateBandFreqs();
    /// @endcond

protected:
    /// @cond
    int m_blockSize, m_stepSize;
    float m_sampleRate;
    float alpha;
    /// @endcond

    int numBands;     /*!< Number of sub-bands to use */
    float *bandHighFreq;  /*!< Upper frequency range of each sub-band */
};

#endif
