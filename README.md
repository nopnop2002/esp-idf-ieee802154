# esp-idf-ieee802154
IEEE802.15.4 communication example for ESP-IDF.   

ESP-IDF includes [this](https://github.com/espressif/esp-idf/tree/master/components/ieee802154) IEEE802.15.4 communication components.   
But there is no example code in ESP-IDF.   
I used [this](https://github.com/espressif/esp-idf/tree/master/examples/ieee802154/ieee802154_cli) as a reference.   

# Software requirements   
ESP-IDF V5.0 or later.   
ESP-IDF V4.4 release branch reached EOL in July 2024.   

# Hardware requirements
Two sets of ESP32C6 or ESP32H2 development boards.   

# Installation
```
git clone https://github.com/nopnop2002/esp-idf-ieee802154
cd esp-idf-ieee802154
idf.py set-target {esp32c6/esp32h2}
idf.py menuconfig
idf.py flash monitor
```

# Configuration

![config-top](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/46ca2c4e-e91b-4134-aa73-c49f66039f7f)
![config-app-1](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/457aa26b-9f17-4eaf-8f74-0e61ea7adf04)
![config-app-2](https://github.com/nopnop2002/esp-idf-ieee802154/assets/6020549/9b999480-5b50-4617-9d3f-7c5e1e8566ae)

- PAN ID   
 It must be the same on the sending and receiving sides.   
- Channel   
 It must be the same on the sending and receiving sides.   
- My Address   
 It must be unique on the sending and receiving sides.   
- Peer Address   
 0xFFFF is the broadcast address.   
- Show details of received packets   
 When enabled, details of received packets will be displayed.
```
I (2274719) ieee802154: 0x40814ae1   41 88 19 42 42 ff ff 22  22 54 68 69 73 20 69 73  |A..BB..""This is|
I (2274719) ieee802154: 0x40814af1   20 45 53 50 33 32 48 32  dd a5                    | ESP32H2..|
I (2274729) ieee802154: Frame type:                   1
I (2274739) ieee802154: Security Enabled:             False
I (2274739) ieee802154: Frame pending:                False
I (2274749) ieee802154: Acknowledge request:          False
I (2274759) ieee802154: PAN ID Compression:           True
I (2274759) ieee802154: Reserved:                     False
I (2274769) ieee802154: Sequence Number Suppression:  False
I (2274779) ieee802154: Information Elements Present: False
I (2274779) ieee802154: Destination addressing mode:  2
I (2274789) ieee802154: Frame version:                0
I (2274789) ieee802154: Source addressing mode:       2
I (2274799) ieee802154: sequence_number (19)
I (2274799) ieee802154: On PAN 0x4242 to short address 0xffff
I (2274809) ieee802154: Originating from short address 0x2222
I (2274819) ieee802154: Data length: 15
I (2274819) ieee802154: checksum: 0xa5dd
I (2274829) ieee802154: PAN 4242 S 2222 00:00:00:00:00:00:00:00 to ffff 00:00:00:00:00:00:00:00
```
