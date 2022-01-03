otimapp
===
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)

A simulator and visualizer used in a paper "Offline Time-Independent Multi-Agent Path Planning" (OTIMAPP).
It is written in C++(17) with [CMake](https://cmake.org/) (≥v3.16) build.
The visualizer uses [openFrameworks](https://openframeworks.cc) and works only on MacOS.

## Demo
![100 agents in arena](./assets/demo.gif)

planned by PP, execution on MAPF-DP

## Building

```sh
cd { this repo }
git clone https://github.com/google/googletest.git third_party/googletest
git clone https://github.com/openframeworks/openFrameworks.git third_party/openFrameworks
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

- `plan.txt`

```txt
instance=../instances/sample.txt
agents=100
map_file=arena.map
seed=1
solver=PP
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

- `exec.txt`

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

## Visualizer

### Building
It takes around 10 minutes.

#### macOS 10.x
Please checkout openFrameworks to [this commit](https://github.com/openframeworks/openFrameworks/tree/b674f7ec1f41d8f0fcfea86e3d3d3df3e9bdcf36) before building.

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
Scripts for the experiments are in `exp_scripts/`.
All instances are included in `./instances.zip`.

## Notes
- Maps in `maps/` are from [Pathfinding Benchmarks](https://movingai.com/benchmarks/).
  When you add a new map, please place it in the `maps/` directory.
- The font in `visualizer/bin/data` is from [Google Fonts](https://fonts.google.com/).

## Licence
This software will be released under the MIT License.
