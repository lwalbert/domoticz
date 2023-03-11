/*
 * IhcResourceInteractionService.hpp
 *
 *  Created on: 30. sep. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_IHCRESOURCEINTERACTIONSERVICE_HPP_
#define IHC_IHCRESOURCEINTERACTIONSERVICE_HPP_

#include <memory>
#include <string>
#include "datatypes/WSResourceValue.hpp"
#include "IhcException.hpp"
#include <boost/variant.hpp>
#include <iostream>
#include <memory>


static const std::string emptyQuery2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\"><soapenv:Body></soapenv:Body></soapenv:Envelope>";
class IhcWireless : public IhcHttpClient
{
private:
	std::string url;

public:
	IhcWireless(std::string hostname)
	{
		url = "http://" + hostname + "/ws/AirlinkManagementService";
	}
	TiXmlDocument getRF()
	{
	    //_log.Log(LOG_STATUS,"IHC_TRACE: %s", __FUNCTION__);
	    //std::string url = url = "http://" + hostname + "/ws/AirlinkManagementService";
        std::string sResult;
        TiXmlDocument doc;
        sResult = sendQuery(url, emptyQuery2, "getDetectedDeviceList");
        doc.Parse(sResult.c_str());

        return doc;
    }
};

class IhcResourceInteractionService : public IhcHttpClient
{
private:
	std::string url;

public:
IhcResourceInteractionService(std::string hostname)
{
//#ifdef _DEBUG
	//_log.Log(LOG_STATUS,"IHC_TRACE: %s", __FUNCTION__);
//#endif
	url = "http://" + hostname + "/ws/ResourceInteractionService";
}

ResourceValue resourceQuery(int resourceId)
{
//#ifdef _DEBUG
	//_log.Log(LOG_STATUS,"IHC_TRACE: %s", __FUNCTION__);
//#endif
	std::stringstream sstr;
    std::string httpData = sstr.str();

    TiXmlDocument doc;

    std::string sResult;

    std::string query = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    query += "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\"><soapenv:Body>";
    query += " <ns1:getRuntimeValue1 xmlns:ns1=\"utcs\">" + boost::to_string(resourceId) + "</ns1:getRuntimeValue1></soapenv:Body>";
    query += "</soapenv:Envelope>";

    sResult = sendQuery(url, query);

    if (doc.Parse(sResult.c_str()))
    {

        TinyXPath::xpath_processor processor ( doc.RootElement(), "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getRuntimeValue2");
        if (1 == processor.u_compute_xpath_node_set())
        {
            TiXmlNode* sss = processor.XNp_get_xpath_node(0);

            ResourceValue val(boost::lexical_cast<int>( sss-> FirstChildElement("ns1:resourceID")->GetText()));

            if (!strcmp(sss->FirstChildElement("ns1:value")->Attribute("xsi:type"), "ns2:WSIntegerValue"))
            {

                val.value = RangedInteger(boost::lexical_cast<int>(sss->FirstChildElement("ns1:value")->FirstChildElement("ns2:integer")->GetText()),
                boost::lexical_cast<int>(sss->FirstChildElement("ns1:value")->FirstChildElement("ns2:minimumValue")->GetText()),
                boost::lexical_cast<int>(sss->FirstChildElement("ns1:value")->FirstChildElement("ns2:maximumValue")->GetText()));
                return val;
            }
            if (!strcmp(sss->FirstChildElement("ns1:value")->Attribute("xsi:type"), "ns2:WSBooleanValue"))
            {
                //doc.Print();
                val.value =  (!strcmp(sss->FirstChildElement("ns1:value")->FirstChildElement("ns2:value")->GetText(), "true"));
                return val;
            }
            else
            {
                //std::cout << "Unknown type: (" << sss->FirstChildElement("ns1:value")->Value() << ")" << std::endl;
                //doc.Print();
            }
        }
        else
        {
            //std::cout << "No resource value found" << std::endl;

        }
    }
}


std::string getValue( TiXmlElement* a, std::string t)
{
//#ifdef _DEBUG
	//_log.Log(LOG_STATUS,"IHC_TRACE: %s", __FUNCTION__);
//#endif
    TinyXPath::xpath_processor proc(a, t.c_str());


	if (1 == proc.u_compute_xpath_node_set())
	{
		TiXmlNode* thisNode = proc.XNp_get_xpath_node(0);

		std::string res = thisNode->ToElement()->GetText();
		return res;
	}
	return "";
}

bool resourceUpdate(ResourceValue const value)
{
//#ifdef _DEBUG
	//_log.Log(LOG_STATUS,"IHC_TRACE: %s", __FUNCTION__);
//#endif
    return doResourceUpdate(value);
}

private:

bool doResourceUpdate(ResourceValue const &query)
{
//#ifdef _DEBUG
	//_log.Log(LOG_STATUS,"IHC_TRACE: %s", __FUNCTION__);
//#endif
    TiXmlDocument doc;
//#ifdef _DEBUG
    std::cout << "------------------\n";
    std::cout << query.toXml() << std::endl;
    std::cout << "------------------\n";
//#endif
    std::string const sResult = sendQuery(url, query.toXml());
    doc.Parse(sResult.c_str());
    return WSLoginResult::parseValueToBoolean(doc, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:setResourceValue2");

}

boost::shared_ptr<ResourceValue> parseResourceValue(TiXmlElement* xmlRes, int const index)
{
//#ifdef _DEBUG
	//_log.Log(LOG_STATUS,"IHC_TRACE: %s", __FUNCTION__);
//#endif
    std::string resourceID = getValue(xmlRes, "/ns1:resourceID");

	if (!resourceID.empty())
	{
		int const id = boost::lexical_cast<int>(resourceID);

		boost::shared_ptr<ResourceValue>  var = boost::shared_ptr<ResourceValue>(new ResourceValue(id));

		std::string const boolVal = getValue(xmlRes, "/ns1:value/ns" + boost::to_string(index) + ":value");
		if (boolVal.size() > 0)
		{
			(strcmp(boolVal.c_str(), "true") == 0) ? var->value = true : var->value = false;
			return boost::move(var);
		}

		std::string const intVal = getValue(xmlRes, "/ns1:value/ns" + boost::to_string(index) + ":integer");
		if (intVal.size() > 0)
		{
			var->value = RangedInteger(boost::lexical_cast<int>(intVal),
					boost::lexical_cast<int>(getValue(xmlRes, "/ns1:value/ns" + boost::to_string(index) + ":maximumValue")),
					boost::lexical_cast<int>(getValue(xmlRes, "/ns1:value/ns" + boost::to_string(index) + ":minimumValue")));
							//	return val;

			return boost::move(var);
		}

		std::string const floatVal = getValue(xmlRes, "/ns1:value/ns" + boost::to_string(index) + ":floatingPointValue");
		if (floatVal.size() > 0)
		{
			var->value = RangedFloatingPointValue(boost::lexical_cast<float>(floatVal),
					boost::lexical_cast<float>(getValue(xmlRes, "/ns1:value/ns" + boost::to_string(index) + ":maximumValue")),
					boost::lexical_cast<float>(getValue(xmlRes, "/ns1:value/ns" + boost::to_string(index) + ":minimumValue")));
							//	return val;
			return boost::move(var);
		}

		//Todo: throw unknown type
		//throw ihcException("Unknown datatype", *xmlRes);
	}

	boost::shared_ptr<ResourceValue> r = boost::shared_ptr<ResourceValue>(new ResourceValue(123));
	r->value = false;
	return r;
}

public:

void enableRuntimeValueNotifications(std::vector<int> &items)
{
//#ifdef _DEBUG
	//_log.Log(LOG_STATUS,"IHC_TRACE: %s", __FUNCTION__);
//#endif
	std::string queryPrefix = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	queryPrefix += "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">";
	queryPrefix += "<soap:Body><enableRuntimeValueNotifications1 xmlns=\"utcs\">";

	std::string querySuffix = "</enableRuntimeValueNotifications1></soap:Body></soap:Envelope>";

	std::string query = queryPrefix;
	int itemCounter = 0;
	for (std::vector<int>::iterator it = items.begin(); it != items.end(); ++it)
	{
		itemCounter++;
		query += "<xsd:arrayItem>" + boost::to_string(*it) + "</xsd:arrayItem>";
	}
	//_log.Log(LOG_STATUS,"IHC_RESULT: waiting for changes on %d items", itemCounter);

	query += querySuffix;

	std::string sResult;
	sResult = sendQuery(url, query);
}


std::vector<boost::shared_ptr<ResourceValue> > waitResourceValueNotifications(int const timeoutInSeconds)
{
//#ifdef _DEBUG
	//_log.Log(LOG_STATUS,"IHC_TRACE: %s", __FUNCTION__);
//#endif
	std::string query = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	query += "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:utcs=\"utcs\">";
	query += "<soapenv:Header/><soapenv:Body>";
	query += "<utcs:waitForResourceValueChanges1>" + boost::to_string(timeoutInSeconds) + "</utcs:waitForResourceValueChanges1></soapenv:Body>";
	query += "</soapenv:Envelope>";

	std::string sResult;

	sResult = sendQuery(url, query);

	TiXmlDocument doc;
	//sResult = "<?xml version='1.0' encoding='UTF-8'?><SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"><SOAP-ENV:Body><SOAP-ENV:Fault><faultcode>SOAP-ENV:Server</faultcode><faultstring>Logon Failed</faultstring><detail>com.wingfoot.soap.server.RouterException: Logon Failed   at _ZN4java4lang11VMThrowable16fillInStackTraceEPNS0_9ThrowableE (/utcs/lib/libgcj.so.5)   at _ZN4java4lang9Throwable16fillInStackTraceEv (/utcs/lib/libgcj.so.5)   at _ZN4java4lang9ThrowableC1EPNS0_6StringE (/utcs/lib/libgcj.so.5)   at _ZN4java4lang9ExceptionC1EPNS0_6StringE (/utcs/lib/libgcj.so.5)   at 0x102965e8 (Unknown Source)   at 0x1029665c (Unknown Source)   at 0x102994d8 (Unknown Source)   at 0x1029abec (Unknown Source)   at 0x10297284 (Unknown Source)   at 0x10256954 (Unknown Source)   at 0x10185cf0 (Unknown Source)   at 0x10185e30 (Unknown Source)   at 0x1019c258 (Unknown Source)   at 0x1019cb14 (Unknown Source)   at 0x1019da6c (Unknown Source)   at _ZN4java4lang6Thread3runEv (/utcs/lib/libgcj.so.5)   at _Z13_Jv_ThreadRunPN4java4lang6ThreadE (/utcs/lib/libgcj.so.5)   at 0x0fc7df24 (Unknown Source)   at GC_start_routine (/utcs/lib/libgcj.so.5)   at 0x0f8f43a0 (Unknown Source)   at __clone (/lib/libc.so.6)</detail></SOAP-ENV:Fault></SOAP-ENV:Body></SOAP-ENV:Envelope>";
	//_log.Log(LOG_STATUS, sResult.c_str());
    doc.Parse(sResult.c_str());
//#ifdef _DEBUG
    //doc.Print();
//#endif
    //_log.Log(LOG_STATUS,"IHC_RESULT: %s", sResult.c_str());
    std::vector<boost::shared_ptr<ResourceValue> > resourceList;

    TinyXPath::xpath_processor processor ( doc.RootElement(), "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:waitForResourceValueChanges2/ns1:arrayItem");
    int n = processor.u_compute_xpath_node_set(); // <-- this is important. It executes the Xpath expression
    if (n > 0)
    {
        if (processor.XNp_get_xpath_node(0)->FirstChild() == 0)
		{
			// Timed out, return empty list
			return resourceList;
		}
		else
		{
			for (int i = 0; i < processor.u_compute_xpath_node_set(); i++)
			{
				TiXmlNode* thisNode = processor.XNp_get_xpath_node(i);

				TiXmlElement * res = thisNode->FirstChild()->ToElement();
				resourceList.push_back(parseResourceValue(res, i + 2));
			}
		}
    }
    else
    {
        //_log.Log(LOG_STATUS,"IHC_RESULT: Controller crash!? ---------------------------------");
        //_log.Log(LOG_STATUS,"IHC DUMP: %s", sResult.c_str());
        //throw std::runtime_error("Empty result");
        throw ihcException("Empty result", query, sResult);
    }

    return resourceList;
}

};

#endif /* IHC_IHCRESOURCEINTERACTIONSERVICE_HPP_ */
