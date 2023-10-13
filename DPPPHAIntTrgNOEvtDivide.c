/* 
 * example to use libbabies
 * H.B. RIKEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "libbabies.h"
#include "segidlist.h"

#include <CAEN_FELib.h>
#include "inttypes.h"

//#define DEBUG 1

struct RawEvent{
  uint8_t *data;
  size_t  size;
  uint32_t  n_events;  
};

int SetValue2Ch (uint64_t, char*, char*, int);
int SetValue2Deg(uint64_t, char*, char*);
int SetValue2LVDS(uint64_t, char*, char*, char*);
int GetValueFromCh (uint64_t, char*, char*, int);
int GetValueFromDeg(uint64_t, char*, char*);
int InitV2740(void);

int efn = 0;
unsigned int scrbuff[32] = {0};
int usl = 1000;
uint64_t devHandle;
const char endpoint[256] = "raw";
int Nnowaveform = 0;

void quit(void){
  printf("Exit\n");
}

void start(void){
  printf("Start\n");
  //  InitV2740();

  int ec;
  fprintf(stdout, "ClearData!\n");
  ec = CAEN_FELib_SendCommand(devHandle, "/cmd/cleardata");
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : fail SendCommand. ec : %d\n", ec);
    return;
  }  
  fprintf(stdout, "DAQ start!\n");
  ec = CAEN_FELib_SendCommand(devHandle, "/cmd/armacquisition");
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : fail SendCommand. ec : %d\n", ec);
    return;
  }
  ec = CAEN_FELib_SendCommand(devHandle, "/cmd/swstartacquisition");
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : fail SendCommand. ec : %d\n", ec);
    return;
  }

  Nnowaveform = 0;    
}

void stop(void){
  printf("Stop\n");

  //DAQ end
  int ec;
  char value[256];

  GetValueFromCh(devHandle, "ChRealtimeMonitor", value, 0);
  GetValueFromCh(devHandle, "ChRealtimeMonitor", value, 4);
  
  GetValueFromCh(devHandle, "ChTriggerCnt",      value, 0);
  fprintf(stdout, "ch=0 ChTriggerCnt before swstop: %s\n", value);    
  GetValueFromCh(devHandle, "ChTriggerCnt",      value, 4);  
  fprintf(stdout, "ch=1 ChTriggerCnt before swstop: %s\n", value);

  GetValueFromCh(devHandle, "ChSavedEventCnt",   value, 0);
  fprintf(stdout, "ch=0 ChSavedEventCnt before swstop: %s\n", value);      
  GetValueFromCh(devHandle, "ChSavedEventCnt",   value, 4);  
  fprintf(stdout, "ch=1 ChSavedEventCnt before swstop: %s\n", value);
  
  GetValueFromCh(devHandle, "ChWaveCnt",         value, 0);
  fprintf(stdout, "ch=0 ChWaveCnt before swstop: %s\n", value);      
  GetValueFromCh(devHandle, "ChWaveCnt",         value, 4);  
  fprintf(stdout, "ch=1 ChWaveCnt before swstop: %s\n", value);
  
  ec = CAEN_FELib_SendCommand(devHandle, "/cmd/swstopacquisition");
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : fail SendCommand. ec : %d\n", ec);
    return;
  }
  ec = CAEN_FELib_SendCommand(devHandle, "/cmd/disarmacquisition");
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : fail SendCommand. ec : %d\n", ec);
    return;
  }
  //
  fprintf(stdout, "Nnowaveform = %d\n", Nnowaveform);  
  fprintf(stdout, "DAQ end!\n");    
}

void reload(void){
  printf("Reload\n");
}

void sca(void){
  int i;
  //  printf("Sca\n");
  for(i=0;i<32;i++){
    scrbuff[i] = scrbuff[i] + 1;
  }
  babies_init_ncscaler(efn);
  babies_scrdata((char *)scrbuff, sizeof(scrbuff));
  babies_end_ncscaler32();
}

void clear(void){
  // write clear function i.e. send end-of-busy pulse
  //  printf("Clear\n");
}

// thread
void evtloop(void){
  int status;
  struct RawEvent *rawEvt = NULL;
  int ec;
  char value[256];

  fprintf(stdout, "evtloop start!\n");    

  //  ec = CAEN_FELib_Open("dig2://usb:21186", &devHandle);
  fprintf(stdout, "open the connection with VX2740B.\n");
  ec = CAEN_FELib_Open("dig2://vx2740", &devHandle);  
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : cannot open connection with device. ec : %d\n", ec);
    return;
  }

  fprintf(stdout, "reset!\n");
  ec = CAEN_FELib_SendCommand(devHandle, "/cmd/reset");
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : fail SendCommand. ec : %d\n", ec);
    return;
  }      
  
  uint64_t endPointHandle;
  char temp[256];
  sprintf(temp, "/endpoint/%s", endpoint);
  ec = CAEN_FELib_GetHandle(devHandle, temp, &endPointHandle);  
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : cannot retrieve EndPointHandle. ec : %d\n", ec);
    return;
  }  

  //allocate RawEvent and tdata
  fprintf(stdout, "Allocate Event\n");  
  rawEvt = malloc(sizeof(struct RawEvent));
  int maxRawDataSize;
  ec = CAEN_FELib_GetValue(devHandle, "/par/MaxRawDataSize", value);
  maxRawDataSize = atoi(value);
  fprintf(stdout, "MaxRawDataSize = %d\n", maxRawDataSize);
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : cannot GetValue. ec : %d\n", ec);
    return;
  }
  rawEvt->data = malloc(maxRawDataSize*sizeof(uint8_t));
  //v2740

  InitV2740();  

  while((status = babies_status()) != -1){
    switch(status){
    case STAT_RUN_IDLE:
      /* noop */
      break;
    case STAT_RUN_START:
    case STAT_RUN_NSSTA:
      usleep(usl); // wait event
      
//      babies_init_event();
//      //babies_init_segment(segid)
//      //MKSEGID(device, focalplane, detector, module)
//      //please see segidlist.h
//      //module is important, e.g. C16 is 16bit data
//      //device, focalplane, detector, can be defined as you want
//      babies_init_segment(MKSEGID(0, 0, 0, V2740));
      
      ec = CAEN_FELib_ReadData(endPointHandle,
//			       -1,
			       1000,
			       rawEvt->data,
			       &(rawEvt->size),
			       &(rawEvt->n_events)	
			       );

//      if(rawEvt->size == 32){
//	fprintf(stdout, "Start Run Event\n");
//	for(int i=0;i<rawEvt->size;i++){
//	  fprintf(stdout, "%u ", rawEvt->data[i]);
//	}
//	fprintf(stdout, "\n");
//	continue;
//      }
//      else if(ec == CAEN_FELib_Timeout){
      if(ec == CAEN_FELib_Timeout){      
	fprintf(stderr, "<evtloop()> ERROR : CAEN_FELib_ReadData() is timeouted.\n");
	continue;
      }
	babies_init_event();
	babies_init_segment(MKSEGID(0, 0, 0, V2740));
	babies_segdata((char *)(rawEvt->data), rawEvt->size*sizeof(uint8_t));
	babies_end_segment();
	babies_end_event();	      

//      size_t evtSize = 0x0000000000000000;      
//      evtSize     |= (rawEvt->data[7]);
//      evtSize     |= (rawEvt->data[6]<<8);
//      evtSize *= 8;
//      
//      if(rawEvt->size%evtSize){
//	fprintf(stderr, "ERROR : rawEvt->size is NOT divided by evtSize\n");
//	continue;
//      }
//      for(int ibuf=0;ibuf<rawEvt->size/evtSize;ibuf++){
//	babies_init_event();
//	babies_init_segment(MKSEGID(0, 0, 0, V2740));
//	babies_segdata((char *)(rawEvt->data + ibuf*evtSize), evtSize*sizeof(uint8_t));
//	babies_end_segment();
//	babies_end_event();	
//      }


      // babies_chk_block(int maxbuff)
      // if block size is larger than maxbuff,
      //  data should be send to the event builder
      //  i.e., read scaler and flush
      // example : 8000 = 16kB
      if(babies_chk_block(32000)){
      //      if(babies_chk_block(2000000)){      
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
  fprintf(stdout, "free allocated memeories\n");
  free(rawEvt->data);
  free(rawEvt);
  //  free(tdata);
  
  //disconnect
  ec = CAEN_FELib_Close(devHandle);
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : cannot close connection with device. ec : %d\n", ec);
    return;
  }
  //

  fprintf(stdout, "evtloop stop!\n");  
}

int InitV2740(void){
  int ec; //CAEN_FELib_ErrorCode;
  char value[256];
  //  uint64_t devHandle;

  //connect
  fprintf(stdout, "Init v2740\n");
//  ec = CAEN_FELib_Open("dig2://usb:21186", &devHandle);
//  if(ec != CAEN_FELib_Success){
//    fprintf(stderr, "ERROR : cannot open connection with device. ec : %d\n", ec);
//    return ec;
//  }
  //
  GetValueFromDeg(devHandle, "ModelName",            value); fprintf(stdout, "ModelName            : %s\n", value);  
  GetValueFromDeg(devHandle, "FPGA_FwVer",           value); fprintf(stdout, "FPGA_FwVer           : %s\n", value);
  GetValueFromDeg(devHandle, "FwType",               value); fprintf(stdout, "FwType               : %s\n", value);
  GetValueFromDeg(devHandle, "License",              value); fprintf(stdout, "License              : %s\n", value);
  GetValueFromDeg(devHandle, "LicenseStatus",        value); fprintf(stdout, "LicenseStatus        : %s\n", value);  
  GetValueFromDeg(devHandle, "LicenseRemainingTime", value); fprintf(stdout, "LicenseRemainingTime : %s\n", value);
  GetValueFromDeg(devHandle, "ITLAGateWidth",        value); fprintf(stdout, "ITLAGateWidth        : %s\n", value);
  GetValueFromDeg(devHandle, "ITLBGateWidth",        value); fprintf(stdout, "ITLBGateWidth        : %s\n", value);    

  //Enabled ch parameters settings
  int chEnabled[] = {1};  
  //  int chEnabled[] = {0, 1};
  //  int chEnabled[] = {4, 8};  
  int sizeChEnabled = sizeof(chEnabled)/sizeof(chEnabled[0]);
  fprintf(stdout, "sizeChEnabled = %d\n", sizeChEnabled);
  //  int *chDisabled = (int*)malloc(sizeof(int)*(64-sizeChEnabled));
  int chDisabled[64];
  int idx = 0;

  uint64_t itlaMask = 0x0000000000000000, itlbMask;
  for(int i=0;i<64;i++){
    int flag = 1;
    for(int j=0;j<sizeChEnabled;j++){
      if(i==chEnabled[j]){
	flag = 0;
      }
    }
    if(flag){
      chDisabled[idx] = i;
      idx++;
    }
    else{
      itlaMask |= (0x0000000000000001<<i);
    }
  }
  itlbMask = itlaMask;
  
  
  char strITLAMask[256], strITLBMask[256];
  snprintf(strITLAMask, sizeof(strITLAMask), "0x%"PRIx64"", itlaMask);
  snprintf(strITLBMask, sizeof(strITLBMask), "0x%"PRIx64"", itlbMask);  
  fprintf(stdout, "StrITLAMask: %s\n", strITLAMask);
  fprintf(stdout, "StrITLBMask: %s\n", strITLBMask);  

  fprintf(stdout, "Enabled channels:");
  for(int i=0;i<sizeChEnabled;i++){    
    fprintf(stdout, "%d ", chEnabled[i]);
  }
  fprintf(stdout, "\n");

  fprintf(stdout, "Disabled channels:");
  for(int i=0;i<64-sizeChEnabled;i++){    
    fprintf(stdout, "%d ", chDisabled[i]);
  }
  fprintf(stdout, "\n");
  
  //Disabled ch parameters settings
  for(int i=0;i<64-sizeChEnabled;i++){
    SetValue2Ch(devHandle, "ChEnable",                   "False",                   chDisabled[i]);  
  }
  //      

  //digitizer parameters settings
  SetValue2Deg (devHandle, "IOLevel",                     "NIM");
  SetValue2Deg (devHandle, "StartSource",                 "SWcmd");
  SetValue2Deg (devHandle, "EnStatEvents",                "False");  
  //  SetValue2Deg (devHandle, "GlobalTriggerSource",         "SwTrg|ITLA");
  SetValue2Deg (devHandle, "ITLAMask"   ,                 strITLAMask);
  SetValue2Deg (devHandle, "ITLAPairLogic",               "OR"                );
  SetValue2Deg (devHandle, "ITLAMainLogic",               "OR");
  SetValue2Deg (devHandle, "ITLAGateWidth",               "0");
  //  SetValue2Deg (devHandle, "ITLAPolarity",                "Inverted");      
  SetValue2Deg (devHandle, "ITLBMask"   ,                 strITLBMask);  
  SetValue2Deg (devHandle, "ITLBPairLogic",               "OR"                );
  SetValue2Deg (devHandle, "ITLBMainLogic",               "OR");
  SetValue2Deg (devHandle, "ITLBGateWidth",               "0");
  //  SetValue2Deg (devHandle, "ITLBPolarity",                "Inverted");        
  SetValue2Deg (devHandle, "ClockSource",                 "Internal");
  SetValue2Deg (devHandle, "EnClockOutFP",                "True");
  SetValue2Deg (devHandle, "BusyInSource",                "Disabled");
  SetValue2Deg (devHandle, "SyncOutMode",                 "IntClk");
  SetValue2Deg (devHandle, "TrgOutMode",                  "ITLA");
  //  SetValue2Deg (devHandle, "GPIOMode",                    "ITLB");    
  SetValue2Deg (devHandle, "GPIOMode",                    "Busy");
  SetValue2LVDS(devHandle, "LVDSMode",                    "SelfTriggers", "0");
  SetValue2LVDS(devHandle, "LVDSDirection",               "Output",       "0");
  SetValue2Deg (devHandle, "LVDSTrgMask",                 "0=0x0000000000000001");
  SetValue2Deg (devHandle, "LVDSTrgMask",                 "1=0x0000000000000002");
  SetValue2Deg (devHandle, "LVDSTrgMask",                 "2=0x0000000000000003");        
  SetValue2LVDS(devHandle, "LVDSMode",                    "SelfTriggers", "1");
  SetValue2LVDS(devHandle, "LVDSDirection",               "Output",       "1");
  SetValue2LVDS(devHandle, "LVDSMode",                    "SelfTriggers", "2");
  SetValue2LVDS(devHandle, "LVDSDirection",               "Output",       "2");
  SetValue2LVDS(devHandle, "LVDSMode",                    "SelfTriggers", "3");
  SetValue2LVDS(devHandle, "LVDSDirection",               "Output",       "3");  
  SetValue2Deg (devHandle, "TestPulsePeriod",             "2000");
  SetValue2Deg (devHandle, "TestPulseWidth",              "500");
  SetValue2Deg (devHandle, "TestPulseLowLevel",           "0");
  SetValue2Deg (devHandle, "TestPulseHighLevel",          "32768");

  for(int i=0;i<sizeChEnabled;i++){
    SetValue2Ch(devHandle, "ChEnable",                    "True",                   chEnabled[i]);    
    //    SetValue2Ch(devHandle, "WaveTriggerSource",           "GlobalTriggerSource",    chEnabled[i]);
    //    SetValue2Ch(devHandle, "EventTriggerSource",          "GlobalTriggerSource",    chEnabled[i]);
//    SetValue2Ch(devHandle, "WaveTriggerSource",           "ChSelfTrigger",    chEnabled[i]);
//    SetValue2Ch(devHandle, "EventTriggerSource",          "ChSelfTrigger",    chEnabled[i]);
    SetValue2Ch(devHandle, "WaveTriggerSource",           "TRGIN",                  chEnabled[i]);
    SetValue2Ch(devHandle, "EventTriggerSource",          "TRGIN",                  chEnabled[i]);
    //    SetValue2Ch(devHandle, "SelfTriggerWidth",            "24",                    chEnabled[i]);            
    SetValue2Ch(devHandle, "DCOffset",                    "20",                     chEnabled[i]);
    SetValue2Ch(devHandle, "TriggerThr",                  "500",                    chEnabled[i]);
    SetValue2Ch(devHandle, "PulsePolarity",               "Negative",               chEnabled[i]);
    SetValue2Ch(devHandle, "WaveDataSource",              "ADC_DATA",               chEnabled[i]);
    SetValue2Ch(devHandle, "ChRecordLengthS",             "1875",                   chEnabled[i]);    
    SetValue2Ch(devHandle, "WaveResolution",              "Res8",                   chEnabled[i]);
    SetValue2Ch(devHandle, "ChPreTriggerS",               "625",                    chEnabled[i]);
    SetValue2Ch(devHandle, "WaveSaving",                  "Always",                 chEnabled[i]);
    SetValue2Ch(devHandle, "WaveAnalogProbe0",            "ADCInput",               chEnabled[i]);
    SetValue2Ch(devHandle, "WaveAnalogProbe1",            "TimeFilter",             chEnabled[i]);
    SetValue2Ch(devHandle, "WaveDigitalProbe0",           "Trigger",                chEnabled[i]);
    SetValue2Ch(devHandle, "WaveDigitalProbe1",           "TimeFilterArmed",        chEnabled[i]);
    SetValue2Ch(devHandle, "WaveDigitalProbe2",           "RetriggerGuard",         chEnabled[i]);
    SetValue2Ch(devHandle, "WaveDigitalProbe3",           "ADCSaturation",          chEnabled[i]);
    SetValue2Ch(devHandle, "TimeFilterRiseTimeS",         "25",                    chEnabled[i]);
    SetValue2Ch(devHandle, "TimeFilterRetriggerGuardS",   "0",                      chEnabled[i]);
    SetValue2Ch(devHandle, "EnergyFilterRiseTimeS",       "25",                    chEnabled[i]);
    SetValue2Ch(devHandle, "EnergyFilterFlatTopS",        "125",                    chEnabled[i]);
    SetValue2Ch(devHandle, "EnergyFilterPeakingPosition", "50",                     chEnabled[i]);
    SetValue2Ch(devHandle, "EnergyFilterPeakingAvg",      "OneShot",                chEnabled[i]);
    SetValue2Ch(devHandle, "EnergyFilterPoleZeroS",       "6250",                   chEnabled[i]);
    SetValue2Ch(devHandle, "EnergyFilterFineGain",        "1",                      chEnabled[i]);  
  }  
  //  

  //DAQ start setting
  ec = CAEN_FELib_SetValue(devHandle, "/endpoint/par/activeendpoint", endpoint);
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : cannot SetValue. ec hogehoge: %d\n", ec);
    return ec;
  }

  char dataFormat[10000] = " \
	[ \
		{ \"name\" : \"DATA\", \"type\" : \"U8\", \"dim\" : 1 }, \
		{ \"name\" : \"SIZE\", \"type\" : \"SIZE_T\" }, \
		{ \"name\" : \"N_EVENTS\", \"type\" : \"U32\" } \
	] \
";  
  uint64_t endPointHandle;
  char temp[256];
  sprintf(temp, "/endpoint/%s", endpoint);
  ec = CAEN_FELib_GetHandle(devHandle, temp, &endPointHandle);
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : cannot retrieve EndPointHandle. ec : %d\n", ec);
    return EXIT_FAILURE;
  }
  ec = CAEN_FELib_SetReadDataFormat(endPointHandle, dataFormat);
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "ERROR : cannot SetReadDataFormat. ec : %d\n", ec);
    return EXIT_FAILURE;
  }

  GetValueFromCh(devHandle, "SelfTriggerWidth",  value, 1);  
  fprintf(stdout, "ch=1 SelfTriggerWidth : %s\n", value);
  GetValueFromCh(devHandle, "TimeFilterRetriggerGuardS",  value, 1);  
  fprintf(stdout, "ch=1 TimeFilterRetriggerGuardS : %s\n", value);            

  return EXIT_SUCCESS;
}

int SetValue2Ch(uint64_t devHand, char *paraNameRel, char *value, int ch){
  int ec;
  char paraNameAbs[256];
  snprintf(paraNameAbs, sizeof(paraNameAbs), "/ch/%d/par/%s", ch, paraNameRel);
  ec = CAEN_FELib_SetValue(devHand, paraNameAbs, value);
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "<SetValue2Ch()> ERROR : cannot SetValue '%s' to parameter '%s'. ec : %d\n", value, paraNameAbs, ec);
    return ec;
  }

  fprintf(stdout, "%-40s <-- %s\n", paraNameAbs, value);
  
  return ec;
}
int SetValue2Deg(uint64_t devHand, char *paraNameRel, char *value){
  int ec;
  char paraNameAbs[256];
  snprintf(paraNameAbs, sizeof(paraNameAbs), "/par/%s", paraNameRel);
  ec = CAEN_FELib_SetValue(devHand, paraNameAbs, value);
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "<SetValue2Deg()> ERROR : cannot SetValue '%s' to parameter '%s'. ec : %d\n", value, paraNameAbs, ec);
    return ec;
  }

  fprintf(stdout, "%-40s <-- %s\n", paraNameAbs, value);
  
  return ec;
}
int SetValue2LVDS(uint64_t devHand, char *paraNameRel, char *value, char *index){
  int ec;
  char paraNameAbs[256];
  snprintf(paraNameAbs, sizeof(paraNameAbs), "/lvds/%s/par/%s", index, paraNameRel);
  ec = CAEN_FELib_SetValue(devHand, paraNameAbs, value);
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "<SetValue2Deg()> ERROR : cannot SetValue '%s' to parameter '%s'. ec : %d\n", value, paraNameAbs, ec);
    return ec;
  }

  fprintf(stdout, "%-40s <-- %s\n", paraNameAbs, value);
  
  return ec;
}

int GetValueFromCh (uint64_t devHand, char *paraNameRel, char *value, int ch){
  int ec;
  char paraNameAbs[256];
  snprintf(paraNameAbs, sizeof(paraNameAbs), "/ch/%d/par/%s", ch, paraNameRel);
  ec = CAEN_FELib_GetValue(devHand, paraNameAbs, value);
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "<GetValueFromCh()> ERROR : cannot GetValue from parameter '%s'. ec : %d\n", paraNameAbs, ec);
    return ec;
  }  
  return ec;  
}
int GetValueFromDeg(uint64_t devHand, char *paraNameRel, char *value){
  int ec;
  char paraNameAbs[256];
  snprintf(paraNameAbs, sizeof(paraNameAbs), "/par/%s", paraNameRel);
  ec = CAEN_FELib_GetValue(devHand, paraNameAbs, value);
  if(ec != CAEN_FELib_Success){
    fprintf(stderr, "<GetValueFromDeg()> ERROR : cannot GetValue from parameter '%s'. ec : %d\n", paraNameAbs, ec);
    return ec;
  }
  return ec;    
}

int main(int argc, char *argv[]){

  if(argc < 2){
    printf("csample EFN [uleep]\n");
    exit(0);
  }else{
    efn = strtol(argv[1], NULL, 0);
  }

  if(argc == 3){
    usl = strtol(argv[2], NULL, 0);
  }

  babies_quit(quit);
  babies_start(start);
  babies_stop(stop);
  babies_reload(reload);
  babies_evtloop(evtloop);
  babies_name("csample");

  babies_init(efn);
  babies_check();

  babies_main();

  return 0;
}
