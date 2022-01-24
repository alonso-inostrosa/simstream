#ifndef GENERIC_MEMORY_H
#define GENERIC_MEMORY_H

#include "../glob.h"

class GenericMemory
{
public:
  std::string _name;
  uint64_t _capacity;
  uint64_t _in_use;
  uint64_t _free_space;
  uint64_t _cumulative_use; //Espacio usado acumulado, para estadisticas
  double _max_use;
  uint32_t _nbr_accesses;   //Nro de accesos, para estadisticas

  GenericMemory(){
    _capacity        = MAX_MEMORY;
    _in_use          = 0;
    _free_space      = MAX_MEMORY;
    _cumulative_use  = 0;
    _max_use         = 0.0;
    _nbr_accesses    = 0;
  }

  GenericMemory( const string& name, uint64_t capacity ){
    _name            = "Mem_" + name;
    _capacity        = capacity;
    _in_use          = 0;
    _free_space      = capacity;
    _cumulative_use  = 0;
    _max_use         = 0.0;
    _nbr_accesses    = 0;
  }

  //Retorna el nombre del objeto, para identificar quien lo contiene
  std::string to_string( ){ return this->_name; }

  //Retorna la capacidad de la memoria
  uint64_t capacity( ){ return this->_capacity; }

  //Retorna espacio disponible
  uint64_t free_space( ){ return this->_free_space; }

  //Accesores de _in_use
  uint64_t in_use( ){ return this->_in_use; }
  void in_use( uint64_t in_use ){ this->_in_use = in_use; }

  //METODOS VIRTUALES
  //Permite liberar el espacio definido como parametro
  virtual void free( uint64_t ) = 0;

  //Permite reservar el espacio definido como parametro
  virtual bool malloc( uint64_t ) = 0;

  //Metodos genericos
  double average_use( ){ return (double)(this->_cumulative_use/this->_nbr_accesses); }

  uint64_t max_memory_consumption( ){ return this->_max_use; }

  void max_use( ){
    if( this->_max_use < this->_in_use )
      this->_max_use = this->_in_use;
  }
};



class RamMemory : public GenericMemory
{
private:
  std::multimap< string, shared_ptr<Tupla>> _contents;

public:
  RamMemory( ) : GenericMemory(){
    
  }

  RamMemory( const string& name, uint64_t capacity ): GenericMemory( name, capacity ){
    
  }

  void free( uint64_t space_to_free ){
    this->_in_use -= space_to_free;
    this->_free_space += space_to_free;
    //std::cout << this->to_string() << " releasing required=" << space_to_free << " - in use (before release)=" << (_in_use+space_to_free) << " - in use (after release)=" << _in_use << " - free=" << _free_space << endl;
  }

  bool malloc( uint64_t space_to_reserve ){
    if( this->_free_space < space_to_reserve ){
      std::cerr << this->to_string() << " cannot allocate required space into memory - in use=" << _in_use << " - free=" << _free_space << endl;
      return false;
    }

    this->_in_use += space_to_reserve;
    this->_free_space -= space_to_reserve;

    //std::cout << this->to_string() << " allocating required=" << space_to_reserve << " - in use (before request)=" << (_in_use-space_to_reserve) << " - in use (after request)=" << _in_use << " - free=" << _free_space << endl;

    //STATS
    //Nro de veces que fue accedida la memoria
    this->_nbr_accesses++;
    this->_cumulative_use += this->_in_use;
    this->max_use( );

    return true;
  }

  //Almacena una tupla (puede haber varias copias)
  void storeTuple( shared_ptr<Tupla> t ){
    _contents.insert( std::pair<string, shared_ptr<Tupla> >(t->contenido(), t) );
  }

  //Remueve solo una tupla del multimap.
  void removeTuple( shared_ptr<Tupla> t ){

    //Encontramos solo una copia de Tupla
    auto it = this->_contents.find( t->contenido( ) );
    //La borramos mediante un iterador
    this->_contents.erase( it );
  }
  
};


class LRU : public GenericMemory
{
private:
  //Comparador para la cola de prioridad
  class TuplaLessThanComparator : public std::binary_function<shared_ptr<Tupla>,shared_ptr<Tupla>,bool>{
    public:
      bool operator()(shared_ptr<Tupla> t1, shared_ptr<Tupla> t2) const {
        return t1->timestamp() > t2->timestamp();
      }
  };
  std::priority_queue< shared_ptr<Tupla>, vector<shared_ptr<Tupla>>, TuplaLessThanComparator > contents;

  class TuplaEqualComparator : public std::binary_function<shared_ptr<Tupla>,shared_ptr<Tupla>,bool>{
    public:
      bool operator()(shared_ptr<Tupla> t1, shared_ptr<Tupla> t2) const {
        return !(t1->contenido() > t2->contenido()) && !(t2->contenido() > t1->contenido());
      }
  };
  std::set< shared_ptr<Tupla>, TuplaEqualComparator > storedElements;

public:
  LRU( ) : GenericMemory(){

  }

  LRU( const string& name, uint64_t capacity ): GenericMemory( name, capacity ){

  } 

  //Remueve tuplas hasta liberar el espacio requerido
  void free( uint64_t space_to_free){
    assert( capacity() > space_to_free );
    while( free_space( ) < space_to_free ){
      //Remover elementos de contents (cola prioridad) y de storedElements (set)
      shared_ptr<Tupla> tupla = contents.top();
      contents.pop();

      //Actualizar espacio libre
      this->_free_space += tupla->tamanio();

      //Remover Tupla si no hay mas referencias a ella.
    }
  }

  bool malloc(uint64_t){
    
    return true;
  }

  void insert( shared_ptr<Tupla> tupla ){
    if( storedElements.find( tupla ) == storedElements.end( ) ){
      free( tupla->tamanio() );           //Libera espacio necesario para almacenar la tupla en Cache
      storedElements.insert( tupla );
      contents.push( tupla );
      malloc( tupla->tamanio( ) );
    }
  }

};
#endif
