//
// Created by Hugo Trippaers on 02/06/2023.
//

#ifndef EPD_STATION_IEEE802154_H
#define EPD_STATION_IEEE802154_H

#define FRAME_VERSION_STD_2003 0
#define FRAME_VERSION_STD_2006 1
#define FRAME_VERSION_STD_2015 2

#define FRAME_TYPE_BEACON       (0)
#define FRAME_TYPE_DATA         (1)
#define FRAME_TYPE_ACK          (2)
#define FRAME_TYPE_MAC_COMMAND  (3)
#define FRAME_TYPE_RESERVED     (4)
#define FRAME_TYPE_MULTIPURPOSE (5)
#define FRAME_TYPE_FRAGMENT     (6)
#define FRAME_TYPE_EXTENDED     (7)

#define ADDR_MODE_NONE     (0)  // PAN ID and address fields are not present
#define ADDR_MODE_RESERVED (1)  // Reseved
#define ADDR_MODE_SHORT    (2)  // Short address (16-bit)
#define ADDR_MODE_LONG     (3)  // Extended address (64-bit)

#define FRAME_TYPE_BEACON  (0)
#define FRAME_TYPE_DATA    (1)
#define FRAME_TYPE_ACK     (2)
#define FRAME_TYPE_MAC_CMD (3)

typedef struct mac_fcs {
    uint8_t frameType                  : 3;
    uint8_t secure                     : 1;
    uint8_t framePending               : 1;
    uint8_t ackReqd                    : 1;
    uint8_t panIdCompressed            : 1;
    uint8_t rfu1                       : 1;
    uint8_t sequenceNumberSuppression  : 1;
    uint8_t informationElementsPresent : 1;
    uint8_t destAddrType               : 2;
    uint8_t frameVer                   : 2;
    uint8_t srcAddrType                : 2;
} mac_fcs_t;

typedef struct {
	uint8_t mode; // ADDR_MODE_NONE || ADDR_MODE_SHORT || ADDR_MODE_LONG
	union {
		uint16_t short_address;
		uint8_t long_address[8];
	};
} ieee802154_address_t;

typedef struct {
	mac_fcs_t* fcs;
	uint8_t sequence_number;
	uint16_t pan_id;
	uint8_t mode;
	uint16_t short_dst_addr;
	uint16_t short_src_addr;
	uint8_t long_dst_addr[8];
	uint8_t long_src_addr[8];
	size_t data_length;
	uint8_t data[128];
} ieee802154_packet_t;

void ieee802154_analysis_packet(uint8_t* packet, size_t packet_length, ieee802154_packet_t *ieee802154_packet);
uint8_t ieee802154_header(const uint16_t *pan_id, ieee802154_address_t *src, ieee802154_address_t *dst, uint8_t *header, size_t header_length);
void ieee802154_send_long(uint8_t dst_long[8], uint8_t *payload, size_t payload_length);
void ieee802154_send_short(uint16_t dst_short, uint8_t *payload, size_t payload_length);
uint16_t ieee802154_checksum(uint8_t * packet, size_t packet_length);

#endif	// EPD_STATION_IEEE802154_H
