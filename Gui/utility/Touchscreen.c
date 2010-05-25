/****************************************************************************
*
* Atmel Corporation
*
* File              : Touchscreen.c
* Compiler          : IAR EWAAVR 4.20A
* Revision          : $Revision: 0.99 $
* Date              : $Date: Thursday, September 20, 2006 $
* Updated by        : $Author: Jens Rotter $
*
* Support mail      : avr@atmel.com
*
* Supported devices : ATmega88
*
* AppNote           : ?
* 
* Description       : Touchscreen Controller 4wire (Lowlevel/Rtouch Measurement) and 5wire (Lowlevel Measurement)
*                     
*
*
****************************************************************************/


/****************************************************************************
  Libraries
****************************************************************************/

#include "Touchscreen.h"

/****************************************************************************
  Global variable
****************************************************************************/

// Interface variable
Interface Touchscreen_Data;

// Counter to Set Sleep Flag
volatile unsigned int Counter_Sleep;
// Counter to Set End Flag
static unsigned char Counter_Untouch_Condition;

// Manages Measurements
volatile unsigned char ADC_ISR_Switch = 1;

// Array for X/Y-Coordinates
static short int Readings_X_Pos[SAMPLES_FOR_ONE_TRUE_XY_PAIR];
static short int Readings_Y_Pos[SAMPLES_FOR_ONE_TRUE_XY_PAIR]; 
static unsigned char i_array;

/****************************************************************************
  Interrupts
****************************************************************************/

#pragma vector = PIN_CHANGE_VECTOR
//__interrupt void Pin_Change_ISR(void)
ISR(PIN_CHANGE_VECTOR)
{
	// Disable Sleep
	//SMCR = (0 << SE);
	
	// Disable Pin Change Interrupt
	ClearBit(PIN_CHANGE_INTERRUPT_CONTROL_REGISTER, PIN_CHANGE_INTERRUPT_ENABLE_1);
	
	// Start Timer Clock (CTC Mode)
	TIMER0_CONTROL_REGISTER_B = (TIMER0_PRESCALER << TIMER0_CLOCK_SELECT);
	
	// Reset Sleep Counter (--> Debounce)
	Counter_Sleep = 0;
	
	//++++++++++++++++++++++++++++++++++
	//SetBit(PORTA, 4);
	//++++++++++++++++++++++++++++++++++
}

#pragma vector = TIMER0_COMPARE_MATCH_A_VECTOR 
//__interrupt void Timer0_Compare_Match_A_ISR(void)
ISR(TIMER0_COMPARE_MATCH_A_VECTOR)
{
	static unsigned char Counter_Debounce = 0;
	
	if (TestBit(ADC_CONTROL_AND_STATUS_REGISTER_A, ADC_ENABLE))
	{
		//++++++++++++++++++++++++++++++++++
		//SetBit(PORTA, 5);
		//++++++++++++++++++++++++++++++++++
		
	  // Measurement in Progress -> Timer0_Compare_Match_A_ISR serves to trigger ADC
		
		// Start ADC Conversion
		START_CONVERSION;
	}
	else
	{
		if (!TestBit(PIN_CHANGE_INTERRUPT_CONTROL_REGISTER, PIN_CHANGE_INTERRUPT_ENABLE_1))
		{
		  // Only Activated After Pin Change Interrupt Was Cleared --> Debounce
			
			if (TestBit(TOUCHSCREEN_INPUT, STANDBY_PIN))
			{
				// Reset Counter_Debounce on High Level at YN
				Counter_Debounce = 0;                  
			}
			else
			{
				// Increment Counter_Debounce on Low Level at YN	
				Counter_Debounce++;    
			}
			
			if (Counter_Debounce == DEBOUNCE_TIME) 
			{
				// Reset Counter_Debounce
				Counter_Debounce = 0;
				
				Start_Measurement();
			}
		}
		
		// Increment Counter_Sleep
		Counter_Sleep++;
		if (Counter_Sleep == SLEEP_COUNTDOWN)
		{
		  // Sleep Condition Reached
			
			// Reset Counter_Debounce
			Counter_Debounce = 0;
			
			// Stop Timer Clock (CTC Mode)
			TIMER0_CONTROL_REGISTER_B = (0 << TIMER0_CLOCK_SELECT); 
			
			Stop_Measurement();
			
			// Interface: New Event --> Sleep
			SetBit(Touchscreen_Data.Flag_Register, FLAG_REGISTER_SLEEP);
			
			//++++++++++++++++++++++++++++++++++
			//SetBit(PORTA, FLAG_REGISTER_SLEEP);
			//++++++++++++++++++++++++++++++++++
		}
	}
}

#pragma vector = TIMER0_COMPARE_MATCH_B_VECTOR 
//__interrupt void Timer0_Compare_Match_B_ISR(void)
ISR(TIMER0_COMPARE_MATCH_B_VECTOR)
{
	// Set I/O directly before Measurements starts
	
	if (TestBit(ADC_ISR_Switch,0)) 
	{	
		// I/O State: Check for Touch
		STANDBY_CONFIGURATION;
	}
	else if (TestBit(ADC_ISR_Switch,1))
	{	
		// I/O State: Set Up X-Coordinate Measurement
		X_POS_CONFIGURATION;
	}
	else if (TestBit(ADC_ISR_Switch,2))
	{	                
		// I/O State: Set Up Y-Coordinate Measurement
		Y_POS_CONFIGURATION;
	}      
#if defined R_TOUCH_MEASUREMENT
	else if (TestBit(ADC_ISR_Switch,3))
	{	                
		// I/O State: Set Up Z1 Measurement
		Z_1_AND_2_CONFIGURATION;
	}        
	else if (TestBit(ADC_ISR_Switch,4))
	{	                
		// I/O State: Set Up Z2 Measurement
		Z_1_AND_2_CONFIGURATION;
	}        
#endif
}

#pragma vector = ADC_CONVERSION_COMPLETE_VECTOR
//__interrupt void ADC_Conversion_Complete_ISR(void)
ISR(ADC_CONVERSION_COMPLETE_VECTOR)
{
	ADC_Measurement();
} 

/****************************************************************************
  Function declarations
****************************************************************************/
void Touchscreen_Init(void)
{
	// Reset registers set by wiring.h delay/millis/micros wont't work
	TIMER0_CONTROL_REGISTER_A = 0;
	TIMER0_CONTROL_REGISTER_B = 0;
	TIMER0_INTERRUPT_MASK_REGISTER = 0;
	ADC_CONTROL_AND_STATUS_REGISTER_A = 0;
	
	// I/O State: Check for Touch
	STANDBY_CONFIGURATION;
	
	// Pin Change
	// Set Pin Change Interrupt Source
	SetBit(PIN_CHANGE_MASK_1_REGISTER, PIN_CHANGE_ENABLE_MASK);
	// Enable Pin Change Interrupt
	SetBit(PIN_CHANGE_INTERRUPT_CONTROL_REGISTER, PIN_CHANGE_INTERRUPT_ENABLE_1); 
	
	
	// Timer0 Initialization (CTC Mode)
	// Set Clear Timer on Compare Match A
	TIMER0_CONTROL_REGISTER_A = (2 << TIMER0_WAVEFORM_GENERATION_MODE);
	// Set Output Compare Register A to a Defined Value
	TIMER0_OUTPUT_COMPARE_REGISTER_A = TIMER0_INITIAL_VALUE_COMPARE_MATCH_A;
	// Set Output Compare Register B to a Defined Value
	TIMER0_OUTPUT_COMPARE_REGISTER_B = TIMER0_INITIAL_VALUE_COMPARE_MATCH_B;
	// Enable Timer0 Compare Match A Interrupt
	TIMER0_INTERRUPT_MASK_REGISTER = (1 << TIMER0_OUTPUT_COMPARE_A_INT_ENABLE);
	// Start Timer Clock (CTC Mode)
	TIMER0_CONTROL_REGISTER_B = (TIMER0_PRESCALER << TIMER0_CLOCK_SELECT);
	
	// Interface: New Event --> Reset Condition
	SetBit(Touchscreen_Data.Flag_Register, FLAG_REGISTER_START);
	
	//++++++++++++++++++++++++++++++++++
	//SetBit(PORTA, FLAG_REGISTER_START);
	//++++++++++++++++++++++++++++++++++
}

void Start_Measurement(void)
{
	// Reset Sleep Counter
	Counter_Sleep = 0;
	
	// Interface: New Event --> Touch detected
	SetBit(Touchscreen_Data.Flag_Register, FLAG_REGISTER_TOUCH);
	
	//++++++++++++++++++++++++++++++++++
	//SetBit(PORTC, (FLAG_REGISTER_TOUCH + 4));
	//++++++++++++++++++++++++++++++++++
	
	// ADC Initialization (Single-Ended)
	// Enable / Interrupt / Prescaler
	ADC_CONTROL_AND_STATUS_REGISTER_A  = (1 << ADC_ENABLE) | (1 << ADC_INTERRUPT_ENABLE) | (ADC_PRESCALER << ADC_PRESCALER_SELECT);
	
	// Set ADC Input Channel for Check for Touch
	ADC_MULTIPLEXER_SELECTION_REGISTER = STANDBY_PIN | (1 << ADC_REFERENCE_SELECTION);
	
	// Clear Timer0 Compare Match B Interrupt Flag
	SetBit(TIMER0_INTERRUPT_FLAG_REGISTER, TIMER0_OUTPUT_COMPARE_B_MATCH_FLAG);
	// Enable Timer0 Compare Match B Interrupt
	SetBit(TIMER0_INTERRUPT_MASK_REGISTER, TIMER0_OUTPUT_COMPARE_B_INT_ENABLE);
}

void Stop_Measurement(void)
{
	// Reset Variables
	Counter_Untouch_Condition = 0;
	i_array = 0;
	ADC_ISR_Switch=1;
	
	// I/O State: Check for Touch
	STANDBY_CONFIGURATION;
	
	// Diable ADC
	ADC_CONTROL_AND_STATUS_REGISTER_A = (0 << ADC_ENABLE);
	
	// Disable Timer0 Compare Match B Interrupt
	ClearBit(TIMER0_INTERRUPT_MASK_REGISTER, TIMER0_OUTPUT_COMPARE_B_INT_ENABLE);
	
	// Clear Pin Change Interrupt Flag
	SetBit(PIN_CHANGE_INTERRUPT_FLAG_REGISTER, PIN_CHANGE_INTERRUPT_FLAG_1);
	// Enable Pin Change Interrupt
	SetBit(PIN_CHANGE_INTERRUPT_CONTROL_REGISTER, PIN_CHANGE_INTERRUPT_ENABLE_1);
}

void Insertion_Sort(short int Array[])
{
	signed char i;
	unsigned char j;
	short int current;
	
	// Sort Array[]
	for (j = 1; j < SAMPLES_FOR_ONE_TRUE_XY_PAIR; j++) 
	{
		current = Array[j]; 
		i = j - 1;
		while (i >= 0 && Array[i] > current) 
		{
			Array[i + 1] = Array[i];
			i--;
		}
		Array[i + 1] = current;
	}
}

void Store_valid_Data(void)
{
	// Store valid Data in Array
	
	if (i_array == (END_OF_ARRAY))
	{
	  // Array Management
		
		// Sort X/Y Readings To Get Median
		Insertion_Sort(Readings_X_Pos);
		Insertion_Sort(Readings_Y_Pos);
		
		// Interface: Check If Old Coordinates Were Read, Otherwise Set Overflow Flag
		Check_Overflow(FLAG_REGISTER_COORDINATES);
		
		// Interface: Make New Coordinates Available
		Touchscreen_Data.x_pos = Readings_X_Pos[MEDIAN];
		Touchscreen_Data.y_pos = Readings_Y_Pos[MEDIAN];  
		
		// Interface: New Event --> New Coordinates Ready
		SetBit(Touchscreen_Data.Flag_Register, FLAG_REGISTER_COORDINATES);
		
		//++++++++++++++++++++++++++++++++++
		//SetBit(PORTC, (FLAG_REGISTER_COORDINATES + 4));
		//++++++++++++++++++++++++++++++++++
		
		// Reset i_array
		i_array=0;
	}
	else
	{
		// Increment i_array
		i_array++;
	}
	
	// Reset Counter_Untouch_Condition  
	Counter_Untouch_Condition = 0;
}

#if defined LOW_LEVEL_MEASUREMENT
void ADC_Measurement(void)
{
	static short int Reading_Low_Level;
	
	// Stop Power Supply for Touchscreen
	HI_Z_CONFIGURATION;
	
	if (TestBit(ADC_ISR_Switch,0)) 
	{	
		// Available: Value for Reading_Low_Level 
		Reading_Low_Level = ADC;
		
		if (Reading_Low_Level < MAXIMUM_LOW_LEVEL)
		{
		  // Reading_Low_Level = TRUE
			
			if (TestBit(ADC_ISR_Switch,7))
			{
				// ADC_ISR_Switch, Bit 7 = Flag for valid X/Y Coordinates available
				
				Store_valid_Data();
			}
			
			// Set ADC_ISR_Switch
			// Reset Reading_Low_Level
			ClearBit(ADC_ISR_Switch,0);
			// Next: X-Coordinate Measurement (I/O Set up in Timer0_Compare_Match_B_ISR)
			SetBit(ADC_ISR_Switch,1);
			// Flag for Reading_Low_Level = TRUE
			SetBit(ADC_ISR_Switch,7);
		}
		else
		{
		  // Reading_Low_Level = FALSE
			
			// Increment Counter_Untouch_Condition
			Counter_Untouch_Condition++;
			
			if(Counter_Untouch_Condition == MAXIMUM_UNTOUCH_CONDITIONS)
			{
				// Terminating Condition
				Stop_Measurement();  
				
				// Interface: New Event --> End of Touch detected
				SetBit(Touchscreen_Data.Flag_Register, FLAG_REGISTER_END);
				
				//++++++++++++++++++++++++++++++++++
				//SetBit(PORTC, (FLAG_REGISTER_END + 4));
				//++++++++++++++++++++++++++++++++++
			}
			
			// Set ADC_ISR_Switch (Next: Reading_Low_Level)
			// Reset Condition "valid X/Y Coordinates available"
			ClearBit(ADC_ISR_Switch,7);
		}
	}
	else if (TestBit(ADC_ISR_Switch,1))
	{
		// Available: Value for X-Coordinate
		Readings_X_Pos[i_array] = ADC;
		
		// Set ADC Input Channel for Y-Coordinate Measurement
		ADC_MULTIPLEXER_SELECTION_REGISTER = ADC1 | (1 << ADC_REFERENCE_SELECTION);
		
		// Set ADC_ISR_Switch
		// Reset X-Coordinate Measurement
		ClearBit(ADC_ISR_Switch,1);
		// Next: Y-Coordinate Measurement (I/O Set up in Timer0_Compare_Match_B_ISR)
		SetBit(ADC_ISR_Switch,2);
	}
	else if (TestBit(ADC_ISR_Switch,2))
	{
		// Available: Value for Y-Coordinate		
		Readings_Y_Pos[i_array] = ADC;
		
		// Set ADC Input Channel for Check for Touch
		ADC_MULTIPLEXER_SELECTION_REGISTER = STANDBY_PIN | (1 << ADC_REFERENCE_SELECTION);
		
		// Set ADC_ISR_Switch
		// Reset Y-Coordinate Measurement
		ClearBit(ADC_ISR_Switch,2);
		// Next: Reading_Low_Level (I/O Set up in Timer0_Compare_Match_B_ISR)
		SetBit(ADC_ISR_Switch,0);
	}
}
#endif


#if defined R_TOUCH_MEASUREMENT
void ADC_Measurement(void)
{
	static short int      Reading_Low_Level;
	static short int      Reading_Z1;
	static short int      Reading_Z2;
	static long int       R_Touch;
	
	// Stop Power Supply for Touchscreen
	HI_Z_CONFIGURATION;
	
	if (TestBit(ADC_ISR_Switch,0)) 
	{	
		// Available: Value for Reading_Low_Level
		Reading_Low_Level = ADC;
		
		if(Reading_Low_Level < MAXIMUM_LOW_LEVEL)
		{
		  // Reading_Low_Level = TRUE
			
			// Set ADC_ISR_Switch
			// Reset Reading_Low_Level
			ClearBit(ADC_ISR_Switch,0);
			// Next: X-Coordinate Measurement (I/O Set up in Timer0_Compare_Match_B_ISR)
			SetBit(ADC_ISR_Switch,1);
		}
		else
		{
		  // Reading_Low_Level = FALSE
			
			// Increment Counter_Untouch_Condition
			Counter_Untouch_Condition++;
			
			if(Counter_Untouch_Condition == MAXIMUM_UNTOUCH_CONDITIONS)
			{
				// Terminating Condition
				Stop_Measurement();  
				
				// Interface: New Event --> End of Touch detected
				SetBit(Touchscreen_Data.Flag_Register, FLAG_REGISTER_END);
				
				//++++++++++++++++++++++++++++++++++
				//SetBit(PORTC, (FLAG_REGISTER_END + 4));
				//++++++++++++++++++++++++++++++++++
			}
			
			// Set ADC_ISR_Switch (Next: Reading_Low_Level)
			SetBit(ADC_ISR_Switch,0);
		}
	}
	else if (TestBit(ADC_ISR_Switch,1)) 
	{	
		// Available: Value for X-Coordinate	
		Readings_X_Pos[i_array] = ADC;
		
		// Set ADC Input Channel for Y-Coordinate Measurement
		ADC_MULTIPLEXER_SELECTION_REGISTER = ADC1 | (1 << ADC_REFERENCE_SELECTION);
		
		// Set ADC_ISR_Switch
		// Reset X-Coordinate Measurement
		ClearBit(ADC_ISR_Switch,1);
		// Next: Y-Coordinate Measurement (I/O Set up in Timer0_Compare_Match_B_ISR)
		SetBit(ADC_ISR_Switch,2);
	}
	else if (TestBit(ADC_ISR_Switch,2))
	{	    
		// Available: Value for Y-Coordinate		
		Readings_Y_Pos[i_array] = ADC;
		
		// Set ADC_ISR_Switch
		// Reset Y-Coordinate Measurement
		ClearBit(ADC_ISR_Switch,2);
		// Next: Z1 - Measurement (I/O Set up in Timer0_Compare_Match_B_ISR)
		SetBit(ADC_ISR_Switch,3);
	} 
	else if (TestBit(ADC_ISR_Switch,3))
	{	    
		// Available: Value for Z1-Coordinate		
		Reading_Z1 = ADC;
		
		// Set ADC Input Channel for Z2 Measurement
		ADC_MULTIPLEXER_SELECTION_REGISTER = ADC2 | (1 << ADC_REFERENCE_SELECTION);
		
		// Set ADC_ISR_Switch
		// Reset Z1 Measurement
		ClearBit(ADC_ISR_Switch,3);
		// Next: Z2 - Measurement (I/O Set up in Timer0_Compare_Match_B_ISR)
		SetBit(ADC_ISR_Switch,4);
	}         
	else if (TestBit(ADC_ISR_Switch,4))
	{	    
		// Available: Value for Z2-Coordinate		
		Reading_Z2 = ADC;
		
		// Calculate Appraisal for R_Touch
		R_Touch = ((float)Reading_Z2/Reading_Z1-1) * Readings_X_Pos[i_array];
		
		if(R_Touch<=MAXIMUM_RTOUCH_LEVEL)                   
		{   
			// Store valid Data in Array
			Store_valid_Data();          
		}
		
		// Set ADC Input Channel for Check for Touch
		ADC_MULTIPLEXER_SELECTION_REGISTER = STANDBY_PIN | (1 << ADC_REFERENCE_SELECTION);
		
		// Set ADC_ISR_Switch
		// Reset Z2 - Measurement
		ClearBit(ADC_ISR_Switch,4);
		// Next: Reading_Low_Level (I/O Set up in Timer0_Compare_Match_B_ISR)
		SetBit(ADC_ISR_Switch,0);
	}         
}
#endif
