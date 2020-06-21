#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <vector>

#include "kissfft/kiss_fft.h"

// ADC properties (12 bit, differential input, +-500mA max input)
const int sampling = 4000;
// Max input 500V over 12 bit hence each step = 500 / 2^12 = 0.1221V
const float stepSize = 0.1221;
const int lsbError = 3;

// Signals to be generated and analysed
const int window = 800; // 800 samples from the ADC will be taken
const int fundamentalFrequency = 50;
const std::map<int, int> harmonics = {{2, 50}, {4, 25}, {5, 10}, {39, 5}, {40, 10}};

// Random number
std::random_device rd;
std::mt19937 numberGenerator(rd());
std::uniform_int_distribution<int> randomLsb(-lsbError, lsbError);

/**
 * Round a value to the nearest step size
 *
 * @param value exact value
 * @param multiple step size
 * @return float output of step size
 */
float roundUp(float value, float multiple)
{
    if(multiple == 0.0f)
        return value;

    return std::round(value / multiple) * multiple;
}

/**
 * Makes a sine wave harmonics
 */
std::vector<float> makeWave(int mag, int position)
{
    // Calculate fundiemntals
    std::vector<float> wave;

    int windowCount = 0;

    // Create a point for every sample
    int samples = sampling / fundamentalFrequency;
    for(int i = 0; i <= samples; i++)
    {
        // Convert degrees
        double degrees = i * (360.0f / samples);

        // Convert degrees to radians
        double radians = degrees * (3.14159265359f / 180.0f);

        // Exact value
        float value = mag * sin(position * radians);

        // Convert value to a possiable 12 bit ADC value
        roundUp(value, stepSize);

        // Create and lsb error and add to value
        int randomLsbValue = randomLsb(numberGenerator);
        float lsb = stepSize * randomLsbValue;
        value += lsb;

        // Calculate sample
        wave.push_back(value);

        // Increase window sample count
        windowCount++;

        // Does sine wave need setting back to 0
        if(i == samples && windowCount < window)
            i = 0;

        // Size of window
        if(windowCount == window)
            break;
    }

    return wave;
}

/**
 * Calculates complex wave from multiple harmonics
 */
template <typename... Waves>
std::vector<float> formComplex(Waves... wave)
{
    std::vector<float> out;

    for(int i = 0; i <= window; i++)
    {
        float value = roundf((wave[i] + ...) * 10000) / 10000;
        out.push_back(value);
    }

    return out;
}

/**
 * Runs FFT on a complex wave form using KissFFT
 *
 * @param complex waveform
 * @return
 */
std::vector<kiss_fft_cpx> fft(std::vector<float>& complex)
{
    // FFT containers
    kiss_fft_cpx in[window], out[window];

    // Load values into input container
    for(int i = 0; i < complex.size(); i++)
    {
        in[i].r = complex[i];
        in[i].i = 0;
    }

    // Allocate memory for FFt
    kiss_fft_cfg cfg = kiss_fft_alloc(window, 0 /*is_inverse_fft*/, nullptr, nullptr);

    // If FFT allocation was successful
    if(cfg != nullptr)
    {
        // Run FFT
        kiss_fft(cfg, in, out);

        // Free the allocated memory
        free(cfg);
    }
    else
    {
        // Fail and bomb out
        std::cout << "Insufficient memory for FFT";
        exit(-1);
    }

    // Return vector of complex numbers outputted from FFT
    return std::vector<kiss_fft_cpx>(out, out + sizeof out / sizeof out[0]);
}

/**
 * Convert all FFT points into frequency vs voltage
 *
 * @param out FFT output
 */
void fullFFT(std::vector<kiss_fft_cpx>& out)
{
    // Find the magnitude of the harmonic using the fft
    // We only use half the data as the second half of the wave form is the same
    // as the first
    std::vector<float> magnitude;
    for(int i = 1; i < out.size() / 2; i++)
    {
        // Find magnitude
        float mag = sqrt(pow(out[i].r / window, 2) + pow(out[i].i / window, 2));

        // Multiple magnitude by two to account for the second half of the wave form
        mag *= 2;

        // Find magnitude from complex numbers
        magnitude.push_back(mag);
    }

    // Create x axis in Hz by using info from sampling rate and window size
    std::vector<float> xValues;
    for(int k = 0; k < (window / 2) + 1; k++)
        xValues.push_back((sampling * k) / window);

    // Find the peaks
    for(int q = 0; q < magnitude.size(); q++)
    {
        // We dont care about noise, hence values less than the ADC can read should
        // be ignored
        if(magnitude.at(q) > stepSize)
            std::cout << xValues.at(q + 1) << "Hz = " << magnitude.at(q) << "V" << std::endl;
    }
}

/**
 * Convert and save just the peaks within the FFT output
 *
 * @param out
 */
std::map<float, float> findFFTPeaks(std::vector<kiss_fft_cpx>& out)
{
    // Find the magnitude of the harmonic using the fft
    // We only use half the data as the second half of the wave form is the same
    // as the first
    std::map<float, float> fftPeaks;
    for(int i = 1; i < out.size() / 2; i++)
    {
        // Find magnitude
        float mag = sqrt(pow(out[i].r / window, 2) + pow(out[i].i / window, 2));

        // Multiple magnitude by two to account for the second half of the wave form
        mag *= 2;

        // We dont care about noise, hence values less than the ADC can read should
        // be ignored Note: we're only dealing witb the positive half of the wave
        // form so the below is ok
        if(mag > (stepSize * lsbError))
        {
            // Get the frequency for the current FFT value
            float freq = (sampling * i) / window;

            // Save the frequency vs magnitude
            fftPeaks[freq] = mag;
        }
    }

    return fftPeaks;
}

int main()
{
    // Make a complex wave, in the real world this could be sampled via the ADC
    std::vector<float> complex = makeWave(315, 1);
    for(const auto& harmonid : harmonics)
        complex = formComplex(complex, makeWave(harmonid.second, harmonid.first));

    // @todo add noise, add jitter

    // Run FFT
    std::vector<kiss_fft_cpx> out = fft(complex);

    // Find the peaks in the FFT
    auto peaks = findFFTPeaks(out);

    // Print the results
    for(auto& peak : peaks)
    {
        std::cout << peak.first << "Hz = " << peak.second << "V" << std::endl;
    }
}
