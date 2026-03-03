/**
  @file flarebin.h

  @brief Stub declarations for the FLAREBIN module wiring. This header only
  defines IDs and function signatures needed to compile and register the module.
  Physics, quadrature, and effective-averaging operators are implemented later.
*/

#define OPTSTARTFLAREBIN 2400
#define OPTENDFLAREBIN 2500

#define OUTSTARTFLAREBIN 2400
#define OUTENDFLAREBIN 2500

/* @cond DOXYGEN_OVERRIDE */

void AddModuleFlareBin(CONTROL *, MODULE *, int, int);
void BodyCopyFlareBin(BODY *, BODY *, int, int, int);
void InitializeBodyFlareBin(BODY *, CONTROL *, UPDATE *, int, int);
void InitializeUpdateFlareBin(BODY *, UPDATE *, int);

void InitializeOptionsFlareBin(OPTIONS *, fnReadOption[]);
void ReadOptionsFlareBin(BODY *, CONTROL *, FILES *, OPTIONS *, SYSTEM *,
                         fnReadOption[], int);

void InitializeOutputFlareBin(OUTPUT *, fnWriteOutput[]);

void VerifyFlareBin(BODY *, CONTROL *, FILES *, OPTIONS *, OUTPUT *, SYSTEM *,
                    UPDATE *, int, int);
void VerifyHaltFlareBin(BODY *, CONTROL *, OPTIONS *, int, int *);
void CountHaltsFlareBin(HALT *, int *);

void AssignFlareBinDerivatives(BODY *, EVOLVE *, UPDATE *, fnUpdateVariable ***,
                               int);
void NullFlareBinDerivatives(BODY *, EVOLVE *, UPDATE *, fnUpdateVariable ***,
                             int);

void fnForceBehaviorFlareBin(BODY *, MODULE *, EVOLVE *, IO *, SYSTEM *,
                             UPDATE *, fnUpdateVariable ***, int, int);
void PropsAuxFlareBin(BODY *, EVOLVE *, IO *, UPDATE *, int);

void LogOptionsFlareBin(CONTROL *, FILE *);
void LogFlareBin(BODY *, CONTROL *, OUTPUT *, SYSTEM *, UPDATE *,
                 fnWriteOutput[], FILE *);
void LogBodyFlareBin(BODY *, CONTROL *, OUTPUT *, SYSTEM *, UPDATE *,
                     fnWriteOutput[], FILE *, int);

/* @endcond */
