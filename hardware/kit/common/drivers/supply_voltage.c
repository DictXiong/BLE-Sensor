/*
 * supply_voltage.c
 *
 *  Created on: 2023Äê8ÔÂ7ÈÕ
 *      Author: me
 */

#include <stddef.h>
#include "i2cspm.h"  // to get uint16_t defined
#include "supply_voltage.h"
#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_iadc.h"


/* ref: https://www.silabs.com/documents/public/example-code/an0018.2-efr32-efm32-supply-voltage-monitoring.zip */

// Set CLK_ADC to 100kHz (this corresponds to a sample rate of 10ksps)
#define CLK_SRC_ADC_FREQ        10000000  // CLK_SRC_ADC
#define CLK_ADC_FREQ            100   // CLK_ADC

// Upper and lower bound for Window Comparator
// 16-bit left-justified format; 12-bit conversion result compared to upper
// 12-bits of window comparator
#define AVDD_LOWER_BOUND 0x7550;	// 2.2V
#define AVDD_UPPER_BOUND 0xCAA0;	// 3.8V

/**************************************************************************//**
 * @brief  IADC Initializer
 *****************************************************************************/
void init_supply_voltage() {
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;

  // Reset IADC to reset configuration in case it has been modified
  IADC_reset(IADC0);

  // Configure IADC clock source to use the FSRCO(EM2/EM3)
  CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);

  // Modify init structs and initialize
  init.warmup = iadcWarmupNormal;

  // Set the clk_src_adc prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  // Set upper bound for window compare
  init.greaterThanEqualThres = AVDD_UPPER_BOUND;

  // Set lower bound for window compare
  init.lessThanEqualThres = AVDD_LOWER_BOUND;

  // Configuration 0 is used by both scan and single conversions by default
  // Use internal 1.2V bandgap as reference
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency for desired sample rate
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(
    IADC0, CLK_ADC_FREQ, 0, iadcCfgModeNormal, init.srcClkPrescale
  );

  // Set conversions to run continuously
  initSingle.triggerAction = iadcTriggerActionOnce;

  // Configure Input sources for single ended conversion
  initSingleInput.posInput = iadcPosInputAvdd;
  initSingleInput.negInput = iadcNegInputGnd;

  // Enable window comparisons on this input
  initSingleInput.compare = true;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize Scan
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);
}

uint16_t get_supply_voltage() {
  uint16_t ans = 0;
  static volatile IADC_Result_t sample;

  IADC_command(IADC0, iadcCmdStartSingle);
  while ((IADC0->STATUS & IADC_STATUS_SINGLEFIFODV) == 0);

  sample = IADC_readSingleResult(IADC0);
  ans = (sample.data)*4*1210/4095;
  IADC_command(IADC0, iadcCmdStopSingle);
  return ans;
}
