#ifndef I2C__H
#define I2C__H

typedef enum {
    SERVO1 = 64,
    SERVO2 = 65
} CommandeType;

typedef enum {
    LECTURE_POTENTIOMETRE  = 0b00001101,
    ECRITURE_SERVO_0 = 0b00001100,
    ECRITURE_SERVO_1 = 0b00001110
} Adresse;

typedef struct {
    Adresse adresse;
    unsigned char valeur;
} Commande;

void i2cPrepareCommandePourEmission(Adresse adresse, unsigned char valeur);

unsigned char i2cDonneesDisponiblesPourEmission();
unsigned char i2cRecupereCaracterePourEmission();
unsigned char i2cCommandeCompletementEmise();

void i2cReinitialise();

#ifdef TEST
void testI2c();
#endif

#endif