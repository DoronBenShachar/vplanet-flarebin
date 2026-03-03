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

  dest[iBody].bFlareBin = src[iBody].bFlareBin;

  dest[iBody].iFlareBinSeed        = src[iBody].iFlareBinSeed;
  dest[iBody].iFlareBinDist        = src[iBody].iFlareBinDist;
  dest[iBody].iFlareBinNormMode    = src[iBody].iFlareBinNormMode;
  dest[iBody].iFlareBinBandPass    = src[iBody].iFlareBinBandPass;
  dest[iBody].iFlareBinQuadNE      = src[iBody].iFlareBinQuadNE;
  dest[iBody].iFlareBinQuadNX      = src[iBody].iFlareBinQuadNX;
  dest[iBody].iFlareBinMaxOverlapN = src[iBody].iFlareBinMaxOverlapN;
  dest[iBody].iFlareBinNEnergy     = src[iBody].iFlareBinNEnergy;
  dest[iBody].iFlareBinNPhase      = src[iBody].iFlareBinNPhase;

  dest[iBody].dFlareBinOverlapTol = src[iBody].dFlareBinOverlapTol;
  dest[iBody].dFlareBinFrac       = src[iBody].dFlareBinFrac;
  dest[iBody].dFlareBinEmin       = src[iBody].dFlareBinEmin;
  dest[iBody].dFlareBinEmax       = src[iBody].dFlareBinEmax;
  dest[iBody].dFlareBinEStochMin  = src[iBody].dFlareBinEStochMin;
  dest[iBody].dFlareBinAlpha      = src[iBody].dFlareBinAlpha;
  dest[iBody].dFlareBinK          = src[iBody].dFlareBinK;
  dest[iBody].dFlareBinSlope      = src[iBody].dFlareBinSlope;
  dest[iBody].dFlareBinYInt       = src[iBody].dFlareBinYInt;
  dest[iBody].dFlareBinLogMu      = src[iBody].dFlareBinLogMu;
  dest[iBody].dFlareBinLogSigma   = src[iBody].dFlareBinLogSigma;
  dest[iBody].dFlareBinRateTot    = src[iBody].dFlareBinRateTot;
  dest[iBody].dFlareBinTau0       = src[iBody].dFlareBinTau0;
  dest[iBody].dFlareBinDurE0      = src[iBody].dFlareBinDurE0;
  dest[iBody].dFlareBinDurExp     = src[iBody].dFlareBinDurExp;
  dest[iBody].dFlareBinXMin       = src[iBody].dFlareBinXMin;
  dest[iBody].dFlareBinXEnd       = src[iBody].dFlareBinXEnd;
  dest[iBody].dFlareBinBandC      = src[iBody].dFlareBinBandC;
  dest[iBody].dFlareBinBandP      = src[iBody].dFlareBinBandP;
  dest[iBody].dFlareBinFXUVThresh1 = src[iBody].dFlareBinFXUVThresh1;
  dest[iBody].dFlareBinFXUVThresh2 = src[iBody].dFlareBinFXUVThresh2;

  dest[iBody].dFlareBinLQ                = src[iBody].dFlareBinLQ;
  dest[iBody].dFlareBinPStoch            = src[iBody].dFlareBinPStoch;
  dest[iBody].dFlareBinMu                = src[iBody].dFlareBinMu;
  dest[iBody].dFlareBinITpl              = src[iBody].dFlareBinITpl;
  dest[iBody].dFlareBinDeltaX            = src[iBody].dFlareBinDeltaX;
  dest[iBody].dFlareBinLastPrecomputeAge = src[iBody].dFlareBinLastPrecomputeAge;

  dest[iBody].daFlareBinQuadU  = src[iBody].daFlareBinQuadU;
  dest[iBody].daFlareBinQuadWU = src[iBody].daFlareBinQuadWU;
  dest[iBody].daFlareBinQuadE  = src[iBody].daFlareBinQuadE;
  dest[iBody].daFlareBinQuadWE = src[iBody].daFlareBinQuadWE;
  dest[iBody].daFlareBinQuadX  = src[iBody].daFlareBinQuadX;
  dest[iBody].daFlareBinQuadWX = src[iBody].daFlareBinQuadWX;
  dest[iBody].daFlareBinTplAtX = src[iBody].daFlareBinTplAtX;
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

  body[iBody].iFlareBinSeed        = 0;
  body[iBody].iFlareBinDist        = FLAREBIN_DIST_POWERLAW;
  body[iBody].iFlareBinNormMode    = FLAREBIN_NORM_FROM_FFD;
  body[iBody].iFlareBinBandPass    = FLAREBIN_BANDPASS_XUV;
  body[iBody].iFlareBinQuadNE      = 24;
  body[iBody].iFlareBinQuadNX      = 16;
  body[iBody].iFlareBinMaxOverlapN = FLAREBIN_OVERLAP_N1;
  body[iBody].iFlareBinNEnergy     = 0;
  body[iBody].iFlareBinNPhase      = 0;

  body[iBody].dFlareBinOverlapTol = 1e-6;
  body[iBody].dFlareBinFrac       = 0;
  body[iBody].dFlareBinEmin       = 0;
  body[iBody].dFlareBinEmax       = 0;
  body[iBody].dFlareBinEStochMin = 0;
  body[iBody].dFlareBinAlpha     = 0;
  body[iBody].dFlareBinK         = 0;
  body[iBody].dFlareBinSlope     = 0;
  body[iBody].dFlareBinYInt      = 0;
  body[iBody].dFlareBinLogMu     = 0;
  body[iBody].dFlareBinLogSigma  = 0;
  body[iBody].dFlareBinRateTot   = 0;
  body[iBody].dFlareBinTau0      = 0;
  body[iBody].dFlareBinDurE0     = 0;
  body[iBody].dFlareBinDurExp    = 0;
  body[iBody].dFlareBinXMin      = -1;
  body[iBody].dFlareBinXEnd      = 20;
  body[iBody].dFlareBinBandC     = 1;
  body[iBody].dFlareBinBandP     = 1;
  body[iBody].dFlareBinFXUVThresh1 = 0;
  body[iBody].dFlareBinFXUVThresh2 = 0;

  body[iBody].dFlareBinLQ        = 0;
  body[iBody].dFlareBinPStoch    = 0;
  body[iBody].dFlareBinMu        = 0;
  body[iBody].dFlareBinITpl      = 0;
  body[iBody].dFlareBinDeltaX    = 0;
  body[iBody].dFlareBinLastPrecomputeAge = -1;

  body[iBody].daFlareBinQuadU  = NULL;
  body[iBody].daFlareBinQuadWU = NULL;
  body[iBody].daFlareBinQuadE  = NULL;
  body[iBody].daFlareBinQuadWE = NULL;
  body[iBody].daFlareBinQuadX  = NULL;
  body[iBody].daFlareBinQuadWX = NULL;
  body[iBody].daFlareBinTplAtX = NULL;
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
