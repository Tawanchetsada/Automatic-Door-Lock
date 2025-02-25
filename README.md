# Automatic Door Lock  

**Automatic Door Lock** is a smart access control system that enhances security and convenience using facial recognition and fingerprint scanning, combined with real-time notifications via LINE Notify.  

## Key Features  
- Facial Recognition: Uses an ESP32-CAM to authenticate users with registered faces.  
- Fingerprint Scanning: Ensures secure access with a fingerprint sensor.  
- Motion Detection: PIR sensor detects movement to activate the system.  
- Real-time Alerts: Sends LINE Notify messages when the door is unlocked.  
- Automatic Flash Control: Adjusts LED lighting based on ambient light using an LDR sensor.  
- Ultrasonic Distance Measurement: Monitors door movement using an ultrasonic sensor.  
- Servo Lock Mechanism: Controls door locking and unlocking.  

## Hardware & Software  
### Hardware Components  
- ESP32-CAM – Facial recognition  
- Fingerprint Sensor – Fingerprint authentication  
- PIR Sensor – Motion detection  
- LDR Sensor – Ambient light detection  
- Ultrasonic Sensor – Distance measurement  
- LM2596 DC-DC Module – Power regulation  
- Servo Motor – Lock control  

### Software & Tools  
- Arduino IDE – Microcontroller programming  
- LINE Notify API – Real-time notifications  

## Status & Testing  
- Successful Authentication: Unlocks with registered faces or fingerprints.  
- Unauthorized Attempt Prevention: Blocks unregistered access attempts.  
- Real-time Notifications: Alerts sent via LINE Notify.  
- Automatic Light Adjustment: Flash activates in low-light conditions.  

## Future Improvements  
- Cloud-based user data storage for better access management.  
- Mobile app integration for remote access control.  

Developed by: Tawanchetsada
