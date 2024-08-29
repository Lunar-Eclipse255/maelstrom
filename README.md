# maelstrom

<img src="https://img.shields.io/badge/version-v0.4.7-blue?style=for-the-badge">

## Table of Contents
1. [Project Overview](#project-overview)
2. [Dependencies](#dependencies)
3. [File Structure](#file-structure)
4. [Key Features](#key-features)
5. [Usage Guide](#usage-guide)
6. [Autonomous Routines](#autonomous-routines)
7. [Operator Control](#operator-control)
8. [Display System](#display-system)
9. [PID Tuning](#pid-tuning)
10. [Debugging](#debugging)
11. [License](#license)

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
1. In `initialize()` in main.cpp call `init()`:
   ```cpp
      bool* init(bool run_error_log, bool run_data_log, std::vector<int> left_motor_ports, std::vector<int> right_motor_ports, int battery_threshold);
   ```
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

       
2. To dereference the returned pointer and assign it to a boolean value do this: (note outside of any functions initialize file_created ex. `bool file_created[2];`)
   ```cpp
       file_created[0] = temp[0];
       file_created[1] = temp[1];
       delete[] temp;
   ```

   
3. In different functions you can use the code provided below to check if a file was succesfully created before doing anything to the file, so if the file failed to create the code wouldn't attempt to write to it:
   ```cpp
      if (file_created[0]) {}
   ```
   * Use `file_created[0]` to check if the error_log file was succesfully created and `file_created[1]` to check if the data_log file was succesfully created

  
4. In `autonomous()` use this code to run the error logger in background
   ```cpp
      pros::Task error_logger(maelstrom::logging::robot_faults_log);
   ```


5. In `opcontrol()` and before the `while (true)` loop put this line of code that in the background writes to the data log file the current coordinates of the robot:
   ```cpp
      pros::Task coords_logging(maelstrom::logging::robot_coords_log);
   ```


6. In `opcontrol()` but inside the `while (true)` loop call this function that updates the coordinates that `maelstrom::logging::robot_coords_log()` uses
   ```cpp
      void set_robot_coords(double x, double y, double theta);
   ```
   * Change double x, double y, and double theta, for variables of the double type that hold those respective coordinates. For example:
     ```cpp
        maelstrom::logging::set_robot_coords(x_pos, y_pos, theta_heading);
     ```

     
## Extra Functions
1. In any function you can use `maelstrom::logging::write_to_file(std::string message, log_file file);` this function can be used to write a message to either the log or error file
   * The std::string will be the message and the log_file will be used to specify which file to write to, use `E_ERROR_LOG` to write to the error log file and `E_DATA_LOG` to write to the data log file.
   * This example will write the message Good Luck to the data log file:
       ```cpp
          maelstrom::logging::write_to_file("Good Luck", maelstrom::logging::E_DATA_LOG);
       ```
2. In any function you can use `task_complete(std::string task_name, bool completion);` to write to the error log file if a task was completed or not
   * It takes an std::string for the task name and a boolean to see if a task was completed or not
   * ex.)
       ```cpp
          maelstrom::logging::task_complete("Auton", auton_complete);
       ```
       would write `Auton Complete` to the error log file if auton_complete was true, and `Auton Incomplete` if auton_complete was false
3. In any function `battery(int battery_threshold);` can be used to see if the battery percentage is lower than an int threshold. This funtion is used in `robot_faults_log()` to logto the error log file if the battery is too low but can also be used seperately
   * In the example below it will return true if the battery percentage is above 50% and returns false if the battery percentage is less than or equal to 50%
      ```cpp
         maelstrom::logging::battery(50);
      ```
4. `motor_status(int port);` can be used to get if a motor is disconnected at a port specified by `int port`. It will return true if the motor is connected and false if the motor is disconnected. This funtion is used in `robot_faults_log()` to log to the error log file if a drive motor is disconnected but can also be used seperately
   * The example below checks if the motor at port 20 is connected
     ```cpp
        maelstrom::logging::motor_status(20);
     ```

5. `get_current_date_time();` returns the time since the program began in a std::string in the format `minutes:seconds:milliseconds` for example `1:50:55`
     ```cpp
        maelstrom::logging::get_current_date_time();
     ```
   
## Output and functionality of maelstrom functions

## Function Compatibility Notes
1. Every function except for `maelstrom::logging::motor_status()`, `maelstrom::logging::get_current_date_time()`, `maelstrom::logging::battery()`, and `maelstrom::logging::set_robot_coords()` needs the function `maelstrom::logging::init()` to have been called

2. `maelstrom::logging::robot_coords_log()` needs `maelstrom::logging::set_robot_coords()` to update the coordinates or `maelstrom::logging::robot_coords_log()` will always log the coordinates as NaN
