# some files for esp8266

## https://www.espressif.com/zh-hans/products/hardware/esp8266ex/resources


# Steps for link server by AT
1. AT+CWMODE=1
      set esp8266 into station mode
  return:
      OK
2. AT+CWJAP=“wifiname”,“wifipassword”
      connect to wifi
   return:
      OK
3. AT+CIPSTART=“TCP”,”192.168.1.1”,8880
      connect to server
   return:
      CONNECT

4. AT+CIPMODE=1
      set esp8266 into transparant mode
   return:
      OK
      
5. AT+CIPSEND
      set esp8266 into recive mode

