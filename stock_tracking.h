#ifndef _STOCK_TRACKING
  #define _STOCK_TRACKING
  typedef struct stock_tracker {
    size_t por_ordenar;
    size_t por_entregar;
    size_t entregados;
  } stock_tracker_t;

  typedef struct pedidos_count {
    stock_tracker_t pizzas;
    stock_tracker_t panes;
    int cant_pedidos;
  } pedidos_count_t;
#endif
