kicad 5.0.2; oshpark takes the pcb file directly, BUT DON'T MAKE THIS PCB without thinking it through!

* open the clock.pro file
* click the icon with the transistor to open the schematic
* OK the initial prompt about rescue old parts, seems to work.
* Red lady bug, run. Should say OK.
* next icon, assign footprints, OK.
* Net icon, generate, save.
* PCB icon, looking at board.
* Red lady bug, report all errors, start DRC. Should be no errors, nothing unconnected.

If you open the 3D viewer, it looks like pins in the 38/40 pin dip are in the wrong place. The important thing is for the hole pattern to match the device; I used a 0.9" wide device with 38 pins from amazon; it has an unusual pinout. (I think I may have miscounted something for the pins; holes 1 and 10 in the tenth-inch perf board are not 1" apart.) There is a nifty measuring tool in the pcb software. Use it.
You probably should make a prototype before making a bunch. Actually, this is not a great design and you should probably rework it.
One small trick: Export as SVG for the mounting holes to import into the acrylic case pattern. I might wish I'd made the acrylic holes slightly larger than the pattern in case/box*.svg.

It seems really unlikely that some of the parts this design uses will be available long term. The 12V power supply and the 38-pin CPU may require some rework if you can't get the exact item. 

It also seems likely there may be something missing in the zip file. I'm sure I didn't (still don't) understand how to install parts and footprints into kicad in the right place for this to work.

The two 100uf filter caps were changed to 47uf each and the 10uf on the enable pin 2 was changed to 1uf or removed. This seems to greatly improve the boot reliability. I think the 12v converter was near the capacitance limit at 100+100 uf and may have had a too slow rise time to boot reliably. Also the random advice on the web for the 10uf cap to make usb programming work without pressing the button...seems to be too big and probably also caused some boot fails.
