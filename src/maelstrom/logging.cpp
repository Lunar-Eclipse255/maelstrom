//inclusions
#include "maelstrom/logging.hpp"
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <thread>
//puts it in the maelstrom namespace
namespace maelstrom {
    //makes a namespace called logging
    namespace logging {
        //declarartion of mutex
        pros::Mutex coords_mutex;
        //initializes the vector of robot_position as an empty vector
        std::vector<double> robot_coords_vector = {NAN, NAN, NAN};
        //declares filename variable for data and error logfiles
        static std::string data_log_filename;
        static std::string error_log_filename;
        //initializes teh rtf header
        std::string rtf_file_header = "{\\rtf1\\ansi\\deff0 {\\fonttbl {\\f0 Helvetica;}}{\\colortbl;\\red0\\green0\\blue0;\\red0\\green128\\blue0;\\red220\\green20\\blue60;\\red255\\green140\\blue0;}\\f0\\fs32\\cf0 ";
        //declares file stream objects for error and data logfiles
        std::fstream error_log_file;
        std::fstream data_log_file;
        //declares the vector of motor ports
        std::vector<int> motor_ports;
        //declares the initilization status array
        std::array<bool, 2> init_arr = {false, false};
        //declares the varaiable that holds the battery threshold
        int battery_threshold;
        //declares teh vector to hold motor faults
        std::vector<std::vector<bool>> faults;
        //creates a lookup table for error codes, to make them human readable
        std::vector<std::pair<pros::motor_fault_e_t, std::string>> pros_motor_faults = {
            {pros::E_MOTOR_FAULT_MOTOR_OVER_TEMP, " over temperature: "},
            {pros::E_MOTOR_FAULT_DRIVER_FAULT, " driver fault (H-bridge fault): "},
            {pros::E_MOTOR_FAULT_OVER_CURRENT, " over current: "},
            {pros::E_MOTOR_FAULT_DRV_OVER_CURRENT, " H-bridge over current: "}
        };



        /* declares a function to returs the time since the program began in a 
            std::string in the format minutes:seconds:milliseconds */
        std::string get_current_time() {
            //gets the current time in ms since the program started
            int milliseconds = pros::millis();
            //converts into min, s, and ms
            int seconds = milliseconds / 1000;
            milliseconds %= 1000;
            int minutes = seconds / 60;
            seconds %= 60;
            //returns a string in the format minutes:seconds:milliseconds
            return std::to_string(minutes) + ":" + std::to_string(seconds) + ":" + std::to_string(milliseconds);
        }

        //declares a function to initialize the log files
        bool* init(bool run_error_log, bool run_data_log, std::vector<int> left_motor_ports, std::vector<int> right_motor_ports, int battery_level){
            //pre-allocates memory for the motor port vector
            motor_ports.reserve(left_motor_ports.size() + right_motor_ports.size()); 
            //appends left motor ports to the motor port vector
            motor_ports.insert(motor_ports.end(), left_motor_ports.begin(), left_motor_ports.end());
            //appends right motor ports to the motor port vector
            motor_ports.insert(motor_ports.end(), right_motor_ports.begin(), right_motor_ports.end());
            //initializes the fault flag vector with five boolean flags set to false at default
            faults.resize(motor_ports.size(), std::vector<bool>(5, false));
            //initializes a string to hold the base file path
            std::string base_path = "/usd/logs/";
            //initializes the variable holding the battery level threshold
            battery_threshold = battery_level;
            //stores initialization status for the error and data log
            init_arr[0] = run_error_log;
            init_arr[1] = run_data_log;
            //opens the file to read what run number the program is on
            std::ifstream run_num_file("/usd/logs/run_nums.txt");
            //checks if the file wasn't opened
            if (!run_num_file.is_open()){
                //if it wasn't opened then set both initialization statuses to false
                init_arr[0] = false;
                init_arr[1] = false;
                //then prints to brain terminal that something went wrong with the file
                printf("Somethings is wrong with run_nums.txt\n");
                //returns pointer to initializaton status array
                return init_arr.data();
            }
            //if the file opens then it prints to brain terminal that the file opened succesfully
            else{
                printf("run_nums.txt successfully found and read\n");
            }
            //declares temporary string to store each line from file
            std::string line;
            //declares string to store the current run number
            std::string run_num;
            //declares string to store the integer version of run_num
            int run_num_int;
            //reads all lines from the run_num_file
            while (std::getline(run_num_file, line)) {
                //stores the last line read
                run_num = line;
            }
            //closes the file
            run_num_file.close();
            //removes teh R prefix from the run number
            run_num = run_num.substr(1);
            //stores the run_num as an int to run_num_int
            run_num_int = std::stoi(run_num);
            //increments the run number for the current run
            run_num_int++;
            //stores teh new run num to the run_num string with the R prefix
            run_num = "R" + std::to_string(run_num_int);
            //checks if the error logfile should be created
            if (run_error_log) {
                //sets the filename for the error logfile to be the base path + the run num + error_logfile.rtf
                error_log_filename = base_path + run_num + "_" + std::string("error_logfile") + ".rtf";
                //declares a file stream object to read and write to
                std::fstream error_log_file;
                //creates and opens the rtf file
                error_log_file.open(error_log_filename, std::ios::out);
                //checks if the file was created properly
                if (error_log_file.is_open()) {
                    //prints to brain terminal that the file was succesfully created
                    printf("%s created successfully\n", error_log_filename.c_str());
                    //writes to file the rtf header and a header that states when the program started
                    error_log_file << rtf_file_header << "\\cf0 Program Started: \\line \\line }";
                    //closes the file
                    error_log_file.close();
                }
                //if the file wasn't created succesfully
                else{
                    //sets the first index of the status array to false
                    init_arr[0] = false;
                    //prints to brain terminal that the file wasn't created succesfully
                    printf("Somethings is wrong with error_logfile\n");
                }
            }
            //checks if the data logfile should be created
            if(run_data_log) {
                //sets the filename for the data logfile to be the base path + the run num + data_logfile.rtf
                data_log_filename = base_path + run_num + "_" + std::string("data_logfile") + ".rtf";
                //declares a file stream object to read and write to
                std::fstream data_log_file;
                //creates and opens the rtf file
                data_log_file.open(data_log_filename, std::ios::out);
                //checks if the file was created properly
                if (data_log_file.is_open()) {
                    //prints to brain terminal that the file was succesfully created
                    printf("%s created successfully\n", data_log_filename.c_str());
                    //writes to file the rtf header and a header that states when the program started
                    data_log_file << rtf_file_header << "\\cf0 Program Started: \\line \\line }";
                    //closes the file
                    data_log_file.close();
                }
                //if the file wasn't created succesfully
                else{
                    //sets the second index of the status array to false
                    init_arr[1] = false;
                    //prints to brain terminal that the file wasn't created succesfully
                    printf("Somethings is wrong with data_logfile\n");
                }
            }
            //if either the data or error logfiles were created
            if (run_data_log || run_error_log) {
                //declares a filestream object for the run_num_file
                std::fstream run_num_file;
                //opens the run_num_file
                run_num_file.open("/usd/logs/run_nums.txt", std::ios::app);
                //appends teh current run number
                run_num_file << "\n" + run_num;
                //closes the file
                run_num_file.close();
            }
            //delays for 0.5s
            pros::delay(500);
            //returns pointer to initializaton status array
            return init_arr.data();
        }

        //declares a function to store motor faults, and write out errors to the error logfile
        void motor_fault_log (int port_index, uint32_t motor_fault) {
            //checks if error logfile was created
            if (init_arr[0]) {
                //declares a file stream object to read and write to
                std::fstream error_log_file;
                //opens the error logfile to append to
                error_log_file.open(error_log_filename, std::ios::app);
                //goes throught the vector of motor fault flags for the specified motor index
                for (int i = 0; i < pros_motor_faults.size(); i++) {
                    //checks if the motor is connected at the specified port
                    if (motor_connected(motor_ports.at(port_index))){
                        //checks if the motor is encountering a certain fault type
                        if (motor_fault & pros_motor_faults[i].first) {
                            //checks if the fault isn't currently stored in the motor fault vector
                            if (!faults[port_index][i]){
                                //writes to the error logfile which motor fault occured and at what port
                                error_log_file.seekp((long) error_log_file.tellp() - 1) << "\\cf3 Motor: " + std::to_string(abs(motor_ports.at(port_index))) + pros_motor_faults[i].second + "\\cf0 " + get_current_time() + "\\line }";
                                //sets that motor fault flag to be true in the vector
                                faults[port_index][i] = true;
                            }
                        }
                        else {
                            /* checks if the fault is currently stored in the motor fault vector
                                (this is to inform the user that the fault has cleared the first time) */
                            if (faults[port_index][i]){
                                //writes to the error logfile which motor fault cleared and at what port
                                error_log_file.seekp((long) error_log_file.tellp() - 1) << "\\cf2 Motor: " + std::to_string(abs(motor_ports.at(port_index))) + pros_motor_faults[i].second + "all clear: \\cf0 " + get_current_time() + "\\line }";
                            }
                            //sets that motor fault flag to be false in the vector
                            faults[port_index][i] = false;
                        }
                    }
                }
                //closes the error logfile
                error_log_file.close();
            }
        }

        //declares a function that returns if a motor is connected
        bool motor_connected(int port) {
            //stores the motor temperature to an int temperature
            int temperature = pros::c::motor_get_current_draw(port);
            //if the temperature is PROS_ERR (ie the motor DC'd) return false else return true
            return !(temperature == PROS_ERR);
        }

        //declares a function that returns if the battery percent is above the threshold
        bool battery(int threshold){
            //returns if the battery percent is above the threshold
            return pros::battery::get_capacity() > threshold;
        }

        //declares a function to manage writting to the error logfile
        void robot_faults_log() {
            //checks if the error logfile was created
            if (init_arr[0]) {
                //initializes auton_start, driver_start, and battery_below_threshold flags as false
                bool auton_start = false;
                bool driver_start = false;
                bool battery_below_threshold = false;
                //has a perpetual while true loop bc this is meant to run in a task
                while (true) {
                    //declares a file stream object to read and write to
                    std::fstream error_log_file;
                    //opens the error logfile to append to
                    error_log_file.open(error_log_filename, std::ios::app);
                    //checks if the robot isn't disabled
                    if (!pros::competition::is_disabled()){
                        //checks if it is currently auton and if auton_start is false
                        if (pros::competition::is_autonomous() && !auton_start) {
                            //writes to error logfile that auton has started
                            error_log_file.seekp((long) error_log_file.tellp() - 1) << "\\cf0 Auton: \\cf0 " + get_current_time() + "\\line \\line }";
                            //sets auton_start to true
                            auton_start = true;
                        }
                        //cehcks if it is currently driver and if driver_start is false
                        else if (!pros::competition::is_autonomous() && !driver_start){
                            //writes to error logfile that driver has started
                            error_log_file.seekp((long) error_log_file.tellp() - 1) << "\\line \\cf0 Driver: \\cf0 " + get_current_time() + "\\line \\line }";
                            //sets driver_start to true
                            driver_start = true;
                        }

                        for (int i = 0; i < motor_ports.size(); i++){
                            //closes the error log file
                            error_log_file.close();
                            //delays 50ms for file system stability
                            pros::delay(50);
                            /* updates the vector of motor fault flags for this motor, as well as to write 
                                errors regarding motor faults to the error logfile */
                            motor_fault_log(i, pros::c::motor_get_faults(motor_ports.at(i)));
                            //delays 50ms for processing time
                            pros::delay(50);
                            //reopens the error log file in append mode
                            error_log_file.open(error_log_filename, std::ios::app);
                            //checks if the motor is not connected at the current port
                            if (!(motor_connected(motor_ports.at(i)))) {
                                //checks if this disconnect was previosuly recorded
                                if (!faults[i][4]){
                                    //writes out to error logfile that the motor at said port disconnected
                                    error_log_file.seekp((long) error_log_file.tellp() - 1) << "\\cf3 Motor: " + std::to_string(abs(motor_ports.at(i))) + " disconnected: \\cf0 " + get_current_time() + "\\line }";
                                    //updates the motor fault flag vector to match
                                    faults[i][4] = true;
                                }
                            }
                            else{
                                /* checks if the disconnect is currently stored in the motor fault vector
                                (this is to inform the user that the disconnect has cleared the first time) */
                                if (faults[i][4]){
                                    //writes out to error logfile that the motor at said port has reconnected
                                    error_log_file.seekp((long) error_log_file.tellp() - 1) << "\\cf2 Motor: " + std::to_string(abs(motor_ports.at(i))) + " reconnected: \\cf0 " + get_current_time() + "\\line }";
                                }
                                //updates the motor fault flag vector to match
                                faults[i][4] = false;
                            }
                        }
                        //checks if the battery is below the threshold
                        if (!(battery(battery_threshold))) {
                            //checks if the battery_below_threshold flag was already set to true
                            if(!battery_below_threshold){
                                //writes to the error logfile that the battery is below the threshold
                                error_log_file.seekp((long) error_log_file.tellp() - 1) << "\\cf4 Battery below " + std::to_string(battery_threshold) + "% \\cf0 " + get_current_time() + "\\line }";
                                //sets the battery_below_threshold flag to true
                                battery_below_threshold = true;
                            }
                        }
                    }
                    //500 ms delay for file system stability
                    pros::delay(500);
                    //closes the error logfile
                    error_log_file.close();
                }
            }
        }

        //declares the function to format cordinates as a string
        std::string format_coords(double value) {
            //checks if the coord is NaN
            if (std::isnan(value)) {
                return "NaN";
            }
            //creates a string stream for formatting
            std::stringstream ss;
            //formats coord to 2 decimal places
            ss << std::fixed << std::setprecision(2) << value;
            //formats value to 2 decimal places
            return ss.str();
        }  

        //declares function to set and store the x,y, and theta position of the robot
        void set_robot_coords(double x, double y, double theta) {
            //locks the mutex to prevent concurrent access to coordinates
            coords_mutex.take(TIMEOUT_MAX);
            //stores x coordinate in vector index 0
            robot_coords_vector.at(0) = x;
            //stores y coordinate in vector index 1
            robot_coords_vector.at(1) = y;
            //stores theta coordinate in vector index 2
            robot_coords_vector.at(2) = theta;
            //realeases mutex
            coords_mutex.give();
        }

        void robot_coords_log() {
            if (init_arr[1]) {
                while (true) {
                    coords_mutex.take(TIMEOUT_MAX);
                    if (!robot_coords_vector.empty()) {
                        std::fstream data_log_file;
                        data_log_file.open(data_log_filename, std::ios::app);
                        data_log_file.seekp((long) data_log_file.tellp() - 1) << "x: " + format_coords(robot_coords_vector.at(0)) + " y: " + format_coords(robot_coords_vector.at(1)) + " theta: " + format_coords(robot_coords_vector.at(2)) + " " + get_current_time() + "\\line }";
                        data_log_file.close();
                    }
                    coords_mutex.give();
                    pros::delay(500);
                }
            }
        }

        void write_to_file(std::string message, log_file file) {
            if (file == E_ERROR_LOG){
                if (init_arr[0]) {
                    std::fstream error_log_file;
                    error_log_file.open(error_log_filename, std::ios::app);
                    error_log_file.seekp((long) error_log_file.tellp() - 1) << message + " " + get_current_time() + "\\line }";
                    error_log_file.close();
                }
            }
            else{
                if (init_arr[1]) {
                    std::fstream data_log_file;
                    data_log_file.open(data_log_filename, std::ios::app);
                    data_log_file.seekp((long) data_log_file.tellp() - 1) << message + " " + get_current_time() + "\\line }";
                    data_log_file.close();
                }
            }
        }

        void task_complete(std::string task_name, bool completion) {
            if (init_arr[0]) {
                std::fstream error_log_file;
                error_log_file.open(error_log_filename, std::ios::app);
                if (completion){
                    error_log_file.seekp((long) error_log_file.tellp() - 1) << "\\cf2 " + task_name + " Complete \\cf0 " + get_current_time() + "\\line }";
                }
                else{
                    error_log_file.seekp((long) error_log_file.tellp() - 1) << "\\cf4 " + task_name + " Incomplete \\cf0 " + get_current_time() + "\\line }";
                }
            }
        }
    }
}