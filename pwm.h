#ifndef PWM__TEST
#define PWM__TEST

unsigned char pwmValeur(unsigned char canal);
void pwmPrepareValeur(unsigned char canal);
void pwmEtablitValeur(unsigned char valeur);
void pwmEtablitValeurCanal(unsigned char adresse, unsigned char valeur);
unsigned char pwmEspacement();
void pwmDemarreCapture(unsigned char canal, unsigned int instant);
void pwmCompleteCapture(unsigned char canal, unsigned int instant);
void pwmReinitialise();

#ifdef TEST
void testPwm();
#endif

#endif