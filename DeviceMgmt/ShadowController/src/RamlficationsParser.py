#!/usr/bin/python
import ramlfications
import json



################################### main parser function #################################################

def ParseRAMLFile_shadow(ramlFile):

	
	dict_schemas = {}
	dict_properties = {}
	#dict_properties["properties"] = []
	key_properties_2 = []
	dict_schema_in_raml = {}
	
	try:
		#	using ramlfication library start parsing.

		api = ramlfications.parse(ramlFile)

		#	schemas parse using ramlfication object.

		schemaslist = api.schemas

		#	resources parse using ramlfication object

		resourcesApi = api.resources
		
		#	json schemas reference path resolve.

		for key_1 in range(len(schemaslist)):
			for key_2 in schemaslist[key_1]:
				dict_schemas[key_2] = api.schemas[key_1][key_2]
				
		
		count_resources_list = 0

		#	list of resources
		#dict_properties["device_status"] = None
		for key_value_resources in resourcesApi	:
			count_resources_list= count_resources_list + 1
			
			if api.resources[count_resources_list - 1].responses != None :	
				#	list of responces for get schemas
				for responses_list_variable in api.resources[count_resources_list - 1].responses:
					if responses_list_variable.body[0].schema in dict_schemas:

						if responses_list_variable.body[0].schema not in dict_schema_in_raml:
								dict_schema_in_raml[responses_list_variable.body[0].schema] = []

						if (key_value_resources.method not in dict_schema_in_raml[responses_list_variable.body[0].schema]) :
							dict_schema_in_raml[responses_list_variable.body[0].schema].append(key_value_resources.method)
							#	find match schemas from json dict schemas

							if ("anyOf" in dict_schemas[responses_list_variable.body[0].schema]) or ("allOf" in dict_schemas[responses_list_variable.body[0].schema]) or ("definitions" in dict_schemas[responses_list_variable.body[0].schema]):
									
								path_defination_1 = dict_schemas[responses_list_variable.body[0].schema]["definitions"]
							else: 
									
								path_defination_1 = dict_schemas[responses_list_variable.body[0].schema]
	

							for key_properties_1 in path_defination_1:									
								if ("anyOf" in dict_schemas[responses_list_variable.body[0].schema]) or ("allOf" in dict_schemas[responses_list_variable.body[0].schema]) or ("definitions" in dict_schemas[responses_list_variable.body[0].schema]):
									path_defination = dict_schemas[responses_list_variable.body[0].schema]["definitions"][key_properties_1]	
								else: 
									path_defination = dict_schemas[responses_list_variable.body[0].schema]

								for count_properties in range(len(path_defination["properties"].keys())):
									if path_defination["properties"].keys()[count_properties] not in key_properties_2:
										key_properties_2 = path_defination["properties"].keys()[count_properties]

									temp_dict_device_object = {}
									#	condition check for main properties
									if(key_value_resources.path.count('/') == 1 and key_value_resources.path != "/notify"):
										if key_properties_2 not in dict_properties:
											dict_properties[key_properties_2] = {}
							
										#if key_value_resources.method == "get":
										dict_properties[key_properties_2]["reported_value"] = None
										if key_value_resources.method == "post":
											dict_properties[key_properties_2]["desired_value"] = []
			

									#	condition check for sub-properties 
								
									if(key_value_resources.path.count('/') == 2):	
										#print dict_properties
										key_find_subresources = key_value_resources.path[key_value_resources.path[1:].index("/")+2:]
										if key_value_resources.path[key_value_resources.path[1:].index("/")+2:] in dict_properties:
											temp_dict_device_object[key_find_subresources]	= dict_properties[key_value_resources.path[key_value_resources.path[1:].index("/")+2:]]
											print "delete" , str (key_find_subresources)
											del dict_properties[key_find_subresources]
																	
										#print temp_dict_device_object
										
										print "after delete"
										if "subproperties" not in temp_dict_device_object[key_find_subresources]:
											temp_dict_device_object[key_find_subresources]["subproperties"] = {}
												
	 									if  key_properties_2 not in temp_dict_device_object[key_find_subresources]["subproperties"]:
											temp_dict_device_object[key_find_subresources]["subproperties"][key_properties_2] = {}
											#if key_value_resources.method == "get":
										temp_dict_device_object[key_find_subresources]["subproperties"][key_properties_2]["reported_value"] = None
										if key_value_resources.method == "post":
											temp_dict_device_object[key_find_subresources]["subproperties"][key_properties_2]["desired_value"] = []
										print temp_dict_device_object
								#print temp_dict_device_object
								#print dict_properties
									print "key list" ,temp_dict_device_object.keys()
									if temp_dict_device_object.keys()!=[]:
									#print temp_dict_device_object.keys()[count_properties]
										print "add"
										dict_properties[temp_dict_device_object.keys()[0]] = temp_dict_device_object[temp_dict_device_object.keys()[0]]
								
		
		#	return json object.
		return json.dumps(dict_properties)
		
	except:
		
		print "Wrong RAML file"
		return None


#print ParseRAMLFile_shadow("/home/vipul/gunjan/iot/gunjan/RAML/RAML_Generator/PythonCode/test.raml")
