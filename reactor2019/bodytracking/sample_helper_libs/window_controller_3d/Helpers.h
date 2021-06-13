// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "glad/glad.h"

void FailedValidation(const char* message);

void Fail(const char* message, ...);

void CheckAssert(bool condition, const char* message, ...);

void CheckAssert(bool condition);

// Shader validation functions
void ValidateShader(GLuint shaderIndex);

void ValidateProgram(GLuint programIndex);

#define RETURN_IF_GL_ERRORS  { bool glErr = false; while (glGetError() != GL_NO_ERROR) { glErr = true; } if (glErr) { return GPU_ERROR_FROM_API; } }

#define UNINIT_IF_GL_ERRORS  { bool glErr = false; while (glGetError() != GL_NO_ERROR) { glErr = true; } if (glErr) { UnInitialize(); return GPU_ERROR_FROM_API; } }