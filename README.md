# Arduino-LoRa-Mesh

This repository contains a series of simple Arduino projects that were used or developed in the context of the doctoral thesis [Towards LoRa mesh networks for the IoT](http://hdl.handle.net/2117/360904).

## Simple LoRa sender/receiver

The "Simple_LoRa_sender" and "Simple_LoRa_receiver" folders contain, respectively, two very simple projects based on the [RadioLib SX127x examples](https://github.com/jgromes/RadioLib/tree/master/examples/SX127x). One is used to send LoRa packets on a given frequency, bandwidth, SF, etc. and the other to receive them and print some of the reception details through the serial port.

These projects were used to benchmark a LoRa point-to-point link, using two [LILYGO TTGO ESP32 LoRa32 v2.1 T3_1.6](http://www.lilygo.cn/prod_view.aspx?TypeId=50003&Id=1130&FId=t3:50003:3) devices, and compare it with the stripped-down LoRa code from [FLoRaMesh](https://github.com/DSG-UPC/FLoRaMesh) (a derivative work from [FLoRa](https://flora.aalto.fi/)):

![Throughput comparison between hardware-based LoRa point-to-point link and FLoRaMesh simulated link.](/img/benchmark_flora_ttgo_throughput.png "Throughput comparison between hardware-based LoRa point-to-point link and FLoRaMesh simulated link.")

As shown in the image above, the throughput curves are pretty similar (note the simulator allows for an arbitrarily big payload, while the hardware LoRa library is limited to 256 bytes). The image is available [here in EPS format](img/benchmark_flora_ttgo_throughput.eps).
