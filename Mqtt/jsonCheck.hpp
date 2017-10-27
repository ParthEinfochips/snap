#ifndef _JSON_CHECKER_H
#define _JSON_CHECKER_H

#include "common/json.hpp"	// For JSON Data
#include <iostream>
using json = nlohmann::json;


/**
 * Check JSON & fill the data in the provided variable
 * If the json packet contains data that does not matches the data-type of variable the function throws error
 **/


/*
 * Returns
 *  0: Success
 * 	1: Failure
 * 	2: Invalid datatype
 */
struct jsonChecker
{
    template <typename T>
    static int CheckJson(json &j, T &data, std::initializer_list<std::string> &&list)
	{
	    std::string s = "";
	    for( auto elem : list )
	    {
	        s += "/";
	        s += elem;        
	    }
	    
	    //~ displayValue(s);
	    try
        {
			auto val = j.at(json::json_pointer(s));	
			if(typeid(data) == typeid(std::string))
			{
				if(!val.is_string())
					return 2;
				
			}
			else if(typeid(data) == typeid(int))
			{
				if(!val.is_number_integer())
					return 2;
			}
			else if(typeid(data) == typeid(bool))
			{
				if(!val.is_boolean())
					return 2;
			}
			
			data = val;
			return 0;
			
	    }
	    catch (std::exception)
        {
            return 1;
	    }
	}
};

#endif // _JSON_CHECKER_H
