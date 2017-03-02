#define LOGGING 1

// Some robots are backwards
#define MOTORS_DIR false

// Pins
#define PIN_PWM_A 16
#define PIN_DIR_A 2

#define PIN_PWM_B 15
#define PIN_DIR_B 0

#define PIN_PWM_C 12
#define PIN_DIR_C 13

#define PIN_PEN 14

// Speeds
#define ROTATION_ERROR 0.5
#define ROTATION_SPEED_MAX 150
#define ROTATION_SPEED_MIN 85

// Robot states
#define STATE_WAITING 0
#define STATE_ROTATING 1
#define STATE_POSITIONING 2
#define STATE_DRAWING 3
#define STATE_MOVING_PEN 4

// Pen states
#define PEN_DOWN 1
#define PEN_UP 0

// Time for pen to move between states
#define PEN_MOVING_TIME 200

#define PEN_DOWN_VAL 160
#define PEN_UP_VAL 90
