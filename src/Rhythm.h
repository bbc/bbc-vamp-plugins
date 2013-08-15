/**
 * BBC Vamp plugin collection
 *
 * Copyright (c) 2011-2013 British Broadcasting Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _RHYTHM_H_
#define _RHYTHM_H_

#define _USE_MATH_DEFINES

#include <cmath>
#include <complex>
#include <vector>
#include <algorithm>
#include <vamp-sdk/Plugin.h>

using std::string;
using std::vector;
using std::complex;
using std::abs;
using std::cos;

/*!
 * \brief Calculates rhythmic features of a signal, including onsets and tempo
 *
 * \section Outputs
 * \par Onset Curve
 * The filtered and half-wave rectified intensity of the signal, used to detect
 * onsets.
 * \par Average
 * The moving average of the onset curve, plus the threshold - used for
 * selecting where the peaks of the onset curve are.
 * \par Difference
 * The difference between the onset curve and its moving average. Used as the
 * input for peak-picking.
 * \par Onset
 * The detected note onsets.
 * \par Average onset frequency
 * The mean number of onsets per minute.
 * \par Rhythm strength
 * The mean value of the peaks in the onset curve.
 * \par Autocorrelation
 * The autocorrelation of the difference curve.
 * \par Mean Correlation Peak
 * The mean value of the peaks in the autocorrelation.
 * \par Peak-Valley Ratio
 * The mean peak-valley ratio of the autocorrelation.
 * \par Tempo
 * The estimated tempo in beats per minute.
 *
 * \section Parameters
 * \par Sub-bands
 * Number of sub-bands to divide the signal into for applying the half-hanning
 * window. A higher increases accuracy at the cost of processing time.
 * (default = 7)
 * \par Threshold
 * Amount by which to increase the moving average filter. A higher number
 * produces fewer onsets. (default = 1.0)
 * \par Moving average window length
 * Length of moving average window. A higher number produces a smoother curve.
 * (default = 200)
 * \par Onset peak window length
 * Length of window used to select peaks in the difference curve. (default = 6)
 * \par Minimum BPM
 * Minimum tempo calculated using the autocorrelation. (default = 12)
 * \par Maximum BPM
 * Maximum tempo calculated using the autocorrelation. (default = 300)
 *
 * \section Description
 *
 * The rhythm features are based on the features described in [1] (section 3C),
 * combined with some techniques from [2].
 *
 * Firstly the spectrum is divided into \f$n\f$ sub-bands with the following
 * frequency ranges.
 * \f[ \left(0,\frac{F_s}{2^n}\right) , \left(\frac{F_s}{2^n},
 * \frac{F_s}{2^{n-1}}\right) , \ldots \left(\frac{F_s}{2^2},
 * \frac{F_s}{2^1}\right) \f]
 *
 * For each sub-band, the magnitude of the FFT bins are summed, producing
 * \f$n\f$ signals. Each of the signals are convolved with a half-hanning
 * window, where \f$L\f$ is set as 12.
 * \f[ H(w) = 0.5 + 0.5\cos\left(2\pi \cdot \frac{w}{2L-1} \right) \hspace{20px}
 * w\in[0, L-1] \f]
 *
 * Subsequently, each of the signals are convolved with a peak-enhancing canny
 * window, where
 * \f$L\f$ is set as 12 and \f$\sigma\f$ is set as 4.
 * \f[ C(w) = \frac{w}{\sigma^2}e^{-\frac{w^2}{2\sigma^2}} \hspace{20px}
 * w\in[-L,L] \f]
 *
 * The \f$n\f$ signals are summed and half-wave rectified to produce the
 * <b>onset curve</b>.
 *
 * The <b>moving average</b> \f$A\f$ of the onset curve \f$O\f$ is produced from
 * the mean value of a rectangular window of length \f$(2L+1)\f$, plus a
 * threshold \f$t\f$. The <i>threshold</i> and <i>moving average window
 * length</i> parameters control \f$t\f$ and \f$L\f$ respectively.
 * \f[ A(x) = \displaystyle\sum\limits_{y=-L}^{L} \frac{O(x+y)}{2L+1} + t \f]
 *
 * The <b>difference</b> signal is created by subtracting the moving average
 * from the onset curve and applying half-wave rectification.
 *
 * An <b>onset</b> is detected when a sample is the maximum within a given
 * window of length \f$(2L+1)\f$, where \f$L\f$ is set by the parameter <i>onset
 * peak window length</i>.
 *
 * The <b>average onset frequency</b> is the total number of onsets divided by
 * the length of the track in minutes.
 *
 * The <b>rhythm strength</b> is the mean value of the peaks of the onset curve
 * (pre-averaging).
 *
 * The <b>autocorrelation</b> is the autocorrelation of the difference signal
 * between delays of \f$\frac{60}{T_{max}}\cdot\frac{F_s}{s}\f$ frames and
 * \f$\frac{60}{T_{min}}\cdot\frac{F_s}{s}\f$ frames, where \f$T_{min}\f$ and
 * \f$T_{max}\f$ are the min/max tempo in BPM and \f$s\f$ is the step size in
 * number of frames.
 *
 * The peaks of the autocorrelation - \f$P_i\f$ - are defined as those which are
 * above a certain threshold, defined as the 95% confidence interval, and whose
 * value is the maximum within a 7-sample window. The <b>mean correleation
 * peak</b> is the mean value of the selected peaks, and the <b>peak-valley
 * ratio</b> is the ratio between the mean correlation peak and the mean value
 * of the valleys. A valley is defined as the minimum value between two peaks.
 *
 * The <b>tempo</b> is defined as the maximum common divisor of the detected
 * peaks. It is found by minimising the function below:
 * \f[ T = \underset{P_k}{argmin} \displaystyle\sum\limits_{i=1}^{N}
 * \left|\frac{P_i}{P_k}-\text{round}\left(\frac{P_i}{P_k}\right)\right|\f]
 *
 * \section References
 *
 * [1] <i>Lu, L., Liu, D., & Zhang, H.-J. (2006). Automatic Mood Detection and
 * Tracking of Music Audio Signals. IEEE Transactions on Audio, Speech and
 * Language Processing (Vol. 14, pp. 5-18).﻿</i>
 *
 * [2] <i>﻿Dixon, S. (2006). Onset Detection Revisited. International Conference
 * on Digital Audio Effects (DAFx) (pp. 133-137).</i>
 */
class Rhythm : public Vamp::Plugin {
 public:
  /// @cond
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
  FeatureSet process(const float * const *inputBuffers,
                     Vamp::RealTime timestamp);
  FeatureSet getRemainingFeatures();
  /// @endcond

 protected:
  void calculateBandFreqs();
  float halfHanning(float n);
  float canny(float n);
  float findRemainder(vector<int> peaks, int thisPeak);
  void findTempo(vector<int> peaks, vector<float> &result);
  float findMeanPeak(vector<float> signal, vector<int> peaks, int shift);
  void findCorrelationPeaks(vector<float> autocor_in, float percentile_in,
                            int windowLength_in, int shift_in,
                            vector<int>& peaks_out, vector<int>& valleys_out);
  void autocorrelation(vector<float> signal_in, int startShift_in,
                       int endShift_in, vector<float>& autocor_out);
  void findOnsetPeaks(vector<float> onset_in, int windowLength_in,
                      vector<int>& peaks_out);
  void movingAverage(vector<float> signal_in, int windowLength_in,
                     float threshold_in, vector<float>& average_out,
                     vector<float>& difference_out);
  void normalise(vector<float> signal_in, vector<float>& normalised_out);
  void halfHannConvolve(vector<vector<float> >& envelope_out);
  void cannyConvolve(vector<vector<float> > envelope_in,
                     vector<float>& onset_out);

  /// @cond
  int m_blockSize, m_stepSize;
  float m_sampleRate;
  /// @endcond

  int numBands;         /*!< Number of sub-bands */
  float *bandHighFreq;  /*!< Upper frequency of each sub-band */
  int halfHannLength;   /*!< Length of half-hanning window */
  float *halfHannWindow;/*!< Co-efficients of half-hanning window */
  int cannyLength;      /*!< Length of canny window */
  float cannyShape;     /*!< Shape of canny window */
  float *cannyWindow;   /*!< Co-efficients of canny window */
  vector<vector<float> > intensity; /*!< Intensity value for each block */
  float threshold;      /*!< Theshold value added to moving average */
  int average_window;   /*!< Length of moving average window */
  int peak_window;      /*!< Length of peak-picking window */
  int max_bpm;          /*!< Maximum BPM detected in autocorrelation */
  int min_bpm;          /*!< Minimum BPM detected in autocorrelation */
};

#endif
