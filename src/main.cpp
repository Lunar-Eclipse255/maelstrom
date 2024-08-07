#include "main.h"


std::vector<int> left_motors = {1, -2, 3};
std::vector<int> right_motors = {-4, 5, -6};
int battery_threshold = 50;
bool file_created[2];
bool auton_complete = false;


/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");
	pros::lcd::register_btn1_cb(on_center_button);
	
	bool* temp = maelstrom::logging::init(true, true, left_motors, right_motors, 50);
	file_created[0] = temp[0];
    file_created[1] = temp[1];
    delete[] temp; 
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {

	if (file_created[0]) {
		maelstrom::logging::write_to_file("Good Luck", maelstrom::logging::E_DATA_LOG);
        pros::Task error_logger(maelstrom::logging::robot_faults_log);
    }
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	if (file_created[0]) {
        maelstrom::logging::task_complete("Auton", auton_complete);
    }
	double x_pos = 0;
	double y_pos = 0;
	double theta_heading = 0;
	pros::Controller master(pros::E_CONTROLLER_MASTER);
	pros::MotorGroup left_mg({left_motors.begin(), left_motors.end()}); // Creates a motor group with forwards ports 1 & 3 and reversed port 2
	pros::MotorGroup right_mg({right_motors.begin(), right_motors.end()});  // Creates a motor group with forwards port 5 and reversed ports 4 & 6
	pros::Task coords_logging(maelstrom::logging::robot_coords_log);
	while (true) {
		x_pos = 0;
		y_pos = 0;
		theta_heading = 0;
		maelstrom::logging::set_robot_coords(x_pos, y_pos, theta_heading);
		pros::lcd::print(0, "%d %d %d", (pros::lcd::read_buttons() & LCD_BTN_LEFT) >> 2,
		                 (pros::lcd::read_buttons() & LCD_BTN_CENTER) >> 1,
		                 (pros::lcd::read_buttons() & LCD_BTN_RIGHT) >> 0);  // Prints status of the emulated screen LCDs

		// Arcade control scheme
		int dir = master.get_analog(ANALOG_LEFT_Y);    // Gets amount forward/backward from left joystick
		int turn = master.get_analog(ANALOG_RIGHT_X);  // Gets the turn left/right from right joystick
		left_mg.move(dir - turn);                      // Sets left motor voltage
		right_mg.move(dir + turn);                     // Sets right motor voltage
		pros::delay(20);                               // Run for 20 ms then update
	}
	maelstrom::logging::task_complete("Driver", true);
}