# CellularAutomataC
A Cellular Automata Model in C language using Raylib for wildfire simulation

# Dependencies 
* ([Raylib](https://www.raylib.com/))

# Code compilation for Graphical Interface (WINDOWS):
`gcc cellularAutomata.c automata.c -lraylib -lopengl32 -lgdi32 -lwinmm`


# Code compilation for Graphical Interface (LINUX):
`./compile.sh`

# Code compilation for Mass Experiments (WINDOWS and LINUX):
`gcc cellularAutomataMassExperiments.c automata.c`

# To print graphs from the output in the Mass Experiments, run the python script graphGenerator.py:
`py graphGenerator.py`

# Running executable
`./<FileName>`
