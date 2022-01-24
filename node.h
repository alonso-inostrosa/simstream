#ifndef NODE_H
#define NODE_H

#include "glob.h"
#include "distributions/wrapper.h"

class Processor;

class Node
{
protected:
  //Stream Grouping
  enum stream_grouping_option {SHUFFLE_GROUPING=0/*RR*/, FIELD_GROUPING/*HASH*/, GLOBAL_GROUPING, ALL_GROUPING};

  //Tupla procesada/generada en run() de Spout o Bolt
  Wrapper _wrapper;
  shared_ptr<Tupla> _tupla;


  //Metricas:
  double _tpo_servicio = 0.0;
  uint32_t _tuplas_procesadas = 0;

  uint32_t _nro_tupla_local = 0;

public:
  enum node_type {SPOUT=0, BOLT};

  node_type _node_type;

  /*Lista de nodos hacia los cuales se envia tuplas (en realidad siempre seran Bolts).
  * Despues de producir la(s) tuplas y de que hayan sido encoladas, se debe activar los bolts correspondientes.
  * Se utiliza en SHUFFLE GRUPING (RR)
  */
  map< std::string, list<Node*> > _node_map_list;

  /*
  * Vector de nodos hacia los cuales se envia tuplas (en realidad siempre seran Bolts).
  * Despues de producir la(s) tuplas y de que hayan sido encoladas, se debe activar los bolts correspondientes.
  * Se utiliza en FIELD GROUPING (HASH)
  */
  map< std::string, vector<Node*> > _node_map_vector;

  uint32_t _rep_id; //Identifica a una replica dentro de un mismo Spout/Bolt, puede repetirse pero para diferentes Bolts.
  uint32_t _rep_level;
  string _node_id;
  string _name;
  
  //Tiempo de servicio del nodo (HOLD)
  std::string _avg_service_time;

  //Tipo de stream grouping para el nodo
  int _stream_grouping;

  //Estados de un nodo (idle, busy)
  enum node_states {NODE_IDLE=0, NODE_BUSY};
  //Estado del nodo (node_states=NODE_IDLE, NODE_BUSY)
  node_states _node_state;

  //Referencia al procesador que aloja al nodo
  Processor *_processor;

  Node( const string& name,
        const node_type& n_type,
        const uint32_t& rep_id,
        const uint32_t& rep_level,
        const std::string& avg_service_time,
        const int& stream_grouping)
  {
    this->_rep_id = rep_id;
    this->_rep_level = rep_level;
    this->_stream_grouping = stream_grouping;

    _name = name;

    this->_node_id = (name + "_" + std::to_string(_rep_id));
    this->_avg_service_time = avg_service_time;
    this->_node_type = n_type;
  }

  //Reemplaza a metodo inner_body(), procesa la tupla.
  virtual double run( double timestamp = 0.0 );

  virtual double send_tuple( );

  //Asigna el nodo a un processor
  void set_processor( Processor *);

  //Establece conexion del nodo source (this) con el nodo target (Node)
  void add_node( Node* );

  //Agrega/pone la tupla a la cola de salida del source; y entrada del target
  double emit_tuple( shared_ptr<Tupla> );

  double emit_tuple_SHUFFLE_GROUPING( shared_ptr<Tupla> );

  double emit_tuple_FIELD_GROUPING( shared_ptr<Tupla> );

  double emit_tuple_GLOBAL_GROUPING( shared_ptr<Tupla> );

  double emit_tuple_ALL_GROUPING( shared_ptr<Tupla> );

  //Tiempo de servicio (tiempo HOLD)
  std::string avg_service_time( );

  //Retorna el id de replica
  int replica_id( );

  //Retorna el nombre del objeto junto concatenado a su id replica
  string to_string();
  //Retorna el nombre del objeto (Bolt_0 o Spout_1) sin nro de replica.
  string name();

  int get_node_state( );
  void set_node_idle( );
  void set_node_busy( );

  //Mediciones
  double tpo_servicio( );
  void tpo_servicio( double );
  void increment_tpo_servicio( double tpo_servicio );
  uint32_t tuplas_procesadas( );
  void tuplas_procesadas( uint32_t );
  void increment_tuplas_procesadas( uint32_t tup=1 );

protected:
  ~Node(){
  }
};
#endif
