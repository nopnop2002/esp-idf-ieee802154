//
// Created by Hugo Trippaers on 02/06/2023.
//

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "esp_ieee802154.h"
#include "esp_log.h"
#include "ieee802154.h"

#define TAG "ieee802154"

static void reverse_memcpy(uint8_t *restrict dst, const uint8_t *restrict src, size_t n);

void ieee802154_analysis_packet(uint8_t* packet, size_t packet_length, ieee802154_packet_t *ieee802154_packet) {
	if (packet_length < sizeof(mac_fcs_t)) return;	// Can't be a packet if it's shorter than the frame control field

	uint8_t position = 0;

#if !CONFIG_PRINT_PACKET
	esp_log_level_set(TAG, ESP_LOG_WARN);
#endif

	mac_fcs_t* fcs = (mac_fcs_t*) &packet[position];
	position += sizeof(uint16_t);
	ieee802154_packet->fcs = fcs;
	ESP_LOGI(TAG, "-----------------------");
	ESP_LOG_BUFFER_HEXDUMP(TAG, packet, packet_length, ESP_LOG_INFO);
	ESP_LOG_BUFFER_HEXDUMP(TAG, fcs, sizeof(uint16_t), ESP_LOG_DEBUG);

	ESP_LOGI(TAG, "Frame type:                   %x", fcs->frameType);
	ESP_LOGI(TAG, "Security Enabled:             %s", fcs->secure ? "True" : "False");
	ESP_LOGI(TAG, "Frame pending:                %s", fcs->framePending ? "True" : "False");
	ESP_LOGI(TAG, "Acknowledge request:          %s", fcs->ackReqd ? "True" : "False");
	ESP_LOGI(TAG, "PAN ID Compression:           %s", fcs->panIdCompressed ? "True" : "False");
	ESP_LOGI(TAG, "Reserved:                     %s", fcs->rfu1 ? "True" : "False");
	ESP_LOGI(TAG, "Sequence Number Suppression:  %s", fcs->sequenceNumberSuppression ? "True" : "False");
	ESP_LOGI(TAG, "Information Elements Present: %s", fcs->informationElementsPresent ? "True" : "False");
	ESP_LOGI(TAG, "Destination addressing mode:  %x", fcs->destAddrType);
	ESP_LOGI(TAG, "Frame version:                %x", fcs->frameVer);
	ESP_LOGI(TAG, "Source addressing mode:       %x", fcs->srcAddrType);

	if (fcs->rfu1) {
		ESP_LOGE(TAG, "Reserved field 1 is set, ignoring packet");
		return;
	}

	ESP_LOGD(TAG, "fcs->frameType=%d", fcs->frameType);
	switch (fcs->frameType) {
		case FRAME_TYPE_BEACON:
		{
			ESP_LOGI(TAG, "Beacon");
			break;
		}
		case FRAME_TYPE_DATA:
		{
			uint8_t sequence_number = packet[position];
			ieee802154_packet->sequence_number = sequence_number;
			position += sizeof(uint8_t);
			ESP_LOGI(TAG, "sequence_number (%x)", sequence_number);

			uint16_t pan_id			= 0;
			uint8_t  dst_addr[8]	= {0};
			uint8_t  src_addr[8]	= {0};
			uint16_t short_dst_addr = 0;
			uint16_t short_src_addr = 0;
			bool	 broadcast		= false;

			ESP_LOGD(TAG, "fcs->destAddrType=%d", fcs->destAddrType);
			switch (fcs->destAddrType) {
				case ADDR_MODE_NONE:
				{
					ESP_LOGI(TAG, "Without PAN ID or address field");
					break;
				}
				case ADDR_MODE_SHORT:
				{
					pan_id = *((uint16_t*) &packet[position]);
					ieee802154_packet->pan_id = pan_id;
					position += sizeof(uint16_t);
					ieee802154_packet->mode = ADDR_MODE_SHORT;
					short_dst_addr = *((uint16_t*) &packet[position]);
					ieee802154_packet->short_dst_addr = short_dst_addr;
					position += sizeof(uint16_t);
					if (pan_id == 0xFFFF && short_dst_addr == 0xFFFF) {
						broadcast = true;
						pan_id	  = *((uint16_t*) &packet[position]);  // srcPan
						position += sizeof(uint16_t);
						ESP_LOGI(TAG, "Broadcast on PAN %04x", pan_id);
					} else {
						ESP_LOGI(TAG, "On PAN 0x%04x to short address 0x%04x", pan_id, short_dst_addr);
					}
					break;
				}
				case ADDR_MODE_LONG:
				{
					pan_id = *((uint16_t*) &packet[position]);
					ieee802154_packet->pan_id = pan_id;
					position += sizeof(uint16_t);
					ieee802154_packet->mode = ADDR_MODE_LONG;
					for (uint8_t idx = 0; idx < sizeof(dst_addr); idx++) {
						dst_addr[idx] = packet[position + sizeof(dst_addr) - 1 - idx];
					}
					position += sizeof(dst_addr);
					memcpy(ieee802154_packet->long_dst_addr, dst_addr, sizeof(dst_addr));
					ESP_LOGI(TAG, "On PAN 0x%04x to long address %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", pan_id, dst_addr[0],
							 dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4], dst_addr[5], dst_addr[6], dst_addr[7]);
					break;
				}
				default:
				{
					ESP_LOGE(TAG, "With reserved destination address type, ignoring packet");
					return;
				}
			}

			ESP_LOGD(TAG, "fcs->srcAddrType=%d", fcs->srcAddrType);
			switch (fcs->srcAddrType) {
				case ADDR_MODE_NONE:
				{
					ESP_LOGI(TAG, "Originating from the PAN coordinator");
					break;
				}
				case ADDR_MODE_SHORT:
				{
					short_src_addr = *((uint16_t*) &packet[position]);
					position += sizeof(uint16_t);
					ieee802154_packet->short_src_addr = short_src_addr;
					ESP_LOGI(TAG, "Originating from short address 0x%04x", short_src_addr);
					break;
				}
				case ADDR_MODE_LONG:
				{
					for (uint8_t idx = 0; idx < sizeof(src_addr); idx++) {
						src_addr[idx] = packet[position + sizeof(src_addr) - 1 - idx];
					}
					position += sizeof(src_addr);
					memcpy(ieee802154_packet->long_src_addr, src_addr, sizeof(src_addr));
					ESP_LOGI(TAG, "Originating from long address %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", src_addr[0], src_addr[1],
							 src_addr[2], src_addr[3], src_addr[4], src_addr[5], src_addr[6], src_addr[7]);
					break;
				}
				default:
				{
					ESP_LOGE(TAG, "With reserved source address type, ignoring packet");
					return;
				}
			}

			uint8_t* header		   = &packet[0];
			uint8_t  header_length = position;
			uint8_t* data		   = &packet[position];
			uint8_t  data_length   = packet_length - position - sizeof(uint16_t);
			position += data_length;
			ieee802154_packet->data_length = data_length;
			memcpy(ieee802154_packet->data, data, data_length);
			ESP_LOG_BUFFER_HEXDUMP(TAG, header, header_length, ESP_LOG_DEBUG);
			ESP_LOG_BUFFER_HEXDUMP(TAG, data, data_length, ESP_LOG_DEBUG);

			ESP_LOGI(TAG, "Data length: %u", data_length);
			for (int i=0;i<data_length;i++) {
				ESP_LOGD(TAG, "Data[%d]=0x%x", i, data[i]);
			}

			uint16_t checksum = *((uint16_t*) &packet[position]);
			ESP_LOGI(TAG, "checksum: 0x%04x", checksum);

			ESP_LOGI(TAG, "PAN %04x S %04x %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X to %04x %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X %s", pan_id,
					 short_src_addr, src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5], src_addr[6], src_addr[7],
					 short_dst_addr, dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4], dst_addr[5], dst_addr[6], dst_addr[7],
					 broadcast ? "BROADCAST" : "");


#if 0
			if (broadcast)
				for (uint8_t idx = 0; idx < 8; idx++) dst_addr[idx] = 0xFF;
#endif

			break;
		}
		case FRAME_TYPE_ACK:
		{
			uint8_t sequence_number = packet[position++];
			ESP_LOGI(TAG, "Ack (%u)", sequence_number);
			break;
		}
		default:
		{
			ESP_LOGE(TAG, "Packet ignored because of frame type (%u)", fcs->frameType);
			break;
		}
	}
	ESP_LOGI(TAG, "-----------------------");
}


uint8_t ieee802154_header(const uint16_t *pan_id, ieee802154_address_t *src, ieee802154_address_t *dst, uint8_t *header, size_t header_length) {
	static uint8_t sequence_number = 0x00;
	mac_fcs_t frame_header = {
			.frameType = FRAME_TYPE_DATA,
			.secure = false,
			.framePending = false,
			.ackReqd = false,
			.panIdCompressed = true,
			.rfu1 = false,
			.sequenceNumberSuppression = false,
			.informationElementsPresent = false,
			.destAddrType = dst->mode,
			.frameVer = FRAME_VERSION_STD_2003,
			.srcAddrType = src->mode
	};
	uint8_t position = 0;
	memcpy(&header[position], &frame_header, sizeof frame_header);
	position = 2;

	memcpy(&header[position], &sequence_number, sizeof sequence_number);
	position += 1;
	sequence_number++;

	memcpy(&header[position], pan_id, sizeof(uint16_t));
	position += 2;


	if (frame_header.destAddrType == ADDR_MODE_SHORT) {
		memcpy(&header[position], &dst->short_address, sizeof dst->short_address);
		position += 2;
	} else if (frame_header.destAddrType == ADDR_MODE_LONG) {
		reverse_memcpy(&header[position], (uint8_t *)&dst->long_address, sizeof dst->long_address);
		position += 8;
	}

	if (frame_header.srcAddrType == ADDR_MODE_SHORT) {
		memcpy(&header[position], &src->short_address, sizeof src->short_address);
		position += 2;
	} else if (frame_header.srcAddrType == ADDR_MODE_LONG) {
		reverse_memcpy(&header[position], (uint8_t *)&src->long_address, sizeof src->long_address);
		position += 8;
	}

	return position;
}

static void reverse_memcpy(uint8_t *restrict dst, const uint8_t *restrict src, size_t n)
{
	size_t i;

	for (i=0; i < n; ++i) {
		dst[n - 1 - i] = src[i];
	}
}

void ieee802154_send_long(uint8_t dst_long[8], uint8_t *payload, size_t payload_length) {
	uint16_t pan_id = esp_ieee802154_get_panid();
    ESP_LOGI(TAG, "Send message to %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x on pan 0x%04x",
             dst_long[0], dst_long[1], dst_long[2], dst_long[3], dst_long[4], dst_long[5], dst_long[6], dst_long[7], pan_id);

    esp_ieee802154_set_panid(pan_id);

    uint8_t ext_addr[8];
    esp_ieee802154_get_extended_address(ext_addr);

    ieee802154_address_t src = {
            .mode = ADDR_MODE_LONG,
            .long_address = { ext_addr[0], ext_addr[1], ext_addr[2], ext_addr[3], ext_addr[4], ext_addr[5], ext_addr[6], ext_addr[7]}
    };

    ieee802154_address_t dst = {
            .mode = ADDR_MODE_LONG,
            .long_address = { dst_long[0], dst_long[1], dst_long[2], dst_long[3], dst_long[4], dst_long[5], dst_long[6], dst_long[7]}
    };

    uint8_t buffer[256];
    uint8_t header_length = ieee802154_header(&pan_id, &src, &dst, &buffer[1], sizeof(buffer) - 1);
    ESP_LOG_BUFFER_HEXDUMP(TAG, &buffer[1], header_length, ESP_LOG_DEBUG);

    // Add the local eui64 as payload
    memcpy(&buffer[1 + header_length], payload, payload_length);

    // packet length
    buffer[0] = header_length + payload_length;

    // calculate checksum
    uint16_t checksum = ieee802154_checksum(buffer, buffer[0]);
    ESP_LOGD(TAG, "checksum=0x%x", checksum);

    memcpy(&buffer[1 + header_length + payload_length], &checksum, 2);
    buffer[0] = buffer[0] + 2;
    ESP_LOG_BUFFER_HEXDUMP(TAG, &buffer[1], buffer[0], ESP_LOG_DEBUG);

    esp_ieee802154_transmit(buffer, false);
}

void ieee802154_send_short(uint16_t dst_short, uint8_t *payload, size_t payload_length) {
	uint16_t pan_id = esp_ieee802154_get_panid();
    ESP_LOGI(TAG, "Send message to 0x%04x on pan 0x%04x", dst_short, pan_id);

    //uint8_t ext_addr[8];
    //esp_ieee802154_get_extended_address(ext_addr);
	uint16_t short_addr;
	short_addr = esp_ieee802154_get_short_address();
    ESP_LOGD(TAG, "short_addr=0x%x", short_addr);

    ieee802154_address_t src = {
        .mode = ADDR_MODE_SHORT,
        .short_address = short_addr
    };

    ieee802154_address_t dst = {
        .mode = ADDR_MODE_SHORT,
        .short_address = dst_short
    };

    uint8_t buffer[256];
    uint8_t header_length = ieee802154_header(&pan_id, &src, &dst, &buffer[1], sizeof(buffer) - 1);
    ESP_LOG_BUFFER_HEXDUMP(TAG, &buffer[1], header_length, ESP_LOG_DEBUG);

    // Add the local eui64 as payload
    memcpy(&buffer[1 + header_length], payload, payload_length);

    // packet length
    buffer[0] = header_length + payload_length;

    // calculate checksum
    uint16_t checksum = ieee802154_checksum(buffer, buffer[0]);
    ESP_LOGI(TAG, "checksum=0x%x", checksum);

    memcpy(&buffer[1 + header_length + payload_length], &checksum, 2);
    buffer[0] = buffer[0] + 2;
    ESP_LOG_BUFFER_HEXDUMP(TAG, &buffer[1], buffer[0], ESP_LOG_INFO);
    esp_ieee802154_transmit(buffer, false);
}

#define polynomial 0x1021 // CRC-16-CCITT X^16+X^12+X^5+X^0 (1 0001 0000 0010 0001)

static uint16_t crc_1byte(unsigned char in, uint16_t crc)
{
	uint16_t a;

	a = ((in << 8) ^ crc) & 0xFF00;
	for (int bit = 8; bit != 0; bit--)
	{
		if (0 == (a & 0x8000)) a = (a << 1);
		else a = polynomial ^ (a << 1);
	}
	crc = a ^ (crc << 8);
	return crc;
}

uint16_t ieee802154_checksum(uint8_t * packet, size_t packet_length) {
	uint16_t crc = 0x0;
	for (int i=0;i<packet_length;i++) {
		crc = crc_1byte(packet[i], crc);
	}
	return crc;
}
