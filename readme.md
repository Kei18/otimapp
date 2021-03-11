otimapp
===
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)

A simulator and visualizer used in a paper "Offline Time-Independent Multi-Agent Path Planning".
It is written in C++(17) with [CMake](https://cmake.org/) (≥v3.16) build.
The repository uses [Google Test](https://github.com/google/googletest) and [the original library for 2D pathfinding](https://github.com/Kei18/grid-pathfinding) as git submodules.
The visualizer uses [openFrameworks](https://openframeworks.cc) and works only on macOS.


| platform | status (public) | status (dev) |
| ---: | :--- |:--- |
| macos-10.15 | ![test_macos](https://github.com/Kei18/otimapp/workflows/test_macos/badge.svg?branch=public) ![build_visualizer_macos](https://github.com/Kei18/otimapp/workflows/build_visualizer_macos/badge.svg?branch=public) | ![test_macos](https://github.com/Kei18/otimapp/workflows/test_macos/badge.svg?branch=dev) ![build_visualizer_macos](https://github.com/Kei18/otimapp/workflows/build_visualizer_macos/badge.svg?branch=dev) |
| ubuntu-latest | ![test_ubuntu](https://github.com/Kei18/otimapp/workflows/test_ubuntu/badge.svg?branch=public) | ![test_ubuntu](https://github.com/Kei18/otimapp/workflows/test_ubuntu/badge.svg?branch=dev) |

## Demo

## Building

```sh
git clone --recursive https://github.com/Kei18/otimapp.git
cd otimapp
mkdir build
cd build
cmake ..
make
```

## Usage


You can find details and explanations for all parameters with:
```sh
./app --help
```

Please see `instances/sample.txt` for parameters of instances, e.g., filed, number of agents, time limit, etc.

### Output File

This is an example output of `../instances/sample.txt`.
Note that `(x, y)` denotes location.
`(0, 0)` is the left-top point.
`(x, 0)` is the location at `x`-th column and 1st row.
```
instance= ../instances/sample.txt
agents=100
map_file=arena.map
solver=PIBT
solved=1
soc=3403
makespan=68
comp_time=58
starts=(32,21),(40,4),(20,22),(26,18), [...]
goals=(10,16),(30,21),(11,42),(44,6), [...]
solution=
0:(32,21),(40,4),(20,22),(26,18), [...]
1:(31,21),(40,5),(20,23),(27,18), [...]
[...]
```

## Visualizer

### Building
It takes around 10 minutes.

#### macOS
```sh
bash ./visualizer/scripts/build_macos.sh
```

Note: The script of openFrameworks seems to contain bugs. Check this [issue](https://github.com/openframeworks/openFrameworks/issues/6623). I fixed this in my script :D


### Usage
```sh
cd build
../visualize.sh result.txt
```

You can manipulate it via your keyboard. See printed info.

## Experimental Environment

Scripts for the experiments are in `exp_scripts/`.

## Notes
- Maps in `maps/` are from [Pathfinding Benchmarks](https://movingai.com/benchmarks/).
  When you add a new map, please place it in the `maps/` directory.
- The font in `visualizer/bin/data` is from [Google Fonts](https://fonts.google.com/).

## Licence
This software is released under the MIT License, see [LICENSE.txt](LICENCE.txt).

## Author
[Keisuke Okumura](https://kei18.github.io) is a Ph.D. student at the Tokyo Institute of Technology, interested in controlling multiple moving agents.

## Reference
