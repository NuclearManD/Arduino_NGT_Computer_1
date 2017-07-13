#include "SomeStuff.h"
#define WORLD_X 64
#define WORLD_Y 64
#define BLOCK_STONE 2
#define BLOCK_GRASS 1
#define BLOCK_AIR 0
#define BLOCK_MAGMA 3
void setup() {
  // put your setup code here, to run once:
  vga.begin();
  kbd.begin(8, 2);
  kbd.setNoBreak(true);
  println("System is booting with GEAR...");
  gr.addProcess(updos,fupos,(char*)"Shell",__empty,P_ROOT|P_KILLER);
  println("Starting system processes...");
  gr.addProcess(__empty,usage_daemon,(char*)"daemon0",P_ROOT);
  println("Setup done, please wait...");
  println("Any key to open shell.");
  while(!kbd.available());
  kbd.read();
  for(byte i=0;i<gr.processes;i++){
    gr.ftimes[i]=millis()+gr.fupd_rate;
  }
  sw_gui(0);
}
void loop() {
  gr.run();
  if(gr.processes==0){
    vga.clear();
    vga.println("Computer has crashed:\n  No more running processes.\n  Any key to reset arduino.");
    while(kbd.available())kbd.read();
    while(!kbd.available());
    reset();
  }
  if(kbd.available())
    last_key=kbd.read();
  else
    last_key=0;
  if(sel_process>=gr.processes){
    sel_process=0;
    sw_gui(0);
  }
}
bool dir=false;
String apps[5]=   {"TaskMan" ,"reboot","Terra"    ,};
void (*upd[5])()= {taskupd   ,__empty ,TerraPlayer,};
void (*fupd[5])()={taskfu    ,__empty ,TerraEnv   ,};
void (*stp[5])()= {__empty   ,0       ,TerraSetup ,};//,__q,__q,__q};
byte napps=3;
void fupos(){
  if(sel_process==0){
    if(redraw){
      selected=0;
      window(0,1,15,6);
      vga.set_color(2);
      vga.set_cursor_pos(0,1);
      vga.print("OPEN PROGRAM:");
      vga.set_cursor_pos(1,2);
      vga.set_color(3);
      vga.print(apps[0]);
      vga.set_color(2);
      for(byte i=1;i<napps;i++){
        vga.set_cursor_pos(1,2+i);
        vga.print(apps[i]);
      }
      term_close();
      redraw=false;
    }
  }else{
    if(redraw){
      redraw=false;
    }
  }
}
void updos(){
  if(sel_process==0){  // Program Opener
    if(available()){
      uint16_t c=read();
      byte q=selected;
      if(c==PS2_DOWNARROW){
        selected++;
        if(selected>=napps)
          selected=0;
      }else if(c==PS2_UPARROW){
        selected--;
        if(selected==255)
          selected+=napps;
      }else if(c==PS2_ENTER){
        q=selected=launch((*upd[selected]),(*fupd[selected]),apps[selected].c_str(),(*stp[selected]));
        
        if(selected>0){
          sw_gui(1);
          vga.clear();
        }else
          alert("ERROR: too many processes!");
      }else if(c==(PS2_ALT|PS2_TAB)){
        selected=0;
        term_close();
        sw_gui(0);
        sel_process+=1;
        if(selected>=gr.processes)
          selected=0;
        vga.clear();
      }
      if(q!=selected){
        for(byte i=0;i<napps;i++){
          if(selected==i)
            vga.set_color(3);
          else
            vga.set_color(2);
          vga.set_cursor_pos(1,2+i);
          vga.print(apps[i]);
        }
      }
    }
  }else{ // Background
    if((last_key==PS2_ESC)&&(sel_process!=0)){
      gr.kill(sel_process);
      selected=0;
      term_close();
      sw_gui(0);
      sel_process=0;
    }else if(last_key==(PS2_ALT|PS2_TAB)){
      selected=0;
      term_close();
      sw_gui(0);
      sel_process+=1;
      if(selected>=gr.processes)
        selected=0;
    }
  }
}
long last_task_time=0;
byte last_task_count=0;
byte task_sel=0;
void taskfu(){
  if(last_task_time>millis())
    return;
  if(last_task_count>gr.processes){
    vga.set_color(0);
    vga.fill_box(112,(gr.processes+3)*13-6,219,(last_task_count+3)*13+3);
  }
  window(18,1,35,2+gr.processes);
  for(int i=0;i<gr.processes&&i<8;i++){
    if(task_sel==i)
      vga.set_color(3);
    else
      vga.set_color(2);
    vga.set_cursor_pos(18,1+i);
    vga.print(48+i);
    if(sel_process!=i)
      vga.print(':');
    else
      vga.print('*');
    vga.print(gr.getName(i));
    if((!strcmp(gr.getName(i),"TaskMan"))&&i!=gr.cprocess){
      alert((char*)"TaskMan already running.");
      gr.kill(i);
    }
  }
  vga.set_color(3);
  vga.set_cursor_pos(18,1+gr.processes);
  vga.print("Free RAM : ");
  vga.print(String(freeRam()));
  last_task_time=millis()+1000;
  last_task_count=gr.processes;
}
void taskupd(){
  byte q=task_sel;
  if(available()){
    uint16_t c=read();
    if(c==PS2_DOWNARROW){
      task_sel++;
    }else if(c==PS2_UPARROW){
      task_sel--;
    }else if(c==PS2_DEL){
      int res=gr.kill(task_sel);
      if(res>=0)
        sel_process=res;  // update selected process
      else if(res==ACCESS_DENIED)
        alert("Access Denied");
      else if(res==INVALID_ARGUMENT)
        alert("Error: A Glitch");
      else
        alert("Unknown Error");
      q=task_sel+1; // redraw IMMEDIATELY
    }
  }
  if(task_sel>=gr.processes)
    task_sel=0;
  else if(task_sel==255)
    task_sel+=gr.processes;
  if(q!=task_sel){
    last_task_time=0; // Allow next call to work...
    taskfu();         // Redraw graphics
  }
}
void usage_daemon(){
  gr.ftimes[gr.cprocess]=millis()+700;
  int freeram=freeRam()/128;
  if(redraw){
    vga.set_color(1);
    vga.fill_box(245,0,256,192);
    vga.set_color(0);
    vga.line(245,127,255,127);
    vga.line(245,192,255,192);
    redraw=false;
  }
  if(freeram<16)
    vga.set_color(2);
  else
    vga.set_color(3);
  vga.fill_box(245,64-freeram+128,255,191);
}
byte img[52];
word plr_img[]={4098,4096, // Sprite args
  //8196,4096, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21844, 21589, 21845, 21845, 21845, 4437, 21845, 21845, 21845, 17749, 21845, 21845, 21824, 5121, 21845, 21845, 21840, 21765, 21845, 21845, 21840, 21765, 21845, 21845, 21845, 21845, 21845, 21845, 21781, 21845, 5461, 21845, 21829, 21844, 21845, 21845, 21840, 1, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845, 21845
//};
10, 40960, 7, 59392, 15, 59392, 3, 59392, 5, 22528, 5, 21504, 5, 21504, 5, 21504, 5, 21504, 13, 23552, 2, 40960, 2, 8192, 2, 8192, 2, 8192, 2, 8192, 15, 12288};
Sprite player;
Sprite magma_1;
byte world[WORLD_X][WORLD_Y];
byte colors[]={0,3,1,2};
byte player_x,player_y;
bool DrawBlocks;
bool isTop=false;
void TerraSetup(){
  isTop=false;
  byte height[WORLD_X];  // For random world generation
  for(byte i=0;i<WORLD_X;i++)
    height[i]=WORLD_Y-11;
  println("Generating world...");
  randomSeed(micros()+millis());
  for(byte i=0;i<16;i++){
    byte q=random(1,3);
    byte n=random(0,WORLD_X);
    height[n-1]-=q;
    height[n]-=q;
    height[n+1]-=q;
  }
  for(byte x=0;x<WORLD_X;x++)
    world[x][height[x]]=BLOCK_GRASS;
  for(byte x=0;x<WORLD_X;x++)
    for(byte y=0;y<height[x];y++)
      world[x][y]=BLOCK_STONE;
  for(byte x=0;x<WORLD_X;x++)
    for(byte y=height[x]+1;y<WORLD_Y;y++)
      world[x][y]=BLOCK_AIR;
  player_x=WORLD_X/2;
  player_y=height[player_x]+1;
  DrawBlocks=true;
  player.binary_image=(byte*)plr_img;
  player.upload();
}
void tesselateRows(){
  byte ux=0,uy=0;
  if(player_x>=6)
    ux=player_x-6;
  if(player_y>=6)
    uy=player_y-6;
  byte block;
  byte x=0;
  byte start=0;
  for(byte y=uy;(y<(player_y+6))&&(y<WORLD_Y);y++){
    block=world[ux][y];
    start=ux;
    for(x=ux+1;(x<(player_x+6))&&(x<WORLD_X);x++){
      if(block!=world[x][y]){
        vga.set_color(colors[block]);
        vga.fill_box((start+6-player_x)*16,(12-y+player_y-7)*16,(x-player_x+6)*16,(12-y+player_y-6)*16);
        start=x;
        block=world[x][y];
      }
    }
    vga.set_color(colors[block]);
    vga.fill_box((start+6-player_x)*16,(12-y+player_y-7)*16,(x-player_x+6)*16,(12-y+player_y-6)*16);
  }
}
void TerraEnv(){
  if(redraw){
    redraw=false;
    DrawBlocks=true;
  }
  if(DrawBlocks){
    byte x=0,uy=0;
    if(player_x>=6)
      x=player_x-6;
    if(player_y>=6)
      uy=player_y-6;
    /*for(x=x;(x<(player_x+6))&&(x<WORLD_X);x++){
      for(byte y=uy;(y<(player_y+6))&&(y<WORLD_Y);y++){
        vga.set_color(colors[(world[x][y])]);
        vga.fill_box((x-player_x+6)*16,(12-y+player_y-7)*16,(x-player_x+7)*16,(12-y+player_y-6)*16);
      }
    }//*/
    tesselateRows();
    vga.set_color(0);
    vga.fill_box(96,80,111,95);
    player.display(96,80,0); // player is always in the center of the screen
    DrawBlocks=false;
  }
}
void TerraPlayer(){
  if(sel_process!=gr.cprocess&&isTop){
    isTop=false;
    vga.tile_color(86,0);
  }else if(sel_process==gr.cprocess&&!isTop){
    isTop=true;
    vga.tile_color(86,62);
    vga.block_color(62,0);
    vga.block_color(126,0x03);
    vga.block_color(190,0x3E);
    vga.block_color(254,0x39);
  }
  if(!available())
    return;  // Nothing to do - No keyboard inp.
  int c=read();
  if(c==PS2_LEFTARROW){
    if(player_x<(WORLD_X-1))
      player_x++;
  }else if(c==PS2_RIGHTARROW){
    if(player_x>0)
      player_x--;
  }else if(c==PS2_UPARROW){
    if(player_y<(WORLD_Y-1))
      player_y++;
  }else if(c==PS2_DOWNARROW){
    if(player_y>0)
      player_y--;
  }else{
    vga.set_cursor_pos(1,1);
    vga.print(String(c).c_str());  // print what key this is
    return;  // prevent redraw
  }
  DrawBlocks=true; // redraw: we moved
}
int freeRam() {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


