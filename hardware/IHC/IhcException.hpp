/*
 * IhcException.hpp
 *
 *  Created on: 7. okt. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_IHCEXCEPTION_HPP_
#define IHC_IHCEXCEPTION_HPP_

// using standard exceptions
#include <iostream>
#include <exception>

class ihcException : public std::exception {
	std::string _msg;
    //int _a;
    //TiXmlElement _xmlRes;
	std::string _xmla;
	std::string _xmlb;

    public:
        //ihcException(const std::string& msg, const int a) : _msg(msg), _a(a){}
        //ihcException(const std::string& msg, const TiXmlElement xmlRes) : _msg(msg), _xmlRes(xmlRes){}
    	ihcException(const std::string& msg, const std::string xmla, const std::string xmlb) : _msg(msg), _xmla(xmla), _xmlb(xmlb){}

        virtual const char* what() const noexcept override
        {
            return _msg.c_str();
        }
        const std::string get_xmla(){ return _xmla;}
        const std::string get_xmlb(){ return _xmlb;}
        //const TiXmlElement get_a(){ return _xmlRes; }

};


/*class myexception: public exception
{
  virtual const char* what() const throw()
  {
    return "My exception happened";
  }
} myex;

int main () {
  try
  {
    throw myex;
  }
  catch (exception& e)
  {
    cout << e.what() << '\n';
  }
  return 0;
}
*/

#endif /* IHC_IHCEXCEPTION_HPP_ */
