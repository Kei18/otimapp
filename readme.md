otimapp
===
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)

A simulator and visualizer used in a paper ["Offline Time-Independent Multi-Agent Path Planning"](https://kei18.github.io/otimapp/) (OTIMAPP, to appear at IJCAI-22 / T-RO-23).
It is written in C++(17) with [CMake](https://cmake.org/) (≥v3.16) build.
The repository uses [Google Test](https://github.com/google/googletest) and [the original library for 2D pathfinding](https://github.com/Kei18/grid-pathfinding) as git submodules.
The visualizer uses [openFrameworks](https://openframeworks.cc) and works only on MacOS.


| platform | status (master) |
| ---: | :--- |
| macos-10.15 | ![test_macos](https://github.com/Kei18/otimapp/workflows/test_macos/badge.svg?branch=master) ![build_visualizer_macos](https://github.com/Kei18/otimapp/workflows/build_visualizer_macos/badge.svg?branch=master) |
| ubuntu-latest | ![test_ubuntu](https://github.com/Kei18/otimapp/workflows/test_ubuntu/badge.svg?branch=master) |

## Demo
![100 agents in arena](./assets/demo.gif)

planned by PP, execution on MAPF-DP

## Building

```sh
git clone --recursive https://github.com/Kei18/otimapp.git
cd otimapp
cmake -B build && make -C build
```

## Usage
### Planning
planed by PP
```sh
./build/app -i ./sample-instance.txt -s PP -o ./plan.txt -v
```

### Execution
MAPF-DP, upper bound of delay probabilities is 0.5
```sh
./build/exec -i ./sample-instance.txt -p ./plan.txt -o ./exec.txt -v -u 0.5
```

### Help
You can find details and explanations for all parameters with:
```sh
./build/app --help
```
or
```sh
./build/exec --help
```

Please see `./sample-instance.txt` for parameters of instances, e.g., filed, number of agents, time limit, etc.

### Output File

This is an example output of `./sample-instance.txt`.
Note that `(x, y)` denotes location.
`(0, 0)` is the left-top point.
`(x, 0)` is the location at `x`-th column and 1st row.
A position `(x, y)` are also represented as a single number `i = widht*y + x`.

<details><summary>plan.txt</summary>

```txt
instance=../instances/sample.txt
agents=100
map_file=arena.map
seed=1
solver=PrioritizedPlanning
solved=1
unsolvable=0
comp_time=97
starts=(32,21),(40,4),(20,22),[...]
goals=(10,16),(30,21),(11,42),[...]
sum-of-path-length:3401
plan=
0:1061,1012,963,[...]
1:236,285,334,[...]
[...]
```

</details>

<details><summary>exec.txt</summary>

```txt
// log from ./plan.txt
---
(copy of plan.txt)
---
// exec result
---
problem_name=MAPF_DP
plan=./plan.txt
ub_delay_prob=0.5
delay_probs=0.274407,0.296422,[...]
exec_succeed=1
exec_seed=0
emulation_time=3
activate_cnts=7158
makespan=106
soc=5022
result=
0:(32,21),(40,4),(20,22),[...]
1:(32,20),(40,5),(20,22),[..]
[...]
```

</details>

## Visualizer

### Building
It takes around 10 minutes.

#### macOS 10.x
```sh
bash ./visualizer/scripts/build_macos_10.sh
```

Note: The script of openFrameworks seems to contain bugs. Check this [issue](https://github.com/openframeworks/openFrameworks/issues/6623). I fixed this in my script :D

#### macOS 11.x
```sh
bash ./visualizer/scripts/build_macos_10.sh
```

### Usage
```sh
cd build
../visualize.sh ./exec.txt
```

You can manipulate it via your keyboard. See printed info.


## Experimental Environment
- IJCAI-22: [![v1.1](https://img.shields.io/badge/tag-v1.1-blue.svg?style=flat)](https://github.com/Kei18/otimapp/releases/tag/v1.1)
- T-RO-23: [![v1.2](https://img.shields.io/badge/tag-v1.2-blue.svg?style=flat)](https://github.com/Kei18/otimapp/releases/tag/v1.2)

Scripts for the experiments are in `exp_scripts/`.
All instances are included in `./instances.zip`.

## Notes
- Maps in `maps/` are from [Pathfinding Benchmarks](https://movingai.com/benchmarks/).
  When you add a new map, please place it in the `maps/` directory.
- The font in `visualizer/bin/data` is from [Google Fonts](https://fonts.google.com/).
- The "preprint" branch is for the initial manuscript at arXiv.

## Licence
This software is released under the MIT License, see [LICENSE.txt](LICENCE.txt).

## Author
[Keisuke Okumura](https://kei18.github.io) is a Ph.D. student at Tokyo Institute of Technology, interested in controlling multiple moving agents.
