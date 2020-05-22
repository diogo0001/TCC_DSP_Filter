/*
 * equalizer.h
 *
 *  Created on: 28 de abr de 2020
 *      Author: Diogo Tavares
 */

#ifndef EQUALIZER_H_
#define EQUALIZER_H_

#include "filter_def.h"

uint8_t eq_check_f0_variation(filter_instance* S, uint16_t f0);
uint8_t eq_check_Q_variation(filter_instance* S, float32_t Q);
uint8_t eq_check_G_variation(filter_instance* S, int8_t G);
uint8_t eq_coef_calc(filter_instance *SL);

#endif /* EQUALIZER_H_ */
