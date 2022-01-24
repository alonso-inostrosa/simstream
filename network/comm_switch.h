#ifndef COMM_SWITCH_H
#define COMM_SWITCH_H

#include "../glob.h"

class NetIface;

class CommSwitch : public process
{

public: 
  enum comm_switch_states {COMM_SWITCH_IDLE=0, COMM_SWITCH_BUSY};
  enum comm_switch_types {SW_CORE = 0, SW_AGGREGATION, SW_EDGE};

  //Estado del Switch
  comm_switch_states _comm_switch_state;

  //Tipo de Sw (Core. Aggregation, Edge), de acuerdo a esto, a _pod y _sw es el ruteo que se define.
  comm_switch_types _comm_switch_type;

  //Cantidad de Puertos (K)
  uint32_t _K;

  double _latency;

  //IP del Switch, Pod en el que esta ubicado, nro de SW.
  std::string _IP;
  uint32_t _pod;
  uint32_t _sw;
  
  std::map< int /*sw*/, int /*output port*/ > _preffix_routing_table;
  std::map< int /*host(remote)*/, int /*output pod/port*/ > _suffix_routing_table;


  static std::map<std::string/*IP*/, NetIface *> _network;

  //Contiene conexiones a otros Switches y a los hosts (placa de red de las maquinas)
  std::map< int/*port_number*/, CommSwitch * /*Switch*/> _ports_switch;
  std::map< int/*port_number*/, NetIface *> _ports_host;

  std::map<std::string/*IP*/, int/*port*/> _map_port_names;

  std::map<int/*port*/, list< Package *> > _queue_out;

  std::map<int/*port*/, list< Package *> > _queue_in;

protected:
  void inner_body( void );

public:
  CommSwitch( const string& name, const uint32_t& K, const string& ip, const CommSwitch::comm_switch_types& comm_sw_type ) : process( name ){
    _IP = ip;

    _comm_switch_state = CommSwitch::COMM_SWITCH_IDLE;

    _K = K;

    //Para fines de ruteo
    //IP=10.pod.sw.1
    std::vector<string> ipParts =(Utilities::tokenizeString( _IP ));
    _pod = std::stoi(ipParts[1]); //pod
    _sw = std::stoi(ipParts[2]);  //sw

    //Para saber si es Core, Aggregation o Edge
    _comm_switch_type  = comm_sw_type;

    //Crea tablas de ruteo preffix y suffix
    createRoutingTables( );
  }

  //Devuelve false si no hay tuplas en ninguna cola de salida
  bool packagesAtOutputQueue( );

  //Recupera la primera tupla de las colas de entrada y las deja en la cola de salida de cada puerto.
  bool obtainInputPackages( );

  //Permite que interfaces de red pongan tuplas en las colas de entrada asociadas a cada puerto de entrada.
  void receivePackage( Package * );

  string IP(){ return _IP; }
  void IP( string _IP ){ this->_IP = _IP; }

  void connectToNode( NetIface * );

  void connectToCommSwitch( int /*port*/, CommSwitch * );

  void createRoutingTables( );
  void createPreffixRoutingTable( );
  void createSuffixRoutingTable( );

  string to_string();
};

#endif
