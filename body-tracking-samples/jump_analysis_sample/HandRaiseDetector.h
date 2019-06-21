// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <k4abttypes.h>
#include <chrono>

class HandRaiseDetector
{
public:
    void UpdateData(k4abt_body_t selectedBody, uint64_t currentTimestampUsec);

    bool AreBothHandsRaising() { return m_bothHandsAreRaising; }

private:
    bool m_bothHandsAreRaising = false;
    uint64_t m_handRaiseTimeSpanUsec = 0;
    uint64_t m_previousTimestampUsec = 0;
    const uint64_t m_stableTimeInUsec = 2000000;
};