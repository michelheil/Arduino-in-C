/**
 * @page TC16_page myTC16.h
 * @section 16-bit Timer/Counter
 * @brief Programm to apply 16-bit Timer/Counter of ATMega328P
 * 
 * @date 10.12.2019 18:41:08
 * @author Michael
 * @copyright GNU General Public License
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MYTC16_H_
#define MYTC16_H_

/**
 * @brief initialize Timer 1A with interrupt and a Clear Timer on Compare Match and a pre-scaler of 1024
 * 
 * @param factor set prescale factor 1, 8, 64, 256, 1024 for Timer/Counter
 * @return float based on the preScaleFactor return the conversion between a seconds and number of ticks
 */
float TC16_init(int factor);


#endif /* MYTC16_H_ */

#ifdef __cplusplus
}
#endif