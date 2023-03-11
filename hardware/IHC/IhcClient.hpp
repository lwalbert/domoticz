/*
 * IhcClient.hpp
 *
 *  Created on: 30. sep. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_IHCCLIENT_HPP_
#define IHC_IHCCLIENT_HPP_
#define BOOST_BIND_NO_PLACEHOLDERS
#include <string>
#include <cstring>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/bind/bind.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <pthread.h>
#include "../../httpclient/HTTPClient.h"
#include "datatypes/WSDate.hpp"
#include "datatypes/WSUser.hpp"
#include "datatypes/WSUserGroup.hpp"
#include "datatypes/WSBaseDataType.hpp"
#include "datatypes/WSLoginResult.hpp"
#include "datatypes/WSResourceValue.hpp"
#include "datatypes/WSProjectInfo.hpp"
#include "datatypes/WSFile.hpp"
#include "datatypes/WSControllerState.hpp"
#include "IhcAuthenticationService.hpp"
#include "IhcResourceInteractionService.hpp"
#include "IhcControllerService.hpp"
#include "../webserver/Base64.h"

class ihcClient
{

public:
	enum ConnectionState {
		DISCONNECTED,
		CONNECTING,
		CONNECTED
	} connectionState;

private:
	std::string username;
	std::string password;
	std::string ip;
	std::string projectFile;
	std::string dumpResourceToFile;

	IhcAuthenticationService* authenticationService;
	IhcResourceInteractionService* resourceInteractionService;
	IhcControllerService* controllerService;
	IhcWireless* wire;

	WSControllerState controllerState;

public:
ihcClient(std::string const &ip, std::string const &username, std::string const &password)
{
    this->ip = ip;
    this->username = username;
    this->password = password;
    this->connectionState = DISCONNECTED;

    authenticationService = new IhcAuthenticationService(ip);
    resourceInteractionService = new IhcResourceInteractionService(ip);
    controllerService = new IhcControllerService(ip);
    wire = new IhcWireless(ip);
}

ResourceValue resourceQuery(int const &res)
{ return resourceInteractionService->resourceQuery(res); }

std::vector<boost::shared_ptr<ResourceValue> > waitResourceValueNotifications(int const timeout)
{ return resourceInteractionService->waitResourceValueNotifications(timeout); }

void reset()
{
	connectionState = DISCONNECTED;
}

void openConnection()
{
    connectionState = CONNECTING;

    WSLoginResult* loginResult;
//    try
//    {
         loginResult = authenticationService->authenticate(username, password);
/*    }
    catch (const char* msg)
    {
        _log.Log(LOG_ERROR, "LK IHC: Error: '%s'", msg);
        throw std::runtime_error(msg);
    }
*/
    if (!loginResult->isLoginWasSuccessful())
    {
        connectionState = DISCONNECTED;

        if (loginResult->isLoginFailedDueToAccountInvalid())
        { std::cout << "account\n";
            throw "invalid username or password"; }

        if (loginResult->isLoginFailedDueToConnectionRestrictions())
        { std::cout << "connect\n";
            throw "connection restrictions"; }

        if (loginResult->isLoginFailedDueToInsufficientUserRights())
        { std::cout << "rights\n";
            throw "insufficient user rights"; }

        throw "unknown connection error";
    }

    connectionState = CONNECTED;
}
/* TODO: Remove  before production*/
void ihclogout()
{
authenticationService->logout();
}

/* end */
public:

TiXmlDocument getRF()
{ return wire->getRF();}
WSProjectInfo getProjectInfo()
{ return controllerService->getProjectInfo(); }

TiXmlDocument loadProject()
{
	std::cout << "Loading IHC /ELKO LS project file from controller...\n";
	TiXmlDocument doc = LoadProjectFileFromController();
	return doc;
}

static std::string decompress(const std::string& data)
{
    namespace bio = boost::iostreams;

    std::stringstream compressed(data);
    std::stringstream decompressed;

    bio::filtering_streambuf<bio::input> out;
    out.push(bio::gzip_decompressor());
    out.push(compressed);
    bio::copy(out, decompressed);

    return decompressed.str();
}


template <typename T> bool resourceUpdate(T value)
{
	return resourceInteractionService->resourceUpdate(value);
}

void enableRuntimeValueNotification(std::vector<int> resourceIdLis)
{
	resourceInteractionService->enableRuntimeValueNotifications(resourceIdLis);
}

private:
std::string iso_8859_1_to_utf8(std::string str, size_t pos)
{
    std::string strOut;
    for (auto it = str.begin(); it != str.end(); ++it)
    {
        uint8_t ch = *it;
        if (ch < 0x80) {
            strOut.push_back(ch);
        }
        else {
            strOut.push_back(0xc0 | ch >> 6);
            strOut.push_back(0x80 | (ch & 0x3f));
        }
    }
    return strOut;
}

TiXmlDocument LoadProjectFileFromController()
{
    WSProjectInfo projectInfo = getProjectInfo();
    int numberOfSegments = controllerService->getProjectNumberOfSegments();
    int segmentationSize = controllerService->getProjectSegmentationSize();

    std::vector<char> projectData;
    for (int i = 0; i < numberOfSegments; i++)
    {
#ifdef _DEBUG
        std::cout << "Downloading segment " << i << "..." << std::endl;
#endif
        WSFile data = controllerService->getProjectSegment(i, getProjectInfo().getProjectMajorRevision(), getProjectInfo().getProjectMinorRevision());

        auto t = data.getData();
        projectData.insert(projectData.end(), t.begin(), t.end());

    }
#ifdef _DEBUG
    std::cout << "Final size: " << projectData.size() << std::endl;
#endif

    std::string str(projectData.begin(), projectData.end());
    std::string decoded = base64_decode(str);
    std::string extracted = decompress(decoded);

#ifdef _DEBUG
    std::cout << "Final size decoded: " << decoded.size() << std::endl;
    std::cout << "Final size extracted: " << extracted.size() << std::endl;
    std::cout << "Project saved as project_file.vis" << std::endl;
    std::ofstream out("project_file.vis");
    out << extracted;
    out.close();
#endif

    // Skip unsupported statements
    size_t pos = 0;
    while (true) {
        pos = extracted.find_first_of("<", pos);
        if (extracted[pos + 1] == '?' || // <?xml...
                extracted[pos + 1] == '!') { // <!DOCTYPE... or [<!ENTITY...
            // Skip this line
            pos = extracted.find_first_of("\n", pos);
        } else
            break;
    }

    extracted = iso_8859_1_to_utf8(extracted.substr(pos), pos);

    // Parse document as usual
    TiXmlDocument doc;
    doc.Parse(extracted.c_str());

    return doc;
}

};

#endif /* IHC_IHCCLIENT_HPP_ */
