// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <vector>
#include <k4abt.h>

#include "HandRaisedDetector.h"
#include "Window3dWrapper.h"

enum JumpStatus
{
    Idle = 0,
    CollectJumpData,
    EvaluateAndReview
};

struct IndexValueTuple;
struct JumpResultsData;

class JumpEvaluator
{
public:
    void UpdateStatus(bool changeStatus);
    void UpdateData(k4abt_body_t selectedBody, uint64_t currentTimestampUsec);

private:
    void InitiateJump();

    JumpResultsData CalculateJumpResults();

    void PrintJumpResults(const JumpResultsData& jumpResults);

    void ReviewJumpResults(const JumpResultsData& jumpResults);

    std::vector<float> GetInverseHeightInfoFromBodies(k4abt_joint_id_t jointId);

    int DetermineCalculationWindowWidth(int jumpStartIndex, const std::vector<float>& timeStampInUsec);

    float GetMinKneeAngleFromBody(k4abt_body_t body);

    IndexValueTuple CalcualateJumpStartingPoint(const std::vector<float>& velocity, const std::vector<IndexValueTuple>& velocityPhases);

    IndexValueTuple CalcualateJumpEndingPoint(const std::vector<float>& velocity, const std::vector<IndexValueTuple>& velocityPhases);

    // Calculate characteristic points of velocity signal
    std::vector<IndexValueTuple> CalculatePhasesFromVelocity(const std::vector<float>& velocity);

    float CalculateStartHeight(std::vector<float> signal, size_t startingPoint = 10, size_t endingPoint = 30);

    k4a_float3_t CalculateStandingPosition(int jumpStartIndex, int firstSquatIndex);

    void CreateRenderWindow(
        Window3dWrapper& window,
        std::string windowName,
        const k4abt_body_t& body,
        int windowIndex,
        k4a_float3_t standingPosition);

private:
    // Constant settings for digial signal processing
    const size_t MinimumBodyNumber = 20;  // Minimum number of bodies required in the body list to perform the jump analysis
    const int AverageFilterWindowSize = 6;

    // Internal status
    bool m_reviewWindowIsRunning = false;
    JumpStatus m_jumpStatus = JumpStatus::Idle;

    std::vector<k4abt_body_t> m_listOfBodyPositions;
    std::vector<float> m_framesTimestampInUsec;

    HandRaisedDetector m_handRaisedDetector;
    bool m_previousHandsAreRaised = false;

    // Default jump analysis window size
    const int m_defaultWindowWidth = 640;
    const int m_defaultWindowHeight = 576;

    Window3dWrapper m_window3dSquatPose;
    Window3dWrapper m_window3dJumpPeakPose;
    Window3dWrapper m_window3dReplay;
};
