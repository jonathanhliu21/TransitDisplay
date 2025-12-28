# Transit Departure Display

This project involves a self-contained display which shows upcoming transit departures. The display can be used in coffee shops, office lobbies, or hotel lobbies to serve as transit information for travelers or commuters, allowing them to plan their travel or commute more conveniently without having to navigate through complex transit apps.

![IMG_8822](https://github.com/user-attachments/assets/6282f56d-d886-4ed4-bb06-226dd19f8674)

## Context

Live departure boards give commuters and travelers important transportation information. They allow travelers to plan their schedule around when the next train or bus departs, and they notify commuters of delays.

Many transportation agencies in the US provide GTFS feeds. These give us geographical locations of stops, the routes which serve the stops, and most importantly, the live positions of transit vehicles. This allows us to calculate departure times for transit vehicles. APIs like [Transitland](https://www.transit.land/) provide a centralized place for us to access this data, allowing us to create a live transit departure board.

## Problem

Many bus stops do not have departure boards which indicate the time at which the next bus or train arrives. This may create a source of confusion for travelers or commuters who are not aware of the current schedule. Additionally, transit information cannot be intuitively accessed on apps like Google Maps, which presents cluttered information within its menus.

## Solution

To solve this, I created a simple departure board that can be mounted on a desk or a wall. It was designed to mirror the simplicity of train/metro station boards, providing essential departure information at a glance.

This project served as an opportunity for me to deepen my knowledge of circuit design and embedded IoT. The departure board was created using an ESP32 and a TFT LCD display. To make sure that the electronics were housed compactly, I created a custom PCB rather than using breakouts.

The PCB features:

* An ESP32-WROVER-E module, which acts as the main processing unit  
* A TFT LCD display, which uses SPI  
* A CH340C USB to Serial converter  
* A USB-C to provide power, along with power protection circuits
* A 3V3 linear voltage regulator  
* LED indicators for power, the TX and RX Serial data lines, and API data retrieval errors  
* A boot and reset button for the ESP32
* Two auxiliary buttons

I learned key circuits, electronic components, and PCB design standards by creating my own PCB for this project. For example, I learned that decoupling capacitors smooth out noise in the input voltage. I also learned the role of using MOSFETs with the boot and reset buttons, as they are voltage controlled and do not require further current draw from the gate.

<img width="640" height="500" alt="Screenshot 2025-09-14 at 2 29 36 PM" src="https://github.com/user-attachments/assets/aff57fec-bea0-4c9a-ad31-f08fa1476205" />


I utilize the [Transitland](https://www.transit.land/) API for data retrieval. This allows users to specify any station for any supported transit agency, without having to set up a specific API or a specific parsing format. All data is parsed with ArduinoJSON, and I use streaming and filters to ensure that memory usage is not unnecessarily high.

In order to specify which stop(s) to retrieve departure from, the user specifies a coordinate, a search radius, and a transit agency filter which consists of Transitland OneStop IDs. This is stored in `UserConstants.h`, and then uploaded onto the ESP32.

## Key Features

* **Multiple Location Retrieval:** The user can specify multiple locations to retrieve transit data from.  
* **Live Updates**: The departure board provides live updates, refreshing every 60 seconds.  
* **Delay Indicators**: Red, yellow, and green colors on the time indicator show whether a vehicle is delayed, early, or on time.

## Usage

1. First, clone the repository:

```  
git clone https://github.com/jonathanhliu21/TransitDisplay  
```

2. Create a `secrets.h` file under `src/`, and copy the below template. Then, specify your network name under `SECRET_SSID`, your network password under `SECRET_PASSWORD`, and your [Transitland API key](https://www.transit.land/documentation/#signing-up-for-an-api-key) under `SECRET_API_KEY`.

```cpp  
#ifndef ARDUINO_SECRETS_H  
#define ARDUINO_SECRETS_H

namespace Secrets
{
  inline constexpr const char *SECRET_SSID = "[Your Wifi Network Name]";
  inline constexpr const char *SECRET_PASSWORD = "[Your Wifi Password]";
  inline constexpr const char *SECRET_API_KEY = "[Your Transitland API Key]";
}

#endif  
```

3. Go into `src/UserConfig.h`  
   1. Specify an allowlist filter of transit agencies to display under `userWhiteList` using the Transitland [operator Onestop IDs](https://www.transit.land/operators)  
      1. Click on the link above, click into a specific operator, and copy the “Onestop ID”  
      2. Feel free to delete the provided list and replace with your own list  
      3. **Not recommended**: If you want all agencies to show up, set `userWhiteListActive` to `false`. However this is highly not recommended because it may lead to very long API calls, increasing the chances of an API call error  
   2. Specify a list of zones under `userTransitZoneList`  
      1. The format is a tuple of size 4: name, latitude, longitude, search radius  
      2. Feel free to delete the provided list and replace with your own list  
4. Upload onto your ESP32  
   1. My PCB uses an ESP32-WROVER-E  
   2. You may need to download a driver. My PCB uses a CH340C, with driver installation instructions found [here](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all)  

## Next Steps

* **Arrival Data**: Currently the display only shows departure data. However, arrival data is also useful in certain cases, such as determining when to pick someone up. An arrival mode can be added to show when a certain vehicle arrives, allowing people such as taxi or rideshare drivers to plan around a specific arrival time.
* **Name Filtering**: Some GTFS feeds provide unnecessarily long route or stop names (e.g. LA’s Metro J Line is provided as `Metro J Line (910/950)`). I have created a filter that checks for hardcoded names, but a more sophisticated name filtering algorithm can be made for more generalized filtering.  