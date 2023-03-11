/*
 * WSProjectInfo.hpp
 *
 *  Created on: 1. okt. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_DATATYPES_WSPROJECTINFO_HPP_
#define IHC_DATATYPES_WSPROJECTINFO_HPP_
#include <iostream>

class WSProjectInfo : public WSBaseDataType {
private:
    int visualMinorVersion;
    int visualMajorVersion;
    int projectMajorRevision;
    int projectMinorRevision;
    WSDate lastmodified;
    std::string projectNumber;
    std::string customerName;
    std::string installerName;

public:
    WSProjectInfo() {
    }

    WSProjectInfo(int visualMinorVersion, int visualMajorVersion, int projectMajorRevision,
            int projectMinorRevision, WSDate lastmodified, std::string projectNumber, std::string customerName,
            std::string installerName) {

        this->visualMinorVersion = visualMinorVersion;
        this->visualMajorVersion = visualMajorVersion;
        this->projectMajorRevision = projectMajorRevision;
        this->projectMinorRevision = projectMinorRevision;
        this->lastmodified = lastmodified;
        this->projectNumber = projectNumber;
        this->customerName = customerName;
        this->installerName = installerName;
    }

    int getVisualMinorVersion() {
        return visualMinorVersion;
    }

    void setVisualMinorVersion(int visualMinorVersion)
    {
        this->visualMinorVersion = visualMinorVersion;
    }

    int getVisualMajorVersion() {
        return visualMajorVersion;
    }

    void setVisualMajorVersion(int visualMajorVersion) {
        this->visualMajorVersion = visualMajorVersion;
    }

    int getProjectMajorRevision() {
        return projectMajorRevision;
    }

    void setProjectMajorRevision(int projectMajorRevision) {
        this->projectMajorRevision = projectMajorRevision;
    }

    int getProjectMinorRevision() {
        return projectMinorRevision;
    }

    void setProjectMinorRevision(int projectMinorRevision) {
        this->projectMinorRevision = projectMinorRevision;
    }

    WSDate getLastmodified() {
        return lastmodified;
    }

    void setLastmodified(WSDate lastmodified) {
        this->lastmodified = lastmodified;
    }

    std::string getProjectNumber() {
        return projectNumber;
    }

    void setProjectNumber(std::string projectNumber) {
        this->projectNumber = projectNumber;
    }

    std::string getCustomerName() {
        return customerName;
    }

    void setCustomerName(std::string customerName) {
        this->customerName = customerName;
    }

    std::string getInstallerName() {
        return installerName;
    }

    void setInstallerName(std::string installerName) {
        this->installerName = installerName;
    }

    void encodeData(TiXmlDocument data)
    {
        std::string value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:visualMinorVersion");
        setVisualMinorVersion(boost::lexical_cast<int>(value));

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:visualMajorVersion");
        setVisualMajorVersion(boost::lexical_cast<int>(value));

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:projectMajorRevision");
        setProjectMajorRevision(boost::lexical_cast<int>(value));

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:projectMinorRevision");
        setProjectMinorRevision(boost::lexical_cast<int>(value));

        WSDate lastmodified;// = new WSDate();

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:lastmodified/ns1:day");
        lastmodified.setDay(boost::lexical_cast<int>(value));

        value = parseValue(data,
                "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:lastmodified/ns1:monthWithJanuaryAsOne");
        lastmodified.setMonthWithJanuaryAsOne(boost::lexical_cast<int>(value));

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:lastmodified/ns1:hours");
        lastmodified.setHours(boost::lexical_cast<int>(value));

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:lastmodified/ns1:minutes");
        lastmodified.setMinutes(boost::lexical_cast<int>(value));

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:lastmodified/ns1:seconds");
        lastmodified.setSeconds(boost::lexical_cast<int>(value));

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:lastmodified/ns1:year");
        lastmodified.setYear(boost::lexical_cast<int>(value));

        setLastmodified(lastmodified);

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:projectNumber");
        setProjectNumber(value);

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:customerName");
        setCustomerName(value);

        value = parseValue(data, "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getProjectInfo1/ns1:installerName");
        setInstallerName(value);

    }

    friend std::ostream& operator<<(std::ostream& os, const WSProjectInfo& proj);
};

std::ostream& operator<<(std::ostream& os, const WSProjectInfo& proj)
{
    os << "Customer name: " << proj.customerName << std::endl;
    os << "Installer name: " << proj.installerName << std::endl;
    os << "Last modified: " << proj.lastmodified << std::endl;
    os << "Project version: " << proj.projectNumber << "." << proj.projectMajorRevision << "." << proj.projectMinorRevision << std::endl;
    os << "Project number: " << proj.projectNumber << std::endl;
    os << "Visual: " << proj.visualMajorVersion << "." << proj.visualMinorVersion << std::endl;

    return os;
}

#endif /* IHC_DATATYPES_WSPROJECTINFO_HPP_ */
