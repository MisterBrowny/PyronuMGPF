#ifndef FEEDBACK_H
#define	FEEDBACK_H

// Indication de mode en cours
// ARMED	: 1 pulse
// GO		: 2 pulses
// STOP		: 3 pulses
// END		: 4 pulses

//#define INFO_OUT	PORTCbits.RC4	// Info de feedback

#define FEEDBACK_PULSE			50		// Durée d'un pulse		: 50ms
#define FEEDBACK_INTER_PULSE	50		// Durée inter-pulse	: 50ms
#define FEEDBACK_PERIOD			500		// Période du feedback	: 500ms // Période > (4 * FEEDBACK_PULSE + 4 * FEEDBACK_INTER_PULSE)

typedef struct StructFeedback {
	uint16_t	Period;
	uint8_t		Pulse;
	uint8_t		State;	// ARMED / GO / STOP / END
	uint8_t		Step;
}struFeedback;

//struFeedback Feedback;

#endif	/* FEEDBACK_H */