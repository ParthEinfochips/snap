## MQTTApp topics:



### Operation : Adding broker:

#### Request from cloud:
```
EI/orgID/gatewayID/LocalControl/Configuration/Local/Put
```
#### Response to cloud:
```
EI/orgID/gatewayID/LocalControl/Configuration/Local/Put/Reply/JobID
```
### Operation : Deleting broker:
```
EI/orgID/gatewayID/LocalControl/Configuration/Local/Del
```
#### Response to cloud:
```
EI/orgID/gatewayID/LocalControl/Configuration/Local/Del/Reply/JobID
```
### Operation : Getting UniqueID with respect to android App in order to subscribe to below five topics:

#### Request from cloud:
```
EI/orgID/gatewayID/LocalControl/LocalControl/Android/Provision/Put
```
#### Response to cloud:
```
EI/orgID/gatewayID/LocalControl/LocalControl/Android/Provision/Put/Reply/JobID
```

### Operation : Providing gatewayMeta to android local app.

#### Request from androidApp:
```
EI/Android/uniqueID/GatewayInfo/Get
```
#### Response to androidApp:
```
EI/Android/uniqueID/GatewayInfo/Get/Reply/JobID
```
### Operation : getting device list

#### Request from androidApp:
```
EI/Android/uniqueID/gatewayID/Devices/GetDeviceList
```
#### Response from androidApp:
```
EI/Android/uniqueID/gatewayID/Devices/GetDeviceList/Reply/JobID
```
### Operation : get device RAML:

#### Request from androidApp:
```
EI/Android/uniqueID/gatewayID/Devices/DeviceID/RAML/Get
```
#### Response from androidApp:
```
EI/Android/uniqueID/gatewayID/Devices/DeviceID/RAML/Get/Reply/JobID
```
### Operation : get device state:

#### Request from androidApp:
```
EI/Android/uniqueID/gatewayID/LocalControl/Devices/DeviceID/State/Get
```
#### Response from androidApp:
```
EI/Android/uniqueID/gatewayID/LocalControl/Devices/DeviceID/State/Get/Reply/JobID
```
### Operation : set device state:

#### Request from androidApp:
```
EI/Android/uniqueID/gatewayID/LocalControl/Devices/DeviceID/State/Put
```
#### Response from androidApp:
```
EI/Android/uniqueID/gatewayID/LocalControl/Devices/DeviceID/State/Put/Reply/JobID
```


