#include <xc.h>
#include <stdio.h>

#ifdef TEST

/**
 * Fonction qui transmet un caractère à la EUSART.
 * Il s'agit de l'implémentation d'une fonction système qui est
 * appelée par <code>printf</code>.
 * Cette implémentation envoie le caractère à la UART. Si un terminal
 * est connecté aux sorties RX / TX, il affichera du texte.
 * @param data Le code ASCII du caractère à afficher.
*/
void putch(char data) {
    while( ! TX1IF);
    TXREG1 = data;
}

void initialiseUART1() {
    // Pour une fréquence de 1MHz, ceci donne 1200 bauds:
    SPBRG = 12;
    SPBRGH = 0;
    // Configure RC6 et RC7 comme entrées digitales, pour que
    // la EUSART puisse en prendre le contrôle:
    TRISCbits.RC6 = 1;
    TRISCbits.RC7 = 1;
    
    // Configure la EUSART:
    // (BRGH et BRG16 sont à leur valeurs par défaut)
    // (TX9 est à sa valeur par défaut)
    RCSTAbits.SPEN = 1;  // Active la EUSART.
    TXSTAbits.SYNC = 0;  // Mode asynchrone.
    TXSTAbits.TXEN = 1;  // Active l'émetteur.
}

/** Nombre de tests en erreur depuis l'initialisation des tests. */
static int testsEnErreur = 0;

void initialiseTests() {
    initialiseUART1();
    testsEnErreur = 0;
    printf("\r\nLancement des tests...\r\n");
}

unsigned char testeEgaliteEntiers(const char *testId, int valeurObtenue, int valeurAttendue) {
    if (valeurObtenue != valeurAttendue) {
        printf("%s: Valeur obtenue %d - Valeur attendue %d\r\n", testId, valeurObtenue, valeurAttendue);
        testsEnErreur++;
        return 255;
    }
    return 0;
}

unsigned char testeEgaliteChars(const char *testId, char valeurObtenue, char valeurAttendue) {
    if (valeurObtenue != valeurAttendue) {
        printf("%s: Valeur obtenue <%c> - Valeur attendue <%c>\r\n", testId, valeurObtenue, valeurAttendue);
        testsEnErreur++;
        return 255;
    }
    return 0;
}

void finaliseTests() {
    printf("%d tests en erreur\r\n", testsEnErreur);    
}

#endif
