#include "clocks.h"

#define PRDIV_VAL			8				/* PLL prescaler */
#define VDIV_VAL			32				/* PLL multiplier */

uint32_t mcg_clk_khz, core_clk_khz, periph_clk_khz;

void clock_init(){

    /*Enable clock on all ports */
    SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | 
                SIM_SCGC5_PORTB_MASK |
                SIM_SCGC5_PORTC_MASK |
                SIM_SCGC5_PORTD_MASK |
                SIM_SCGC5_PORTE_MASK );

    /*Set up system's clock dividers */
    SIM->CLKDIV1 |= (0 |
                    SIM_CLKDIV1_OUTDIV1(0) |
                    SIM_CLKDIV1_OUTDIV2(0) |
                    SIM_CLKDIV1_OUTDIV4(1));
    
    if(PMC->REGSC & PMC_REGSC_ACKISO_MASK){
        PMC->REGSC |= PMC_REGSC_ACKISO_MASK;
    }
    
    uint32_t mcg_clk_hz = pll_init(PRDIV_VAL, VDIV_VAL);

    if(mcg_clk_hz < 0x100){
        while(1);
    }

    mcg_clk_khz = mcg_clk_hz/1000;
    core_clk_khz = mcg_clk_khz/(((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> 28) +1);
    periph_clk_khz = mcg_clk_khz/(((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> 24) +1);

}   

uint32_t pll_init(uint8_t prdiv_val, uint8_t vdiv_val){
    uint8_t frdiv_val;
    uint8_t temp_reg;
    uint8_t prdiv;
    uint8_t vdiv;
    uint16_t i;
    uint32_t ref_freq;
    uint32_t pll_freq;
    uint32_t crystal_val = 16000000;
    uint8_t hgo_val;
    uint8_t erefs_val;

    /* check if in FEI mode */
    if (!((((MCG->S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) == 0x0) && // check CLKS mux has selcted FLL output
    (MCG->S & MCG_S_IREFST_MASK) &&                          // check FLL ref is internal ref clk
    (!(MCG->S & MCG_S_PLLST_MASK)))){                        // check PLLS mux has selected FLL
        return 0x1;                                           // return error code
    }
    /*Clear high-gain flag (Teensy always uses low-power mode) */
    hgo_val = 0;
    /* Removed check of external select; Teensy 3.x always uses external crystal oscillator */
    erefs_val = 1;

    // Check PLL divider settings are within spec.
    if ((prdiv_val < 1) || (prdiv_val > 25)) {return 0x41;}
    if ((vdiv_val < 24) || (vdiv_val > 55)) {return 0x42;}

    // Check PLL reference clock frequency is within spec.
    ref_freq = crystal_val / prdiv_val;
    if ((ref_freq < 2000000) || (ref_freq > 4000000)) {return 0x43;}

    // Check PLL output frequency is within spec.
    pll_freq = (crystal_val / prdiv_val) * vdiv_val;
    if ((pll_freq < 48000000) || (pll_freq > 100000000)) {return 0x45;}

    // configure the MCG_C2 register
    // the RANGE value is determined by the external frequency. Since the RANGE parameter affects the FRDIV divide value
    // it still needs to be set correctly even if the oscillator is not being used
        
    temp_reg = MCG->C2;
    temp_reg &= ~(MCG_C2_RANGE0_MASK | MCG_C2_HGO0_MASK | MCG_C2_EREFS0_MASK); // clear fields before writing new values
    temp_reg |= (MCG_C2_RANGE0(2) | (hgo_val << MCG_C2_HGO0_SHIFT) | (erefs_val << MCG_C2_EREFS0_SHIFT));
    MCG->C2 = temp_reg;
    
    /*
    *  Removed tests around frdiv_val.  The frdiv_val is fixed at 4 because the Teensy
    *  always uses a 16 MHz crystal.
    */
    frdiv_val = 4;

    /*
    *  Select external oscillator and Reference Divider and clear IREFS to start ext osc
    *  If IRCLK is required it must be enabled outside of this driver, existing state
    *  will be maintained.
    *  CLKS=2, FRDIV=frdiv_val, IREFS=0, IRCLKEN=0, IREFSTEN=0
    */
    temp_reg = MCG->C1;
    temp_reg &= ~(MCG_C1_CLKS_MASK | MCG_C1_FRDIV_MASK | MCG_C1_IREFS_MASK); // Clear values in these fields
    temp_reg = MCG_C1_CLKS(2) | MCG_C1_FRDIV(frdiv_val); // Set the required CLKS and FRDIV values
    MCG->C1 = temp_reg;

    /*
    *  if the external oscillator is used need to wait for OSCINIT to set
    */
    for (i = 0 ; i < 10000 ; i++){
        if (MCG->S & MCG_S_OSCINIT0_MASK) break; // jump out early if OSCINIT sets before loop finishes
    }
    if (!(MCG->S & MCG_S_OSCINIT0_MASK)) return 0x23; // check bit is really set and return with error if not set

    /*
    *  Wait for clock status bits to show clock source is ext ref clk
    */
    for (i = 0 ; i < 2000 ; i++){
        if (((MCG->S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) == 0x2) break; // jump out early if CLKST shows EXT CLK slected before loop finishes
    }
    if (((MCG->S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2) return 0x1A; // check EXT CLK is really selected and return with error if not

    /*
    *  Now in FBE
    *  It is recommended that the clock monitor is enabled when using an external clock
    *  as the clock source/reference.
    *  It is enabled here but can be removed if this is not required.
    */
    MCG->C6 |= MCG_C6_CME0_MASK;
   
    /*
    *  Configure PLL
    *  Configure MCG_C5
    *  If the PLL is to run in STOP mode then the PLLSTEN bit needs to be OR'ed
    *  in here or in user code.
    */
    temp_reg = MCG->C5;
    temp_reg &= ~MCG_C5_PRDIV0_MASK;
    temp_reg |= MCG_C5_PRDIV0(prdiv_val - 1);    //set PLL ref divider
    MCG->C5 = temp_reg;

    /*
    *  Configure MCG_C6
    *  The PLLS bit is set to enable the PLL, MCGOUT still sourced from ext ref clk
    *  The loss of lock interrupt can be enabled by seperately OR'ing in the LOLIE bit in MCG_C6
    */
    temp_reg = MCG->C6;					// store present C6 value
    temp_reg &= ~MCG_C6_VDIV0_MASK;		// clear VDIV settings
    temp_reg |= MCG_C6_PLLS_MASK | MCG_C6_VDIV0(vdiv_val - 24); // write new VDIV and enable PLL
    MCG->C6 = temp_reg;					// update MCG_C6

    /*
    *  wait for PLLST status bit to set
    */
    for (uint16_t i = 0 ; i < 2000 ; i++){
        if (MCG->S & MCG_S_PLLST_MASK) break; // jump out early if PLLST sets before loop finishes
    }
    if (!(MCG->S & MCG_S_PLLST_MASK)) return 0x16; // check bit is really set and return with error if not set

    /*
    *  Wait for LOCK bit to set
    */
    for (uint16_t i = 0 ; i < 2000 ; i++){
        if (MCG->S & MCG_S_LOCK0_MASK) break; // jump out early if LOCK sets before loop finishes
    }
    if (!(MCG->S & MCG_S_LOCK0_MASK)) return 0x44; // check bit is really set and return with error if not set

    /*
    *  Use actual PLL settings to calculate PLL frequency
    */
    prdiv = ((MCG->C5 & MCG_C5_PRDIV0_MASK) + 1);
    vdiv = ((MCG->C6 & MCG_C6_VDIV0_MASK) + 24);

    /*
    *  now in PBE
    */
    MCG->C1 &= ~MCG_C1_CLKS_MASK; // clear CLKS to switch CLKS mux to select PLL as MCG_OUT

    /*
    *  Wait for clock status bits to update
    */
    for (i = 0 ; i < 2000 ; i++){
    if (((MCG->S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) == 0x3) break; // jump out early if CLKST = 3 before loop finishes
    }
    if (((MCG->S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3) return 0x1B; // check CLKST is set correctly and return with error if not

    /*
    *  Now in PEE
    */
    return ((crystal_val / prdiv) * vdiv); //MCGOUT equals PLL output frequency // pll_init
}

void wdog_disable(void)
{
	WDOG->UNLOCK = 0xC520;			// Write 0xC520 to the unlock register
	WDOG->UNLOCK = 0xD928;			// Followed by 0xD928 to complete the unlock
	WDOG->STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;	// Clear the WDOGEN bit to disable the watchdog
}
