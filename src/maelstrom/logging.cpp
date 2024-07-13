//inclusions
#include "maelstrom/logging.hpp"
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <thread>
//puts it in the tempest namespace
namespace maelstrom {
    //makes a namespace called log
    namespace logging {
        const char* timezone = "ESTEDT";
        std::vector<double> robot_coords_vector;
        pros::mutex_t robot_coords_mutex;
        std::string data_log_filename;
        std::string error_log_filename;
        std::fstream error_log_file;
        std::fstream data_log_file;
        std::vector<int> motor_ports;
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


        std::string get_current_date_time(date_time_format format) {
            setenv("TZ", "ESTEDT", 1);
            tzset();
            std::time_t now = std::time(nullptr);
            std::tm* tm_info = std::localtime(&now);
            char buffer[20];

            switch(format) {
                case E_YEAR: 
                    std::strftime(buffer, 20, "%Y", tm_info);
                    break;
                case E_MONTH:
                    std::strftime(buffer, 20, "%m", tm_info);
                    break;
                case E_DAY:
                    std::strftime(buffer, 20, "%d", tm_info);
                    break;
                case E_DATE: 
                    std::strftime(buffer, 20, "%d-%m-%Y", tm_info);
                    break;
                case E_TIME:
                    std::strftime(buffer, 20, "%H-%M", tm_info);
                    break;
                case E_DATE_TIME:
                    std::strftime(buffer, 20, "%d-%m-%Y_%H-%M", tm_info);
                    break;
                case E_DATE_TIME_FILE_PATH_FORMAT:
                    std::strftime(buffer, 20, "%d-%m-%Y_%H-%M", tm_info);
                    break;
            }
            return std::string(buffer);
        }

        bool* init(bool run_error_log, bool run_data_log, std::vector<int> left_motor_ports, std::vector<int> right_motor_ports){
            motor_ports.reserve(left_motor_ports.size() + right_motor_ports.size()); 
            motor_ports.insert(motor_ports.end(), left_motor_ports.begin(), left_motor_ports.end());
            motor_ports.insert(motor_ports.end(), right_motor_ports.begin(), right_motor_ports.end());
            std::string base_path = "/usd/logs/";
            struct stat sb;
            bool folder_status = true;
            std::string log_folder_path;
            bool* init_arr = new bool[2];
            init_arr[0] = run_error_log;
            init_arr[1] = run_data_log;
            if (run_error_log || run_data_log){
                std::string path_year = base_path + get_current_date_time(E_YEAR);
                if (stat(path_year.c_str(), &sb) != 0) {
                    if (mkdir(path_year.c_str(), 0777) != 0) {
                        init_arr[0] = false, init_arr[1] = false, folder_status = false;
                    }
                }

                // Create directory month
                std::string path_month = path_year + "/" + get_current_date_time(E_MONTH);
                if (stat(path_month.c_str(), &sb) != 0) {
                    if (mkdir(path_month.c_str(), 0777) != 0) {
                        init_arr[0] = false, init_arr[1] = false, folder_status = false;
                    }
                }

                // Create directory day
                std::string path_day = path_month + "/" + get_current_date_time(E_DAY);
                if (stat(path_day.c_str(), &sb) != 0) {
                    if (mkdir(path_day.c_str(), 0777) != 0) {
                        init_arr[0] = false, init_arr[1] = false, folder_status = false;
                    }
                }
                std::string log_folder_path = path_day + "/" + get_current_date_time(E_DATE_TIME);
                if (mkdir(log_folder_path.c_str(), 0777) != 0) {
                    init_arr[0] = false, init_arr[1] = false, folder_status = false;
                }
            }
            if (run_error_log && folder_status) {
                std::string error_log_filename = log_folder_path + std::string("/error_logfile_") + get_current_date_time(E_DATE_TIME) + ".txt";
                std::fstream error_log_file;
                error_log_file.open(error_log_filename, std::ios::out);
                if (error_log_file.is_open()) {
                    error_log_file << get_current_date_time(E_DATE_TIME) + "\n \n";
                    error_log_file.close();
                }
                else{
                    init_arr[0] = false;
                }
            }
            if(run_data_log && folder_status) {
                std::string data_log_filename = log_folder_path + std::string("/data_logfile_") + get_current_date_time(E_DATE_TIME) + ".txt";
                std::fstream data_log_file;
                data_log_file.open(data_log_filename, std::ios::out);
                if (data_log_file.is_open()) {
                    data_log_file << get_current_date_time(E_DATE_TIME) + "\n \n";
                    data_log_file.close();
                }
                else{
                    init_arr[1] = false;
                }
            }
            return init_arr;
        }

        void motor_fault_log (int port_index, uint32_t motor_fault) {
            for (int i = 0; i < pros_motor_faults.size(); i++) {
                if (motor_fault & pros_motor_faults[i].first) {
                    if (!faults[port_index][i]){
                        error_log_file << "Motor: " + std::to_string(abs(motor_ports.at(port_index))) + pros_motor_faults[i].second + get_current_date_time(E_TIME) + "\n";
                        faults[port_index][i] = true;
                    }
                }
                else {
                    if (faults[port_index][i]){
                        error_log_file << "Motor: " + std::to_string(abs(motor_ports.at(port_index))) + pros_motor_faults[i].second + "all clear: " + get_current_date_time(E_TIME) + "\n";
                    }
                    faults[port_index][i] = false;
                }
            }
            
        }

        bool motor_status(int port) {
            int temperature = pros::c::motor_get_current_draw(port);
            return !(temperature == PROS_ERR);
        }

        bool battery(){
            return pros::battery::get_capacity()>50;
        }

        void motor_faults() {
            bool faults[motor_ports.size()][5];
            bool battery_low = false;
            bool auton_start = false;
            bool driver_start = false;
            for (int i = 0; i < motor_ports.size(); ++i) {
                for (int j = 0; j < 4; ++j) {
                    faults[i][j] = false;
                }
            }
            while (true) {
                std::fstream error_log_file;
                error_log_file.open(error_log_filename, std::ios::out);
                if (!pros::competition::is_disabled()){
                    if (pros::competition::is_autonomous() && !auton_start) {
                        error_log_file << "Auton: \n";
                        auton_start = true;
                    }
                    else if (!driver_start){
                        error_log_file << "Driver: \n";
                        driver_start = true;
                    }
                    for (int i = 0; i < motor_ports.size(); i++){
                        motor_fault_log(i, pros::c::motor_get_faults(motor_ports.at(i)));
                        if (!(motor_status(motor_ports.at(i)))) {
                            if (!faults[i][4]){
                                error_log_file << "Motor: " + std::to_string(abs(motor_ports.at(i))) + " disconnected: " + get_current_date_time(E_TIME) + "\n";
                                faults[i][4] = true;
                            }
                        }
                        else{
                            if (faults[i][4]){
                                error_log_file << "Motor: " + std::to_string(abs(motor_ports.at(i))) + " reconnected: " + get_current_date_time(E_TIME) + "\n";
                            }
                            faults[i][4] = false;
                        }
                    }
                    if (!(battery())) {
                        error_log_file << "Battery below 50%" + get_current_date_time(E_TIME) + "\n";
                    }
                }
                pros::delay(500);
                error_log_file.close();
            }
        }

        void set_robot_coords(double x, double y, double theta) {
            pros::c::mutex_take(robot_coords_mutex, TIMEOUT_MAX);
            robot_coords_vector.push_back(x);
            robot_coords_vector.push_back(y);
            robot_coords_vector.push_back(theta);
            pros::c::mutex_give(robot_coords_mutex);
        }

        void robot_coords_log() {
            while (true) {
                pros::c::mutex_take(robot_coords_mutex, TIMEOUT_MAX);
                if (!robot_coords_vector.empty()) {
                    std::fstream data_log_file;
                    data_log_file.open(data_log_filename, std::ios::out);
                    data_log_file << "x: " + std::to_string(robot_coords_vector.at(0)) + " y: " + std::to_string(robot_coords_vector.at(1))+  " theta: " + std::to_string(robot_coords_vector.at(2)) + get_current_date_time(E_TIME) + "\n";
                    data_log_file.close();
                }
                pros::c::mutex_give(robot_coords_mutex);
                pros::delay(500);
            }
        }

        void task_complete(std::string task_name, bool completion) {
            std::fstream error_log_file;
            error_log_file.open(error_log_filename, std::ios::out);
            if (completion){
                error_log_file << task_name + " Complete" + get_current_date_time(E_TIME) + "\n";
            }
            else{
                error_log_file << task_name + " Incomplete" + get_current_date_time(E_TIME) + "\n";
            }
        }
    }
}