#include "generator.h"

uint32_t Generator::_generated_tuples = 0;
uint32_t Generator::_max_tuples = 0;

void Generator::inner_body( void ){

  //cout << this->to_string() << " entering inner_body" << endl;

  while(true){
      //Verificamos si debemos seguir generando tuplas
      if( _generated_tuples > _max_tuples ){
        //cout << "GENERATOR TUPLES GENERATED:" << _generated_tuples << " MAX:" << _max_tuples << " - DONE!" << endl;
        passivate( );
      }

      //cout << this->to_string() << " before hold - time=" << this->time() << endl;

      //Incrementamos contador de generacion de tuplas
      _generated_tuples++;

      double arrival_rate = this->_wrapper.gen_continuous( this->_arrival_rate );
		std::cout << "ARRIVAL_RATE " << arrival_rate << std::endl;
      hold( arrival_rate );
      //cout << this->to_string() << " after hold - time=" << this->time() << endl;

      //cout << this->to_string() << " before getting next spout (amount=" << _spout_list.size()
      //     << ") - time=" << this->time() << endl;

      _current_spout = this->next_spout( );
      //cout << this->to_string() << " schedulling processing of spout " << _current_spout->to_string() << " at processor " << this->_processor->to_string() << " - time=" << this->time() << endl;
      this->_processor->schedule_spout_processing( _current_spout->to_string() );
  }
}

void Generator::set_processor( Processor *proc){
  this->_processor = proc;
  //cout << this->to_string( ) << " processor added proc=" << proc->to_string() << " IP=" << proc->get_IP() << endl;
}

void Generator::add_spout( Spout *spout){
  this->_spout_list.push_back( spout );
  //cout << this->to_string( ) << " spout " << spout->to_string() << " - total spouts at generator=" << _spout_list.size() << endl;
}

Spout* Generator::next_spout( ){
  Spout *spout = _spout_list.front( );
  _spout_list.pop_front( );
  _spout_list.push_back( spout );
  //cout << this->to_string() << " returning next spout " << spout->to_string() << " - time=" << this->time() << endl;
  return spout;
}
