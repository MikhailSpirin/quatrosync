# Quatrosync

This is a module in Eurorack format. Basically it is complex wave 4-channel LFO, based on ESP8266 and MCP4728 Quad DAC. It has sync, it has patterns, perlin noise smooth random, all controlled with one potentiometer; each channel has different logic, but all complex lfos are based on [Adventure Kid's Waveforms](https://www.adventurekid.se/akrt/waveforms/adventure-kid-waveforms/).

Longer story is in my Digital Garden [here](https://mikhailspirin.github.io/khaki-book/electronics/ownprojects/quatrosync/)


### Basic scheme 
```
┌───────┐ ┌──────────────┐                            
│Trig IN│ │Potentiometer │                            
└───┬───┘ └──┬───────────┘                            
    │        │                                        
    │        │                                        
┌───▼────────▼──┐      ┌────────┐      ┌───────┐      
│ESP8266 NodeMCU├──────► MCP4728├──────► TL074 │      
└───────────────┘      └────────┘      └┬─┬─┬─┬┘      
                                        │ │ │ │       
                                        │ │ │ ▼       
                                        │ │ │ CV out A
                                        │ │ ▼         
                                        │ │ CV out B  
                                        │ ▼           
                                        │ CV out C    
                                        ▼             
                                        CV out D          
```
### Some photos of working device

### Video of the workflow