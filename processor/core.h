#ifndef CORE_H
#define CORE_H

#include "glob.h"
#include "../node.h"
#include "../bolt.h"

class Node;
class Bolt;
class Processor;

class Core : public process
{

public:
  //Estados posibles del core, para saber si puede procesar algun task de un Spout/Bolt
  enum core_states {CORE_IDLE=0, CORE_BUSY};

  int _cid; //Core ID

  //Metricas
  double _in_use=0.0;

  //Estado del Core
  core_states _state;

  //Utilizada para saber el tiempo en que se dejan de procesar tuplas realmente
  static double SIM_TIME;

protected:
  void inner_body( void );

private:
  Processor *_processor;  //Procesador que aloja los cores

  Node *_node; //Alberga al Node (Spout o Bolt) que se esta ejecutando en el core

  std::string _name;
public:
  Core( const string &name, int cid/*, handle<Processor> processor*/ ) : process (name)
  {
    this->_cid = cid;
    //this->_processor = processor;
    this->_name = name + "_" + std::to_string( cid );
    this->_state = CORE_IDLE;
  }

  //enlaza a procesador que lo contiene
  void set_processor( Processor *);

  //Obtiene un nodo que requiere procesamiento desde cola del procesador
  void get_node_from_processor( );

  //Recibe un nodo para su procesamiento
  //void set_node( handle<Node> *);

  std::string to_string();

  ~Core( ){
  }
};
#endif
