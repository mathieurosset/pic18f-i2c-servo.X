#include <xc.h>
#include "i2c.h"
#include "file.h"
#include "pwm.h"
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

typedef enum {
    I2C_MASTER_EMISSION_ADRESSE,
    I2C_MASTER_PREPARE_RECEPTION_DONNEE,
    I2C_MASTER_RECEPTION_DONNEE,
    I2C_MASTER_EMISSION_DONNEE,
    I2C_MASTER_EMISSION_STOP,
    I2C_MASTER_FIN_OPERATION,
            
} EtatMaitreI2C;

static EtatMaitreI2C etatMaitre = I2C_MASTER_EMISSION_ADRESSE;

/**
 * Prépare l'émission de la commande indiquée.
 * @param type Type de commande. 
 * @param valeur Valeur associée.
 */
void i2cPrepareCommandePourEmission(I2cAdresse adresse, unsigned char valeur) {
    fileEnfile(&fileEmission, adresse);
    fileEnfile(&fileEmission, valeur);
    if (etatMaitre == I2C_MASTER_EMISSION_ADRESSE) {
        SSP1CON2bits.SEN = 1;
    }
}

/** 
 * Adresse de la fonction à appeler pour compléter 
 * l'exécution d'une commande I2C.
 */
static I2cRappelCommande rappelCommande;

/**
 * Établit la fonction à appeler pour compléter l'exécution d'une commande I2C.
 * Le maître appelle cette fonction pour terminer l'exécution 
 * d'une commande de lecture. L'esclave appelle cette fonction pour gérer 
 * l'exécution d'une commande d'écriture.
 * @param r La fonction à appeler.
 */
void i2cRappelCommande(I2cRappelCommande r) {
    rappelCommande = r;
}

void i2cMaitre() {
    static unsigned char adresse; // Adresse associée à la commande en cours.
    
    switch (etatMaitre) {
        case I2C_MASTER_EMISSION_ADRESSE:
            if (i2cDonneesDisponiblesPourEmission()) {
                adresse = i2cRecupereCaracterePourEmission();
                if (adresse & 1) {
                    etatMaitre = I2C_MASTER_PREPARE_RECEPTION_DONNEE;
                } else {
                    etatMaitre = I2C_MASTER_EMISSION_DONNEE;
                }
                SSP1BUF = adresse;
            }
            break;
            
        case I2C_MASTER_EMISSION_DONNEE:
            etatMaitre = I2C_MASTER_EMISSION_STOP;
            SSP1BUF = i2cRecupereCaracterePourEmission();
            break;

        case I2C_MASTER_PREPARE_RECEPTION_DONNEE:
            etatMaitre = I2C_MASTER_RECEPTION_DONNEE;
            i2cRecupereCaracterePourEmission();
            SSP1CON2bits.RCEN = 1;  // MMSP en réception.
            break;
            
        case I2C_MASTER_RECEPTION_DONNEE:
            etatMaitre = I2C_MASTER_EMISSION_STOP;
            rappelCommande(adresse, SSP1BUF);
            SSP1CON2bits.ACKDT = 1; // NACK
            SSP1CON2bits.ACKEN = 1; // Transmet le NACK
            break;
            
        case I2C_MASTER_EMISSION_STOP:
            etatMaitre = I2C_MASTER_FIN_OPERATION;
            SSP1CON2bits.PEN = 1;
            break;
            
        case I2C_MASTER_FIN_OPERATION:
            etatMaitre = I2C_MASTER_EMISSION_ADRESSE;
            if (i2cDonneesDisponiblesPourEmission()) {
                SSP1CON2bits.SEN = 1;
            }
            break;
    }
}
/**
 * Le bit moins signifiant de SSPxBUF contient R/W.
 * Il faut donc décaler l'adresse de 1 bit vers la droite.
 * @param adresse Adresse reçue par le bus I2C.
 * @return Adresse locale.
 */
unsigned char convertitEnAdresseLocale(unsigned char adresse) {
    adresse >>= 1;
    adresse &= I2C_MASQUE_ADRESSES_LOCALES;
    return adresse;
}

/** Liste des valeurs exposées par l'esclave I2C. */
unsigned char i2cValeursExposees[I2C_NOMBRE_ADRESSES_PAR_ESCLAVE];

/**
 * L'esclave rendra la valeur indiquée à prochaine lecture de l'adresse indiquée
 * sur le bus I2C.
 * @param adresse Adresse locale, entre 0 et 4 (l'adresse locale est constituée
 * des 2 bits moins signifiants de l'adresse demandée par le maître).
 * @param valeur La valeur.
 */
void i2cExposeValeur(unsigned char adresse, unsigned char valeur) {
    i2cValeursExposees[adresse] = valeur;
}

/**
 * Automate esclave I2C.
 * @param valeursLecture Liste de valeurs à rendre en cas d'opération
 * de lecture.
 */
void i2cEsclave() {
    static unsigned char adresse;
    
    // Machine à état extraite de Microchip AN734 - Apendix B
    if (SSP1STATbits.S) {
        if (SSP1STATbits.RW) {
            // État 4 - Opération de lecture, dernier octet transmis est une donnée:
            // Jamais utilisé si les commandes ont un seul octet associé.
            if (SSP1STATbits.DA) {
                SSP1BUF = i2cValeursExposees[adresse];
                SSP1CON1bits.CKP = 1;
            } 
            // État 3 - Opération de lecture, dernier octet reçu est une adresse:
            else {
                adresse = convertitEnAdresseLocale(SSP1BUF);
                SSP1BUF = i2cValeursExposees[adresse];
                SSP1CON1bits.CKP = 1;
            }
        } else {
            if (SSP1STATbits.BF) {
                // État 2 - Opération de lecture, dernier octet reçu est une donnée:
                if (SSP1STATbits.DA) {
                    rappelCommande(adresse, SSP1BUF);
                }
                // État 1 - Opération de lecture, dernier octet reçu est une adresse:
                else {
                    adresse = convertitEnAdresseLocale(SSP1BUF);
                }
            }
        }
    }
    PIR1bits.SSP1IF = 0;
}

/**
 * Réinitialise la machine i2c.
 */
void i2cReinitialise() {
    etatMaitre = I2C_MASTER_EMISSION_ADRESSE;
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