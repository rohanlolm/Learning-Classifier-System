Psuedo-Michigan Style Supervidsed Learning Classifier System (LCS) Readme
============
### Typeset in Markdown - Best viewed in a Markdown Interpreter. 
### Rohan Mehra - rohan.lol.m@gmail.com
### Date: 1/06/2017
### Please get in contact on the email above if you'd like to learn more, or get access to my thesis and performance results. 
________________

The PXR-UCS
----------------
The PXR-UCS is a machine learning algorithm developed for adaptive schedule optimisation, but can also be used as a general purpose classification algorithm. It implements a novel, pseudo-Pittsburgh style, accuracy based, supervised learning classifier system, and was built as part of a full year CEED thesis project. This project is extensively documented in my  thesis: *Learning Classifier System for Adaptive Schedule Optimisation*. 

Installation Requirements
----------------------------
In order to run the PXR-UCS system, the following must be installed: 

- Python 3 with Anaconda Libraries installed 
- GCC C compiler 
___________________________________

## Compiling 
The project is intended to be compiled with ``gcc`` under windows. A batch file ``make.bat`` has been provided to compile/build the project. Simply run ``make.bat`` in the command window and an executable will be built. The `march=native` compiler option speeds up execution but preduces an executable which is architecture specific. This option should be removed from the  `make` file for cross-platform executable generation. 

Currently the **LCS\_C\_imp** has the following files: 

### Header Files: 
0.   ``LCS_protos.h`` : Contains all function prototypes and ``#define`` constants 
0. ``LCS_structs.h``: Contains all structure and type definitions 

### C Source Files 
0. ``LCS_main.c``:  Contains all core LCS functions as well as the ``main()`` function. 
0. ``LCS_datafs.c``: Contains all functions for importing data, creating data structures, and populating data structures. 
0. ``LCS_helperfs.c``: Contains all helper functions, such as random number generators, sorting, cleaning, etc. 
0. ``LCS_geneticfs.c``: Contains all functions required for the GA to function correctly. Including evolution, mutation functions, and children updaters. 
_______________________

Quick Start
---------------------
To run the PXR-UCS system move the following files to a directory:

- The CSV data set to use as  a data source
- All LCS C source code files, or alternatively an LCS executable. Source files are listed in `C_commline_interfcace.md`. 
- The Application Interface `data_parser_cmd.py` Python script. 
- An appropriate configuration `.ini` file for your data 

To then run the system, open the console and navigate to the local directory, and enter:

`data_parser_cmd.py conifg.ini`

Where `confi.ini` is the appropriate config file.

### Output:
Once complete the system will output a trained LCS model file (Ruleset) `.lcs` file, Ruleset text file `.txt` (with names specified in the config file). The `.lcs` file can be opened with the `.lcs` tester and used to provide predictions for a given data set with the selected model. The text file can be used for manual analysis of the Ruleset.  
___________________

Configuration Files
--------------------
The system is interacted with though a `Configuration File`. 

There are 5 key sections to the config file as stated below: 

### LCS:
- `recompile`: Set to 1 to recompile to the LCS C code, else 0 to use an existing executable build. 

### Datafile_Names:
- `is_mux`: Enable to 1 if using Multiplexer data set. Else 0 
- `input_datafilename`: Name of input data set. Typically a `.CSV` file
- `training_filename`:  An output plaintext file is generated through the data parser containing training data. This is the name of the created file.
- `validation_filename`:An output plaintext file is generated through the data parser containing  validation data. This is the name of the created file.
- `testing_filename`: An output plaintext file is generated through the data parser containing testing data. This is the name of the created file. 
- `pickle_filename`: The name of the output Rulset model binary file. Typically choose a `.lcs` file extension
- `rule_text_filename`: Output text file name. 

### Columns 

- `predictor_names`: Column headings names for predictors/features as a comma separated list. 
- `class_names`: Column heading names for output class columns as comma separated list. 

### GA_params
The following parameters define the parameters of the LCS Hyper-parameter tuning GA.

- `pop_init_size`: Initial population size round 50 works. 
- `num_gen`: Number of generations to run. 50 works again 
- `default_opts`: Defualt LCS options which are not operated on by the GA. Typically: `-p -l 1 -s 1` is recommended. Note, `-p` must be enabled for the GA to work. 
- `upper_lims`: A triple comma separated vaue specifying the maximum limits for parameter values. Typically: `0.99,2,0.99`
- `fit_weights`: Weighting for GA to convert accuracy to fitness. Typically: `1000,0`. Note, the second parameter is used to weight smaller Rulesets and is typically set to 0. 

### Other: 
The following parameters are used to set train/test data splits and PRNG seeds. 

- `global_seed`: Global PRNG seed for Python data parser and GA. That is, the seed for the Application interface. 
- `file_shuffle_seed`: Used to seed the PRNG for shuffling train, test, validate sets. 
- `train_percent`: Fraction of data to split into training set Typically 0.6 
- `validation_percent`: Fraction of data to split into validation set Typically 0.2. The remaining data gets put into the  testing set. 

PXR-UCS C99 Command Line Interface 
=========================
This program implements a *pseudo-Pittsburgh*, accuracy based, supervised Learning Classifier System on for multi-class classification, and adaptive scheduling. This program is **not** intended to be used in isolation,  but rather with the companion Python Application Interface and associated configuration file. 
___________________________

## Versions
### Arbitrary Data (A-PXR-UCS): 
The arbitrary data PXR-UCS can be used for any real-valued, multi-class classification problems.

___________________________

## Arbitrary Data Version (A-PXR-UCS)
The A-PXR-UCS must be called in the following way, with 5 command line arguments and a combination of possible options. 

`./LCS [-opt] <training_filename, num_predictors (int), num_train_rows (int), testing_filename, num_test_rows (int)>`

**Command Line Arguments:**

- `training_filename`: Plain text file which contains white space separated, real-valued, ASCII encoded training data. The first column contains an indexing `key`, the next `n`  variables are real valued features, finally followed by an integer classification label as the last column. 
- `num_predictors`: An integer value specifying the number of features. Equal to `n`. 
- `num_train_rows`: An integer specifying the number of data instances/rows in the training data file. 
- `testing_filename`: Plain text file of the same format as `training_filename`, but contains testing data. 
- `num_test_rows`: An integer specifying the number of data instances/rows in the testing data file. 

The following options have also been implemented:

**Switch Options:**  
`-r`: Enables ``rule_writer()`` which prints all found rules to text file *Rules.txt*   
`-f`: Enables `rule_set_pickler()` which serialises the Ruleset and saves it in a `.lcs` binary file.    
`-p`: Enables Python printing. Output is formatted for Python Caller. Default is console printer.   
`-t`: Switch ``kill_bad_rules`` on for every row of training rather than after training. Seems to have minimal effect.   
`-g`: Enable the GA - off by default. The `-e` option does nothing if `-g` is not also enabled.  

**Options with Arguments:**   
`-l`: The number of retraining loops. Default 1  
`-s`: PRNG Seed. Default 1    
`-k`: Kill threshold for rules of poor accuracy. Default 0.7  - typically tuned by Python Caller  
`-d`: Spread percentage in values for new rules. Default 0.075    - typically tuned by Python Caller  
`-w`: Wild Card probability in rule. Default 0.15  - typically tuned by Python Caller  
`-e`: Set number of Evolution Iterations. Default 100
`-i`: For iterative testing. If enabled, LCS prints Ruleset accuracy after `k` iterations. Recommend ~100.    
`-m`: GA genome mutation probability   
`-n`: GA nomination mutation probability 

______________________

PXR-UCS Data Sets 
============
More detailed information about data sets presented below can be found in my thesis in `Section 4: Experimental Validation`. Associated with each data set type is a pre-populated  `.ini` configuration file, to be able to run the *PXR-UCS* System. 
_____________
## Generic Data Sets
The following data sets were used to test the PXR-UCS algorithm's potential as a generalised classifier. These data sets are from the UCI machine learning data base and are publically available. Except for the MUX dataset which was generated through a Python script. 

**Files:**

* `banknote_data.txt`: Bank Note Fraud Detection Data set
* `bank_config.ini`: Config file for bank dataset 
*  `iris_data.txt`: Iris data set
* `iris_config.ini`: Config file for iris dataset 
* `mux_testing.txt`, `mux_training.txt`, `mux_validation.txt`
* `mux_config.ini`: Config file for MUX data sets. All three of the above files must be present for the file to work. 

___________







