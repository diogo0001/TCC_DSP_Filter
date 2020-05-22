/*
 * crossover.h
 *
 *  Created on: 10 de mar de 2020
 *      Author: kulie
 */

#ifndef CROSSOVER_H_
#define CROSSOVER_H_

#include "filter_def.h"

uint8_t cross_check_f0_variation(filter_instance* SL, filter_instance *SH, uint16_t f0);
uint8_t cross_check_Q_variation(filter_instance* SL, filter_instance *SH, float32_t Q);
uint8_t cross_bind_coef_calc(filter_instance *SL, filter_instance *SH);

#endif /* CROSSOVER_H_ */
