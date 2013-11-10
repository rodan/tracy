#ifndef __CALIB_H__
#define __CALIB_H__

// see calibration equations in adc_calibration.ods (OpenOffice spreadsheet)

// internal temperature voltage
//#define VREF_1_5        1.5
#define T_INT_A         -288.8764
#define T_INT_B         0.6180

// 12V battery
#define VREF_2_5_6_0    2.4343
#define DIV_BAT         0.7083

// photovoltaic cell
#define VREF_2_5_6_1    2.4518
#define DIV_PV          1.0040

// current sense ic
#define VREF_2_5_5_0    2.4991
#define INA168_A        0.0167
#define INA168_B        0.9037

// thermistor
#define VREF_2_5_5_1    2.4983
#define TH_A            94.2421
#define TH_B            -42.7252

#endif
