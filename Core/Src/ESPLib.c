/*
 * ESPLib.c
 *
 *  Created on: Feb 14, 2022
 *      Author: Ümit Can Güveren
 */


/*EKLENECEKLER*/
// mqtt connect yaparken password ve id ekleme, flag kontrolleri ve flag'e göre veri paketi oluşturma
// do-while döngülerinin içinde belli bir süre sonra işlemciye reset atma


#include <ESPLib.h>
#include "main.h"

extern UART_HandleTypeDef huart2;

char Buffer[BUFFERSIZE];
char rxBuffer[BUFFERSIZE];
char mqttBuffer[300];
char mqttPacket[BUFFERSIZE];
char temp_mqttBuffer[100];

uint8_t temp_mqttMsgLen, temp_mqttTopicLen;

/*Responses and Commands*/
char *OK = "OK\r\n";
char *WIFIDISCONNECT = "OK\r\nWIFI DISCONNECT\r\n"; //AT+CWQAP komutu karşılığı cihaz bir ağa bağlı ise verdiği cevap.
char *WIFICONNECT = "WIFI CONNECTED\r\nWIFI GOT IP\r\n";
char *SEND_OK = "SEND OK\r\n";
char *ERROR_ = "ERROR\r\n";
char *CIPSEND_RESPONSE = "OK\r\n> ";
char *PORTCONNECT = "CONNECT\r\n";
char *WIFICONNECTED = "WIFI CONNECTED\r\n";


void sendData(char *cmd, uint8_t cmdSize, uint8_t responseSize, uint32_t timeout) //size'ı fonksiyonda al.
{
	HAL_StatusTypeDef uartCheck;

	//HAL_UART_Receive(&huart2, (uint8_t *)Buffer, 1, 10); //Receive bufferı temizlemek için.

	memset(Buffer, 0, BUFFERSIZE);

	uartCheck = HAL_UART_Transmit(&huart2, (uint8_t *)cmd, cmdSize, 100);

	if(uartCheck == HAL_OK)
	{
		HAL_UART_Receive(&huart2, (uint8_t *)Buffer, cmdSize + responseSize + 2, timeout); //Timeout fonksiyona gelecek. //responseSize eklenecek // +2 = \r\n
	}

}

funcState_t ESP8266_Init(espMode_t Mode)
{
	funcState_t checkFunc;
	char cmd[20];
	uint8_t cmdSize;

	do{
	sendData("AT\r\n", strlen("AT\r\n"), strlen(OK), 100);
	checkFunc = checkResponse(OK); //OK cevabı alınıyorsa ESP doğru şekilde çalışıyor.
	}while(checkFunc != funcOk);

	do{
	memset(cmd, 0, sizeof(cmd));
	cmdSize = sprintf(cmd, "AT+CWMODE=%d\r\n", Mode); //Seçilen mode göre gönderilecek komut ayarlanır.
	sendData(cmd, cmdSize, strlen(OK), 100);					//Komut gönderilir.
	checkFunc = checkResponse(OK); //OK cevabı alındıysa istenilen mode ayarlanmıştır.
	}while(checkFunc != funcOk);

	return funcOk;
}


funcState_t ESP8266_Reset(void)
{
	funcState_t checkFunc;
	char cmd[10];

	do{
    memset(cmd, 0, sizeof(cmd));
	sendData("AT+RST\r\n", strlen("AT+RST\r\n"), strlen(OK), 100);
	checkFunc = checkResponse(OK); //OK cevabı alınıyorsa ESP8266 resetlendi.
	}while(checkFunc != funcOk);

	return funcOk;
}



/*
funcState_t ESP8266_checkVersion()
{
	funcState_t checkFunc;
	char cmd[10];

	do{
    memset(cmd, 0, 20);
	sendData("AT+GMR\r\n", strlen("AT+GMR\r\n"), strlen(OK) + 100, 100);
	checkFunc = checkResponse(OK);
	}while(checkFunc != funcOk);

	return funcOk;
}
*/

funcState_t ESP8266_wifiConnect(char *SSID, char *Password)
{
	funcState_t checkFunc;
	char cmd[100];
	uint8_t cmdSize;

	do{
	sendData("AT+CWQAP\r\n", strlen("AT+CWQAP\r\n"), strlen(WIFIDISCONNECT), 100);				//Cihaz herhangi bir ağa bağlı ise bağlantı kesilir.
	checkFunc = (checkResponse(OK) | checkResponse(WIFIDISCONNECT));	//Bağlantının kesildiği kontrol edilir  ve doğruysa döngüden çıkılır.
	}while(checkFunc != funcOk);

	do{
	memset(cmd, 0, sizeof(cmd));
	cmdSize = sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, Password);	//Ağ id ve şifresine göre gönderilecek komut ayarlanır.
	sendData(cmd, cmdSize, strlen(WIFICONNECT) + strlen(OK), 20000);		   								//Gelen tüm metinleri almak için timeout süresi uzun tutulmuştur.
	checkFunc = (checkResponse(OK) | checkResponse(WIFICONNECTED)); 										//OK cevabı alınıyorsa ağa bağlanılmıştır.
	}while(checkFunc != funcOk);

	return funcOk;
}

funcState_t ESP8266_portConnect(char *type, char *remoteIP, char *remotePort)
{
	funcState_t checkFunc;
	char cmd[100];
	uint8_t cmdSize;

	do{
	sendData("AT+CIPCLOSE\r\n", strlen("AT+CIPCLOSE\r\n"), strlen(ERROR_) ,100);	//Cihaz herhangi bir ağa bağlı ise bağlantı kesilir.
	checkFunc = checkResponse(OK) | checkResponse(ERROR_);							//Bağlantının kesildiği kontrol edilir  ve doğruysa döngüden çıkılır.
	}while(checkFunc != funcOk);

	do{
	memset(cmd, 0, sizeof(cmd));
	cmdSize = sprintf(cmd, "AT+CIPSTART=\"%s\",\"%s\",%s\r\n", type, remoteIP, remotePort);		//Ağ id ve şifresine göre gönderilecek komut ayarlanır.
	sendData(cmd, cmdSize, strlen(PORTCONNECT) + strlen(OK), 50000);		   										//Gelen tüm metinleri almak için timeout süresi uzun tutulmuştur.
	checkFunc = checkResponse(OK); 																//OK cevabı alınıyorsa ağa bağlanılmıştır.
	}while(checkFunc != funcOk);

	return funcOk;
}

funcState_t ESP8266_sendMessage(char *msg, uint8_t msgSize)
{
	funcState_t checkFunc;
	char cmd[100];
	uint8_t cmdSize;

	HAL_UART_AbortReceive_IT(&huart2);

	do{
	memset(cmd, 0, sizeof(cmd));
    cmdSize = sprintf(cmd, "AT+CIPSEND=%d\r\n", msgSize);		//Ağ id ve şifresine göre gönderilecek komut ayarlanır.
    sendData(cmd, cmdSize, 15, 100);
	checkFunc = checkResponse(CIPSEND_RESPONSE) | checkResponse(SEND_OK);
	}while(checkFunc != funcOk);

	if(checkFunc == funcOk)
	{
		sendData(msg, msgSize, 70, 1000);
	}

	HAL_UART_Receive_IT(&huart2, &uartRxData, 1);

	return funcOk;
}

funcState_t checkResponse(char *response)
{
	uint8_t i;
	funcState_t checkState;
	uint8_t responseSize;
	uint8_t sizeBuffer;

	responseSize = strlen(response);

	sizeBuffer = strlen(Buffer);
	memcpy(rxBuffer, &Buffer[sizeBuffer - responseSize], responseSize);

	for(i = 0; i < responseSize; i++)
	{
		if(rxBuffer[i] != response[i])
		{
			checkState = funcErr;
			break;
		}
		else
		{
			checkState = funcOk;
		}
	}

	memset(rxBuffer, 0, BUFFERSIZE);
	return checkState;
}


/******************************************************************************************************************************/
/********************************************************MQTT Functions********************************************************/
/******************************************************************************************************************************/

void MQTT_connectBroker(MQTT_InitTypeDef_t *MQTTConnect)
{
	char *protocolName = "MQTT";

	uint16_t protocolLenght  = strlen(protocolName); //protocolLenght 2 byte. "MQTT" 4 byte.
	uint16_t clientIDLenght = strlen(MQTTConnect->clientID);

	uint8_t mqttLenght;

	memset(mqttPacket, 0, sizeof(mqttPacket));

	if(MQTTConnect->connectFlag == (PasswordFlag | UserNameFlag))
	{
		uint16_t usernameLenght = strlen(MQTTConnect->username);
		uint16_t passwordLenght = strlen(MQTTConnect->password);
		uint8_t remainingLenght = 2 + protocolLenght + 4 + 2 + clientIDLenght + 2 + usernameLenght + 2 + passwordLenght;

		mqttLenght = sprintf(mqttPacket, "%c%c%c%c%s%c%c%c%c%c%c%s%c%c%s%c%c%s", (char)MQTTconnect, (char)remainingLenght, (char)(protocolLenght >> 8), (char)(protocolLenght & 0x00FF),
							 protocolName,(char)MQTTlevel,(char)MQTTConnect->connectFlag, (char)(MQTTConnect->keepAlive >> 8), (char)MQTTConnect->keepAlive, (char)(clientIDLenght >> 8), (char)(clientIDLenght & 0x00FF),
							 MQTTConnect->clientID, (char)(usernameLenght >> 8), (char)usernameLenght, MQTTConnect->username, (char)(passwordLenght >> 8), (char)passwordLenght, MQTTConnect->password);
	}
	else if(MQTTConnect->connectFlag == PasswordFlag)
	{
		uint16_t passwordLenght = strlen(MQTTConnect->password);
		uint8_t remainingLenght = 2 + protocolLenght + 4 + 2 + clientIDLenght + 2 + passwordLenght;

		mqttLenght = sprintf(mqttPacket, "%c%c%c%c%s%c%c%c%c%c%c%s%c%c%s", (char)MQTTconnect, (char)remainingLenght, (char)(protocolLenght >> 8), (char)(protocolLenght & 0x00FF),
							 protocolName,(char)MQTTlevel,(char)MQTTConnect->connectFlag, (char)(MQTTConnect->keepAlive >> 8), (char)MQTTConnect->keepAlive, (char)(clientIDLenght >> 8), (char)(clientIDLenght & 0x00FF),
							 MQTTConnect->clientID, (char)(passwordLenght >> 8), (char)passwordLenght, MQTTConnect->password);
	}
	else if(MQTTConnect->connectFlag == UserNameFlag)
	{
		uint16_t usernameLenght = strlen(MQTTConnect->username);
		uint8_t remainingLenght = 2 + protocolLenght + 4 + 2 + clientIDLenght + 2 + usernameLenght;

		mqttLenght = sprintf(mqttPacket, "%c%c%c%c%s%c%c%c%c%c%c%s%c%c%s", (char)MQTTconnect, (char)remainingLenght, (char)(protocolLenght >> 8), (char)(protocolLenght & 0x00FF),
							 protocolName,(char)MQTTlevel,(char)MQTTConnect->connectFlag, (char)(MQTTConnect->keepAlive >> 8), (char)MQTTConnect->keepAlive, (char)(clientIDLenght >> 8), (char)(clientIDLenght & 0x00FF),
							 MQTTConnect->clientID, (char)(usernameLenght >> 8), (char)usernameLenght, MQTTConnect->username);
	}
	else
	{
		uint8_t remainingLenght = 2 + protocolLenght + 4 + 2 + clientIDLenght;

		mqttLenght = sprintf(mqttPacket, "%c%c%c%c%s%c%c%c%c%c%c%s", (char)MQTTconnect, (char)remainingLenght, (char)(protocolLenght >> 8), (char)(protocolLenght & 0x00FF),
							 protocolName,(char)MQTTlevel,(char)MQTTConnect->connectFlag, (char)(MQTTConnect->keepAlive >> 8), (char)MQTTConnect->keepAlive, (char)(clientIDLenght >> 8), (char)(clientIDLenght & 0x00FF),
							 MQTTConnect->clientID);
	}

	ESP8266_sendMessage(mqttPacket, mqttLenght);

}

void MQTT_publishTopic(char *topic, char *msg, uint8_t QoS)
{
	uint8_t mqttLenght;

	uint16_t topicLenght = strlen(topic);
	uint16_t msgLenght = strlen(msg);

	uint8_t remainingLenght = 2 + topicLenght + msgLenght;

	memset(mqttPacket, 0, sizeof(mqttPacket));

	mqttLenght = sprintf(mqttPacket, "%c%c%c%c%s%s", (char)(MQTTpublish | QoS), (char)remainingLenght, (char)topicLenght >> 8, (char)topicLenght, topic, msg);

	ESP8266_sendMessage(mqttPacket, mqttLenght);
}

void MQTT_subscribeTopic(char *topic, uint8_t QoS)
{
	uint8_t mqttLenght;

	uint16_t topicLenght = strlen(topic);

	uint8_t remainingLenght = 5 + topicLenght;

	uint16_t packetID = 0x01;
	memset(mqttPacket, 0, sizeof(mqttPacket));

	mqttLenght = sprintf(mqttPacket, "%c%c%c%c%c%c%s%c", (char)MQTTsubscribe, (char)remainingLenght, (char)packetID >> 8, (char)packetID, (char)topicLenght >> 8, (char)topicLenght, topic, QoS);

	ESP8266_sendMessage(mqttPacket, mqttLenght);
}


void MQTT_pingReq()
{
	uint8_t mqttLenght;
	uint8_t remainingLenght = 0;

	memset(mqttPacket, 0, 100);

	mqttLenght = sprintf(mqttPacket, "%c%c", (char)MQTTpingreq, (char)remainingLenght);

	ESP8266_sendMessage(mqttPacket, mqttLenght);
}


void MQTT_unsubsribeTopic(char *topic)
{
	uint8_t mqttLenght;

	uint16_t topicLenght = strlen(topic);

	uint8_t remainingLenght = 4 + topicLenght;

	uint16_t packetID = 0x01;

	memset(mqttPacket, 0, 100);

	mqttLenght = sprintf(mqttPacket, "%c%c%c%c%c%c%s", (char)MQTTunsub, (char)remainingLenght, (char)packetID >> 8, (char)packetID, (char)topicLenght >> 8, (char)topicLenght, topic);

	ESP8266_sendMessage(mqttPacket, mqttLenght);
}

void MQTT_disconnectBroker(void)
{
	uint8_t mqttLenght;
	uint8_t remainingLenght = 0;

	memset(mqttPacket, 0, 100);

	mqttLenght = sprintf(mqttPacket, "%c%c", (char)MQTTdisconnect, (char)remainingLenght);

	ESP8266_sendMessage(mqttPacket, mqttLenght);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	RxInterruptFlag = SET;
	mqttBuffer[DataCounter++] = uartRxData;
	if(DataCounter >= 300)
	{
		DataCounter  = 0;
	}

	HAL_UART_Receive_IT(&huart2 , &uartRxData , 1);
	uartTimeCounter = 0;
}


void MQTTDataHandler(char *mqttTopic, char *mqttMsg)
{
	uint8_t mqttMsgLen, digit_mqttMsgLen;
	uint16_t mqttTopicLen;
	uint8_t colonIndex;
	uint8_t i;
	uint8_t digitNumber = 0;

	if(uartPacketComplatedFlag == SET)     //Data receiving is finished
	{
		uartPacketComplatedFlag = RESET;

		for(i = 0; i < DataCounter; i++)
		{
			if(mqttBuffer[i] == ':')
			{
				colonIndex = i;
				break;
			}
		}

		memset(mqttTopic, 0, temp_mqttTopicLen);
		memset(mqttMsg, 0, temp_mqttMsgLen);

		mqttMsgLen = mqttBuffer[colonIndex + 2];
		digit_mqttMsgLen = mqttMsgLen + 2;

	    while(digit_mqttMsgLen > 0){
	    	digit_mqttMsgLen = digit_mqttMsgLen / 10;
	        digitNumber++;
	    }

		mqttTopicLen = (mqttBuffer[10 + digitNumber] << 8 ) | (mqttBuffer[11 + digitNumber]);
		memcpy(mqttMsg, &mqttBuffer[15 + digitNumber], mqttMsgLen - mqttTopicLen - 2);
		memcpy(mqttTopic, &mqttBuffer[12 + digitNumber], mqttTopicLen);
		memset(mqttBuffer, 0, DataCounter);

		temp_mqttMsgLen = mqttMsgLen;
		temp_mqttTopicLen = mqttTopicLen;
		DataCounter = 0;

	}
}

/* This function should be called in systick timer */
void MQTTTimer(void)
{
	if(RxInterruptFlag == SET)
	{
		if(uartTimeCounter++ > 100)
		{

			RxInterruptFlag = RESET;
			uartTimeCounter = 0;
			uartPacketComplatedFlag = SET;
		}
	}
}

