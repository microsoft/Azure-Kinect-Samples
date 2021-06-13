// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <chrono>
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

    std::vector<float> DivideTwoArrays(std::vector<float>& dividend, std::vector<float>& divisor);

    IndexValueTuple FindMaximum(const std::vector<float>& signal, size_t minIdx, size_t maxIdx);

    IndexValueTuple FindMinimum(const std::vector<float>& signal, size_t minIdx, size_t maxIdx);

    float Angle(k4a_float3_t A, k4a_float3_t B, k4a_float3_t C);

    class RollingWindow
    {
    public:
        RollingWindow(int windowSize)
        {
            m_window.resize(windowSize, 0);
            m_circularIndex = windowSize - 1;
        }
        void Update(const std::chrono::microseconds& timestamp, float v)
        {
            m_circularIndex = (m_circularIndex + 1) % m_window.size();

            m_movingAverageDelta = (v - m_window[m_circularIndex]) / m_window.size();
            m_movingAverage += m_movingAverageDelta;
            m_window[m_circularIndex] = v;

            if (m_circularIndex == m_window.size() - 1)
            {
                m_filled = true;
            }
            m_timestampDelta = timestamp - m_timestamp;
            m_timestamp = timestamp;
        }
        bool IsValid() const { return m_filled; }
        float GetMovingAverage() const { return m_movingAverage; }
        float GetMovingAverageVelocity() const { return m_movingAverageDelta / m_timestampDelta.count(); }

    private:
        std::vector<float> m_window;
        float m_movingAverage = 0;
        float m_movingAverageDelta = 0;
        size_t m_circularIndex = 0;
        bool m_filled = false;
        std::chrono::microseconds m_timestamp;
        std::chrono::microseconds m_timestampDelta;
    };
};
