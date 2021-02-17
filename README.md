# ISSSort

Based on the code by Marc Labiche from Daresbury, i.e. copy and paste with some edits on top.
The CommandLineInterface code is stolen from [MiniballCoulexSort](https://github.com/Miniball/MiniballCoulexSort), which is in turn stolen from the TREX code by Vincenz Bildstein (TUM/Guelph).



## Download

```bash
git clone --recursive https://github.com/ISOLDESolenoidalSpectrometer/ISSSort
```

## Compile

Before compiling:
Edit midas2root.cc and change the file paths to yours as needed.
Also change the prefix of the MIDAS run file to be read (default: "R7", see file in scratch/). Todo: This would be best as an input argument 

```bash
make clean
make
```


## Execute

```
midas2root
```
if you add the ISSSort/bin to your path. You can also add ISSSort/lib to your (DY)LD_LIBRARY_PATH too.

or
```
./bin/midas2root
```

The output is written in the analysis/ subdirectory



## Dependencies

You also need to have ROOT installed. At the moment it works with v5 or v6, but let me know of any problems.


 
