/**
  @file flarebin.c

  @brief Stub-only module plumbing for FLAREBIN integration points.
  This file intentionally provides no flare physics and no stochastic logic.
  It only wires function pointers so the module compiles and registers cleanly.
*/

#include "vplanet.h"

void BodyCopyFlareBin(BODY *dest, BODY *src, int foo, int iNumBodies,
                      int iBody) {
  (void)foo;
  (void)iNumBodies;

  dest[iBody].bFlareBin        = src[iBody].bFlareBin;
  dest[iBody].iFlareBinNEnergy = src[iBody].iFlareBinNEnergy;
  dest[iBody].iFlareBinNPhase  = src[iBody].iFlareBinNPhase;
  dest[iBody].iFlareBinNMax    = src[iBody].iFlareBinNMax;
  dest[iBody].dFlareBinEStochMin = src[iBody].dFlareBinEStochMin;
  dest[iBody].dFlareBinLQ        = src[iBody].dFlareBinLQ;
  dest[iBody].dFlareBinPStoch    = src[iBody].dFlareBinPStoch;
  dest[iBody].dFlareBinMu        = src[iBody].dFlareBinMu;
  dest[iBody].dFlareBinITpl      = src[iBody].dFlareBinITpl;
}

void InitializeOptionsFlareBin(OPTIONS *options, fnReadOption fnRead[]) {
  (void)options;
  (void)fnRead;
}

void ReadOptionsFlareBin(BODY *body, CONTROL *control, FILES *files,
                         OPTIONS *options, SYSTEM *system,
                         fnReadOption fnRead[], int iBody) {
  int iOpt;

  (void)body;
  (void)control;
  (void)files;
  (void)system;

  for (iOpt = OPTSTARTFLAREBIN; iOpt < OPTENDFLAREBIN; iOpt++) {
    if (options[iOpt].iType != -1 && fnRead[iOpt] != NULL) {
      fnRead[iOpt](body, control, files, &options[iOpt], system, iBody + 1);
    }
  }
}

void PropsAuxFlareBin(BODY *body, EVOLVE *evolve, IO *io, UPDATE *update,
                      int iBody) {
  (void)body;
  (void)evolve;
  (void)io;
  (void)update;
  (void)iBody;
}

void VerifyFlareBin(BODY *body, CONTROL *control, FILES *files, OPTIONS *options,
                    OUTPUT *output, SYSTEM *system, UPDATE *update, int iBody,
                    int iModule) {
  (void)files;
  (void)options;
  (void)output;
  (void)system;

  control->fnForceBehavior[iBody][iModule]   = &fnForceBehaviorFlareBin;
  control->fnPropsAux[iBody][iModule]        = &PropsAuxFlareBin;
  control->Evolve.fnBodyCopy[iBody][iModule] = &BodyCopyFlareBin;

  (void)body;
  (void)update;
}

void AssignFlareBinDerivatives(BODY *body, EVOLVE *evolve, UPDATE *update,
                               fnUpdateVariable ***fnUpdate, int iBody) {
  (void)body;
  (void)evolve;
  (void)update;
  (void)fnUpdate;
  (void)iBody;
}

void NullFlareBinDerivatives(BODY *body, EVOLVE *evolve, UPDATE *update,
                             fnUpdateVariable ***fnUpdate, int iBody) {
  (void)body;
  (void)evolve;
  (void)update;
  (void)fnUpdate;
  (void)iBody;
}

void fnForceBehaviorFlareBin(BODY *body, MODULE *module, EVOLVE *evolve, IO *io,
                             SYSTEM *system, UPDATE *update,
                             fnUpdateVariable ***fnUpdate, int iBody,
                             int iModule) {
  (void)body;
  (void)module;
  (void)evolve;
  (void)io;
  (void)system;
  (void)update;
  (void)fnUpdate;
  (void)iBody;
  (void)iModule;
}

void InitializeBodyFlareBin(BODY *body, CONTROL *control, UPDATE *update,
                            int iBody, int iModule) {
  (void)control;
  (void)update;
  (void)iModule;

  body[iBody].iFlareBinNEnergy = 0;
  body[iBody].iFlareBinNPhase  = 0;
  body[iBody].iFlareBinNMax    = 0;
  body[iBody].dFlareBinEStochMin = 0;
  body[iBody].dFlareBinLQ        = 0;
  body[iBody].dFlareBinPStoch    = 0;
  body[iBody].dFlareBinMu        = 0;
  body[iBody].dFlareBinITpl      = 0;
}

void InitializeUpdateFlareBin(BODY *body, UPDATE *update, int iBody) {
  (void)body;
  (void)update;
  (void)iBody;
}

void CountHaltsFlareBin(HALT *halt, int *iNumHalts) {
  (void)halt;
  (void)iNumHalts;
}

void VerifyHaltFlareBin(BODY *body, CONTROL *control, OPTIONS *options,
                        int iBody, int *iHalt) {
  (void)body;
  (void)control;
  (void)options;
  (void)iBody;
  (void)iHalt;
}

void InitializeOutputFlareBin(OUTPUT *output, fnWriteOutput fnWrite[]) {
  (void)output;
  (void)fnWrite;
}

void LogOptionsFlareBin(CONTROL *control, FILE *fp) {
  (void)control;
  (void)fp;
}

void LogFlareBin(BODY *body, CONTROL *control, OUTPUT *output, SYSTEM *system,
                 UPDATE *update, fnWriteOutput fnWrite[], FILE *fp) {
  (void)body;
  (void)control;
  (void)output;
  (void)system;
  (void)update;
  (void)fnWrite;
  (void)fp;
}

void LogBodyFlareBin(BODY *body, CONTROL *control, OUTPUT *output,
                     SYSTEM *system, UPDATE *update, fnWriteOutput fnWrite[],
                     FILE *fp, int iBody) {
  (void)body;
  (void)control;
  (void)output;
  (void)system;
  (void)update;
  (void)fnWrite;
  (void)fp;
  (void)iBody;
}

void AddModuleFlareBin(CONTROL *control, MODULE *module, int iBody,
                       int iModule) {
  (void)control;

  module->iaModule[iBody][iModule] = FLAREBIN;

  module->fnCountHalts[iBody][iModule]        = &CountHaltsFlareBin;
  module->fnReadOptions[iBody][iModule]       = &ReadOptionsFlareBin;
  module->fnLogBody[iBody][iModule]           = &LogBodyFlareBin;
  module->fnVerify[iBody][iModule]            = &VerifyFlareBin;
  module->fnAssignDerivatives[iBody][iModule] = &AssignFlareBinDerivatives;
  module->fnNullDerivatives[iBody][iModule]   = &NullFlareBinDerivatives;
  module->fnVerifyHalt[iBody][iModule]        = &VerifyHaltFlareBin;

  module->fnInitializeBody[iBody][iModule]      = &InitializeBodyFlareBin;
  module->fnInitializeUpdate[iBody][iModule]    = &InitializeUpdateFlareBin;
  module->fnInitializeOutput[iBody][iModule]    = &InitializeOutputFlareBin;
}
