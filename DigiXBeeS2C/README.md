# Digi XBee S2C
Digi XBee S2C supports ZigBee/DigiMesh/IEEE 802.15.4 protocols and three firmwares are available for each protocol.   
ESP-IDF can communicate with Digi Xbee S2C 802.15.4 firmware.   

![digi-xbee-s2c](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/82b1ebdb-f292-415e-9259-fb2f8a17b5d7)

# How to update firmware
Use the XCTU application to update firmware.   
XCTU applications can be freely downloaded.   

Start XCTU and select a port.   
![digi-xbee-s2c-1](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/cf966e74-c251-4885-a546-e082466bd835)   
![digi-xbee-s2c-2](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/22ce3724-4c50-45f3-8436-085d962a5232)   
![digi-xbee-s2c-3](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/ec3fb1f4-121a-405d-8ff2-f082d5bfe797)   
![digi-xbee-s2c-4](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/da5dadc4-33ce-46d8-b308-2b82850f8997)   
![digi-xbee-s2c-5](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/15c7a802-6267-4780-9e0b-0c9f4c27ca1b)   

Update firmware.   
![digi-xbee-s2c-6](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/552db5d4-bf95-4469-897e-9184050b4da1)   

Select 802.15.4 TH   
![digi-xbee-s2c-7](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/609aedec-dc35-4112-a8a9-e3e8fc534491)   
![digi-xbee-s2c-8](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/6fe7f098-69e6-4325-8066-68ba5aca817b)   

# How to send packet.
Change Channel,PAN ID,Destination Address Low,16-bit Source Address.   
![digi-xbee-s2c-9](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/94fe2d0f-7c4c-4931-8d36-2ff931a54b10)

Open terminal.   
![digi-xbee-s2c-10](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/c61a0ddb-d8a3-4fc9-90e7-6bc5a2974a84)   

Open port.   
Now you are ready to receive.   
![digi-xbee-s2c-11](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/97b39733-1dc5-4db0-94c3-cf26ef82e7e8)   

Add send packet.   
![digi-xbee-s2c-12](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/66ef22ce-ade6-4265-94fc-7009abab099c)   
![digi-xbee-s2c-13](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/24108a63-d753-4bc8-9c19-225317dc190e)   

Send selected packet.   
![digi-xbee-s2c-14](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/ccfed819-dc3e-4df1-8b4e-22a7338341f7)   


# Shield for UNO
You can get such shields from eBay or AliExpress.   
The output to the UNO's Serial port becomes the input to the wireless module, and the output from the wireless module becomes the input to the UNO's Serial port.   
UNO and ESP32 can communicate directly via XBee S2C.   
![ForUno](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/2a749423-80c6-4ffb-8c96-dd9419d9fe3d)

# Base board for NANO
You can get such base board from eBay or AliExpress.   
The output to the NANO's Serial port becomes the input to the wireless module, and the output from the wireless module becomes the input to the NANO's Serial port.   
NANO and ESP32 can communicate directly via XBee S2C.   
![ForNano](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/d596b8e9-fe05-4345-bcdf-d692ef8de8a5)
