#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "slUtil.h"

//==============================================================================
void applyGain (AudioSampleBuffer& buffer, LinearSmoothedValue<float>& gain);

//==============================================================================

class ToneGen
{
public:
    enum WaveShape : int
    {
        Sine,
        Triangle,
        Saw,
        Square,
    };
    
    struct Parameters
    {
        float frequency {0};
        float phaseOffset {0};
        float offset {0};
        float beat {0};
        WaveShape waveShape {Sine};
        
        Parameters () {}
        
        Parameters (float frequencyIn, float phaseOffsetIn, float offsetIn, WaveShape waveShapeIn) :
            frequency (frequencyIn), phaseOffset (phaseOffsetIn), offset (offsetIn), waveShape (waveShapeIn) {}
        
        bool operator== (const Parameters& other)
        {
            return frequency    == other.frequency      &&
            phaseOffset  == other.phaseOffset    &&
            offset       == other.offset         &&
            beat         == other.beat           &&
            waveShape    == other.waveShape;
        }
        
        bool operator!= (const Parameters& other)
        {
            return ! (*this == other);
        }
    };
    
    void setSampleRate (float newSampleRate)
    {
        if (updateIfDifferent (sampleRate, newSampleRate))
        {
            dirty = true;
            
            const double smoothTime = 0.01;
            
            frequencySmooth.reset (sampleRate, smoothTime);
            phaseOffsetSmooth.reset (sampleRate, smoothTime);
            offsetSmooth.reset (sampleRate, smoothTime);
            fade.reset (sampleRate, smoothTime);
        }
    }
    
    void setParameters (Parameters newParameters)
    {
        if (newParameters != parameters)
        {
            parameters = newParameters;
            
            frequencySmooth.setValue(parameters.frequency);
            phaseOffsetSmooth.setValue(parameters.phaseOffset);
            offsetSmooth.setValue(parameters.offset);
            
            dirty = true;
        }
    }
    
    void processBlockSmoothed (float* buffer, int numSamples)
    {
        if (isSmoothing())
        {
            if (! frequencySmooth.isSmoothing()          &&
                frequencySmooth.getTargetValue() == 0.0f &&
                ! fade.isSmoothing())
            {
                memset (buffer, 0, sizeof(float) * numSamples);
                return;
            }
            
            for (int i = 0; i < numSamples; i++)
            {
                if (updateIfDifferent (dirty, false) || frequencySmooth.isSmoothing())
                {
                    float frequency = frequencySmooth.getNextValue();
                    double newStep = (frequency >= 0.05f) ? (frequency / sampleRate) : 0.0;
                    
                    if (newStep != step)
                    {
                        if (newStep == 0.0)
                            fade.setValue (0.0);
                        else if (step == 0.0)
                            fade.setValue (1.0);
                        
                        step = newStep;
                    }
                }
                
                double currentStep = step;
                
                phase += currentStep;
                if (phase >= 1.0)
                    phase -= 1.0;
                
                float val = 0;
                float localPhase;
                
                float phaseOffset = phaseOffsetSmooth.getNextValue();
                if (phaseOffset != 0)
                    localPhase = wrapValue (std::fmod (float (phase + phaseOffset), 1.0f), 1.0f);
                else
                    localPhase = wrapValue ((float) phase, 1.0f);
                
                jassert (localPhase >= 0.0f && localPhase <= 1.0f);
                
                switch (parameters.waveShape)
                {
                    case Sine:      val = std::sin (localPhase * float_Pi * 2); break;
                    case Triangle:  val = (localPhase < 0.5f) ? (4.0f * localPhase - 1.0f) : (-4.0f * localPhase + 3.0f); break;
                    case Saw:       val = localPhase * 2.0f - 1.0f; break;
                    case Square:    val = (localPhase < 0.5f) ? -1.0f : 1.0f; break;
                }
                
                buffer[i] = (val + offsetSmooth.getNextValue()) * fade.getNextValue();
            }
        }
        else
        {
            processBlock (buffer, numSamples);
        }
    }
    
    void processBlock (float* buffer, int numSamples)
    {
        if (parameters.frequency == 0.0f)
        {
            memset (buffer, 0, sizeof(float) * numSamples);
            return;
        }
        
        step = (parameters.frequency > 0.05f) ? (parameters.frequency / sampleRate) : 0.0;
        fade.setValue (parameters.frequency == 0.0f ? 0.0f : 1.0f);
        fade.reset (sampleRate, 0.05);
        
        double currentStep = step;
        
        for (int i = 0; i < numSamples; i++)
        {
            phase += currentStep;
            if (phase >= 1.0)
                phase -= 1.0;
            
            float val = 0;
            float localPhase;
            
            float phaseOffset = phaseOffsetSmooth.getNextValue();
            if (phaseOffset != 0)
                localPhase = wrapValue (std::fmod (float (phase + phaseOffset), 1.0f), 1.0f);
            else
                localPhase = wrapValue ((float) phase, 1.0f);
            
            jassert (localPhase >= 0.0f && localPhase <= 1.0f);
            
            switch (parameters.waveShape)
            {
                case Sine:      val = std::sin (localPhase * float_Pi * 2); break;
                case Triangle:  val = (localPhase < 0.5f) ? (4.0f * localPhase - 1.0f) : (-4.0f * localPhase + 3.0f); break;
                case Saw:       val = localPhase * 2.0f - 1.0f; break;
                case Square:    val = (localPhase < 0.5f) ? -1.0f : 1.0f; break;
            }
            
            buffer[i] = (val + parameters.offset);
        }
    }
    
    float getNextValue()
    {
        float result;
        processBlockSmoothed (&result, 1);
        return result;
    }
    
private:
    Parameters parameters;
    
    LinearSmoothedValue<float> frequencySmooth;
    LinearSmoothedValue<float> phaseOffsetSmooth;
    LinearSmoothedValue<float> offsetSmooth;
    LinearSmoothedValue<float> fade;
    
    bool dirty {true};
    
    double phase {0};
    double step {0};
    
    float sampleRate {0};
    
    inline bool isBehind (double target, double current)
    {
        if (target < current)
            target += 1.0;
        return (target - current) < 0.5;
    }
    
    inline float wrapValue (float v, float range)
    {
        while (v >= range) v -= range;
        while (v <  0)     v += range;
        return v;
    }
    
    bool isSmoothing()
    {
        return frequencySmooth.isSmoothing()    ||
               phaseOffsetSmooth.isSmoothing()  ||
               offsetSmooth.isSmoothing()       ||
               fade.isSmoothing();
    }
    
};

//==============================================================================
class CircularAudioBuffer
{
public:
    CircularAudioBuffer (int channels = 1, int numSamples = 1000)
      : fifo (numSamples), buffer (channels, numSamples)
    {
        buffer.clear();
        
        int toFill = fifo.getFreeSpace();
        
        int start1, size1, start2, size2;
        fifo.prepareToWrite (toFill, start1, size1, start2, size2);
        fifo.finishedWrite (toFill);
    }
    
    void setSize (int numChannels, int numSamples)
    {
        fifo.setTotalSize (numSamples);
        buffer.setSize (numChannels, numSamples);
        buffer.clear();
        
        int toFill = fifo.getFreeSpace();
        
        int start1, size1, start2, size2;
        fifo.prepareToWrite (toFill, start1, size1, start2, size2);
        fifo.finishedWrite (toFill);
    }
    
    void reset() noexcept
    {
        fifo.reset();
        buffer.clear();
    }
    
    bool write (const AudioSampleBuffer& src)
    {
        return write (src.getArrayOfReadPointers(), src.getNumSamples());
    }
    
    bool write (const float* const* data, int numSamples)
    {
        if (numSamples <= 0)
            return true;
        
        int spaceNeeded = numSamples - fifo.getFreeSpace();
        if (spaceNeeded > 0)
        {
            int start1, end1, start2, end2;
            fifo.prepareToRead (spaceNeeded, start1, end1, start2, end2);
            fifo.finishedRead (spaceNeeded);
        }
        
        int start1, size1, start2, size2;
        fifo.prepareToWrite (numSamples, start1, size1, start2, size2);
        
        if (size1 + size2 < numSamples)
            return false;
        
        for (int i = buffer.getNumChannels(); --i >= 0;)
        {
            buffer.copyFrom (i, start1, data[i], size1);
            buffer.copyFrom (i, start2, data[i] + size1, size2);
        }
        
        fifo.finishedWrite (size1 + size2);
        return true;
    }
    
    bool read (AudioSampleBuffer& dest, int startSampleInDestBuffer = 0)
    {
        return read (dest, startSampleInDestBuffer, dest.getNumSamples());
    }
    
    bool read (AudioSampleBuffer& dest, int startSampleInDestBuffer, int numSamples)
    {
        int start1, size1, start2, size2;
        fifo.prepareToRead (numSamples, start1, size1, start2, size2);
        
        if ((size1 + size2) < numSamples)
            return false;
        
        for (int i = buffer.getNumChannels(); --i >= 0;)
        {
            dest.copyFrom (i, startSampleInDestBuffer, buffer, i, start1, size1);
            dest.copyFrom (i, startSampleInDestBuffer + size1, buffer, i, start2, size2);
        }
        
        return true;
    }
    
private:
    AbstractFifo fifo;
    AudioSampleBuffer buffer;
    
    JUCE_LEAK_DETECTOR (CircularAudioBuffer)
};


//==============================================================================
/** White Gaussian Noise. - box muller method */
class WhiteGaussianNoise
{
public:
    void addNoiseToBuffer (float* buffer, int numSamples)
    {
        float x = 0;
        float y = 0;
        
        for (int i = 0; i < numSamples; i += 2)
        {
            getTwo (x, y);
            
            buffer[i] += x;
            if (i + 1 < numSamples)
                buffer[i + 1] += y;
        }
    }
    
    void copyNoiseToBuffer (float* buffer, int numSamples)
    {
        float x = 0;
        float y = 0;
        
        for (int i = 0; i < numSamples; i += 2)
        {
            getTwo (x, y);
            
            buffer[i] = jlimit (-1.0f, 1.0f, x);
            if (i + 1 < numSamples)
                buffer[i + 1] = jlimit (-1.0f, 1.0f, y);
        }
    }
    
private:
    inline void getTwo (float& x, float& y)
    {
        float s = 0.0f;
        float v1;
        float v2;
        
        do
        {
            const float u1 = rnd.nextFloat();
            const float u2 = rnd.nextFloat();
            v1 = 2 * u1 - 1;
            v2 = 2 * u2 - 1;
            s = v1 * v1 + v2 * v2;
        } while (s >=1);
        
        x = std::sqrt (-2 * std::log (s) / s) * v1;
        y = std::sqrt (-2 * std::log (s) / s) * v2;
    }
    
    Random rnd;
};

//==============================================================================
struct RollingAverage
{
    RollingAverage (int numVals_) : numVals (numVals_), currAvg (0) {}
    
    float average (float nextValue)
    {
        currAvg = (nextValue + numVals * currAvg) / (float)(numVals + 1);
        return currAvg;
    }
    
    float getAverage()
    {
        return currAvg;
    }
    
private:
    int numVals;
    float currAvg;
};
