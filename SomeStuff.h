#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h" // for attachInterrupt, FALLING
#else
#include "WProgram.h"
#endif
#define redraw __redraw[gr.cprocess]
#include <NMT_GFX.h>
#include <GEAR.h>
#include <PS2KeyAdvanced.h>
#define PS2_TAB        0x11D
#define PS2_ENTER      0x11E
#define PS2_DOWNARROW  0x118
#define PS2_UPARROW    0x117
#define PS2_LEFTARROW  0x116
#define PS2_RIGHTARROW 0x115
#define PS2_ESC        0x11B
#define PS2_DEL        0x11A
GearControl gr;
byte data = 0;
NMT_GFX vga;
PS2KeyAdvanced kbd;
byte selected=0;
bool term_opn=false;
byte term_y=0;
byte gui=0;
byte sel_process=0;
bool __redraw[40];
void (*reset)()=0;/*
byte _HEAP[4096];
uint32_t RAMS[32];
byte allocs=0;
byte* alloc(uint16_t len){
  int start=0;
  if(allocs>0)
    start=RAMS[allocs-1]>>16;
  if((start+len)>4095){
    byte i=0;
    for(i=0;i<allocs;i++){
      if(RAMS[i]&0x00FF>=gr.processes){
        if(RAMS[i]>>16-RAMS[i-1]>>16>=len){
          start=RAMS[i-1]>>16;
        }
      }
    }
    RAMS[i]&=0xFFFF0000;
    RAMS[i]|=gr.cprocess;
  }else{
    RAMS[allocs]=(start+len)<<16;
    RAMS[allocs]|=gr.cprocess;
    allocs++;
  }
  return (byte*)(((uint32_t)RAMS)+start);
}//*/
void term_close(){
  term_opn=false;
  term_y=0;
}
void sw_gui(byte q){
  gui=q;
  for(byte i=0;i<40;i++)
    __redraw[i]=true;
}
uint16_t last_key=0;
uint16_t read(){
  if(gr.cprocess==sel_process)
    return last_key;
  else{
    return 0;
  }
}
bool available(){
  return (gr.cprocess==sel_process)&&(last_key>0);
}
int launch(void (*a)(),void (*b)(),char* name="?", void (*c)()=__empty){
  //if(gr.cprocess==sel_process)
    sel_process=gr.addProcess(a,b,name,c,P_KILLER|P_KILLABLE);  // do this before addProcess() to get ID of next new process
  return sel_process;
}
void window(int a, int b, int c, int d){
  a=(a+1)*6-2;
  b=(b+1)*13-1;
  c=(c+1)*6+2;
  d=(d+1)*13+3;
  vga.set_color(1);
  vga.fill_box(a,b,c,d);
  vga.set_color(2);
  vga.box(a,b,c,d);
  vga.fill_box(a,b-5,c+1,b);
}
void alert(const char* q){
  window(3,2,18,4);
  vga.set_color(2);
  vga.set_cursor_pos(3,2);
  vga.print(q);
  vga.set_cursor_pos(4,3);
  vga.set_color(3);
  vga.print("OK");
  vga.set_color(2);
  while(true){
    if(kbd.available())
      if(kbd.read()==PS2_ENTER)
        break;
  }
  vga.set_color(0);
  vga.fill_box(22,33,117,68);
  for(byte i=0;i<gr.processes;i++){
    gr.ftimes[i]=millis()+gr.fupd_rate;
  }
  for(byte i=0;i<40;i++)
    __redraw[i]=true;
}
void println(const char* q){
  if(term_opn==false){
    window(0,0,30,12);
    for(int i=0;i<gr.processes;i++){
      __redraw[i]=true;
    }
  }
  term_opn=true;
  vga.set_cursor_pos(0,term_y);
  term_y++;
  vga.print(q);
}


