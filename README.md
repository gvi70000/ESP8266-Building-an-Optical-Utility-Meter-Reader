# ESP8266-Building-an-Optical-Utility-Meter-Reader
![PCB_Bottom](https://github.com/gvi70000/ESP8266-Building-an-Optical-Utility-Meter-Reader/assets/248221/f64e4562-4cf8-498c-91a5-36396601064c)

![PCB_Top](https://github.com/gvi70000/ESP8266-Building-an-Optical-Utility-Meter-Reader/assets/248221/1e3faf1f-f343-4d21-a392-6de393a19b5c)
Introduction:

In the era of smart homes and energy efficiency, monitoring and managing your utility consumption has become a key aspect of modern living.

This article introduces a DIY project that combines new technology with a user-friendly approach, presenting an Optical Utility Meter Reader based on the ESP8266 microcontroller, BPW34 photodiode, and OPA2348/OPA2354 operational amplifier.

This intelligent device not only counts LED pulses from your utility meter but also seamlessly connects to your home network via WiFi and MQTT, offering a convenient and efficient way to integrate energy data into your smart home ecosystem.

This device will work only with utility meters that have a LED to indicate the consumption, for example my energy meter has a LED that blinks 1000time/kWh. It is marked on the meter as 1000 imp/kWh followed by a square pulse symbol = 1Wh.

Components and Technology:


ESP8266 for WiFi Connectivity: The ESP8266 is a powerful and cost-effective microcontroller with built-in WiFi capabilities. Leveraging its connectivity features, this Optical Utility Meter ensures seamless communication with your home network, allowing you to access real-time energy data from anywhere.

BPW34 Photodiode for Precision Sensing: The BPW34 photodiode serves as the optical sensor in the meter. Its high sensitivity to light ensures accurate counting of LED pulses from your utility meter. This precision is crucial for providing reliable energy consumption data, allowing you to make informed decisions about your usage patterns.

OPA2348/OPA2354 Op-Amp as a Schmidt Trigger: The OPA2348/2354 op-amp is utilized as a Schmidt trigger, helping to shape and stabilize the incoming signal from the photodiode. This ensures that the ESP8266 receives a clean and well-defined signal for processing, enhancing the overall reliability and accuracy of the meter.

MQTT Integration for Smart Home Connectivity: MQTT (Message Queuing Telemetry Transport) is a lightweight and efficient messaging protocol ideal for IoT devices. The Optical Utility Meter employs MQTT to transmit the pulse count data to a designated MQTT broker. This data can then be easily integrated into your smart home system, providing a comprehensive overview of your energy consumption.

Building the Optical Utility Meter Reader: The construction of this device involves connecting the components, programming the ESP8266 to handle the incoming data, and setting up the MQTT communication. Detailed step-by-step instructions and code snippets will be provided to guide you through the process, making it accessible to both beginners and experienced DIY enthusiasts.

The KiCAD schematic and PCB are provided in order to change them if you feel the need and also to make the understanding of the circuit easier.

The R2 value can be changed depending on the ambient light conditions, the larger it is the larger the voltage at the op-amp, I am using a 1MOhm resistor for R2.

The threshold voltage is set by the voltage divider formed by R10 and R11, I have used 470Ohm for R10 and 10kOhm for R11 to get a trigger voltage of 3.15V and also to be able to keep IO3 of the ESP high at boot.

With the values stated above I can read 1000pulses/sec from a 3mm red led 2cm away from photodiode.

The ESP8266 shall be connect with the VCC pin to the + on the PCB and GND to the - on the PCB. UART is not used so you can also use Receive and Transmit pins as you like.

I have attached also the stl files to 3D print the enclosure parts are: Upper cover and Lower Cover. You will need a small self tapping screw with a diameter smaller than 2mm to fix the PCB in the Lower cover. The Upper Cover, after the power cable is connected through the hole, can be glued in place.

Double sided adhesive tape is used to fix the assembled enclosure on the meter.
![Enclosure _1](https://github.com/gvi70000/ESP8266-Building-an-Optical-Utility-Meter-Reader/assets/248221/af06db09-3dfa-4cad-b613-ba467763b9ee)
![Enclosure _2](https://github.com/gvi70000/ESP8266-Building-an-Optical-Utility-Meter-Reader/assets/248221/402e7fb1-e769-424c-91fe-274e9be18a09)
