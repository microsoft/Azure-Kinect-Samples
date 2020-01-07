// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "SampleMathTypes.h"

#include <optional>
#include <vector>

namespace Samples
{
    std::optional<Samples::Vector> TryEstimateGravityVectorForDepthCamera(
        const k4a_imu_sample_t& imuSample,
        const k4a_calibration_t& sensorCalibration);

    class FloorDetector
    {
    public:
        static std::optional<Samples::Plane> TryDetectFloorPlane(
            const std::vector<k4a_float3_t>& cloudPoints,
            const k4a_imu_sample_t& imuSample,
            const k4a_calibration_t& sensorCalibration,
            size_t minimumFloorPointCount);
    };
}
