#ifndef BOLT_H
#define BOLT_H

#include "glob.h"
#include "node.h"

//#include "processor/processor.h"
//class Processor;

class Bolt: public Node
{
public:
  //Distribucion que indica si el Bolt emite un tweet, multiples o los filtra
  std::string _nbr_output_tuples;

protected:
  void inner_body( void ){;}

public:
  Bolt( const string& name,
        const uint32_t& id,
        const uint32_t& rep_level,
        const std::string& avg_service_time, 
        const std::string& nbr_output_tuples,
        const int& stream_grouping
	) : Node( name, Node::BOLT, id, rep_level, avg_service_time, stream_grouping )
  {
    this->_node_state = NODE_IDLE;
    this->_nbr_output_tuples = nbr_output_tuples;

    //cout << "Bolt name=" << name << " id=" << id << " dist_avg_service_time=" << avg_service_time
    //     << " grouping=" << stream_grouping << " dist_output_tuples=" << _nbr_output_tuples << endl;
  }

  double run( double timestamp = 0.0 );
  double send_tuple( );

  //Procesa la tupla (la puede modificar) y la retorna con el tiempo de hold
  double/*hold*/ process_tuple( shared_ptr<Tupla> );

  //Realiza un pull de la cola del Processor para obtener una tupla.
  shared_ptr<Tupla> pull_tuple( );

  shared_ptr<Tupla> get_current_tuple( );

  ~Bolt(){
    //node_list.clear( );
  }
};
#endif
