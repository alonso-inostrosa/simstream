#include "bolt.h"
#include "processor/processor.h"

double Bolt::run( double timestamp ){
  //ASSERT para evitar que no haya tuplas que procesar por el Bolt
  //cout << this->to_string( ) << " pulling tuple - tuple_queue_size()=" << _processor->tuple_queues_size_for_bolt( this->to_string() ) << endl; 
  assert( !_processor->tuple_queues_empty( this->to_string() ) );

  //Extrae (pull) tupla de la cola.
  _tupla = this->pull_tuple( );
  //cout << this->to_string() << " - tupla PULLED - tupla_id=" << _tupla->id() << endl;

  //Actualiza timestamp de la tupla
  _tupla->timestamp( timestamp );

  //Devuelve tupla procesada y tiempo de Hold
  double tpo = process_tuple( _tupla );
  std::cout << "SERVICE_TIME " << this->to_string() << " " << tpo << std::endl;
	
  //Para medir tiempos y throughput
  this->increment_tpo_servicio( tpo );
  this->increment_tuplas_procesadas( );

  //cout << this->to_string() << " - tupla PROCESSED - tupla_id=" << _tupla->id() << endl;

  return tpo;
}

double Bolt::send_tuple( ){

/**
* emit_tuple alojara las tuplas procesadas en la(s) colas que correspondan (nodo source).
* nodo source despues debera despertar al nodo target para que este haga el pull.
* process_tuple target debera hacer pull de la cola, luego de ser despertado por source.
*/

  uint32_t nroTuplasEnviar = _wrapper.gen_discrete( this->_nbr_output_tuples );
  std::cout << "NUMBER_OF_TUPLES " << this->to_string() << " " << nroTuplasEnviar << std::endl;

  if( nroTuplasEnviar != 0 ){
    double tpo_emit = 0.0;
    for(uint32_t i = 0; i < nroTuplasEnviar; i++){
      shared_ptr<Tupla> t = make_shared<Tupla>( *_tupla );
      t->copia( i );
      tpo_emit += this->emit_tuple( t );
      //cout << this->to_string( ) << " tupla EMITTED - tupla_id=" << t->id() << " id_tramo=" << t->id_tramo() << " nro_local=" << t->nro_tupla() << " copia=" << t->copia() << " nroTuplas=" << nroTuplasEnviar << endl;
    } //endfor
    return tpo_emit;
  }

  //cout << this->to_string() << " tuplas a enviar=" << nroTuplasEnviar
  //     << " is acting as a final node, not sending anything - tupla_id=" << _tupla->id()
  //     << " nro_local=" << _tupla->nro_tupla() << " smart_ptr.count=" << _tupla.use_count() << endl;

  //Assuming a super fast network
  return 1e-9;
}


double Bolt::process_tuple( shared_ptr<Tupla> tupla ){
  //cout << this->to_string( ) << " tupla PROCESSING" << endl;
  return  this->_wrapper.gen_continuous( this->_avg_service_time );
}

/**
* Extrae una tupla de la lista desde Processor
*/
shared_ptr<Tupla> Bolt::pull_tuple( ){
  shared_ptr<Tupla> t = _processor->pull_tuple( this->to_string( ) );
  return t;
}

//Retorna la ultima tupla extraida desde la cola
shared_ptr<Tupla> Bolt::get_current_tuple( ){
  return _tupla;
}
