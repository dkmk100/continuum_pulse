#include <Arduino.h>
#include "pedometer.h"

// ------------------------------------------------------------------------------------
//  Make a new measurement of total acceleration and return the exponentially-weighted
//  average of this measurement and recent measurements.
//  The smoothing acts as a low-pass filter to reduce high frequency noise.
//  The smoothing parameter is alfa:  0 < alfa <= 1.0.  When alfa = 1, no smoothing
//  is applied -- the most recent reading of total acceleration is returned.
//  As alfa is reduced toward zero more smoothing is applied. You will need to
//  experiment to determine a value of alfa that works with the acceleration you
//  are measuring.  A value in the range 0.3 < alfa < 0.6 is a usually first guess.
//
//  Gerald Recktenwald, gerry@pdx.edu,  2022-06-16
float accelSmooth(float alfa, bool printDetail, float ax, float ay, float az) {
  float aTot, aTotSmooth;   //  acceleration components and magnitudes
  static float aTotOld=-99.0;           //  Value retained for the next call to accelSmooth

  // -- Retrieve acceleration components and compute the total
  aTot = sqrt(ax*ax + ay*ay + az*az);         //  could also use pow(ax,2), but ax*ax is faster

  // -- Use exponential smoothing of the total acceleration value (magnitude of acceleration vector).
  //    The "if" statement tests whether this is the first measurement.  aTotOld is initialized
  //    to -99 because for the very first measurement, there is no true "old" value, and
  //    we simply use the current reading.  This test avoids causing the exponential average to
  //    include a bogus intial reading.  Without this test the first few values returned by
  //    this (accelSmooth) function would include a reading of zero, which would cause
  //    the trend in values to start at an artificially low value.
  if ( aTotOld<0.0 ) {
    aTotSmooth = aTot;
  } else {
    aTotSmooth = aTot*alfa + (1-alfa)*aTotOld;
  }
  aTotOld = aTotSmooth;  //  Save for next measurement

  // -- Print acceleration values without extra text so that Serial Plotter can be used
  //    When everything is working, there is no need to print these details
  if ( printDetail ) {
    Serial.print(ax);
    Serial.print("\t");   Serial.print(ay);
    Serial.print("\t");   Serial.print(az);  
    Serial.print("\t");   Serial.print(aTot);
    Serial.print("\t");   Serial.println(aTotSmooth);
  }

  return(aTotSmooth);
}

//basically the same thing as the above function, but with a different aTotOld to track disabling sleep mode instead of a step.
//TERRIBLE way to organize it, these should be pass-by-reference variables, but not time for futher refactoring...
//
// Copied from the original by Gerald Recktenwald, gerry@pdx.edu,  2022-06-16
float awakeSmooth(float alfa, bool printDetail, float ax, float ay, float az) {
  float aTot, aTotSmooth;   //  acceleration components and magnitudes
  static float aTotOld=-99.0;           //  Value retained for the next call to accelSmooth

  aTot = sqrt(ax*ax + ay*ay + az*az);         //  could also use pow(ax,2), but ax*ax is faster

  if ( aTotOld<0.0 ) {
    aTotSmooth = aTot;
  } else {
    aTotSmooth = aTot*alfa + (1-alfa)*aTotOld;
  }
  aTotOld = aTotSmooth;  //  Save for next measurement

  if ( printDetail ) {
    Serial.print(ax);
    Serial.print("\t");   Serial.print(ay);
    Serial.print("\t");   Serial.print(az);  
    Serial.print("\t");   Serial.print(aTot);
    Serial.print("\t");   Serial.println(aTotSmooth);
  }
  return(aTotSmooth);
}