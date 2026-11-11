# Blinds controller KiCad design

**Current fabrication package:** [corrected solder-mask ZIP](manufacturing/BlindsController-Gerbers-2026-11-10-mask-fix.zip). This supersedes the original ordered package; replace the order files with this complete export.

The reviewed KiCad PCB is routed on two copper layers. The 12 V motor supply trace is 1.2 mm wide for the specified 1 A load, assuming 1 oz copper. GND traces are 1.2 mm wide; +5 V and +3.3 V supply traces are 0.6 mm wide; motor outputs are 0.6 mm. Signal traces are 0.25 mm. Vias are 0.8 mm with 0.4 mm drills.

## Changes for review

- Preserved the component placements and used the PCB's concise values consistently in both views.
- Corrected the SparkFun TB6612FNG v11 breakout mapping using its Eagle board: 20.32 mm square module, 15.24 mm header separation, 2.54 mm pitch. Pad identifiers now use SparkFun header/pin identifiers JP1.1 through JP1.8 and JP2.1 through JP2.8.
- Top view, VM at upper left: left row top to bottom = VM, VCC, GND, AO1, AO2, BO2, BO1, GND; right row = PWMA, AIN2, AIN1, STBY, BIN1, BIN2, PWMB, GND.
- Connected all three driver ground pins in the netlist. VCC is 3.3 V; VM is 12 V. Buck EN remains intentionally open.
- Repaired the schematic input-capacitor wire that joined 12 V to GND and repaired sensor wire endpoint gaps.
- Corrected MAGL to physical ESP32 header pad 7 (GPIO32); pad 13 is GPIO12 and is unused. Firmware remains GPIO32 for MAGL and GPIO14 for MAGR.
- Added four non-plated 2.7 mm mounting holes for M2.5 standoffs. Centers: (116,68.5), (183,68.5), (116,129.5), (183,129.5) mm. Expanded each board edge by 4 mm to preserve placements and clear the hardware. Outline is now 74 x 68 mm.
- Restored stable schematic UUID paths on every footprint and added matching project-local symbol/footprint libraries. Open BlindsController.kicad_pro; normal Update PCB from Schematic uses these links.

## Source files and checks

The .kicad_sch and .kicad_pcb files are the maintained sources. The Blinds.pretty directory, Blinds.kicad_sym, fp-lib-table and sym-lib-table are part of the project and must travel with it. Historical generators are archived and must not overwrite the edited design.

ERC: zero violations. Schematic/PCB parity: zero issues. Current PCB DRC reports zero violations and zero unconnected items. Schematic/PCB parity reports zero issues. See erc.txt and parity.txt for the current reports.

Local working backups and historical generators are ignored by Git. The ordered fabrication release and source snapshots are retained under manufacturing/2026-11-10/.

References:
- https://github.com/sparkfun/Motor_Driver-Dual_TB6612FNG/blob/master/Hardware/SparkFun_Motor_Driver-TB6612FNG_v11.brd
- https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html

## Routing cleanup

Replaced the grid staircases with straight clearance-checked runs and simple bends. Track segments decreased from 1,093 to 196, with 31 vias. The 12 V motor supply remains 1.2 mm wide. Footprint placements and net assignments are unchanged. Current checks: zero DRC violations, zero unconnected items, zero schematic/PCB parity issues. Prior routing drafts are retained locally in ignored backups.

## Ground planes

Added filled GND zones on F.Cu and B.Cu with 0.25 mm clearance and disconnected islands removed. The ESP32 antenna end is excluded from both pours. Input connector J3 GND and all three driver J2 GND pads use solid connections to the planes; other pads use thermal reliefs where feasible. GND traces are 1.2 mm wide, matching the 12 V rail. The input ground joins the filled plane through a 1.2 mm wide bottom-layer entry. Both planes are connected by through-hole GND pads. No component placements were changed. Final DRC and schematic/PCB parity: zero violations and zero unconnected items.

## Fabrication export

The original ordered ZIP, now superseded by the mask correction, is `manufacturing/BlindsController-Gerbers-2026-11-10.zip`. Fabrication notes, source snapshots, export hashes and final check reports are in `manufacturing/2026-11-10/`. This export was made after refilling the ground zones and passing DRC, ERC, connectivity and schematic/PCB parity checks. Board specifications: 74 x 68 mm, two copper layers, 1.6 mm FR4, 1 oz copper per side.

## Version control

Commit the KiCad project, schematic, board, Blinds.pretty/, Blinds.kicad_sym, and both library tables together. Release Gerbers, drill files, the ordering ZIP, check reports, source snapshots, and any BOM CSV outputs belong in Git. Local session files, locks, working backups, temporary candidates, and historical generators are ignored. No BOM CSV had been generated at the time of this initial CAD commit.

## Solder-mask correction

The former custom component pads omitted mask layers. All 84 plated component pads now have explicit openings on both mask layers, verified in the exported Gerbers (88 flashes per side including the four mounting holes). Standard KiCad footprints now replace the resistor, test-point and LED placeholders; the other standard component footprints reference their official library IDs. Only ESP32 and TB6612 modules remain in Blinds.pretty. LED symbol numbering was adjusted to standard pin 1 cathode / pin 2 anode while preserving physical polarity and wiring. DRC, ERC and schematic/PCB parity pass. Use the mask-fix release and its current source snapshot for manufacturing.
