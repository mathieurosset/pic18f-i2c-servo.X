
/**
 * Chaque type de commande correspond à une action différente.
 */
typedef enum {    
    SERVO1,
    SERVO2,       
} CommandeType;

/**
 * Une commande contient un type de commande plus une valeur associée.
 */
typedef struct {
    CommandeType type;
    unsigned char valeur;
} CommandeType;

