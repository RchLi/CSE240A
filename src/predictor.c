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

int ghistoryBits = 9; // Number of bits used for Global History
int lhistoryBits = 10; // Number of bits used for Local History
int pcIndexBits = 10;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

int ghistory;
int g_table[1 << 15];
int l_table[1 << 15];
int l_pht[1 << 15];
int chooser[1 << 15];
int gmask;
int lmask;
int pcmask;
int hist_array[1 << 10];
int weight[1 << 10];
int bias;
int p_size = 1 << 10;
int thresh = 25;

//------------------------------------//
//        Auxiliary Functions         //
//------------------------------------//
// Make prediction based on the store value in the 2-bit table
// 0 - N, 1 - WN, 2 - WT, 3 - T
int select_pred(int value){
  if(value < 2){
    return NOTTAKEN;
  }
  else{
    return TAKEN;
  }
}

// update table based on the outcome
// 0 - N, 1 - WN, 2 - WT, 3 - T
void update_table(int index, uint8_t outcome, int* table){
  int value = table[index];
  if(value == 0 & outcome == NOTTAKEN){
    return;
  }
  else if(value == 0 & outcome == TAKEN){
    table[index] = 1;
  }
  else if(value == 1 & outcome == NOTTAKEN){
    table[index] = 0;
  }
  else if(value == 1& outcome == TAKEN){
    table[index] = 2;
  }
  else if(value == 2 & outcome == NOTTAKEN){
    table[index] = 1;
  }
  else if(value == 2 & outcome == TAKEN){
    table[index] = 3;
  }
  else if(value == 3 & outcome == NOTTAKEN){
    table[index] = 2;
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
  gmask = (1 << ghistoryBits) - 1;
  lmask = (1 << lhistoryBits) - 1;
  pcmask = (1 << pcIndexBits) - 1;

  int i;
  int size; 
  // initialize global bht and chooser
  size = 1 << ghistoryBits;
  ghistory = (1 << ghistoryBits) - 1;
  for(i = 0; i < size; i++){
      g_table[i] = 0;
      chooser[i] = 0;
  }
  // initialize local pht
  size = 1 << pcIndexBits;
  for(i = 0; i < size; i++){
    l_pht[i] = 0;
  }
  // initialize local bht
  size = 1 << lhistoryBits;
  for(i = 0; i < size; i++){
    l_table[i] = 0;
  }

  // initialize perceptron
  bias = 1;
  if(bpType == CUSTOM){
    for(i = 0; i < p_size; i++){
      hist_array[i] = 1;
      weight[i] = 0;
    }
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
  int lhistory;
  int choice;
  int i;
  int output = bias;
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      index = (pc ^ ghistory) & ((1 << ghistoryBits) - 1);
      return select_pred(g_table[index]);
    case TOURNAMENT:
      choice = chooser[gmask ^ ghistory];
      // p1 - global, p2 - local
      if(choice < 2){
        index = (pc ^ ghistory) & gmask;
        return select_pred(g_table[index]);
      }
      else{
        index = pc & pcmask;
        lhistory = l_pht[index] & lmask;
        return select_pred(l_table[lhistory]);
      }
    case CUSTOM:
      choice = chooser[gmask ^ ghistory];  
      if(choice < 2){
        for(i = 0; i < p_size; i++){
          output += hist_array[i] * weight[i];
        }
        return output > 0;
      }
      else{
        index = pc & pcmask;
        lhistory = l_pht[index] & lmask;
        return select_pred(l_table[lhistory]);
      }
      
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
  // update bht
  int lhistory;
  int gindex =  (pc ^ ghistory) & gmask;
  update_table(gindex, outcome, g_table);
  int lindex = l_pht[pc & pcmask] & lmask;
  update_table(lindex, outcome, l_table);

  //update chooser
  int p1_pred;
  int i;
  int output = bias;
  if(bpType == TOURNAMENT){
    p1_pred = select_pred(g_table[gindex]);
  }
  else if(bpType == CUSTOM){
    for(i = 0; i < p_size; i++){
      output += hist_array[i] * weight[i];
    }
    p1_pred = output > 0;
  }
  int p2_pred = select_pred(l_table[lindex]);
  int cindex = ghistory & gmask;
  int choice = chooser[cindex];
  switch (choice){
    case 0:
      if(p1_pred != outcome & p2_pred == outcome){
        chooser[cindex] += 1;
      }
      break;
    case 1:
      if(p1_pred != outcome & p2_pred == outcome){
        chooser[cindex] += 1;
      }
      else if(p1_pred == outcome){
        chooser[cindex] -= 1;
      }
      break;
    case 2:
      if(p2_pred != outcome & p1_pred == outcome){
        chooser[cindex] -= 1;
      }
      else if(p2_pred == outcome){
        chooser[cindex] += 1;
      }
    case 3:
      if(p2_pred != outcome & p1_pred == outcome){
        chooser[cindex] -= 1;
      }
    default:
      break;
  }

  // update history
  int pht_index = pc & pcmask;
  if(outcome == 1){
    ghistory = ((ghistory << 1)  + 1) & gmask;
    lhistory = l_pht[pht_index];
    l_pht[pht_index] = ((lhistory << 1) + 1) & lmask;
  }
  else{
    ghistory = (ghistory << 1) & gmask;
    lhistory = l_pht[pht_index];
    l_pht[pht_index] = (lhistory << 1) & lmask;
  }

  int update_sgn;
  if(outcome == TAKEN){
    update_sgn = 1;
  }
  else{
    update_sgn = -1;
  }
  if(bpType == CUSTOM){
    if(p1_pred != outcome && (output < thresh || output > -thresh)){
      for(i = 0; i < p_size; i++){
        if(update_sgn == hist_array[i]){
          weight[i] += 1;
        }
        else{
          weight[i] += -1;
        }
      }
      bias += update_sgn;
    }
    for(i = 1; i < p_size; i++){
      hist_array[i - 1]= hist_array[i];
    }
    hist_array[p_size - 1] = update_sgn;
  }
}
