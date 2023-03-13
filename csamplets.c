/* 
 * example to use libbabies
 * H.B. RIKEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libbabies.h"
#include "segidlist.h"

int efn = 0;
unsigned int scrbuff[32] = {0};
unsigned long long int ts = 0;

void quit(void){
  printf("Exit\n");
}

void start(void){
  printf("Start\n");
}

void stop(void){
  printf("Stop\n");
}

void reload(void){
  printf("Reload\n");
}

void sca(void){
  int i;
  printf("Sca\n");
  for(i=0;i<32;i++){
    scrbuff[i] = scrbuff[i] + 1;
  }
  babies_init_ncscaler(efn);
  babies_scrdata((char *)scrbuff, sizeof(scrbuff));
  babies_end_ncscaler32();
}

void clear(void){
  // write clear function i.e. send end-of-busy pulse
  //printf("Clear\n");
}

// thread
void evtloop(void){
  int status, i;
  short tdata[64];

  while((status = babies_status()) != -1){
    switch(status){
    case STAT_RUN_IDLE:
      /* noop */
      break;
    case STAT_RUN_START:
    case STAT_RUN_NSSTA:
      usleep(1000); // wait event
      //printf("evtloop\n");
      
      babies_init_eventts();
      //babies_init_segment(segid)
      //MKSEGID(device, focalplane, detector, module)
      //please see segidlist.h
      //module is important, e.g. C16 is 16bit data
      //device, focalplane, detector, can be defined as you want
      babies_init_segment(MKSEGID(0, 0, 0, C16));

      for(i=0;i<64;i++){
	tdata[i] = i;
      }
      babies_segdata((char *)tdata, sizeof(tdata));

      babies_end_segment();

      ts += efn;
      babies_copyts(ts);

      babies_end_eventts();

      // babies_chk_block(int maxbuff)
      // if block size is larger than maxbuff,
      //  data should be send to the event builder
      //  i.e., read scaler and flush
      // example : 8000 = 16kB
      if(babies_chk_block(8000)){
	sca();
	babies_flush();
      }
      clear();

      break;
    case STAT_RUN_WAITSTOP:
      // for the last sequense of run
      sca();
      babies_flush();
      babies_last_block();
      break;
    default:
      break;
    }
  }

  // write codes to quit safely

}

int main(int argc, char *argv[]){

  if(argc != 2){
    printf("csample EFN\n");
    exit(0);
  }else{
    efn = strtol(argv[1], NULL, 0);
  }

  babies_quit(quit);
  babies_start(start);
  babies_stop(stop);
  babies_reload(reload);
  babies_evtloop(evtloop);
  babies_name("csample");

  babies_init(efn);
  babies_check();

  ts = efn;

  babies_main();

  return 0;
}
