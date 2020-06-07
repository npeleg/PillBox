# PillBox

PillBOX is a smart pill box for patients.  
After the pharmacist fills it with pills and configures pill taking hours, the patient can take the device home and configure the Wi-Fi credentials and a contact list on the PillBOX website. The PillBOX can then be placed in a visible location and begin its work.  

When it is time to take a pill, PillBOX lights up the exact cell in which the pills are located using a red LED.  
The PillBOX uses a magnetic hall effect sensor to detect if the cell has been opened after the LED has been lit, thus determining if the pill was taken or not. If after 30 minutes the pill was not taken, an SMS alert is sent to the pre-configured contact list, letting them know that the pills weren't taken, and that they should contact the patient (See appendix A for a flow chart describing this procedure).  

The motivation for such a product is that most of the people who use such pill boxes are elderly people, who tend to forget taking their pills or which pills they are supposed to take. Our system alerts them via LED and alerts family members of friends if pills were not taken, allowing the contacts to take action to make sure the pills are taken on time, each time.

## System Components

I. Texas Instruments CC1350 LaunchPad – runs the BLE app and the main PillBOX application logic, including sensor logic.

II. Adafruit feather HUZZAH ESP8266 – connected to the CC1350 for Wi-Fi functionality.

III. PillBOX Android App – allows the pharmacist to connect to the CC1350 using BLE and configure current time and pill taking hours.

IV. PillBOX Website – written in python, allows clients to configure their list of contacts, and an admin user to supervise over all clients. The website was uploaded to Amazon EC2 cloud computing service onboard an ubuntu 16.04 virtual machine. The website also exposed a REST API for the feather huzzah to trigger alerts from the board itself.

V. Amazon SNS – the Simple Notification Service by Amazon cloud services was used to manage clients contact lists and send them SMS notifications when triggered by the board. The services' API was accessed from the PillBOX website.
    
&nbsp;
    
![Imgur](https://i.imgur.com/xSzMVQU.png) 

&nbsp;

## System flow chart

![Imgur](https://i.imgur.com/0NTAcjE.png)
