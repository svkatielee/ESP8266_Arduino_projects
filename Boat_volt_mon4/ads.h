

#include <Wire.h>
#include <Adafruit_ADS1015.h>
//#include "QuickStats.h"


Adafruit_ADS1115 ads; 


float curr_correction = 10.1F ;  // it was reading Ahour about 10% low so add the 0.1
float multiplier = 0.03125F; //
void setup_ads(){
  // moved to before each call 
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  ads.begin();
}

float filter_cur(float cur){
  if (volt >= 13.0) {
     if (abs(cur) >= 20.0 + abs(last_cur)) return last_cur;
  }
  return cur;
}
void bsort(long A[],int len) {
  unsigned long newn;
  unsigned long n=len;
  long temp=0;
  do {
    newn=1;
    for(int p=1;p<len;p++){
      if(A[p-1]>A[p]){
        temp=A[p];           //swap places in array
        A[p]=A[p-1];
        A[p-1]=temp;
        newn=p;
      } //end if
    } //end for
    n=newn;
  } while(n>1);
}


void loop_ads(){
  // read ADS1115 A1 for battery volts
  ads.setGain(GAIN_TWOTHIRDS);     // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  //ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  adc1 = ads.readADC_SingleEnded(1);
  long v4 = map(adc1,  14600, 21350, 10010, 14600);  // for GAIN_TWOTHIRDS
  //long v4 = map(adc1,  21998, 32126, 10000, 14600);  // for GAIN_ONE
  Serial.print("adc1: "); Serial.println(adc1);
  Serial.print("v4: "); Serial.println((float)v4/1000.0f);
  volt = ((float)v4/1000.0f);
  dtostrf(volt, 2, 3, v_str);
  Serial.print("v_str: "); Serial.println(v_str);

  // read ADS1115 A2&A3 as differential for current shunt, 50mV/Amp
  //ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  multiplier = ads.voltsPerBit()*1000.0F;           // Gets the millivolts per bit 
  
// take median reading
  for (cnt=0; cnt<9; cnt++){
    curr_in[cnt] = ads.readADC_Differential_2_3();
    delay(212);  // a total of 2 second delay/cycle for amphour calc
  }
 
  bsort(curr_in, 9);
  adc2=curr_in[4];
  
  current =  (float)adc2 * -1.0F;
  
  Serial.print("Differential: "); Serial.print(current); 
     Serial.print("("); Serial.print(current * multiplier); Serial.println("mV)");

  // the current appears to be 10 * the actual milivolt reading
  current = current * multiplier * curr_correction;
  //current = filter_cur(current);
  last_cur = current;
  dtostrf(current, 2, 3, c_str);
  ahour += current / (30.0F * 60.0F);  //dt is delta time for amp hour
  if(op_hour >= 16 && ahour >= 0) { ahour = 0; mystate=1; }
  dtostrf(ahour, 2, 3, ah_str);
//  loop_count_avg = (loop_count_avg +loop_count )/2;
//  loop_count = 0;
  ads_dur = millis() - last_ads;
}
