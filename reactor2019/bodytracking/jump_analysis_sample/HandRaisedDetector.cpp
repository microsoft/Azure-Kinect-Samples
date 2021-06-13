// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "HandRaisedDetector.h"

using namespace std::chrono;

void HandRaisedDetector::UpdateData(k4abt_body_t selectedBody, uint64_t currentTimestampUsec)
{
    k4a_float3_t leftWristJoint = selectedBody.skeleton.joints[K4ABT_JOINT_WRIST_LEFT].position;
    k4a_float3_t rightWristJoint = selectedBody.skeleton.joints[K4ABT_JOINT_WRIST_RIGHT].position;
    k4a_float3_t headJoint = selectedBody.skeleton.joints[K4ABT_JOINT_HEAD].position;

    // Notice: y direction is pointing towards the ground! So jointA.y < jointB.y means jointA is higher than jointB
    bool bothHandsAreRaised = leftWristJoint.xyz.y < headJoint.xyz.y &&
                               rightWristJoint.xyz.y < headJoint.xyz.y;

    microseconds currentTimestamp(currentTimestampUsec);
    if (m_previousTimestamp == microseconds::zero())
    {
        m_previousTimestamp = currentTimestamp;
        m_handRaisedTimeSpan = microseconds::zero();
    }

    if (!m_bothHandsAreRaised && bothHandsAreRaised)
    {
        // Start accumulating the hand raising time
        m_handRaisedTimeSpan += currentTimestamp - m_previousTimestamp;
        if (m_handRaisedTimeSpan > m_stableTime)
        {
            m_bothHandsAreRaised = bothHandsAreRaised;
        }
    }
    else if (!bothHandsAreRaised)
    {
        // Stop the time accumulation immediately when hands are put down
        m_bothHandsAreRaised = false;
        m_previousTimestamp = microseconds::zero();
        m_handRaisedTimeSpan = microseconds::zero();
    }
}
