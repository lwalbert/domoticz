/*
 * WSResourceValue.hpp
 *
 *  Created on: 30. sep. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_WSRESOURCEVALUE_HPP_
#define IHC_WSRESOURCEVALUE_HPP_
#include <string>
#include <boost/variant.hpp>

struct RangedInteger {
    int min;
    int max;
    int value;

    explicit RangedInteger(int value, int min = 0, int max = 100)
      : min(min)
      , max(max)
      , value(value)
    {}
};

struct RangedFloatingPointValue {
    float min;
    float max;
    float value;

    explicit RangedFloatingPointValue(float value, float min = 0, float max = 100)
      : min(min)
      , max(max)
      , value(value)
    {}
};

struct valueVisitor : boost::static_visitor<int> {
	int ID;

	valueVisitor()
	{
		ID = 0;
	}

    int operator()(bool const value) const{
    	int const val = (value) ? 1 : 0;
    	return val;
    }

    int operator()(RangedInteger const value) const{
        return value.value;
    }

    float operator()(RangedFloatingPointValue const value) const{
            return value.value;
    }

    template<typename T>
    int operator()(T const&) const
    {
        return 0;
    }
};

struct ToStringVisitor : boost::static_visitor<std::string> {
	int ID;

	ToStringVisitor(int const ID)
	: ID(ID)
	{}

    std::string operator()(bool const value) const{
    	std::string const val = (value) ? "true" : "false";
    	std::string const out = "Object " + boost::to_string(ID) + ", value \"" + val + "\"";
    	return out;
    }

    std::string operator()(RangedInteger const value) const{
    	std::string const out = "Object " + boost::to_string(ID) + ", value \"" + boost::lexical_cast<std::string>(value.value) + "\"";
        return out;
    }

    std::string operator()(RangedFloatingPointValue const value) const{
        	std::string const out = "Object " + boost::to_string(ID) + ", value \"" + boost::lexical_cast<std::string>(value.value) + "\"";
            return out;
    }

    template<typename T>
    std::string operator()(T const&) const{
    	std::string const out = "Object " + boost::to_string(ID) + ", Unknown type";
        return out;
    }
};

struct XmlVisitor : boost::static_visitor<std::string> {
	int ID;

	XmlVisitor(int const ID)
	: ID(ID)
	{}

	std::string operator()(bool const value) const
	{
		std::string status;
		status = (value) ? "true" : "false";
		std::string query = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
		query += "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">";
		query +=  "<soap:Body><setResourceValue1 xmlns=\"utcs\">";
		query +=  "  <value xmlns:q1=\"utcs.values\" xsi:type=\"q1:WSBooleanValue\"><q1:value>" + status + "</q1:value>";
		query +=  "  </value><resourceID>" + boost::to_string(ID) + "</resourceID><isValueRuntime>true</isValueRuntime>";
		query +=  " </setResourceValue1></soap:Body></soap:Envelope>";

		return query;
	}

	std::string operator()(RangedInteger const value) const
	{
		std::string query = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        query += "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">";
        query += "<soap:Body><setResourceValue1 xmlns=\"utcs\">";
        query += "  <value xmlns:q1=\"utcs.values\" xsi:type=\"q1:WSIntegerValue\">";
        query += "   <q1:maximumValue>" + boost::to_string(value.max) + "</q1:maximumValue><q1:minimumValue>" + boost::to_string(value.min) + "</q1:minimumValue>";
        query += "   <q1:integer>" + boost::to_string(value.value) + "</q1:integer></value><resourceID>" + boost::to_string(ID) + "</resourceID>";
        query += "  <isValueRuntime>true</isValueRuntime></setResourceValue1></soap:Body>";
        query += "</soap:Envelope>";

        return query;
	}

	template<typename T>
    std::string operator()(T const&) const
    {
        return "default";
    }
};

struct Null {};

struct ResourceValue {
    int ID;
    boost::variant<Null, bool, RangedInteger, RangedFloatingPointValue> value;

    explicit ResourceValue(int ID)
      : ID(ID)
    {}

    template<typename T>
    ResourceValue(int ID, T const& val)
      : ID(ID)
      , value(val)
    {}

    std::string toString() const
    {
        return boost::apply_visitor(ToStringVisitor(ID), value);
    }

    int intValue() const
    {
    	return boost::apply_visitor(valueVisitor(), value);
    }

    float floatValue() const
        {
        	return boost::apply_visitor(valueVisitor(), value);
        }

    std::string toXml( ) const
    {
    	return boost::apply_visitor( XmlVisitor(ID), value);
    }
};

class WSResourceValue
{
private:
	int resourceID;

public:
	virtual ~WSResourceValue(){};

	WSResourceValue (int resourceID = 0)
	: resourceID(resourceID)
	{}

	int getResourceID() const
	{ return resourceID; }

	void setResourceID(int resID)
	{ this->resourceID = resID; }

    virtual std::string toString() const = 0;

};

#endif /* IHC_WSRESOURCEVALUE_HPP_ */
