

#include <Wire.h>
#include <Adafruit_ADS1015.h>


Adafruit_ADS1115 ads; 


float curr_correction = 10.1F ;  // it was reading Ahour about 10% low so add the 0.1
float multiplier = 0.03125F; 
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

void loop_ads(){
  // read ADS1115 A1 for battery volts
  ads.setGain(GAIN_TWOTHIRDS);     // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  //ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  adc1 = ads.readADC_SingleEnded(1);
  long v4 = map(adc1,  20667, 26298, 11010, 14010);  // for GAIN_TWOTHIRDS
  //long v4 = map(adc1,  21998, 32126, 10000, 14600);  // for GAIN_ONE
  Serial.print("adc1: "); Serial.println(adc1);
  Serial.print("v4: "); Serial.println((float)v4/1000.0f);
  float_t v3 = ((float)v4/1000.0f);
  dtostrf(v3, 2, 3, v_str);
  Serial.print("v_str: "); Serial.println(v_str);

  // read ADS1115 A2&A3 as differential for current shunt, 50mV/Amp
  //ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  multiplier = ads.voltsPerBit()*1000.0F;           // Gets the millivolts per bit 
  adc2 = ads.readADC_Differential_2_3();
// I tried to average current...got random readings periodically  
//    curr_in[cnt] = adc2;
//    cnt += 1;
//    if (cnt >= 5) {cnt = 0;};
//    current =  (float)( curr_in[0] + curr_in[1] + curr_in[2] + curr_in[3] + curr_in[4])/5.0F; 
  current =  (float)adc2 * -1.0F;
  
  Serial.print("Differential: "); Serial.print(current); 
     Serial.print("("); Serial.print(current * multiplier); Serial.println("mV)");
  dtostrf(current, 2, 3, c3);
  dtostrf((current * multiplier), 2, 3, c4);
  // the current appears to be 10 * the actual milivolt reading
  current = current * multiplier * curr_correction;
  dtostrf(current, 2, 3, c_str);
  ahour += current / (60.0F * 60.0F);  //dt is delta time for amp hour
  if(op_hour >= 16 && ahour >= 0) { ahour = 0; mystate=1; }
  dtostrf(ahour, 2, 3, ah_str);
  loop_count_avg = (loop_count_avg +loop_count )/2;
  loop_count = 0;
}
