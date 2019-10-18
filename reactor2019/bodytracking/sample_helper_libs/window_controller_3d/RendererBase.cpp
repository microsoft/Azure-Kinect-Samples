// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "RendererBase.h"

using namespace linmath;
using namespace Visualization;

void RendererBase::UpdateViewProjection(mat4x4 view, mat4x4 projection)
{
    mat4x4_dup(m_view, view);
    mat4x4_dup(m_projection, projection);
}
