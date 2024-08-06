# maelstrom

<img src="https://img.shields.io/badge/version-v0.4.4-blue?style=for-the-badge">

## Intro
maelstrom is a library for [PROS](https://pros.cs.purdue.edu/)

## Installing maelstrom library
1. In the integrated PROS terminal, run the command `pros c add-depot maelstrom https://lunar-eclipse255.github.io/maelstrom/template.json`

2.  `cd` into your pros project

3.  Make sure you are in the kernel version 4.1.0, your version can be found out with `pros c info-project` 

4. Apply the library to the project with `pros c apply maelstrom`

5. Put `#include "maelstrom/api.hpp"` in your main.h


## Alternative instructions for installing maelstrom library


1.  To get the library/template, head over to the releases page and download maelstrom@<version>.zip

2. Move the zip file to the pros project

3.  Make sure you are in the kernel version 4.1.0, your version can be found out with `pros c info-project` 

4. Run `pros c fetch maelstrom@<version>.zip` in order to import the template

5. Apply the library to the project with `pros c apply maelstrom`

6. Put `#include "maelstrom/api.hpp"` in your main.h


## Set-up

1. From this Github repository download the folder called logs

2. Put that folder in the root of the SD Card


## Alternative set-up

1. In the root of the SD Card make a folder called logs

2. In the folder logs make a file called run_nums.txt

3. On the first line of run_nums.txt write R0


## Using maelstrom
1. In initialize() in main.cpp call 'init()':
```cpp
bool* init(bool run_error_log, bool run_data_log, std::vector<int> left_motor_ports, std::vector<int> right_motor_ports, int battery_threshold);
```
-
    * 'bool run_error_log' is used if you want to create a file to log errors
    * 'bool run_data_log' is used if you want to create a file to log data such as coordinates
    * `std::vector<int> left_motor_ports``` is for inputting the ports of your left motors in the form of a std::vector
    * `std::vector<int> right_motor_ports`` is for inputting the ports of your right motors in the form of a std::vector
    * `int battery_threshold` is for inputting at what battery level do you want to be warned about
    * Here is an example:
       ```cpp
        std::vector<int> left_motors = {1, -2, 3};
        std::vector<int> right_motors = {-4, 5, -6};
        bool* temp = maelstrom::logging::init(true, true, left_motors, right_motors, 50);
        ```
2. To derefrence the returned pointer and assign it to a boolean value do this: (note outside of any functions initialize file_created ex, `bool file_created[2];`)
   ```cpp
       file_created[0] = temp[0];
       file_created[1] = temp[1];
       delete[] temp;
   ```
3. 
   

