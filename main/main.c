/*
	IEEE802.15.4 communication example.
	This example code is in the Public Domain (or CC0 licensed, at your option.)

	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.

	I used this as a reference.
	https://github.com/spark404/ieee802154-sender
	https://github.com/spark404/ieee802154-receiver

	esp_ieee802154.h is here.
	https://github.com/espressif/esp-idf/blob/master/components/ieee802154/include/esp_ieee802154.h
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/message_buffer.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_ieee802154.h"

#include "ieee802154.h"

#define TAG "main"
#define RADIO_TAG "ieee802154"

// for debug
#define PANID 0x4242
#define CHANNEL 12

StreamBufferHandle_t xMessageBuffer = NULL;

void esp_ieee802154_receive_done(uint8_t* frame, esp_ieee802154_frame_info_t* frame_info) {
	ESP_EARLY_LOGI(RADIO_TAG, "rx OK, received %d bytes", frame[0]);
	xMessageBufferSendFromISR(xMessageBuffer, frame, frame[0], NULL);
}

void esp_ieee802154_receive_failed(uint16_t error) {
	ESP_EARLY_LOGI(RADIO_TAG, "rx failed, error %d", error);
}

void esp_ieee802154_receive_sfd_done(void) {
	ESP_EARLY_LOGI(RADIO_TAG, "rx sfd done, Radio state: %d", esp_ieee802154_get_state());
}
void esp_ieee802154_transmit_done(const uint8_t *frame, const uint8_t *ack, esp_ieee802154_frame_info_t *ack_frame_info) {
	ESP_EARLY_LOGI(RADIO_TAG, "tx OK, sent %d bytes, ack %d", frame[0], ack != NULL);
}

void esp_ieee802154_transmit_failed(const uint8_t *frame, esp_ieee802154_tx_error_t error) {
	ESP_EARLY_LOGI(RADIO_TAG, "tx failed, error %d", error);
}

void esp_ieee802154_transmit_sfd_done(uint8_t *frame) {
	ESP_EARLY_LOGI(RADIO_TAG, "tx sfd done");
}

void dump(uint8_t *dt, int n)
{
	uint16_t clm = 0;
	uint8_t data;
	uint32_t saddr =0;
	uint32_t eaddr =n-1;

	printf("----------------------------------------------------------\n");
	uint32_t addr;
	for (addr = saddr; addr <= eaddr; addr++) {
		data = dt[addr];
		if (clm == 0) {
			printf("%05"PRIx32": ",addr);
		}
		printf("%02x ",data);
		clm++;
		if (clm == 16) {
			printf("\n");
			clm = 0;
		}
	}
	if (clm != 0) printf("\n");
	printf("----------------------------------------------------------\n");
}

static void receive_packet_task(void *pvParameters) {
	uint8_t packet[1024];
	ieee802154_packet_t ieee802154_packet;
	while (1) {
		size_t readBytes = xMessageBufferReceive(xMessageBuffer, packet, sizeof(packet), portMAX_DELAY);
		if (readBytes == 0) break;
		ieee802154_analysis_packet(&packet[1], packet[0], &ieee802154_packet);
		ESP_LOG_BUFFER_HEXDUMP(TAG, ieee802154_packet.fcs, sizeof(uint16_t), ESP_LOG_DEBUG);
		if (ieee802154_packet.fcs->frameType == FRAME_TYPE_DATA) {
			ESP_LOGI(TAG, "ieee802154_packet.data_length=%d", ieee802154_packet.data_length);
			dump(ieee802154_packet.data, ieee802154_packet.data_length);
			ESP_LOGI(TAG, "[%.*s]", ieee802154_packet.data_length, ieee802154_packet.data);
		}
	}

	ESP_LOGE(__FUNCTION__, "Terminated");
	vTaskDelete(NULL);
}

static void send_packet_task(void *pvParameters) {
	uint16_t peer_short = CONFIG_IEEE802514_PEER_ADDR;
	while (true) {
		uint8_t payload[128];
#if CONFIG_IDF_TARGET_ESP32C6
		strcpy((char *)payload, "This is ESP32C6");
		size_t payload_length = strlen((char *)payload);
#endif
#if CONFIG_IDF_TARGET_ESP32H2
		strcpy((char *)payload, "This is ESP32H2");
		size_t payload_length = strlen((char *)payload);
#endif
		//memcpy((char *)payload, "This is ESP32H2", 10);
		//size_t payload_length = sizeof(payload);
		ieee802154_send_short(peer_short, payload, payload_length);
		vTaskDelay(pdMS_TO_TICKS(5000));
	}

	// Never come here.
	ESP_LOGE(__FUNCTION__, "Terminated");
	vTaskDelete(NULL);
}

void app_main() {
	ESP_LOGI(TAG, "Initializing NVS from flash...");
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);

	// Create Message Buffer
	xMessageBuffer = xMessageBufferCreate(1024);
	configASSERT( xMessageBuffer );

	ESP_ERROR_CHECK(esp_ieee802154_enable());
	ESP_ERROR_CHECK(esp_ieee802154_set_promiscuous(true));
	ESP_ERROR_CHECK(esp_ieee802154_set_rx_when_idle(true));

	//ESP_ERROR_CHECK(esp_ieee802154_set_panid(PANID));
	ESP_ERROR_CHECK(esp_ieee802154_set_panid(CONFIG_IEEE802514_PANID));
	ESP_ERROR_CHECK(esp_ieee802154_set_coordinator(false));

	//ESP_ERROR_CHECK(esp_ieee802154_set_channel(CHANNEL));
	ESP_ERROR_CHECK(esp_ieee802154_set_channel(CONFIG_IEEE802514_CHANNEL));

	// Set long address to the mac address
	uint8_t mac_addr[8] = {0};
	esp_read_mac(mac_addr, ESP_MAC_IEEE802154);
	uint8_t eui64_rev[8] = {0};
	for (int i=0; i<8; i++) {
		eui64_rev[7-i] = mac_addr[i];
		ESP_LOGD(TAG, "mac_addr[%d]=0x%x", i, mac_addr[i]);
	}
	esp_ieee802154_set_extended_address(eui64_rev);

	// Set short address
	esp_ieee802154_set_short_address(CONFIG_IEEE802514_MY_ADDR);

	// Start receiver
	ESP_ERROR_CHECK(esp_ieee802154_receive());

#if 0
	uint8_t extended_address[8];
	esp_ieee802154_get_extended_address(extended_address);
	ESP_LOGI(TAG, "Ready, panId=0x%04x, channel=%d, long=%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, short=0x%04x",
		 esp_ieee802154_get_panid(), esp_ieee802154_get_channel(),
		 extended_address[0], extended_address[1], extended_address[2], extended_address[3],
		 extended_address[4], extended_address[5], extended_address[6], extended_address[7],
		 esp_ieee802154_get_short_address());
#endif
	ESP_LOGI(TAG, "Ready, panId=0x%04x, channel=%d, myAddr=0x%04x",
		 esp_ieee802154_get_panid(), esp_ieee802154_get_channel(), esp_ieee802154_get_short_address());

	xTaskCreate(&send_packet_task, "TX", 1024*3, NULL, 5, NULL);
	xTaskCreate(&receive_packet_task, "RX", 1024*5, NULL, 5, NULL);
}
