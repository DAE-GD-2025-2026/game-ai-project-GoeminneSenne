# Game AI Project
This repo contains my project for the course "Game AI Programming", part of the Game Development DAE curriculum. 

## Extra Assignment
The extra assignment I choose was from W5. Graph Theory: Minimum Spanning Tree.
In the level "MinimumSpanningTree" you can add nodes/connections to the existing graph with the left mouse button. To calculate the MST, click on the IMGUI Button on screen.
The MST is calculated using Prim's algorithm and the implementation can be found [here](Source/GameAIProg/GraphTheory/Algorithms/PrimMST.h)


## Steering Behaviors
The project contains steering behaviors for AI Agents such as :
- Seek
- Pursuit
- Evade
- Wander
- ...

This project also includes a flocking system to simulate large groups of enemies. The implementation is optimized by using a memory pool and space partitioning.

## Pathfinding & Navigation
The project also includes following features related to Navigation

- Eulerian Graph traversal
- BFS and A* Pathfinding
- Navmesh traversal for large areas
- Path Smoothing using a SSFA as optimization


## Engine
This project was made in Unreal Engine
