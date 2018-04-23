#include "mbed.h"
#include "XNucleoIKS01A2.h"

/* Instantiate the expansion board */
static XNucleoIKS01A2 *mems_expansion_board = XNucleoIKS01A2::instance(D14, D15, D4, D5);

/* Retrieve the composing elements of the expansion board */
static LSM6DSLSensor *acc_gyro = mems_expansion_board->acc_gyro;

InterruptIn mybutton(USER_BUTTON);
DigitalOut myled(LED1);

volatile int mems_event = 0;
volatile int toggle_free_fall_enable = 0;
static int free_fall_is_enabled = 1;

void pressed_cb() {
  toggle_free_fall_enable = 1;
}

/* Interrupt 1 callback. */
void int1_cb() {
  mems_event = 1;
}



int main() {
  mybutton.fall(&pressed_cb);
  /* Attach callback to LSM6DSL INT1 */
  acc_gyro->attach_int1_irq(&int1_cb);
  
  /* Set timer */
  set_time(1524497278); 
  
  /* Enable LSM6DSL accelerometer */
  acc_gyro->enable_x();
  /* Enable Free Fall Detection. */
  acc_gyro->enable_free_fall_detection();
  
  printf("\r\n--- Starting new run ---\r\n");

  while(1) {
      time_t seconds = time(NULL);
    
    if(toggle_free_fall_enable) {
      toggle_free_fall_enable = 0;
      if(free_fall_is_enabled == 0) {
        acc_gyro->enable_free_fall_detection();
        free_fall_is_enabled = 1;
      } else {
        acc_gyro->disable_free_fall_detection();
        free_fall_is_enabled = 0;
      }
    }
            
    if (mems_event) {
      mems_event = 0;
      LSM6DSL_Event_Status_t status;
      acc_gyro->get_event_status(&status);
      
      if (status.FreeFallStatus) {
        /* Led blinking. */
        myled = 1;
        wait(0.2);
        myled = 0;
        
        printf("Pothole detected! %s \r\n",ctime(&seconds));
      }
    }
  }
}
