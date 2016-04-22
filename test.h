#ifndef TEST_H
#define	TEST_H

#ifdef TEST

/**
 * Initialise la console de test.
 */
void initialiseTests();

/**
 * Vérifie si la valeur obtenue est égale à la valeur attendue.
 * @param testId Identifiant du test.
 * @param value Valeur obtenue.
 * @param expectedValue Valeur attendue.
 * @return 1 si le test échoue, ce qui permet de compter le nombre
 * de tests en erreur.
 */
int testeEgaliteEntiers(const char *testId, int value, int expectedValue);

/**
 * Vérifie si la valeur obtenue est égale à la valeur attendue.
 * @param testId Identifiant du test.
 * @param valeurObtenue Valeur obtenue.
 * @param valeurAttendue Valeur attendue.
 * @return 255 si le test échoue.
 */
unsigned char testeEgaliteChars(const char *testId, char valeurObtenue, char valeurAttendue);

/**
 * Affiche le nombre de tests en échec.
 */
void finaliseTests();

#endif

#endif
