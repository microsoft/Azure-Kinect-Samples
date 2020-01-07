// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "FloorDetector.h"

#include <algorithm>    // std::sort
#include <cassert>      // assert
#include <numeric>      // std::iota

std::optional<Samples::Vector> Samples::TryEstimateGravityVectorForDepthCamera(
    const k4a_imu_sample_t& imuSample,
    const k4a_calibration_t& sensorCalibration)
{
    // An accelerometer at rest on the surface of the Earth will measure an acceleration
    // due to Earth's gravity straight **upwards** (by definition) of g ~ 9.81 m/s2.
    // https://en.wikipedia.org/wiki/Accelerometer

    Samples::Vector imuAcc = imuSample.acc_sample; // in meters per second squared.

    // Estimate gravity when device is not moving.
    if (std::abs(imuAcc.Length() - 9.81f) < 0.2f)
    {
        // Extrinsic Rotation from ACCEL to DEPTH.
        const auto& R = sensorCalibration.extrinsics[K4A_CALIBRATION_TYPE_ACCEL][K4A_CALIBRATION_TYPE_DEPTH].rotation;

        Samples::Vector Rx = { R[0], R[1], R[2] };
        Samples::Vector Ry = { R[3], R[4], R[5] };
        Samples::Vector Rz = { R[6], R[7], R[8] };
        Samples::Vector depthAcc = { Rx.Dot(imuAcc), Ry.Dot(imuAcc) , Rz.Dot(imuAcc) };

        // The acceleration due to gravity, g, is in a direction toward the ground.
        // However an accelerometer at rest in a gravity field reports upward acceleration
        // relative to the local inertial frame (the frame of a freely falling object).
        Samples::Vector depthGravity = depthAcc * -1;
        return depthGravity;
    }
    return {};
}

struct HistogramBin
{
    size_t count;
    float leftEdge;
};

std::vector<HistogramBin> Histogram(const std::vector<float>& values, float binSize)
{
    // Bounds
    const auto [minEl, maxEl] = std::minmax_element(values.begin(), values.end());
    const float minVal = *minEl;
    const float maxVal = *maxEl;

    size_t binCount = 1 + static_cast<size_t>((maxVal - minVal) / binSize);

    // Bins
    std::vector<HistogramBin> histBins{ binCount };
    for (size_t i = 0; i < binCount; ++i)
    {
        histBins[i] = { /*binCount*/ 0, /*binLeftEdge*/ i * binSize + minVal };
    }

    // Counts
    for (float v : values)
    {
        int binIndex = static_cast<int>((v - minVal) / binSize);
        histBins[binIndex].count++;
    }
    return histBins;
}

std::optional<Samples::Plane> FitPlaneToInlierPoints(const std::vector<k4a_float3_t>& cloudPoints, const std::vector<size_t>& inlierIndices)
{
    // https://www.ilikebigbits.com/2015_03_04_plane_from_points.html

    if (inlierIndices.size() < 3)
    {
        return {};
    }

    // Compute centroid.
    Samples::Vector centroid(0, 0, 0);
    for (const auto& index : inlierIndices)
    {
        centroid = centroid + cloudPoints[index];
    }
    centroid = centroid / static_cast<float>(inlierIndices.size());

    // Compute the zero-mean 3x3 symmetric covariance matrix relative.
    float xx = 0, xy = 0, xz = 0, yy = 0, yz = 0, zz = 0;
    for (const auto& index : inlierIndices)
    {
        Samples::Vector r = cloudPoints[index] - centroid;
        xx += r.X * r.X;
        xy += r.X * r.Y;
        xz += r.X * r.Z;
        yy += r.Y * r.Y;
        yz += r.Y * r.Z;
        zz += r.Z * r.Z;
    }

    float detX = yy * zz - yz * yz;
    float detY = xx * zz - xz * xz;
    float detZ = xx * yy - xy * xy;

    float detMax = std::max({ detX, detY, detZ });
    if (detMax <= 0)
    {
        return {};
    }

    Samples::Vector normal(0, 0, 0);
    if (detMax == detX)
    {
        normal = { detX, xz * yz - xy * zz, xy * yz - xz * yy };
    }
    else if (detMax == detY)
    {
        normal = { xz * yz - xy * zz, detY, xy * xz - yz * xx };
    }
    else
    {
        normal = { xy * yz - xz * yy, xy * xz - yz * xx, detZ };
    }

    return Samples::Plane::Create(normal.Normalized(), centroid);
}

std::optional<Samples::Plane> Samples::FloorDetector::TryDetectFloorPlane(
    const std::vector<k4a_float3_t>& cloudPoints,
    const k4a_imu_sample_t& imuSample,
    const k4a_calibration_t& sensorCalibration,
    size_t minimumFloorPointCount)
{
    auto gravity = TryEstimateGravityVectorForDepthCamera(imuSample, sensorCalibration);
    if (gravity.has_value() && !cloudPoints.empty())
    {
        // Up normal is opposite to gravity down vector.
        Samples::Vector up = (gravity.value() * -1).Normalized();

        // Compute elevation of cloud points (projections on the floor normal).
        std::vector<float> offsets(cloudPoints.size());
        for (size_t i = 0; i < cloudPoints.size(); ++i)
        {
            offsets[i] = up.Dot(Samples::Vector(cloudPoints[i]));
        }

        // There could be several horizontal planes in the scene (floor, tables, ceiling).
        // For the floor, look for lowest N points whose elevations are within a small range from each other.

        const float planeDisplacementRangeInMeters = 0.050f; // 5 cm in meters.
        const float planeMaxTiltInDeg = 5.0f;

        const int binAggregation = 6;
        assert(binAggregation >= 1);
        const float binSize = planeDisplacementRangeInMeters / binAggregation;
        const auto histBins = Histogram(offsets, binSize);
        
        // Cumulative histogram counts.
        std::vector<size_t> cumulativeHistCounts(histBins.size());
        cumulativeHistCounts[0] = histBins[0].count;
        for (size_t i = 1; i < histBins.size(); ++i)
        {
            cumulativeHistCounts[i] = cumulativeHistCounts[i - 1] + histBins[i].count;
        }

        assert(cumulativeHistCounts.back() == offsets.size());

        for (size_t i = 1; i + binAggregation < cumulativeHistCounts.size(); ++i)
        {
            size_t aggBinStart = i;                 // inclusive bin
            size_t aggBinEnd = i + binAggregation;  // exclusive bin
            size_t inlierCount = cumulativeHistCounts[aggBinEnd - 1] - cumulativeHistCounts[aggBinStart - 1];
            if (inlierCount > minimumFloorPointCount)
            {
                float offsetStart = histBins[aggBinStart].leftEdge; // inclusive
                float offsetEnd = histBins[aggBinEnd].leftEdge;     // exclusive

                // Inlier indices.
                std::vector<size_t> inlierIndices;
                for (size_t j = 0; j < offsets.size(); ++j)
                {
                    if (offsetStart <= offsets[j] && offsets[j] < offsetEnd)
                    {
                        inlierIndices.push_back(j);
                    }
                }

                // Fit plane to inlier points.
                auto refinedPlane = FitPlaneToInlierPoints(cloudPoints, inlierIndices);

                if (refinedPlane.has_value())
                {
                    // Ensure normal is upward.
                    if (refinedPlane->Normal.Dot(up) < 0)
                    {
                        refinedPlane->Normal = refinedPlane->Normal * -1;
                    }

                    // Ensure normal is mostly vertical.
                    auto floorTiltInDeg = acos(refinedPlane->Normal.Dot(up)) * 180.0f / 3.14159265f;
                    if (floorTiltInDeg < planeMaxTiltInDeg)
                    {
                        // For reduced jitter, use gravity for floor normal.
                        refinedPlane->Normal = up;
                        return refinedPlane;
                    }
                }

                return {};
            }
        }
    }

    return {};
}
