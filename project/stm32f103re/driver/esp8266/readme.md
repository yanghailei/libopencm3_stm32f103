# some files for esp8266

## https://www.espressif.com/zh-hans/products/hardware/esp8266ex/resources

## 4b-esp8266_at_command_examples_cn.pdf

## AT bin Version
```
ESP8266 AT Bin V1.7.0
基于 ESP8266_NonOS_SDK V3.0.0。

受 AT bin 的大小限制，默认仅支持 `1024+1024 flash map` 。
```

# Steps for link server by AT
1. AT+CWMODE=1
>> set esp8266 into station mode
> return:
>> OK
2. AT+CWJAP=“wifiname”,“wifipassword”
>> connect to wifi
> return:
>> OK
3. AT+CIPSTART=“TCP”,”192.168.1.1”,8880
>> connect to server
> return:
>> CONNECT

4. AT+CIPMODE=1
>> set esp8266 into transparant mode
> return:
>> OK
      
5. AT+CIPSEND
>> set esp8266 into recive mode
> return:
>> ">"
