/*

 Control functions for NeoPixelController.ino
 
 */
//
//Control position to byte interface
//==============================================================
// Converts input from menu position readings into increments 
// or decrements to the input byte, this allows you to control
// brightness, color or many other byte variables

byte incrementer(byte input){
  
  switch (menu_position[1]){
          case 1:
            c+=1;
            if(c >= animation_speed){
              input = input += 1;
              input = constrain(input,1,254);
              c=0;
            };
          break;
          
          case 2:
            input = input;
          break;

          case 3:
            c2+=1;
            if(c2 >= animation_speed){
              input = input -= 1;
              input = constrain(input,1,254);
              c2=0;
            };
          break;
        }
//      Serial.println(input);
//      Serial.println(c);
  return(input);
}

//
// Button Menu Handling (2 button)
//===================================================================================
// Interprets high signals on the set pins as either an increment or decrement to 
// menu position depending on which pin was high, if any. With debounce.

int checkButtons(){
  int p, p2;
  while(digitalRead(BUTTON_A_PIN)== HIGH){
    p++;
  }
  if (p >= 3000){
    level = constrain(level += 1, 0, 3);
  }else if (p >= 1000){
    menu_position[level] = constrain(menu_position[level] += 1, 0, max_level[level]);
  }
  while(digitalRead(BUTTON_B_PIN)== HIGH){
    p2++;
  }
  if (p2 >=3000){
    level = constrain(level -= 1, 0, 3);
  }else if (p2 >= 1000){
    menu_position[level] = constrain(menu_position[level] -= 1, 0, max_level[level]);
  }
  return(level);
}

//
// Gesture Handling - Optional
//=============================================================
// Interprets inturrupts from a gesture sensor as increments or 
// decrements to menu position depending on direction and 
// distance of motion

void gestureSense() {
  if( isr_flag == 1 ) {
     detachInterrupt(0);
     handleGesture();
     isr_flag = 0;
    attachInterrupt(INTURRUPT_PIN, interruptRoutine, FALLING);
  }
}

// Inturrupt Routine
void interruptRoutine() {
  isr_flag = 1;
}

//
byte handleGesture() {
    if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        Serial.println("UP");
        level = level += 1;
        level = constrain(level, 0, 1);
        return(1);
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        level -= 1;
        level = constrain(level, 0, 1);
        return(2);
        break;
      case DIR_LEFT:
        menu_position[level] -= 1;
        menu_position[level] = constrain(menu_position[level], 1, max_level[level]);
        Serial.print(menu_position[0]);
        Serial.println(menu_position[1]);
        return(3);
        break;
      case DIR_RIGHT:
        menu_position[level] += 1;
        menu_position[level] = constrain(menu_position[level], 1, max_level[level]);
        Serial.print(menu_position[0]);
        Serial.println(menu_position[1]);
        return(4);
        break;
      case DIR_NEAR:
        Serial.println("NEAR");
        return(5);
        break;
      case DIR_FAR:
        Serial.println("FAR");
        return(6);
        break;
      default:
        Serial.println("NONE");
        return(7);
    }
  }
}
