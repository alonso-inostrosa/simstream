#include "comm_switch.h"
#include "net_iface.h"

map<std::string/*nombre*/, NetIface *> CommSwitch::_network;

void CommSwitch::inner_body( ){

  while( true ){

    //Si no hay tuplas ni en colas de entrada ni en colas de salida, entonces dormir
    if( !this->obtainInputPackages( ) || !this->packagesAtOutputQueue( ) ){
      _comm_switch_state = COMM_SWITCH_IDLE;
      //cout << this->to_string() << " no tuples to send - time=" << this->time() << endl;
      passivate( );
      continue; //Para que una vez despierto vuelva a intentar obtener las tuplas
    }

    _comm_switch_state = COMM_SWITCH_BUSY;

    hold( 1e-9 );

    for(map<int, list< Package *> >::iterator q_out = _queue_out.begin(); q_out != _queue_out.end(); q_out++){
      if( !q_out->second.empty( ) ){
        //Obtenemos un paquete de la cola de salida (un paquete por cada cola) e inmediatamente se remueve de ella
        Package *p = q_out->second.front( );
        q_out->second.pop_front( );

        //Obtenemos elementos de la IP destino del Paquete.
        //EJ: IP=10.4.2.1 -> [0]=10; [1]=4 (pod); [2]=2 (switch); [3]=2 (host);
        std::vector< uint32_t > ip_parts = Utilities::tokenizeStringToInt( p->tupla()->IP_destino( ), '.' );

        //Decidir forma de ruteo
        switch( _comm_switch_type ){
          case SW_CORE://Solo pone paquetes en otros Switches (SW_AGGREGATION)
            //cout << "SWITCH CORE IP:" << _IP << " - paq destino=" << p->tupla()->IP_destino() << " pod destino=" << ip_parts[1] << " TYPE=" << _comm_switch_type << endl;
            _ports_switch[ ip_parts[ 1 ]/*indica pod destino*/ ]->receivePackage( p );
            break;

          case SW_AGGREGATION://Solo pone paquetes en otros switches (SW_CORE o SW_EDGE)

            //Identificar si paquete va "hacia abajo" en el Fat-Tree (es IP conocida para los EDGE) o no
            if( _pod == ip_parts[ 1 ]){ //Destino esta en el mismo POD, va hacia abajo, usar preffix
              //cout << "SWITCH AGGR IP=" << _IP << " DOWN paq destino=" << p->tupla()->IP_destino()
              //     << " preffix=" << _preffix_routing_table[ ip_parts[2] ]
              //     << " - count_preff=" << _preffix_routing_table.count( ip_parts[2] )
              //     << " iparts[2]=" << ip_parts[2]
              //     << " - count=" << _ports_switch.count( _preffix_routing_table[ ip_parts[2] ] ) << endl;
              _ports_switch[ _preffix_routing_table[ ip_parts[ 2 ]/*subnet o nroSwitch*/ ] ]->receivePackage( p );

            }else{
              //Esta en otro POD, va hacia nivel arriba en el Fat-Tree, usar suffix
              //cout << "SWITCH AGGR IP=" << _IP << " UP paq destino=" << p->tupla()->IP_destino()
              //     << " suffix=" << _suffix_routing_table[ ip_parts[3] ] << " iparts[3]=" << ip_parts[3] << endl;
              _ports_switch[ _suffix_routing_table[ ip_parts[3] ] ]->receivePackage( p );

            }             
            break;

          case SW_EDGE://Pone paquetes en otros switches o en hosts
            //Se trata de un Host local al SW?
            if( _pod == ip_parts[ 1 ] && _sw == ip_parts[ 2 ] ){

              //Si destino paquete es un Host, entonces se pasa el paquete a su interfaz de red
              //cout << "SWITCH EDGE IP:" << _IP << " - DOWN paq destino=" << p->tupla()->IP_destino()
              //     << " host=" << ip_parts[ 3 ] << " output port=" << _preffix_routing_table[ ip_parts[ 3 ]]
              //     << " TYPE=" << _comm_switch_type << endl;
              _ports_host[ _preffix_routing_table[ ip_parts[ 3 ] ]/*output port*/]->receivePackage( p );

            }else{ //Se trata de un host ubicado en otro SW EDGE (de este u otro pod)
              //cout << "SWITCH EDGE IP:" << _IP << " - UP paq destino=" << p->tupla()->IP_destino() << endl;
              //cout << " host=" << ip_parts[ 3 ] << " - output_port=" << _suffix_routing_table[ ip_parts[3] ]
              //     << " to SWITCH AGGR IP:" << _ports_switch[ _suffix_routing_table[ ip_parts[3] ] ]->_IP
              //     << " output port=" << _suffix_routing_table[ ip_parts[ 3 ]]
              //     << " TYPE=" << _comm_switch_type << endl;
              _ports_switch[ _suffix_routing_table[ ip_parts[3] ] ]->receivePackage( p ); 

            }
            break;
        }//end switch
      }//end id
    }//end for

  }//end while true

}

//Devuelve falso si no hay paquetes en las colas de salida
bool CommSwitch::packagesAtOutputQueue( ){
  bool not_empty = false;
  for( auto queue : _queue_out )
    if( !queue.second.empty( ) ){
      not_empty = true;
      break;
    }
  return not_empty; 
}

//Devuelve falso si no hay packages en alguna cola de entrada
bool CommSwitch::obtainInputPackages( ){
  bool not_empty = false;

  //Recorre las colas de entrada
  //Extraer todas las tuplas de cada cola de entrada y asignarla a las colas de salida
  for(map< int ,list< Package *> >::iterator queue = _queue_in.begin(); queue != _queue_in.end(); queue++){
    if( !queue->second.empty( ) ){
      Package *p = queue->second.front( );     //Obtiene la primera tupla de cada cola/puerto de entrada
      _queue_out[ _map_port_names[ p->tupla()->IP_destino( ) ]/*puerto*/ ].push_back( p );   //Pone la tupla en la cola de salida
      //cout << "SW IP=" << this->to_string( ) << " obtain queue key=" << queue->first << " queue size="
      //     << queue->second.size() << endl;
      queue->second.pop_front( );
      not_empty = true;
      //cout << "SW IP=" << this->to_string() << " pack id=" << p->id() << " tupla id="  << p->tupla()->id()
      //     <<  " found in port " << queue->first << endl; 
    }
  }

  return not_empty;
}

void CommSwitch::receivePackage( Package *p ){
  string name = p->_tupla->IP_origen();
  //cout << "RECV SWITCH IP:" <<this->to_string()<< " map_port_name[" << name << "] port=" << _map_port_names[ name ]
  //     << " pack id=" << p->id() << " tupla-id=" << p->tupla()->id() << " received from " << name << endl;
  _queue_in[ _map_port_names[ name ] ].push_back( p );

  //Si switch esta disponible, entonces lo despertamos
  if( _comm_switch_state == CommSwitch::COMM_SWITCH_IDLE )
    activate( );
}

/*
* Realiza la conexion entre el Switch y un host (mediante su interfaz de red)
*/
void CommSwitch::connectToNode( NetIface *net_iface ){
  //Mapeo IP con Iface a la que esta conectada
  _network[ net_iface->name() ] = net_iface;

  //Puerto al que esta conectada la interfaz de red (iface del Host)
  uint32_t connected_to_port = std::stoi((Utilities::tokenizeString( net_iface->to_string( )/*IP*/ ))[3]) - 2;
  _ports_host[ connected_to_port ] = net_iface;

  //cout << "SW IP:" << this->_IP << " connected to net_iface=" << net_iface->name( )
  //     << " at port=" << connected_to_port << endl;
}

void CommSwitch::connectToCommSwitch( int port, CommSwitch * remote_comm_sw ){
  _ports_switch[ port ] = remote_comm_sw;
  //cout << "SW IP:" << this->_IP << ":" << port << " connected to switch IP=" << _ports_switch[ port ]->_IP
  //     << " at port=" << port << endl;
}

/*
* Construye tablas de ruteo de acuerdo a paper "A Scalable, Commodity Data Center Network Architecture"
* Mohamed Al-Fares, et al.
*/
void CommSwitch::createRoutingTables( ){
  //cout << "SW IP=" << this->_IP << " creating routing tables" << endl;
  createPreffixRoutingTable( );
  createSuffixRoutingTable( );
}

/*
* Construye las reglas de ruteo para IPs conocidas (dentro del POD)
* Dado un Pod/Sw/Host de destino, entrega el puerto por donde enviar el paquete.
*/
void CommSwitch::createPreffixRoutingTable( ){
  //cout << "SW IP=" << this->_IP << " creating PREFFIX routing tables" << endl;
  switch( _comm_switch_type ){
    case SW_CORE:
      for( uint32_t pod = 0; pod < _K; pod++ ){
        _preffix_routing_table[ pod ] = pod/*output port*/; //I.e: IP=10.2.0.1 -> Port=2;
        //cout << "SW CORE IP=" << this->_IP << " - preffix POD=" << pod << " - output port=" << pod << endl;
      }
      break;
    case SW_AGGREGATION:
      for( uint32_t subnet /*SW al que estan conectados los hosts*/ = 0; subnet < ( _K / 2 ); subnet++){
         _preffix_routing_table[ subnet ] = subnet; //I.e: IP=10.2.0.x -> Port=0; IP=10.2.1.x -> Port=1
         //cout << "SW AGGREGATION IP=" << _IP << " - switch=" << subnet << " - output port=" << _preffix_routing_table[ subnet ] << endl;
      }
      break;
    case SW_EDGE:
      for( uint32_t i = 0; i < ( _K / 2 ); i++ ){
        _preffix_routing_table[ i + 2 /*10.pod.switch.(i+2)*/] = i /*output port*/;
        //cout << "SW EDGE IP=" << _IP << " - host=" << i+2 << " output port=" << _preffix_routing_table[ i+2 ] << endl;
      }
      break;
  }
}

/*
* Construye las reglas de ruteo de IPs no conocidas (de otro POD)
* Paquetes que "suben" en las capas del Fat-Tree.
*/
void CommSwitch::createSuffixRoutingTable( ){
  //cout << "SW IP=" << this->_IP << " creating SUFFIX routing tables" << endl;
  switch( _comm_switch_type ){
    case SW_CORE:
      //No utiliza
      break;
    case SW_EDGE:
      //for( uint32_t x = (_K/2); x < _K; x++)
        for( uint32_t i = 2; i <= ( _K / 2 ) + 1; i++ ){
          _suffix_routing_table[ i ] = ( (i - 2 + _sw ) % ( _K / 2 ) ) + ( _K / 2 );
          //cout << "SW EDGE IP=" << this->_IP << " host=" << i << " - output port=" << _suffix_routing_table[ i ]<< endl;
        }
      break;
    case SW_AGGREGATION:
      //for( uint32_t x = (_K/2); x < _K; x++)
        for( uint32_t i = 2; i <= ( _K / 2 ) + 1; i++ ){
          _suffix_routing_table[ i ] = ( (i - 2 + _sw ) % ( _K / 2 ) ) + ( _K / 2 );
          //cout << "SW AGGREGATION IP=" << this->_IP << " host=" << i << " - output port=" << _suffix_routing_table[ i ]<< endl;
        }
      break;
  }
}

string CommSwitch::to_string( ){
  return this->_IP;
}
