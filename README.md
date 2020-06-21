# FFT of a Complex Waveform

The aim of this mini-project is to show how FFT can be performed on a complex wave. 

The project takes into account the restrictions and limitations of a ADC:
  * Resolution: 12 bit
  * Sampling: 4kHz
  * LSB: +-3
 
The complex wave used for FFT is created on boot using the following specifications:
  * Fundamental: 220Vrms / 315V peak at 50Hz
  * Harmonics:
    * 2nd at 50V
    * 4th at 25V
    * 5th at 10V
    * 39th at 5V
    * 40th at 10V
  * Rounding of waveform samples to the nearest 12 bit resolution
  
**Disclaimer:** it was not the aim of this project to generate clean, efficient or modern C++ code. It's
purely an educational project and a tool to show the limitation of FFT when using an imperfect sampled wave form.
  
## Requirements

  * CMake
  * Tested with Clang on Mac OS

## Execution

On execution of the program:
  * Generates the complex wave
  * Runs FFT on the complex wave
  * Converts the FFT into a list of peak values (frequency vs voltage) and outputs to the terminal
  
### Program Console Output
  
```
50Hz = 315.018V
100Hz = 49.9975V
200Hz = 25.0172V
250Hz = 9.98469V
1950Hz = 4.97646V
```

 
## Summary

In summary the FFT works well at abstracting the harmonics and their magnitudes from the complex wave; even 
when the sampled wave form is limited by the simulated 12 bit ADC.

### Maximum Detectable Harmonic

The project also shows how Nyquist theorem limits the maximum harmonic detectable to the 39th. This
is due to the sampling rate of the ADC being 4KHZ and the 40th harmonic being 2kHz.

### Windows Size



### Compression of Data

Another area this project highlighted is the use of FFT as data compression. In this situation one can: 
  * Option 1: store 80 samples to represent the wave form
  * Option 2: store just the fundamental & harmonic frequencies and voltages
  
Taking option 2 would mean the stored values reduces to just 10 values, saving 70 in comparison to option 1.
The main negative being that to uncompress you must re-assemble the wave form.

## Acknowledgements

  * KissFFT is used for the FFT processing 