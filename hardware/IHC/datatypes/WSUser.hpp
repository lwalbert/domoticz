/*
 * WSUser.hpp
 *
 *  Created on: 30. sep. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_WSUSER_HPP_
#define IHC_WSUSER_HPP_

#include "WSUserGroup.hpp"
#include "WSDate.hpp"
class WSUser {
private:
    WSDate createdDate;
    WSDate loginDate;
    std::string username;
    std::string password;
    std::string email;
    std::string firstname;
    std::string lastname;
    std::string phone;
    WSUserGroup group;
    std::string project;

public:
    WSDate getCreatedDate()
    { return createdDate; }

    void setCreatedDate(WSDate value)
    { this->createdDate = value; }

    WSDate getLoginDate()
    { return loginDate; }

    void setLoginDate(WSDate value)
    { this->loginDate = value; }

    std::string getUsername()
    { return username; }

    void setUsername(std::string value)
    { this->username = value; }

    std::string getPassword()
    { return password; }

    void setPassword(std::string value)
    { this->password = value; }

    std::string getEmail()
    { return email; }

    void setEmail(std::string value)
    { this->email = value; }

    std::string getFirstname()
    { return firstname; }

    void setFirstname(std::string value)
    { this->firstname = value; }

    std::string getLastname()
    { return lastname; }

    void setLastname(std::string value)
    { this->lastname = value; }

    std::string getPhone()
    { return phone; }

    void setPhone(std::string value)
    { this->phone = value; }

    WSUserGroup getGroup()
    { return group; }

    void setGroup(WSUserGroup value)
    { this->group = value; }

    std::string getProject()
    { return project; }

    void setProject(std::string value)
    { this->project = value; }

};

#endif /* IHC_WSUSER_HPP_ */
