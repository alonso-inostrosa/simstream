#include "node.h"

#include "processor/processor.h"

double Node::run( double timestamp ){

}

double Node::send_tuple(){

}

void Node::set_processor( Processor *processor){
  this->_processor = processor;
}

/**
* Se utiliza para generar la topologia, corresponde a los enlaces de un Node con sus destinos
*/
void Node::add_node( /*handle<Node>*/ Node *_node ){
  this->_node_map_list[ _node->name( ) ].push_back( _node );
  this->_node_map_vector[ _node->name( ) ].push_back( _node );
}
/**

* Deja la tupla en las colas que corresponda de acuerdo al tipo de GROUPING
*/
double Node::emit_tuple( shared_ptr<Tupla> t ){
  //cout << this->to_string( ) << " emitting tuple - tupla_id=" << t->id() << endl;
  switch( _stream_grouping ){
    case SHUFFLE_GROUPING:
      return emit_tuple_SHUFFLE_GROUPING( t );
      break;
    case FIELD_GROUPING:
      return emit_tuple_FIELD_GROUPING( t );
      break;
    case GLOBAL_GROUPING:
      return emit_tuple_GLOBAL_GROUPING( t );
      break;
    case ALL_GROUPING:
      return emit_tuple_ALL_GROUPING( t );
      break;
    default:
      return 1e-9;
      break;
  }
}

//Realiza RR sobre los elementos de destino
double Node::emit_tuple_SHUFFLE_GROUPING( shared_ptr<Tupla> tupla ){
  //ITERAR SOBRE EL MAPA para enviar a una replica de cada Bolt de destino
  //DUPLICAR TUPLA SI HAY MAS DE UN DESTINO???? SI! Para evitar que quede con el ultimo
  //destino asignado (dado que es shared_ptr. CREAR COPIAS!!
  uint32_t i = 0;
  for ( std::map< std::string, list< Node *> >::iterator it=_node_map_list.begin(); it!=_node_map_list.end(); ++it){
    //cout << this->to_string( ) << " emit tuple - tupla_id=" << tupla->id() << " - choosing node from available=" << it->second.size() << endl;

    //Copiamos una nueva tupla para cada destino
    shared_ptr<Tupla> t = make_shared<Tupla>( *tupla );

    //Obtenemos el nodo (de forma RR) y se envia al final de la lista.
    Node* temp_node = it->second.front( );
    it->second.pop_front( );
    it->second.push_back( temp_node );

    //Modificamos el ID de envio entre el Spout/Bolt origen y el Bolt destino
    std::string id_tramo_tupla = ( this->to_string() + "_" + temp_node->to_string() + "_" + std::to_string( _nro_tupla_local ) + "_" + std::to_string( tupla->copia() ) + "_" + std::to_string( i ) );
    t->id_tramo( id_tramo_tupla );
    
    //cout << this->to_string( ) << " (RR) sending tuples to " << temp_node->to_string() << " - tupla_id="  << t->id() << " to IP=" << temp_node->_processor->_IP << " tramo=" << t->id_tramo() <<" copia=" << tupla->copia() << endl; 

    //NOTA: Se duplica (porque ahora puede tener varios destinos cuando hay un fork)
    t->nro_tupla( this->_nro_tupla_local ); //Utilizado para diferenciar una tupla de otra.
    t->id_destino( temp_node->to_string( ) );
    t->IP_destino( temp_node->_processor->_IP );

    t->id_origen( this->to_string() );
    t->IP_origen( this->_processor->_IP );
 
    //Pide al Processor que aloja al Spout/Bolt actual que ponga la tupla en el Bolt de destino.
    this->_processor->send_tuple( t/*upla*/ );

    this->_nro_tupla_local++;
    i++;
  }

  //Procesadores de destino son encargados de despertar a los que reciben las tuplas
  return 1e-9;
}

double Node::emit_tuple_FIELD_GROUPING( shared_ptr<Tupla> tupla ){

  for (std::map<std::string,vector<Node*>>::iterator it=_node_map_vector.begin(); it!=_node_map_vector.end(); ++it){
    uint32_t hash = Utilities::hash( tupla->contenido( ) );
    uint32_t selected_node = hash % it->second.size( );
    Node* temp_node = it->second.at( selected_node );

    //Copiamos una nueva tupla para cada destino (hay fork)
    shared_ptr<Tupla> t = make_shared<Tupla>( *tupla );

    //Modificamos el ID de envio entre el Spout/Bolt origen y el Bolt destino
    std::string id_tramo_tupla = ( this->to_string() + "_" + temp_node->to_string() + "_" + std::to_string( _nro_tupla_local ) + "_" + std::to_string( tupla->copia() ) );
    t->id_tramo( id_tramo_tupla );

    t->nro_tupla( this->_nro_tupla_local ); //Utilizado para diferenciar una tupla de otra.
    t->id_destino( temp_node->to_string( ) );
    t->IP_destino( temp_node->_processor->_IP );

    t->id_origen( this->to_string() );
    t->IP_origen( this->_processor->_IP );

    //cout << this->to_string( ) << " (HASH) sending tuples to " << temp_node->to_string()
    //     << " - tupla_id="  << t->id() << " to IP=" << temp_node->_processor->_IP
    //     << " tramo=" << t->id_tramo() << " tupla_local=" << t->nro_tupla() << endl;

    //Pide al Processor que aloja al Spout/Bolt actual que ponga la tupla en el Bolt de destino.
    this->_processor->send_tuple( tupla );

    this->_nro_tupla_local++;
  }
  return 1e-9;
}
double Node::emit_tuple_GLOBAL_GROUPING( shared_ptr<Tupla> tupla ){
  return 1e-9;
}

double Node::emit_tuple_ALL_GROUPING( shared_ptr<Tupla> tupla ){
  /*for( auto node : _node_map_list ){
    //Pide al Processor que aloja al Spout/Bolt actual que ponga la tupla en el Bolt de destino.
    this->_processor->send_tuple( node->to_string( ), tupla );


  }*/
  return 1e-9;
}

//Retorna el tiempo de servicio (tiempo HOLD)
std::string Node::avg_service_time( ){
  return this->_avg_service_time;
}

//Retorna el id de replica
int Node::replica_id( ){
  return _rep_id;
}

//Retorna el nombre del objeto junto concatenado a su id replica
std::string Node::to_string(){
  return this->_node_id;
}

std::string Node::name(){
  return this->_name;
}

int Node::get_node_state( ){
  return this->_node_state;
}

void Node::set_node_idle( ){
  this->_node_state = Node::NODE_IDLE;
}

void Node::set_node_busy( ){
  this->_node_state = Node::NODE_BUSY;
}

double Node::tpo_servicio( ){
  return this->_tpo_servicio;
}
void Node::tpo_servicio( double tpo_servicio ){
  this->_tpo_servicio = tpo_servicio;
}
void Node::increment_tpo_servicio( double tpo_servicio ){
  this->_tpo_servicio += tpo_servicio;
}
uint32_t Node::tuplas_procesadas( ){
  return this->_tuplas_procesadas;
}
void Node::tuplas_procesadas( uint32_t tuplas_procesadas ){
  this->_tuplas_procesadas = tuplas_procesadas;
}
void Node::increment_tuplas_procesadas( uint32_t tup ){
  this->_tuplas_procesadas += tup;
}
