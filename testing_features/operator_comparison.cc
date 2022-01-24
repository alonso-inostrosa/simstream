#include <iostream>
#include <queue>
#include <vector>
#include <set>
#include <functional>

using namespace std;

class A{
public:

  int x;
  int tamanio;

  A( const uint32_t& x, const uint32_t tamanio){
    this->x = x;
    this->tamanio = tamanio;
  }
};

class AContainer{
public:

  uint32_t free_space;

  AContainer( uint32_t free_space ){
    this->free_space = free_space;
  }

  class AComparator : public std::binary_function<A*,A*,bool>{
    public:
      bool operator()(const A *t1, const A *t2){
        return t1->x > t2->x;
      }
  };
  priority_queue<A*, std::vector<A*>, AComparator> pq;

  class ASetComparator : public std::binary_function<A*,A*,bool>{
    public:
      bool operator()(const A *t1, const A *t2){
        //cout << "\tt1->x=" << t1->x << " vs t2->x=" << t2->x << endl;
        return !(t1->x < t2->x) && !( t2->x == t1->x );
      }
  };
  set<A*, ASetComparator> keys;

  void insert (A *a){
    auto it = keys.find( a );
    if( /*keys.find( a )*/ it == keys.end() ){ 
      cout << "Elemento a x=" << a->x << " no existe, insertando. Espacio libre antes=" << free_space;
      free_space -= a->tamanio;
      cout << " - espacio libre ahora=" << free_space << endl;
      keys.insert( a );
      pq.push( a );
    }else{
      cout << "Elemento a x=" << a->x << " ya existe " << (*it)->x << endl;
    }
  }

  void free( uint32_t space_to_free){
  }

  
};

int main( ){

  AContainer *cont = new AContainer(10);

  A *a1 = new A(7,3);
  A *a2 = new A(3,3);
  A *a3 = new A(3,3);
  A *a4 = new A(5,4);

  cont->insert( a1 );
  cont->insert( a2 );
  cont->insert( a2 );
  cont->insert( a3 );
  cont->insert( a4 );

  while( !cont->pq.empty() ){
    cout << "Elem: " <<  cont->pq.top( )->x << endl;
    a1 = cont->pq.top( );
    cont->free_space += a1->tamanio;
    cont->pq.pop();
    cout << "Espacio libre=" << cont->free_space << endl;
  }

  /*if( *a1 > *a4 ){
    cout << a1->x << ">" << a4->x << endl;
  }else
    cout << a1->x << "<=" << a4->x << endl;

   if( *a4 > *a1 ){
    cout << a4->x << ">" << a1->x << endl;
  }else
    cout << a4->x << "<=" << a1->x << endl;*/


}
