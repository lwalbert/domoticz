/*
 * IhcControllerService.hpp
 *
 *  Created on: 1. okt. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_IHCCONTROLLERSERVICE_HPP_
#define IHC_IHCCONTROLLERSERVICE_HPP_

#include "datatypes/WSBaseDataType.hpp"

static const std::string emptyQuery = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\"><soapenv:Body></soapenv:Body></soapenv:Envelope>";

class IhcControllerService : public IhcHttpClient
{
private:
	std::string url;

public:
	IhcControllerService(std::string hostname)
	{
		url = "http://" + hostname + "/ws/ControllerService";
	}

public:

	WSProjectInfo getProjectInfo()
	{

		TiXmlDocument doc;
		WSProjectInfo projectInfo;
		std::string sResult;
		sResult = sendQuery(url, emptyQuery, "getProjectInfo");
		doc.Parse(sResult.c_str());
		projectInfo.encodeData(doc);
		return projectInfo;
	}

	int getProjectNumberOfSegments()
	{
		std::string sResult;
		sResult = sendQuery(url, emptyQuery, "getIHCProjectNumberOfSegments");
		TiXmlDocument doc;
		doc.Parse(sResult.c_str());

		std::string numberOfSegments = WSBaseDataType::parseValue(doc, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getIHCProjectNumberOfSegments1");
		return boost::lexical_cast<int>(numberOfSegments);
	}

	int getProjectSegmentationSize()
	{
		std::string sResult;
		sResult = sendQuery(url, emptyQuery, "getIHCProjectSegmentationSize");
		TiXmlDocument doc;
		doc.Parse(sResult.c_str());

		std::string segmentationSize = WSBaseDataType::parseValue(doc, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getIHCProjectSegmentationSize1");

		return boost::lexical_cast<int>(segmentationSize);
    }

	WSFile getProjectSegment(int index, int major, int minor)
	{
		std::string query = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
		query += "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">";
		query += "<soap:Body>";
		query += "<ns1:getIHCProjectSegment1 xmlns:ns1=\"utcs\" xsi:type=\"xsd:int\">" + boost::to_string(index) + "</ns1:getIHCProjectSegment1>";
		query += "<ns2:getIHCProjectSegment2 xmlns:ns2=\"utcs\" xsi:type=\"xsd:int\">" + boost::to_string(major) + "</ns2:getIHCProjectSegment2>";
		query += "<ns3:getIHCProjectSegment3 xmlns:ns3=\"utcs\" xsi:type=\"xsd:int\">" + boost::to_string(minor) + "</ns3:getIHCProjectSegment3>";
		query += "</soap:Body></soap:Envelope>";

		std::string sResult;
		TiXmlDocument doc;
		sResult = sendQuery(url, query, "getIHCProjectSegment");
		doc.Parse(sResult.c_str());

		WSFile file;// = new WSFile();
		file.encodeData(doc);
		return file;
	}

	WSControllerState getControllerState()
	{
		std::string sResult;
		TiXmlDocument doc;

		sResult = sendQuery(url, emptyQuery, "getState");
		doc.Parse(sResult.c_str());

		WSControllerState controllerState;
		controllerState.encodeData(doc);

		return controllerState;
	}



};

#endif /* IHC_IHCCONTROLLERSERVICE_HPP_ */
