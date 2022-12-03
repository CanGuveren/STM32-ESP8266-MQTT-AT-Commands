# STM32-ESP8266-MQTT-AT-Commands

## Publish

```C
int main(void)
{
  ESP8266_Init(StationMode, huart2);
  ESP8266_wifiConnect("******","******");
  ESP8266_portConnect("TCP", "***.***.*.***", "1883");

  MQTT_InitTypeDef_t mqttSetting;
  mqttSetting.keepAlive = 500;
  mqttSetting.connectFlag = CleanSession;
  mqttSetting.clientID = "MqttID";
  MQTT_connectBroker(&mqttSetting);
  
  while (1)
  {
	  MQTT_publishTopic("test", "hello", QoS0);
	  HAL_Delay(1000);
  }
}
```

## Subscribe
:warning:In this mode, you must write the MQTTTimer() in the systick timer.

```C
char mqttData[50];
char mqttTopic[10];

int main(void)
{
  ESP8266_Init(StationMode, huart2);
  ESP8266_wifiConnect("******","******");
  ESP8266_portConnect("TCP", "***.***.*.***", "1883");

  MQTT_InitTypeDef_t mqttSetting;
  mqttSetting.keepAlive = 500;
  mqttSetting.connectFlag = CleanSession;
  mqttSetting.clientID = "MqttID";
  MQTT_connectBroker(&mqttSetting);
  MQTT_subscribeTopic("IOT", 0);
  HAL_UART_Receive_IT(&huart2 , &uartRxData , 1);

  while (1)
  {
	  MQTTDataHandler(mqttTopic, mqttData);
  }
}
```
```C
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  MQTTTimer();
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}
```
