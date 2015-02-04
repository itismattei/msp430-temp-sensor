#include <msp430.h> 

/// ADC init with INTeRnAL REFERENCE
void initADC(unsigned char channel){
	P6SEL |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6;
	/// set reference register for internal reference: enable, 1.5V
	REFCTL0 = REFMSTR + REFVSEL_0 + REFON;
	/// disable the AD unit
	ADC12CTL0 = 0;
	/// set the AD to have internal ck, not divided, with single channel operation
	ADC12CTL1 =  ADC12SHP;
	/// 12 bit conversion
	ADC12CTL2 = ADC12RES_2;
	if (channel < 12){
		if (channel == 10){
		/// use of internal temperature sensor. It requires 100us sampling time, with a clock of 5MHz:
		/// 100 us * 5MHz = 500, => ADC12SHT0x = 1010b: 512 clock sample time
			ADC12CTL0 |= ADC12SHT0_10 + ADC12ON;
		/// Vref and channel 10 (Temperature sensor)
			ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;

		}
		else{
			ADC12CTL0 |= ADC12SHT0_3 + ADC12ON;
			ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_5;
		}
		__delay_cycles(300);                      // 35us delay to allow Ref to settle
		                                          // based on default DCO frequency.
		                                          // See Datasheet for typical settle
		                                          // time.
		ADC12IE = ADC12IE0;                       // Enable ADC12IFG.0
		/// Enable of conversion
		ADC12CTL0 |= ADC12ENC;

	}

}


/// ADC init with INTeRnAL REFERENCE
void initADCmultiCH(unsigned char channel, unsigned char beg, unsigned char end){

	/// init port P6.0 ... P6.5
	P6SEL |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5;
	/// set reference register for internal reference: enable, 2.5V
	REFCTL0 = REFMSTR + REFVSEL_2 + REFON;
	/// disable the AD unit
	ADC12CTL0 = 0;
	/// set the AD to have internal ck, not divided, with sequence of channel operation
	/// sequence start at ADC12MEM0 memory cell.
	ADC12CTL1 =  ADC12SHP + ADC12CONSEQ_1;
	//ADC12CTL1 =  ADC12SHP;

	/// 12 bit conversion
	ADC12CTL2 = ADC12RES_2;

	/// time of conversion: 6us * 6 channels + 6 * 13 clk cycles (1 clk = 0,2us) = 36 + 6 * 26 = 36 + 156 = 192 us
	ADC12CTL0 |= ADC12SHT0_3 + ADC12ON;			/// sample time = 32 clk cycles. At 5MHz => 0.2u * 32 = 6us
	ADC12CTL0 |= ADC12MSC;						/// enable multiple conversion

	/// set the last channel in the sequence
	ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_0;
	ADC12MCTL1 = ADC12SREF_1 + ADC12INCH_1;
	ADC12MCTL2 = ADC12SREF_1 + ADC12INCH_2;
	ADC12MCTL3 = ADC12SREF_1 + ADC12INCH_3;
	ADC12MCTL4 = ADC12SREF_1 + ADC12INCH_4;

	/// set the last channel in the sequence
	ADC12MCTL5 = ADC12SREF_1 + ADC12INCH_5 	+ ADC12EOS;

	__delay_cycles(300);                      	// 35us delay to allow Ref to settle
		                                        // based on default DCO frequency.

	/// Enable of conversion
	ADC12CTL0 |= ADC12ENC;

	ADC12IE = ADC12IE5;                           // Enable ADC12IFG.5
	///while (ADC12CTL1 & ADC12BUSY);
	///lettura = ADC12MEM0;

}



/*
 * main.c
 */
int main(void) {
    volatile int i, j;
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    initADCmultiCH(6, 0, 5);
    P4DIR |= BIT1;
    P4OUT = 0;
    //initADC(1);
    // Enable interrupt
    __bis_SR_register(GIE);
    ADC12CTL0 |= ADC12SC;
    while(1){
		for (i = 0; i < 5; i++)
			for (j= 0; j < 30000; j++);
			/// legge i sensori ogni 100 ms

		P4OUT ^= BIT1;
		ADC12CTL0 |= ADC12SC;

		//for(i = 0; i < 1000; i++);
		//while (ADC12CTL1 & ADC12BUSY);
    }
	return 0;

}

volatile int pippo, adcResults[6];
#pragma vector = ADC12_VECTOR
__interrupt void ADC12ISR (void){


	switch(__even_in_range(ADC12IV,34)){
		case  0: break;                           	// Vector  0:  No interrupt
		case  2: break;                           	// Vector  2:  ADC overflow
		case  4: break;                          	// Vector  4:  ADC timing overflow
		case  6:
			pippo = ADC12MEM0;
		break;                           			// Vector  6:  ADC12IFG0
		case  8:
			pippo = ADC12MEM1;
		break;										// Vector  8:  ADC12IFG1
		case 10: break;                           	// Vector 10:  ADC12IFG2
		case 12: break;                           	// Vector 12:  ADC12IFG3
		case 14: break;                           	// Vector 14:  ADC12IFG4
		case 16:									// Vector 16:  ADC12IFG5

			adcResults[0] = ADC12MEM0;
			adcResults[1] = ADC12MEM1;
			adcResults[2] = ADC12MEM2;
			adcResults[3] = ADC12MEM3;
			adcResults[4] = ADC12MEM4;
			adcResults[5] = ADC12MEM5;
			/// segnala la necessita' di aggiornare la media
			/*lettDistanzaPronta = TRUE;*/
			P4OUT ^= BIT1;


		break;
		case 18: break;                           // Vector 18:  ADC12IFG6
		case 20: break;                           // Vector 20:  ADC12IFG7
		case 22: break;                           // Vector 22:  ADC12IFG8
		case 24: break;                           // Vector 24:  ADC12IFG9
		case 26: break;                           // Vector 26:  ADC12IFG10
		case 28: break;                           // Vector 28:  ADC12IFG11
		case 30: break;                           // Vector 30:  ADC12IFG12
		case 32: break;                           // Vector 32:  ADC12IFG13
		case 34: break;                           // Vector 34:  ADC12IFG14
		default: break;
	 }
}


