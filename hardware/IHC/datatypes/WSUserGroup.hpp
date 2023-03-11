/*
 * WSUserGroup.hpp
 *
 *  Created on: 30. sep. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_WSUSERGROUP_HPP_
#define IHC_WSUSERGROUP_HPP_

class WSUserGroup {
private:
    std::string type;

public:
    std::string getType()
    { return type; }

    void setType(std::string value)
    { this->type = value; }

};

#endif /* IHC_WSUSERGROUP_HPP_ */
