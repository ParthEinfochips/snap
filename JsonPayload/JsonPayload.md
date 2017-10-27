## Cloud App Connection Mechinism For Different Apps:

#### Connect

##### Command : "connect"

REQUEST=	
```
payload : {

	    "appname":"org.alljoyn.eI.DMA.GW58c3ce458441383a938c5c75.APPDeviceM",
		"clientid":"ff",
		"hostname":"104.236.138.227",
		"username":"iotuser",
		"password":"ei12@",
		"port":1883

}
```
RESPONSE:

```
payload : {

	    "appname":"org.alljoyn.eI.DMA.GW58c3ce458441383a938c5c75.APPDeviceM",
	    "clientid":"ff",
	    "hostname":"104.236.138.227",
	    "username":"iotuser",
	    "password":"ei12@",
	    "port":1883,
        "endpointid":int,
	    "status": std::string,
	    "responsecode": int,
	    "msg": std::string

	  }
```

#### RegisterEndPoint

##### Command : "registerendpoint"
REQUEST=
```
payload : {

		"topic":std::string,
		"endpointid":int
}
```
RESPONSE=
```
payload : {

		"topic":std::string,
		"endpointid":int,
		"msg":std::string,
		"responsecode":200/400,
		"status":std::string

}
```

#### 3) Un-RegisterEndPoint

##### Command : "unregisterendpoint"
REQUEST:
```
payload : {

		"topic":std::string,
		"endpointid":int,

}
```

RESPONSE:
```
payload : {

		"appName":"org.alljoyn.eI.DMA.GW58c3ce458441383a938c5c75.APPDeviceMgmt",
		"topic":std::string,
		"endpointid":int,
		"msg":std::string,
		"responsecode":200/400,
		"status":std::string

	  }
```

#### 4) SendToCloud

###### Command : "sentocloud"

REQUEST=
```
payload : {

		"topic":std::string,
		"endpointid":int,
		...(Meta Data)

}
```
RESPONSE:
```
payload : {

		"topic":std::string,
		"endpointid":int,
		"msg":std::string,
		"responsecode":200/400,
		"status":std::string

	  }
```

#### Disconnect

##### Command : "disconnect"

REQUEST;
```
payload : {

		"appName":"org.alljoyn.eI.DMA.GW58c3ce458441383a938c5c75.APPDeviceMgmt",
		"endpointid":int

	  }
```
RESPONSE:
```
payload : {

		"appName":"org.alljoyn.eI.DMA.GW58c3ce458441383a938c5c75.APPDeviceMgmt",
		"endpointid":int,
		"msg":std::string,
		"responsecode":int,
		"status": std::string

	  }
```
