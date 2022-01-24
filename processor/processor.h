#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "../glob.h"

#include "../network/net_iface.h"

#include "core.h"
#include "generic_memory.h"

class Node;
class Bolt;
class Spout;

class Processor
{
public:
  int _pid;   //Id del procesador
  std::string _IP; //IP del procesador 

  //Para mediciones
  double _in_use = 0.0;

  //Cola con los Spout/Bolt que requieren servicio
  std::list<Node *> _nodes_queue;

  //Tarjeta de Red
  NetIface *_net_iface;

  //Memoria Ram
  RamMemory _ram_memory;
 
private:
  //Almacena las tuplas recibidas para cada Bolt - (local o desde otros procesadores)
  std::map<std::string/*target_Bolt_id*/,std::list<shared_ptr<Tupla>>/*tupla generada/procesada por un spout/bolt*/> _tuple_queue;

  //Spouts o bolts que seran ejecutados
  std::list<handle<Core>> _cores;

public:
  //Spouts alojados en el procesador
  std::map<std::string,Spout *> _spouts;

  //Bolts alojados en el procesador
  std::map<std::string,Bolt *> _bolts;

  std::string _name;

public:
  Processor( const std::string& name, int cant_cores, NetIface *net_iface, uint64_t size ) : _ram_memory( name, size ){
    _name = name;
    _IP = name;
    for( int i = 0; i < cant_cores; i++){
      handle<Core> core = new Core("Core",i);
      _cores.push_back( core );
     core->set_processor( this );
    }
    this->_net_iface = net_iface;
    this->_net_iface->processor( this );

  }

  //Obtiene el siguiente Spout/Bolt que requiere servicio en un Core.
  bool get_next_node( Node ** );

  //Realiza el envio fisico de una tupla a otro processor
  void send_tuple(std::string, shared_ptr<Tupla>);

  //Realiza el envio de una tupla a otro procesador
  void send_tuple( shared_ptr<Tupla> );

  //Agrega tupla recibida desde otro procesador.
  void insert_tuple( shared_ptr<Tupla> );

  //Agenda la ejecucion de un spout en algun Core del Processor
  void schedule_spout_processing( string );

  //Activa el primer core que encuentre libre, sino quiere decir que estan
  //todos en uso, por lo que seguiran extrayendo elementos de _node_queue
  void activate_core( );

  //Indica la cantidad de Spouts/Bolts que requieren servicio en algun Core del Processor
  int nodes_queue_size( );

  //Indica si hay Spout/Bolts que requieren servicio en algun Core del Processor
  bool nodes_queue_empty();

  //Indica si la cola de tuplas del Bolt esta vacia.
  bool tuple_queues_empty( std::string );

  int tuple_queues_size_for_bolt( std::string );

  //Nodo realiza pull de la tupla desde la cola
  shared_ptr<Tupla> pull_tuple( std::string );

  //Guardamos los Bolts en un mapa (separado de los Spouts)
  void assign_bolt( Bolt * );

  //Guardamos los Spouts en un mapa diferente a los Bolts
  void assign_spout( Spout * );

  //Reserva espacio de memoria
  void malloc( uint32_t );

  //Almacena una tupla en memoria Ram
  void storeTuple( shared_ptr<Tupla> t );

  //Remueve una copia de la tupla desde memoria Ram
  void removeTuple( shared_ptr<Tupla> t );

  //Espacio promedio de uso de memoria
  double average_memory_consumption( );

  //Espacio maximo usado de memoria
  uint32_t max_memory_consumption( );

  string to_string( );

  string get_IP( );

  void increment_in_use( double tpo );
  double in_use( );
  void in_use( double tpo );

  std::list<handle<Core>> cores();
};
#endif
