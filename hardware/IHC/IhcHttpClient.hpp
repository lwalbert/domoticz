/*
 * IhcHttpClient.hpp
 *
 *  Created on: 30. sep. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_IHCHTTPCLIENT_HPP_
#define IHC_IHCHTTPCLIENT_HPP_
#include "../../httpclient/HTTPClient.h"

class IhcHttpClient {
public:
	std::string sendQuery(std::string httpUrl, std::string query, std::string action = "")
	{

	    std::string result;
	    std::vector < std::string > ExtraHeaders;
	    if (action.length())
	    {
	    	ExtraHeaders.push_back("SOAPAction: \"" + action + "\"");
	    }

	    ExtraHeaders.push_back("Content-Type: text/xml; charset=UTF-8");
	    HTTPClient::SetTimeout(8);

	    bool ret = HTTPClient::POST(httpUrl, query, ExtraHeaders, result);
	    if (!ret) {
	        throw std::runtime_error("Seems like the IHC controller has crashed...");//std::logic_error("LK IHC: Error connecting to Server...");
	    }
	    return result;
	}

};

#endif /* IHC_IHCHTTPCLIENT_HPP_ */
