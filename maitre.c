#include <xc.h>
#include "pwm.h"
#include "i2c.h"

/**
 * Point d'entrée des interruptions pour le maître.
 */
void maitreInterruptions() {

    static CommandeType commandeType;
    
    if (INTCON3bits.INT1F) {
        INTCON3bits.INT1F = 0;
        commandeType = ECRITURE_SERVO_0;
        ADCON0bits.GO = 1;
    }
    
    if (INTCON3bits.INT2F) {
        INTCON3bits.INT2F = 0;
        commandeType = ECRITURE_SERVO_1;
        ADCON0bits.GO = 1;
    }
    
    if (PIR1bits.ADIF) {
        PIR1bits.ADIF = 0;
        i2cPrepareCommandePourEmission(commandeType, ADRESH);
        SSP1CON2bits.SEN = 1;
    }
    
    if (PIR1bits.SSP1IF) {
        if (SSP1STATbits.P) {
            if (i2cDonneesDisponiblesPourEmission()) {
                SSP1CON2bits.SEN = 1;
            }
        } else {
            if (SSP1STATbits.BF == 0) {
                if (i2cCommandeCompletementEmise()) {
                    SSP1CON2bits.PEN = 1;
                } else {
                    SSP1BUF = i2cRecupereCaracterePourEmission();
                }
            }
        }
        PIR1bits.SSP1IF = 0;
    }
}

/**
 * Initialise le hardware pour le maître.
 */
static void maitreInitialiseHardware() {
    
    // Interruptions INT1 et INT2:
    TRISBbits.RB1 = 1;          // Port RB1 comme entrée...
    ANSELBbits.ANSB1 = 0;       // ... digitale.
    TRISBbits.RB2 = 1;          // Port RB2 comme entrée...
    ANSELBbits.ANSB2 = 0;       // ... digitale.
    
    INTCON2bits.RBPU = 0;       // Active les résistances de tirage...
    WPUBbits.WPUB1 = 1;         // ... pour INT1 ...
    WPUBbits.WPUB2 = 1;         // ... et INT2.
    
    INTCON3bits.INT1E = 1;      // INT1
    INTCON2bits.INTEDG1 = 0;    // Flanc descendant.
    INTCON3bits.INT2E = 1;      // INT2
    INTCON2bits.INTEDG2 = 0;    // Flanc descendant.

    // Active le module de conversion A/D:
    TRISBbits.RB3 = 1;      // Active RB4 comme entrée.
    ANSELBbits.ANSB3 = 1;   // Active AN11 comme entrée analogique.
    ADCON0bits.ADON = 1;    // Allume le module A/D.
    ADCON0bits.CHS = 9;     // Branche le convertisseur sur AN09
    ADCON2bits.ADFM = 0;    // Les 8 bits plus signifiants sur ADRESH.
    ADCON2bits.ACQT = 3;    // Temps d'acquisition à 6 TAD.
    ADCON2bits.ADCS = 0;    // À 1MHz, le TAD est à 2us.

    PIE1bits.ADIE = 1;      // Active les interruptions A/D
    IPR1bits.ADIP = 0;      // Interruptions A/D sont de basse priorité.

    // Active le MSSP1 en mode Maître I2C:
    TRISCbits.RC3 = 1;      // RC3 comme entrée...
    ANSELCbits.ANSC3 = 0;   // ... digitale.
    TRISCbits.RC4 = 1;      // RC4 comme entrée...
    ANSELCbits.ANSC4 = 0;   // ... digitale.

    SSP1CON1bits.SSPEN = 1;     // Active le module SSP.
    
    SSP1CON3bits.PCIE = 1;      // Active l'interruption en cas STOP.
    SSP1CON3bits.SCIE = 1;      // Active l'interruption en cas de START.
    SSP1CON1bits.SSPM = 0b1000; // SSP1 en mode maître I2C.
    SSP1ADD = 3;                // FSCL = FOSC / (4 * (SSP1ADD + 1)) = 62500 Hz.

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
void maitreMain(void) {
    maitreInitialiseHardware();
    i2cReinitialise();
    pwmReinitialise();

    while(1);
}
