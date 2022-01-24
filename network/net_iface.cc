#include "net_iface.h"
#include "../processor/processor.h"

//map<std::string/*nombre*/, NetIface *> NetIface::_network;

void NetIface::sendMessage( shared_ptr<Tupla> t ){

  //La tupla se separa en paquetes, los que son agregados al comm_switch
  std::list< Package * > package_list = splitTupleIntoPackages( t );
  while( !package_list.empty() ){
    Package *p = package_list.front();
    package_list.pop_front();
    //cout << this->to_string() << " - pack id=" << p->id() << " sending tuple id=" << p->tupla()->id() << " to " << t->id_destino( ) << " nro paqs=" << p->nro_paquetes()<< endl;
    (_comm_switch)->receivePackage( p );
  }

  //Si switch esta disponible, entonces lo despertamos
  if( (_comm_switch)->_comm_switch_state == CommSwitch::COMM_SWITCH_IDLE )
    (_comm_switch)->activate( );
}

/**
* Recibe paquetes desde el comm_switch
*/
void NetIface::receivePackage( Package *p ){
  //Aumentamos la cantidad de paquetes arribados para la version de la tupla
  uint32_t tupla_id       = p->tupla()->id();
  std::string tupla_tramo = p->tupla()->id_tramo();
  uint32_t tupla_nro      = p->tupla()->nro_tupla();
  uint32_t tupla_copia    = p->tupla()->copia();
  std::string tupla_key = ( std::to_string( tupla_id ) + "_" + tupla_tramo + "_" + std::to_string( tupla_nro ) + "_" + std::to_string( tupla_copia ) );
  packages[ tupla_key ]++;

  //cout << to_string() << " package received pack id=" << p->id() << " - t->id=" << p->tupla()->id()
  //     << " - arrived=" << packages[ tupla_key ] << " - total=" << p->nro_paquetes( )
  //     << " tupla_id=" << tupla_id << " tramo=" << tupla_tramo << " tupla_nro=" << tupla_nro
  //     << " copia=" << tupla_copia << endl;

  //si han llegado todos los paquetes, entonces se pasa la tupla al processor
  if( packages[ tupla_key ] == p->nro_paquetes() ){
    //cout << to_string() << " all packages received for tuple id=" << p->tupla()->id() << endl;
    shared_ptr<Tupla> t = p->tupla();
    this->_processor->insert_tuple( t );

    //Si han llegado todos los paquetes que componen la tupla, entonces se borra la entrada, porque si la tupla es enviada nuevamente
    //a algun bolt en esta netIface se debe volver a contar los paquetes que arrivan.
    packages.erase( tupla_key );
  }

  delete p; //Borramos el package
}

/*
* Convierte una tupla a una lista de paquetes
*/
std::list<Package *> NetIface::splitTupleIntoPackages( shared_ptr<Tupla> t ){
  std::list<Package *> pack_list;
  for( uint32_t i = 0; i<(t->tamanio()/MTU);i++){
    Package *p = new Package( i, uint32_t (t->tamanio()/MTU), t);
    pack_list.push_back( p );
  }
  return pack_list;
}

/*void NetIface::addProcessorToNetwork( string id, NetIface *net){
  this->_network[ id ] = net;
}*/

void NetIface::connectToCommSwitch( handle<CommSwitch> comm_sw ){
  _comm_switch = comm_sw;
  //cout << this->to_string() << " connected to " << (_comm_switch)->to_string() << endl;
}

//Accessors
Processor* NetIface::processor(){
  return this->_processor;
}

void NetIface::processor( Processor *processor){
  this->_processor = processor;
}

string NetIface::to_string( ){
  return ( this->name( ) );
}

string NetIface::name( ){
  return this->_name;
}

void NetIface::name( const string& name ){
  this->_name = name;
}
