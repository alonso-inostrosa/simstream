#ifndef GENERATOR_H
#define GENERATOR_H

#include "glob.h"
#include "../spout.h"
#include "../processor/processor.h"

#include "../distributions/wrapper.h"

class Generator : public process
{

private:
  //double _tpo_arribo;
  Wrapper _wrapper;

  //Identifica a un generador asignado a un Spout (Gen-Spout relacion 1:1)
  int _rep_id; 

  //Nombre unico de generador
  string _generator_id;

  //Processor que aloja al Spout
  Processor *_processor;

  //Spout al que se le generan eventos de arribo/procesamiento de streams
  std::list<Spout *> _spout_list;
  Spout *_current_spout;

  static uint32_t _generated_tuples;
  static uint32_t _max_tuples;

  std::string _arrival_rate;

protected:
  void inner_body( void );

public:
  Generator( const string& name,
             const int& rep_id,
             const std::string& arrival_rate
           ) : process( name ){

    this->_arrival_rate = arrival_rate;

    //cout << this->to_string() << " this->_arrival_rate=" << this->_arrival_rate << " - arrival_rate=" << arrival_rate << endl;

    //this->_tpo_arribo = this->_wrapper.gen_continuous( this->_tasa );
    
    this->_rep_id = rep_id;

    this->_generator_id = (this->name() + "_" + std::to_string(_rep_id));
  }

  static void set_max_tuples( const uint32_t& m_tuples ){ Generator::_max_tuples = m_tuples; };

  string to_string( ){ return _generator_id;}

  void set_processor(Processor *);

  Spout* next_spout( );

  void add_spout( Spout *);

  ~Generator(){
  }

};
#endif
