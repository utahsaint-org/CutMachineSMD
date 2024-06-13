# CutMachineFW

### Parts List:
- 2020 Extruded Aluminium cut to
  - 2x 30cm - Top/Bottom
  - 3x 26cm - Verticals
  - 2x 20cm - Legs
  - 1x 155mm - Horizontal Crossmember
  - 1x 155mm - Roll Attachment Crossmember (Optional)
- 2020 Series Connectors and Corner Bracket Set (30 Piece)
- Stepper Motors
  - NEMA23 4.2Amp Motor (StepperOnline 23HS45-4204S)
  - NEMA17 1.2Amp Motor (StepperOnline 17HS19-2004S)
- Motor Drivers
  - 1x EASON Stepper Motor Driver TB6600 4A
  - 2x Stepper Motor Driver DRV8825
- Cutting Blades - 9mm Knife Replacement Blade Cartridge
- HiLetgo CNC Drive Expansion Board (CNC Shield)
- 2.42" 128x64 OLED LCD Display (SSD1309) i2C
- M3 Hardware
  - Assorted M2 Length Screws, Nuts, Washers
  - Assorted M3 Length Screws, Nuts, Washers
  - Assorted M5 Length Screws, Nuts, Washers
  - M3 Lever Steering Linkage
  - M3 Link Strut shaft 35mm
- 4x 2020 Series Rubber Feet
- 1x Miniature Linear Sliding Guideway Rail 150mm

### OLED Screen Modification
The OLED screen generally comes configured for SPI interface, and we are using I2C protocol in this project.   As such, you will need to modify the OLED screen by bridging several solder pads to put the screen into I2C configuration.   You will need to disconnect the ribbon-cable and underneath you will need to bridge the solder pads labeled: R9, R10, R11, R12.  You will also need to remove the Resistor labeled R8. (Follow the published instructions for the screen to make this modification)

### Building the Framework
The Framework for the machine is fairly stratight-forward.  The 30cm struts are mounted as the top and bottom rails.  Use normal corner attachments to make the connections.  The 26cm vertical struts are attached with one in the middle and the other two are attached on the edges to make a 30cm x 30cm frame.  The center vertical is attached at 105mm spacing from center to center.  You can use the Acrylic Cut Head mount as the spacing guide.   Lastly the cross-member can be attached at the horizontal mid-point.

![Frame Dimensions](Dimensions.png)
