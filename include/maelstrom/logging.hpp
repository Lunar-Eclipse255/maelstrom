#ifndef _LOGGING_HPP_
#define _LOGGING_HPP_

//inclusions
#include "api.h"

namespace maelstrom {
    namespace logging {
        //declarations of namespace functions
        typedef enum {
            E_YEAR,
            E_MONTH,
            E_DAY,
            E_DATE,
            E_TIME,
            E_DATE_TIME,
            E_DATE_TIME_FILE_PATH_FORMAT
        } date_time_format;

        bool* init(bool run_error_log, bool run_data_log, std::vector<int> left_motor_ports, std::vector<int> right_motor_ports);
        std::string get_current_date_time(date_time_format format);
        bool motor_status(int port);
        bool battery();
        void motor_faults();
        void task_complete(std::string task_name, bool completion);
    }
}  

#endif