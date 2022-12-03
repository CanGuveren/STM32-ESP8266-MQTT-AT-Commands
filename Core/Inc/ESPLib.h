/*
 * ESPLib.h
 *
 *  Created on: Feb 14, 2022
 *      Author: Can Guveren
 */

#ifndef INC_ESPLIB_H_
#define INC_ESPLIB_H_

#include "stdbool.h"
#include <stdio.h>
#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"


#define BUFFERSIZE 100

uint8_t uartRxData;
uint8_t DataCounter;
uint8_t RxInterruptFlag;
uint8_t uartTimeCounter;
uint8_t uartPacketComplatedFlag;

uint8_t temp_mqttMsgLen, temp_mqttTopicLen;

char Buffer[BUFFERSIZE];
char rxBuffer[BUFFERSIZE];
char mqttBuffer[BUFFERSIZE];
char mqttPacket[BUFFERSIZE];


typedef enum
{
	funcErr,
	funcOk,
}funcState_t;


typedef enum
{
	StationMode = 1,
	AccessPointMode,
	StationAP,
}espMode_t;

typedef struct
{
	char *clientID;
	char *username;
	char *password;
	uint16_t keepAlive;
	uint8_t connectFlag;

} MQTT_InitTypeDef_t;

/* MQTT Commands defines */
#define MQTTlevel		0x04
#define MQTTconnect 	0x10
#define MQTTlength	 	0x04
#define MQTTsubscribe 	0x82
#define MQTTpublish		0x30
#define MQTTpingreq		0xC0
#define MQTTunsub		0xA2
#define MQTTdisconnect	0xE0

/* MQTT Connects Flag defines */
#define CleanSession 	0x02
#define PasswordFlag 	0x40
#define UserNameFlag 	0x80

/* QoS Level defines */
#define QoS0			0x00
#define QoS1			0x02
#define QoS2			0x04



/* ESP8266 Function */
funcState_t ESP8266_Init(espMode_t mode, UART_HandleTypeDef UARTHandle);
funcState_t ESP8266_Reset();
funcState_t ESP8266_wifiConnect(char *SSID, char *Password);
funcState_t ESP8266_portConnect(char *type, char *remoteIP, char *remotePort);
funcState_t ESP8266_sendMessage(char *msg, uint8_t msgSize);
funcState_t checkResponse(char * response);


/* MQTT Function */
void MQTT_connectBroker(MQTT_InitTypeDef_t *MQTTConnect);
void MQTT_publishTopic(char *topic, char *msg, uint8_t QoS);
void MQTT_subscribeTopic(char *topic, uint8_t QoS);
void MQTT_unsubsribeTopic(char *topic);
void MQTT_disconnectBroker(void);
void MQTT_pingReq();


/* Receiver functions */
void MQTTDataHandler(char *mqttTopic, char *mqttMsg);
void MQTTTimer(void);


#endif /* INC_ESPLIB_H_ */
