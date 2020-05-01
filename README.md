# [CC2541](https://github.com/sochub/CC2541) 

[![sites](http://182.61.61.133/link/resources/SoC.png)](http://SoC.Xin) 

#### [Vendor](https://github.com/SoCXin/Vendor) ：[TI](https://github.com/SoCXin/TI) 
#### [Core](https://github.com/SoCXin/8051) ：[E8051](https://github.com/SoCXin/8051) 
#### [Level](https://github.com/SoCXin/Level) ：16MHz

## [CC2541简介](https://github.com/SoCXin/CC2541/wiki)

[CC2541](http://www.ti.com.cn/product/cn/CC2541) 是TI低成本8051 BLE 4.0 SoC，集成2.4GHz 符合低能耗规范和私有的 RF 片载系统，最高0dBm的可编程输出功率，出色的接收器灵敏度（1 Mbps 时为 –94 dBm）。

与 CC2540 相比，CC2541 提供更低 RF 流耗，没有CC2540所具有的USB接口，并在 TX 模式中提供较低的最大输出功率，CC2541 还增加了1个HW I2C接口。CC2541与CC2540在 6mm x 6mm 方形扁平无引脚 (QFN)40 封装内引脚兼容。 

- 低功率
	* 工作模式 RX 低至： 17.9 mA
	* 工作模式 TX (0 dBm)： 18.2 mA
	* 功率模式 1 （4-µs 唤醒）： 270 µA
	* 功率模式 2 （睡眠定时器打开）： 1 µA
	* 功率模式 3 （外部中断）： 0.5 µA
	* 宽泛的电源电压范围 (2 V–3.6 V)
	* 工作模式下 TPS62730 兼容低功率
	* RX 低至： 14.7 mA （3-V 电源）
	* TX (0 dBm)：14.3 mA（3V 电源）
- 微控制
	* 系统内可编程闪存128 - 256 KB
	* 在所有功率模式下具有保持功能的 8-KB RAM
	* 功能强大的 5 通道直接内存访问 (DMA)
	* 通用定时器 （1 个 16 位，2 个 8 位）
	* 红外 (IR) 生成电路
	* 具有捕捉功能的 32-kHz 睡眠定时器
	* 精确数字接收到的数字信号强度指示器 (RSSI) 支持
	* 电池监视器和温度传感器
	* 含 8 通道和可配置分辨率的 12 位模数转换器 (ADC)
	* 高级加密标准 (AES) 安全协处理器
	* 2 个功能强大的支持几个串行协议的通用异步接收发器 (UART)
	* 23 个通用 I/O 引脚(21 × 4 mA，2 × 20 mA),2 个具有 LED 驱动功能的 I/O 引脚
	* HW I2C 接口
	* 集成的高性能比较器

CC2541具有 128kB 和 256kB 闪存两个版本： CC2541F128/F256


### [资源收录](https://github.com/SoCXin)

* [文档](docs/)
* [资源](src/)

### [关联资源](https://github.com/SoCXin)

* [IAR 8051](https://www.iar.com/iar-embedded-workbench/#!?architecture=8051) 

### [开源项目](https://github.com/SoCXin)



### [选型建议](https://github.com/SoCXin)

[CC2541](https://github.com/SoCXin/CC2541)同系列[CC2540](https://github.com/SoCXin/CC2540) 具有USB功能（没有IIC），相似产品[CC2530](https://github.com/sochub/CC2530)支持ZigBee，友商竞争产品[nRF51822](https://github.com/sochub/nRF51822)，相似产品还有[nRF24LE1](https://github.com/SoCXin/nRF24LE1) 

###  [SoC芯平台](http://SoC.Xin) 
