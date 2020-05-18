#ifndef _MENSAJES
  #define _MENSAJES
  #define DEBUG_TAG L"[🐞 DEBUG] "
  #define INFO_TAG L"[📋 INFO]  "
  #define ERROR_TAG L"[❌ ERROR] "
  #define FATAL_TAG L"[💀 FATAL] "
  #define FATAL_TAG_NOT_UNICODE "[FATAL] "
  #define LOG_DATA_FORMAT "[%s:%d, pid: %ld] "
  // Main logs
  #define CANTIDAD_RECEPCIONISTAS L"🛎️ La cantidad de recepcionistas es %d"
  #define CANTIDAD_PIZZEROS L"👨‍🍳🍕 La cantidad de pizzeros es %d"
  #define CANTIDAD_PANADEROS L"👨‍🍳🍞 La cantidad de panaderos es %d"
  // Recepcionista logs
  #define INICIADO_RECEPCIONISTA L"🛎️ Iniciando recepcionista"
  #define RECEPCIONISTA_PIZZA_PEDIDA L"🛎️ Pedido por pizza recibido"
  #define RECEPCIONISTA_PAN_PEDIDO L"🛎️ Pedido por pan recibido"
  #define RECEPCIONISTA_PIDIENDO_PIZZA L"🛎️ Pidiendo pizza (pedido número %d) al maestro pizzero 👨‍🍳🍕"
  #define RECEPCIONISTA_PIDIENDO_PAN L"🛎️ Pidiendo pan (pedido número %d) al maestro panadero 👨‍🍳🍞"
  #define RECEPCIONISTA_RECIBIENDO_PAN L"🛎️ Recibiendo pan de maestro panadero 👨‍🍳🍞"
  #define RECEPCIONISTA_ERROR_PROCESAR_ORDEN L"🛎️ Se ha encontrado un error al procesar pedido número %d, se volverá a pedir luego"
  #define RECEPCIONISTA_STOP L"🛎️🛑 No se encontraron más trabajos pendientes, pizzas procesadas: %zu, panes procesados: %zu"
  // Maestro pizzero logs
  #define INICIANDO_MAESTRO_PIZZERO L"👨‍🍳🍕 Iniciando maestro pizzero"
  #define COCINANDO_PIZZA L"👨‍🍳🍕 Cocinando pizza (pedido número %d), tiempo de coccion: %.3f"
  #define ENTREGANDO_PIZZA L"👨‍🍳🍕 Entregando pizza (pedido número %d) al repartidor 🏍️"
  #define MAESTRO_PIZZERO_ERROR_AL_ENTREGAR L"👨‍🍳🍕 Error al entregar pizza (pedido número %d) al repartidor, reintentando"
  #define MAESTRO_PIZZERO_STOP L"👨‍🍳🛑 No se cocinaran mas pedidos, pizzas cocinadas: %d"
  #define PIDIENDO_MASA_MADRE_PIZZA L"👨‍🍳🍕 Pidiendo masa madre para pizza (pedido número %d)"
  // Maestro panadero logs
  #define INICIANDO_MAESTRO_PANADERO L"👨‍🍳🍞 Iniciando maestro panadero"
  #define COCINANDO_PAN L"👨‍🍳🍞 Cocinando pan (pedido número %d), tiempo de coccion: %.3f"
  #define ENTREGANDO_PAN L"👨‍🍳🍞 Entregando pan (pedido número %d) al repartidor 🏍️"
  #define MAESTRO_PANADERO_ERROR_AL_ENTREGAR L"👨‍🍳🍞 Error al entregar pan (pedido número %d) al repartidor, reintentando"
  #define MAESTRO_PANADERO_STOP L"👨‍🍳🛑 No se cocinaran mas pedidos, panes cocinados: %d"
  #define PIDIENDO_MASA_MADRE_PAN L"👨‍🍳🍞 Pidiendo masa madre para pan (pedido número %d)"
  // Especialista de masa madre
  #define INICIANDO_ESPECIALISTA_MASA_MADRE L"👨‍🔬🌾 Iniciando especialista de masa madre"
  #define ENTREGANDO_MASA_MADRE L"👨‍🔬🌾 Entregando masa madre 👨‍🍳"
  #define ENCOLANDO_MASA_MADRE L"👨‍🔬🌾 Agregando masa madre a la cola"
  #define ESPECIALISTA_MASA_MADRE_ERROR_AL_ENTREGAR L"👨‍🔬🌾 No se pudo entregar una masa madre"
  #define ALIMENTANDO_MASA_MADRE L"👨‍🔬🌾 Por alimentar masa madre, hay %zu masas nuevas y %zu masas listas"
  #define MASA_MADRE_ALIMENTADA L"👨‍🔬🌾 Masa madre alimentada, hay %zu listas"
  #define ESPECIALISTA_STOP L"👨‍🔬🌾🛑 No se encontraron más trabajos pendientes, %zu masas entregadas"
  // Repartidor logs
  #define INICIANDO_REPARTIDOR L"🏍️ Iniciando repartidor"
  #define REPARTIDOR_RECIBIENDO_PAN L"🏍️ Despachando pan (pedido número %d) del maestro panadero 💲"
  #define REPARTIDOR_RECIBIENDO_PIZZA L"🏍️ Despachando pizza (pedido número %d) del maestro pizzero 💲"
  #define REPARTIDOR_STOP L"🏍️🛑 No se encontraron más trabajos pendientes, pizzas repartidas: %zu, panes repartidos: %zu"
  // Errores generales
  #define ERROR_MENSAJE_NO_COMPRENDIDO L"Se recibio un mensaje incomprensible '%s'"
  // Errores fatales (se aborta la ejecucion)
  #define FATAL_SIGINT "Sigint atrapado, terminando proceso"
  #define FATAL_MALLOC "Error alocando recursos"
  #define FATAL_LECTURA "Error al leer archivo de entrada"
  #define FATAL_ARCHIVO_LOG "No se pudo abrir el archivo para guardar el log"
  #define FATAL_CAMPOS_NEGATIVOS "Uno de los parametros obligatorios es 0 o negativo"
  #define FATAL_PARAMETROS_FALTANTES "No se especificaron todos los parametros obligatorios"
  #define FATAL_ARCHIVO_ENTRADA "No fue posible abrir el archivo de entrada"
  #define FATAL_RESOURCE_MANAGER "Error al inicializar el administrador de memoria"
  #define FATAL_ACQUIRE_LOCK "Error fatal al adquirir lock"
  #define FATAL_ERROR_CHILD "Error fatal en proceso hijo, terminando todas las ejecuciones"
  #define FATAL_ERROR_WAIT "Error esperando proceso hijo, terminando todas las ejecuciones"
  #define FATAL_ERROR_PIPE_OPEN "Error al abrir pipes"
  #define FATAL_ERROR_PIPE_CLOSE "Error al cerrar pipe en el proceso padre"
  #define FATAL_ERROR_SHARED_MEMORY "Error fatal al reservar memoria compartida"
  #define FATAL_ERROR_LOCKFILE "Error al crear un lockfile"
  #define FATAL_ERROR_SIGPIPE_IGNORE "Error fatal al intentar ignorar SIGPIPE"
#endif
