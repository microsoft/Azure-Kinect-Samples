// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <vector>

#include <k4abttypes.h>

struct IndexValueTuple
{
    int Index = -1;
    float Value = 0.f;
};

namespace DSP
{
    std::vector<float> MovingAverage(const std::vector<float>& signal, size_t numOfPoints);

    std::vector<float> FirstDerivate(const std::vector<float>& signal);

    std::vector<float> Divide2Arrays(std::vector<float>& dividend, std::vector<float>& divisor);

    IndexValueTuple FindMaximum(const std::vector<float>& signal, size_t minIdx, size_t maxIdx);

    IndexValueTuple FindMinimum(const std::vector<float>& signal, size_t minIdx, size_t maxIdx);

    float Angle(k4a_float3_t A, k4a_float3_t B, k4a_float3_t C);
};
