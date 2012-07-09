#ifndef _FLUX_H_
#define _FLUX_H_

#include <cmath>
#include <complex>
#include <vector>
#include <vamp-sdk/Plugin.h>

using std::string;
using std::vector;
using std::complex;
using std::abs;

/*!
 * \brief Calculates the spectral flux
 */
class SpectralFlux : public Vamp::Plugin
{
public:
	/// @cond
    SpectralFlux(float inputSampleRate);
    virtual ~SpectralFlux();
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
    /// @endcond

protected:
    /// @cond
    int m_blockSize, m_stepSize;
    /// @endcond

    bool l2norm;	//!< Flag to indicate use of L2 normalisation
};

#endif
