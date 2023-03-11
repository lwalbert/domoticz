/*
 * WSDate.hpp
 *
 *  Created on: 30. sep. 2016
 *      Author: Jonas Bo Jalling
 */

#ifndef IHC_WSDATE_HPP_
#define IHC_WSDATE_HPP_

#include <boost/format.hpp>

class WSDate
{
private:
    	int hours;
    	int minutes;
    	int seconds;
    	int year;
    	int day;
    	int monthWithJanuaryAsOne;
public:
    	WSDate(){}

    	WSDate(int hours, int minutes, int seconds, int year, int monthWithJanuaryAsOne)
    	{
    		this->hours = hours;
    		this->minutes = minutes;
    		this->seconds = seconds;
    		this->year = year;
    		this->monthWithJanuaryAsOne = monthWithJanuaryAsOne;
    	}

    	int getHours()
    	{
    		return hours;
    	}

    	void setHours(int hours)
    	{ this->hours = hours; }

    	int getMinutes ()
    	{ return minutes; }

    	void setMinutes(int minutes)
    	{ this->minutes = minutes; }

    	int getSecondss ()
    	{ return seconds; }

    	void setSeconds(int seconds)
    	{ this->seconds = seconds; }

    	int getYear()
    	{ return year; }

    	void setYear(int year)
    	{ this->year = year; }

    	int getDay()
    	{ return day; }

    	void setDay(int day)
    	{ this->day = day; }

    	int getMonthWithJanuaryAsOne()
    	{ return monthWithJanuaryAsOne; }

    	void setMonthWithJanuaryAsOne(int monthWithJanuaryAsOne)
    	{ this->monthWithJanuaryAsOne = monthWithJanuaryAsOne; }

    	/*std::string toString()
    	{
    	    return boost::format("%02i:%02i:%02i %02i/%02i/%02i") %this->hours % this->minutes % this->seconds % this->day % this->monthWithJanuaryAsOne % this->year;
    	}*/

    	friend std::ostream& operator<<(std::ostream& os, const WSDate& proj);
};

std::ostream& operator<<(std::ostream& os, const WSDate& date)
{
    os << boost::format("%02i:%02i:%02i %02i/%02i/%02i") %date.hours % date.minutes % date.seconds % date.day % date.monthWithJanuaryAsOne % date.year;
    //os << "b" << proj.projectMajorRevision << "." << proj.projectMinorRevision << "." << proj.projectNumber << std::endl;
    return os;
}

#endif /* IHC_WSDATE_HPP_ */
