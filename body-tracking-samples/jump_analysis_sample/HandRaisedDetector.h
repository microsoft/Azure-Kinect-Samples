// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <k4abttypes.h>
#include <chrono>

class HandRaisedDetector
{
public:
    void UpdateData(k4abt_body_t selectedBody, uint64_t currentTimestampUsec);

    bool AreBothHandsRaised() { return m_bothHandsAreRaised; }

private:
    bool m_bothHandsAreRaised = false;
    std::chrono::microseconds m_handRaisedTimeSpan = std::chrono::microseconds::zero();
    std::chrono::microseconds m_previousTimestamp = std::chrono::microseconds::zero();
    const std::chrono::seconds m_stableTime = std::chrono::seconds(2);
};