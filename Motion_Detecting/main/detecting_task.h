

#define PIR_SENSOR_PIN GPIO_NUM_27
#define SERVER_URL "http://192.168.4.1/detected"  // 서버의 IP 주소와 경로를 설정하세요.


void Sensor_Init(void);

void Sensor_Task(void* param);

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);