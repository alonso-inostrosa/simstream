#include "processor.h"

#include "../node.h"
#include "../spout.h"
#include "../bolt.h"


//Extrae y retorna el siguiente nodo que requiere procesamiento, llamado por Core
bool Processor::get_next_node( Node **node_address ){

  bool resp = false;
  list<Node*> auxiliary_list;

  //Extrae nodo a nodo hasta encontrar el primero que no este en uso (NODE_IDLE), mientras los guarda en list auxiliar
  while( !_nodes_queue.empty( ) ){
    //Extrae el nodo
    Node *node = _nodes_queue.front( );
    _nodes_queue.pop_front( );

    //Verifica que no este en uso (en ejecucion en algun otro core)
    if( node->get_node_state() == Node::NODE_IDLE ){
      //cout << this->to_string() << " obtaining node " << node->to_string() << " from queues_node" << endl;
      resp = true;
      *node_address = node;  //Es un puntero al nodo (Spout/Bolt) que se debe ejecutar a continuacion en el core.
      break;
    }else{
      auxiliary_list.push_back( node );
    }
  }

  //Repone los nodos en la lista
  while( !auxiliary_list.empty() ){
    Node *aux_node = auxiliary_list.back( );
    _nodes_queue.push_front( aux_node );
   auxiliary_list.pop_back( );
  }
  return resp;
}

//Sobrecarga del metodo
//Envia fisicamente la tupla al procesador de destino del Spout/Bolt
void Processor::send_tuple( string bolt_id, shared_ptr<Tupla> tupla ){

  //cout << this->to_string( ) << " sending tuples to " << bolt_id << " - tupla_id=" << tupla->id() << endl;
  this->send_tuple( tupla );
}


//Envia "fisicamente" la tupla al procesador de destino del Spout/Bolt
void Processor::send_tuple( shared_ptr<Tupla> t ){
  //cout << this->to_string( ) << " sending tuple to " << t->id_destino( ) << " - tupla_id=" << t->id() << endl;

  this->_net_iface->sendMessage( t );

  //cout << this->to_string( ) << " tuple sent to " << t->id_destino( ) << endl;
}


//Agrega tupla para ser procesada por Spout/Bolt en algun Core del Processor actual
//Y agenda la ejecucion del Bolt en algun core
void Processor::insert_tuple( shared_ptr<Tupla> t ){
  //Inserta la tupla en cola para que Bolt la extraiga
  this->_tuple_queue[ t->id_destino() ].push_back( t );
  //cout << this->to_string( ) << " tuple inserted" << endl;

  //Tambien reservamos y asignamos espacio de memoria en que se almacena la Tupla
  this->malloc( t->tamanio() );  //Reserva espacio en memoria
  this->storeTuple( t );         //Guarda efectivamente la tupla

  //Ahora se inserta al Bolt en la cola para que algun Core lo ejecute.
  //cout << this->to_string( ) << " choosing bolt=" << t->id_destino() << " - to process tuple - tupla_id="
  //     << t->id() << endl;
  Bolt *bolt = _bolts[ t->id_destino() ];

  //Tupla es smart_ptr, por lo que no es necesario llevar contador de referencias para decidir su borrado

  _nodes_queue.push_back( ( Node*) bolt );

  //cout << this->to_string( ) << " bolt to process tuple pushed=" << bolt->to_string() << endl;

  //Despertar a algun Core para que el Spout/Bolt procese la tupla
  this->activate_core( );
}

//Agenda la ejecucion del bolt en algun core del procesador que lo aloja
void Processor::schedule_spout_processing( string spout_name ){
  //cout << this->to_string() << " schedulling spout " << spout_name << " to be executed" << endl;
  _nodes_queue.push_back( ( Node * ) _spouts[ spout_name ] );

  this->activate_core( );
}

//activa el primer core que encuentre libre, sino quiere decir que estan
//todos en uso, por lo que seguiran extrayendo elementos de _node_queue
void Processor::activate_core( ){
  for( unsigned int i = 0; i < this->_cores.size( ); i++ ){
    handle<Core> c = this->_cores.front();
    this->_cores.pop_front();
    this->_cores.push_back( c );
    if( c->_state == Core::CORE_IDLE ){
      //cout << this->to_string() << " activating core " << c->to_string() << " (state=" << c->_state << ")" << endl;
      c->activate();
      break;
    }//else
      //cout << this->to_string() << " core already activated " << c->to_string() << " (state=" << c->_state << ")" << endl;
  }
}

//Nodo realiza pull de la tupla desde la cola
shared_ptr<Tupla> Processor::pull_tuple( string id /*nombre del Spout/Bolt*/ ){
  shared_ptr<Tupla> t = this->_tuple_queue[ id ].front( );
  this->_tuple_queue[ id ].pop_front();
  return t;
}

//Indica la cantidad de Spouts/Bolts que requieren servicio en algun Core del Processor
int Processor::nodes_queue_size( ){
  return _nodes_queue.size( );
}

//Indica si hay Spout/Bolts que requieren servicio en algun Core del Processor
bool Processor::nodes_queue_empty(){
  return _nodes_queue.empty( );
}

//Indica si la cola de tuplas del Bolt esta vacia.
bool Processor::tuple_queues_empty( string id ){
  return _tuple_queue[ id ].empty();
}

int Processor::tuple_queues_size_for_bolt( string id ){
  return _tuple_queue[ id ].size();
}

//Guardamos los Bolts en un mapa (separado de los Spouts)
void Processor::assign_bolt( Bolt *bolt ){
  //Solicitar espacio de memoria para el Bolt
  this->malloc( /*bolt->_memory_size*/ 100 );
  this->_bolts[ bolt->to_string() ] = bolt;
  //this->_node_processor[ (bolt)->to_string() ]=this;  //Registra la ubicacion del Bolt en el procesador (static)
  //this->_net_iface->addProcessorToNetwork( (bolt)->to_string(), this->_net_iface );
}

//Guardamos los Spouts en un mapa diferente a los Bolts
void Processor::assign_spout( Spout *spout ){
  this->malloc( /*Spout size*/ 100 );
  this->_spouts[ (spout)->to_string() ] = spout;
  //this->_node_processor[ (spout)->to_string() ]=this; //Registra Spout en el processor (static) otros proc lo saben
  //this->_net_iface->addProcessorToNetwork( (spout)->to_string(), this->_net_iface );
}

//Solicita espacio de memoria
void Processor::malloc( uint32_t required_space ){
  bool free_space = this->_ram_memory.malloc( required_space );
  assert ( free_space );
}

//Almacena una tupla
void Processor::storeTuple( shared_ptr<Tupla> t ){
  this->_ram_memory.storeTuple( t );
}

//Remueve una tupla
void Processor::removeTuple( shared_ptr<Tupla> t ){
  this->_ram_memory.removeTuple( t );
  this->_ram_memory.free( t->tamanio() );
}

double Processor::average_memory_consumption( ){
  return this->_ram_memory.average_use();
}

uint32_t Processor::max_memory_consumption( ){
  return this->_ram_memory.max_memory_consumption( );
}

string Processor::to_string( ){
  return "Proc_" + _name;
}

string Processor::get_IP( ){
  return _IP;
}

void Processor::increment_in_use( double tpo ){
  this->_in_use += tpo;
}
double Processor::in_use( ){
  return this->_in_use;
}
void Processor::in_use( double tpo ){
  this->_in_use = tpo;
}

std::list<handle<Core>> Processor::cores(){
  return this->_cores;
}
