#include <xc.h>
#include "pwm.h"
#include "test.h"
#include "i2c.h"
/**
 * Point d'entrée des interruptions basse priorité.
 */
void recepteurInterruptions() {
    unsigned char p1, p3;
    unsigned char adresse;
    
    if (PIR1bits.TMR2IF) {
        if (pwmEspacement()) {
            p1 = pwmValeur(0);
            p3 = pwmValeur(1);
            CCPR3L = p3;
            CCPR1L = p1;
        } else {
            CCPR3L = 0;
            CCPR1L = 0;
        }
        PIR1bits.TMR2IF = 0;
    }

    if (PIR1bits.SSP1IF) {
        if (SSP1STATbits.BF) {
            if (SSP1STATbits.DA) {
                pwmEtablitValeur(SSP1BUF);
            } else {
                adresse = SSP1BUF;
                // Le bit moins signifiant de SSPxBUF contient R/W.
                // Il faut donc décaler l'adresse de 1 bit vers la droite.
                adresse >>= 1;
                adresse &= 0b00000001;
                pwmPrepareValeur(adresse);
            }
        }
        PIR1bits.SSP1IF = 0;
    }
}

/**
 * Initialise le hardware pour l'émetteur.
 */
static void recepteurInitialiseHardware() {
    
    // Prépare Temporisateur 2 pour PWM (compte jusqu'à 125 en 2ms):
    T2CONbits.T2CKPS = 1;       // Diviseur de fréquence 1:4
    T2CONbits.T2OUTPS = 0;      // Pas de diviseur de fréquence à la sortie.
    T2CONbits.TMR2ON = 1;       // Active le temporisateur.
    
    PIE1bits.TMR2IE = 1;        // Active les interruptions ...
    IPR1bits.TMR2IP = 0;        // ... de basse priorité ...
    PIR1bits.TMR2IF = 0;        // ... pour le temporisateur 2.

    // Configure PWM 1 et 3 pour émettre le signal de radio-contrôle:
    ANSELCbits.ANSC2 = 0;
    TRISCbits.RC2 = 0;
    ANSELCbits.ANSC6 = 0;
    TRISCbits.RC6 = 0;
    
    CCP3CONbits.P3M = 0;        // Simple PWM sur la sortie P3A.
    CCP3CONbits.CCP3M = 12;     // Active le CCP3 comme PWM.
    CCPTMRS0bits.C3TSEL = 0;    // Branché sur le temporisateur 2.
    
    CCP1CONbits.P1M = 0;        // Simple PWM sur la sortie P1A
    CCP1CONbits.CCP1M = 12;     // Active le CCP1 comme PWM.
    CCPTMRS0bits.C1TSEL = 0;    // Branche le CCP1 sur le temporisateur 2.

    PR2 = 200;                  // Période est 2ms plus une marge de sécurité.
                                // (Proteus n'aime pas que CCPRxL dépasse PRx)

    // Active le MSSP1 en mode Esclave I2C:
    TRISCbits.RC3 = 1;          // RC3 comme entrée...
    ANSELCbits.ANSC3 = 0;       // ... digitale.
    TRISCbits.RC4 = 1;          // RC4 comme entrée...
    ANSELCbits.ANSC4 = 0;       // ... digitale.

    SSP1CON1bits.SSPEN = 1;     // Active le module SSP.    
    
    SSP1ADD = ECRITURE_SERVO_0;   // Adresse de l'esclave.
    SSP1MSK = 0b11111100;       // L'esclave occupe 2 adresses.
    SSP1CON1bits.SSPM = 0b1110; // SSP1 en mode esclave I2C avec adresse de 7 bits et interruptions STOP et START.
    
    SSP1CON3bits.PCIE = 0;      // Désactive l'interruption en cas STOP.
    SSP1CON3bits.SCIE = 0;      // Désactive l'interruption en cas de START.
    SSP1CON3bits.SBCDE = 0;     // Désactive l'interruption en cas de collision.

    PIE1bits.SSP1IE = 1;        // Interruption en cas de transmission I2C...
    IPR1bits.SSP1IP = 0;        // ... de basse priorité.

    // Active les interruptions générales:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */
void recepteurMain(void) {
    recepteurInitialiseHardware();
    pwmReinitialise();
    i2cReinitialise();

    while(1);
}
