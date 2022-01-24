#ifndef NET_IFACE_H
#define NET_IFACE_H

#include "../glob.h"

#include "comm_switch.h"

class Processor;
//class CommSwitch;

class NetIface
{
private:

  //Procesador en que se ubica la Tarj. Red
  Processor *_processor;

  string _name; 

  handle<CommSwitch> _comm_switch;

  std::map< std::string/*tupla key*/, uint32_t/*paquetes arribados*/> packages;

public:
  NetIface( ){
  }

  NetIface( const string& name ){
    this->_name = name;
  }

  //Accessors
  Processor* processor();
  void processor( Processor * );

  string to_string();
  string name( );
  void name( const string& );

  /*
  * Realiza el envio de una tupla a un destino (destino indicado en la misma tupla)
  */
  void sendMessage( shared_ptr<Tupla> );

  /*
  * Realiza la recepcion de paquetes. Cuando han llegado todas las partes de una tupla
  * entonces entrega la tupla correspondiente al procesador para ser procesada por un bolt.
  */
  void receivePackage( Package * );

  //Registra la tarjeta de red de un procesador en la Red
  //void addProcessorToNetwork( string, NetIface *);

  /*
  * Convierte una tupla a paquetes (segun su tamanio, tambien indicado en la misma tupla)
  */
  std::list<Package *> splitTupleIntoPackages( shared_ptr<Tupla> );

  void connectToCommSwitch( handle<CommSwitch> );
};
#endif
