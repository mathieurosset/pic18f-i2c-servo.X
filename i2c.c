#include <xc.h>
#include "i2c.h"
#include "file.h"
#include "test.h"

#define I2C_LONGEUR_COMMANDE 3

/**
 * États possibles de la commande en cours.
 */
typedef enum {
    ADRESSE,
    COMMANDE,
    VALEUR,
    COMMANDE_TERMINEE
} EtatTransmissionCommande;

/** État de la commande en cours. */
EtatTransmissionCommande etatTransmissionCommande = COMMANDE_TERMINEE;

File fileEmission;

/**
 * @return 255 / -1 si il reste des données à émettre.
 */
unsigned char i2cDonneesDisponiblesPourEmission() {
    if (fileEstVide(&fileEmission)) {
        return 0;
    } else {
        if (etatTransmissionCommande == COMMANDE_TERMINEE) {
            etatTransmissionCommande = ADRESSE;
        }
        return 255;
    }
}

/**
 * Rend le prochain caractère à émettre.
 * Appelez i2cCommandeCompletementEmise avant, pour éviter
 * de consommer la prochaine commande.
 * @return 
 */
unsigned char i2cRecupereCaracterePourEmission() {
    switch(etatTransmissionCommande) {
        case ADRESSE:
            etatTransmissionCommande = COMMANDE;
            return fileDefile(&fileEmission);
        case COMMANDE:
            etatTransmissionCommande = VALEUR;
            return fileDefile(&fileEmission);
        case VALEUR:
            etatTransmissionCommande = COMMANDE_TERMINEE;
            return fileDefile(&fileEmission);
        default:
            return 0;
    }
}

/**
 * Indique si la commande en cours a été complètement émise.
 * @return -1 / 255 si tous les caractères d'une commande ont été émis.
 */
unsigned char i2cCommandeCompletementEmise() {
    if (etatTransmissionCommande == COMMANDE_TERMINEE) {
        return 255;
    } else {
        return 0;
    }
}

/**
 * Prépare l'émission de la commande indiquée.
 * @param type Type de commande. 
 * @param valeur Valeur associée.
 */
void i2cPrepareCommandePourEmission(Adresse adresse, CommandeType type, unsigned char valeur) {
    fileEnfile(&fileEmission, adresse);
    if (etatTransmissionCommande == COMMANDE_TERMINEE) {
        etatTransmissionCommande = ADRESSE;
    }
    fileEnfile(&fileEmission, type);
    fileEnfile(&fileEmission, valeur);
}

Commande commandeEnCoursDeReception;

void i2cReceptionAdresse(Adresse adresse) {
    commandeEnCoursDeReception.adresse = adresse;
    commandeEnCoursDeReception.commande = 0;
    commandeEnCoursDeReception.valeur = 0;
}

void i2cReceptionDonnee(unsigned char donnee) {
    commandeEnCoursDeReception.commande = commandeEnCoursDeReception.valeur;
    commandeEnCoursDeReception.valeur = donnee;    
}

File fileReception;

void i2cFinDeReception() {
    fileEnfile(&fileReception, commandeEnCoursDeReception.commande);
    fileEnfile(&fileReception, commandeEnCoursDeReception.valeur);
}

unsigned char i2cCommandeRecue() {
    if (fileEstVide(&fileReception)) {
        return 0;
    } else {
        return 1;
    }
}

void i2cLitCommandeRecue(Commande *commande) {
    commande->commande = fileDefile(&fileReception);
    commande->valeur = fileDefile(&fileReception);
}

/**
 * Réinitialise la machine i2c.
 */
void i2cReinitialise() {
    fileReinitialise(&fileEmission);
    fileReinitialise(&fileReception);
    etatTransmissionCommande = COMMANDE_TERMINEE;
}

#ifdef TEST
void testEmissionUneCommande() {
    i2cReinitialise();
    
    i2cPrepareCommandePourEmission(MODULE_SERVO, SERVO1, 10);
    testeEgaliteEntiers("I2CE01", i2cDonneesDisponiblesPourEmission(), 255);
    testeEgaliteEntiers("I2CE02", i2cRecupereCaracterePourEmission(), MODULE_SERVO);
    testeEgaliteEntiers("I2CE03", i2cCommandeCompletementEmise(), 0);
    testeEgaliteEntiers("I2CE04", i2cRecupereCaracterePourEmission(), SERVO1);
    testeEgaliteEntiers("I2CE05", i2cCommandeCompletementEmise(), 0);
    testeEgaliteEntiers("I2CE06", i2cRecupereCaracterePourEmission(), 10);
    testeEgaliteEntiers("I2CE07", i2cCommandeCompletementEmise(), 255);
    testeEgaliteEntiers("I2CE08", i2cDonneesDisponiblesPourEmission(), 0);    
}

void testEmissionDeuxCommandes() {
    i2cReinitialise();
    i2cPrepareCommandePourEmission(MODULE_SERVO, SERVO1, 10);

    // Première commande:
    testeEgaliteEntiers("I2CEA01", i2cDonneesDisponiblesPourEmission(), 255);
    testeEgaliteEntiers("I2CEA02", i2cRecupereCaracterePourEmission(), MODULE_SERVO);
    testeEgaliteEntiers("I2CEA03", i2cCommandeCompletementEmise(), 0);
    testeEgaliteEntiers("I2CEA04", i2cRecupereCaracterePourEmission(), SERVO1);
    testeEgaliteEntiers("I2CEA05", i2cCommandeCompletementEmise(), 0);
    testeEgaliteEntiers("I2CEA06", i2cRecupereCaracterePourEmission(), 10);
    // Deuxième commande arrive avant que la première se termine.
    i2cPrepareCommandePourEmission(MODULE_SERVO, SERVO2, 20);
    testeEgaliteEntiers("I2CEAD07", i2cCommandeCompletementEmise(), 255);
    
    // Deuxième commande:
    testeEgaliteEntiers("I2CEA09", i2cDonneesDisponiblesPourEmission(), 255);
    testeEgaliteEntiers("I2CEA10", i2cRecupereCaracterePourEmission(), MODULE_SERVO);
    testeEgaliteEntiers("I2CEA11", i2cCommandeCompletementEmise(), 0);
    testeEgaliteEntiers("I2CEA12", i2cRecupereCaracterePourEmission(), SERVO2);
    testeEgaliteEntiers("I2CEA13", i2cCommandeCompletementEmise(), 0);
    testeEgaliteEntiers("I2CEA14", i2cRecupereCaracterePourEmission(), 20);
    testeEgaliteEntiers("I2CEA15", i2cCommandeCompletementEmise(), 255);
    
    // Plus de commandes:
    testeEgaliteEntiers("I2CEA16", i2cDonneesDisponiblesPourEmission(), 0);
}

void testI2c() {
    testEmissionUneCommande();
    testEmissionDeuxCommandes();
}
#endif