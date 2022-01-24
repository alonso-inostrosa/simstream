#ifndef _WRAPPER_H_
#define _WRAPPER_H_
#include <chrono>
#include <string.h>
#include <iostream>
#include <map>
#include <random>
#include "prob.h"
#include "spline.h"

class Wrapper{
	private:	int _seed;
				std::map<std::string,std::tuple<int,int,tk::spline>> _spline;
	
	public:	Wrapper(void){
					this->_seed=std::chrono::system_clock::now().time_since_epoch().count();
				}
				Wrapper(const Wrapper &_wrapper){
					this->_seed=_wrapper._seed;
				}
				Wrapper& operator=(const Wrapper &_wrapper){
					this->_seed=_wrapper._seed;		
					return(*this);
				}	
				~Wrapper(void){

				}

				double gen_continuous(const std::string &_function){
					std::string copy_function = _function;
					char *distribution=strtok((char*)copy_function.c_str(),"()");
					char *params=strtok(NULL,"()");
					vector<double> values;

					if(!strcmp(distribution,"spline")){
						return(this->spline(params));
					}
			
					char *token=strtok(params,",");
					while(token!=NULL){
						values.push_back(std::stod(token));
						token=strtok(NULL,",");
					}

					if(!strcmp(distribution,"fixed"))
						return(values[0]);
					else if(!strcmp(distribution,"chi2"))
						return(this->chi2(values[0],values[1],values[2]));
					else if(!strcmp(distribution,"maxwell"))
						return(this->maxwell(values[0],values[1]));
					else if(!strcmp(distribution,"expon"))
						return(this->expon(values[0],values[1]));
					else if(!strcmp(distribution,"invgauss"))
						return(this->invgauss(values[0],values[1],values[2]));
					else if(!strcmp(distribution,"norm"))
						return(this->norm(values[0],values[1]));
					else if(!strcmp(distribution,"lognorm"))
						return(this->lognorm(values[0],values[1],values[2]));
					else{
						std::cerr << "Unknown distribution: " << distribution << std::endl;
						exit(EXIT_FAILURE);
					}
				}
				int gen_discrete(const std::string &_function){
					std::string copy_function = _function;
               char *distribution=strtok((char*)copy_function.c_str(),"()");
					char *params=strtok(NULL,"()");
					vector<double> values;
			
					if(!strcmp(distribution,"spline")){
						return(int(this->spline(params)));
					}
			
					char *token=strtok(params,",");
					while(token!=NULL){
						values.push_back(std::stod(token));
						token=strtok(NULL,",");
					}
					if(!strcmp(distribution,"fixed"))
						return(int(values[0]));
					else if(!strcmp(distribution,"bernoulli"))
						return(this->geom(values[0],values[1],values[2]));
					else if(!strcmp(distribution,"geom"))
						return(this->geom(values[0],values[1],values[2]));
					else if(!strcmp(distribution,"nbinom"))
						return(this->nbinom(int(values[0]),values[1],values[2],values[3]));
					else{
						std::cerr << "Unknown distribution: " << distribution << std::endl;
						exit(EXIT_FAILURE);
					}
				}

	private:	double spline(const string &_params){
					static std::mt19937 rng(this->_seed);

					if(this->_spline.count(_params)==0){
						FILE *in=fopen(_params.c_str(),"r+t");
						double x=0.0,y=0.0;
						std::vector<double> X,Y;
	
						while(fscanf(in,"%lf",&y)!=EOF){
							Y.push_back(y);
							X.push_back(x);
							x+=1.0;
						}
						fclose(in);
						tk::spline s;
   					s.set_points(X,Y);
						
						this->_spline[_params]=std::tuple<int,int,tk::spline>(0,int(X.size()),s);
					}

					std::uniform_int_distribution<int> uniform(std::get<0>(this->_spline[_params]),std::get<1>(this->_spline[_params])-1);
					return((std::get<2>(this->_spline[_params]))(uniform(rng)));
				}
				double chi2(double a,double loc,double scale){
					return(chi_square_sample(a,this->_seed)*scale+loc);
				}	
				double maxwell(double loc,double scale){
					return(maxwell_sample(1.0,this->_seed)*scale+loc);
				}	
				double expon(double loc,double scale){
					return(exponential_sample(0.0,1.0,this->_seed)*scale+loc);
				}
				double invgauss(double mu,double loc,double scale){
					return(inverse_gaussian_sample(mu,1.0,this->_seed)*scale+loc);
				}	
				double norm(double loc,double scale){
					return(normal_sample(0.0,1.0,this->_seed)*scale+loc);
				}
				double lognorm(double s,double loc,double scale){
					return(log_normal_sample(0.0,s,this->_seed)*scale+loc);
				}
				int nbinom(int a,double b,double loc,double scale){
					return(int(binomial_sample(a,b,this->_seed)*scale+loc));
				}
				int geom(double a,double loc,double scale){
					return(int(geometric_sample(a,this->_seed)*scale+loc));
				}
				int bernoulli(double a,double loc,double scale){
					return(int(bernoulli_sample(a,this->_seed)*scale+loc));
				}
};
#endif
