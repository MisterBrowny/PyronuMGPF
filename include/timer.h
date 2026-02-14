#ifndef TIMER_H
#define	TIMER_H

#define TDef5ms		5
#define TDef10ms	10
#define TDef20ms	20
#define TDef50ms	50
#define TDef100ms	100
#define TDef200ms	200
#define TDef250ms	250
#define TDef500ms	500
#define TDef1sec	1000
#define TDef2sec	2000
#define TDef3sec	3000
#define TDef4sec	4000
#define TDef5sec	5000
#define TDef10sec	10000


// volatile uint16_t Cptms; // Remplacer par millis()


#define TempsSup(t,c)	(((unsigned long) (millis() - t) > c) ? 1 : 0)
#define TempsInf(t,c)	(((unsigned long) (millis() - t) < c) ? 1 : 0)


#endif	/* TIMER_H */

