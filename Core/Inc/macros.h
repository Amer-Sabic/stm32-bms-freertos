/*
 * macros.h
 *
 *  Created on: 27. okt 2026.
 *      Author: Korisnik
 */

#ifndef INC_MACROS_H_
#define INC_MACROS_H_




#define GATE_CTRL_PIN			GPIO_PIN_12

#define MUX1_SEL_PIN_A			GPIO_PIN_2
#define MUX1_SEL_PIN_B			GPIO_PIN_1
#define MUX1_SEL_PIN_C			GPIO_PIN_0

#define MUX2_SEL_PIN_A			GPIO_PIN_4
#define MUX2_SEL_PIN_B			GPIO_PIN_3
#define MUX2_SEL_PIN_C			GPIO_PIN_2

#define CURR_MEASUREMENT_PIN_P  GPIO_PIN_7
#define CURR_MEASUREMENT_PIN_N  GPIO_PIN_5

#define ADC_PAIRS 8
#define ADC_BUF_LEN (2*ADC_PAIRS)



#define CELL_OV_V   4.25f
#define CELL_UV_V   3.00f
#define TEMP_MAX_C  50.0f
#define CURR_MAX_A  20.0f

#define FAULT_OC            (1U << 0)
#define FAULT_CELL_OV       (1U << 1)
#define FAULT_CELL_UV       (1U << 2)
#define FAULT_OT            (1U << 3)
#define FAULT_MEAS_ERR      (1U << 4)
#define FAULT_MEAS_TIMEOUT  (1U << 5)


#define FRAME_START   0xAA
#define TYPE_TELEM    0x01
#define CMD_ENABLE    0x10
#define CMD_DISABLE   0x11
#define CMD_RESET     0x12

#define RX_BUF_LEN    64
#define AWD_HIGH 2600
#define AWD_LOW 1000
#define SUP_PERIOD 200

#define ACQ_BIT  (1U << 0)
#define PROT_BIT (1U << 1)
#define COMM_BIT (1U << 2)
#define ALL_BIT (ACQ_BIT | PROT_BIT | COMM_BIT)
#endif /* INC_MACROS_H_ */
