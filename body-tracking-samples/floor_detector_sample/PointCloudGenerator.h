// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <k4a/k4atypes.h>

#include <vector>

namespace Samples
{
    class PointCloudGenerator
    {
    public:
        PointCloudGenerator(const k4a_calibration_t& sensorCalibration);
        ~PointCloudGenerator();

        void Update(k4a_image_t depthImage);
        const std::vector<k4a_float3_t>& GetCloudPoints(int downsampleStep = 1);

    private:
        k4a_transformation_t m_transformationHandle = nullptr;
        k4a_image_t m_pointCloudImage_int16x3 = nullptr;
        std::vector<k4a_float3_t> m_cloudPoints;
    };
}
