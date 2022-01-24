#include "spout.h"

//Es STATIC para que todos los generadores utilicen nros consecutivos diferentes de tuplas (id).
int Spout::_nro_tupla=0;

double Spout::run( double timestamp ){
  //cout << this->to_string() << " stream processed" << endl;
 
  //Id de tupla consta de NombreSpout_NroTupla
  _tupla = make_shared<Tupla>( _nro_tupla );

  //Actualiza timestamp de la tupla
  _tupla->timestamp( timestamp );
  //cout << this->to_string() << " - tupla CREATED - tupla_id=" << _tupla->id() << " tramo=" << _tupla->id_tramo() << endl;

  _nro_tupla++;

  //Genera el tiempo de servicio para este requerimiento
  double avg_s_time = _wrapper.gen_continuous( this->_avg_service_time );

  //Para medir tiempos y throughput
  this->increment_tpo_servicio( avg_s_time );
  this->increment_tuplas_procesadas( );

  return avg_s_time;
}

double Spout::send_tuple( ){
  //cout << this->to_string( ) << " about to send tuple" << endl;
  double tpo_emit = this->emit_tuple( _tupla );
  //cout << this->to_string( ) << " tupla EMITTED - tupla_id=" << _tupla->id() <<  endl;
  return tpo_emit;
}

