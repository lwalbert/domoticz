/*
 * IhcAuthenticationService.hpp
 *
 *  Created on: 30. sep. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_IHCAUTHENTICATIONSERVICE_HPP_
#define IHC_IHCAUTHENTICATIONSERVICE_HPP_

#include "IhcHttpClient.hpp"
class IhcAuthenticationService : public IhcHttpClient {
private:
	std::string url;
	//TODO: int timeout;
public:

	IhcAuthenticationService(std::string hostname)
	{
		url = "http://" + hostname + "/ws/AuthenticationService";
	}

	IhcAuthenticationService(std::string host, int timeout)
	{
		new IhcAuthenticationService(host);
		//this->timeout = timeout;
		//Todo: Set connection Timeout
	}

	WSLoginResult* authenticate(std::string username, std::string password, std::string application = "treeview")
	{
	    TiXmlDocument doc;

	    std::string sResult;

	    std::string query = "<x:Envelope xmlns:x=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:utc=\"utcs\">\r\n";
	    query += "<x:Header/>\r\n";
	    query += "<x:Body>\r\n";
	    query += "<utc:authenticate1>\r\n";
	    query += "<utc:password>" + password + "</utc:password>\r\n";
	    query += "<utc:username>" + username + "</utc:username>\r\n";
	    query += "<utc:application>" + application + "</utc:application>\r\n";
	    query += "</utc:authenticate1>\r\n";
	    query += "</x:Body>\r\n";
	    query += "</x:Envelope>\r\n\r\n";

        sResult = sendQuery(url, query);

	    doc.Parse(sResult.c_str());

	    WSLoginResult * loginResult = new WSLoginResult;
	    loginResult->encodeData(doc);
	    return loginResult;
	}

	void logout()
	{
		TiXmlDocument doc;

			    std::string sResult;

			    std::string query = "<x:Envelope xmlns:x=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:utc=\"utcs\">\r\n";
			    query += "<x:Header/>\r\n";
			    query += "<x:Body/>\r\n";
			    query += "</x:Envelope>\r\n\r\n";
		        sResult = sendQuery(url, query, "disconnect");

			    doc.Parse(sResult.c_str());
			    doc.Print();




	}
};

#endif /* IHC_IHCAUTHENTICATIONSERVICE_HPP_ */
