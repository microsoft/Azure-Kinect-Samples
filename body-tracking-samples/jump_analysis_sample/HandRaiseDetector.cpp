// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "HandRaiseDetector.h"

void HandRaiseDetector::UpdateData(k4abt_body_t selectedBody, uint64_t currentTimestampUsec)
{
    k4a_float3_t leftWristJoint = selectedBody.skeleton.joints[K4ABT_JOINT_WRIST_LEFT].position;
    k4a_float3_t rightWristJoint = selectedBody.skeleton.joints[K4ABT_JOINT_WRIST_RIGHT].position;
    k4a_float3_t headJoint = selectedBody.skeleton.joints[K4ABT_JOINT_HEAD].position;

    // Notice: y direction is pointing towards the ground! So jointA.y < jointB.y means jointA is higher than jointB
    bool bothHandsAreRaising = leftWristJoint.xyz.y < headJoint.xyz.y &&
                               rightWristJoint.xyz.y < headJoint.xyz.y;

    if (m_previousTimestampUsec == 0)
    {
        m_previousTimestampUsec = currentTimestampUsec;
        m_handRaiseTimeSpanUsec = 0;
    }

    if (!m_bothHandsAreRaising && bothHandsAreRaising)
    {
        // Start accumulating the hand raising time
        m_handRaiseTimeSpanUsec += currentTimestampUsec - m_previousTimestampUsec;
        if (m_handRaiseTimeSpanUsec > m_stableTimeInUsec)
        {
            m_bothHandsAreRaising = bothHandsAreRaising;
        }
    }
    else if (!bothHandsAreRaising)
    {
        // Stop the time accumulation immediately when hands are put down
        m_bothHandsAreRaising = false;
        m_previousTimestampUsec = 0;
        m_handRaiseTimeSpanUsec = 0;
    }
}
