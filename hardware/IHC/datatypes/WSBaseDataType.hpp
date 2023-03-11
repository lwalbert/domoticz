/*
 * WSBaseDataType.hpp
 *
 *  Created on: 1. okt. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_DATATYPES_WSBASEDATATYPE_HPP_
#define IHC_DATATYPES_WSBASEDATATYPE_HPP_

class WSBaseDataType
{
public:

	static std::string parseValue(TiXmlDocument const doc, std::string const xpath)
	{
		std::string xp = xpath + "/text()"; // <-- Fixing what I think is a bug in tinyXpath
		TinyXPath::xpath_processor processor(doc.RootElement(), xp.c_str());
		TiXmlString xresult = processor.S_compute_xpath();
		return xresult.c_str();
	}

	static bool parseValueToBoolean(TiXmlDocument const doc, std::string const xpath)
	{
	    bool result;
		std::string xresult = parseValue(doc, xpath);
	    std::istringstream is(xresult);
	    is >> std::boolalpha >> result;
	    return result;
	}
};

#endif /* IHC_DATATYPES_WSBASEDATATYPE_HPP_ */
