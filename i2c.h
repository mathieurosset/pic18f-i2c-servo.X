#ifndef I2C__H
#define I2C__H

typedef enum {
    SERVO1 = 64,
    SERVO2 = 65
} CommandeType;

typedef enum {
    MODULE_SERVO = 0b00001100
} Adresse;

typedef struct {
    Adresse adresse;
    CommandeType commande;
    unsigned char valeur;
} Commande;

void i2cPrepareCommandePourEmission(Adresse adresse, CommandeType type, unsigned char valeur);
unsigned char i2cDonneesDisponiblesPourEmission();
unsigned char i2cRecupereCaracterePourEmission();
unsigned char i2cCommandeCompletementEmise();
void i2cMaitre();

void i2cReceptionAdresse(Adresse adresse);
void i2cReceptionDonnee(unsigned char donnee);
void i2cFinDeReception();
unsigned char i2cCommandeRecue();
void i2cLitCommandeRecue(Commande *commande);

void i2cReinitialise();

#ifdef TEST
void testI2c();
#endif

#endif