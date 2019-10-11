// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "JumpEvaluator.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "DigitalSignalProcessing.h"

using namespace Visualization;
using namespace std::chrono;

struct JumpResultsData
{
    // Jump analysis results
    float Height = 0;
    float PreparationSquatDepth = 0;
    float LandingSquatDepth = 0;
    float PushOffVelocity = 0;
    float KneeAngle = 0;

    // Fields that help to visualize the results
    k4a_float3_t StandingPosition;
    int PeakIndex = 0;
    int SquatPointIndex = 0;
    bool JumpSuccess = false;
};

/******************************************************************************************************/
/******************************************* Demo functions *******************************************/
/******************************************************************************************************/

void JumpEvaluator::UpdateData(k4abt_body_t selectedBody, uint64_t currentTimestampUsec)
{
#pragma region Hand Raise Detector
    // Update hand raise detector data
    m_handRaisedDetector.UpdateData(selectedBody, currentTimestampUsec);

    // Use hand raise detector to decide whether we should initialize/end a jump session
    bool handsAreRaised = m_handRaisedDetector.AreBothHandsRaised();
    if (!m_previousHandsAreRaised && handsAreRaised)
    {
        UpdateStatus(true);
    }
    m_previousHandsAreRaised = handsAreRaised;
#pragma endregion

    // Collect jump data
    if (m_jumpStatus == JumpStatus::CollectJumpData)
    {
        m_listOfBodyPositions.push_back(selectedBody);
        m_framesTimestampInUsec.push_back(static_cast<float>(currentTimestampUsec));
    }

    // Calculate jump results
    if (m_jumpStatus == JumpStatus::EvaluateAndReview)
    {
        JumpResultsData jumpResults = CalculateJumpResults();
        PrintJumpResults(jumpResults);

        if (jumpResults.JumpSuccess)
        {
            ReviewJumpResults(jumpResults);
        }
        m_jumpStatus = JumpStatus::Idle;
    }
}

/******************************************************************************************************/
/****************************************** Helper functions ******************************************/
/******************************************************************************************************/

void JumpEvaluator::UpdateStatus(bool changeStatus)
{
    if (changeStatus)
    {
        // Hand raise status is changed!
        if (m_jumpStatus == JumpStatus::Idle)
        {
            InitiateJump();
            std::cout << "Jump Session Started!" << std::endl;
            m_jumpStatus = JumpStatus::CollectJumpData;
        }
        else if (m_jumpStatus == JumpStatus::CollectJumpData)
        {
            std::cout << "Jump Session End!" << std::endl;
            m_jumpStatus = JumpStatus::EvaluateAndReview;
        }
    }
}

void JumpEvaluator::InitiateJump()
{
    m_listOfBodyPositions.clear();
    m_framesTimestampInUsec.clear();
}

JumpResultsData JumpEvaluator::CalculateJumpResults()
{
    JumpResultsData jumpResults;
    jumpResults.JumpSuccess = false;

    // Make sure we have enough data point
    if (m_listOfBodyPositions.size() <= MinimumBodyNumber)
    {
        return jumpResults;
    }

    try
    {
        // Y direction of the sensor coordinate is pointing down. We need to inverse the Y direction to make sure it
        // points towards the jump direction
        std::vector<float> posY = GetInverseHeightInfoFromBodies(K4ABT_JOINT_PELVIS);
        std::vector<float>& timestamp = m_framesTimestampInUsec;

        std::vector<float> heightFiltered = DSP::MovingAverage(posY, AverageFilterWindowSize);

        // Calculate key phases based on height
        IndexValueTuple maxHeight = DSP::FindMaximum(heightFiltered, 0, heightFiltered.size());
        IndexValueTuple preparationSquatPoint = DSP::FindMinimum(heightFiltered, 0, maxHeight.Index);
        IndexValueTuple landingSquatPoint = DSP::FindMinimum(heightFiltered, maxHeight.Index, heightFiltered.size());

        std::vector<float> heightDerivative = DSP::FirstDerivate(heightFiltered);

        // Calculate key phases based on height derivative (vertical velocity)
        std::vector<IndexValueTuple> velocityPhases = CalculatePhasesFromVelocity(heightDerivative);
        IndexValueTuple jumpStartingPoint = CalcualateJumpStartingPoint(heightDerivative, velocityPhases);

        // First derivate of timestamp array
        std::vector<float> timeFirstDerivate = DSP::FirstDerivate(timestamp);

        // Calculate unit velocity by dV/dt
        std::vector<float> velocityY = DSP::DivideTwoArrays(heightDerivative, timeFirstDerivate);

        // Maximum velocity
        IndexValueTuple maxVelocityInMmPerUsec = DSP::FindMaximum(velocityY, 0, velocityY.size());

        // Knee angles
        float kneeAngleRes = GetMinKneeAngleFromBody(m_listOfBodyPositions[preparationSquatPoint.Index]);

        int jumpStartIndex = jumpStartingPoint.Index;

        int calculationWindowWidth = DetermineCalculationWindowWidth(jumpStartIndex, timestamp);
        float startHeight = 0;
        if (calculationWindowWidth > 0)
        {
            startHeight = CalculateStartHeight(posY, jumpStartIndex - calculationWindowWidth, jumpStartIndex);
        }

        k4a_float3_t standingPosition = CalculateStandingPosition(jumpStartIndex, preparationSquatPoint.Index);

        const float UsecToSecond = 1e-6f;
        jumpResults.JumpSuccess = true;
        jumpResults.Height = maxHeight.Value - startHeight;
        jumpResults.PreparationSquatDepth = preparationSquatPoint.Value - startHeight;
        jumpResults.LandingSquatDepth = landingSquatPoint.Value - startHeight;
        jumpResults.PushOffVelocity = maxVelocityInMmPerUsec.Value / UsecToSecond;
        jumpResults.KneeAngle = kneeAngleRes;
        jumpResults.StandingPosition = standingPosition;
        jumpResults.PeakIndex = maxHeight.Index;
        jumpResults.SquatPointIndex = preparationSquatPoint.Index;
    }
    catch (const std::runtime_error&)
    {
        jumpResults.JumpSuccess = false;
    }

    return jumpResults;
}

void JumpEvaluator::PrintJumpResults(const JumpResultsData& jumpResults)
{
    if (jumpResults.JumpSuccess)
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Jump Analysis: " << std::endl;
        std::cout << "   Height (cm): " << jumpResults.Height / 10.f << std::endl;
        std::cout << "   Countermovement (cm): " << -jumpResults.PreparationSquatDepth / 10.f << std::endl;
        std::cout << "   Push-off Velocity (m/second): " << jumpResults.PushOffVelocity / 1000.f << std::endl;
        std::cout << "   Knee Angle (degree): " << jumpResults.KneeAngle << std::endl;
    }
    else
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Jump Analysis Failed! Please try again!" << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
    }

}

void JumpEvaluator::ReviewJumpResults(const JumpResultsData& jumpResults)
{
    CreateRenderWindow(m_window3dSquatPose, "Squat Pose", m_listOfBodyPositions[jumpResults.SquatPointIndex], 0, jumpResults.StandingPosition);
    CreateRenderWindow(m_window3dJumpPeakPose, "Jump Peak Pose", m_listOfBodyPositions[jumpResults.PeakIndex], 1, jumpResults.StandingPosition);
    CreateRenderWindow(m_window3dReplay, "Replay", m_listOfBodyPositions[0], 2, jumpResults.StandingPosition);

    milliseconds duration = milliseconds::zero();
    milliseconds expectedFrameDuration = milliseconds(33);
    size_t currentReplayIndex = 0;
    m_reviewWindowIsRunning = true;
    while (m_reviewWindowIsRunning)
    {
        auto start = high_resolution_clock::now();
        if (duration > expectedFrameDuration)
        {
            currentReplayIndex = (currentReplayIndex + 1) % m_listOfBodyPositions.size();
            auto currentBody = m_listOfBodyPositions[currentReplayIndex];

            // Try to skip one frame if we detected a flip
            if (currentBody.skeleton.joints[K4ABT_JOINT_ANKLE_LEFT].position.xyz.x <=
                currentBody.skeleton.joints[K4ABT_JOINT_ANKLE_RIGHT].position.xyz.x)
            {
                currentReplayIndex = (currentReplayIndex + 1) % m_listOfBodyPositions.size();
            }

            m_window3dReplay.CleanJointsAndBones();
            m_window3dReplay.AddBody(m_listOfBodyPositions[currentReplayIndex], g_bodyColors[0]);
            duration = milliseconds::zero();
        }

        m_window3dSquatPose.Render();
        m_window3dJumpPeakPose.Render();
        m_window3dReplay.Render();

        duration += duration_cast<milliseconds>(high_resolution_clock::now() - start);
    }

    m_window3dSquatPose.Delete();
    m_window3dJumpPeakPose.Delete();
    m_window3dReplay.Delete();
}

std::vector<float> JumpEvaluator::GetInverseHeightInfoFromBodies(k4abt_joint_id_t jointId)
{
    std::vector<float> inversePosY(m_listOfBodyPositions.size());
    for (size_t i = 0; i < m_listOfBodyPositions.size(); i++)
    {
        inversePosY[i] = -m_listOfBodyPositions[i].skeleton.joints[(int)jointId].position.xyz.y;
    }
    return inversePosY;
}

int JumpEvaluator::DetermineCalculationWindowWidth(int jumpStartIndex, const std::vector<float>& timeStampInUsec)
{
    float stableTimeInUsec = 200000;
    float deltaTime = 0.0f;
    int i = 0;
    for (i = jumpStartIndex - 1; ((i >= 0) && (deltaTime < stableTimeInUsec)); --i)
    {
        deltaTime = timeStampInUsec[jumpStartIndex] - timeStampInUsec[i];
    }
    if (i >= 0)
    {
        return jumpStartIndex - i;
    }
    else
    {
        throw std::runtime_error("Data error");
    }
}

float JumpEvaluator::GetMinKneeAngleFromBody(k4abt_body_t body)
{
    k4a_float3_t footLeft = body.skeleton.joints[K4ABT_JOINT_ANKLE_LEFT].position;
    k4a_float3_t kneeLeft = body.skeleton.joints[K4ABT_JOINT_KNEE_LEFT].position;
    k4a_float3_t torzoLeft = body.skeleton.joints[K4ABT_JOINT_HIP_LEFT].position;

    k4a_float3_t footRight = body.skeleton.joints[K4ABT_JOINT_ANKLE_RIGHT].position;
    k4a_float3_t kneeRight = body.skeleton.joints[K4ABT_JOINT_KNEE_RIGHT].position;
    k4a_float3_t torzoRight = body.skeleton.joints[K4ABT_JOINT_HIP_RIGHT].position;

    float leftKneeAngle = 180 - DSP::Angle(torzoLeft, kneeLeft, footLeft);
    float rightKneeAngle = 180 - DSP::Angle(torzoRight, kneeRight, footRight);
    return std::min(leftKneeAngle, rightKneeAngle);
}

IndexValueTuple JumpEvaluator::CalcualateJumpStartingPoint(
    const std::vector<float>& velocity,
    const std::vector<IndexValueTuple>& velocityPhases)
{
    const float MinimumValuePrecent = 0.03f;

    int i = velocityPhases[0].Index - 1;
    if (i < 0)
    {
        i = 0;
    }

    while (velocity[i] < MinimumValuePrecent * velocityPhases[0].Value)
    {
        i--;
        if (i <= 0)
        {
            i = 0;
            throw std::runtime_error("Data error");
        }
    }
    return { i, velocity[i] };
}

IndexValueTuple JumpEvaluator::CalcualateJumpEndingPoint(
    const std::vector<float>& velocity,
    const std::vector<IndexValueTuple>& velocityPhases)
{
    const float MaximumValuePrecent = 0.02f;

    int i = velocityPhases[3].Index - 1;
    if (i < 0)
    {
        i = 0;
    }

    while (velocity[i] > MaximumValuePrecent * velocityPhases[3].Value)
    {
        i++;
        if (i == static_cast<int>(velocity.size()) - 1)
        {
            throw std::runtime_error("Data error");
        }
    }
    return { i, velocity[i] };
}

std::vector<IndexValueTuple> JumpEvaluator::CalculatePhasesFromVelocity(const std::vector<float>& velocity)
{
    IndexValueTuple firstMax = DSP::FindMaximum(velocity, 0, velocity.size());

    IndexValueTuple firstMin = DSP::FindMinimum(velocity, 0, firstMax.Index);

    IndexValueTuple secondMin = DSP::FindMinimum(velocity, firstMax.Index, velocity.size());

    IndexValueTuple secondMax = DSP::FindMaximum(velocity, secondMin.Index, velocity.size());

    std::vector<IndexValueTuple> result = { firstMin, firstMax, secondMin, secondMax };

    return result;
}

float JumpEvaluator::CalculateStartHeight(std::vector<float> signal, size_t startingPoint, size_t endingPoint)
{
    if (startingPoint > signal.size() || startingPoint > endingPoint || endingPoint <= startingPoint)
    {
        throw std::runtime_error("Data error");
    }
    if (endingPoint >= signal.size())
    {
        endingPoint = signal.size();
    }

    float sum = 0;
    for (size_t i = startingPoint; i < endingPoint; i++)
    {
        sum += signal[i];
    }

    return sum / (endingPoint - startingPoint);
}

k4a_float3_t JumpEvaluator::CalculateStandingPosition(int jumpStartIndex, int firstSquatIndex)
{
    float xPos = m_listOfBodyPositions[jumpStartIndex].skeleton.joints[K4ABT_JOINT_PELVIS].position.xyz.x;
    float zPos = m_listOfBodyPositions[jumpStartIndex].skeleton.joints[K4ABT_JOINT_PELVIS].position.xyz.z;

    float yPos = 0.f;
    yPos += m_listOfBodyPositions[jumpStartIndex].skeleton.joints[K4ABT_JOINT_ANKLE_LEFT].position.xyz.y;
    yPos += m_listOfBodyPositions[jumpStartIndex].skeleton.joints[K4ABT_JOINT_ANKLE_RIGHT].position.xyz.y;
    yPos += m_listOfBodyPositions[firstSquatIndex].skeleton.joints[K4ABT_JOINT_ANKLE_LEFT].position.xyz.y;
    yPos += m_listOfBodyPositions[firstSquatIndex].skeleton.joints[K4ABT_JOINT_ANKLE_RIGHT].position.xyz.y;
    yPos /= 4.f;
    return { xPos, yPos, zPos };
}

int64_t ReviewWindowCloseCallback(void* context)
{
    bool* running = (bool*)context;
    *running = false;
    return 1;
}

void JumpEvaluator::CreateRenderWindow(
    Window3dWrapper& window,
    std::string windowName,
    const k4abt_body_t& body,
    int windowIndex,
    k4a_float3_t standingPosition)
{
    window.Create(windowName.c_str(), K4A_DEPTH_MODE_WFOV_2X2BINNED, m_defaultWindowWidth, m_defaultWindowHeight);
    window.SetCloseCallback(ReviewWindowCloseCallback, &m_reviewWindowIsRunning);
    window.AddBody(body, g_bodyColors[0]);
    window.SetFloorRendering(true, standingPosition.v[0] / 1000.f, standingPosition.v[1] / 1000.f, standingPosition.v[2] / 1000.f);

    int xPos = windowIndex * m_defaultWindowWidth;
    int yPos = 100;
    window.SetWindowPosition(xPos, yPos);
}
