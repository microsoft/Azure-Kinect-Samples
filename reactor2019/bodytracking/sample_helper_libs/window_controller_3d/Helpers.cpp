// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "Helpers.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "GLFW/glfw3.h"

//// ASSERT METHODS
void FailedValidation(const char* message)
{
#ifdef DEBUG
    MessageBox(NULL, message, "FailedValidation (DEBUG ONLY)", 0);
#endif
    printf("%s", message);
    exit(-1);
}
void Fail(const char* message, ...)
{
    char fullMessage[256];
    va_list argptr;
    va_start(argptr, message);
#ifdef _WIN32
    vsprintf_s(fullMessage, 256, message, argptr);
#else
    vsprintf(fullMessage, message, argptr);
#endif
    va_end(argptr);

    FailedValidation(fullMessage);
}
void CheckAssert(bool condition, const char* message, ...)
{
    if (!condition)
    {
        char fullMessage[256];
        va_list argptr;
        va_start(argptr, message);
#ifdef _WIN32
        vsprintf_s(fullMessage, 256, message, argptr);
#else
        vsprintf(fullMessage, message, argptr);
#endif
        va_end(argptr);

        FailedValidation(fullMessage);
    }
}
void CheckAssert(bool condition)
{
    CheckAssert(condition, "Missing Assert description");
}


// Shader validation functions
void ValidateShader(GLuint shaderIndex)
{
    GLint success = GL_FALSE;
    char infoLog[512];
    glGetShaderiv(shaderIndex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderIndex, 512, NULL, infoLog);
        Fail("Shader Error: %s", infoLog);
    }
}

void ValidateProgram(GLuint programIndex)
{
    GLint success = GL_FALSE;
    char infoLog[512];
    glGetProgramiv(programIndex, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programIndex, 512, NULL, infoLog);
        Fail("Shader Error: %s", infoLog);
    }
}
