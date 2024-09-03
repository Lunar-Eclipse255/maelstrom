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
//puts it in the tempest namespace
namespace maelstrom {
    //makes a namespace called log
    namespace logging {
        pros::Mutex coords_mutex;
        const char* timezone = "ESTEDT";
        std::vector<double> robot_coords_vector = {NAN, NAN, NAN};
        pros::mutex_t robot_coords_mutex;
        static std::string data_log_filename;
        static std::string error_log_filename;
        std::fstream error_log_file;
        std::fstream data_log_file;
        std::vector<int> motor_ports;
        int battery_threshold;
        bool faults[][5] = { };
        std::vector<std::pair<pros::motor_fault_e_t, std::string>> pros_motor_faults = {
            {pros::E_MOTOR_FAULT_MOTOR_OVER_TEMP, " over temperature: "},
            {pros::E_MOTOR_FAULT_DRIVER_FAULT, " driver fault (H-bridge fault): "},
            {pros::E_MOTOR_FAULT_OVER_CURRENT, " over current: "},
            {pros::E_MOTOR_FAULT_DRV_OVER_CURRENT, " H-bridge over current: "}
        };

        void set_timezone (const char* timezone_string) {
            timezone = timezone_string;
        }


        std::string get_current_date_time() {
            int milliseconds = pros::millis();
            int seconds = milliseconds / 1000;
            milliseconds %= 1000;
            int minutes = seconds / 60;
            seconds %= 60;
            return std::to_string(minutes) + ":" + std::to_string(seconds) + ":" + std::to_string(milliseconds);
        }

        bool* init(bool run_error_log, bool run_data_log, std::vector<int> left_motor_ports, std::vector<int> right_motor_ports, int battery_level){
            motor_ports.reserve(left_motor_ports.size() + right_motor_ports.size()); 
            motor_ports.insert(motor_ports.end(), left_motor_ports.begin(), left_motor_ports.end());
            motor_ports.insert(motor_ports.end(), right_motor_ports.begin(), right_motor_ports.end());
            std::string base_path = "/usd/logs/";
            battery_threshold = battery_level;
            struct stat sb;
            std::string log_folder_path;
            bool* init_arr = new bool[2];
            init_arr[0] = run_error_log;
            init_arr[1] = run_data_log;
            std::ifstream run_num_file("/usd/logs/run_nums.txt");
            if (!run_num_file.is_open()){
                init_arr[0] = false;
                init_arr[1] = false;
                printf("Somethings is wrong with run_nums.txt\n");
                return init_arr;
            }
            std::string line;
            std::string run_num;
            int run_num_int;
            while (std::getline(run_num_file, line)) {
                run_num = line;
            }
            run_num_file.close();
            run_num = run_num.substr(1);
            run_num_int = std::stoi(run_num);
            run_num_int++;
            run_num = "R" + std::to_string(run_num_int);
            
            if (run_error_log) {
                error_log_filename = base_path + run_num + "_" + std::string("error_logfile_") + ".txt";
                //std::string error_log_filename = log_folder_path + std::string("/error_logfile_") + get_current_date_time(E_DATE_TIME) + ".txt";
                std::fstream error_log_file;
                error_log_file.open(error_log_filename, std::ios::out);
                if (error_log_file.is_open()) {
                    error_log_file << "Program Started: \n \n";
                    error_log_file.close();
                }
                else{
                    init_arr[0] = false;
                    printf("Somethings is wrong with error_logfile\n");
                }
            }
            if(run_data_log) {
                data_log_filename = base_path + run_num + "_" + std::string("data_logfile_") + ".txt";
                //std::string data_log_filename = log_folder_path + std::string("/data_logfile_") + get_current_date_time(E_DATE_TIME) + ".txt";
                std::fstream data_log_file;
                data_log_file.open(data_log_filename, std::ios::out);
                if (data_log_file.is_open()) {
                    data_log_file << "Program Started: \n \n";
                    data_log_file.close();
                }
                else{
                    init_arr[1] = false;
                    printf("Somethings is wrong with data_logfile\n");
                }
            }
            if (run_data_log || run_error_log) {
                std::fstream run_num_file;
                run_num_file.open("/usd/logs/run_nums.txt", std::ios::app);
                run_num_file << "\n" + run_num;
                run_num_file.close();
            }
            return init_arr;
        }

        void motor_fault_log (int port_index, uint32_t motor_fault) {
            std::fstream error_log_file;
            error_log_file.open(error_log_filename, std::ios::app);
            for (int i = 0; i < pros_motor_faults.size(); i++) {
                if (motor_fault & pros_motor_faults[i].first) {
                    if (!faults[port_index][i]){
                        error_log_file << "Motor: " + std::to_string(abs(motor_ports.at(port_index))) + pros_motor_faults[i].second + get_current_date_time() + "\n";
                        faults[port_index][i] = true;
                    }
                }
                else {
                    if (faults[port_index][i]){
                        error_log_file << "Motor: " + std::to_string(abs(motor_ports.at(port_index))) + pros_motor_faults[i].second + "all clear: " + get_current_date_time() + "\n";
                    }
                    faults[port_index][i] = false;
                }
            }
            error_log_file.close();
            
        }

        bool motor_status(int port) {
            int temperature = pros::c::motor_get_current_draw(port);
            return !(temperature == PROS_ERR);
        }

        bool battery(int threshold){
            return pros::battery::get_capacity() > threshold;
        }

        void robot_faults_log() {
            bool faults[motor_ports.size()][5];
            bool battery_low = false;
            bool auton_start = false;
            bool driver_start = false;
            bool battery_below_threshold = false;
            for (int i = 0; i < motor_ports.size(); ++i) {
                for (int j = 0; j <= 4; ++j) {
                    faults[i][j] = false;
                }
            }
            while (true) {
                std::fstream error_log_file;
                error_log_file.open(error_log_filename, std::ios::app);
                if (!pros::competition::is_disabled()){
                    if (pros::competition::is_autonomous() && !auton_start) {
                        error_log_file << "Auton: " + get_current_date_time() + "\n \n";
                        auton_start = true;
                    }
                    else if (!pros::competition::is_autonomous() && !driver_start){
                        error_log_file << "\n" + std::string("Driver: ") + get_current_date_time() + "\n \n";
                        driver_start = true;
                    }
                    for (int i = 0; i < motor_ports.size(); i++){
                        motor_fault_log(i, pros::c::motor_get_faults(motor_ports.at(i)));
                        if (!(motor_status(motor_ports.at(i)))) {
                            if (!faults[i][4]){
                                error_log_file << "Motor: " + std::to_string(abs(motor_ports.at(i))) + " disconnected: " + get_current_date_time() + "\n";
                                faults[i][4] = true;
                            }
                        }
                        else{
                            if (faults[i][4]){
                                error_log_file << "Motor: " + std::to_string(abs(motor_ports.at(i))) + " reconnected: " + get_current_date_time() + "\n";
                            }
                            faults[i][4] = false;
                        }
                    }
                    if (!(battery(battery_threshold))) {
                        if(!battery_below_threshold){
                            error_log_file << "Battery below " + std::to_string(battery_threshold) + "% " + get_current_date_time() + "\n";
                            battery_below_threshold = true;
                        }
                    }
                }
                pros::delay(500);
                error_log_file.close();
            }
        }

        std::string format_coords(double value) {
            if (std::isnan(value)) {
                return "NaN";
            }
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << value;
            return ss.str();
        }  

        void set_robot_coords(double x, double y, double theta) {
            coords_mutex.take(TIMEOUT_MAX);
            robot_coords_vector.at(0) = x;
            robot_coords_vector.at(1) = y;
            robot_coords_vector.at(2) = theta;
            coords_mutex.give();
        }

        void robot_coords_log() {
            while (true) {
                coords_mutex.take(TIMEOUT_MAX);
                if (!robot_coords_vector.empty()) {
                    std::fstream data_log_file;
                    data_log_file.open(data_log_filename, std::ios::app);
                    data_log_file << "x: " + format_coords(robot_coords_vector.at(0)) + " y: " + format_coords(robot_coords_vector.at(1)) + " theta: " + format_coords(robot_coords_vector.at(2)) + " " + get_current_date_time() + "\n";
                    data_log_file.close();
                }
                coords_mutex.give();
                pros::delay(500);
            }
        }

        void write_to_file(std::string message, log_file file) {
            if (file == E_ERROR_LOG){
                std::fstream error_log_file;
                error_log_file.open(error_log_filename, std::ios::app);
                error_log_file << message + " " + get_current_date_time() + "\n";
                error_log_file.close();
            }
            else{
                std::fstream data_log_file;
                data_log_file.open(data_log_filename, std::ios::app);
                data_log_file << message + " " + get_current_date_time() + "\n";
                data_log_file.close();
            }
        }

        void task_complete(std::string task_name, bool completion) {
            std::fstream error_log_file;
            error_log_file.open(error_log_filename, std::ios::app);
            if (completion){
                error_log_file << task_name + " Complete " + get_current_date_time() + "\n";
            }
            else{
                error_log_file << task_name + " Incomplete " + get_current_date_time() + "\n";
            }
        }
    }
}