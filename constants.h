#ifndef _CONSTANTS
  #define _CONSTANTS
  // Constantes para el intercambio de mensajes
  #define PIZZA_KEYWORD "pizza"
  #define PAN_KEYWORD "pan"
  #define MASA_KEYWORD "masa"
  #define MASA_KEYWORD_LEN 4
  #define PAN_KEYWORD_LEN 3
  #define PIZZA_KEYWORD_LEN 5
  #define DEFAULT_BUFFER_LEN 15
  // Tiempo de coccion del pan
  #define TIEMPO_COCCION_PANADERO 0.5
  // Parametro de la distribucion uniforme para el tiempo de coccion de la pizza
  #define PIZZA_TIEMPO_LOWER_UNIFORM_VALUE 0.5
  #define PIZZA_TIEMPO_UPPER_UNIFORM_VALUE 1.0
  // Tiempo que se tarda en alimentar las masas madres
  #define TIEMPO_ALIMENTAR_MASA_MADRE 0.1
  // Cantidad de alimentaciones que debe recibir una masa madre antes de estar lista
  #define ETAPAS_MASA_MADRE 7
  // Cantidad de cocciones simultaneas que puede realizar un cocinero
  #define MAXIMO_SIMULTANEO_POR_COCINERO 12
  // Stock inicial de masas madre que el especialista tiene del dia anterior
  #define MASAS_MADRE_INICIALES 10
#endif
