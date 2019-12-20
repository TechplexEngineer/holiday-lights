#ifndef ZCDimmer_h
#define ZCDimmer_h

// This will load the definition for common Particle variable types
#include "Particle.h"

// Depends on library for precise timing
#include "SparkIntervalTimer.h"


class DimmerOutput
{
	public:
		DimmerOutput()
		{
			this->pin = 0;
			this->dim = 0;
		}
		DimmerOutput(int pin, int dim)
		{
			this->pin = pin;
			this->dim = dim;
		}
		// Which pin to output this channel to
		int pin;
		// Dimming level DIM_MIN = on, DIM_MAX = 0ff
		int dim;
		// Boolean to store a "switch" to tell us if we have crossed zero
		bool isOff = false;
};

class ZCDimmer
{
	public:

		/**
		 * Static access method
		 * @return Instance of ZCDimmer
		 */
        static ZCDimmer* getInstance();

		/**
		 * Must be called in setup()
		 */
		void begin(int PIN_ZC_IN, int outputPins[], int numOutputs);

		/**
		 * Change the brightness
		 * @param  value 0=off 255=brightest
		 */
		void setBrightness(int channel, int value);


	private:

		/**
		* Constructor
		*/
		ZCDimmer();


		static void timer_dim();
		static void isr_on_zero_cross();

		int PIN_ZC_IN;
		IntervalTimer timer;

		// Variable to use as a counter volatile as it is in an interrupt
		volatile int counter = 0;

		static const int DIM_MIN = 60;  // on
		static const int DIM_MAX = 115; // off

		// max number of channels we support
		// Photon only has so many pins
		const static int MAX_OUTPUTS = 12;
		// MUST USE D6 or D7 as ZC IN pin
		// const int OUT_PINS = [D0, D1, D2, D3, D4, D5, A0, A1, A2, A3, A4, A5]
		// Dimming level and output pin
		DimmerOutput outputs[MAX_OUTPUTS];
		int numOutputs;

};

#endif //ZCDimmer_h
