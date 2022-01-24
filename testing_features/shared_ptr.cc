// shared_ptr constructor example
#include <iostream>
#include <memory>
#include <iomanip>

using namespace std;

class A{
public: 
  int x;

  A(){
    
  }

  //Constructor copia
  A( const A &a ){
    x = a.x;
  }

  ~A(){
    //delete x;
    cout << "Object destroyed" << endl;
  }
};

/*class B{
public:
  shared_ptr<A> a;

  B(){
    
  }
};*/

int main () {

  std::cout << std::setprecision(9) << (double)1e-9 << endl;

  shared_ptr<A> a1 = make_shared<A>();
  a1->x = 10;
  shared_ptr<A> a2 = make_shared<A>(*a1);

  cout << "A1 x=" << a1->x << " - " << a1 << endl;
  cout << "A2 x=" << a2->x << " - " << a2 << endl; 

  a2.get()->x = 11;

  cout << "A1 x=" << a1->x << endl;
  cout << "A2 x=" << a2->x << endl;

  return 0;
}
