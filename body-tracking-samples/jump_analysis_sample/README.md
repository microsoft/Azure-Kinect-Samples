# Azure Kinect Body Tracking JumpAnalysis Sample

## Introduction

The Azure Kinect Body Tracking JumpAnalysis sample leverages the body tracking SDK to perform quantitative analysis to
a jump section performed by a single user. After each jump section, it will output the jump height, counter movement,
push-off velocity and the squat knee angle. It demonstrates how users can write some simple code to build analysis in 3d.

## Usage Info

```
jump_analysis_sample.exe
```

## Instruction

1. Make sure you place the camera parallel to the floor and there is only one person in the scene.
2. Raise both of your hands above your head or hit 'space' key to start the jump session.
3. Perform a jump. Try to land at the same location as the starting point.
4. Raise both of your hands above your head or hit 'space' key again to finish the session.
5. Three 3d windows will pop up to show the moment of your deepest squat, jump peak and a replay of your full jump session.
   Your jump analysis results will also be printed out on the command prompt.
6. Close any of the 3d windows to go back to the idle stage.
