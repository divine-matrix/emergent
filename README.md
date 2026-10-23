# Cellular Automata Simulation
## Installation
Requirements:
 - vcpkg
 - CMake
 - a C++ compiler

To install: [Instructions will be added in a future commit]

## Information

A program that can simulate many different cellular automata.  
Right now the program simulates:

- Brian's Brain
- Langton's Ant
  
On my laptop (a 2013 MacBook Air Intel) the simulation runs at ~200fps. On my computer (which has a very old graphics card) I haven’t tested it yet, but plan to soon. Because I have built it on such old devices, it should theoretically run well and be compatible with most technology of the decade.  
## Game Engine
The backend of the simulation runs on a custom game engine I built for this project originally, but I plan to use it in other projects. I can’t decide whether to call it Raptor Engine or Rapier, but either I am happy with. It has an abstracted rendering backend that currently runs in SDL, but I might migrate to OpenGL.
