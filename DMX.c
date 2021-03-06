/*
 * File:   DMX.c
 * Author: METEOR
 *
 * Created on May 23, 2017, 11:30 AM
 */


#include <xc.h>
#include "DMX.h"
#include "PWM.h"
#include "RDM.h"
#include "Timer.h"
#include <stdint.h>
#include "rdm_define.h"
//16bit
void DMX_init(void) {
    // DMX UART START
    RXPPS = 0b10101; //RX=RC5
    TRISC5 = 1; //set RC5 as input

    RCSTA = 0b10010000; //enable RX  ; 8bit
    SYNC = 0; // UART Enable
    BAUDCON = 0b00000000; //BRG16 =0
    BRGH = 1; //High Buad Speed
    SPBRGH = 0x00;
    SPBRGL = 0x7; //  32M/(16*(SPBRG+1)) = 250k
    RCIE = 1; //Enable RC interrupt
    //    RxArPtr = &RxAr[0];
    // DMX UART END
    //    RC4PPS=0b1001; //RC4=TX
    //    TXSTA=0b01000101;//disable TX,

//    DMX_Address = 1;
    PWMDCLptr[0] = &PWM1DCL;
    PWMDCLptr[1] = &PWM2DCL;
    PWMDCLptr[2] = &PWM3DCL;
    PWMDCLptr[3] = &PWM4DCL;
    PWMDCHptr[0] = &PWM1DCH;
    PWMDCHptr[1] = &PWM2DCH;
    PWMDCHptr[2] = &PWM3DCH;
    PWMDCHptr[3] = &PWM4DCH;
    PWMLDCONptr[0] = &PWM1LDCON;
    PWMLDCONptr[1] = &PWM2LDCON;
    PWMLDCONptr[2] = &PWM3LDCON;
    PWMLDCONptr[3] = &PWM4LDCON;
            
//    PWMDCptr[0]=&PWM1DC;
//    PWMDCptr[1]=&PWM1DC;
//    PWMDCptr[2]=&PWM1DC;
//    PWMDCptr[3]=&PWM1DC;
    
//    PWM_Pin[0]=RA2;
//    PWM_Pin[1]=RC0;
//    PWM_Pin[2]=RC1;
//    PWM_Pin[3]=RC2;
    
    Addr=0;
    while(Addr<RX_BUFFER_SIZE){
        DMX_sumRepeat[Addr]=2;
        DMX_Repeat[Addr][0]=DMX_Repeat[Addr][1]=1;
        DMXSign[Addr].InfiniteLoop=0;
        DMX_TargetBright[Addr]=DMX_CurrentBright[Addr]=0.0;
        Addr++;
    }
}
void DMX_loop(void) {
    
    //DMX Renew
    if (DMX_Flags.RxNew == 1) {
        DMX_Flags.RxNew = 0;
        ////
        Addr = 0;
//        if(PERSONALITY<4){
//            while (Addr < FOOTPRINT) {
//                *PWMDCHptr[Addr] = PWM[RxData[Addr]].DCH;
//                *PWMDCLptr[Addr] = PWM[RxData[Addr]].DCL;
//                *PWMLDCONptr[Addr] = 0b10000000;
//                Addr++;
//            }
//        }else{
////            while (Addr < FOOTPRINT) {
////                rxdata=RxData[Addr];    //LOG
////                tmp8=RxData[Addr+1];    //CCT
////                CurrentPWM.DC[Addr]=PWM[rxdata].DC*(255.0-tmp8)/255.0; //interpolation
////                *PWMDCHptr[Addr] = CurrentPWM.PWM[Addr].DCH;
////                *PWMDCLptr[Addr] = CurrentPWM.PWM[Addr].DCL;
////                *PWMLDCONptr[Addr] = 0b10000000;
////                Addr++;
////                DMX_CurrentBright[Addr]=PWM[rxdata]*(tmp8)/255.0;
//////                CurrentPWM.DC[Addr]=PWM.DC[rxdata]+(PWM.DC[rxdata+1]-PWM.DC[rxdata])*(DMX_CurrentBright[Addr]-rxdata); //interpolation
////                CurrentPWM.DC[Addr]=PWM[rxdata].DC*(tmp8)/255.0; //interpolation
////                *PWMDCHptr[Addr] = CurrentPWM.PWM[Addr].DCH;
////                *PWMDCLptr[Addr] = CurrentPWM.PWM[Addr].DCL;
////                *PWMLDCONptr[Addr] = 0b10000000;
////                Addr++;
////            }
//        }
        
            //////////
        DMXDimming=0;
        DMXStepConst=DMXStep;
        if(PERSONALITY<5){
            while (Addr < FOOTPRINT) {
                rxdata=RxData[Addr];  //avoid violatile to acculate.

                if (DMX_TargetBright[Addr] < rxdata) {
                    DMX_difference=rxdata-DMX_TargetBright[Addr];
                    if(DMX_difference<0.05){
                        DMXSign[Addr].SIGN = 0b00;//increase
                        DMX_CurrentBright[Addr]=DMX_TargetBright[Addr]=rxdata;
                    }else{
                        DMXSign[Addr].SIGN = 0b01;//increase
                        DMX_CurrentBright[Addr]=DMX_TargetBright[Addr];
                        DMX_TargetBright[Addr]=DMX_TargetBright[Addr]+(DMX_difference*DMX_repeatcoeff[Addr]/DMX_sumRepeat[Addr]);
                        DMX_SpaceBright[Addr]=(DMX_TargetBright[Addr]-DMX_CurrentBright[Addr])/(DMXPeriod>>DMX_Shift_bits);
                    }
                } else if (DMX_TargetBright[Addr] > rxdata) {
                    DMX_difference=DMX_TargetBright[Addr]-rxdata;
                    if(DMX_difference<0.05){
                        DMXSign[Addr].SIGN = 0b00;//increase
                        DMX_CurrentBright[Addr]=DMX_TargetBright[Addr]=rxdata;
                    }
                    else{
                        DMXSign[Addr].SIGN = 0b10;//decrease
                        DMX_CurrentBright[Addr]=DMX_TargetBright[Addr];
                        DMX_TargetBright[Addr]=DMX_TargetBright[Addr]-(DMX_difference*DMX_repeatcoeff[Addr]/DMX_sumRepeat[Addr]);
                        DMX_SpaceBright[Addr]=(DMX_CurrentBright[Addr]-DMX_TargetBright[Addr])/(DMXPeriod>>DMX_Shift_bits);
                    }
                } else {    //unchange
                    DMX_CurrentBright[Addr]=DMX_TargetBright[Addr]=rxdata;
                    DMXSign[Addr].SIGN = 0b00;
                }

                if(preRxData[Addr]==RxData[Addr] && !DMXSign[Addr].InfiniteLoop){
                    DMX_Repeat[Addr][0]++;
                    if(DMX_Repeat[Addr][0]==0x10){
                        DMX_Repeat[Addr][0]=DMX_Repeat[Addr][1]=1;
                        DMX_sumRepeat[Addr]=2;
                        DMXSign[Addr].InfiniteLoop=1;
                        DMX_repeatcoeff[Addr]=2;
                    }
                    DMX_repeat_hereditary[Addr]=DMX_repeat_hereditary[Addr]/2+DMX_Repeat[Addr][0];
                }else{
                    DMX_sumRepeat[Addr]=DMX_Repeat[Addr][0]+DMX_Repeat[Addr][1];
                    DMX_Repeat[Addr][1]=DMX_Repeat[Addr][0];
                    DMX_repeat_hereditary[Addr]=DMX_repeat_hereditary[Addr]/2+DMX_Repeat[Addr][0];
                    DMX_Repeat[Addr][0]=1;
                    DMXSign[Addr].InfiniteLoop=0;
                    if(DMX_repeat_hereditary[Addr]>2.1){
                        DMX_repeatcoeff[Addr]=1;
                    }else{
                        DMX_repeatcoeff[Addr]=2;
                    }
                }
                preRxData[Addr]=RxData[Addr];
                Addr++;
            }
        }
    }
    //DMX didn't get Signal
    if (Timer.MS ) {
        DMXPeriodConst++;
        Timer.MS = 0;
        Addr = 0;
        DMXDimming++;
        if(DMXStepConst==0 && DMXDimming<DMXPeriod){
            DMXStepConst=DMXStep;
            while (Addr < FOOTPRINT) {
                switch (DMXSign[Addr].SIGN) {
                    case 0b01:
                        DMX_CurrentBright[Addr]=DMX_CurrentBright[Addr]+DMX_SpaceBright[Addr];
                        if(DMX_CurrentBright[Addr]>255){
                            DMX_CurrentBright[Addr]=255;
                        }
                        break;
                    case 0b10:
                        DMX_CurrentBright[Addr]=DMX_CurrentBright[Addr]-DMX_SpaceBright[Addr];
                        if(DMX_CurrentBright[Addr]<0){
                            DMX_CurrentBright[Addr]=0;
                        }
                        break;
                    case 0b00:
                        break;
                }
                rxdata=DMX_CurrentBright[Addr];
                
                CurrentPWM.DC[Addr]=PWM[rxdata].DC+(PWM[rxdata+1].DC-PWM[rxdata].DC)*(DMX_CurrentBright[Addr]-rxdata); //interpolation
                if(!DMX_Flags.RDM_Identify_Device){ //if !RDM_Identify
                    *PWMDCHptr[Addr] = CurrentPWM.PWM[Addr].DCH;
                    *PWMDCLptr[Addr] = CurrentPWM.PWM[Addr].DCL;
                    *PWMLDCONptr[Addr] = 0b10000000;
                }
                Addr++;
            }
        }else{
            DMXStepConst--;
        }
        
        // If no data received for 1200ms turn the lights off
//        if (DMX_Flags.RxTimeout == 1) {
//            PWM1DC = PWM2DC = PWM3DC = PWM4DC = 0;
//            PWM1LDCON = PWM2LDCON = PWM3LDCON = PWM4LDCON = 0b10000000;
//            Addr = 0;
//            while (Addr < FOOTPRINT) {
//                DMXSign[Addr].SIGN = 0b00;
//                CurrentPWM.DC[Addr]=0;
//                DMX_TargetBright[Addr]=DMX_CurrentBright[Addr]=0.0;
//                DMX_Repeat[Addr][0]=DMX_Repeat[Addr][1]=1;
//                DMX_sumRepeat[Addr]=2;
//                DMXSign[Addr].InfiniteLoop=1;
//                Addr++;
//            }
//        }
    }
}

void DMX_interrput(void) {
    if (RCIE & RCIF) {
        volatile char RxDat;
        if (FERR) // if get error bit, clear the bit ;  occur at space for "break"
        {
            RxDat = RCREG; // Clear the Framing Error - do not read before the bit test
            DMX_Flags.RxBreak = 1; // Indicate a break
            RxState = WAIT_FOR_START;
            RxTimer = 0;
        }
        switch (RxState) {
            case RX_WAIT_FOR_BREAK:
                RxDat = RCREG; // Just keep clearing the buffer until overflow.
                break;
            case WAIT_FOR_START:
                if (RCIF) // make sure there is data avaliable (ie not a break)
                {
                    RxDat = RCREG;
                    if (RxDat == DMX_StartCode) { // DMX_StartCode == 00;
                        // Valid Start Received
                        RxState = RX_DMX_READ_DATA;
                        RxDataPtr = &RxData[0]; // Point to Buffer
                        RxAddrCount = 1; // Reset current addr - Start at address 1! (zero is OFF)
//                        DMX_Flags.RxStart = 1; // Indicate a Start
                        DMXPeriod = DMXPeriodConst; // DMX period Record;
                        DMXPeriodConst = 0; // DMX period reset;
                    } else if (RxDat == E120_SC_RDM) { // RDM_StartCode == 0xCC;
                        // Valid Start Received
                        RxState = RX_RDM_READ_SubStartCode;
//                        DMX_Flags.RxStart = 1; // Indicate a Start
                    } else {
                        RxState = RX_WAIT_FOR_BREAK;
                    }
                }
                break;
            case RX_DMX_READ_DATA:
                RxDat = RCREG;
                if (RxAddrCount >= DMX_Address && (DMX_Address != 0)) // A selection of channel zero is "OFF"
                {
                    *RxDataPtr = RxDat;
                    RxDataPtr++;
                }
                RxAddrCount++;
                // Check for end of data packet we can read
                if (RxAddrCount >= (DMX_Address + RX_BUFFER_SIZE) && DMX_Address != 0) {
                    DMX_Flags.RxNew = 1;
                    RxState = RX_WAIT_FOR_BREAK;
                    RxTimer = 0;
                    DMX_Flags.RxTimeout = 0;
                }
                break;
            case RX_RDM_READ_SubStartCode:
                RxDat = RCREG;
                if (RxDat == E120_SC_SUB_MESSAGE) // RDM_SubStartCode == 0x01;
                {
                    RxState = RX_RDM_READ_DATA;
                    ReadCount = 23;
                    PD_Flag = 0;
                }
                break;

            case RX_RDM_READ_DATA:
                RxDat = RCREG;
                RX_RDM_Data.value[ReadCount] = RxDat;

                if (ReadCount == 2 && RX_RDM_Data.value[ReadCount] > 0 && PD_Flag == 0) {
                    RxState = RX_RDM_PD;
                    tmp8 = PD_LEN-1;
                    ReadCount--;
                    break;
                }
                if (ReadCount == 0) {
                    DMX_Flags.RDMNew = 1;
                    RxState = RX_WAIT_FOR_BREAK;
                    RxTimer = 0;
                    DMX_Flags.RxTimeout = 0;
                }

                ReadCount--;
                break;
            case RX_RDM_PD:
                RxDat = RCREG;
                PD.u8[tmp8] = RxDat;
                if ((PD_LEN - RX_RDM_Data.PDL) == tmp8) {
                    RxState = RX_RDM_READ_DATA;
                    PD_Flag = 1;
                }else{
                    tmp8--;
                }
                break;
        }
        
        if (RxTimer > DMX_RX_TIMEOUT_MS) {
            DMX_Flags.RxTimeout = 1;
            RxTimer = 0;
        }
    }
}


void PWM_TurnOn(void) {
    Addr=0;
    while (Addr < FOOTPRINT) {
        *PWMDCHptr[Addr] = PWM[IDENTIFY_MODE].DCH;
        *PWMDCLptr[Addr] = PWM[IDENTIFY_MODE].DCL;
        *PWMLDCONptr[Addr] = 0b10000000;
        Addr++;
    }
}
void PWM_TurnOff(void) {
    Addr=0;
    while (Addr < RX_BUFFER_SIZE) {
        *PWMDCHptr[Addr] = 0x00;
        *PWMDCLptr[Addr] = 0x00;
        *PWMLDCONptr[Addr] = 0b10000000;
        Addr++;
    }
}