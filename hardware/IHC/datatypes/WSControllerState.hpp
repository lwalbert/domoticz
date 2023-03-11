/*
 * WSControllerState.hpp
 *
 *  Created on: 2. okt. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_DATATYPES_WSCONTROLLERSTATE_HPP_
#define IHC_DATATYPES_WSCONTROLLERSTATE_HPP_

class WSControllerState : public WSBaseDataType
{
protected:
    std::string state;

public:
    WSControllerState() {}

    WSControllerState(std::string state)
    { this->state = state; }

    std::string getState()
    { return state; }

    void setState(std::string state)
    { this->state = state; }

    void encodeData(TiXmlDocument data)
    {
    	std::string returnVal;

    	returnVal = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getState1/ns1:state");
    	if (returnVal.size() > 0)
    	{
    		state = returnVal;
    		return;
		}

		returnVal = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:waitForControllerStateChange3/ns1:state");
		if (returnVal.size() > 0)
		{
			state = returnVal;
			return;
		}
    }
};

#endif /* IHC_DATATYPES_WSCONTROLLERSTATE_HPP_ */
