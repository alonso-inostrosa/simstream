#include "core.h"

#include "processor.h"
//#include "../node.h"
//#include "../bolt.h"

double Core::SIM_TIME = 0.0;

void Core::inner_body( ){

  //Indica si los nodos de la cola estan marcados como ocupados (por otro core)
  bool _no_nodes_to_process = false;

  while( true ){
    //cout << this->to_string( ) << " (@" << this->_processor->to_string() << ") begin new loop - time()=" << time() << endl;

    //Revisamos si hay algun Spout/Bolt que necesite procesamiento.
    if( this->_processor->nodes_queue_empty( ) || _no_nodes_to_process ){
      this->_state = CORE_IDLE;
      //cout << this->to_string( ) << " (@" << this->_processor->to_string() << ") nothing to process - time()=" << time() << endl;
      SIM_TIME = this->time();
      passivate( );
    }

    //cout << this->to_string( ) << " getting node to process (qty left=" << this->_processor->nodes_queue_size()  << ") time()=" << time() << endl;

    //Obtenemos el Bolt/Spout que demanda servicio
    Node *node;
    if( !this->_processor->get_next_node( &node ) ){
      //cout << this->to_string() << " can't get IDLE nodes - time=" << time() << endl;
      _no_nodes_to_process = true;
      continue;
    }

    this->_state = CORE_BUSY;
    _no_nodes_to_process = false;
    node->set_node_busy( );

    //cout << this->to_string( ) << " (@" << this->_processor->to_string() << ") starting processing of node " << node->to_string() << " - time=" << time()  << endl;

    double tpo_hold;
    Node::node_type n_type = node->_node_type;
    switch( n_type ){
      case Node::SPOUT:
        //Do whatever a Spout must do
        //cout << this->to_string( ) << " (@" << this->_processor->to_string() << ") running( " << node->to_string() << " ) - time=" << time() << endl;

        //Mediciones
        tpo_hold = node->run( this->time( ) );
        this->_in_use += tpo_hold;
        this->_processor->increment_in_use( tpo_hold );
        //Probando/revisando Tiempos
        //cout << this->to_string() << " HOLDTIME " << tpo_hold << " time " << this->time() << " proc "
        //     << this->_processor->to_string() << " core " << this->to_string()   
        //     << " NODE " << node->to_string () << " tpoServicio " << node->tpo_servicio() << endl;
        hold( tpo_hold );

        //cout << this->to_string( ) << " (@" << this->_processor->to_string() << ") sending tuples from " << node->to_string() << " - time=" << time() << endl;

        //Mediciones
        tpo_hold = node->send_tuple( );
        this->_in_use += tpo_hold;
        this->_processor->increment_in_use( tpo_hold );

        //Probando/revisando Tiempos
        //cout << this->to_string() << " HOLDTIME " << tpo_hold << " time " << this->time() << " proc "
        //     << this->_processor->to_string() << " core " << this->to_string()   
        //     << " NODE " << node->to_string () << " tpoServicio " << node->tpo_servicio() << endl;
        hold( tpo_hold );

        break;
      case Node::BOLT:
        //Do whatever a Bolt should do
        node->_node_state = Node::NODE_BUSY;
        //cout << this->to_string( ) << " (@" << this->_processor->to_string() << ") running( " << node->to_string() << " ) - time=" << time() << endl;
        tpo_hold = node->run( this->time( ) );

        //Mediciones
        this->_in_use += tpo_hold; 
        this->_processor->increment_in_use( tpo_hold );

        hold( tpo_hold );
        //cout << this->to_string( ) << " (@" << this->_processor->to_string() << ") sending tuples from " << node->to_string() << " - time=" << time() << endl;
        tpo_hold = node->send_tuple( );
        //cout << this->to_string( ) << " (@" << this->_processor->to_string() << ") tuples sent but not HOLD " << node->to_string() << " - time=" << time() << endl;

        //Mediciones
        this->_in_use += tpo_hold; 
        this->_processor->increment_in_use( tpo_hold );

        hold( tpo_hold );

        //Remove Tuple from Ram Memory
        this->_processor->removeTuple( ((Bolt*)node)->get_current_tuple() );

        node->_node_state = Node::NODE_IDLE;

        //cout << this->to_string( ) << " (@" << this->_processor->to_string() << ") HOLD DONE - time=" << time() << endl;
        break;
    }
    node->set_node_idle( );

    //cout << this->to_string( )<< " (@" << this->_processor->to_string() << ") ending processing of node " << node->to_string() << " - time=" << time() << endl;

  }
}

void Core::get_node_from_processor( ){
  //this->_processor->get_next_node( this->_node );
}

/*
* enlaza a procesador que lo contiene
*/
void Core::set_processor( Processor *processor){
  this->_processor = processor;
}

std::string Core::to_string(){
  return _name;
}
