#include "FastLED.h"
#define NUM_LEDS 8
#define BLINKER_PRINT Serial
#define BLINKER_WIFI 
#include <Blinker.h>

#define LED_PIN 15 //15号引脚，其实是D8这个引脚

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define SAMPLE_SIZE 200


CRGB leds[NUM_LEDS];
const int analogInPin = A0;  //ESP8266模拟引脚ADC0,也就是A0
int count = 0; //计数器，从0数到SAMPLE_SIZE-1然后归零
int sensorValue = 0;  //A0获得的电压，值为0-1023. 但是在小马丁直插上最大也就是40左右。
int sensorBias = 8; //小马丁没开声音时候的默认输入电压偏置
int sensorMax = 60; //最大电压，这个电压将作为输出灯的最大亮度的参考依据
int sensorAvg = 8; //平均电压，采样200次信号并做平均，由于电压不能小于0，即使电压偏置是8，假设电压在0-40之间反复震荡，最终的均值可能也在16左右。
int last_sensor_values[SAMPLE_SIZE]; //存储近200次采样的电压，方便做均值、最大值等
int envl_window = 30;//计算音量包络时用的最大值窗口
int light = 255;//灯光亮度
int last_light_values[SAMPLE_SIZE]; //存储近200次采样的亮度
int hue = 0; //灯的颜色(HSV)
int light_lock = 0; //防闪烁锁，当灯的亮度达到0以后，会被锁住，直到给定的输入亮度大于light_lock_threshold才会解锁
int light_lock_threshold = 5;
float lightScale = 2.5; //亮度和音量的放大倍数

char auth[] = "0421ad55ab63";
char ssid[] = "CMCC-58V2E";
char pswd[] = "15889933840";

// 新建组件对象
BlinkerButton Button1("btn-abc");
BlinkerSlider Slider1("slider_light_scale");
BlinkerNumber Number1("num-abc");

int counter = 0;

// 按下按键即会执行该函数
void button1_callback(const String & state) {
    BLINKER_LOG("get button state: ", state);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//调整亮度和音量的放大倍数
void slider_scale_callback(int32_t value)
{
    BLINKER_LOG("get slider value: ", value);
    lightScale = value/10.0;
}


// 如果未绑定的组件被触发，则会执行其中内容
void dataRead(const String & data)
{
    BLINKER_LOG("Blinker readString: ", data);
    counter++;
    Number1.print(counter);
}


//根据音量包络计算当前音量。音量包络：把震荡的音频波形转化为音量曲线，目前用的方法是找极大值点
int calcVolume(){
  int peakValue = sensorValue;
  int findPeak = 0;

  int volume = max((peakValue-sensorBias),0) * (SAMPLE_SIZE*1.0/sensorMax);
  return volume;
}

//根据音量和放大倍数确定亮度，在0-255之间
int calcLight(int volume){
  return min(int(volume*lightScale),255);
}

//读取A0的电压记录到sensorValue，last_sensor_values, 更新平均电压，返回计算的volume值
void getSensor(){
  sensorValue = analogRead(analogInPin);
  last_sensor_values[count] = sensorValue;
  
}

void setup() {
  // 初始化串口的波特率为115200
  Serial.begin(115200);
  count = 0;
  for(int i=0;i<SAMPLE_SIZE;i++)
  {
    last_sensor_values[i] = 0;
    last_light_values[i] = 0;
  }
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255);  //可以设置全局亮度
  
  #if defined(BLINKER_PRINT)
      BLINKER_DEBUG.stream(BLINKER_PRINT);
  #endif
  
  // 初始化blinker
  Blinker.begin(auth, ssid, pswd);
  Blinker.attachData(dataRead);
  Button1.attach(button1_callback);
  Slider1.attach(slider_scale_callback);
  Slider1.print(int(lightScale*10));
  Blinker.run();
}


void loop() {

  //读取模拟数值
  getSensor();

  int volume = calcVolume(); //sensorValue标准化和包络后计算出的音量大小
  
  //灯光获取和灯光渐弱
  int targetLight = calcLight(volume);

  light -= 1; //如果音量没有超过当前亮度，那么亮度会缓缓下降。
  if(targetLight>light){
    light = targetLight; //如果音量很大，那么亮度会瞬间点亮到目标值
  }
  
  if(light<0){
    light=0;
  }

  last_light_values[count] = light;

  
  double light_avg_short = 0;
  for(int i=0;i<20;i++){
    int current = (count+SAMPLE_SIZE-i) % SAMPLE_SIZE ;
    light_avg_short+=last_light_values[current];
  }
  
  light_avg_short = light_avg_short/40;
  
  double light_avg_long = 0;
  for(int i=0;i<50;i++){
    int current = (count+SAMPLE_SIZE-i) % SAMPLE_SIZE ;
    light_avg_long+=last_light_values[current];
  }
  light_avg_long = light_avg_long/60;

  double light_avg_final = max(light_avg_long,light_avg_short);
  
  if(int(light_avg_final)==0){
    light_lock =1;
  }

  if(light_avg_final>light_lock_threshold){
    light_lock =0;
  }

  
    for(int i=0;i<NUM_LEDS;i++){
  //    leds[i] = CHSV( hue, 128, light); //用HSV色彩空间，不断改变H即可
        if(light_lock==0){
          leds[i] = CRGB( light_avg_final, light_avg_final, light_avg_final); //用HSV色彩空间，不断改变H即可
        } else {
          leds[i] = CRGB(0, 0, 0); //用HSV色彩空间，不断改变H即可
        }
    }
  
  
  Serial.print(light_avg_final);
  Serial.print("\n");
  
  FastLED.show();

  hue = (hue + 1) % 255;

  count = (count + 1) % SAMPLE_SIZE;

}
