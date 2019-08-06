// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "DigitalSignalProcessing.h"

#include <cmath>
#include <limits>

std::vector<float> DSP::MovingAverage(const std::vector<float>& signal, size_t numOfPoints)
{
    if (numOfPoints > signal.size())
    {
        return std::vector<float>();
    }

    if (signal.size() == 1)
    {
        return signal;
    }
    std::vector<float> cumSum(signal.size());

    std::vector<float> window(numOfPoints);
    std::vector<float> filteredSignal(signal.size());

    int windowIndex = 0;

    for (size_t i = 0; i < signal.size(); i++)
    {
        window[windowIndex] = signal[i] / numOfPoints;
        float currentMean = 0.0f;
        for (size_t j = 0; j < numOfPoints; j++)
        {
            currentMean = currentMean + window[j];
        }
        filteredSignal[i] = currentMean;
        windowIndex = (windowIndex + 1) % numOfPoints;
    }
    return filteredSignal;
}

std::vector<float> DSP::FirstDerivate(const std::vector<float>& signal)
{
    std::vector<float> outputSignal(signal.size() - 1);

    for (size_t i = 1; i < signal.size(); i++)
    {
        outputSignal[i - 1] = signal[i] - signal[i - 1];
    }
    return outputSignal;
}

std::vector<float> DSP::DivideTwoArrays(std::vector<float>& dividend, std::vector<float>& divisor)
{
    if (dividend.size() != divisor.size())
    {
        return std::vector<float>();
    }

    std::vector<float> result(dividend.size());

    for (size_t i = 0; i < dividend.size(); i++)
    {
        if (divisor[i] == 0)
        {
            result[i] = 0;
        }
        else
        {
            result[i] = dividend[i] / divisor[i];
        }
    }
    return result;
}

IndexValueTuple DSP::FindMaximum(const std::vector<float>& signal, size_t minIdx, size_t maxIdx)
{
    if (minIdx > signal.size() || minIdx > signal.size() || minIdx > maxIdx)
    {
        return IndexValueTuple();
    }
    float maxValue = std::numeric_limits<float>::min();
    int maxIndex = 0;

    for (size_t i = minIdx; i < maxIdx; i++)
    {
        if (signal[i] > maxValue)
        {
            maxValue = signal[i];
            maxIndex = static_cast<int>(i);
        }
    }
    return { maxIndex, maxValue };
}

IndexValueTuple DSP::FindMinimum(const std::vector<float>& signal, size_t minIdx, size_t maxIdx)
{
    if (minIdx > signal.size() || minIdx > signal.size() || minIdx > maxIdx)
    {
        return IndexValueTuple();
    }
    float minValue = std::numeric_limits<float>::max();
    int minIndex = 0;

    for (size_t i = minIdx; i < maxIdx; i++)
    {
        if (signal[i] < minValue)
        {
            minValue = signal[i];
            minIndex = static_cast<int>(i);
        }
    }
    return { minIndex, minValue };
}

float DSP::Angle(k4a_float3_t A, k4a_float3_t B, k4a_float3_t C)
{
    k4a_float3_t AbVector;
    k4a_float3_t BcVector;

    AbVector.xyz.x = B.xyz.x - A.xyz.x;
    AbVector.xyz.y = B.xyz.y - A.xyz.y;
    AbVector.xyz.z = B.xyz.z - A.xyz.z;

    BcVector.xyz.x = C.xyz.x - B.xyz.x;
    BcVector.xyz.y = C.xyz.y - B.xyz.y;
    BcVector.xyz.z = C.xyz.z - B.xyz.z;

    float AbNorm = (float)sqrt(AbVector.xyz.x * AbVector.xyz.x + AbVector.xyz.y * AbVector.xyz.y + AbVector.xyz.z * AbVector.xyz.z);
    float BcNorm = (float)sqrt(BcVector.xyz.x * BcVector.xyz.x + BcVector.xyz.y * BcVector.xyz.y + BcVector.xyz.z * BcVector.xyz.z);

    k4a_float3_t AbVectorNorm;
    k4a_float3_t BcVectorNorm;

    AbVectorNorm.xyz.x = AbVector.xyz.x / AbNorm;
    AbVectorNorm.xyz.y = AbVector.xyz.y / AbNorm;
    AbVectorNorm.xyz.z = AbVector.xyz.z / AbNorm;

    BcVectorNorm.xyz.x = BcVector.xyz.x / BcNorm;
    BcVectorNorm.xyz.y = BcVector.xyz.y / BcNorm;
    BcVectorNorm.xyz.z = BcVector.xyz.z / BcNorm;

    float result = AbVectorNorm.xyz.x * BcVectorNorm.xyz.x + AbVectorNorm.xyz.y * BcVectorNorm.xyz.y + AbVectorNorm.xyz.z * BcVectorNorm.xyz.z;

    result = (float)std::acos(result) * 180.0f / 3.1415926535897f;
    return result;
}
