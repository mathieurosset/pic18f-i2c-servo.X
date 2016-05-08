#ifndef I2C__H
#define I2C__H

#define I2C_NOMBRE_ADRESSES_PAR_ESCLAVE 4
#define I2C_MASQUE_ADRESSES_LOCALES 0b11
#define I2C_MASQUE_ADRESSES_ESCLAVES 0b11111000

typedef enum {
    LECTURE_POTENTIOMETRE  = 0b00011001,
    ECRITURE_SERVO_0 = 0b00011000,
    ECRITURE_SERVO_1 = 0b00011010
} I2cAdresse;

typedef struct {
    I2cAdresse adresse;
    unsigned char valeur;
} I2cCommande;

typedef void (*I2cRappelCommande)(unsigned char, unsigned char);
void i2cRappelCommande(I2cRappelCommande r);
void i2cExposeValeur(unsigned char adresse, unsigned char valeur);
void i2cPrepareCommandePourEmission(I2cAdresse adresse, unsigned char valeur);
unsigned char i2cDonneesDisponiblesPourEmission();
unsigned char i2cRecupereCaracterePourEmission();

void i2cMaitre();
void i2cEsclave();

void i2cReinitialise();

#ifdef TEST
void testI2c();
#endif

#endif