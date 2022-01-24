#ifndef SPOUT_H
#define SPOUT_H

#include "glob.h"
#include "node.h"

//#include "processor/processor.h"
//class Processor;

class Spout: public Node
{
private:
  //static ifstream fin; //Fuente de mensajes/datos (streams, logs, db, etc).

protected:

  //Nro de tupla actual
  static int _nro_tupla;

public:

  Spout( const string& name,
         const uint32_t& id,
         const uint32_t& rep_level,
         const std::string& avg_service_time,
         const int& stream_grouping
	) : Node( name, Node::SPOUT, id, rep_level, avg_service_time, stream_grouping )
  {
    this->_node_state = NODE_IDLE;

    //cout << "Spout name=" << name << " id=" << id << " dist_avg_service_time=" << avg_service_time << " grouping=" << stream_grouping << endl;

    //tuple_list.clear( );   

    //cout << "Abriendo archivo: " << _trazas << endl;
    //fin.open( _trazas );
    //assert( fin!= NULL );
  }

  double run( double timestamp = 0.0 );
  double send_tuple( );

  ~Spout(){
    //fin.close( );
    //tuple_list.clear( );
  }

};
#endif
