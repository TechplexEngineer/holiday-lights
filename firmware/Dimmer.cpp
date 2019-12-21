#include "SparkIntervalTimer.h"

#include "Dimmer.h"
#include <algorithm>

// #define debugbb


// This is the delay-per-brightness step in microseconds.
// It is calculated based on the frequency of your voltage supply (50Hz or 60Hz)
// and the number of brightness steps you want.
// Firing angle calculation : 1 full 60Hz wave =1/60=16.7ms
// Every zerocrossing thus: (60Hz)-> 8.3ms (1/2 Cycle) For 60Hz => 8.33ms
// 8.33ms=8333us
// 128 = brightness steps
// (8333us - 10us) / 128 = 65 (Approx)
const int freqStep = 65;


/**
 * Constructor.
 */
ZCDimmer::ZCDimmer()
{
	// be sure not to call anything that requires hardware be initialized here,
	// put those in begin()
}

// Singleton for ease of access in interrupts
ZCDimmer* ZCDimmer::getInstance()
{
	// create the one and only instance
    static ZCDimmer instance;

    return &instance;
}

/**
 * Init the dimmer
 *
 * if numOutputs > MAX_OUTPUTS
 * then any channels > MAX_OUTPUTS are not set
 *
 * outputPins cannot contain PIN_ZC_IN
 */
void ZCDimmer::begin(int PIN_ZC_IN, int outputPins[], int numOutputs)
{
	#ifdef debugbb
	//DEBUG
	pinMode(D3, OUTPUT);
	pinMode(D4, OUTPUT);
	#endif

	// store the input pin
	this->PIN_ZC_IN = PIN_ZC_IN;
	// Set the input pin as an input
	pinMode(this->PIN_ZC_IN, INPUT);


	// store the number of outputs
	this->numOutputs = std::min(numOutputs, this->MAX_OUTPUTS);

	// for each output
	for(int outNum = 0; outNum < this->numOutputs; outNum ++)
	{
		const int pin = outputPins[outNum];
		// set its pin as an output
		pinMode(pin, OUTPUT);
		// store the pin and init output to off
		this->outputs[outNum].pin = pin;
		this->outputs[outNum].dim = DIM_MAX;
	}

	Serial.println("Starting Timer");
	// Setup the timer
	timer.begin(ZCDimmer::timer_dim, freqStep, uSec, TIMER5);
	Serial.println("Starting Interrupt");
	// Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
	attachInterrupt(this->PIN_ZC_IN, ZCDimmer::isr_on_zero_cross, RISING);
}

/**
 * Change the brightness
 * @param  channel index of the channel to control
 * @param  value   0=off 255=brightest
 */
void ZCDimmer::setBrightness(int channel, int value)
{
	// DMX/sACN/E1.31 sends values in the range [0,255]
	// Convert to our known good range
	value = map(value, 0, 255, DIM_MAX, DIM_MIN);
	if(value >= DIM_MAX) {
		this->outputs[channel].dim = DIM_MAX;
		return;
	}
	if(value <= DIM_MIN) {
		this->outputs[channel].dim = DIM_MIN;
		return;
	}
	if (channel >= 0 && channel < this->numOutputs)
	{
		// valid channel
		this->outputs[channel].dim = value;
	}
	else
	{
		Serial.printlnf("Invalid Channel %d", channel);
	}
}

/**
 * Called from the timer.
 * Treat as an ISR, no long running things.
 * If the requested delay (dim) has elapsed turn on the output, otherwise wait
 */
void ZCDimmer::timer_dim()
{
	#ifdef debugbb
		static bool dbgtmr = false;
		dbgtmr = !dbgtmr;
		digitalWrite(D3, dbgtmr);
	#endif

	// increment time step counter
	ZCDimmer::getInstance()->counter++;

	// for each output
	for(int outNum = 0; outNum < ZCDimmer::getInstance()->numOutputs; outNum ++)
	{
		DimmerOutput* output = &ZCDimmer::getInstance()->outputs[outNum];

		// if the output is off and we have waited long enough
		if(output->isOff && ZCDimmer::getInstance()->counter >= output->dim)
		{
			// if we are at DIM_MAX don't turn on at all
			if (output->dim == ZCDimmer::DIM_MAX)
			{
				// don't turn on
			}
			else
			{
				// turn on output
				digitalWrite(output->pin, HIGH);

				// // reset time step counter
				// ZCDimmer::getInstance()->counter = 0;
				//reset zero cross detection
				output->isOff = false;
			}
		}
	}
}

/**
 * Attatched as an interrupt when a zero cross is detected.
 */
void ZCDimmer::isr_on_zero_cross()
{
	#ifdef debugbb
		static bool dbg = false;
		dbg = !dbg;
		digitalWrite(D4, dbg);
	#endif
	// static unsigned long last_interrupt_time = 0;
	// unsigned long interrupt_time = micros();


	// If interrupts come faster than 1ms, assume it's a bounce and ignore
	// if (interrupt_time - last_interrupt_time > 4000)
	// {

		// begin counting the (dim) amount before turning on
		ZCDimmer::getInstance()->counter = 0;

		// turn off output at the zero crossing
		// Which is the only place we can turn off the TRIAC

		// for each output
		for(int outNum = 0; outNum < ZCDimmer::getInstance()->numOutputs; outNum ++)
		{
			DimmerOutput* output = &ZCDimmer::getInstance()->outputs[outNum];
			// turn off at the zero crossing
			digitalWrite(output->pin, LOW);

			// keep track of when we turn off the output, ie: at a ZC
			output->isOff = true;
		}

	// 	last_interrupt_time = interrupt_time;
	// }
}

