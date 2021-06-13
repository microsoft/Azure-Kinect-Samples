// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

// Redefine all shader related macros

// Macro for stringifying given argument (two level macro is needed for correct macro expansion)
#undef GLSL_STRINGIFY_I
#undef GLSL_STRING
#define GLSL_STRINGIFY_I(A) #A
#define GLSL_STRING(A) GLSL_STRINGIFY_I(A)

static const char* const glslShaderVersion = "#version 430\n";
