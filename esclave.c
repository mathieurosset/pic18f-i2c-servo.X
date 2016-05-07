#include <xc.h>
#include "pwm.h"
#include "test.h"
#include "i2c.h"

/**
 * Point d'entrée des interruptions pour l'esclave.
 */
void esclaveInterruptions() {
    unsigned char p1, p3;

    if (PIR1bits.TMR1IF) {
        TMR1 = 3035;
        ADCON0bits.GO = 1;
        PIR1bits.TMR1IF = 0;
    }

    if (PIR1bits.ADIF) {
        i2cExposeValeur(0, ADRESH);
        PIR1bits.ADIF = 0;
    }

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
        i2cEsclave();
    }
}

/**
 * Initialise le hardware pour l'émetteur.
 */
static void esclaveInitialiseHardware() {

    // Prépare Temporisateur 1 pour 4 interruptions par sec.
    T1CONbits.TMR1CS = 0;   // Source FOSC/4
    T1CONbits.T1CKPS = 0;   // Pas de diviseur de fréquence.
    T1CONbits.T1RD16 = 1;   // Compteur de 16 bits.
    T1CONbits.TMR1ON = 1;   // Active le temporisateur.
    
    PIE1bits.TMR1IE = 1;    // Active les interruptions...
    IPR1bits.TMR1IP = 0;    // ... de basse priorité.
    
    // Active le module de conversion A/D:
    TRISBbits.RB3 = 1;      // Active RB3 comme entrée.
    ANSELBbits.ANSB3 = 1;   // Active AN09 comme entrée analogique.
    ADCON0bits.ADON = 1;    // Allume le module A/D.
    ADCON0bits.CHS = 9;     // Branche le convertisseur sur AN09
    ADCON2bits.ADFM = 0;    // Les 8 bits plus signifiants sur ADRESH.
    ADCON2bits.ACQT = 3;    // Temps d'acquisition à 6 TAD.
    ADCON2bits.ADCS = 0;    // À 1MHz, le TAD est à 2us.

    PIE1bits.ADIE = 1;      // Active les interruptions A/D
    IPR1bits.ADIP = 0;      // Interruptions A/D sont de basse priorité.
    
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
    SSP1MSK = I2C_MASQUE_ADRESSES_ESCLAVES;
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
 * Point d'entrée pour l'esclave.
 */
void esclaveMain(void) {
    esclaveInitialiseHardware();
    pwmReinitialise();
    i2cReinitialise();
    i2cRappelCommande(pwmEtablitValeurCanal);
    while(1);
}
