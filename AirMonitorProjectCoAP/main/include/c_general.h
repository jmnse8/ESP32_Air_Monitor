
typedef enum {
    Estado_NOWifi, // Estado inicial
    Estado_SIWifi,
    Estado_Provisionado
} ESTADOS;

extern ESTADOS estado_actual;