#include <AccelStepper.h>
#include <MultiStepper.h>


/*
 * Motor Control Client
 * 
 * version 0.0.2
 *  
 */

//Step, Dir
AccelStepper wheel_left(1, 31, 30);
AccelStepper wheel_right(1, 28, 27);
AccelStepper box_grab(1, 25, 24);
AccelStepper up_down(1, 22, 21);
AccelStepper box_spin(1, 19, 18);
AccelStepper grabber_rotate(1, 15, 16);

int motor_status[6] = {1,1,1,1,1,1}; //Set to 0 to pause

struct movement {
   signed long steps;
   char direction;
};

const int buffer_size = 30;
struct movement * motor_buffers[6][buffer_size];

void setup() {
   //Start serial communications for command
   Serial.begin(38400);
   //TODO: add enable/disable with wheel_left.setEnablePin

   //Set all motor buffers to null
   for(int j =0; j < 6; j++)
   {
    for(int i = 0; i < buffer_size; i++)
    {
      motor_buffers[j][i] = NULL;      
    }
   }
    
}

void loop() {
  //Read the incoming serial data and parse
  char serial_buffer[10];
  while(Serial.readBytesUntil('#', serial_buffer, 10) != 0)
  {
      //A command has now been read into the buffer
      int motor_id = serial_buffer[0]  - '0';

      //Look what the command is
      switch(serial_buffer[1]){
        case 'f':
        case 'F':
            append_to_buffer(motor_id, 'f', serial_buffer);
            break;
        case 'b':
        case 'B':
            append_to_buffer(motor_id, 'b', serial_buffer);
            break;
        case 'p':
        case 'P':
            //Pause motor
            motor_status[motor_id] = 0;
            break;
        case 'r':
        case 'R':
            //Resume motor
            motor_status[motor_id] = 1;
            break;
        case 'c':
        case 'C':
            clear_motor_buffer(motor_id);
            break;
      }
  }

  //Now move the motors - or if they're done then look at what's next for them in the buffer
  if(motor_status[0] == 1)
  {
    wheel_left.run();
    if(wheel_left.distanceToGo() == 0)
    {
      wheel_left.move(get_buffer_next(0));
    }
  }
  if(motor_status[1] == 1)
  {
    wheel_right.run();
    if(wheel_right.distanceToGo() == 0)
    {
      wheel_right.move(get_buffer_next(1));
    }
  }
  if(motor_status[2] == 1)
  {
    box_grab.run();
    if(box_grab.distanceToGo() == 0)
    {
      box_grab.move(get_buffer_next(2));
    }
  }
  if(motor_status[3] == 1)
  {
    up_down.run();
    if(up_down.distanceToGo() == 0)
    {
      up_down.move(get_buffer_next(3));
    }
  }
  if(motor_status[4] == 1)
  {
    box_spin.run();
    if(box_spin.distanceToGo() == 0)
    {
      box_spin.move(get_buffer_next(4));
    }
  }
  if(motor_status[5] == 1)
  {
    grabber_rotate.run();
    if(grabber_rotate.distanceToGo() == 0)
    {
      grabber_rotate.move(get_buffer_next(5));
    }
  }
}

void append_to_buffer(int motor_id, char direction, char buffer[])
{
  //Read from buffer [2] to [9] until we get something that's not 0,1,2,3,4,5,6,7,8,9
  char numberbuffer[8];
  for(int i = 2; i<10; i++)
  {
    char c = buffer[i];
    //Check it's a number
    if(c >= '0' && c <= '9')
    {
      numberbuffer[i-2] = c;
    }
    else
    {
      //Null terminate it
      numberbuffer[i-2] = '\0';
      break;
    }
  }

  //Convert steps to integer
  long steps = atol(numberbuffer);

  //Create the new struct
  struct movement * new_movement;
  new_movement->direction = direction;
  new_movement->steps = steps;

  //Now add into the buffer where we have a null
  for(int i = 0; i < buffer_size; i++)
  {
      if(motor_buffers[motor_id][i] == NULL)
      {
            motor_buffers[motor_id][i] = new_movement;
            break;  
      }
  }
}

signed long get_buffer_next(int motor_id)
{
    if(motor_buffers[motor_id][0] != NULL)
    {
      //Return what's in the array and shift everything forwards
      signed long steps = (motor_buffers[motor_id][0])->steps;
      if((motor_buffers[motor_id][0])->direction == 'b')
      {
        steps = -1 * steps;
      }
      //Shift it all forward
      for(int i = 1; i<(buffer_size-1); i++)
      {
        if(motor_buffers[motor_id][i] == NULL)
        {
          break;
        }
         motor_buffers[motor_id][i-1] = motor_buffers[motor_id][i];
      }
      return steps;
    }
    return 0;
}

//Clear the buffer for a motor and reset
void clear_motor_buffer(int motor_id)
{
    //Set the array to 0
    for(int i = 0; i < buffer_size; i++)
    {
      motor_buffers[motor_id][i] = NULL;      
    }
    //Stop accelstepper
    switch(motor_id){
      case 0:
        wheel_left.stop();
        break;
      case 1:
        wheel_right.stop();
        break;
      case 2:
        box_grab.stop();
        break;
      case 3:
        up_down.stop();
        break;
      case 4:
        box_spin.stop();
        break;
      case 5:
        grabber_rotate.stop();
        break;
    }
}
