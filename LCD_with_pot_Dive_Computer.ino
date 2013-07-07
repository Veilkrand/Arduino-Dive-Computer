/*
  
  SIMPLE ARDUINO DIVE COMPUTER V1.0
  June 2011 by Alberto Naranjo Galet (http://twitter.com/albertonaranjo) 
  is licensed under a Creative Commons Attribution 3.0 Unported License.
  
  MANUAL:
  - Use dials to setup time and depth when you plan a dive. Calculations are made based on PADI dive tables.
  - ND: No Decompression limit. Maximun recomended diving time.
  - G: Pressure group letter after first diving, useful when planning multiple divings in the same day.
  - Selectable diving time will be adjusted to maximum ND time when higher, and 'max' will be shown.
  
  HARDWARE:
   * 2x POT (Check SLIDERX constant for pin conf)
   * 1x LCD
   * 1x Arduino
  
  LCD PIN SETUP:
   * LCD RS pin to digital pin 12
   * LCD Enable pin to digital pin 11
   * LCD D4 pin to digital pin 5
   * LCD D5 pin to digital pin 4
   * LCD D6 pin to digital pin 3
   * LCD D7 pin to digital pin 2
   * LCD R/W pin to ground
   * 10K resistor:
   * ends to +5V and ground
   * wiper to LCD VO pin (pin 3)
 
 */

// include the library code:
#include <LiquidCrystal.h>

#define SLIDER1  0
#define SLIDER2  1
#define SLIDER3  2

boolean meters_to_feets=false;

double DEPTH_METERS[12]= {10.5, 12, 14, 16, 18, 20, 22, 25, 30, 35, 40, 42}; //12 STATES
int NODECO_TIME[12]= {219, 147, 98, 72, 56, 45, 37, 29, 20, 14, 9, 8}; //12 STATES
int STOP_TIME[12]= {160, 116, 82, 63, 51, 40, 32, 25, 0, 0, 0, 0}; //12 STATES

char GROUP_LETTER[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //26 STATES

int DIVE_TIME[12][26]={

   {10, 20, 26, 30, 34, 37, 41, 45, 50, 54, 59, 64, 70, 75, 82, 88, 95, 104, 112, 122, 133, 145, 160, 178, 199, 219}, //10.5
   {9, 17, 23, 26, 29,32,35,38,42,45,49,53,57,62,66,71,76,82,88,94,101,108,116,125,134,147}, //12
   {8, 15, 19, 22,24,27,29,32,35,37,40,43,47,50,53,57,61,64,68,73,77,82,87,92,98,0}, //14
   {7, 13, 19,21,23,25,27,29,32,34,37,39,42,45,48,50,53,56,60,63,67,70,72,0}, //16
   {6, 11,15,16,18,20,22,24,26,28,30,32,34,36,39,41,43,46,48,51,53,55,56,0}, //18
   {6, 10,13,15,16,18,20,21,23,25,26,28,30,32,33,36,38,40,42,44,45,0}, //20
   {5, 9,12,13,15, 16,18,19,21,22,24,25,27,29,30,32,34,36,37,0}, //22
   {4, 8, 10,11,13,14,15,17,18,19,21,22,23,25,26,28,29,0}, //25
   {3, 6,8,9,10,11,12,13,14,15,16,17,19,20,0}, //30
   {3, 5, 7, 8,8,9,10,11,12,13,14,0}, //35
   {1, 5, 6, 6,7,8,9,0}, //40
   {1, 4, 4, 6,7,8,0}, //42
};

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

}

void loop() {

  boolean security_stop=false;
  boolean max_time=false;
  
  int depth=map(analogRead(SLIDER1),0,1023,1,50);
  int dive_time=map(analogRead(SLIDER2),0,1023,0,220);
  
  int index_depth= getIndexDepth(depth);
  int nodeco=NODECO_TIME[index_depth];
  
  if (dive_time>=STOP_TIME[index_depth]){
    //dive_time=dive_time+5;
    security_stop=true;
  }
  
  if (dive_time>=nodeco){
    dive_time=nodeco;
    security_stop=true;
    max_time=true;
    
  }
  
  int index_group = getIndexGroupTime(index_depth,dive_time);
  char group_letter = GROUP_LETTER[index_group];
   

  if (meters_to_feets){
    //1 Meter = 3.28083989501312 Feet
    depth=depth*3.28083989501312;

  }
 
  
  lcd.setCursor(0, 0);
  lcd.print(depth);
  if (meters_to_feets){
    lcd.print("f ");
  }else{
    lcd.print("m ");
  }
  
  
  lcd.setCursor(4, 0);
  lcd.print("ND:");
  lcd.print(nodeco);
  lcd.print("' ");
  
  lcd.setCursor(11, 0);
  if (security_stop){
    lcd.print("STOP5");
  }else{
    lcd.print("     ");
  }
  
  lcd.setCursor(0, 1);
  
  lcd.print(dive_time);
  lcd.print("'");
  if (max_time){
    lcd.print("max ");
  }else{
    lcd.print("    ");
  }
  
  lcd.setCursor(7, 1);
  lcd.print("G:");
  lcd.print(group_letter);
  // print the number of seconds since reset:
  //lcd.print(millis()/1000);
  
  delay(100);
}

int getIndexDepth(int meters){
  
  int i;
  for (i = 0; i < (sizeof(DEPTH_METERS)/sizeof(double)) - 1; i++){
    
   double top_meters=DEPTH_METERS[i]; 
   double floor_meters;
   if (i>0){
     floor_meters=DEPTH_METERS[i-1];
   }else{
     floor_meters=0;
   }
   
   if (meters>floor_meters && meters<= top_meters){
     return i;
   }
    
  }
  return (sizeof(DEPTH_METERS)/sizeof(double)) - 1;
} 

int getIndexGroupTime(int index, int dtime){
  
  
  int i;
  for (i = 0; i < 25; i++){
    
    int top_divetime=DIVE_TIME[index][i];
    if (top_divetime==0){
      return i-1;
    }
    
    
    int floor_divetime;
    if (i>0){
       floor_divetime=DIVE_TIME[index][i-1];
     }else{
       floor_divetime=0;
     }
     
     if (dtime>floor_divetime && dtime<= top_divetime){
       return i;
     }
    
  }
  
  return 25;
}
