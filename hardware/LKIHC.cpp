/*!
 * \file hardware/LKIHC.cpp
 * \author  Jonas Bo Jalling <jonas@jalling.dk>
 * \version 0.1
 *
 * \section LICENSE
 *
 * The MIT License
 *
 * Copyright (c) 2017 Jonas Bo Jalling
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * \section DESCRIPTION
 *
 * This provides a interface to the LK IHC home automation system
 *
 */

/*
 * if (firstRun)
 * {
 *     connect to controller and get project info
 *     if (project info has changed)
 *     {
 *         update database with new items
 *     }
 * }
 *
 * if (watched devices > 0)
 * {
 *     connect to controller
 *     updated watchlist
 *
 *     wait for changes
 *     if changes
 *     {
 *         handle changes
 *     }
 *
 * }
 *
 */
#include "stdafx.h"
#include "LKIHC.h"
#include "../main/Helper.h"
#include "../main/Logger.h"
#include "../main/localtime_r.h"
#include "../main/SQLHelper.h"
#include "../main/mainworker.h"
#include "../main/WebServer.h"
#include "../webserver/cWebem.h"
#include "../httpclient/HTTPClient.h"
#include "hardwaretypes.h"
#include "../tinyxpath/tinyxml.h"
#include "../tinyxpath/xpath_static.h"
#include <boost/shared_ptr.hpp>
#include "IHC/IhcClient.hpp"
#include <string.h>
#include <exception>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
/*#include <boost/bind/bind.hpp>*/
#include <map>

#define RESOURCE_NOTIFICATION_TIMEOUT_S 20
std::vector<int> activeResourceIdList;
CLKIHC::CLKIHC(const int ID, const std::string &IPAddress, const unsigned short Port, const std::string &Username, const std::string &Password) :
    m_IPAddress(IPAddress),
    m_UserName(Username),
    m_Password(Password)
{
    m_HwdID=ID;
    m_Port = Port;
    m_stoprequested=false;
    Init();
}

CLKIHC::~CLKIHC(void)
{
}

ihcClient* ihcC;

void CLKIHC::Init()
{
}

bool CLKIHC::StartHardware()
{
    /*Start worker thread*/
    m_bIsStarted=true;
    sOnConnected(this);
    try {
        ihcC = new ihcClient(m_IPAddress, m_UserName, m_Password);
    }
    catch(...)
    {

        Log(LOG_STATUS,"THIS ERROR SHOULD NEVER OCCUR...");
    }
    m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CLKIHC::Do_Work, this)));

    return (m_thread!=nullptr);
}

bool CLKIHC::StopHardware()
{
    if (m_thread!=nullptr)
    {
        assert(m_thread);
        m_stoprequested = true;
        m_thread->join();
    }
    m_bIsStarted=false;
    return true;
}

using IhcDeviceID = unsigned long int;
using IhcDeviceSerial = long unsigned int;
using IhcDeviceSerialToID = std::multimap<IhcDeviceSerial, IhcDeviceID>;

struct IhcObject {
    IhcDeviceSerial SerialNumber;
    uint8_t Type;
    uint8_t SubType;
    uint8_t RSSI;
    uint8_t Battery;
    IhcObject () : SerialNumber(0), Type(0), SubType(0), RSSI(0), Battery(0) {}
    IhcObject (IhcDeviceSerial serial, uint8_t ttype, uint8_t subtype, int rssi, int battery) :
        SerialNumber(serial), Type(ttype), SubType(subtype), RSSI(rssi), Battery(battery) {}
};

using IhcDeviceTypeCache = std::map<IhcDeviceID, IhcObject>;

IhcDeviceSerialToID IhcDeviceSerialToDeviceID;
IhcDeviceTypeCache DeviceCache;

void CLKIHC::Do_Work()
{
    Log(LOG_STATUS,"Worker started...");
    auto crashCounter = 0;
    auto rssiAndBatteryUpdate = 0;
    auto firstTime = true;
    auto sec_counter = 28;

    while (!m_stoprequested)
    {

        m_LastHeartbeat = mytime(nullptr);

        if (ihcC->CONNECTED != ihcC->connectionState)
        {
            //#ifdef _DEBUG
            Log(LOG_STATUS,"Connecting to IHC controller...");
            //#endif
            try
            {
                if (0 == sec_counter % 30)
                    ihcC->openConnection();
            }
            catch (const char* msg)
            {
                Log(LOG_ERROR, "Error: '%s'", msg);
                ihcC->reset();
                firstTime = true;
            }
            catch(std::exception& e )
            {

                std::string what = e.what();
                Log(LOG_ERROR, "Error: '%s'", what.c_str());
                ihcC->reset();
                firstTime = true;
            }
            sec_counter++;
        }
        else
        {
            try
            {
                if (firstTime)
                {
                    //Log(LOG_STATUS, "IHC First time");
                    firstTime = false;
                    rssiAndBatteryUpdate = 98;
                    activeResourceIdList.clear();
                    DeviceCache.clear();
                    IhcDeviceSerialToDeviceID.clear();

                    // Lets create a device cache and a map for device serialnumbers -> deviceID
                    const auto result = m_sql.safe_query("SELECT DeviceID, Type, SubType, BatteryLevel, SignalLevel, Options FROM DeviceStatus WHERE HardwareID==%d", m_HwdID);
                    if (!result.empty())
                    {
                        //Log(LOG_STATUS, "IHC Creating cache");
                        for (auto itt = result.begin(); itt != result.end(); ++itt)
                        {
                            auto sd = *itt;

                            auto DeviceID = std::strtoul(sd[0].c_str(), nullptr, 16);
                            auto DeviceType = atoi(sd[1].c_str());
                            auto DeviceSubType = atoi(sd[2].c_str());
                            auto DeviceSerialNumber = std::strtoul(sd[5].c_str(), nullptr, 10);
                            auto DeviceBatteryValue = atoi(sd[3].c_str());
                            auto DeviceRSSIValue = atoi(sd[4].c_str());

                            IhcObject IhcDevice (DeviceSerialNumber, DeviceType, DeviceSubType, DeviceBatteryValue, DeviceRSSIValue);
                            DeviceCache.insert(std::make_pair( DeviceID, IhcDevice));

                            if (DeviceSerialNumber != 0)
                            {
                                IhcDeviceSerialToDeviceID.insert(std::make_pair(DeviceSerialNumber, DeviceID));
                            }
                        }
                    }

                    //Log(LOG_STATUS, "Cache length: %d", DeviceCache.size());
                    /*for(std::multimap<IhcDeviceSerial, IhcDeviceID>::iterator iter = IhcSerialList.begin(); iter != IhcSerialList.end(); ++iter)
                      {
                      IhcDeviceSerial k =  (*iter).first;
                      IhcDeviceID l = (*iter).second;
                    //std::cout << k << " .. " << l << std::endl;
                    //std::cout << cache[k].Type << "." << cache[k].SubType << std::endl;
                    //ignore value
                    //Value v = iter->second;
                    }
                    //WSProjectInfo inf = ihcC->getProjectInfo();
                    //Log(LOG_STATUS, "Project info. %s", inf);
                    for(std::multimap<IhcDeviceSerial, IhcDeviceID>::iterator it = IhcSerialList.begin(), end = IhcSerialList.end(); it != end; it = IhcSerialList.upper_bound(it->first))
                    {
                    std::cout << it->first << ' ' << it->second << std::endl;

                    }
                    IhcDeviceSerial id = 109955793651093;
                    auto receiversEntries = IhcSerialList.equal_range(id);
                    for (auto receiverEntry = receiversEntries.first; receiverEntry != receiversEntries.second; ++receiverEntry)
                    {

                    std::cout << receiverEntry->second << std::endl;
                    }

*/
                }


                const auto result = m_sql.safe_query("SELECT DeviceID FROM DeviceStatus WHERE HardwareID==%d AND Used == 1", m_HwdID);
                if (!result.empty())
                {
                    if (ihcC->CONNECTED != ihcC->connectionState)
                    {
                        ihcC->openConnection();
                    }

                    if (rssiAndBatteryUpdate % 100 == 0)
                        UpdateBatteryAndRSSI();
                    rssiAndBatteryUpdate++;

                    std::vector<int> resourceIdList;
                    for (auto itt = result.begin(); itt != result.end(); ++itt)
                    {
                        std::vector<std::string> sd = *itt;
                        IhcDeviceID DeviceID = std::strtoul(sd[0].c_str(), nullptr, 16);
                        resourceIdList.push_back(std::strtoul(sd[0].c_str(), nullptr, 16));
                    }

                    if (resourceIdList != activeResourceIdList)
                    {
                        activeResourceIdList = resourceIdList;
                        ihcC->enableRuntimeValueNotification(activeResourceIdList);
                        //Log(LOG_STATUS, "Updating listener resource list with %d elements", activeResourceIdList.size());
                    }

                    const auto updatedResources = ihcC->waitResourceValueNotifications(RESOURCE_NOTIFICATION_TIMEOUT_S);

                    // Handle object state changes
                    for (auto  it = updatedResources.begin(); it != updatedResources.end(); ++it)
                    {
                        ResourceValue & obj = *(*it);

                        char szID[10];
                        std::sprintf(szID, "%08lX", (long unsigned int)obj.ID);

                        auto device = DeviceCache.find(obj.ID);
                        if (device != DeviceCache.end())
                        {
                            _tGeneralSwitch ycmd;
                            ycmd.subtype = (*device).second.SubType;
                            ycmd.rssi = (*device).second.RSSI;
                            ycmd.id =  (long unsigned int)obj.ID;
                            ycmd.unitcode = 0;
                            if (obj.intValue() > 1)
                            {
                                //ycmd.cmnd = 2;
				//ycmd.cmnd = gswitch_sOn;
				ycmd.cmnd = gswitch_sSetLevel;
	                        ycmd.level = obj.intValue();
				//Log(LOG_STATUS, "logics say ON, update level %i",ycmd.level);
                            }
                            else
                            {
				//ycmd.cmnd = gswitch_sSetLevel;
				ycmd.cmnd = gswitch_sOff;
                                //ycmd.cmnd = obj.intValue();
                                //ycmd.level= 0;
				//Log(LOG_STATUS, "logics say OFF, an the level is %i", ycmd.level);
                            }
                            m_mainworker.PushAndWaitRxMessage(this, (const unsigned char *)&ycmd, nullptr, (*device).second.Battery, m_Name.c_str());
                        }
                    }
                }
                else
                {
                    Log(LOG_STATUS, "No devices active - disconnecting");
                    sleep_seconds(10);
                    ihcC->reset();
                    sec_counter = 0;
                }
            }
            catch (ihcException &e)
            {
                Log(LOG_ERROR,"IHC_RESULT: ------------- START -----------------");
                //Log(LOG_ERROR,"IHC DUMP: %s", sResult.c_str());
                //std::cout << "got ihc exception\n";
                Log(LOG_ERROR,"IHC DUMP Query: %s", e.get_xmla().c_str());
                Log(LOG_ERROR,"IHC_RESULT: - - - - - - - - - - - - - - - - - - -");
                Log(LOG_ERROR,"IHC DUMP Response: %s", e.get_xmlb().c_str());
                Log(LOG_ERROR,"IHC_RESULT: ------------- END -------------------");
                crashCounter++;
                ihcC->reset();
                sec_counter = 0;
                firstTime = true;
            }
            catch (...)
            {
                crashCounter++;
                //Log(LOG_ERROR, "LK IHC: The IHC controller has crashed %d times since Domoticz was started", crashCounter);
                //ihcC->reset();
                sec_counter = 0;
                firstTime = true;
            }
        }
        sleep_seconds(1);
    }

    Log(LOG_STATUS,"Worker stopped...");
}

bool CLKIHC::WriteToHardware(const char *pdata, const unsigned char length)
{
    if (ihcC->CONNECTED == ihcC->connectionState)
    {
        auto result = false;
        const auto *pSen = reinterpret_cast<const tRBUF*>(pdata);

        try
        {
            if (pSen->ICMND.packettype == pTypeGeneralSwitch)
            {
                const auto *general = reinterpret_cast<const _tGeneralSwitch*>(pdata);
                switch (pSen->ICMND.subtype)
                {

                    case sSwitchIHCFBInput:
                        {
                            /* Boolean value */
                            ResourceValue const output_value(general->id, general->cmnd == gswitch_sOn ? true : false);
                            result = ihcC->resourceUpdate(output_value);
                            break;
                        }
                    case sSwitchIHCFBOutput:
                        return false;
                        break;

                    case sSwitchIHCOutput:
                    case sSwitchIHCInput:
                        {
                            /* Boolean value */
                            ResourceValue const output_value(general->id, general->cmnd == gswitch_sOn ? true : false);
                            result = ihcC->resourceUpdate(output_value);
                            break;
                        }

                    case sSwitchIHCDimmer:
                    //case sSwitchGeneralSwitch:
                        {
                            /* Integer value */
                            auto dimmer_value = 0;
                            if (general->cmnd == gswitch_sOff)
                            {
                                dimmer_value = 0;
                            }
                            else if (general->cmnd == gswitch_sOn)
                            {
                                dimmer_value = general->level; /*40;*/
                            }
                            else if (general->cmnd == gswitch_sSetLevel)
                            {
                                dimmer_value = general->level;
                            }
                            ResourceValue const output_value(general->id, RangedInteger(dimmer_value));
                            result = ihcC->resourceUpdate(output_value);
                            break;
                        }
                }
            }

            if (result)
            {
                Log(LOG_STATUS, "Resource update was successful");
            }
            else
            {
                Log(LOG_STATUS, "Failed resource update");
            }
        }
        catch (const char* msg)
        {
            Log(LOG_ERROR, "Error: '%s'", msg);
            ihcC->reset();
        }

        return result;
    }
    else
    {
        // Not connected to controller
        return false;
    }
}

void CLKIHC::addDeviceIfNotExists(const TiXmlNode* device, const unsigned char deviceType, bool isFunctionBlock)
{
    IhcDeviceSerial serialNumber = 0;
    const auto deviceID = std::string(device->ToElement()->Attribute("id")).substr(3);

    char sid[10];
    sprintf(sid, "%08X", (std::stoul(deviceID, nullptr, 16)));

    const auto result = m_sql.safe_query("SELECT ID FROM DeviceStatus WHERE (HardwareID==%d) AND (DeviceID=='%q')",
            m_HwdID, sid);
    if (result.size() < 1)
    {
        //        #ifdef _DEBUG
        Log(LOG_NORM, "Added device %d %s", m_HwdID, sid);
        //        #endif
        char buff[100];

        if (isFunctionBlock)
        {
            snprintf(buff, sizeof(buff), "FB | %s | %s | %s",
                    device->Parent()->ToElement()->Parent()->ToElement()->Parent()->ToElement()->Attribute("name"),
                    device->Parent()->ToElement()->Parent()->ToElement()->Attribute("name"),
                    device->ToElement()->Attribute("name"));
        }
        else
        {
            snprintf(buff, sizeof(buff), "%s | %s | %s",
                    device->Parent()->ToElement()->Parent()->ToElement()->Attribute("name"),
                    device->Parent()->ToElement()->Attribute("position"),
                    device->ToElement()->Attribute("name"));

            // If it's a wireless device, get the serial number so we can use it for the RSSI and battery level mappings
            if (device->Parent()->ToElement()->Attribute("serialnumber") != 0)
            {
                const auto serialNumber_raw = std::string(device->Parent()->ToElement()->Attribute("serialnumber")).substr(3);
                serialNumber = std::strtoul(serialNumber_raw.c_str(), nullptr, 16);
            }
        }
        _eSwitchType device;
        switch (deviceType)
        {
            case sSwitchIHCFBOutput:
            case sSwitchIHCInput:
                device = STYPE_Contact;
                break;
            case sSwitchIHCFBInput:
                device = STYPE_PushOn;
                break;
            case sSwitchIHCDimmer:
                device = STYPE_Dimmer;
                break;
            case sSwitchIHCOutput:
                device = STYPE_OnOff;
                break;
            default:
                return;
        }

        m_sql.safe_query(
                "INSERT INTO DeviceStatus (HardwareID, DeviceID, Type, SubType, SwitchType, SignalLevel, BatteryLevel, Name, nValue, AddjValue, AddjValue2,  sValue, Options) "
                "VALUES (%d,'%q',%d,%d,%d,12,255,'%q',0, '0.0', '0.0', '', '%lld')",
                m_HwdID, sid, pTypeGeneralSwitch, deviceType, device, buff,1.0,1.0, serialNumber);
    }
    /*else
      {
    // TODO: Remove when merging to master
    // Device already exists - add serialnumber
    // If it's a wireless device, get the serial number so we can use it for the RSSI and battery level mappings
    if (device->Parent()->ToElement()->Attribute("serialnumber") != 0)
    {
    std::string const serialNumber_raw = std::string(device->Parent()->ToElement()->Attribute("serialnumber")).substr(3);
    serialNumber = std::strtoul(serialNumber_raw.c_str(), nullptr, 16);
    m_sql.safe_query("UPDATE DeviceStatus SET Options = '%lld' WHERE HardwareID = '%d' AND DeviceID = '%q'", serialNumber, m_HwdID, sid);

    }
    }*/
}

void CLKIHC::iterateDevices(const TiXmlNode* deviceNode)
{
    bool isFunctionBlock = false;
    unsigned char deviceType = -1;

    if (strcmp(deviceNode->Value(), "airlink_dimming") == 0)
    {
        deviceType = sSwitchIHCDimmer;
    }
    else if ((strcmp(deviceNode->Value(), "airlink_relay") == 0) || (strcmp(deviceNode->Value(), "dataline_output") == 0))
    {
        deviceType = sSwitchIHCOutput;
    }
    else if ((strcmp(deviceNode->Value(), "airlink_input") == 0) || (strcmp(deviceNode->Value(), "dataline_input") == 0))
    {
        deviceType = sSwitchIHCInput;
    }
    else if (strcmp(deviceNode->Value(), "resource_input") == 0)
    {
        deviceType = sSwitchIHCInput;
        isFunctionBlock = true;
    }
    else if (strcmp(deviceNode->Value(), "resource_output") == 0)
    {
        deviceType = sSwitchIHCOutput;
        isFunctionBlock = true;
    }
    else if (strcmp(deviceNode->Value(), "resource_temperature") == 0)
    {
        deviceType = sSwitchIHCOutput;
        isFunctionBlock = true;
    }

    if (deviceType != -1)
        addDeviceIfNotExists(deviceNode, deviceType, isFunctionBlock);

    for (const TiXmlNode* node = deviceNode->FirstChild(); node; node = node->NextSibling())
    {
        iterateDevices(node);
    }
}
void CLKIHC::logout()
{
    std::cout << "Reset"<< std::endl;
    ihcC->ihclogout();
}
void CLKIHC::GetDevicesFromController()
{
    if (ihcC->CONNECTED != ihcC->connectionState) {
        ihcC->openConnection();
    }
    TiXmlDocument doc = ihcC->loadProject();

    TinyXPath::xpath_processor processor ( doc.RootElement(), "/utcs_project/groups/*/*[self::functionblock or self::product_dataline or self::product_airlink]");

    unsigned const numberOfDevices = processor.u_compute_xpath_node_set();

    for (int i = 0; i < numberOfDevices; i++)
    {
        TiXmlNode* thisNode = processor.XNp_get_xpath_node(i);
        iterateDevices(thisNode);
    }
}
std::string CLKIHC::getValue( TiXmlElement* const a, std::string const t)
{
    TinyXPath::xpath_processor proc(a, t.c_str());
    if (1 == proc.u_compute_xpath_node_set())
    {
        TiXmlNode* thisNode = proc.XNp_get_xpath_node(0);
        std::string res = thisNode->ToElement()->GetText();
        return res;
    }
    return "";
}
bool CLKIHC::UpdateBatteryAndRSSI()
{
    Log(LOG_STATUS, "Updating battery and RSSI levels");
    TiXmlDocument const RFandRSSIinfo =  ihcC->getRF();
    TinyXPath::xpath_processor processor ( RFandRSSIinfo.RootElement(), "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:getDetectedDeviceList1/ns1:arrayItem");
    processor.u_compute_xpath_node_set(); // <-- this is important. It executes the Xpath expression
    if (processor.XNp_get_xpath_node(0)->FirstChild() != 0)
    {
        for (int i = 0; i < processor.u_compute_xpath_node_set(); i++)
        {
            TiXmlNode* const thisNode = processor.XNp_get_xpath_node(i);
            TiXmlElement * const res = thisNode->FirstChild()->ToElement();
            int const batteryLevel = boost::lexical_cast<int>(getValue(res, "/ns1:batteryLevel")) == 1 ? 100 : 9;
            int const signalStrength = (int)(boost::lexical_cast<int>(getValue(res, "/ns1:signalStrength"))>>2);
            IhcDeviceSerial serialNumber   = boost::lexical_cast<unsigned long>(getValue(res, "/ns1:serialNumber"));

            auto receiversEntries = IhcDeviceSerialToDeviceID.equal_range(serialNumber);
            for (auto receiverEntry = receiversEntries.first; receiverEntry != receiversEntries.second; ++receiverEntry)
            {
                DeviceCache[receiverEntry->second].Battery = batteryLevel;
                DeviceCache[receiverEntry->second].RSSI = signalStrength;
            }
            m_sql.safe_query("UPDATE DeviceStatus SET BatteryLevel=%d, SignalLevel=%d WHERE (HardwareID==%d) AND (Options==%lld)",
                    batteryLevel, signalStrength, m_HwdID, serialNumber);
        }
        return true;
    }
    return false;
}

//Webserver helpers
namespace http {
    namespace server {

        void CWebServer::GetIHCProjectFromController(WebEmSession & session, const request& req, std::string & redirect_uri)
        {
            redirect_uri = "/index.html";
            if (session.rights != 2)
            {
                //No admin user, and not allowed to be here
                return;
            }

            std::string idx = request::findValue(&req, "idx");
            if (idx == "") {
                return;
            }

            int iHardwareID = atoi(idx.c_str());
            CDomoticzHardwareBase *pBaseHardware = m_mainworker.GetHardware(iHardwareID);
            if (pBaseHardware == nullptr)
                return;
            if (pBaseHardware->HwdType != HTYPE_IHC)
                return;
            CLKIHC *pHardware = reinterpret_cast<CLKIHC*>(pBaseHardware);

            try
            {
                pHardware->GetDevicesFromController();
            }
            catch (const char* msg)
            {
                _log.Log(LOG_ERROR, "IHC: Exception: '%s'", msg);
            }
        }
        void CWebServer::debugLogout(WebEmSession & session, const request& req, std::string & redirect_uri)
        {
            redirect_uri = "/index.html";
            if (session.rights != 2)
            {
                //No admin user, and not allowed to be here
                return;
            }

            std::string idx = request::findValue(&req, "idx");
            if (idx == "") {
                return;
            }

            int iHardwareID = atoi(idx.c_str());
            CDomoticzHardwareBase *pBaseHardware = m_mainworker.GetHardware(iHardwareID);
            if (pBaseHardware == nullptr)
                return;
            if (pBaseHardware->HwdType != HTYPE_IHC)
                return;
            CLKIHC *pHardware = reinterpret_cast<CLKIHC*>(pBaseHardware);

            try
            {
                pHardware->logout();
            }
            catch (const char* msg)
            {
                _log.Log(LOG_ERROR, "IHC: Exception: '%s'", msg);
            }
        }

    }
}
