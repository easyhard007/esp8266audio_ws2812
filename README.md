# esp8266audio_ws2812
Arduino code for nodeMCU (esp8266) to read analog voltage from  3.5mm or 6.35mm audio interface(e.g. output of a guitar pick-up), and transform such signal to light up WS2812b leds. It implements an effect that the leds light up when strumming the guitar strings.

使用NodeMCU的A0针脚读取音频接口的电压（例如吉他拾音器的输出），转换为控制ws2812b灯带的亮度信号。可以实现弹吉他则灯亮一下的效果。


/*Future features*/

The project at present can only flash white lights, but I think this can be improved or extended at these ways:

1. Use a third-party extension (such as the Blinker) which makes the color of the lights can be changed by a cell-phone APP.
2. Add some FFT modules (such as MSGEQ7), to detect the frequencies of the strings, and try to change the color when different notes of chords are played.

/*未来可能添加的特性*/

现在这个项目只能闪白光，但是我觉得可以从以下两个方面拓展它：
1. 使用第三方插件（例如点灯科技Blinker）使得可以用手机APP控制灯光颜色。
2. 增加FFT模块（例如MSGEQ7），来检测当前弹奏的音符的频率或者和弦，从而改变灯光颜色。
