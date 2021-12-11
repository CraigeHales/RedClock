# RedClock
## GPS + ESP32 + 7seg + laser cut box clock 

Nothing but a clock. No alarm, no fancy display. But...

* Sets, either from GPS time or SNTP. Discovers timezone via GPS.
* Dims at night.
* Has rest/JSON interface for BME (temperature/presure/humidity). Temp is high, inside case.
* Has rest/JSON interface for GPS sky.
* Has rest/JSON interface for WIFI recent APs.
* Has web interface for above and setup. Both local AP and STA mode.
* Has OTA (over the air) update via WIFI.

![2" tall 7-segment red LEDs in a laser cut box](/Images/front.png)

No idea if the vent slots are needed; they are very narrow slits that had to be carefully cleared of a very thin piece of cut out acrylic.

![View into case](/Images/back.png)

OshPark generated the PCB images for review before committing.

![pcb top](/Images/top.png)

V1 used very hot linear voltage regulators. Then I learned about the 120/240 vac to 12vdc efficient converter, and a smaller 12vdc to 5dc converter, also efficient.  

![pcb bottom](/Images/bottom.png)

This may or may not be the V3 schematic. It says V2, but I think it never got updated.

![circuit diagram](/Images/schematic.png)

This is *not* a cost effective project. I made two prototype runs of three boards and a final medium run of ten (and received 12, thanks!). Between the boards, parts, and case, at low volume, each clock may be around $100. 

I wanted to use the 2" displays that were < $2 each. And I wanted to use some drivers I had. I did learn a lot multiplexing the displays with an interrupt service routine. But if I was doing it over I'd look for a proper 7-seg driver, or more likely use a small video display. Each board probably took half a day to assemble into a completed clock. Maybe closer to a day if I was honest.

## Related posts
* https://community.jmp.com/t5/Uncharted/Tabbed-Box-Generator-in-JSL/ba-p/367718
* https://community.jmp.com/t5/Uncharted/JSL-BLOB-in-an-ESP32-Clock/ba-p/349164
* https://community.jmp.com/t5/Uncharted/What-time-is-it/ba-p/308205



## Instructions for web browser set up

The clock is likely delivered with a built in wifi access point named RCnn, or RCxxxxxx, where xxxxxx is derived from the mac address. I've been unsuccessful connecting with a cell phone (phone must be forcefully rebooted), quite successful with a desktop+wifi dongle. I use *avahi-browse -atr* to see a list of local servers on my wifi network.

The default user name is RC and password 88888888 (for wifi and for setup.)

After your computer connects to the clock's access point, direct your browser to http://192.168.5.1/ or http://RCnn.local or http://RCxxxxxx.local 

You should land on the setup screen. 

![Setup screen](/Images/setupScreen.png)

On the left is a vertical bar of displays the clock supports with setup at the top. The time should be updating every few seconds. Check out the displays and come back to the setup screen. If you don't do anything else, you'll probably want to *change the password*. Read the words at the top of the setup screen; they mean you need to fill in a block, one block at a time, and save that block. To change the clock's password, the top left block, *MYFI*, contains the access point name (which is also the clock's name) and password. The single period represents the current value (88888888) and should be replaced with an 8-63 character password. You'll have to supply the current password to change it. This password prevents your neighbor from accessing the clock and, if you connect the clock to your wifi/router, makes it a little harder for other users on your wifi/router to mess with your clock (the password is sent in clear text. Hopefully you are using WPA encryption, but a sniffer can grab it from the lan if it is connected to the wifi/router access point.) Users on your network *can* use the non-setup data from the various displays, without the password.

don't forget to save. Remember the name if you changed it. Click Reboot/Restart Clock Now button. Wait...

If the *password is lost*, wave a strong magnet back and forth on the minutes end of the clock right after power up to reset the clock to factory default 88888888. Once you see the display change, stop waving.

If you choose to put the clock on the wifi/router access point, in the WIFI box turn on wifi and put in your router credentials. This is your wifi/router password that you use in all your other devices that connect via your home's wifi. 

don't forget to save. Click Reboot/Restart Clock Now button. Wait...

Once you do that, you can change the AP your desktop is using back to the home wifi/router. If you are lucky, something on your network will make http://RCnn.local/ be known. If not, you might need to hunt for it. Many wifi/routers will produce a list of IP addresses of connected devices; these will change when the router reboots. http://192.168.1.123 would be typical.

For the clock to use SNTP off of your wifi/router, pick one of the SNTP options in the Time source block. I do *not* recommend getting the time from an unsecured router's HTML date header; I'm unclear if there are additional security issues. If you want to do that, the wifi password is empty for the router you want to steal the time from.

You can save anything you like in the location field.

The recent watcher changes the percentage of time the clock spends watching for recent wifi advertisements. It doesn't change much, but the clock may be harder to use if set to max.

Timezone should use GPS if possible. Otherwise put an appropriate string for your location.

Brightness can be changed; the sensor is a bit variable between clocks.

## Other screens

![Status screen](/Images/statusScreen.png)

The status display has a large table of internal parameters, many of which are not interesting and only used briefly during development.

![Recently-Heard screen](/Images/recentScreen.png)

The recently-heard display shows a list of recent AP's and recent GPS positions.

![Internal screen](/Images/internalScreen.png)

The internal display is used to update files (including some special, undisplayed names for the binary executable and map files. If you are going to try to update the executable or the map binaries, copy them locally, and on the scary internal screen, set the path on server to *Executable_Update.Bin* or *Timezone_Update.Bin* ... case matters. Try not to have a power failure while uploading to the clock. If it gets bricked, you might need to attach a USB cable and have the IDF download again. It will probably be OK, but there isn't really enough error checking for the timezone file. You can see version info on the status screen. You might also need to update the files from the spiffs folder; they have the right names already. The pushall.py script deletes all the spiffs files it knows about and reloads them, biggest first, which seems to recover the spiffs directory.)

![About screen](/Images/aboutScreen.png)

Nothing to see here. This is a static page, complete with an browser cache expiration date a few hours in the future, like the css and js files.

There is no logout screen; pressing the logout button attemps to make the browser forget the basic auth credentials and returns to the setup screen.


Watch the RedClock adjust from Daylight to Standard time...

<div>
  <a href="https://www.youtube.com/watch?v=JaAEYl-IkMY"><img src="/Images/video.png" alt="Short video of 2AM to 1AM fall-back time change"></a>
</div>

## License
Code I wrote is BSD license, see license file.
There is some Apache license code in the IOT directory.
Some code came from StackOverflow. *I do not know the licensing status of that code.*
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/COPYRIGHT.html describes many licenses that might apply to code that might be mixed in here as well.

