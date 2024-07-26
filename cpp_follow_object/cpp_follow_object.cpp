#include "ev3dev.h"
#include <iostream>

using namespace ev3dev;
using namespace std;


#define D_MIN 30   // min. distance to object
#define D_MAX 150  // max. range
#define DEG 60     // max. rotation degree


// calculate appropriate wheel speed depending on distance
int calc_speed(float d) {

    float thresh = (D_MAX-D_MIN)*0.5 + D_MIN;

    if (d < D_MIN || d > D_MAX) return 0;    // no speed if out of range
    else if (d > thresh)        return 100;  // max. speed if further away
    else                        return ((float)thresh-D_MIN)/100 * d + D_MIN;
}

// calculate wheel turning speed depending on angle
int calc_turn_speed(int speed, float angle) {
    return speed - (float)speed/DEG * abs(angle);
}

// drive to nearest (moving) object
void drive_to_object() {

    // EV3 components
    motor m_left(OUTPUT_A, motor::motor_large);
    motor m_right(OUTPUT_B, motor::motor_large);
    motor m_uss(OUTPUT_D, motor::motor_medium);
    touch_sensor ts(INPUT_1);
    ultrasonic_sensor uss(INPUT_4);

    // initial state (0 = STANDBY, 1 = ACTIVE)
    int state = 0;

    // pressed touch sensor on edge
    bool press;
    bool edge = false;

    // ultrasonic sensor variables
    int d = D_MAX;
    int speed_left = 0, speed_right = 0;
    int speed, turn_speed;
    float angle;

    m_uss.set_position(0);
    m_uss.set_duty_cycle_sp(100);

    cout << "Press enter to exit." << endl;

    do {
        press = ts.is_pressed();

        // toggle state
        if (press && !edge) state = !state;
        edge = press;

        switch(state) {
            case 1:  // ACTIVE
                // locate nearest object
                if (uss.distance_centimeters() < d) {
                    d = uss.distance_centimeters();
                    angle = m_uss.position();
                }

                // ultrasonic sensor alternately rotates left and right 
                if (m_uss.position() >= DEG) {
                    m_uss.stop();
                    m_uss.set_duty_cycle_sp(-100);
                } else if (m_uss.position() <= -DEG) {
                    m_uss.stop();
                    m_uss.set_duty_cycle_sp(100);

                    // calculate wheel speed
                    speed = calc_speed(d);

                    // set wheel speeds and turn to nearest object
                    if (angle > 0) {          // turn right
                        turn_speed = calc_turn_speed(speed, d);

                        speed_left = speed;
                        speed_right = turn_speed;
                    } else if (angle < 0) {   // turn left
                        turn_speed = calc_turn_speed(speed, d);

                        speed_left = turn_speed;
                        speed_right = speed;
                    } else {                  // drive straight on
                        speed_left = speed_right = speed;
                    }

                    m_left.set_duty_cycle_sp(speed_left);
                    m_right.set_duty_cycle_sp(speed_right);
    
                    d = D_MAX;
                }

                // activate all motors
                m_left.run_direct();
                m_right.run_direct();
                m_uss.run_direct();

                break;

            case 0:  // STANDBY
                // deactivate all motors
                m_left.stop();
                m_right.stop();
                m_uss.stop();
        }

    // exit by pressing enter
    } while (!button::enter.pressed());
}


int main() {

    drive_to_object();
    return 0;
}