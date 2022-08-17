#ifndef REGISTER_MAP
#define REGISTER_MAP

////////////////////
// Analizer
#define ANALIZER_COEFF                  0x3FC80000

////////////////////
// Encoder
#define ENCODER_COEFF                   0x3FC80010

////////////////////
// Decoder
#define DECODER_COEFF                   0x3FC80020

////////////////////
// Synthesis
#define SYNTHESIS_COEFF                 0x3FC80030

////////////////////
// Filter Audio
#define FILTER_AUDIO_GAIN               0x3FC80040
#define FILTER_AUDIO_CUTOFF_FREQUENCY   0x3FC80050

////////////////////
// ADC
#define ADC_SAMPLE_FREQUENCY            0x3FC80060

////////////////////
// Filter Audio
#define FILTER_CHANNEL_GAIN             0x3FC80070
#define FILTER_CHANNEL_CUTOFF_FREQUENCY 0x3FC80080

////////////////////
// Sampler
#define SAMPLER_THRESHOLD               0x3FC80090

////////////////////
// Carrier
#define CARRIER_FREQUENCY               0x3FC800A0

////////////////////
// Carrier
#define CHANNEL_COEFF                   0x3FC800B0

#endif
