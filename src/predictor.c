//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Ruicheng Li";
const char *studentID   = "A59001918";
const char *email       = "rul004@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

int history;
int g_table[1 << 15];

//------------------------------------//
//        Auxiliary Functions         //
//------------------------------------//
// Make prediction based on the store value in the 2-bit table
// 0 - N, 1 - WN, 2 - WT, 3 - T
int select_pred(int value){
  switch (value)
  {
  case 0:
    return NOTTAKEN;
  case 1:
    return NOTTAKEN;
  case 2:
    return TAKEN;
  case 3:
    return TAKEN;
  default:
    return TAKEN;
  }
}

// update table based on the outcome
// 0 - N, 1 - WN, 2 - WT, 3 - T
void update_table(int index, uint8_t outcome){
  int value = g_table[index];
  if(value == 0 & outcome == NOTTAKEN){
    return;
  }
  else if(value == 0 & outcome == TAKEN){
    g_table[index] = 1;
  }
  else if(value == 1 & outcome == NOTTAKEN){
    g_table[index] = 0;
  }
  else if(value == 1& outcome == TAKEN){
    g_table[index] = 2;
  }
  else if(value == 2 & outcome == NOTTAKEN){
    g_table[index] = 1;
  }
  else if(value == 2 & outcome == TAKEN){
    g_table[index] = 3;
  }
  else if(value == 3 & outcome == NOTTAKEN){
    g_table[index] = 2;
  }
  return;
}

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//
// Initialize the predictor
void
init_predictor()
{
  
  int i;
  int table_size = 1 << ghistoryBits;
  history = (1 << ghistoryBits) - 1;
  for(i = 0; i < table_size; i++){
      g_table[i] = 0;
  }



}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  int index;
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      index = (pc ^ history) & ((1 << ghistoryBits) - 1);
      return select_pred(g_table[index]);
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  // updaet table
  int index = (pc ^ history) & ((1 << ghistoryBits) - 1);
  update_table(index, outcome);

  // update history
  if(outcome == 1){
    history = ((history << 1)  + 1) & ((1 << ghistoryBits) - 1);
  }
  else{
    history = (history << 1) & ((1 << ghistoryBits) - 1);
  }
}
