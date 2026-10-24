# Cellular Automata Simulation
## Installation

### Prerequisites

* **C++ Compiler** (supporting C++17 or later, e.g., `gcc`, `clang`, or MSVC)
* **CMake** (version 3.15 or higher)
* **Git**

---

### Step 1: Clone the Repository

Clone the project repository along with any submodules:

```bash
git clone --recursive https://github.com/divine-matrix/emergent.git
cd emergent
```

*(If you already cloned without `--recursive`, run `git submodule update --init --recursive` inside the root folder).*

---

### Step 2: Build the Project

Use modern CMake commands to configure and compile the build:

```bash
# Configure build files into the 'build' directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build using all available CPU cores
cmake --build build -j
```

---

### Step 3: Install

Install the compiled binaries, headers, and libraries to your system:

```bash
cmake --build build --target install
```

> **Note:**
> * If installing system-wide (e.g., to `/usr/local` on Linux/macOS), you may need administrative privileges:
>   ```bash
>   sudo cmake --build build --target install
>   ```
> * To install to a custom path, add `-DCMAKE_INSTALL_PREFIX=/path/to/install` to Step 2:
>   ```bash
>   cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/.local
>   ```

## Information

A program that can simulate many different cellular automata.  
Right now the program simulates:

- Brian's Brain
- Langton's Ant
  
On my laptop (a 2013 MacBook Air Intel) the simulation runs at ~200fps. On my computer (which has a very old graphics card) I haven’t tested it yet, but plan to soon. Because I have built it on such old devices, it should theoretically run well and be compatible with most technology of the decade.  
## Game Engine
The backend of the simulation runs on a custom game engine I built for this project originally, but I plan to use it in other projects. I can’t decide whether to call it Raptor Engine or Rapier, but either I am happy with. It has an abstracted rendering backend that currently runs in SDL, but I might migrate to OpenGL.
