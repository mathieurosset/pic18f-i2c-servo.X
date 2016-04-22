#include <xc.h>
#include "emetteur.h"
#include "recepteur.h"
#include "pwm.h"
#include "i2c.h"
#include "test.h"

/**
 * Bits de configuration:
 */
#pragma config FOSC = INTIO67   // Osc. interne, A6 et A7 comme IO.
#pragma config IESO = OFF       // Pas d'osc. au démarrage.
#pragma config FCMEN = OFF      // Pas de monitorage de l'oscillateur.

// Nécessaires pour ICSP / ICD:
#pragma config MCLRE = EXTMCLR  // RE3 est actif comme master reset.
#pragma config WDTEN = OFF      // Watchdog inactif.
#pragma config LVP = OFF        // Single Supply Enable bits off.

// Sorties des PWM:
#pragma config CCP3MX = PORTC6  // Sortie PA3 connectée sur PORTC6.

#ifndef TEST

typedef enum {
    EMETTEUR,
    RECEPTEUR
} Mode;

Mode mode;

/**
 * Point d'entrée des interruptions basse priorité.
 */
void low_priority interrupt interruptionsBassePriorite() {
    if (mode == EMETTEUR) {
        emetteurInterruptions();
    } else {
        recepteurInterruptions();
    }    
}

/**
 * Point d'entrée.
 * Suivant la valeur du port B4, il lance le programme
 * en mode émetteur ou en mode récepteur.
 */
void main(void) {
    
    TRISBbits.RB4 = 1;
    ANSELBbits.ANSB4 = 0;
    
    if (PORTBbits.RB4) {
        mode = RECEPTEUR;
    } else {
        mode = EMETTEUR;
    }
    
    if (mode == EMETTEUR) {
        emetteurMain();
    } else {
        recepteurMain();
    }
    
    while(1);
}
#endif

#ifdef TEST
void main() {
    initialiseTests();
    testPwm();
    testI2c();
    finaliseTests();
    while(1);
}
#endif
