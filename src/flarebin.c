/**
  @file flarebin.c

  @brief Stub-only module plumbing for FLAREBIN integration points.
  This file intentionally provides no flare physics and no stochastic logic.
  It only wires function pointers so the module compiles and registers cleanly.
*/

#include "vplanet.h"

/* Internal helper from flarebin_effavg.c */
int fiFlareBinGaussLegendreRule(int, double, double, double *, double *);

#define FLAREBIN_PROB_CLIP_TOL 1e-12

typedef struct {
  double dThreshold;
} FLAREBINTHRESHCTX;

static void fvFlareBinCopyArray(double **daDest, const double *daSrc, int iSize,
                                 const char *cName) {
  double *daTmp;
  int i;

  if (iSize <= 0 || daSrc == NULL) {
    if (*daDest != NULL) {
      free(*daDest);
      *daDest = NULL;
    }
    return;
  }

  daTmp = realloc(*daDest, iSize * sizeof(double));
  if (daTmp == NULL) {
    fprintf(stderr, "ERROR: FLAREBIN failed to allocate %s.\n", cName);
    exit(EXIT_FAILURE);
  }
  *daDest = daTmp;

  for (i = 0; i < iSize; i++) {
    (*daDest)[i] = daSrc[i];
  }
}

static double *fdaFlareBinMallocArray(int iSize, const char *cName) {
  double *daTmp;

  if (iSize <= 0) {
    return NULL;
  }

  daTmp = malloc(iSize * sizeof(double));
  if (daTmp == NULL) {
    fprintf(stderr, "ERROR: FLAREBIN failed to allocate %s.\n", cName);
    exit(EXIT_FAILURE);
  }

  return daTmp;
}

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

  fvFlareBinCopyArray(&dest[iBody].daFlareBinQuadU, src[iBody].daFlareBinQuadU,
                      dest[iBody].iFlareBinNEnergy, "daFlareBinQuadU");
  fvFlareBinCopyArray(&dest[iBody].daFlareBinQuadWU,
                      src[iBody].daFlareBinQuadWU,
                      dest[iBody].iFlareBinNEnergy, "daFlareBinQuadWU");
  fvFlareBinCopyArray(&dest[iBody].daFlareBinQuadE, src[iBody].daFlareBinQuadE,
                      dest[iBody].iFlareBinNEnergy, "daFlareBinQuadE");
  fvFlareBinCopyArray(&dest[iBody].daFlareBinQuadWE,
                      src[iBody].daFlareBinQuadWE,
                      dest[iBody].iFlareBinNEnergy, "daFlareBinQuadWE");
  fvFlareBinCopyArray(&dest[iBody].daFlareBinQuadX, src[iBody].daFlareBinQuadX,
                      dest[iBody].iFlareBinNPhase, "daFlareBinQuadX");
  fvFlareBinCopyArray(&dest[iBody].daFlareBinQuadWX,
                      src[iBody].daFlareBinQuadWX,
                      dest[iBody].iFlareBinNPhase, "daFlareBinQuadWX");
  fvFlareBinCopyArray(&dest[iBody].daFlareBinTplAtX,
                      src[iBody].daFlareBinTplAtX,
                      dest[iBody].iFlareBinNPhase, "daFlareBinTplAtX");
}

static void fvFlareBinOptionError(CONTROL *control, FILES *files, OPTIONS *options,
                                  int iFile, int iLine, const char *cMsg) {
  if (control->Io.iVerbose >= VERBERR) {
    fprintf(stderr, "ERROR: %s %s\n", options->cName, cMsg);
  }
  if (iLine < 0) {
    iLine = 0;
  }
  LineExit(files->Infile[iFile].cIn, iLine);
}

static int fbReadFlareBinInt(CONTROL *control, FILES *files, OPTIONS *options,
                             int iFile, int *piValue) {
  int lTmp = -1;
  int iTmp;

  AddOptionInt(files->Infile[iFile].cIn, options->cName, &iTmp, &lTmp,
               control->Io.iVerbose);

  if (lTmp >= 0) {
    NotPrimaryInput(iFile, options->cName, files->Infile[iFile].cIn, lTmp,
                    control->Io.iVerbose);
    *piValue = iTmp;
    UpdateFoundOption(&files->Infile[iFile], options, lTmp, iFile);
    return 1;
  }

  if (iFile > 0) {
    *piValue = (int)options->dDefault;
  }
  return 0;
}

static int fbReadFlareBinDouble(CONTROL *control, FILES *files,
                                OPTIONS *options, int iFile, double *pdValue) {
  int lTmp = -1;
  double dTmp;

  AddOptionDouble(files->Infile[iFile].cIn, options->cName, &dTmp, &lTmp,
                  control->Io.iVerbose);

  if (lTmp >= 0) {
    NotPrimaryInput(iFile, options->cName, files->Infile[iFile].cIn, lTmp,
                    control->Io.iVerbose);
    *pdValue = dTmp;
    UpdateFoundOption(&files->Infile[iFile], options, lTmp, iFile);
    return 1;
  }

  if (iFile > 0) {
    *pdValue = options->dDefault;
  }
  return 0;
}

static int fbReadFlareBinString(CONTROL *control, FILES *files, OPTIONS *options,
                                int iFile, char cTmp[OPTLEN], int *piLine) {
  AddOptionString(files->Infile[iFile].cIn, options->cName, cTmp, piLine,
                  control->Io.iVerbose);

  if (*piLine >= 0) {
    NotPrimaryInput(iFile, options->cName, files->Infile[iFile].cIn, *piLine,
                    control->Io.iVerbose);
    return 1;
  }
  return 0;
}

void ReadFlareBinSeed(BODY *body, CONTROL *control, FILES *files,
                      OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinInt(control, files, options, iFile,
                    &body[iBody].iFlareBinSeed);

  if (body[iBody].iFlareBinSeed < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinQuadNE(BODY *body, CONTROL *control, FILES *files,
                        OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinInt(control, files, options, iFile,
                    &body[iBody].iFlareBinQuadNE);

  if (body[iBody].iFlareBinQuadNE < 1) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 1.");
  }
}

void ReadFlareBinQuadNX(BODY *body, CONTROL *control, FILES *files,
                        OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinInt(control, files, options, iFile,
                    &body[iBody].iFlareBinQuadNX);

  if (body[iBody].iFlareBinQuadNX < 1) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 1.");
  }
}

void ReadFlareBinMaxOverlapN(BODY *body, CONTROL *control, FILES *files,
                             OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinInt(control, files, options, iFile,
                    &body[iBody].iFlareBinMaxOverlapN);

  if (body[iBody].iFlareBinMaxOverlapN < 0 ||
      body[iBody].iFlareBinMaxOverlapN > 2) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be in [0,2].");
  }
}

void ReadFlareBinOverlapTol(BODY *body, CONTROL *control, FILES *files,
                            OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinOverlapTol);

  if (body[iBody].dFlareBinOverlapTol < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinDist(BODY *body, CONTROL *control, FILES *files,
                      OPTIONS *options, SYSTEM *system, int iFile) {
  int lTmp = -1;
  int iBody = iFile - 1;
  char cTmp[OPTLEN];

  (void)system;
  if (fbReadFlareBinString(control, files, options, iFile, cTmp, &lTmp)) {
    if (!memcmp(sLower(cTmp), "po", 2)) {
      body[iBody].iFlareBinDist = FLAREBIN_DIST_POWERLAW;
    } else if (!memcmp(sLower(cTmp), "lo", 2)) {
      body[iBody].iFlareBinDist = FLAREBIN_DIST_LOGNORMAL;
    } else {
      fvFlareBinOptionError(control, files, options, iFile, lTmp,
                            "must be POWERLAW_FFD or LOGNORMAL.");
    }
    UpdateFoundOption(&files->Infile[iFile], options, lTmp, iFile);
  } else if (iFile > 0) {
    body[iBody].iFlareBinDist = (int)options->dDefault;
  }
}

void ReadFlareBinNormMode(BODY *body, CONTROL *control, FILES *files,
                          OPTIONS *options, SYSTEM *system, int iFile) {
  int lTmp = -1;
  int iBody = iFile - 1;
  char cTmp[OPTLEN];
  char *cLow;

  (void)system;
  if (fbReadFlareBinString(control, files, options, iFile, cTmp, &lTmp)) {
    cLow = sLower(cTmp);
    if (!memcmp(cLow, "ffd", 3) || strstr(cLow, "from_ffd") != NULL) {
      body[iBody].iFlareBinNormMode = FLAREBIN_NORM_FROM_FFD;
    } else if (strstr(cLow, "flarepower") != NULL || strstr(cLow, "fraction") != NULL) {
      body[iBody].iFlareBinNormMode = FLAREBIN_NORM_FROM_FLAREPOWER_FRACTION;
    } else if (strstr(cLow, "rate") != NULL) {
      body[iBody].iFlareBinNormMode = FLAREBIN_NORM_FROM_RATE_AT_E0;
    } else {
      fvFlareBinOptionError(control, files, options, iFile, lTmp,
                            "must be NORM_FROM_FFD, NORM_FROM_FLAREPOWER_FRACTION, or NORM_FROM_RATE_AT_E0.");
    }
    UpdateFoundOption(&files->Infile[iFile], options, lTmp, iFile);
  } else if (iFile > 0) {
    body[iBody].iFlareBinNormMode = (int)options->dDefault;
  }
}

void ReadFlareBinFrac(BODY *body, CONTROL *control, FILES *files,
                      OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinFrac);

  if (body[iBody].dFlareBinFrac < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinBandPass(BODY *body, CONTROL *control, FILES *files,
                          OPTIONS *options, SYSTEM *system, int iFile) {
  int lTmp = -1;
  int iBody = iFile - 1;
  char cTmp[OPTLEN];

  (void)system;
  if (fbReadFlareBinString(control, files, options, iFile, cTmp, &lTmp)) {
    if (!memcmp(sLower(cTmp), "xu", 2)) {
      body[iBody].iFlareBinBandPass = FLAREBIN_BANDPASS_XUV;
    } else if (!memcmp(sLower(cTmp), "ke", 2)) {
      body[iBody].iFlareBinBandPass = FLAREBIN_BANDPASS_KEPLER;
    } else if (!memcmp(sLower(cTmp), "te", 2)) {
      body[iBody].iFlareBinBandPass = FLAREBIN_BANDPASS_TESS;
    } else if (!memcmp(sLower(cTmp), "bo", 2)) {
      body[iBody].iFlareBinBandPass = FLAREBIN_BANDPASS_BOLOMETRIC;
    } else {
      fvFlareBinOptionError(control, files, options, iFile, lTmp,
                            "must be XUV, KEPLER, TESS, or BOLOMETRIC.");
    }
    UpdateFoundOption(&files->Infile[iFile], options, lTmp, iFile);
  } else if (iFile > 0) {
    body[iBody].iFlareBinBandPass = (int)options->dDefault;
  }
}

void ReadFlareBinEmin(BODY *body, CONTROL *control, FILES *files,
                      OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinEmin);

  if (body[iBody].dFlareBinEmin < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinEmax(BODY *body, CONTROL *control, FILES *files,
                      OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinEmax);

  if (body[iBody].dFlareBinEmax < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinEStochMin(BODY *body, CONTROL *control, FILES *files,
                           OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinEStochMin);

  if (body[iBody].dFlareBinEStochMin < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinAlpha(BODY *body, CONTROL *control, FILES *files,
                       OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinAlpha);

  if (body[iBody].dFlareBinAlpha < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinK(BODY *body, CONTROL *control, FILES *files, OPTIONS *options,
                   SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinK);

  if (body[iBody].dFlareBinK < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinSlope(BODY *body, CONTROL *control, FILES *files,
                       OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinSlope);
}

void ReadFlareBinYInt(BODY *body, CONTROL *control, FILES *files,
                      OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinYInt);
}

void ReadFlareBinLogMu(BODY *body, CONTROL *control, FILES *files,
                       OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinLogMu);
}

void ReadFlareBinLogSigma(BODY *body, CONTROL *control, FILES *files,
                          OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinLogSigma);

  if (body[iBody].dFlareBinLogSigma < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinRateTot(BODY *body, CONTROL *control, FILES *files,
                         OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinRateTot);

  if (body[iBody].dFlareBinRateTot < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinTau0(BODY *body, CONTROL *control, FILES *files,
                      OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinTau0);

  if (body[iBody].dFlareBinTau0 < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinDurE0(BODY *body, CONTROL *control, FILES *files,
                       OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinDurE0);

  if (body[iBody].dFlareBinDurE0 < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinDurExp(BODY *body, CONTROL *control, FILES *files,
                        OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinDurExp);
}

void ReadFlareBinXMin(BODY *body, CONTROL *control, FILES *files,
                      OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinXMin);
}

void ReadFlareBinXEnd(BODY *body, CONTROL *control, FILES *files,
                      OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinXEnd);
}

void ReadFlareBinBandC(BODY *body, CONTROL *control, FILES *files,
                       OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinBandC);

  if (body[iBody].dFlareBinBandC < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinBandP(BODY *body, CONTROL *control, FILES *files,
                       OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinBandP);

  if (body[iBody].dFlareBinBandP < 0) {
    fvFlareBinOptionError(control, files, options, iFile, options->iLine[iFile],
                          "must be >= 0.");
  }
}

void ReadFlareBinFXUVThresh1(BODY *body, CONTROL *control, FILES *files,
                             OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinFXUVThresh1);
}

void ReadFlareBinFXUVThresh2(BODY *body, CONTROL *control, FILES *files,
                             OPTIONS *options, SYSTEM *system, int iFile) {
  int iBody = iFile - 1;

  (void)system;
  fbReadFlareBinDouble(control, files, options, iFile,
                       &body[iBody].dFlareBinFXUVThresh2);
}

void InitializeOptionsFlareBin(OPTIONS *options, fnReadOption fnRead[]) {
  fvFormattedString(&options[OPT_FLAREBINSEED].cName, "iFlareBinSeed");
  fvFormattedString(&options[OPT_FLAREBINSEED].cDescr, "Flarebin validation seed");
  fvFormattedString(&options[OPT_FLAREBINSEED].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINSEED].cDimension, "none");
  options[OPT_FLAREBINSEED].dDefault   = 0;
  options[OPT_FLAREBINSEED].iType      = 1;
  options[OPT_FLAREBINSEED].bMultiFile = 1;
  options[OPT_FLAREBINSEED].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINSEED].iFileType  = 1;
  fnRead[OPT_FLAREBINSEED]             = &ReadFlareBinSeed;

  fvFormattedString(&options[OPT_FLAREBINQUADNE].cName, "iFlareBinQuadNE");
  fvFormattedString(&options[OPT_FLAREBINQUADNE].cDescr, "Log-energy quadrature nodes");
  fvFormattedString(&options[OPT_FLAREBINQUADNE].cDefault, "24");
  fvFormattedString(&options[OPT_FLAREBINQUADNE].cDimension, "none");
  options[OPT_FLAREBINQUADNE].dDefault   = 24;
  options[OPT_FLAREBINQUADNE].iType      = 1;
  options[OPT_FLAREBINQUADNE].bMultiFile = 1;
  options[OPT_FLAREBINQUADNE].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINQUADNE].iFileType  = 1;
  fnRead[OPT_FLAREBINQUADNE]             = &ReadFlareBinQuadNE;

  fvFormattedString(&options[OPT_FLAREBINQUADNX].cName, "iFlareBinQuadNX");
  fvFormattedString(&options[OPT_FLAREBINQUADNX].cDescr, "Phase quadrature nodes");
  fvFormattedString(&options[OPT_FLAREBINQUADNX].cDefault, "16");
  fvFormattedString(&options[OPT_FLAREBINQUADNX].cDimension, "none");
  options[OPT_FLAREBINQUADNX].dDefault   = 16;
  options[OPT_FLAREBINQUADNX].iType      = 1;
  options[OPT_FLAREBINQUADNX].bMultiFile = 1;
  options[OPT_FLAREBINQUADNX].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINQUADNX].iFileType  = 1;
  fnRead[OPT_FLAREBINQUADNX]             = &ReadFlareBinQuadNX;

  fvFormattedString(&options[OPT_FLAREBINMAXOVERLAPN].cName, "iFlareBinMaxOverlapN");
  fvFormattedString(&options[OPT_FLAREBINMAXOVERLAPN].cDescr, "Maximum overlap order");
  fvFormattedString(&options[OPT_FLAREBINMAXOVERLAPN].cDefault, "1");
  fvFormattedString(&options[OPT_FLAREBINMAXOVERLAPN].cValues, "0 1 2");
  fvFormattedString(&options[OPT_FLAREBINMAXOVERLAPN].cDimension, "none");
  options[OPT_FLAREBINMAXOVERLAPN].dDefault   = 1;
  options[OPT_FLAREBINMAXOVERLAPN].iType      = 1;
  options[OPT_FLAREBINMAXOVERLAPN].bMultiFile = 1;
  options[OPT_FLAREBINMAXOVERLAPN].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINMAXOVERLAPN].iFileType  = 1;
  fnRead[OPT_FLAREBINMAXOVERLAPN]             = &ReadFlareBinMaxOverlapN;

  fvFormattedString(&options[OPT_FLAREBINOVERLAPTOL].cName, "dFlareBinOverlapTol");
  fvFormattedString(&options[OPT_FLAREBINOVERLAPTOL].cDescr, "Neglected overlap probability tolerance");
  fvFormattedString(&options[OPT_FLAREBINOVERLAPTOL].cDefault, "1e-6");
  fvFormattedString(&options[OPT_FLAREBINOVERLAPTOL].cDimension, "none");
  options[OPT_FLAREBINOVERLAPTOL].dDefault   = 1e-6;
  options[OPT_FLAREBINOVERLAPTOL].iType      = 2;
  options[OPT_FLAREBINOVERLAPTOL].bMultiFile = 1;
  options[OPT_FLAREBINOVERLAPTOL].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINOVERLAPTOL].iFileType  = 1;
  fnRead[OPT_FLAREBINOVERLAPTOL]             = &ReadFlareBinOverlapTol;

  fvFormattedString(&options[OPT_FLAREBINDIST].cName, "sFlareBinDist");
  fvFormattedString(&options[OPT_FLAREBINDIST].cDescr, "Flare population distribution");
  fvFormattedString(&options[OPT_FLAREBINDIST].cDefault, "POWERLAW_FFD");
  fvFormattedString(&options[OPT_FLAREBINDIST].cValues, "POWERLAW_FFD LOGNORMAL");
  fvFormattedString(&options[OPT_FLAREBINDIST].cDimension, "none");
  options[OPT_FLAREBINDIST].dDefault   = FLAREBIN_DIST_POWERLAW;
  options[OPT_FLAREBINDIST].iType      = 3;
  options[OPT_FLAREBINDIST].bMultiFile = 1;
  options[OPT_FLAREBINDIST].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINDIST].iFileType  = 1;
  fnRead[OPT_FLAREBINDIST]             = &ReadFlareBinDist;

  fvFormattedString(&options[OPT_FLAREBINNORMMODE].cName, "sFlareBinNormMode");
  fvFormattedString(&options[OPT_FLAREBINNORMMODE].cDescr, "Flarebin normalization mode");
  fvFormattedString(&options[OPT_FLAREBINNORMMODE].cDefault, "NORM_FROM_FFD");
  fvFormattedString(&options[OPT_FLAREBINNORMMODE].cValues,
                    "NORM_FROM_FFD NORM_FROM_FLAREPOWER_FRACTION NORM_FROM_RATE_AT_E0");
  fvFormattedString(&options[OPT_FLAREBINNORMMODE].cDimension, "none");
  options[OPT_FLAREBINNORMMODE].dDefault   = FLAREBIN_NORM_FROM_FFD;
  options[OPT_FLAREBINNORMMODE].iType      = 3;
  options[OPT_FLAREBINNORMMODE].bMultiFile = 1;
  options[OPT_FLAREBINNORMMODE].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINNORMMODE].iFileType  = 1;
  fnRead[OPT_FLAREBINNORMMODE]             = &ReadFlareBinNormMode;

  fvFormattedString(&options[OPT_FLAREBINFRAC].cName, "dFlareBinFrac");
  fvFormattedString(&options[OPT_FLAREBINFRAC].cDescr, "Target flare power fraction");
  fvFormattedString(&options[OPT_FLAREBINFRAC].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINFRAC].cDimension, "none");
  options[OPT_FLAREBINFRAC].dDefault   = 0;
  options[OPT_FLAREBINFRAC].iType      = 2;
  options[OPT_FLAREBINFRAC].bMultiFile = 1;
  options[OPT_FLAREBINFRAC].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINFRAC].iFileType  = 1;
  fnRead[OPT_FLAREBINFRAC]             = &ReadFlareBinFrac;

  fvFormattedString(&options[OPT_FLAREBINBANDPASS].cName, "sFlareBinBandPass");
  fvFormattedString(&options[OPT_FLAREBINBANDPASS].cDescr, "Input flare bandpass");
  fvFormattedString(&options[OPT_FLAREBINBANDPASS].cDefault, "XUV");
  fvFormattedString(&options[OPT_FLAREBINBANDPASS].cValues, "XUV KEPLER TESS BOLOMETRIC");
  fvFormattedString(&options[OPT_FLAREBINBANDPASS].cDimension, "none");
  options[OPT_FLAREBINBANDPASS].dDefault   = FLAREBIN_BANDPASS_XUV;
  options[OPT_FLAREBINBANDPASS].iType      = 3;
  options[OPT_FLAREBINBANDPASS].bMultiFile = 1;
  options[OPT_FLAREBINBANDPASS].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINBANDPASS].iFileType  = 1;
  fnRead[OPT_FLAREBINBANDPASS]             = &ReadFlareBinBandPass;

  fvFormattedString(&options[OPT_FLAREBINEMIN].cName, "dFlareBinEmin");
  fvFormattedString(&options[OPT_FLAREBINEMIN].cDescr, "Minimum flare energy");
  fvFormattedString(&options[OPT_FLAREBINEMIN].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINEMIN].cDimension, "energy");
  options[OPT_FLAREBINEMIN].dDefault   = 0;
  options[OPT_FLAREBINEMIN].iType      = 2;
  options[OPT_FLAREBINEMIN].bMultiFile = 1;
  options[OPT_FLAREBINEMIN].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINEMIN].iFileType  = 1;
  fnRead[OPT_FLAREBINEMIN]             = &ReadFlareBinEmin;

  fvFormattedString(&options[OPT_FLAREBINEMAX].cName, "dFlareBinEmax");
  fvFormattedString(&options[OPT_FLAREBINEMAX].cDescr, "Maximum flare energy");
  fvFormattedString(&options[OPT_FLAREBINEMAX].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINEMAX].cDimension, "energy");
  options[OPT_FLAREBINEMAX].dDefault   = 0;
  options[OPT_FLAREBINEMAX].iType      = 2;
  options[OPT_FLAREBINEMAX].bMultiFile = 1;
  options[OPT_FLAREBINEMAX].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINEMAX].iFileType  = 1;
  fnRead[OPT_FLAREBINEMAX]             = &ReadFlareBinEmax;

  fvFormattedString(&options[OPT_FLAREBINESTOCHMIN].cName, "dFlareBinEStochMin");
  fvFormattedString(&options[OPT_FLAREBINESTOCHMIN].cDescr,
                    "Minimum stochastic flare energy");
  fvFormattedString(&options[OPT_FLAREBINESTOCHMIN].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINESTOCHMIN].cDimension, "energy");
  options[OPT_FLAREBINESTOCHMIN].dDefault   = 0;
  options[OPT_FLAREBINESTOCHMIN].iType      = 2;
  options[OPT_FLAREBINESTOCHMIN].bMultiFile = 1;
  options[OPT_FLAREBINESTOCHMIN].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINESTOCHMIN].iFileType  = 1;
  fnRead[OPT_FLAREBINESTOCHMIN]             = &ReadFlareBinEStochMin;

  fvFormattedString(&options[OPT_FLAREBINALPHA].cName, "dFlareBinAlpha");
  fvFormattedString(&options[OPT_FLAREBINALPHA].cDescr, "Power-law differential index");
  fvFormattedString(&options[OPT_FLAREBINALPHA].cDefault, "2.0");
  fvFormattedString(&options[OPT_FLAREBINALPHA].cDimension, "none");
  options[OPT_FLAREBINALPHA].dDefault   = 2.0;
  options[OPT_FLAREBINALPHA].iType      = 2;
  options[OPT_FLAREBINALPHA].bMultiFile = 1;
  options[OPT_FLAREBINALPHA].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINALPHA].iFileType  = 1;
  fnRead[OPT_FLAREBINALPHA]             = &ReadFlareBinAlpha;

  fvFormattedString(&options[OPT_FLAREBINK].cName, "dFlareBinK");
  fvFormattedString(&options[OPT_FLAREBINK].cDescr, "Power-law normalization");
  fvFormattedString(&options[OPT_FLAREBINK].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINK].cDimension, "1/time/energy");
  options[OPT_FLAREBINK].dDefault   = 0;
  options[OPT_FLAREBINK].iType      = 2;
  options[OPT_FLAREBINK].bMultiFile = 1;
  options[OPT_FLAREBINK].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINK].iFileType  = 1;
  fnRead[OPT_FLAREBINK]             = &ReadFlareBinK;

  fvFormattedString(&options[OPT_FLAREBINSLOPE].cName, "dFlareBinSlope");
  fvFormattedString(&options[OPT_FLAREBINSLOPE].cDescr, "Cumulative FFD slope");
  fvFormattedString(&options[OPT_FLAREBINSLOPE].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINSLOPE].cDimension, "none");
  options[OPT_FLAREBINSLOPE].dDefault   = 0;
  options[OPT_FLAREBINSLOPE].iType      = 2;
  options[OPT_FLAREBINSLOPE].bMultiFile = 1;
  options[OPT_FLAREBINSLOPE].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINSLOPE].iFileType  = 1;
  fnRead[OPT_FLAREBINSLOPE]             = &ReadFlareBinSlope;

  fvFormattedString(&options[OPT_FLAREBINYINT].cName, "dFlareBinYInt");
  fvFormattedString(&options[OPT_FLAREBINYINT].cDescr, "Cumulative FFD intercept");
  fvFormattedString(&options[OPT_FLAREBINYINT].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINYINT].cDimension, "1/time");
  options[OPT_FLAREBINYINT].dDefault   = 0;
  options[OPT_FLAREBINYINT].iType      = 2;
  options[OPT_FLAREBINYINT].bMultiFile = 1;
  options[OPT_FLAREBINYINT].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINYINT].iFileType  = 1;
  fnRead[OPT_FLAREBINYINT]             = &ReadFlareBinYInt;

  fvFormattedString(&options[OPT_FLAREBINLOGMU].cName, "dFlareBinLogMu");
  fvFormattedString(&options[OPT_FLAREBINLOGMU].cDescr, "Lognormal mean in ln(E)");
  fvFormattedString(&options[OPT_FLAREBINLOGMU].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINLOGMU].cDimension, "none");
  options[OPT_FLAREBINLOGMU].dDefault   = 0;
  options[OPT_FLAREBINLOGMU].iType      = 2;
  options[OPT_FLAREBINLOGMU].bMultiFile = 1;
  options[OPT_FLAREBINLOGMU].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINLOGMU].iFileType  = 1;
  fnRead[OPT_FLAREBINLOGMU]             = &ReadFlareBinLogMu;

  fvFormattedString(&options[OPT_FLAREBINLOGSIGMA].cName, "dFlareBinLogSigma");
  fvFormattedString(&options[OPT_FLAREBINLOGSIGMA].cDescr, "Lognormal sigma in ln(E)");
  fvFormattedString(&options[OPT_FLAREBINLOGSIGMA].cDefault, "1");
  fvFormattedString(&options[OPT_FLAREBINLOGSIGMA].cDimension, "none");
  options[OPT_FLAREBINLOGSIGMA].dDefault   = 1;
  options[OPT_FLAREBINLOGSIGMA].iType      = 2;
  options[OPT_FLAREBINLOGSIGMA].bMultiFile = 1;
  options[OPT_FLAREBINLOGSIGMA].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINLOGSIGMA].iFileType  = 1;
  fnRead[OPT_FLAREBINLOGSIGMA]             = &ReadFlareBinLogSigma;

  fvFormattedString(&options[OPT_FLAREBINRATETOT].cName, "dFlareBinRateTot");
  fvFormattedString(&options[OPT_FLAREBINRATETOT].cDescr, "Total flare rate");
  fvFormattedString(&options[OPT_FLAREBINRATETOT].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINRATETOT].cDimension, "1/time");
  options[OPT_FLAREBINRATETOT].dDefault   = 0;
  options[OPT_FLAREBINRATETOT].iType      = 2;
  options[OPT_FLAREBINRATETOT].bMultiFile = 1;
  options[OPT_FLAREBINRATETOT].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINRATETOT].iFileType  = 1;
  fnRead[OPT_FLAREBINRATETOT]             = &ReadFlareBinRateTot;

  fvFormattedString(&options[OPT_FLAREBINTAU0].cName, "dFlareBinTau0");
  fvFormattedString(&options[OPT_FLAREBINTAU0].cDescr, "Duration scaling tau0");
  fvFormattedString(&options[OPT_FLAREBINTAU0].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINTAU0].cDimension, "time");
  options[OPT_FLAREBINTAU0].dDefault   = 0;
  options[OPT_FLAREBINTAU0].iType      = 2;
  options[OPT_FLAREBINTAU0].bMultiFile = 1;
  options[OPT_FLAREBINTAU0].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINTAU0].iFileType  = 1;
  fnRead[OPT_FLAREBINTAU0]             = &ReadFlareBinTau0;

  fvFormattedString(&options[OPT_FLAREBINDURE0].cName, "dFlareBinDurE0");
  fvFormattedString(&options[OPT_FLAREBINDURE0].cDescr, "Duration scaling E0");
  fvFormattedString(&options[OPT_FLAREBINDURE0].cDefault, "1");
  fvFormattedString(&options[OPT_FLAREBINDURE0].cDimension, "energy");
  options[OPT_FLAREBINDURE0].dDefault   = 1;
  options[OPT_FLAREBINDURE0].iType      = 2;
  options[OPT_FLAREBINDURE0].bMultiFile = 1;
  options[OPT_FLAREBINDURE0].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINDURE0].iFileType  = 1;
  fnRead[OPT_FLAREBINDURE0]             = &ReadFlareBinDurE0;

  fvFormattedString(&options[OPT_FLAREBINDUREXP].cName, "dFlareBinDurExp");
  fvFormattedString(&options[OPT_FLAREBINDUREXP].cDescr, "Duration scaling exponent");
  fvFormattedString(&options[OPT_FLAREBINDUREXP].cDefault, "0");
  fvFormattedString(&options[OPT_FLAREBINDUREXP].cDimension, "none");
  options[OPT_FLAREBINDUREXP].dDefault   = 0;
  options[OPT_FLAREBINDUREXP].iType      = 2;
  options[OPT_FLAREBINDUREXP].bMultiFile = 1;
  options[OPT_FLAREBINDUREXP].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINDUREXP].iFileType  = 1;
  fnRead[OPT_FLAREBINDUREXP]             = &ReadFlareBinDurExp;

  fvFormattedString(&options[OPT_FLAREBINXMIN].cName, "dFlareBinXMin");
  fvFormattedString(&options[OPT_FLAREBINXMIN].cDescr, "Template minimum phase");
  fvFormattedString(&options[OPT_FLAREBINXMIN].cDefault, "-1");
  fvFormattedString(&options[OPT_FLAREBINXMIN].cDimension, "none");
  options[OPT_FLAREBINXMIN].dDefault   = -1;
  options[OPT_FLAREBINXMIN].iType      = 2;
  options[OPT_FLAREBINXMIN].bMultiFile = 1;
  options[OPT_FLAREBINXMIN].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINXMIN].iFileType  = 1;
  fnRead[OPT_FLAREBINXMIN]             = &ReadFlareBinXMin;

  fvFormattedString(&options[OPT_FLAREBINXEND].cName, "dFlareBinXEnd");
  fvFormattedString(&options[OPT_FLAREBINXEND].cDescr, "Template maximum phase");
  fvFormattedString(&options[OPT_FLAREBINXEND].cDefault, "20");
  fvFormattedString(&options[OPT_FLAREBINXEND].cDimension, "none");
  options[OPT_FLAREBINXEND].dDefault   = 20;
  options[OPT_FLAREBINXEND].iType      = 2;
  options[OPT_FLAREBINXEND].bMultiFile = 1;
  options[OPT_FLAREBINXEND].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINXEND].iFileType  = 1;
  fnRead[OPT_FLAREBINXEND]             = &ReadFlareBinXEnd;

  fvFormattedString(&options[OPT_FLAREBINBANDC].cName, "dFlareBinBandC");
  fvFormattedString(&options[OPT_FLAREBINBANDC].cDescr, "Band conversion C");
  fvFormattedString(&options[OPT_FLAREBINBANDC].cDefault, "1");
  fvFormattedString(&options[OPT_FLAREBINBANDC].cDimension, "none");
  options[OPT_FLAREBINBANDC].dDefault   = 1;
  options[OPT_FLAREBINBANDC].iType      = 2;
  options[OPT_FLAREBINBANDC].bMultiFile = 1;
  options[OPT_FLAREBINBANDC].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINBANDC].iFileType  = 1;
  fnRead[OPT_FLAREBINBANDC]             = &ReadFlareBinBandC;

  fvFormattedString(&options[OPT_FLAREBINBANDP].cName, "dFlareBinBandP");
  fvFormattedString(&options[OPT_FLAREBINBANDP].cDescr, "Band conversion p");
  fvFormattedString(&options[OPT_FLAREBINBANDP].cDefault, "1");
  fvFormattedString(&options[OPT_FLAREBINBANDP].cDimension, "none");
  options[OPT_FLAREBINBANDP].dDefault   = 1;
  options[OPT_FLAREBINBANDP].iType      = 2;
  options[OPT_FLAREBINBANDP].bMultiFile = 1;
  options[OPT_FLAREBINBANDP].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINBANDP].iFileType  = 1;
  fnRead[OPT_FLAREBINBANDP]             = &ReadFlareBinBandP;

  fvFormattedString(&options[OPT_FLAREBINFXUVTHRESH1].cName, "dFlareBinFXUVThresh1");
  fvFormattedString(&options[OPT_FLAREBINFXUVTHRESH1].cDescr,
                    "Diagnostic FXUV threshold 1 (<0 disables output, sentinel=-1)");
  fvFormattedString(&options[OPT_FLAREBINFXUVTHRESH1].cDefault, "-1");
  fvFormattedString(&options[OPT_FLAREBINFXUVTHRESH1].cDimension, "energy/time/area");
  options[OPT_FLAREBINFXUVTHRESH1].dDefault   = -1;
  options[OPT_FLAREBINFXUVTHRESH1].iType      = 2;
  options[OPT_FLAREBINFXUVTHRESH1].bMultiFile = 1;
  options[OPT_FLAREBINFXUVTHRESH1].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINFXUVTHRESH1].iFileType  = 1;
  fnRead[OPT_FLAREBINFXUVTHRESH1]             = &ReadFlareBinFXUVThresh1;

  fvFormattedString(&options[OPT_FLAREBINFXUVTHRESH2].cName, "dFlareBinFXUVThresh2");
  fvFormattedString(&options[OPT_FLAREBINFXUVTHRESH2].cDescr,
                    "Diagnostic FXUV threshold 2 (<0 disables output, sentinel=-1)");
  fvFormattedString(&options[OPT_FLAREBINFXUVTHRESH2].cDefault, "-1");
  fvFormattedString(&options[OPT_FLAREBINFXUVTHRESH2].cDimension, "energy/time/area");
  options[OPT_FLAREBINFXUVTHRESH2].dDefault   = -1;
  options[OPT_FLAREBINFXUVTHRESH2].iType      = 2;
  options[OPT_FLAREBINFXUVTHRESH2].bMultiFile = 1;
  options[OPT_FLAREBINFXUVTHRESH2].iModuleBit = FLAREBIN;
  options[OPT_FLAREBINFXUVTHRESH2].iFileType  = 1;
  fnRead[OPT_FLAREBINFXUVTHRESH2]             = &ReadFlareBinFXUVThresh2;
}

void ReadOptionsFlareBin(BODY *body, CONTROL *control, FILES *files,
                         OPTIONS *options, SYSTEM *system,
                         fnReadOption fnRead[], int iBody) {
  int iFile = iBody + 1;
  int iOpt;
  int iLine;

  (void)system;
  for (iOpt = OPTSTARTFLAREBIN; iOpt < OPTENDFLAREBIN; iOpt++) {
    if (options[iOpt].iType != -1 && fnRead[iOpt] != NULL) {
      fnRead[iOpt](body, control, files, &options[iOpt], system, iFile);
    }
  }

  if (body[iBody].dFlareBinEmax < body[iBody].dFlareBinEmin) {
    iLine = options[OPT_FLAREBINEMAX].iLine[iFile];
    if (iLine < 0) {
      iLine = options[OPT_FLAREBINEMIN].iLine[iFile];
    }
    fvFlareBinOptionError(control, files, &options[OPT_FLAREBINEMAX], iFile,
                          iLine, "must be >= dFlareBinEmin.");
  }

  if (body[iBody].dFlareBinEStochMin < body[iBody].dFlareBinEmin) {
    iLine = options[OPT_FLAREBINESTOCHMIN].iLine[iFile];
    if (iLine < 0) {
      iLine = options[OPT_FLAREBINEMIN].iLine[iFile];
    }
    fvFlareBinOptionError(control, files, &options[OPT_FLAREBINESTOCHMIN],
                          iFile, iLine, "must be >= dFlareBinEmin.");
  }

  if (body[iBody].dFlareBinEStochMin > body[iBody].dFlareBinEmax) {
    iLine = options[OPT_FLAREBINESTOCHMIN].iLine[iFile];
    if (iLine < 0) {
      iLine = options[OPT_FLAREBINEMAX].iLine[iFile];
    }
    fvFlareBinOptionError(control, files, &options[OPT_FLAREBINESTOCHMIN],
                          iFile, iLine, "must be <= dFlareBinEmax.");
  }

  if (body[iBody].dFlareBinXEnd <= body[iBody].dFlareBinXMin) {
    iLine = options[OPT_FLAREBINXEND].iLine[iFile];
    if (iLine < 0) {
      iLine = options[OPT_FLAREBINXMIN].iLine[iFile];
    }
    fvFlareBinOptionError(control, files, &options[OPT_FLAREBINXEND], iFile,
                          iLine, "must be > dFlareBinXMin.");
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
  int iFile = iBody + 1;
  int iLine;

  (void)output;
  (void)system;

  if (body[iBody].iBodyType != 0) {
    iLine = options[OPT_BODYTYPE].iLine[iFile];
    if (iLine < 0) {
      iLine = options[OPT_MODULES].iLine[iFile];
    }
    fvFlareBinOptionError(control, files, &options[OPT_BODYTYPE], iFile, iLine,
                          "must be 0 for FLAREBIN (star body only).");
  }

  if (!body[iBody].bStellar) {
    fvFlareBinOptionError(control, files, &options[OPT_MODULES], iFile,
                          options[OPT_MODULES].iLine[iFile],
                          "requires module STELLAR on the same body.");
  }

  if (body[iBody].bFlare) {
    fvFlareBinOptionError(control, files, &options[OPT_MODULES], iFile,
                          options[OPT_MODULES].iLine[iFile],
                          "cannot be used with module FLARE on the same body.");
  }

  if (body[iBody].dFlareBinEmax <= body[iBody].dFlareBinEmin) {
    iLine = options[OPT_FLAREBINEMAX].iLine[iFile];
    if (iLine < 0) {
      iLine = options[OPT_FLAREBINEMIN].iLine[iFile];
    }
    fvFlareBinOptionError(control, files, &options[OPT_FLAREBINEMAX], iFile,
                          iLine, "must be > dFlareBinEmin.");
  }

  if (body[iBody].dFlareBinEStochMin <= body[iBody].dFlareBinEmin) {
    iLine = options[OPT_FLAREBINESTOCHMIN].iLine[iFile];
    if (iLine < 0) {
      iLine = options[OPT_FLAREBINEMIN].iLine[iFile];
    }
    fvFlareBinOptionError(control, files, &options[OPT_FLAREBINESTOCHMIN],
                          iFile, iLine, "must be > dFlareBinEmin.");
  }

  if (body[iBody].dFlareBinEStochMin > body[iBody].dFlareBinEmax) {
    iLine = options[OPT_FLAREBINESTOCHMIN].iLine[iFile];
    if (iLine < 0) {
      iLine = options[OPT_FLAREBINEMAX].iLine[iFile];
    }
    fvFlareBinOptionError(control, files, &options[OPT_FLAREBINESTOCHMIN],
                          iFile, iLine, "must be <= dFlareBinEmax.");
  }

  if (body[iBody].dFlareBinXEnd <= body[iBody].dFlareBinXMin) {
    iLine = options[OPT_FLAREBINXEND].iLine[iFile];
    if (iLine < 0) {
      iLine = options[OPT_FLAREBINXMIN].iLine[iFile];
    }
    fvFlareBinOptionError(control, files, &options[OPT_FLAREBINXEND], iFile,
                          iLine, "must be > dFlareBinXMin.");
  }

  if (body[iBody].dFlareBinTau0 <= 0) {
    fvFlareBinOptionError(control, files, &options[OPT_FLAREBINTAU0], iFile,
                          options[OPT_FLAREBINTAU0].iLine[iFile],
                          "must be > 0.");
  }

  if (body[iBody].dFlareBinDurE0 <= 0) {
    fvFlareBinOptionError(control, files, &options[OPT_FLAREBINDURE0], iFile,
                          options[OPT_FLAREBINDURE0].iLine[iFile],
                          "must be > 0.");
  }

  control->fnForceBehavior[iBody][iModule]   = &fnForceBehaviorFlareBin;
  control->fnPropsAux[iBody][iModule]        = &PropsAuxFlareBin;
  control->Evolve.fnBodyCopy[iBody][iModule] = &BodyCopyFlareBin;

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
  int iE, iX;
  int iNE, iNX;
  double dULo, dUHi;
  double dXMin, dXEnd;

  (void)control;
  (void)update;
  (void)iModule;

  /* User-provided option values are already parsed into BODY before this hook.
   * Initialize only derived cache/storage fields here. */
  body[iBody].dFlareBinLQ                = 0;
  body[iBody].dFlareBinPStoch            = 0;
  body[iBody].dFlareBinMu                = 0;
  body[iBody].dFlareBinLastPrecomputeAge = -1;
  body[iBody].iFlareBinNEnergy           = 0;
  body[iBody].iFlareBinNPhase            = 0;
  body[iBody].daFlareBinQuadU            = NULL;
  body[iBody].daFlareBinQuadWU           = NULL;
  body[iBody].daFlareBinQuadE            = NULL;
  body[iBody].daFlareBinQuadWE           = NULL;
  body[iBody].daFlareBinQuadX            = NULL;
  body[iBody].daFlareBinQuadWX           = NULL;
  body[iBody].daFlareBinTplAtX           = NULL;

  dXMin                       = body[iBody].dFlareBinXMin;
  dXEnd                       = body[iBody].dFlareBinXEnd;
  body[iBody].dFlareBinDeltaX = dXEnd - dXMin;
  body[iBody].dFlareBinITpl   = fdFlareTplIntegral(dXMin, dXEnd);

  /* Initialize caches only when integration bounds are valid; VerifyFlareBin
   * enforces these constraints and reports user-facing errors. */
  if (body[iBody].dFlareBinEStochMin <= 0 ||
      body[iBody].dFlareBinEmax <= body[iBody].dFlareBinEStochMin ||
      body[iBody].dFlareBinDeltaX <= 0 || body[iBody].dFlareBinITpl <= 0 ||
      body[iBody].iFlareBinQuadNE < 1 || body[iBody].iFlareBinQuadNX < 1) {
    return;
  }

  dULo = log(body[iBody].dFlareBinEStochMin);
  dUHi = log(body[iBody].dFlareBinEmax);

  iNE = body[iBody].iFlareBinQuadNE;
  iNX = body[iBody].iFlareBinQuadNX;

  body[iBody].iFlareBinNEnergy = iNE;
  body[iBody].iFlareBinNPhase  = iNX;

  body[iBody].daFlareBinQuadU =
        fdaFlareBinMallocArray(iNE, "daFlareBinQuadU");
  body[iBody].daFlareBinQuadWU =
        fdaFlareBinMallocArray(iNE, "daFlareBinQuadWU");
  body[iBody].daFlareBinQuadE =
        fdaFlareBinMallocArray(iNE, "daFlareBinQuadE");
  body[iBody].daFlareBinQuadWE =
        fdaFlareBinMallocArray(iNE, "daFlareBinQuadWE");

  body[iBody].daFlareBinQuadX =
        fdaFlareBinMallocArray(iNX, "daFlareBinQuadX");
  body[iBody].daFlareBinQuadWX =
        fdaFlareBinMallocArray(iNX, "daFlareBinQuadWX");
  body[iBody].daFlareBinTplAtX =
        fdaFlareBinMallocArray(iNX, "daFlareBinTplAtX");

  if (!fiFlareBinGaussLegendreRule(iNE, dULo, dUHi, body[iBody].daFlareBinQuadU,
                                   body[iBody].daFlareBinQuadWU)) {
    fprintf(stderr, "ERROR: FLAREBIN failed to build log-energy quadrature.\n");
    exit(EXIT_FAILURE);
  }

  for (iE = 0; iE < iNE; iE++) {
    body[iBody].daFlareBinQuadE[iE] = exp(body[iBody].daFlareBinQuadU[iE]);
    body[iBody].daFlareBinQuadWE[iE] =
          body[iBody].daFlareBinQuadWU[iE] * body[iBody].daFlareBinQuadE[iE];
  }

  if (!fiFlareBinGaussLegendreRule(iNX, dXMin, dXEnd, body[iBody].daFlareBinQuadX,
                                   body[iBody].daFlareBinQuadWX)) {
    fprintf(stderr, "ERROR: FLAREBIN failed to build phase quadrature.\n");
    exit(EXIT_FAILURE);
  }

  for (iX = 0; iX < iNX; iX++) {
    body[iBody].daFlareBinTplAtX[iX] =
          fdFlareTpl(body[iBody].daFlareBinQuadX[iX]);
  }

#ifdef DEBUG
  for (iE = 0; iE < iNE; iE++) {
    if (body[iBody].daFlareBinQuadWU[iE] <= 0 ||
        body[iBody].daFlareBinQuadWE[iE] <= 0) {
      fprintf(stderr, "ERROR: FLAREBIN non-positive energy quadrature weight.\n");
      abort();
    }
  }

  for (iX = 0; iX < iNX; iX++) {
    if (body[iBody].daFlareBinQuadWX[iX] <= 0) {
      fprintf(stderr, "ERROR: FLAREBIN non-positive phase quadrature weight.\n");
      abort();
    }
    if (body[iBody].daFlareBinQuadX[iX] < dXMin - 1e-12 ||
        body[iBody].daFlareBinQuadX[iX] > dXEnd + 1e-12) {
      fprintf(stderr, "ERROR: FLAREBIN phase node out of bounds.\n");
      abort();
    }
  }
#endif
}

void InitializeUpdateTmpBodyFlareBin(BODY *body, CONTROL *control, UPDATE *update,
                                     int iBody) {
  (void)body;
  (void)update;

  control->Evolve.tmpBody[iBody].iFlareBinNEnergy = 0;
  control->Evolve.tmpBody[iBody].iFlareBinNPhase  = 0;
  control->Evolve.tmpBody[iBody].daFlareBinQuadU  = NULL;
  control->Evolve.tmpBody[iBody].daFlareBinQuadWU = NULL;
  control->Evolve.tmpBody[iBody].daFlareBinQuadE  = NULL;
  control->Evolve.tmpBody[iBody].daFlareBinQuadWE = NULL;
  control->Evolve.tmpBody[iBody].daFlareBinQuadX  = NULL;
  control->Evolve.tmpBody[iBody].daFlareBinQuadWX = NULL;
  control->Evolve.tmpBody[iBody].daFlareBinTplAtX = NULL;
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

static int fiFlareBinFindUniqueSourceStar(BODY *body, CONTROL *control) {
  int i;
  int iStar = -1;

  for (i = 0; i < control->Evolve.iNumBodies; i++) {
    if (body[i].bStellar && body[i].bFlareBin) {
      if (iStar >= 0) {
        return -1;
      }
      iStar = i;
    }
  }

  return iStar;
}

static int fbFlareBinPlanetDiagContext(BODY *body, CONTROL *control, int iBody,
                                       int *iStar) {
  if (iBody < 0 || iBody >= control->Evolve.iNumBodies || body[iBody].bStellar) {
    return 0;
  }

  *iStar = fiFlareBinFindUniqueSourceStar(body, control);
  return *iStar >= 0;
}

static void fvFlareBinWriteFXUVUnits(OUTPUT *output, int iBody, double *dTmp,
                                     char **cUnit) {
  if (output->bDoNeg[iBody]) {
    *dTmp *= output->dNeg;
    fvFormattedString(cUnit, output->cNeg);
  } else {
    fvFormattedString(cUnit, "W/m^2");
  }
}

static double fdFlareBinIndicatorAboveThreshold(double dFXUV, void *pContext) {
  FLAREBINTHRESHCTX *context = (FLAREBINTHRESHCTX *)pContext;

  if (dFXUV > context->dThreshold) {
    return 1.0;
  }
  return 0.0;
}

static double fdFlareBinClampProbability(double dProb) {
  if (dProb < 0 && fabs(dProb) <= FLAREBIN_PROB_CLIP_TOL) {
    dProb = 0;
  }
  if (dProb > 1.0 && fabs(dProb - 1.0) <= FLAREBIN_PROB_CLIP_TOL) {
    dProb = 1.0;
  }

  if (dProb < 0) {
    dProb = 0;
  }
  if (dProb > 1.0) {
    dProb = 1.0;
  }

  return dProb;
}

static double fdFlareBinProbFXUVAboveThreshold(BODY *body, SYSTEM *system,
                                                int iStar, int iBody,
                                                double dThreshold) {
  FLAREBINTHRESHCTX context;

  context.dThreshold = dThreshold;
  return fdFlareBinClampProbability(fdFlareBinExpectFunction(
      body, system, iStar, iBody, fdFlareBinIndicatorAboveThreshold, &context));
}

/* FLAREBIN writers expose deterministic cached and expectation-based outputs. */
static void WriteLXUVMeanFlareBin(BODY *body, CONTROL *control, OUTPUT *output,
                                  SYSTEM *system, UNITS *units, UPDATE *update,
                                  int iBody, double *dTmp, char **cUnit) {
  *dTmp = body[iBody].dLXUV;

  if (output->bDoNeg[iBody]) {
    *dTmp *= output->dNeg;
    fvFormattedString(cUnit, output->cNeg);
  } else {
    *dTmp /= fdUnitsPower(units->iTime, units->iMass, units->iLength);
    fsUnitsPower(units, cUnit);
  }
}

static void WriteLXUVQuiescentFlareBin(BODY *body, CONTROL *control,
                                       OUTPUT *output, SYSTEM *system,
                                       UNITS *units, UPDATE *update, int iBody,
                                       double *dTmp, char **cUnit) {
  *dTmp = body[iBody].dFlareBinLQ;

  if (output->bDoNeg[iBody]) {
    *dTmp *= output->dNeg;
    fvFormattedString(cUnit, output->cNeg);
  } else {
    *dTmp /= fdUnitsPower(units->iTime, units->iMass, units->iLength);
    fsUnitsPower(units, cUnit);
  }
}

static void WriteFlareBinPStoch(BODY *body, CONTROL *control, OUTPUT *output,
                                SYSTEM *system, UNITS *units, UPDATE *update,
                                int iBody, double *dTmp, char **cUnit) {
  *dTmp = body[iBody].dFlareBinPStoch;

  if (output->bDoNeg[iBody]) {
    *dTmp *= output->dNeg;
    fvFormattedString(cUnit, output->cNeg);
  } else {
    *dTmp /= fdUnitsPower(units->iTime, units->iMass, units->iLength);
    fsUnitsPower(units, cUnit);
  }
}

static void WriteFlareBinMuActive(BODY *body, CONTROL *control, OUTPUT *output,
                                  SYSTEM *system, UNITS *units, UPDATE *update,
                                  int iBody, double *dTmp, char **cUnit) {
  *dTmp = body[iBody].dFlareBinMu;
  fvFormattedString(cUnit, "");
}

static void WriteFlareBinPAnyActive(BODY *body, CONTROL *control, OUTPUT *output,
                                    SYSTEM *system, UNITS *units, UPDATE *update,
                                    int iBody, double *dTmp, char **cUnit) {
  *dTmp = 1.0 - exp(-body[iBody].dFlareBinMu);
  fvFormattedString(cUnit, "");
}

static void WriteFlareBinEStochMin(BODY *body, CONTROL *control, OUTPUT *output,
                                   SYSTEM *system, UNITS *units, UPDATE *update,
                                   int iBody, double *dTmp, char **cUnit) {
  *dTmp = body[iBody].dFlareBinEStochMin;

  if (output->bDoNeg[iBody]) {
    *dTmp *= output->dNeg;
    fvFormattedString(cUnit, output->cNeg);
  } else {
    *dTmp /= fdUnitsEnergy(units->iTime, units->iMass, units->iLength);
    fsUnitsEnergy(units, cUnit);
  }
}

static void WriteFlareBinITemplate(BODY *body, CONTROL *control, OUTPUT *output,
                                   SYSTEM *system, UNITS *units, UPDATE *update,
                                   int iBody, double *dTmp, char **cUnit) {
  *dTmp = body[iBody].dFlareBinITpl;
  fvFormattedString(cUnit, "");
}

static void WriteFXUVMeanFlareBin(BODY *body, CONTROL *control, OUTPUT *output,
                                  SYSTEM *system, UNITS *units, UPDATE *update,
                                  int iBody, double *dTmp, char **cUnit) {
  int iStar;

  (void)units;
  (void)update;
  if (!fbFlareBinPlanetDiagContext(body, control, iBody, &iStar)) {
    *dTmp = FLAREBIN_OUTPUT_SENTINEL_DISABLED;
    fvFormattedString(cUnit, "");
    return;
  }

  *dTmp = fdFlareBinMeanFXUV(body, system, iStar, iBody);
  fvFlareBinWriteFXUVUnits(output, iBody, dTmp, cUnit);
}

static void WriteFXUVQuiescentFlareBin(BODY *body, CONTROL *control,
                                       OUTPUT *output, SYSTEM *system,
                                       UNITS *units, UPDATE *update, int iBody,
                                       double *dTmp, char **cUnit) {
  int iStar;
  double dLbar;
  double dFXUVMean;

  (void)units;
  (void)update;
  if (!fbFlareBinPlanetDiagContext(body, control, iBody, &iStar)) {
    *dTmp = FLAREBIN_OUTPUT_SENTINEL_DISABLED;
    fvFormattedString(cUnit, "");
    return;
  }

  fvFlareBinPrecompute(body, system, iStar, body[iStar].dAge);

  dLbar     = body[iStar].dLXUV;
  dFXUVMean = fdFlareBinMeanFXUV(body, system, iStar, iBody);
  if (dLbar > 0) {
    *dTmp = dFXUVMean * body[iStar].dFlareBinLQ / dLbar;
  } else {
    *dTmp = 0;
  }

  fvFlareBinWriteFXUVUnits(output, iBody, dTmp, cUnit);
}

static void WriteFlareBinPFXUVAbove1(BODY *body, CONTROL *control,
                                     OUTPUT *output, SYSTEM *system,
                                     UNITS *units, UPDATE *update, int iBody,
                                     double *dTmp, char **cUnit) {
  int iStar;
  double dThresh;

  (void)output;
  (void)system;
  (void)units;
  (void)update;
  if (!fbFlareBinPlanetDiagContext(body, control, iBody, &iStar)) {
    *dTmp = FLAREBIN_OUTPUT_SENTINEL_DISABLED;
    fvFormattedString(cUnit, "");
    return;
  }

  dThresh = body[iStar].dFlareBinFXUVThresh1;
  if (dThresh < 0) {
    *dTmp = FLAREBIN_OUTPUT_SENTINEL_DISABLED;
    fvFormattedString(cUnit, "");
    return;
  }

  *dTmp = fdFlareBinProbFXUVAboveThreshold(body, system, iStar, iBody, dThresh);
  fvFormattedString(cUnit, "");
}

static void WriteFlareBinPFXUVAbove2(BODY *body, CONTROL *control,
                                     OUTPUT *output, SYSTEM *system,
                                     UNITS *units, UPDATE *update, int iBody,
                                     double *dTmp, char **cUnit) {
  int iStar;
  double dThresh;

  (void)output;
  (void)system;
  (void)units;
  (void)update;
  if (!fbFlareBinPlanetDiagContext(body, control, iBody, &iStar)) {
    *dTmp = FLAREBIN_OUTPUT_SENTINEL_DISABLED;
    fvFormattedString(cUnit, "");
    return;
  }

  dThresh = body[iStar].dFlareBinFXUVThresh2;
  if (dThresh < 0) {
    *dTmp = FLAREBIN_OUTPUT_SENTINEL_DISABLED;
    fvFormattedString(cUnit, "");
    return;
  }

  *dTmp = fdFlareBinProbFXUVAboveThreshold(body, system, iStar, iBody, dThresh);
  fvFormattedString(cUnit, "");
}

void InitializeOutputFlareBin(OUTPUT *output, fnWriteOutput fnWrite[]) {
  fvFormattedString(&output[OUT_FLAREBINLXUVMEAN].cName, "LXUVMean");
  fvFormattedString(&output[OUT_FLAREBINLXUVMEAN].cDescr,
                    "Baseline stellar XUV luminosity (alias of STELLAR dLXUV)");
  fvFormattedString(&output[OUT_FLAREBINLXUVMEAN].cNeg, "LSUN");
  output[OUT_FLAREBINLXUVMEAN].bNeg       = 1;
  output[OUT_FLAREBINLXUVMEAN].dNeg       = 1. / LSUN;
  output[OUT_FLAREBINLXUVMEAN].iNum       = 1;
  output[OUT_FLAREBINLXUVMEAN].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINLXUVMEAN]           = &WriteLXUVMeanFlareBin;

  fvFormattedString(&output[OUT_FLAREBINLXUVQUIESCENT].cName, "LXUVQuiescent");
  fvFormattedString(&output[OUT_FLAREBINLXUVQUIESCENT].cDescr,
                    "Flarebin quiescent XUV luminosity L_q");
  fvFormattedString(&output[OUT_FLAREBINLXUVQUIESCENT].cNeg, "LSUN");
  output[OUT_FLAREBINLXUVQUIESCENT].bNeg       = 1;
  output[OUT_FLAREBINLXUVQUIESCENT].dNeg       = 1. / LSUN;
  output[OUT_FLAREBINLXUVQUIESCENT].iNum       = 1;
  output[OUT_FLAREBINLXUVQUIESCENT].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINLXUVQUIESCENT]           = &WriteLXUVQuiescentFlareBin;

  fvFormattedString(&output[OUT_FLAREBINPSTOCH].cName, "FlareBinPStoch");
  fvFormattedString(&output[OUT_FLAREBINPSTOCH].cDescr,
                    "Flarebin stochastic mean power P_stoch");
  fvFormattedString(&output[OUT_FLAREBINPSTOCH].cNeg, "LSUN");
  output[OUT_FLAREBINPSTOCH].bNeg       = 1;
  output[OUT_FLAREBINPSTOCH].dNeg       = 1. / LSUN;
  output[OUT_FLAREBINPSTOCH].iNum       = 1;
  output[OUT_FLAREBINPSTOCH].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINPSTOCH]           = &WriteFlareBinPStoch;

  fvFormattedString(&output[OUT_FLAREBINMUACTIVE].cName, "FlareBinMuActive");
  fvFormattedString(&output[OUT_FLAREBINMUACTIVE].cDescr,
                    "Flarebin expected active flare count mu");
  output[OUT_FLAREBINMUACTIVE].bNeg       = 0;
  output[OUT_FLAREBINMUACTIVE].iNum       = 1;
  output[OUT_FLAREBINMUACTIVE].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINMUACTIVE]           = &WriteFlareBinMuActive;

  fvFormattedString(&output[OUT_FLAREBINPANYACTIVE].cName,
                    "FlareBinPAnyActive");
  fvFormattedString(&output[OUT_FLAREBINPANYACTIVE].cDescr,
                    "Flarebin probability at least one flare is active");
  output[OUT_FLAREBINPANYACTIVE].bNeg       = 0;
  output[OUT_FLAREBINPANYACTIVE].iNum       = 1;
  output[OUT_FLAREBINPANYACTIVE].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINPANYACTIVE]           = &WriteFlareBinPAnyActive;

  fvFormattedString(&output[OUT_FLAREBINESTOCHMIN].cName, "FlareBinEStochMin");
  fvFormattedString(&output[OUT_FLAREBINESTOCHMIN].cDescr,
                    "Flarebin stochastic lower energy bound EStochMin");
  fvFormattedString(&output[OUT_FLAREBINESTOCHMIN].cNeg, "ergs");
  output[OUT_FLAREBINESTOCHMIN].bNeg       = 1;
  output[OUT_FLAREBINESTOCHMIN].dNeg       = 1.0e7;
  output[OUT_FLAREBINESTOCHMIN].iNum       = 1;
  output[OUT_FLAREBINESTOCHMIN].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINESTOCHMIN]           = &WriteFlareBinEStochMin;

  fvFormattedString(&output[OUT_FLAREBINITEMPLATE].cName, "FlareBinItemplate");
  fvFormattedString(&output[OUT_FLAREBINITEMPLATE].cDescr,
                    "Flarebin template integral I_tpl");
  output[OUT_FLAREBINITEMPLATE].bNeg       = 0;
  output[OUT_FLAREBINITEMPLATE].iNum       = 1;
  output[OUT_FLAREBINITEMPLATE].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINITEMPLATE]           = &WriteFlareBinITemplate;

  fvFormattedString(&output[OUT_FLAREBINFXUVMEAN].cName, "FXUVMean");
  fvFormattedString(&output[OUT_FLAREBINFXUVMEAN].cDescr,
                    "Planet mean XUV flux convenience output (matches FXUV)");
  fvFormattedString(&output[OUT_FLAREBINFXUVMEAN].cNeg, "W/m^2");
  output[OUT_FLAREBINFXUVMEAN].bNeg       = 1;
  output[OUT_FLAREBINFXUVMEAN].dNeg       = 1;
  output[OUT_FLAREBINFXUVMEAN].iNum       = 1;
  output[OUT_FLAREBINFXUVMEAN].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINFXUVMEAN]           = &WriteFXUVMeanFlareBin;

  fvFormattedString(&output[OUT_FLAREBINFXUVQUIESCENT].cName, "FXUVQuiescent");
  fvFormattedString(&output[OUT_FLAREBINFXUVQUIESCENT].cDescr,
                    "Planet quiescent XUV flux L_q/(4 pi a^2)");
  fvFormattedString(&output[OUT_FLAREBINFXUVQUIESCENT].cNeg, "W/m^2");
  output[OUT_FLAREBINFXUVQUIESCENT].bNeg       = 1;
  output[OUT_FLAREBINFXUVQUIESCENT].dNeg       = 1;
  output[OUT_FLAREBINFXUVQUIESCENT].iNum       = 1;
  output[OUT_FLAREBINFXUVQUIESCENT].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINFXUVQUIESCENT]           = &WriteFXUVQuiescentFlareBin;

  fvFormattedString(&output[OUT_FLAREBINPFXUVABOVE1].cName,
                    "FlareBinPFXUVAbove1");
  fvFormattedString(&output[OUT_FLAREBINPFXUVABOVE1].cDescr,
                    "P(FXUV > dFlareBinFXUVThresh1); -1 when threshold < 0");
  output[OUT_FLAREBINPFXUVABOVE1].bNeg       = 0;
  output[OUT_FLAREBINPFXUVABOVE1].iNum       = 1;
  output[OUT_FLAREBINPFXUVABOVE1].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINPFXUVABOVE1]           = &WriteFlareBinPFXUVAbove1;

  fvFormattedString(&output[OUT_FLAREBINPFXUVABOVE2].cName,
                    "FlareBinPFXUVAbove2");
  fvFormattedString(&output[OUT_FLAREBINPFXUVABOVE2].cDescr,
                    "P(FXUV > dFlareBinFXUVThresh2); -1 when threshold < 0");
  output[OUT_FLAREBINPFXUVABOVE2].bNeg       = 0;
  output[OUT_FLAREBINPFXUVABOVE2].iNum       = 1;
  output[OUT_FLAREBINPFXUVABOVE2].iModuleBit = FLAREBIN;
  fnWrite[OUT_FLAREBINPFXUVABOVE2]           = &WriteFlareBinPFXUVAbove2;
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
  module->fnInitializeUpdateTmpBody[iBody][iModule] = &InitializeUpdateTmpBodyFlareBin;
  module->fnInitializeUpdate[iBody][iModule]    = &InitializeUpdateFlareBin;
  module->fnInitializeOutput[iBody][iModule]    = &InitializeOutputFlareBin;
}
