## Wiring
| Pico Pins | Module Pins |
| :------: |:----:|
| GP6 | SDA |
| GP7 | SCL |
| GP8 | LCD_DC |
| GP9 | LCD_CS |
| GP10 | LCD_CLK |
| GP11 | LCD_MOSI |
| GP12 | LCD_MISO |
| GP13 | LCD_RST |
| GP25 | LCD_BL |
| GP21 | Touch_INT |
| GP22 | Touch_RST |
| GP23 | DOF_INT1 |
| GP24 | DOF_INT2 |
| GP29 | BAT_ADC |
| GP3 | BAR_CHANNEL |

## Usage
1. `git clone` this repo

2. Update necessary submodules (LVGL)
```
git submodule update --init --recursive
```

3. Set your Pico SDK Path
```
export PICO_SDK_PATH="/path/to/your/pico-sdk/"
```

4. Build and compile
```
mkdir build
cd build
cmake ..
make
```
5. Flash the RP2040-Touch-LCD-1.28.uf2 
