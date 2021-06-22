# ISSSort

Based on the code by Marc Labiche from Daresbury, i.e. copy and paste with some edits on top.
The CommandLineInterface code is stolen from [MiniballCoulexSort](https://github.com/Miniball/MiniballCoulexSort), which is in turn stolen from the TREX code by Vincenz Bildstein (TUM/Guelph).



## Download

```bash
git clone https://github.com/ISOLDESolenoidalSpectrometer/ISSSort
```

## Compile

```bash
make clean
make
```


## Execute

```
iss_sort
```
if you add the ISSSort/bin to your PATH variable. You can also add ISSSort/lib to your (DY)LD_LIBRARY_PATH too.

or
```
./bin/iss_sort
```

The input options are described below

```
use iss_sort with following flags:
        [-i <vector<string>>: List of input files]
        [-m <int           >: Monitor input file every X seconds]
        [-o <string        >: Output file for events tree]
        [-f                 : Flag to force new ROOT conversion]
        [-s                 : Flag to sort file by time]
        [-e                 : Flag to build physics events]
        [-c <string        >: Calibration file]
        [-h                 : Print this help]
```

## Sorting Philosophy

### Step 1: Converter
Running iss_sort with a list of input files, using the -i flag, will simple convert them to ROOT format.
This step produces one output file per input file, which has the name of the input file, appended with .root.
The ouptut file contains a single ROOT tree of the data and a series of diagnostic histograms.
If the output file already exists, iss_sort will skip this step unless the -f flag is used.

### Step 2: Calibration and Time Sorting
In order to combine timestamp and ADC data, the time sorting step needs to be performed by adding the -s flag.
This step will follow the conversion step and will produce a new output file, appended with _sort.root.
If a calibration file is added with the -c flag, the ADC data is calibrated for energy.
The format of the calibration file is discussed elsewhere, but it can also contain geometric information such as target-detector distance.
The output will contain two identical trees, one in the original data order (iss_calib), and a second tree sorted by time (iss_sort).

### Step 3: Event Builder
The final step is the event builder, which runs if the -e flag is used.
This uses all the calibrated, time sorted data from the previous step to produce a single output file, given by the -o flag.
Events are built according to the particles on the array, correlating n- and p-side data to give one particle per event in the evt_tree.
A particle has the properties of interaction position, p-side and n-side energies and also their respective time differences to the start of the event.
As part of this event, a list of recoils are also forseen to be included.

### Step 4: Histogramming
Not yet implemented...

## Dependencies

You also need to have ROOT installed. At the moment it works with v5 or v6, but let me know of any problems.


 
