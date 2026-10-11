# Starbiew — Adding Sound, Light Sensing & Haptics

### A complete, beginner-friendly build guide

This guide takes the **Starbiew** board — a star-shaped variant of
[SharKingStudios' Starbie](https://github.com/SharKingStudios/Starbie) Week 1 project — and adds three
features to it, from empty schematic to working firmware.

| # | Feature | What it adds to the pet |
|---|---------|-------------------------|
| 1 | **Piezo buzzer** | Real chiptune melodies — happy, sad, sleep, alert sounds |
| 2 | **LDR light sensor** | The pet knows when it's dark: it naps at night, wakes in daylight, and the LEDs change palette |
| 3 | **Vibration motor** | You can feel the pet react without looking at it |

> ### ✅ Verified, not just written
> This is not a guide someone typed from memory. Everything in it was checked against real tools:
>
> | Check | Result |
> |-------|--------|
> | Firmware compiles for the XIAO ESP32-C3 | **Pass** — `arduino-cli 1.5.1`, ESP32 core **3.3.12**, 0 errors |
> | Build size | 393,624 bytes flash (30%), 15,952 bytes RAM (4%) |
> | Same code with all 4 new features disabled | **Pass** — the `USE_*` switches work |
> | State-machine behaviour tests | **Pass** — 6/6 haptic patterns, 7/7 tunes, light hysteresis |
> | Every KiCad symbol & footprint name | **Verified** against a real KiCad **10.0** install |
> | Board facts (pins, nets, parts) | **Verified** from your actual `Starbiew.kicad_sch` netlist |
> | Tested library versions | Adafruit GFX 1.12.6, SSD1306 2.5.17, MPU6050 2.2.9, Unified Sensor 1.1.15, DHT 1.4.7, NeoPixel 1.15.5 |

**Parts cost:** about **$1.00–1.50** per board if you order properly.
**Your time:** roughly **4–6 hours** for a first build, spread over a few sessions.

---

## How to read this guide

There are three ways to use it. Pick one.

**🟢 Track 1 — "Just tell me what to do."**
Read [Part 0](#part-0--the-60-second-overview), skim [Part 3](#part-3--the-design-decisions-why-it-is-built-this-way),
then work through Parts 5 → 12 in order. Skip anything marked *Background*.

**🔵 Track 2 — "I want to understand it."**
Read straight through. Parts 1–4 explain the reasoning, 5–10 the build, 11 the code, 12–14 testing and next steps.

**🟠 Track 3 — "Something is broken."**
Jump to [Part 13 — Troubleshooting](#part-13--troubleshooting). It's a lookup table: find your symptom, follow the fix.
If your **compile** fails, go straight to [§11.6](#116-if-the-compile-fails).

### Symbols used in this guide

| Symbol | Meaning |
|--------|---------|
| 🛑 | **Stop.** Getting this wrong can damage the board or waste a board order. |
| ⚠️ | **Careful.** A common mistake that costs you an hour. |
| 💡 | **Tip.** Makes life easier; safe to ignore. |
| ✅ | **Check.** How to confirm the step worked before moving on. |
| 🔍 | **Background.** *Why* something works. Safe to skip if you just want to build. |

### How pins are written

Whenever this guide names a pin, it gives all the ways to identify it, like this:

> **D1 / GPIO3 / U1 pad 2 / ADC1_CH3**

* `D1` — the label printed on the XIAO module
* `GPIO3` — the chip pin number used in code
* `U1 pad 2` — the pin number in the KiCad schematic
* `ADC1_CH3` — its analogue-input channel, if it has one

You only need one of these to find the pin. The redundancy is on purpose.

---

## Table of contents

| Part | What it covers | Time |
|------|----------------|------|
| [0](#part-0--the-60-second-overview) | The whole plan at a glance | 3 min |
| [1](#part-1--before-you-start) | Tools, software, skills, words you'll see | 10 min |
| [2](#part-2--know-your-board) | What's already on your PCB, and how to check it | 10 min |
| [3](#part-3--the-design-decisions-why-it-is-built-this-way) | **The important part** — which pins, and why | 15 min |
| [4](#part-4--what-to-buy-and-where) | Exact parts, prices, and where to order | 15 min |
| [5](#part-5--drawing-the-schematic-step-by-step) | Drawing the schematic, click by click | 45 min |
| [6](#part-6--checking-the-schematic-erc) | Running the electrical check | 5 min |
| [7](#part-7--laying-out-the-pcb) | Placing and routing on the board | 60 min |
| [8](#part-8--ordering-the-pcb) | Exporting Gerbers and ordering | 30 min |
| [9](#part-9--soldering-it-together) | Assembly, with a soldering crash course | 90 min |
| [10](#part-10--building-without-an-enclosure) | Making the three features work on a bare board | 20 min |
| [11](#part-11--firmware) | The code, verified to compile | 30 min |
| [12](#part-12--testing-it-step-by-step) | Five tests, in order | 30 min |
| [13](#part-13--troubleshooting) | Symptom → cause → fix | — |
| [14](#part-14--ideas-for-later) | Where to go next | — |

---

## Part 0 — The 60-second overview

**What you're doing:** adding three small circuits to a PCB that already works, then flashing updated
code to the microcontroller that's already on it.

**The three circuits, in plain English:**

1. **Buzzer** — a flat metal disc that clicks when you switch a pin on and off very fast. Different
   speeds make different musical notes. That's how you get melodies. *(Cost: ~$0.21)*
2. **Light sensor** — a resistor whose resistance drops when light hits it. Wire it with a normal
   resistor to make a "voltage divider", and the microcontroller can measure how bright it is.
   *(Cost: ~$0.06)*
3. **Motor** — a tiny motor with an off-centre weight that buzzes. It needs too much current to run
   straight off a chip pin, so a transistor switches it on and off. *(Cost: ~$0.40 total)*

**The one thing that actually matters:** which pins you connect them to. Get that wrong and the board
won't start. [Part 3](#part-3--the-design-decisions-why-it-is-built-this-way) explains it — read that
part even if you skip everything else.

**The plan:**

```
   Buy parts ──► Draw schematic ──► Lay out PCB ──► Order board ──► Solder ──► Flash code ──► Test
   (Part 4)      (Part 5)           (Part 7)        (Part 8)        (Part 9)    (Part 11)     (Part 12)
                                                                    └── board arrives in ~1-2 weeks ──┘
```

💡 **Order the parts and the board together, early.** Shipping is the long pole. Do Part 4 and Part 8
first if you're impatient, then come back and finish the design while you wait.

---

## Part 1 — Before you start

### 1.1 What skills you need

Honestly: **none to start.** Every KiCad click is spelled out in Part 5. You need to be able to install
software and follow instructions.

You'll also be doing some soldering. If you've never soldered, [§9.2](#92-how-to-solder-the-small-parts-smd)
and [§9.3](#93-how-to-solder-the-through-hole-parts) are written as a crash course. Nothing in this
design is smaller than 0805 (a 2 × 1.25 mm part) or SOT-23 (a 3-pin transistor), both of which are
hand-solderable with a normal iron and a steady hand.

🔍 **Background — why this design is beginner-friendly:** we deliberately picked through-hole parts
wherever possible (the buzzer, the LDR, the motor connector) and kept the surface-mount parts to
ordinary resistors and capacitors plus one small transistor. No fine-pitch chips, no BGA, no
microscope needed.

### 1.2 Software

| What | Version | Where | Notes |
|------|---------|-------|-------|
| **KiCad** | 9 or 10 | [kicad.org](https://www.kicad.org/download/) | Free, open source. This guide was checked against 10.0. You already have 10.0 installed. |
| **Arduino IDE** | 2.x | [arduino.cc](https://www.arduino.cc/en/software) | Free. You'll install the ESP32 board package inside it (Part 11.1). |

⚠️ The tutorial's screenshots come from an older KiCad. Dialogs may look different in 9/10 — the field
names are the same, just rearranged. Follow the field *names*, not the positions.

### 1.3 Tools

**Essential:**

| Tool | Why you need it | Budget |
|------|-----------------|--------|
| Temperature-controlled soldering iron | The single most important tool | **PINECIL V2 ≈ $26** (needs a separate 65 W USB-C PD power supply) or **Hakko FX-888DX ≈ $120** |
| Solder wire, 63/37 rosin-core, 0.5–0.6 mm | Thin solder is much easier for small joints | ~$8–16 |
| Flux pen (e.g. MG Chemicals 835-P) | Turns "impossible" SMD work into easy work | ~$11 |
| Digital multimeter with a continuity beeper | To check for shorts **before** powering up | **Klein MM325 ≈ $35** (the MM300 is discontinued) |
| Tweezers | Placing tiny parts | ~$8 |
| Side cutters | Trimming through-hole legs | ~$8 |
| Solder wick or a desoldering pump | You will bridge two pads. This fixes it. | ~$5 |

**Nice to have:**

| Tool | Why |
|------|-----|
| USB power meter (~$10) | Shows you the current draw instantly — tells you a board is shorting *before* anything gets hot |
| Magnifier or phone with a good macro camera | Inspecting small joints |
| Helping-hands or a PCB holder | Keeps the board still |

💡 You do **not** need a hot-air rework station, a microscope, or a reflow oven for this build.

### 1.4 The words you'll see

Read this once; it'll make the rest much easier.

| Word | Plain English |
|------|---------------|
| **Schematic** | The logic drawing: what parts exist and what connects to what. No physical size. |
| **Footprint** | The physical land pattern for a part — where its pads and holes go on the board. |
| **Symbol** | The drawing of a part *in the schematic* (usually a rectangle with pins). |
| **Net** | One electrical connection. Everything on a net is connected together. "GND" is a net. |
| **Net label** | A text tag that names a net. Two wires with the same label are connected without drawing a wire between them. Your project uses these heavily, which is why the changes in this guide are so small. |
| **THT** | Through-hole: the part's legs go through the board and you solder them on the back. Easy. |
| **SMD / SMT** | Surface-mount: the part sits on top of pads. Small, no holes. |
| **0805** | An SMD size: 2.0 × 1.25 mm. Hand-solderable. KiCad calls it `R_0805_2012Metric`. |
| **SOT-23** | A 3-pin SMD transistor package. |
| **GND pour / zone** | A big area of copper connected to ground, filling unused space on the board. |
| **ERC** | Electrical Rules Check — KiCad checks the *logic* of your schematic. |
| **DRC** | Design Rules Check — KiCad checks the *physical* layout (clearances, missed connections). |
| **Gerber** | The file format sent to a PCB factory. |
| **GPIO** | A chip pin you can switch on/off or read. |
| **ADC** | Analogue-to-digital converter. Measures a voltage. Needed for the light sensor. |
| **PWM** | Pulse-width modulation — switching a pin on and off rapidly to dim an LED, set motor strength, or make a musical tone. |
| **LEDC** | The ESP32's PWM hardware. This is what makes tones. |
| **RMT** | Another ESP32 hardware block, used to talk to WS2812B LEDs. |
| **Strapping pin** | A pin whose voltage is read **once at power-on** to decide how the chip starts. Get it wrong and the board may not boot. **This is the trap in Part 3.** |
| **Netlist** | The schematic exported in a form the PCB editor can read. In KiCad: press **F8**. |
| **Flyback diode** | A diode across a motor that safely absorbs the voltage spike when it switches off. Required, not optional. |

### 1.5 Prerequisites

* The Starbiew project files: `Starbiew.kicad_pro`, `.kicad_sch`, `.kicad_pcb`. You have these.
* 💡 **You do not need to install any extra KiCad libraries.** Every symbol and footprint used in this
  guide ships with KiCad itself. The tutorial's "Care Package" is only needed for the original board's
  custom OLED and XIAO footprints, which your project already contains.

---

## Part 2 — Know your board

Five minutes here saves an hour later.

### 2.1 What you already have

| Item | Value |
|------|-------|
| Board size | **104.05 × 92.55 mm** |
| Layers | 2 (top and bottom copper) |
| Shape | Star, with stepped edges |
| Ground | A ground "pour" covering most of the board |
| Decorations | Two flower-pattern footprints on the top copper layer |

**The 12 real parts:**

| Ref | Part | What it does |
|-----|------|--------------|
| U1 | XIAO-ESP32-C3-SMD | The microcontroller — the brain |
| J1 | 4-pin header | 128×64 OLED display |
| J2 | 8-pin header | MPU6050 accelerometer (tilt + shake sensing) |
| U2 | DHT11 | Temperature and humidity |
| R1 | 10 kΩ | Pull-up resistor for the DHT11's data line |
| SW1, SW2 | Cherry MX switches | The two menu buttons |
| D1, D2 | WS2812B ×2 | Addressable RGB LEDs, chained together |
| R3 | 330 Ω | Series resistor on the LED data line |
| C1, C2 | 100 nF | Decoupling capacitors for the LEDs |

### 2.2 Which pins are free

This table is the whole basis of Part 3.

| XIAO | GPIO | U1 pad | Used for | Analogue? | Boot-strapping pin? |
|------|------|--------|----------|-----------|---------------------|
| **D0** | GPIO2 | 1 | *free* | **Yes** (ADC1_CH2) | 🛑 **YES — must be HIGH at power-on** |
| **D1** | GPIO3 | 2 | DHT11 data | **Yes** (ADC1_CH3) | No |
| **D2** | GPIO4 | 3 | SW1 (Button 1) | Yes (ADC1_CH4) | No |
| **D3** | GPIO5 | 4 | SW2 (Button 2) | Yes, but unusable (ADC2) | No |
| **D4** | GPIO6 | 5 | **SCL** → OLED + MPU6050 | No | No |
| **D5** | GPIO7 | 6 | **SDA** → OLED + MPU6050 | No | No |
| **D6** | GPIO21 | 7 | WS2812B data (through R3) | No | No |
| **D7** | GPIO20 | 8 | *free* | No | No |
| **D8** | GPIO8 | 9 | *free* | No | ⚠️ YES |
| **D9** | GPIO9 | 10 | *free* | No | ⚠️ YES — this is the BOOT button |
| **D10** | GPIO10 | 11 | *free* | No | No |
| — | 3V3_OUT | 12 | 3.3 V power rail | — | — |
| — | VBUS | 14 | 5 V from USB | — | — |
| — | GND | 13, 18, 22 | Ground | — | — |

**Also free:** J2 pins 5–8. A standard MPU6050 module only uses VCC, GND, SCL and SDA, so the XDA, XCL,
AD0 and INT positions on that header are unused.

### 2.3 ✅ Check this yourself (30 seconds)

1. Open `Starbiew.kicad_pro` in KiCad.
2. Click **Schematic Editor**.
3. `Tools → Assign Footprints`. Compare the list against the table in §2.1 — they should match.
4. `Tools → Electrical Rules Checker → Run ERC`. You should see a short, boring report with 0 errors.
5. Go back to the project window, click **PCB Editor**, then `Tools → Update PCB from Schematic`. It
   should say there are no changes. Press Cancel.

### 2.4 ⚠️ A known mistake in the original tutorial

**The Week 1 `Guide.md` text and the actual `Starbie.ino` file disagree about two pin numbers.**

| Source | SDA | SCL |
|--------|-----|-----|
| Week 1 `Guide.md` prose | 6 | 7 |
| The actual `Starbie.ino` file | **7** | **6** |
| **Your board (correct)** | **7** | **6** |

Your board wires SCL to D4/GPIO6 and SDA to D5/GPIO7 — confirmed from your own schematic netlist:

```
/SCL:  J1/3, J2/3, U1/5    ← U1 pad 5 = D4 = GPIO6
/SDA:  J1/4, J2/4, U1/6    ← U1 pad 6 = D5 = GPIO7
```

✅ **So the shipped `.ino` is right and the guide text is wrong.** If you ever re-derive these numbers,
trust the schematic. (The *comments* inside the `.ino` are also swapped, which doesn't help.)

If your OLED or accelerometer has never worked, this is almost certainly why.

---

## Part 3 — The design decisions (why it is built this way)

> 🔍 This is the *only* part of the guide where understanding the reasoning matters. If you read
> nothing else, read §3.2 and §3.3.

### 3.1 What we need

| Feature | What it needs from the chip |
|---------|------------------------------|
| Buzzer | 1 ordinary output pin |
| Light sensor | **1 analogue input pin** ← the scarce one |
| Motor | 1 ordinary output pin |

You have five nominally free pins: D0, D7, D8, D9, D10. But two of them are traps:

* **D8 (GPIO8)** and **D9 (GPIO9)** are strapping pins. D9 is literally the BOOT button. Anything that
  can pull them low at power-on is dangerous. **Leave both empty.**
* That leaves **D0, D7, D10** — exactly three.

Now the problem: **only D0 (GPIO2) is analogue-capable among those three.** So at first glance the light
sensor *must* go on D0.

### 3.2 🛑 Why the light sensor must NOT go on D0

> 🔍 **Background — what a strapping pin is.** When the ESP32-C3 powers on, it reads the voltage on
> three specific pins and uses those readings to decide *how to start up* — like a radio that checks
> whether a switch is up or down the moment you turn it on. This happens in the first few milliseconds,
> before any of your code runs. If a pin is at the wrong level at that instant, the chip can enter an
> undocumented state and simply not boot. Nobody's code can fix this, because your code hasn't started yet.

The ESP32-C3's strapping pins are **GPIO2, GPIO8 and GPIO9**, and the datasheet's boot table says:

| Strapping pin | Default level | SPI Boot | Download Boot |
|---------------|---------------|----------|---------------|
| **GPIO2** | *none defined* | **HIGH** | **HIGH** |
| GPIO8 | *none defined* | Don't care | HIGH |
| GPIO9 | Weak pull-up | HIGH | LOW |

*(Source: Espressif, [ESP32-C3 Wireless Adventure §5.2.6](https://espressif.github.io/esp32-c3-book-en/chapter_5/5.2/5.2.6.html))*

**GPIO2 must be HIGH at power-on.** And Seeed does not fit a pull-up or pull-down to it on the XIAO.

Now look at what a light sensor does. An LDR is a resistor that changes with light, and it has to be
paired with a normal resistor to make a voltage divider. A divider produces a voltage near **zero** at
one end of its range — and there is no way to avoid that, whichever way round you wire it:

| How you wire it | In a bright room | In a dark room |
|-----------------|------------------|----------------|
| 3V3 → 10 kΩ → pin → LDR → GND | ~0.3 V (**LOW** 🛑) | ~3.3 V (HIGH ✅) |
| 3V3 → LDR → pin → 10 kΩ → GND | ~3.0 V (HIGH ✅) | ~0.03 V (**LOW** 🛑) |

Either way, the pin sits at logic-low in one common lighting condition. Power the board on in that
condition and it may fail to start — intermittently, which is the worst kind of bug.

**You can't fix this with resistor values.** The LDR's resistance spans roughly 1 kΩ (bright) to 1 MΩ
(dark) — a 1000:1 range. Any divider across that range reaches near 0 V at one end.

### 3.3 ✅ The fix: swap the DHT11 and the LDR

The DHT11 is a **purely digital** sensor. It doesn't need an analogue pin at all. So:

* move the **DHT11 data line** from **D1 / GPIO3** to **D0 / GPIO2**, and
* put the **LDR** on **D1 / GPIO3 (ADC1_CH3)**.

This costs one label move, and it makes the board *safer than it is now*. Here's the nice part:

> Your DHT11 data line already has a **10 kΩ pull-up resistor to 3.3 V** — that's R1. Put the DHT11 on
> GPIO2 and R1 holds GPIO2 **HIGH** at power-on, which is exactly what the boot table requires.

So the swap:

* gives the light sensor a clean analogue pin with no boot constraints,
* turns D0 from "dangerous" into "ideal", because it's now pulled up,
* and doesn't affect the DHT11 at all — it only pulls the line low *after* boot, when the code asks it to.

### 3.4 Why not use D3 (GPIO5) for the light sensor?

Because it's ADC2, and ADC2 doesn't work reliably on this chip:

* Espressif **withdrew ADC2 support** on the ESP32-C3 because of a hardware erratum
  ([source](https://docs.espressif.com/projects/esp-chip-errata/en/latest/esp32c3/03-errata-description/shared/sar-adc-adc2-not-work.html)).
* Seeed's own XIAO docs warn that the A3/GPIO5 analogue input may be inoperative and recommend the ADC1
  pins (A0–A2).

So the board has exactly **three** usable analogue pins, and your two buttons already own two of them.
Hence §3.3.

### 3.5 The final pin plan

| Feature | Net name | XIAO | GPIO | U1 pad | Why here |
|---------|----------|------|------|--------|----------|
| Piezo buzzer | `BUZZ` | D10 | GPIO10 | 11 | Free, no boot constraint, can make PWM tones |
| Vibration motor gate | `HAPTX` | D7 | GPIO20 | 8 | Free, no boot constraint, can do PWM |
| Light sensor | `LIGHT` | D1 | GPIO3 | 2 | **Analogue input, and not a strapping pin** |
| DHT11 *(moved)* | `DHT11 DATA` | D0 | GPIO2 | 1 | Digital signal; R1's pull-up makes GPIO2 boot-safe |
| *spare* | — | D8, D9 | GPIO8, GPIO9 | 9, 10 | Deliberately unused |
| *spare* | — | J2 pins 5–8 | — | — | Already on your MPU header |

### 3.6 Two problems this design has to solve

Not optional polish — without these, the features misbehave.

**Problem 1: your own LEDs will blind the light sensor.**
You have two WS2812B LEDs on this board. An LDR next to them measures *your LEDs*, not the room. At
night, with the LEDs on, it will cheerfully report "daytime".

*Fix, in two parts:*
* **Mechanical:** mount the LDR as far from D1/D2 as the star shape allows, and rotate it so its sensing
  face points *away* from them. Then slide a short piece of **black heat-shrink tube** over the body so
  only the top face is exposed — that turns an omnidirectional sensor into a directional one and blocks
  the LED glow almost completely.
* **Firmware:** the code switches the LEDs off for 2 ms while it reads the LDR. Two milliseconds is
  invisible to the eye and removes the error completely. **This is already built into the code in
  Part 11 — you don't have to write it.**

**Problem 2: the vibration motor will fake "shake" gestures.**
Starbie's code watches the accelerometer for motion (a shake, to make the pet react). A vibration motor
bolted to the same PCB produces *exactly* that signal. Fire the haptics and the pet will think it's
being shaken, change its own stats, and save them.

*Fix, in two parts:*
* **Firmware:** the code ignores shake detection while the motor runs *and* for **500 ms** afterwards
  (coin motors coast). **Also already built into Part 11's code.** That guard is doing more work in a
  bare-board build than it would in a cased one — see §10.3.
* **Mechanical:** keep the motor as far from U1 as the board allows, and stick it down with a small piece
  of foam tape. With no case to absorb the vibration, the PCB itself is what shakes.

### 3.7 Power check

Everything running at once, worst case:

| Rail | Worst-case current |
|------|--------------------|
| 3.3 V (chip, OLED, sensors, **motor**) | ~185 mA |
| 5 V (the two LEDs at full white) | ~170 mA |
| **Total** | **~355 mA** |

USB supplies 500 mA, so you're fine, and **you don't need a separate motor supply**.

But note the last row: the motor's surge lands on the *same* 3.3 V rail that the light sensor measures.
Without a bulk capacitor you'll see the rail dip and the light reading twitch every time the motor
fires. That's why there's a **22 µF capacitor (C3)** in Part 5 — it's not decoration.

---

## Part 4 — What to buy, and where

### 4.1 The exact shopping list

| Ref | Part | Package | KiCad footprint to use | LCSC # | ~Price |
|-----|------|---------|------------------------|--------|--------|
| BZ1 | **TDK PS1240P02BT** passive piezo sounder, 12.2 mm | Through-hole | `Buzzer_Beeper:Buzzer_TDK_PS1240P02BT_D12.2mm_H6.5mm` | **C76871** | $0.21 |
| R7 | 100 Ω — buzzer series resistor *(optional)* | 0805 | `Resistor_SMD:R_0805_2012Metric` | — | $0.01 |
| R2 | 10 kΩ — light sensor divider | 0805 | `Resistor_SMD:R_0805_2012Metric` | **C17414** | $0.01 |
| R4 | **GL5528** LDR / photoresistor | Through-hole | `OptoDevice:R_LDR_5.1x4.3mm_P3.4mm_Vertical` | **C10081** | $0.05 |
| Q1 | **AO3400A** logic-level N-channel MOSFET | SOT-23 | `Package_TO_SOT_SMD:SOT-23` | **C20917** | $0.09 |
| R5 | 220 Ω — gate series resistor | 0805 | `Resistor_SMD:R_0805_2012Metric` | — | $0.01 |
| R6 | 100 kΩ — gate pull-down | 0805 | `Resistor_SMD:R_0805_2012Metric` | — | $0.01 |
| D3 | **SS14** Schottky flyback diode | SOD-123 | `Diode_SMD:D_SOD-123` | **C2909920** | $0.02 |
| C3 | 22 µF — motor bulk capacitor | 0805 | `Capacitor_SMD:C_0805_2012Metric` | — | $0.05 |
| C4 | 100 nF — motor bypass | 0805 | `Capacitor_SMD:C_0805_2012Metric` | — | $0.01 |
| J3 | 2-pin header for the motor | Through-hole | `Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical` | — | $0.05 |
| M1 | **Coin motor 1027** (10 × 2.7 mm), 3 V | Wired | plugs into J3 | — | $0.20–0.80 |

**Total: about $1.00–1.50 per board.**

**Notes:**

* **All those KiCad names are real.** They were checked against a KiCad 10.0 installation. If a name
  won't autocomplete in Part 5, you mistyped it.
* **Reference numbers `R2`, `R4`–`R7`, `C3`, `C4`, `D3`, `Q1`, `BZ1`, `J3` are all free** in your
  project. `D3` here is a *diode*, not your XIAO's D3 pin — context will make it clear.
* ⚠️ **The LDR footprint pitch — measure before you commit.** GL55-series LDRs have a 5.1 × 4.3 mm body.
  The footprint above has its holes **3.4 mm apart** (verified from the footprint file itself). If your
  LDR's legs are on 2.54 mm centres, use `OptoDevice:R_LDR_4.9x4.2mm_P2.54mm_Vertical` instead.
  💡 **In practice this is low-risk:** these are through-hole parts with ~31 mm flexible legs, so a
  ~1 mm mismatch can simply be bent to fit.
* **SS14 vs 1N4148W:** both work. SS14 is a Schottky with a lower voltage drop; 1N4148W is smaller and
  cheaper. Pick either.

### 4.2 Where to buy — four routes, costed

#### 🥇 Route A — LCSC, ordered with your PCB (cheapest)

**Why it wins:** JLCPCB (who make the board) and LCSC (who sell the parts) are the same company. Order
both together and you don't pay for shipping twice.

* Every part above is an LCSC stock item with the number given.
* LCSC has no minimum order, but **orders under $15 incur a handling fee** — so bundle this with the PCB
  order rather than buying $1.20 of parts on their own.
* Part minimums mean you'll buy 50 diodes and 100 resistors. That's fine — they cost pennies and you'll
  want spares.

**Realistic delivered cost:** ~$5–15 of parts, plus your PCB cost, plus one lot of shipping.

#### 🥈 Route B — AliExpress (cheapest per part, slowest)

| Part | Search for | Rough price |
|------|-----------|-------------|
| Piezo | `passive piezo buzzer 12mm 2pin` | $2–4 / 5–10 pcs |
| LDR | `GL5528 photoresistor` | $2–3 / 20 pcs |
| Motor | `1027 coin vibration motor 3V` | $2–4 / 5 pcs |
| MOSFET | `AO3400A SOT-23` | $2–3 / 20 pcs |

**Delivered:** ~$10–15 for several boards' worth. **2–4 weeks shipping.**

⚠️ Buzzer listings on AliExpress frequently mix up active and passive parts, and sometimes lie. Buying
the TDK part from LCSC removes all doubt.

#### 🥉 Route C — Amazon (fastest, most expensive)

| Part | Search for | Rough price |
|------|-----------|-------------|
| Piezo | `passive piezo buzzer 12mm 10 pack` | ~$8 |
| LDR | `GL5528 photoresistor 20 pack` | ~$7 |
| Motor | `1027 coin vibration motor 5 pack` | ~$6 |
| MOSFET | `AO3400A SOT-23 20 pack` | ~$6 |
| 0805 resistor + capacitor assortment kits | `0805 resistor kit`, `0805 capacitor kit` | ~$15 each |

**Delivered:** ~$30–50, arriving in a day or two, with leftovers for years.

#### Route D — Digi-Key / Mouser

Most expensive per part, but the best availability. Use only if you're missing one part and you're stuck.

#### 🛑 What NOT to buy

| Don't buy | Why not |
|-----------|---------|
| An **active** buzzer | It has a built-in oscillator and can only make one fixed beep. No melodies, no chiptune. |
| An **LRA** / "linear haptic actuator" | Needs AC drive from a dedicated driver chip. Costs several times more. You want a plain brushed **ERM coin motor**. |
| A **"sound sensor module"** | That's a *microphone* — the opposite of what you need. |
| A **"light sensor module"** with an LM393 | Unnecessary here: bigger, more expensive, and only gives on/off. See Option C in §14 if you specifically want that. |
| Adafruit / SparkFun breakouts for these parts | Good products, but a 1027 motor is **$1.95 each** there versus ~$0.40 on AliExpress/LCSC, and you don't need their breakout board. |
| Any MOSFET picked only by its threshold voltage | It must be **logic-level**. The AO3400A specifies its on-resistance at 2.5 V gate drive, so it works properly from 3.3 V. A 2N7002 works but has much higher resistance. |

---

## Part 5 — Drawing the schematic, step by step

Open `Starbiew.kicad_pro` and click **Schematic Editor**.

> ✅ Handy before you start: in KiCad, **A** = add a symbol, **W** = draw a wire, **L** = place a net
> label, **P** = place a power symbol, **R** = rotate, **Delete** = remove.
>
> 💡 **This project connects things with net labels instead of long wires.** That's why moving a signal
> later is a two-click job. Follow that convention.

### 5.0 Housekeeping — 2 minutes, do it first

Your schematic has two confusingly named nets. `GPIO02` and `GPIO03` are named after the *U1 pad number*,
but they're actually on **GPIO4** and **GPIO5**. Fix that now or it will confuse you later.

1. Double-click the label `GPIO02`. Type `BUTTON1`, press **Enter**.
2. Double-click the label `GPIO03`. Type `BUTTON2`, press **Enter**.
3. Save (**Ctrl+S**).

### 5.1 Feature 1 — the piezo buzzer

**The circuit you're building:**

```
                             R7 (100 Ω)
BUZZ  ──────────────────── /\/\/\/\ ────┬──── BZ1 pin 1  (+)
                                         │
                              BZ1 pin 2 (−) ──── GND
```

And the label `BUZZ` also goes on a short wire from **U1 pad 11 (D10)**.

**Steps:**

- [ ] **1.** Press **A**. Type `Buzzer` in the filter. Select **`Device:Buzzer`**. Click **OK**.
- [ ] **2.** Click in a free area of the canvas to place it. *(Position doesn't matter — the net labels
      do the connecting.)*
- [ ] **3.** Double-click the symbol. In the properties dialog:
      * **Reference:** `BZ1`
      * **Value:** `Passive Piezo 12mm`
      * **Footprint:** `Buzzer_Beeper:Buzzer_TDK_PS1240P02BT_D12.2mm_H6.5mm`
      * Click **OK**.
- [ ] **4.** Press **A**, pick **`Device:R`**, place it to the left of BZ1. Double-click it:
      * **Reference:** `R7`, **Value:** `100`, **Footprint:** `Resistor_SMD:R_0805_2012Metric`
- [ ] **5.** Press **W**. Wire R7's right-hand pin to **BZ1 pin 1** (the `+` pin). Press **Esc**.
- [ ] **6.** Press **P**, choose `GND`, place it below BZ1. Press **W** and wire **BZ1 pin 2** (the `−`
      pin) to that GND symbol.
- [ ] **7.** Press **L**. Type `BUZZ`, press **Enter**, and click on the free end of R7's left pin.
      ⚠️ The label must sit exactly **on the wire end**. A label floating in space causes an ERC error.
- [ ] **8.** Press **W** and draw a short wire from **U1 pad 11 (D10)**. Press **L**, type `BUZZ`, and
      click on that wire.

> 💡 **Skip the resistor?** You can wire BZ1 pin 1 straight to the `BUZZ` net if you like. R7 only limits
> current and tames the volume.

> 🔍 **Why this needs no drive circuit:** a piezo sounder is electrically a capacitor — it draws almost no
> DC current, so a chip pin can drive it directly. That's true *only* of piezo. A magnetic speaker would
> need an amplifier.

**Optional but recommended — add sourcing data.** Select BZ1 and add these custom fields (they export in
your BOM and are read by JLCPCB/LCSC tooling):

| Field | Value |
|-------|-------|
| `MPN` | `PS1240P02BT` |
| `Manufacturer` | `TDK` |
| `LCSC` | `C76871` |

✅ **Check:** you should have a `BUZZ` label appearing twice — once at R7, once at U1 pad 11.

### 5.2 Feature 2 — the light sensor, with the DHT11 swap

#### Step 1 — move the DHT11 data line

This is the fix from §3.3. It's a single label move.

- [ ] **1.** Find the `DHT11 DATA` label attached to the wire coming from **U1 pad 2 (D1)**.
      In your schematic it's at roughly (102.87, 95.25) mm.
- [ ] **2.** Click the label to select it, click the wire stub it's on, and press **Delete** to remove both.
- [ ] **3.** Press **W** and draw a short wire from **U1 pad 1 (D0)**.
- [ ] **4.** Press **L**, type `DHT11 DATA`, press **Enter**, and click on that new wire.

✅ **Check before continuing:** the text `DHT11 DATA` should now appear in exactly three places — at U1
pad 1, at U2 pin 2, and at R1's lower pin. Run `Tools → Electrical Rules Checker` and confirm no new errors.

> 🛑 **Do not** move the label near U2. Only the MCU end changes; the net name does the rest.

#### Step 2 — build the divider

**The circuit:**

```
+3.3V ──── R2 (10 kΩ) ────┬──── LIGHT ────►  U1 pad 2  (D1 / GPIO3 / ADC1_CH3)
                          │
                     R4 (GL5528 LDR)
                          │
                         GND
```

- [ ] **1.** Press **A**, pick **`Device:R`**, place it. Double-click it:
      **Reference** `R2`, **Value** `10k`, **Footprint** `Resistor_SMD:R_0805_2012Metric`.
- [ ] **2.** Press **A**, pick **`Device:R_Photo`**, place it below R2. Double-click it:
      **Reference** `R4`, **Value** `GL5528`,
      **Footprint** `OptoDevice:R_LDR_5.1x4.3mm_P3.4mm_Vertical`
      *(or the 2.54 mm variant — see §4.1).*
- [ ] **3.** Press **P**, place a `+3.3V` symbol above R2, and wire it to R2's top pin.
- [ ] **4.** Press **W**, wire R2's bottom pin to R4's top pin.
- [ ] **5.** Press **P**, place `GND` below R4, and wire it to R4's bottom pin.
- [ ] **6.** Press **L**, type `LIGHT`, and click on the wire *between* R2 and R4.
- [ ] **7.** Press **W** to draw a short wire from **U1 pad 2 (D1)**. Press **L**, type `LIGHT`, and click
      on it.

#### Step 3 — understand the polarity now, thank yourself later

| Condition | LDR resistance | Voltage at `LIGHT` | Reading in code |
|-----------|----------------|--------------------|-----------------|
| Bright sunlight | ~1 kΩ | ~0.3 V | low |
| Lit room | ~5–10 kΩ | ~0.5–0.9 V | low-ish |
| Dim room | ~100 kΩ | ~2.2 V | high |
| Total darkness | ~1 MΩ | ~3.3 V | high |

> ✅ **Remember this one line: high millivolts means dark.**
> It's the single most common cause of "my day/night mode is inverted".

⚠️ **Don't panic when the dark reading stops climbing.** The ESP32-C3's ADC defaults to a range of about
**0–2500 mV**. In total darkness your divider outputs ~3.3 V, which is *above* that range, so the reading
flat-lines near the top instead of going higher. **That is expected, not a fault** — and it doesn't matter
for day/night detection, because you only need to know "darker than my threshold". See §11.5 if you want
finer resolution.

#### Step 4 — decide where the LDR physically goes

This is §3.6's Problem 1. Best to worst:

1. **Best (if you add `J4`):** on a two-wire flying lead, with only R2 on the PCB. *(Add a 2-pin header —
   `Connector:Conn_01x02_Pin`, ref `J4` — next to R2, and leave ~100 mm of flexible wire.)* There's no case
   to route it through, but a remote sensor sidesteps the LED problem **by distance**: you can stand it
   wherever the light actually is and point it wherever you like. Still heat-shrink it (see §10.2).
2. **Good (if you skip `J4`):** the LDR soldered through-hole at the board edge, sensing face pointing
   outward, as far from D1/D2 as the star shape allows — then follow all three defences in §10.2.
3. **Acceptable:** any through-hole position, **plus** the heat-shrink and the firmware LED-blanking
   that's already built in.

✅ **Check:** you now have a `LIGHT` label appearing twice — once between R2/R4, once at U1 pad 2.

### 5.3 Feature 3 — the vibration motor

🛑 **Never connect the motor directly to a chip pin.** It needs 60–80 mA running and far more when
stalled, and it's inductive. It needs a transistor to switch it and a diode to protect against the
voltage spike. This section builds both.

**The circuit in plain English:**

* The motor's **+** goes to 3.3 V.
* The motor's **−** goes to the transistor, which switches it to ground.
* The transistor's **gate** is driven by D7 through a 220 Ω resistor, with a 100 kΩ resistor holding it
  off when nothing is driving it.
* A diode across the motor gives the inductive kick somewhere harmless to go.
* Two capacitors at the connector absorb the motor's current surges.

```
                             +3.3V ──┬────── J3 pin 1  (motor +)
                                     │
                          C3 22µF ───┤           ┌──────────────┐
                          C4 100nF ──┤           │  M1  coin    │
                                     │           │     motor    │
                                     │           └──────────────┘
                                     │                  │
   U1 pad 8 (D7) ── R5 220Ω ──┬─► Q1.G              J3 pin 2  (motor −)
                              │                        │
                        R6 100kΩ                       │
                              │                        │
                             GND          D3 (SS14) ───┤
                                          cathode → +3.3V
                                          anode   → J3 pin 2
                                                       │
                              Q1.S ──── GND      Q1.D ─┘
```

**Step 1 — the motor connector**

- [ ] **1.** Press **A**, pick **`Connector:Conn_01x02_Pin`**, place it. Double-click it:
      **Reference** `J3`, **Value** `Motor`,
      **Footprint** `Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical`.
- [ ] **2.** Press **P**, place `+3.3V`, and wire it to **J3 pin 1**.
- [ ] **3.** Leave **J3 pin 2** free for now.

**Step 2 — the transistor**

- [ ] **1.** Press **A**, type `AO3400A`, select **`Transistor_FET:AO3400A`**, place it.
- [ ] **2.** Double-click it: **Reference** `Q1`, **Value** `AO3400A`,
      **Footprint** `Package_TO_SOT_SMD:SOT-23`.
- [ ] **3.** Its pins are **1 = G** (gate), **2 = S** (source), **3 = D** (drain).
- [ ] **4.** Press **W** and wire:
      * Q1 **pin 3 (D)** → **J3 pin 2**
      * Q1 **pin 2 (S)** → a `GND` symbol

**Step 3 — the gate network**

- [ ] **1.** Press **A**, pick `Device:R`, place it. **Reference** `R5`, **Value** `220`,
      **Footprint** `Resistor_SMD:R_0805_2012Metric`. Wire R5's right pin → Q1 **pin 1 (G)**.
- [ ] **2.** Press **A**, pick `Device:R`, place it. **Reference** `R6`, **Value** `100k`, same footprint.
      Wire one end to Q1 pin 1 (the same point as R5) and the other end to a `GND` symbol.
      * 🔍 This is the **pull-down**. It guarantees the motor stays off while the chip is resetting or the
        pin is floating. Without it, the motor can twitch on power-up.
- [ ] **3.** Press **L**, type `HAPTX`, and click on the free end of R5's left pin.
- [ ] **4.** Press **W** to draw a short wire from **U1 pad 8 (D7)**. Press **L**, type `HAPTX`, click on it.

**Step 4 — the flyback diode**

- [ ] **1.** Press **A**, type `SS14`, select **`Diode:SS14`**, place it near J3.
- [ ] **2.** Double-click it: **Reference** `D3`, **Footprint** `Diode_SMD:D_SOD-123`.
- [ ] **3.** Wire it **across the motor**, watching the orientation:
      * **Cathode** (the bar end, pin `K`) → the **+3.3V / J3 pin 1** side
      * **Anode** (the triangle end, pin `A`) → **J3 pin 2** (the motor's − side)

🛑 **A reversed diode is a dead short across the power supply the moment the motor runs.** Double-check
it here, and check the physical marking when you solder (the banded end is the cathode).

> 🔍 **Why the diode matters:** when the transistor switches off, the motor's inductance tries to keep
> current flowing and generates a large negative voltage spike. The diode gives that current a harmless
> path. Without it, you'll eventually destroy the transistor.

**Step 5 — the capacitors**

- [ ] **1.** Press **A**, pick `Device:C`. **Reference** `C3`, **Value** `22uF`,
      **Footprint** `Capacitor_SMD:C_0805_2012Metric`. Wire from the +3.3V/J3 pin 1 node to `GND`.
- [ ] **2.** Press **A**, pick `Device:C`. **Reference** `C4`, **Value** `100nF`, same footprint. Wire it
      in parallel with C3, right at the connector.

**✅ Step 6 — read the whole block back before moving on**

- [ ] Exactly **one** wire connects a chip pin to the motor circuit, and it goes through **R5** into the
      transistor's **gate**. No chip pin touches the motor directly.
- [ ] Q1 **source** → GND. Q1 **drain** → motor **−**. Motor **+** → 3.3 V.
- [ ] D3 **cathode** → 3.3 V, **anode** → motor **−**.
- [ ] `HAPTX` appears twice: at R5 and at U1 pad 8.

If any of those is wrong, fix it now. This is the block most likely to release magic smoke.

### 5.4 Save

Press **Ctrl+S**. Now go to Part 6.

---

## Part 6 — Checking the schematic (ERC)

Click `Inspect → Electrical Rules Checker`, then **Run ERC**.

### 6.1 Errors you must fix

| Error message | What it means | How to fix |
|---------------|---------------|------------|
| **Pin not connected** | A wire isn't actually touching the pin | Zoom right in. The wire must end exactly on the pin's endpoint. Redraw it. |
| **Label not connected** / dangling label | You placed a net label in empty space | Delete it and place it again, on the wire. |
| **Multiple net names on one wire** | The same wire has two different labels | Delete one. |
| **Input power pin not driven** on `+3.3V` | Nothing declares itself as the power source for that rail | Your project already has three `PWR_FLAG` symbols. Make sure one is on `+3.3V`. If not, press **P**, place `power:PWR_FLAG`, attach it to `+3.3V`. |

### 6.2 Warnings you can safely ignore

| Warning | Why it's expected |
|---------|-------------------|
| **Input pin not driven by any output** on `Q1` pin 1 (gate) | The AO3400A's gate is defined as an *input* pin, and it's driven through R5/R6, whose pins are *passive*. ERC doesn't trace through passives, so it flags this. It's a false positive — a MOSFET gate driven through a series resistor is completely standard. Right-click it and choose to ignore that specific violation. |
| **Symbol doesn't match copy in library** | Cosmetic. The cached symbol differs slightly from the library. Harmless. |
| Anything about the `G***` logo footprints | Those are the decorative flower patterns. |

### 6.3 ✅ Target

**0 errors.** A couple of the warnings above are normal.

🛑 Don't continue with an error outstanding. ERC errors become unconnected pads on the PCB, which become
dead features you only discover *after* you've paid for and assembled the board.

---

## Part 7 — Laying out the PCB

### 7.1 Bring the changes into the board

- [ ] **1.** Save the schematic.
- [ ] **2.** Go back to the project window, click **PCB Editor**.
- [ ] **3.** Press **F8** (`Tools → Update PCB from Schematic`).
- [ ] **4.** In the dialog: **untick** "Delete footprints with no symbols", **tick** "Update footprints".
      Click **Update PCB**.

✅ **Check:** you should now see 11 new footprints sitting outside the board outline, with thin blue
"airwires" showing what needs to connect to what.

### 7.2 Where to put things on a star-shaped board

Your board is a star with stepped edges, mostly filled by a ground pour, with two large decorative
flowers. There's no big empty rectangle, so use the **free star points and edge areas**.

| Cluster | Parts | Where to put it | Why |
|---------|-------|-----------------|-----|
| **Motor driver** | Q1, R5, R6, D3, C3, C4, J3 | A star point at the board edge, as far from U1 as you can | Keeps switching noise, *and the motor's vibration*, as far from the chip and the accelerometer as possible |
| **Buzzer** | BZ1, R7 | Any spot with a clear 12.2 mm circle — check that a narrow star tip is actually wide enough | A bare disc radiates from both faces, so don't bury it under tall parts |
| **Light sensor** | R4, R2 | Opposite side of the board from **D1/D2**, ideally at an edge | Keeps your own LEDs out of the sensor's view |

💡 **Practical tip:** the further the motor is from U1, the better. On this board that means a bottom or
side star point, not anything next to the XIAO.

### 7.3 The rules that actually matter

**Keep the motor's current loop short.** The high-current path is
`+3.3V → J3 pin 1 → motor → J3 pin 2 → Q1 drain → Q1 source → GND`. Short means less noise; wide means
less voltage drop. Use a **0.5–1.0 mm** track for this path. Everything else can be 0.25–0.3 mm.

**Put the bulk capacitors at the connector.** C3 and C4 must sit at **J3 pin 1**, not near the XIAO. A
bulk capacitor 40 mm away from the load does essentially nothing.

**Keep the `LIGHT` track away from noise.** Route it clear of the `HAPTX` gate track, the LED data track,
and the motor loop. If you can, run a ground track or pour alongside it.

**Ground the motor properly.** Return the motor's ground to the ground pour near U1's GND pins rather
than through a thin track shared with the sensors or LEDs. With a full-board pour this is nearly free —
just don't build a long thin shared return.

### 7.4 Track widths

| Net | Width | Why |
|-----|-------|-----|
| Motor current path | 0.5–1.0 mm | Up to 120 mA |
| `+3.3V` and `GND` generally | 0.5 mm | Shared rails |
| `LIGHT`, `DHT11 DATA`, `BUZZ`, `HAPTX` | 0.25–0.3 mm | Signals only |
| WS2812B data | 0.25 mm | Already routed |

### 7.5 Placing and routing

- [ ] **1.** Drag each new footprint into position. Press **R** while dragging to rotate.
- [ ] **2.** Keep each reference designator (like `Q1`) readable and not overlapping other silkscreen text.
- [ ] **3.** Watch the airwires — when they stop crossing each other, the placement is reasonable.
- [ ] **4.** Route in this order: **power first**, then the motor loop, then the signals, and **`LIGHT`
      last** so you can keep it away from everything.
- [ ] **5.** Press **X** to start a track. Click a pad, follow the airwire, click at each corner. Press
      **Esc** to stop. Press **V** to drop a via and change layer mid-route.

💡 **Placement order that works well:** the motor cluster first (it's the part that most wants distance
from U1), then J3 and its support parts, then the sensor cluster.

### 7.6 Re-fill the copper and check

- [ ] **1.** Press **B** to re-fill all copper zones. **Do this after every routing change** — stale fills
      are a classic source of confusing DRC results.
- [ ] **2.** `Inspect → Design Rules Checker → Run DRC`.
- [ ] **3.** Target: **0 errors**.
- [ ] **4.** Zoom in on the star points. A thin board tip plus a pour can leave copper slivers or isolated
      islands. Delete anything that looks like a floating fragment.

### 7.7 Finishing touches

- [ ] Make sure every new reference designator is readable.
- [ ] Add silkscreen labels for the new connectors (`MOTOR`, `LIGHT`, `BUZZ`) using `Place → Text` on the
      **F.Silkscreen** layer. Future-you will thank present-you.
- [ ] Mark the motor connector polarity (`+` / `−`) on silkscreen.
- [ ] Add a silkscreen circle around the buzzer disc (12.2 mm) so its keep-out area is visible, and a
      small dot or arrow marking which way the LDR should face.

### 7.8 ✅ Final review before ordering

- [ ] **1.** `View → 3D Viewer`. Look at the whole board.
- [ ] **2.** `View → Board Statistics`. Confirm the layer count is still 2.
- [ ] **3.** Run DRC one last time. 0 errors.
- [ ] **4.** Save.

---

## Part 8 — Ordering the PCB

### 8.1 Export the Gerbers

In the **PCB Editor**:

- [ ] **1.** `File → Fabrication Outputs → Gerbers (.gbr)`. Point the output at a tidy folder like `gerbers/`.
- [ ] **2.** Select exactly these seven layers:

      `F.Cu` · `B.Cu` · `F.Mask` · `B.Mask` · `F.Silkscreen` · `B.Silkscreen` · **`Edge.Cuts`**

      🛑 **`Edge.Cuts` is the board outline and is essential.** It must form a closed shape.
      ⚠️ Do **not** select `F.Paste` or `B.Paste` unless you're also ordering a solder stencil.

- [ ] **3.** Set these options:

      | Setting | Value |
      |---------|-------|
      | Format | Gerber / RS-274X |
      | Plot mode | **Filled** |
      | Regenerate zone fills before plotting | **On** |
      | Use Protel filename extensions | **On** |
      | Gerber coordinates | Millimeters, 4.6 format |
      | Use extended X2 format | On |
      | Subtract soldermask from silkscreen | On |
      | Drill marks | None |

- [ ] **4.** Click **Plot**. Then click **Generate Drill Files** in the same dialog:

      | Setting | Value |
      |---------|-------|
      | Format | **Excellon** |
      | Units | Millimeters |
      | Origin | Absolute |
      | Zeros | Decimal format |
      | Merge PTH and NPTH into one file | **Off** (keep them separate) |
      | Alternate drill mode (oval holes) | On, if your design has slots |

- [ ] **5.** Zip all the `.gbr` and `.drl` files together into one archive. That single file is what you upload.

### 8.2 ✅ Check the Gerbers before you pay

This step is the difference between a working board and a $15 mistake.

- [ ] Open your zip in KiCad's **Gerber Viewer** (or <https://gerber-viewer.easyeda.com/>).
- [ ] The board outline is **closed** — no gap in `Edge.Cuts`.
- [ ] All seven layers are present.
- [ ] The drill holes line up with the pads.
- [ ] The new pads for BZ1, R4, J3 and Q1 are there and the right size.
- [ ] Silkscreen text is **not** printed over any pads.

### 8.3 Placing the JLCPCB order

- [ ] **1.** Go to JLCPCB, sign in, upload your zip.
- [ ] **2.** Let it detect the dimensions. **Verify they match your board.**

      ⚠️ **Your board is 104.05 × 92.55 mm — just over the 100 mm boundary** in one direction. That puts
      you in the next price tier instead of the very cheapest one. This is normal and only costs a few
      dollars. **Don't "fix" it by trimming the star.**

- [ ] **3.** Choose these options:

      | Option | Choose |
      |--------|--------|
      | Layers | 2 |
      | Material | FR-4 |
      | Thickness | 1.6 mm |
      | Copper weight | 1 oz |
      | Surface finish | HASL (or lead-free HASL) |
      | Solder mask | Green (colour barely affects price) |
      | Silkscreen | White |
      | Quantity | 5 (minimum, and cheapest per board) |
      | Gold fingers, castellation, impedance control | **No** |

- [ ] **4.** Open JLCPCB's built-in **Gerber Viewer** and click through every layer. Verify the star
      outline, the new pads, and the silkscreen. This catches export mistakes before production.
- [ ] **5.** Choose shipping, then check out.

**Expect:** a few dollars for the boards, ~24 h production for a standard 2-layer board, then shipping.
DHL is typically 3–5 days in transit; the cheap option is much slower. **Plan on a week or more from
payment to delivery.**

💡 JLCPCB may email you a DFM (design-for-manufacturing) report. **Read it** — it often catches footprint
mistakes.

---

## Part 9 — Soldering it together

### 9.1 What you're soldering

| Ref | Package | Difficulty |
|-----|---------|------------|
| BZ1 | Through-hole, 12.2 mm disc | Easy |
| R4 | Through-hole LDR | Easy |
| J3 | Through-hole 2-pin header | Easy |
| R2, R5, R6, R7, C3, C4 | 0805 SMD | Moderate |
| D3 | SOD-123 SMD | Moderate |
| Q1 | SOT-23 SMD | The fiddliest thing here |

💡 **Order of operations: smallest and flattest first.** Solder the SMD parts before any through-hole part
is standing up in your way.

### 9.2 How to solder the small parts (SMD)

The technique for 0805, SOD-123 and SOT-23:

1. **Tin one pad.** Put a small blob of solder on **one** pad only.
2. **Place the part** with tweezers, holding it in position, and re-flow that tinned pad. Use a fine tip
   at roughly **320–350 °C**.
3. **Solder the other pad(s)** normally.
4. **Inspect.** Zoom in with your phone camera. Look for solder bridges between adjacent pads.

Specific notes:

* **0805 resistors and capacitors** are 2.0 × 1.25 mm. Very approachable. Use flux — it's the difference
  between easy and infuriating.
* **SOD-123 (D3):** the **banded end is the cathode** and must go to the **+3.3V** side (§5.3).
* **SOT-23 (Q1):** three pads on 0.95 mm centres. Flood with flux, use a small amount of solder, and drag
  the iron *away* from the part rather than toward it. If you bridge two pads, add flux and wick it clean.

🛑 **Check Q1 for bridges with a multimeter before powering up.** A source-to-drain bridge means the motor
runs continuously the moment you plug in.

### 9.3 How to solder the through-hole parts

1. Insert the part from the top.
2. Bend the legs slightly apart underneath so it stays put.
3. Solder each pad: heat the **pad and the leg together** for about 2 seconds, feed in solder, remove the
   solder, then remove the iron.
4. Aim for a shiny cone. A dull ball means too little heat or too little flux.
5. Trim the legs flush with side cutters.

### 9.4 🛑 Pre-power checks — do not skip

Before you plug in USB for the first time:

- [ ] **1.** **Unplug the motor** from J3.
- [ ] **2.** Set your multimeter to **continuity** (the beep mode).
- [ ] **3.** Check there is **no short** between:

      | Test | Expected |
      |------|----------|
      | `+3.3V` and `GND` | Open (may beep briefly as capacitors charge, then open) |
      | Q1 **drain** and Q1 **source** | Open |
      | `+5V` and `GND` | Open |
      | Q1 **gate** and Q1 **source** | ~100 kΩ (that's R6) |

- [ ] **4.** Check there **is** continuity where it should be:

      | Test | Expected |
      |------|----------|
      | J3 pin 1 → `+3.3V` | Connected |
      | J3 pin 2 → Q1 drain | Connected |
      | Q1 source → `GND` | Connected |
      | BZ1 pin 2 → `GND` | Connected |
      | R4's ground pin → `GND` | Connected |

- [ ] **5.** Check **D3's orientation** in diode mode: anode-to-cathode should read ~0.2–0.4 V, and open
      the other way.
- [ ] **6.** Only now plug in USB. **If anything gets hot, unplug immediately.**

---

## Part 10 — Building without an enclosure

There is no case in this build: **the PCB is the finished object.** That's a perfectly good way to do it
— the original Starbie is a bare board too — but of the three things a case normally handles, one gets
*easier* and two get *harder*.

### 10.1 The buzzer needs no help — it's better bare

A piezo disc radiates sound from **both** faces, so a case is usually what *muffles* it. Standing on its
own leads a millimetre or two above the PCB, the PS1240P02BT will be at its loudest. Expect it to be
clearly audible across a room.

Two small placement notes, both handled in Part 7:

* **Give the disc its own clear circle.** It's 12.2 mm across with two leads 5.0 mm apart. On a stepped
  star outline, **check that your chosen spot is actually wide enough** — a narrow star tip is not.
  Use the footprint's silkscreen outline in the PCB editor as your guide.
* **Don't crowd it.** Mark the disc's outline on silkscreen so you don't later slide a tall component into
  its space. The disc *is* the diaphragm; anything resting against it kills the volume.

There is no acoustic path to design and no hole to drill.

### 10.2 The light sensor needs more help bare — this one matters

With no case, the only thing between your two WS2812B LEDs and the LDR is air. **This is the biggest
difference between a cased and a bare build**, and it makes Problem 1 from §3.6 worse, not better.

Three layers of defence, cheapest first:

1. **Position (free).** Put the LDR as far from D1/D2 as the star shape allows — the opposite star point
   if you can.
2. **Orientation (free).** The chosen footprint is a *vertical* one, so the sensing face looks sideways
   off the board. Rotate the part until that face points **away** from the LEDs.
3. **Shield (a few pence).** Slide a short piece of **black heat-shrink tube** over the LDR body so only
   its top face is exposed. That turns an omnidirectional sensor into a directional one and blocks the
   LEDs' glow almost completely. **This is the trick that makes a bare-board build work properly**, and
   it costs nothing.

Plus the firmware defence that's already built in: `updateLight()` blanks the LEDs for 2 ms while it
samples (§11.3), so even light reflected off the board surface can't fool it.

⚠️ **If you only do one thing here, do the heat-shrink.** With the LEDs a couple of centimetres away and
nothing to block them, position and orientation alone are often not enough.

💡 The LDR responds in roughly 50 ms. Don't design anything needing fast light changes.

### 10.3 The motor needs more help too — the firmware already covers it

A case normally does two jobs for a vibration motor: it **amplifies** the buzz (the whole shell becomes
the haptic surface) and it **damps** the vibration before it reaches the accelerometer. Bare, you get
neither.

* **Weaker feel.** The PCB — plus whatever it's resting on — is your vibrating mass. That's fine; it's
  clearly perceptible in your hand or on a desk. Just don't expect phone-grade haptics.
* **Much stronger coupling into the accelerometer.** This is the part that causes bugs, and it's why the
  firmware's `hapticBusyUntil` guard (500 ms) is doing more work in a bare build than it would in a
  cased one.
* **Still decouple it if you can.** Stick the motor to the PCB with a small square of **foam tape**
  rather than flat against bare laminate, and keep its leads short but not taut. Foam has give, so the
  motor does less work shaking the sensor.
* **Don't solder it down.** You *can* solder the motor's leads straight to two pads and skip J3. Don't:
  a rigid joint transfers the most vibration into the board and makes the motor impossible to replace
  when the brushes wear out. The 2-pin header plus ~20 mm of wire is better in every way.

**If you still get phantom shakes** (the pet reacting when nothing shook it), two dials, in this order:

1. **Raise the guard:** change `now + 500` to `now + 800` in `hapticUpdate()`.
2. **Only if that fails, raise the threshold:** in BEGINNER SETTINGS, change
   `const float SHAKE_THRESHOLD = 7.0f;` to `9.0f`. This makes *real* shakes harder to trigger too, so
   it's a last resort.

### 10.4 Handling a bare board

Without a case, you're handling the electronics directly.

* 🛑 **The bottom of the board has a ground pour and a 5 V net.** Don't rest it on anything conductive
  while it's powered. The ground pour is harmless, but if a `+5V` or `+3.3V` pad lands on something
  conductive you'll short the supply.
* 💡 **Add four self-adhesive rubber feet** (the little hemispherical bumpers — pennies for a hundred) to
  the bottom corners. They stop it sliding, protect the solder joints, keep the board off conductive
  surfaces, and add clearance for the through-hole leg ends.
* **Static.** Bare boards are more exposed to ESD. Touch something earthed before picking it up, and
  don't slide it across synthetic carpet.
* **Don't flex the board.** It's a star with thin points, and the buzzer and LDR are through-hole parts
  whose leads take the strain. Support the board when pressing the buttons.
* **Picking it up counts as a shake.** Handling the board registers on the accelerometer, exactly as it
  does on the original Starbie. That's part of the charm.

### 10.5 ✅ Bare-board checklist

- [ ] The buzzer disc has a clear 12.2 mm circle, with nothing resting against it.
- [ ] The LDR is on the opposite side of the board from D1/D2, with its sensing face rotated away from them.
- [ ] The LDR has black heat-shrink over it, exposing only its top face.
- [ ] The motor is at the far end from U1, stuck down with foam tape, not rigidly soldered.
- [ ] Rubber feet or standoffs on the bottom.
- [ ] Nothing conductive touches the board while it is powered.

---

## Part 11 — Firmware

> ✅ **Everything in this section has been compiled.** The exact code below builds cleanly for
> `esp32:esp32:XIAO_ESP32C3` with arduino-cli 1.5.1 and ESP32 core 3.3.12: **0 errors**,
> 393,624 bytes of flash (30%) and 15,952 bytes of RAM (4%).

### 11.1 One-time Arduino IDE setup

Skip anything you've already done.

- [ ] **1.** Install the Arduino IDE (2.x).
- [ ] **2.** `File → Preferences`. In **Additional Boards Manager URLs**, add:
      ```
      https://espressif.github.io/arduino-esp32/package_esp32_index.json
      ```
- [ ] **3.** `Tools → Board → Boards Manager`, search `esp32`, install **esp32 by Espressif Systems**
      (version **3.x**).
- [ ] **4.** `Tools → Board → esp32 → XIAO_ESP32C3`.
- [ ] **5.** `Sketch → Include Library → Manage Libraries`. Install these, one at a time:

      | Library | Used for | Version tested |
      |---------|----------|----------------|
      | Adafruit GFX Library | OLED graphics | 1.12.6 |
      | Adafruit SSD1306 | OLED driver | 2.5.17 |
      | Adafruit MPU6050 | Accelerometer | 2.2.9 |
      | DHT sensor library | DHT11 | 1.4.7 |
      | **Adafruit NeoPixel** | **Your WS2812B LEDs** | 1.15.5 |

      When Arduino offers to install dependencies, choose **Install All**.

> ⚠️ **The stock `Starbie.ino` has no WS2812B code at all.** The upstream sketch predates your LED
> addition. §11.3 adds it. If your LEDs have never worked, this is why.

### 11.2 Step 1 — edit the BEGINNER SETTINGS block

Open `Starbie.ino`. Find the section starting with
`// =========================== BEGINNER SETTINGS ===========================`.

**Change 1 — add one line to the includes** at the very top of the file, just after `#include <math.h>`:

```cpp
#include <Adafruit_NeoPixel.h>
```

**Change 2 — replace the pin block.** Find these five lines:

```cpp
const int I2C_SDA_PIN = 7;       // XIAO D4: OLED + MPU6050 SDA
const int I2C_SCL_PIN = 6;       // XIAO D5: OLED + MPU6050 SCL
const int DHT_PIN = 3;           // XIAO D1: DHT11 data
const int BUTTON_ONE_PIN = 4;    // XIAO D2: opens/confirms the radial menu
const int BUTTON_TWO_PIN = 5;    // XIAO D3: shows/hides stats
```

Replace them with:

```cpp
// The Week 1 Guide.md quotes SDA=6 / SCL=7. That is WRONG for this board; the
// comments inside the shipped .ino are swapped too. Trust these numbers - they
// match Starbiew.kicad_sch, where SCL is on U1 pad 5 (D4/GPIO6) and SDA is on
// U1 pad 6 (D5/GPIO7).
const int I2C_SDA_PIN = 7;       // GPIO7 = XIAO D5
const int I2C_SCL_PIN = 6;       // GPIO6 = XIAO D4
const int DHT_PIN = 2;           // CHANGED: GPIO2 = D0 (was 3. See the guide Part 3)
const int BUTTON_ONE_PIN = 4;    // XIAO D2: opens/confirms the radial menu
const int BUTTON_TWO_PIN = 5;    // XIAO D3: shows/hides stats

// --- Starbiew expansion ---------------------------------------------------
const int BUZZER_PIN = 10;  // XIAO D10
const int HAPTIC_PIN = 20;  // XIAO D7
const int LIGHT_PIN  = 3;   // XIAO D1 (ADC1_CH3)
const int LED_PIN    = 21;  // XIAO D6: WS2812B data via R3
```

**Change 3 — turn the features on.** Find `const bool USE_DHT11 = true;` and add these lines right after it:

```cpp
const bool USE_SOUND        = true;  // passive piezo buzzer
const bool USE_LIGHT_SENSOR = true;  // LDR day/night
const bool USE_HAPTIC       = true;  // vibration motor
const bool USE_LEDS         = true;  // WS2812B
```

💡 **These are your on/off switches.** Set any of them to `false` and that feature is disabled — useful
while debugging. *(Verified: the sketch compiles with all four set to `false`.)*

### 11.3 Step 2 — paste the expansion block

⚠️ **Where you paste this matters. This is the one step people get wrong.**

KiCad isn't involved here — this is about how Arduino processes `.ino` files. Arduino automatically
generates a "prototype" (a forward declaration) for every function in your sketch and inserts them all
**immediately above the first function in the file**. If this block sits too high up, it becomes that
first function, which drags the insertion point above Starbie's own `struct PetState` and
`struct ButtonState`. The generated prototypes then reference a type the compiler hasn't seen, and the
build fails with `'ButtonState' was not declared in this scope`.

✅ **So: paste the block immediately BEFORE this exact line:**

```cpp
int keepInRange(int value, int smallest, int largest) {
```

That position is **after** Starbie's struct and global declarations and **before** its functions. Get that
right and everything below compiles — this exact block was compiled for the XIAO ESP32-C3 as part of
writing this guide.

**Paste this whole block:**

```cpp
// ===========================================================================
// EXPANSION: sound, light sensing, haptics, LEDs
// ===========================================================================
// Everything here is non-blocking. Starbie's loop runs at ~16 ms and every
// animation is driven from millis(), so a delay() in a sound or haptic routine
// would visibly stutter the pet. Instead, each feature is advanced by an
// update function, called once per loop().
//
// WHY PLACEMENT MATTERS (read this once, it will save you an hour):
// Arduino generates a prototype for every function in a .ino file and inserts
// them all immediately above the FIRST function definition it finds. If this
// block sits higher up in the file, it becomes the first function definition,
// which drags that insertion point above Starbie's own `struct PetState` and
// `struct ButtonState`. The generated prototypes for setUpButton() and
// wasPressed() then reference a type the compiler has not seen yet, and the
// build fails with "ButtonState was not declared in this scope".
//
// Two rules keep it working:
//   1. Paste this block immediately BEFORE the line
//          int keepInRange(int value, int smallest, int largest) {
//      which is after Starbie's structs and globals and before its functions.
//   2. Inside this block, every struct is declared before the first function.
//
// The functions below also avoid custom types in their signatures (they take a
// plain uint8_t ID instead of a Note* or Pulse*), so the generated prototypes
// resolve no matter how the file is later rearranged.

// ---------------------------------------------------------------------------
// Types - all type definitions come first in this block
// ---------------------------------------------------------------------------
struct Note {
  uint16_t freq;  // Hz; 0 means a rest
  uint16_t ms;    // duration
};

struct Pulse {
  uint8_t  percent; // motor strength, 0-100
  uint16_t onMs;
  uint16_t offMs;   // gap before the next pulse
};

// ---------------------------------------------------------------------------
// Note names and lookup-table IDs
// ---------------------------------------------------------------------------
// Anonymous enums give us plain integer constants. A *named* enum would be a
// custom type and would have to be declared before the generated prototypes.
enum {
  N_REST = 0,
  N_C4 = 262, N_D4 = 294, N_E4 = 330, N_F4 = 349,
  N_G4 = 392, N_A4 = 440, N_B4 = 494,
  N_C5 = 523, N_D5 = 587, N_E5 = 659, N_F5 = 698,
  N_G5 = 784, N_A5 = 880, N_C6 = 1047
};

enum {
  TUNE_BLIP = 0, TUNE_HAPPY, TUNE_SAD, TUNE_FEED,
  TUNE_SLEEP, TUNE_ALERT, TUNE_NIGHT, TUNE_COUNT
};

enum {
  HAP_TICK = 0, HAP_SELECT, HAP_FEED,
  HAP_PET, HAP_ALERT, HAP_NIGHT, HAP_COUNT
};

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
// The LED strip lives here, not with Starbie's globals, so that everything
// this block uses is declared in one place, above its first use.
Adafruit_NeoPixel pixels(2, LED_PIN, NEO_GRB + NEO_KHZ800);

// Tunes. Rows are padded with {0,0} rests, so TUNE_LENGTHS says how much of
// each row is real. TUNE_MAX_NOTES just has to be >= the longest tune.
const uint8_t TUNE_MAX_NOTES = 6;
const Note TUNES[TUNE_COUNT][TUNE_MAX_NOTES] = {
  /* BLIP  */ {{N_C6, 30}},
  /* HAPPY */ {{N_C5, 100}, {N_E5, 100}, {N_G5, 100}, {N_C6, 170}},
  /* SAD   */ {{N_G4, 150}, {N_E4, 150}, {N_C4, 260}},
  /* FEED  */ {{N_E5, 90},  {N_G5, 160}},
  /* SLEEP */ {{N_G4, 180}, {N_C4, 320}},
  /* ALERT */ {{N_A5, 60},  {N_REST, 40}, {N_A5, 60}},
  /* NIGHT */ {{N_D4, 200}, {N_C4, 300}},
};
const uint8_t TUNE_LENGTHS[TUNE_COUNT] = {1, 4, 3, 2, 2, 3, 2};

// Haptic pulse patterns, same idea.
const uint8_t PATTERN_MAX_PULSES = 3;
const Pulse PATTERNS[HAP_COUNT][PATTERN_MAX_PULSES] = {
  /* TICK   */ {{70, 25, 0}},
  /* SELECT */ {{100, 45, 55}, {100, 45, 0}},
  /* FEED   */ {{85, 35, 45},  {85, 35, 0}},
  /* PET    */ {{55, 240, 0}},
  /* ALERT  */ {{100, 70, 60}, {100, 70, 60}, {100, 70, 0}},
  /* NIGHT  */ {{60, 150, 0}},
};
const uint8_t PATTERN_LENGTHS[HAP_COUNT] = {1, 2, 2, 1, 3, 1};

// Buzzer state. activeTune == TUNE_COUNT means "nothing is playing".
uint8_t  activeTune = TUNE_COUNT;
uint8_t  activeNote = 0;
uint32_t nextNoteAt = 0;

// Light sensor state.
//   +3V3 -> R2 (10k) -> LIGHT -> LDR -> GND
//   Bright = LDR resistance low  = node pulled LOW  = low millivolts
//   Dark   = LDR resistance high = node pulled HIGH = high millivolts
// So HIGH millivolts means DARK.
// analogReadMilliVolts() applies the ESP32-C3's factory calibration; raw
// analogRead() is strongly non-linear on this chip, so avoid it.
// Calibrate these two numbers with the procedure in Part 12.3 of the guide.
const int LIGHT_DARK_MV   = 2200; // above this we consider it night
const int LIGHT_BRIGHT_MV = 1500; // below this we consider it day
const uint16_t LIGHT_READ_INTERVAL_MS = 1000;

bool nightMode = false;
int  ambientMilliVolts = 0;
uint32_t lastLightReadAt = 0;

// Haptic state. activePattern == HAP_COUNT means "nothing is playing".
// hapticBusyUntil mutes shake detection while the motor runs AND while it
// coasts down: a coin motor shakes the accelerometer hard enough to look
// exactly like a deliberate shake gesture. This matters more on a bare board,
// where there is no case to damp the vibration first.
uint8_t  activePattern = HAP_COUNT;
uint8_t  activePulse = 0;
bool     hapticIsOn = false;
uint32_t hapticNextAt = 0;
uint32_t hapticBusyUntil = 0;

// ---------------------------------------------------------------------------
// WS2812B LEDs
// ---------------------------------------------------------------------------
// Day palette: bright and colourful. Night palette: dim and warm, so the pet
// reads as "asleep" and the LEDs disturb the LDR less.
void ledsApply(bool night) {
  if (!USE_LEDS) return;
  if (night) {
    pixels.setBrightness(20);
    pixels.setPixelColor(0, pixels.Color(40, 12, 0));  // dim amber
    pixels.setPixelColor(1, pixels.Color(8, 4, 20));   // dim violet
  } else {
    pixels.setBrightness(120);
    pixels.setPixelColor(0, pixels.Color(0, 90, 60));
    pixels.setPixelColor(1, pixels.Color(60, 0, 90));
  }
  pixels.show();
}

// Blank the WS2812s for a couple of milliseconds while the LDR is sampled, so
// our own LEDs cannot light the sensor and fake "daytime". Imperceptible.
void ledsBlankForRead() {
  if (!USE_LEDS) return;
  pixels.clear();
  pixels.show();
}

void ledsFlash(uint8_t r, uint8_t g, uint8_t b, uint16_t ms) {
  if (!USE_LEDS) return;
  pixels.setBrightness(200); // brighter than the resting palette
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.setPixelColor(1, pixels.Color(r, g, b));
  pixels.show();
  delay(ms);
  ledsApply(nightMode);
}

// ---------------------------------------------------------------------------
// Piezo buzzer (passive)
// ---------------------------------------------------------------------------
// tone() in Arduino-ESP32 core 3.x is built on the LEDC peripheral, while the
// WS2812B library uses RMT. Different hardware, so they coexist happily.
// IMPORTANT: let tone()/noTone() own BUZZER_PIN. Do not ledcAttach() it
// yourself, or the two will fight over the pin.
void tunePlay(uint8_t tuneId) {
  if (!USE_SOUND || tuneId >= TUNE_COUNT) return;
  activeTune = tuneId;
  activeNote = 0;
  nextNoteAt = millis();
}

void tuneUpdate() {
  if (activeTune >= TUNE_COUNT) return; // nothing playing
  const uint32_t now = millis();
  if (now < nextNoteAt) return;

  if (activeNote >= TUNE_LENGTHS[activeTune]) {
    noTone(BUZZER_PIN);
    activeTune = TUNE_COUNT;
    return;
  }

  const Note &n = TUNES[activeTune][activeNote];
  if (n.freq == N_REST) {
    noTone(BUZZER_PIN);
  } else {
    tone(BUZZER_PIN, n.freq, n.ms);
  }
  nextNoteAt = now + n.ms + 25; // 25 ms gap keeps repeated notes distinct
  activeNote++;
}

// ---------------------------------------------------------------------------
// LDR light sensor
// ---------------------------------------------------------------------------
void updateLight() {
  if (!USE_LIGHT_SENSOR) return;
  const uint32_t now = millis();
  if (now - lastLightReadAt < LIGHT_READ_INTERVAL_MS) return;
  lastLightReadAt = now;

  ledsBlankForRead();
  delay(2);
  ambientMilliVolts = static_cast<int>(analogReadMilliVolts(LIGHT_PIN));
  ledsApply(nightMode); // restore the LEDs

  // Hysteresis: the gap between the two thresholds stops the mode flapping at
  // dusk, or when a cloud passes.
  if (!nightMode && ambientMilliVolts > LIGHT_DARK_MV)   nightMode = true;
  if ( nightMode && ambientMilliVolts < LIGHT_BRIGHT_MV) nightMode = false;
}

// React to a day<->night change exactly once: nap at night, wake in daylight.
// This lives here rather than in loop() because nappingUntil, nappingPetX,
// isNapping() and petWalkingX() are all declared above this block, so they are
// visible here - and putting it here means loop() only needs ONE new line.
void updateNightModeTransition() {
  static bool lastNightMode = nightMode;
  if (nightMode == lastNightMode) return;
  lastNightMode = nightMode;

  if (nightMode) {
    if (!isNapping()) {
      nappingUntil = millis() + NAP_DURATION_MS;
      nappingPetX = petWalkingX(millis());
    }
    tunePlay(TUNE_NIGHT);
    hapticPlay(HAP_NIGHT);
  } else {
    tunePlay(TUNE_HAPPY);
  }
  ledsApply(nightMode);
}

// ---------------------------------------------------------------------------
// Haptic motor (through the AO3400A MOSFET - never straight from a GPIO)
// ---------------------------------------------------------------------------
// 20 kHz keeps the PWM above the audible range; a coin motor PWM'd at 1 kHz
// whines annoyingly.
void hapticPlay(uint8_t patternId) {
  if (!USE_HAPTIC || patternId >= HAP_COUNT) return;
  activePattern = patternId;
  activePulse = 0;
  hapticIsOn = false;
  hapticNextAt = millis();
}

void hapticUpdate() {
  if (activePattern >= HAP_COUNT) return; // nothing playing
  const uint32_t now = millis();
  if (now < hapticNextAt) return;

  if (hapticIsOn) {
    // Finish the pulse that is currently running.
    ledcWrite(HAPTIC_PIN, 0);
    hapticIsOn = false;
    const uint16_t gap = PATTERNS[activePattern][activePulse].offMs;
    activePulse++;
    if (activePulse >= PATTERN_LENGTHS[activePattern]) {
      activePattern = HAP_COUNT;
      // 500 ms was chosen for a motor mounted directly on the board, where
      // nothing damps the vibration before it reaches the accelerometer.
      // Raise it to 800 if you still see phantom shakes (see Part 10.3).
      hapticBusyUntil = now + 500;
      return;
    }
    hapticNextAt = now + gap;
    return;
  }

  // Start the next pulse.
  const Pulse &p = PATTERNS[activePattern][activePulse];
  ledcWrite(HAPTIC_PIN, static_cast<uint32_t>(p.percent) * 255 / 100);
  hapticIsOn = true;
  hapticNextAt = now + p.onMs;
}

// ---------------------------------------------------------------------------
// Setup and per-loop update
// ---------------------------------------------------------------------------
void expansionSetup() {
  if (USE_LEDS) {
    pixels.begin();
    pixels.setBrightness(120);
    pixels.clear();
    pixels.show();
  }
  if (USE_HAPTIC) {
    // Core 3.x LEDC API: attach the pin, not a channel.
    ledcAttach(HAPTIC_PIN, 20000, 8); // 20 kHz, 8-bit duty (0-255)
    ledcWrite(HAPTIC_PIN, 0);
  }
  if (USE_LIGHT_SENSOR) {
    // One reading at boot, while the LEDs are still off. This is the most
    // trustworthy sample you will ever get.
    delay(5);
    ambientMilliVolts = static_cast<int>(analogReadMilliVolts(LIGHT_PIN));
    nightMode = ambientMilliVolts > LIGHT_DARK_MV;
  }
  if (USE_SOUND) {
    tunePlay(TUNE_HAPPY);
  }
}

void expansionUpdate() {
  tuneUpdate();
  hapticUpdate();
  updateLight();
  updateNightModeTransition();
}
```

### 11.4 Step 3 — connect it to Starbie (one line + six hooks)

**First, the two one-line additions.**

- [ ] **1.** In `setup()`, add `expansionSetup();` as the **last** line, right after `drawCurrentView();`:

      ```cpp
        drawCurrentView();
        expansionSetup();     // <-- add this line
      }
      ```

- [ ] **2.** In `loop()`, add `expansionUpdate();` right after `updateMpu();`:

      ```cpp
      void loop() {
        updateMpu();
        expansionUpdate();    // <-- add this line
        updateDht();
        ...
      ```

That's it for the plumbing. Day/night behaviour is **fully automatic** — you don't need to write anything
for it.

**Now the six small hooks**, so the pet reacts with sound and vibration. Each is one or two lines inside a
function that already exists.

- [ ] **3. In `checkForShake()`** — add the mute guard at the top, and the reactions where a shake is accepted:

      ```cpp
      void checkForShake() {
        if (!mpuFound || currentView != PET_VIEW) {
          return;
        }
        if (millis() < hapticBusyUntil) {   // ADD: ignore our own vibration
          return;
        }
        ...
          changePet(SHAKE_JOY_CHANGE, SHAKE_ENERGY_CHANGE, SHAKE_FULLNESS_CHANGE);
          savePet();
          tunePlay(TUNE_ALERT);            // ADD
          hapticPlay(HAP_ALERT);           // ADD
        }
      }
      ```

- [ ] **4. In `openMenu()`** — a blip and a tick when the menu opens:

      ```cpp
        menuCenterAccelerationX = accelerationX;
        menuCenterAccelerationY = accelerationY;
        tunePlay(TUNE_BLIP);               // ADD
        hapticPlay(HAP_TICK);              // ADD
      }
      ```

- [ ] **5. In `chooseMenuItem()`** — one line pair per reaction:

      ```cpp
        if (item.reaction == NAP_REACTION) {
          petJumpStartedAt = 0;
          nappingPetX = petWalkingX(now);
          nappingUntil = now + NAP_DURATION_MS;
          tunePlay(TUNE_SLEEP);            // ADD
          hapticPlay(HAP_PET);             // ADD
        } else if (item.reaction == RUN_REACTION) {
          petJumpStartedAt = 0;
          playRunStartedAt = now;
          heartAnimationEndsAt = now + playRunDuration();
          tunePlay(TUNE_HAPPY);            // ADD
          hapticPlay(HAP_SELECT);          // ADD
          ledsFlash(0, 60, 90, 120);       // ADD
        } else {
          petJumpStartedAt = now;
          if (item.reaction == HEART_REACTION) {
            heartAnimationEndsAt = now + HEARTS_DURATION_MS;
            tunePlay(TUNE_HAPPY);          // ADD
            hapticPlay(HAP_PET);           // ADD
          } else {  // JUMP_REACTION == FEED
            tunePlay(TUNE_FEED);           // ADD
            hapticPlay(HAP_FEED);          // ADD
          }
        }
      ```

- [ ] **6. In `handleButtons()`** — a tick on every button press:

      ```cpp
      void handleButtons() {
        if (wasPressed(buttonOne)) {
          hapticPlay(HAP_TICK);            // ADD
          if (currentView == MENU_VIEW) {
            chooseMenuItem();
          } else {
            openMenu();
          }
        }

        if (wasPressed(buttonTwo)) {
          hapticPlay(HAP_TICK);            // ADD
          currentView = currentView == STATS_VIEW ? PET_VIEW : STATS_VIEW;
        }
      }
      ```

### 11.5 Optional — using `nightMode` elsewhere

`nightMode` and `ambientMilliVolts` are ordinary global variables. Any code **defined after** the
expansion block can read them — which includes all of Starbie's drawing and animation functions.

**A. Show the light level on the stats screen.** In `drawStats()`, the bottom line currently prints
temperature and humidity. Add to it:

```cpp
  display.setCursor(4, 53);
  display.print(nightMode ? "NIGHT" : "DAY");
  display.print(" ");
  display.print(ambientMilliVolts);
```

**B. Make the pet walk slower at night.** In `petWalkingX()`, change one line:

```cpp
  const uint32_t step = (now / (PET_WALK_PIXEL_MS * (nightMode ? 2 : 1))) % roundTrip;
```

**C. Getting more resolution from the ADC.** Two options if your threshold sits in a crowded part of
the range:

*Change the attenuation.* The default 11 dB is the **widest** range, so it has the coarsest steps.
Narrower settings trade range for resolution:

```cpp
// Put this in expansionSetup() before the first reading, then re-calibrate your
// thresholds - they will move.
analogSetPinAttenuation(LIGHT_PIN, ADC_6db);
```

*Averaging.* This ADC is genuinely noisy; four reads averaged removes most of the jitter:

```cpp
int lightMilliVolts() {
  uint32_t total = 0;
  for (uint8_t i = 0; i < 4; i++) {
    total += analogReadMilliVolts(LIGHT_PIN);
  }
  return static_cast<int>(total / 4);
}
```

Then call `lightMilliVolts()` instead of `analogReadMilliVolts(LIGHT_PIN)` inside `updateLight()`.

**D. Your own sounds.** Add a row to the `TUNES` table, add a name to the `TUNE_*` enum, add its length to
`TUNE_LENGTHS`, and call `tunePlay(YOUR_NAME)`. The `N_*` names are note frequencies in Hz.

### 11.6 If the compile fails

| Error message | Cause | Fix |
|---------------|-------|-----|
| **`'ButtonState' was not declared in this scope`**<br>**`'Pulse' does not name a type`** | 🎯 **The block is pasted in the wrong place.** Arduino hoisted the generated prototypes above Starbie's `struct` declarations. | Move the block so it starts immediately **before** `int keepInRange(int value, int smallest, int largest) {`. See §11.3. |
| `'wasPressed' cannot be used as a function` | Same cause as above. | Same fix. |
| `'ledcAttach' was not declared in this scope` | You have ESP32 core **2.x**, not 3.x | Update the board package in Boards Manager, or replace the haptics setup with the old API: `ledcSetup(0, 20000, 8); ledcAttachPin(HAPTIC_PIN, 0);` and use `ledcWrite(0, duty)` |
| `Adafruit_NeoPixel.h: No such file or directory` | Library not installed | Install **Adafruit NeoPixel** via `Sketch → Include Library → Manage Libraries` |
| `expected '}' at end of input` | The block got pasted *inside* another function | Move it to file scope — it should sit between two function definitions, not inside one |
| `redefinition of 'Note'` / `'Pulse'` | You already have a `Note` or `Pulse` struct | Rename yours, or delete mine |
| `'pixels' was not declared in this scope` | You deleted the `Adafruit_NeoPixel pixels(...)` line from the block | Put it back. It must be in the block, **not** next to `Adafruit_MPU6050 mpu;`, because it has to be declared above the functions that use it |
| Warnings about unused variables | Harmless | Ignore |

### 11.7 Upload

- [ ] **1.** Connect the XIAO by USB.
- [ ] **2.** `Tools → Port` → select the XIAO's port.
- [ ] **3.** Click **Upload**.
- [ ] **4.** `Tools → Serial Monitor` at **115200** baud shows Starbie's diagnostics.

---

## Part 12 — Testing it, step by step

> **Follow this order.** Each test assumes the previous one passed. Testing everything at once means you
> won't know what's broken.

### 12.1 Test 0 — power and boot (motor NOT plugged in)

- [ ] **1.** Plug in USB **without** the motor connected to J3.
- [ ] **2.** With a USB power meter: expect well under 200 mA at idle.
- [ ] **3.** The OLED should light up and show the pet.
- [ ] **4.** **Unplug and re-plug ten times.** It must boot every single time.

🛑 **This test validates the whole GPIO2 fix from §3.3.** If it ever fails to boot, then either the
`DHT11 DATA` label isn't on U1 pad 1, or R1 isn't pulling that node high. Go back to §5.2 Step 1.

### 12.2 Test 1 — buzzer

Power-cycle and listen. `expansionSetup()` plays `TUNE_HAPPY`.

| What you observe | What it means |
|------------------|---------------|
| Four ascending notes | ✅ Working |
| A single click, then silence | You almost certainly have an **active** buzzer. Replace it with a passive one. |
| Silence | Check BZ1's polarity: the `−` pin must go to **GND**, not to `BUZZ`. Check `BUZZ` is on U1 pad 11. |
| Very quiet, buzzy | R7 (100 Ω) may be too large, or the disc is pressed against something. Try shorting R7. |

### 12.3 Test 2 — calibrating the light sensor

Temporarily add this to `loop()`:

```cpp
Serial.print("ambient mV: ");
Serial.println(analogReadMilliVolts(LIGHT_PIN));
```

Note the reading in three conditions (keep the LEDs off):

| Condition | Expected | Typical reading |
|-----------|----------|-----------------|
| Cover the LDR completely with your hand | HIGH (dark) | 2000–2500 mV |
| Normal room lighting | middle | 500–1200 mV |
| Shine a phone torch at it | LOW (bright) | 50–400 mV |

✅ **Set your thresholds from your own numbers**, not from the table:

```
LIGHT_DARK_MV   = (your room reading + your dark reading) / 2
LIGHT_BRIGHT_MV = LIGHT_DARK_MV - 700        // the hysteresis band
```

| Symptom | What it means |
|---------|---------------|
| Reading never changes | The LDR and R2 are swapped, the LDR is shorted, or `LIGHT` isn't on U1 pad 2 |
| Stuck near 0 | The LDR is missing (open circuit), or R2 isn't connected to 3.3 V |
| Stuck near 2500 | R2 is missing, or the LDR is open |
| Jumps by hundreds of mV | Normal for this ADC. Rely on the hysteresis, or average 4 samples (§11.5C) |
| Says "night" when the LEDs turn on | You're still running with the LEDs on while sampling — use the built-in `updateLight()` |

### 12.4 Test 3 — vibration motor

Plug the motor into J3, then trigger an alert (shake the board, or call `hapticPlay(HAP_ALERT);`).

| What you observe | What it means |
|------------------|---------------|
| Three distinct pulses | ✅ Working |
| **Runs continuously the moment you plug in USB** | 🛑 **Unplug immediately.** Q1 is shorted (check for a solder bridge between its pins), D3 is backwards, or the gate is stuck high. |
| One weak pulse | The 3.3 V rail is sagging. Check C3/C4 are actually at J3. |
| Nothing at all | Check J3 polarity, Q1's orientation (SOT-23 pin 1 is the gate — verify against the footprint silkscreen), and that `HAPTX` is on U1 pad 8. |
| Audible whine | Raise the `ledcAttach` frequency from 20000 to 25000, or lower the duty percentage. |
| The pet randomly changes stats when the motor fires | The `hapticBusyUntil` guard is missing from `checkForShake()`. |

**Rail check:** while the motor pulses, measure `+3.3V`. If it dips below ~3.0 V, add bulk capacitance
at C3.

**Coupling check (bare board):** while the motor pulses, watch the pet's stats on the OLED. They must
not change. With the motor stuck to the same PCB as the accelerometer and no case to damp it, this is
the test most likely to catch something. If the stats do change, raise the `+ 500` in `hapticUpdate()`
to `+ 800` and re-test (§10.3).

### 12.5 Test 4 — the LEDs

They should follow the day/night palette. Wave your hand over the LDR; after the mode flips, the LEDs
should visibly change brightness and colour.

| Symptom | What it means |
|---------|---------------|
| Random colours or flickering | The classic case of 3.3 V logic driving a 5 V WS2812B. Usually works via R3; if unreliable, add a level shifter or power the LEDs from 3.3 V. |
| Nothing at all | Check C1/C2 are present and near D1/D2, and that `LED_PIN = 21`. |

### 12.6 Test 5 — everything together

- [ ] **1.** Cover the LDR → the pet naps, the LEDs dim, the night tune plays, the motor buzzes once.
- [ ] **2.** Uncover it → happy tune, LEDs brighten.
- [ ] **3.** Press Button 1 → tick + blip. Tilt to select. Press again → that action's sound and haptic.
- [ ] **4.** Shake the board → alert chirp and triple pulse, stats change **once**.
- [ ] **5.** Shake it **while the motor is running** → stats must **not** change. This is the guard test.

### 12.7 ✅ Final boot test

After everything works, unplug and re-plug **ten more times**, in both bright and dark conditions.
A board that boots reliably only in daylight is not finished.

---

## Part 13 — Troubleshooting

| Symptom | Most likely cause | Fix |
|---------|-------------------|-----|
| Board won't boot at all | GPIO2 strap held low | Confirm `DHT11 DATA` is on **U1 pad 1** and R1 connects to `+3.3V`. See §3.2–3.3. |
| Boots only sometimes | Same, marginally; or a cold solder joint | Reflow the DHT11 line and R1. Measure GPIO2 at power-on: it must be > 2.5 V. |
| OLED blank, "OLED not found" | I²C pins | `I2C_SDA_PIN = 7`, `I2C_SCL_PIN = 6` (§2.4) |
| MPU6050 not found | Same I²C issue, or a bad J2 joint | Check J2 pins 3 and 4 |
| DHT11 reads `nan` | Data line moved but `DHT_PIN` still 3 | Set `DHT_PIN = 2` (§11.2) |
| Buzzer silent | Active buzzer, or reversed polarity | §12.2 |
| Buzzer always on | `BUZZ` label accidentally on a power net | Check U1 pad 11 |
| Light sensor always "night" | Divider wired the wrong way round | Must be `+3V3 → R2 → node → LDR → GND` |
| Light sensor always "day" | LDR open, or R2 missing | Measure the node voltage directly |
| Day/night flapping at dusk | Thresholds too close together | Widen the gap to 700 mV or more (§11.3) |
| Night mode triggers when LEDs turn on | The LDR can see D1/D2 | Move the LDR |
| Motor runs constantly | Q1 shorted, D3 backwards, or gate stuck high | Unplug. Check continuity per §9.4 |
| Motor never runs | `HAPTX` not on U1 pad 8, Q1 misoriented, or Q1 source not grounded | §5.3 Step 2 |
| Motor weak | Rail sag | Bigger C3, or lower the `percent` in the patterns |
| Fake "shakes" when the motor fires | Shake guard missing | Add `if (millis() < hapticBusyUntil) return;` to `checkForShake()` |
| Stats change on their own | Same as above, or the MPU6050 is too close to the motor | Move the motor; add foam |
| Haptics stutter the animation | You used a blocking `delay()` | Use the non-blocking `hapticUpdate()` from Part 11 |
| Everything resets when the motor fires | Insufficient bulk capacitance | Put C3 at J3; widen the power tracks |
| Light reading is noisy | `LIGHT` routed near `HAPTX` or the motor loop | Reroute, or add a ground guard track |
| ERC says the gate isn't driven | False positive on MOSFET gates | §6.2 |
| DRC clearance errors on new pads | Parts too close to a star tip edge | Nudge them inward |
| DRC "unconnected items" | You changed the schematic after the last F8 | Re-run `Update PCB from Schematic` |
| Compile fails with `ButtonState`/`Pulse` errors | The block is in the wrong place | §11.6 — move it before `keepInRange` |

---

## Part 14 — Ideas for later

**Easy wins**

1. **A "pet me" gesture.** Use the gyro to detect a distinctive stroke; respond with the rumble and hearts.
2. **Log the light levels** to flash over a day, then set your thresholds from real data instead of two
   hand-picked numbers.
3. **Battery-aware haptics.** On a LiPo, reduce motor duty when the battery is low, and disable haptics
   below a cutoff so you don't brown out the regulator.

**Medium**

4. **Replace the LDR with a BH1750.** An I²C ambient-light sensor with calibrated **lux** output — it
   needs no analogue pin at all, freeing `LIGHT` completely, and gives you real numbers to build a proper
   brightness curve from. A few dollars.
5. **I²S audio.** Swap the piezo for something like a MAX98357A and a small speaker, and play real WAV
   samples instead of square waves. A big step up in capability, and a modest step up in cost.
6. **Better haptics: an LRA.** A linear resonant actuator feels far more refined than a coin motor, but
   needs AC drive at its resonant frequency (~235 Hz typical) — use a DRV2605 or an H-bridge.

**Ambitious**

7. **Touch-sensitive petals.** ⚠️ Note that the **ESP32-C3 has no built-in capacitive-touch peripheral**
   (unlike the classic ESP32, S2 and S3). Use a cheap external **TTP223** module instead — about $0.20,
   and it gives you a clean digital HIGH/LOW you can treat exactly like a button. One behind each petal.
8. **A two-board "language".** Give two Starbiews a short piezo chirp vocabulary, or use the disc as a
   crude ultrasonic transmitter/receiver pair.
9. **Have JLCPCB assemble it.** All the SMD parts here are LCSC stock items, and most are Basic/Preferred
   parts with no extended-part fee. Then you only solder the through-hole parts.

---

## Appendix A — Pin reference

| XIAO | GPIO | ADC | Strapping? | Used for |
|------|------|-----|-----------|----------|
| D0 | 2 | ADC1_CH2 | 🛑 must be HIGH at boot | **DHT11 data** (after the swap) |
| D1 | 3 | ADC1_CH3 | No | **LDR (`LIGHT`)** |
| D2 | 4 | ADC1_CH4 | No | SW1 Button 1 |
| D3 | 5 | ADC2_CH0 *(unusable)* | No | SW2 Button 2 |
| D4 | 6 | — | No | **SCL** (OLED + MPU6050) |
| D5 | 7 | — | No | **SDA** (OLED + MPU6050) |
| D6 | 21 | — | No | **WS2812B data** (via R3) |
| D7 | 20 | — | No | **Motor gate (`HAPTX`)** |
| D8 | 8 | — | ⚠️ Yes | *unused* |
| D9 | 9 | — | ⚠️ Yes (BOOT) | *unused* |
| D10 | 10 | — | No | **Buzzer (`BUZZ`)** |

**Power:** U1 pad 12 = `3V3_OUT` · pad 14 = `VBUS` (5 V USB) · pads 13, 18, 22 = `GND` · pad 21 = `VBAT`

**New nets:** `BUZZ` (U1 pad 11 → R7 → BZ1+) · `HAPTX` (U1 pad 8 → R5 → Q1 gate) · `LIGHT` (U1 pad 2 → R2/R4 junction)

---

## Appendix B — Net list after the change

```
/DHT11 DATA:  R1/2, U1/1, U2/2          <- was U1/2; now U1/1 (D0 / GPIO2)
/LIGHT:       R2/*, R4/*, U1/2          <- NEW  (D1 / GPIO3 / ADC1_CH3)
/BUZZ:        R7/*, U1/11               <- NEW  (D10 / GPIO10)
/HAPTX:       R5/*, U1/8                <- NEW  (D7 / GPIO20)
Net-(Q1-D):   J3/2, Q1/3, D3/A          <- NEW
Net-(Q1-G):   Q1/1, R5/*, R6/*          <- NEW
+3.3V:        ... + R2/1, J3/1, C3/1, C4/1, D3/K
GND:          ... + BZ1/2, R4/2, R6/2, Q1/2, C3/2, C4/2
```

Everything else is unchanged.

---

## Appendix C — Where the facts come from

| Claim | Source |
|-------|--------|
| GPIO2/GPIO8/GPIO9 are strapping pins; GPIO2 must be **HIGH** for both boot modes | [ESP32-C3 Wireless Adventure §5.2.6](https://espressif.github.io/esp32-c3-book-en/chapter_5/5.2/5.2.6.html) |
| ESP32-C3 strapping pins | [Espressif GPIO API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/gpio.html) |
| Boot-mode combinations | [esptool boot mode selection](https://docs.espressif.com/projects/esptool/en/latest/esp32c3/advanced-topics/boot-mode-selection.html) |
| ADC1 = GPIO0–4; ADC2 = GPIO5 | [ESP-IDF ADC (ESP32-C3)](https://docs.espressif.com/projects/esp-idf/en/v4.3.1/esp32c3/api-reference/peripherals/adc.html) |
| ADC2 not-work erratum | [Espressif chip errata](https://docs.espressif.com/projects/esp-chip-errata/en/latest/esp32c3/03-errata-description/shared/sar-adc-adc2-not-work.html) |
| XIAO ESP32-C3 pin map; ADC1 recommended | [Seeed getting started](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/) |
| XIAO GPIO allocation notes | [Seeed pin multiplexing](https://wiki.seeedstudio.com/exp32c3_d9_d6_d8/) |
| `ledcAttach`/`ledcWrite`/`ledcDetach` (core 3.x) | [Arduino-ESP32 LEDC](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html) |
| `ledcSetup`/`ledcAttachPin` removed in 3.0 | [2.x → 3.0 migration](https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html) |
| `analogReadMilliVolts`, ADC defaults | [Arduino-ESP32 ADC](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html) |
| `tone()` is implemented on LEDC | [`Tone.cpp`](https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/Tone.cpp) |
| Adafruit_NeoPixel uses RMT on ESP32 | [`esp.c`](https://raw.githubusercontent.com/adafruit/Adafruit_NeoPixel/master/esp.c) |
| Discrete motor driver design | [Precision Microdrives AB-001](https://www.precisionmicrodrives.com/ab-001-discrete-driver-circuits-for-vibration-motors) |
| JLCPCB Gerber/drill settings for KiCad | [JLCPCB KiCad guide](https://jlcpcb.com/help/article/how-to-generate-gerber-and-drill-files-in-kicad-9) |
| LCSC handling fee under $15 | [LCSC terms](https://www.lcsc.com/help/terms-and-conditions) |
| Part datasheets: [TDK PS1240P02BT (C76871)](https://www.lcsc.com/product-detail/C76871.html) · [GL5528 (C10081)](https://www.lcsc.com/product-detail/C10081.html) · [AO3400A](https://www.aosmd.com/res/data_sheets/AO3400A.pdf) · [SS14 (C2909920)](https://www.lcsc.com/product-detail/Schottky-Barrier-Diodes-SBD_KUU-SS14_C2909920.html) · [10 kΩ 0805 (C17414)](https://www.lcsc.com/product-detail/Chip-Resistor-Surface-Mount_Uniroyal-Elec-0805W8F1002T5E_C17414.html) |
| Upstream project | [SharKingStudios/Starbie](https://github.com/SharKingStudios/Starbie) |

---

## Revision notes

**Revision 4 (this document).** Adjusted for a **bare-board build with no enclosure** — the PCB is the
finished object, so nothing is mounted in a case.

* **Part 10 rewritten** from "The case" to "Building without an enclosure". The buzzer needs *less* help
  (a disc on open leads is louder than one pressed into plastic), while the light sensor and the motor
  need *more*. Added the black-heat-shrink trick that turns the LDR into a directional sensor, and a
  bare-board handling checklist (rubber feet, no conductive surfaces, don't flex the star tips).
* **The motor's shake guard was raised from 300 ms to 500 ms**, because with no case to damp it the
  vibration reaches the accelerometer far harder. §10.3 gives the two dials to turn if phantom shakes
  persist, and §12.4 adds a coupling test for it.
* **Removed every case-dependent instruction:** no acoustic holes, no "press it against the case wall",
  no enclosure clearance checklist, and no case-constrained placement ordering. §5.3's LDR placement
  choice previously recommended "a hole in the case" as the best option; it now reads as a *flying lead*
  (`J4`) that wins by distance instead.
* **Buzzer placement now calls out a real board constraint:** the disc is 12.2 mm across, so a narrow
  star tip will not fit it — check against the outline before committing to a spot.

**Revision 3.** Restructured to be beginner-first — three reading tracks, per-part time
estimates, plain-English summaries, checkbox steps, ✅ checkpoints, and a lookup-table troubleshooting
section — while keeping the full technical depth.

**Correctness changes carried in from revision 2's verification pass:**

* **The firmware was actually compiled.** It now builds cleanly for `esp32:esp32:XIAO_ESP32C3`
  (arduino-cli 1.5.1, ESP32 core 3.3.12, 0 errors). Revision 2's code did **not** compile.
* **Two real build failures were found and fixed:**
  1. **Wrong placement.** The expansion block was positioned above Starbie's `struct PetState` /
     `struct ButtonState`. Arduino inserts function prototypes just above the first function definition,
     so the block dragged that insertion point above the structs, and the build failed with
     `'ButtonState' was not declared in this scope` and `'Pulse' does not name a type`. The block now goes
     **immediately before `int keepInRange(...)`**, and all its structs are declared before its first
     function. §11.3 and §11.6 explain this.
  2. **`pixels` declared too late.** The NeoPixel object was to be added next to `Adafruit_MPU6050 mpu;`,
     which is *below* the code that uses it. It is now declared inside the expansion block.
* **The API was simplified.** `tunePlay(TUNE_HAPPY)` and `hapticPlay(HAP_ALERT)` replace the old
  `tunePlay(TUNE_HAPPY, sizeof(...)/sizeof(...))` calls. This removed custom types from the function
  signatures, which is what makes the code immune to prototype-hoisting problems, and made call sites
  much easier to read.
* **The night-mode logic moved into the block**, so it's automatic and `loop()` needs only one added line
  instead of a spliced-in code fragment.
* **The I²C note was corrected.** Revision 1 blamed the sketch; in fact the shipped `Starbie.ino` is
  correct (SDA 7 / SCL 6) and the **Week 1 `Guide.md` prose is wrong**. See §2.4.
* **Verified additions:** the `USE_*` feature switches were tested (the sketch compiles with all four
  off); the LDR and buzzer footprint geometry was confirmed from the KiCad footprint files (3.4 mm and
  5.0 mm pin spacing); and the haptic and tone state machines were tested for termination, correct pulse
  and note counts, and — most importantly — that neither the motor nor the buzzer is ever left on.

**Known limitations:** the firmware has been *compiled and its logic tested*, but not **run on hardware**.
The instructions in Parts 5–10 have been checked against the KiCad libraries and your project files, but
the schematic and PCB have not been modified by this guide's author — you are the first to build it.
