/*
 * WSLoginResult.hpp
 *
 *  Created on: 30. sep. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_WSLOGINRESULT_HPP_
#define IHC_WSLOGINRESULT_HPP_

class WSLoginResult : public WSBaseDataType
{
private:
    WSUser loggedInUser;
    bool loginWasSuccessful;
    bool loginFailedDueToConnectionRestrictions;
    bool loginFailedDueToInsufficientUserRights;
    bool loginFailedDueToAccountInvalid;

public:
    WSLoginResult() { }

    WSUser getLoggedInUser()
    { return loggedInUser; }

    void setLoggedInUser(WSUser value)
    { this->loggedInUser = value; }

    bool isLoginWasSuccessful()
    { return loginWasSuccessful; }

    void setLoginWasSuccessful(bool value)
    { this->loginWasSuccessful = value; }

    bool isLoginFailedDueToConnectionRestrictions()
    { return loginFailedDueToConnectionRestrictions; }

    void setLoginFailedDueToConnectionRestrictions(bool value)
    { this->loginFailedDueToConnectionRestrictions = value; }

    bool isLoginFailedDueToInsufficientUserRights()
    { return loginFailedDueToInsufficientUserRights; }

    void setLoginFailedDueToInsufficientUserRights(bool value)
    { this->loginFailedDueToInsufficientUserRights = value; }

    bool isLoginFailedDueToAccountInvalid()
    { return loginFailedDueToAccountInvalid; }

    void setLoginFailedDueToAccountInvalid(bool value)
    { this->loginFailedDueToAccountInvalid = value; }

    void encodeData(TiXmlDocument data)
    {

        loginWasSuccessful = parseValueToBoolean(data,
                "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:authenticate2/ns1:loginWasSuccessful");

        loginFailedDueToConnectionRestrictions = parseValueToBoolean(data,
                "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:authenticate2/ns1:loginFailedDueToConnectionRestrictions");

        loginFailedDueToInsufficientUserRights = parseValueToBoolean(data,
                "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:authenticate2/ns1:loginFailedDueToInsufficientUserRights");

        loginFailedDueToAccountInvalid = parseValueToBoolean(data,
                "/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:authenticate2/ns1:loginFailedDueToAccountInvalid");
    }

};

#endif /* IHC_WSLOGINRESULT_HPP_ */
