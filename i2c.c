#include <xc.h>
#include "i2c.h"
#include "file.h"
#include "test.h"

File fileEmission;

/**
 * @return 255 / -1 si il reste des données à émettre.
 */
unsigned char i2cDonneesDisponiblesPourEmission() {
    if (fileEstVide(&fileEmission)) {
        return 0;
    }
    return 255;
}

/**
 * Rend le prochain caractère à émettre.
 * Appelez i2cCommandeCompletementEmise avant, pour éviter
 * de consommer la prochaine commande.
 * @return 
 */
unsigned char i2cRecupereCaracterePourEmission() {
    return fileDefile(&fileEmission);
}

/**
 * Prépare l'émission de la commande indiquée.
 * @param type Type de commande. 
 * @param valeur Valeur associée.
 */
void i2cPrepareCommandePourEmission(Adresse adresse, unsigned char valeur) {
    fileEnfile(&fileEmission, adresse);
    fileEnfile(&fileEmission, valeur);
}

typedef enum {
    DEBUT_OPERATION,
    PREPARE_RECEPTION,
    RECEPTION_DONNEE,
    EMISSION_DONNEE,
    EMISSION_STOP,
    FIN_OPERATION,
            
} EtatMaitreI2C;

static EtatMaitreI2C etatMaitre = DEBUT_OPERATION;

void i2cMaitre() {
    unsigned char adresse;
    
    switch (etatMaitre) {
        case DEBUT_OPERATION:
            if (i2cDonneesDisponiblesPourEmission()) {
                adresse = i2cRecupereCaracterePourEmission();
                SSP1BUF = adresse;
                if (adresse & 1) {
                    etatMaitre = PREPARE_RECEPTION;
                } else {
                    etatMaitre = EMISSION_DONNEE;
                }
            }
            break;
            
        case EMISSION_DONNEE:
            SSP1BUF = i2cRecupereCaracterePourEmission();
            etatMaitre = EMISSION_STOP;
            break;

        case PREPARE_RECEPTION:
            SSP1CON2bits.RCEN = 1;
            etatMaitre = RECEPTION_DONNEE;
            break;
            
        case RECEPTION_DONNEE:
            PORTA = SSP1BUF;
            SSP1CON2bits.ACKEN = 1;
            etatMaitre = EMISSION_STOP;
            break;
            
        case EMISSION_STOP:
            SSP1CON2bits.PEN = 1;
            etatMaitre = FIN_OPERATION;
            break;
            
        case FIN_OPERATION:
            if (i2cDonneesDisponiblesPourEmission()) {
                SSP1CON2bits.SEN = 1;
            }
            etatMaitre = DEBUT_OPERATION;
            break;
    }
}

/**
 * Réinitialise la machine i2c.
 */
void i2cReinitialise() {
    etatMaitre = DEBUT_OPERATION;
    fileReinitialise(&fileEmission);
}

#ifdef TEST
void testEmissionUneCommande() {
    i2cReinitialise();
    
    i2cPrepareCommandePourEmission(ECRITURE_SERVO_0, 10);
    testeEgaliteEntiers("I2CE01", i2cDonneesDisponiblesPourEmission(), 255);
    testeEgaliteEntiers("I2CE02", i2cRecupereCaracterePourEmission(), ECRITURE_SERVO_0);
    testeEgaliteEntiers("I2CE03", i2cCommandeCompletementEmise(), 0);
    testeEgaliteEntiers("I2CE06", i2cRecupereCaracterePourEmission(), 10);
    testeEgaliteEntiers("I2CE07", i2cCommandeCompletementEmise(), 255);
    testeEgaliteEntiers("I2CE08", i2cDonneesDisponiblesPourEmission(), 0);    
}

void testEmissionDeuxCommandes() {
    i2cReinitialise();
    i2cPrepareCommandePourEmission(ECRITURE_SERVO_0, 10);

    // Première commande:
    testeEgaliteEntiers("I2CEA01", i2cCommandeCompletementEmise(), 0);
    testeEgaliteEntiers("I2CEA02", i2cDonneesDisponiblesPourEmission(), 255);
    testeEgaliteEntiers("I2CEA03", i2cRecupereCaracterePourEmission(), ECRITURE_SERVO_0);
    testeEgaliteEntiers("I2CEA04", i2cCommandeCompletementEmise(), 0);
    testeEgaliteEntiers("I2CEA05", i2cRecupereCaracterePourEmission(), 10);
    // Deuxième commande arrive avant que la première se termine.
    i2cPrepareCommandePourEmission(ECRITURE_SERVO_1, 20);
    testeEgaliteEntiers("I2CEAD06", i2cCommandeCompletementEmise(), 255);
    
    // Deuxième commande:
    testeEgaliteEntiers("I2CEA07", i2cDonneesDisponiblesPourEmission(), 255);
    testeEgaliteEntiers("I2CEA08", i2cRecupereCaracterePourEmission(), ECRITURE_SERVO_1);
    testeEgaliteEntiers("I2CEA09", i2cCommandeCompletementEmise(), 0);
    testeEgaliteEntiers("I2CEA10", i2cRecupereCaracterePourEmission(), 20);
    testeEgaliteEntiers("I2CEA11", i2cCommandeCompletementEmise(), 255);
    
    // Plus de commandes:
    testeEgaliteEntiers("I2CEA12", i2cDonneesDisponiblesPourEmission(), 0);
}

void testI2c() {
    testEmissionUneCommande();
    testEmissionDeuxCommandes();
}
#endif