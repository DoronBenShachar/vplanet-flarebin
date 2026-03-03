/**
  @file flarebin.h

  @brief Public declarations for FLAREBIN module wiring and state layout IDs.
  Physics, quadrature, and effective-averaging implementations are added later.
*/

/* Option IDs */
#define OPTSTARTFLAREBIN 2400
#define OPTENDFLAREBIN 2500

/* FLAREBIN 2400-2499 */
#define OPT_FLAREBINSEED 2410
#define OPT_FLAREBINQUADNE 2411
#define OPT_FLAREBINQUADNX 2412
#define OPT_FLAREBINMAXOVERLAPN 2413
#define OPT_FLAREBINOVERLAPTOL 2414

#define OPT_FLAREBINDIST 2420
#define OPT_FLAREBINNORMMODE 2421
#define OPT_FLAREBINFRAC 2422

#define OPT_FLAREBINBANDPASS 2430
#define OPT_FLAREBINEMIN 2431
#define OPT_FLAREBINEMAX 2432
#define OPT_FLAREBINESTOCHMIN 2433

#define OPT_FLAREBINALPHA 2440
#define OPT_FLAREBINK 2441
#define OPT_FLAREBINSLOPE 2442
#define OPT_FLAREBINYINT 2443
#define OPT_FLAREBINLOGMU 2444
#define OPT_FLAREBINLOGSIGMA 2445
#define OPT_FLAREBINRATETOT 2446

#define OPT_FLAREBINTAU0 2450
#define OPT_FLAREBINDURE0 2451
#define OPT_FLAREBINDUREXP 2452

#define OPT_FLAREBINXMIN 2460
#define OPT_FLAREBINXEND 2461

#define OPT_FLAREBINBANDC 2470
#define OPT_FLAREBINBANDP 2471

#define OPT_FLAREBINFXUVTHRESH1 2480
#define OPT_FLAREBINFXUVTHRESH2 2481

/* Output IDs */
#define OUTSTARTFLAREBIN 2400
#define OUTENDFLAREBIN 2500

#define OUT_FLAREBINLXUVMEAN 2410
#define OUT_FLAREBINLXUVQUIESCENT 2411
#define OUT_FLAREBINPSTOCH 2412
#define OUT_FLAREBINMUACTIVE 2413
#define OUT_FLAREBINPANYACTIVE 2414
#define OUT_FLAREBINESTOCHMIN 2415
#define OUT_FLAREBINITEMPLATE 2416

#define OUT_FLAREBINFXUVMEAN 2420
#define OUT_FLAREBINFXUVQUIESCENT 2421
#define OUT_FLAREBINPFXUVABOVE1 2422
#define OUT_FLAREBINPFXUVABOVE2 2423

/* Diagnostic sentinel used when an optional flarebin output is disabled. */
#define FLAREBIN_OUTPUT_SENTINEL_DISABLED (-1.0)

/* Distribution type */
typedef enum {
  FLAREBIN_DIST_POWERLAW = 0,
  FLAREBIN_DIST_LOGNORMAL = 1
} FLAREBINDIST;

/* Normalization mode */
typedef enum {
  FLAREBIN_NORM_FROM_FFD = 0,
  FLAREBIN_NORM_FROM_FLAREPOWER_FRACTION = 1,
  FLAREBIN_NORM_FROM_RATE_AT_E0 = 2
} FLAREBINNORMMODE;

/* Overlap truncation N_max mode */
typedef enum {
  FLAREBIN_OVERLAP_N0 = 0,
  FLAREBIN_OVERLAP_N1 = 1,
  FLAREBIN_OVERLAP_N2 = 2
} FLAREBINOVERLAPMODE;

/* Bandpass selector */
#define FLAREBIN_BANDPASS_XUV 0
#define FLAREBIN_BANDPASS_KEPLER 1
#define FLAREBIN_BANDPASS_TESS 2
#define FLAREBIN_BANDPASS_BOLOMETRIC 3

/* @cond DOXYGEN_OVERRIDE */

void AddModuleFlareBin(CONTROL *, MODULE *, int, int);
void BodyCopyFlareBin(BODY *, BODY *, int, int, int);
void InitializeBodyFlareBin(BODY *, CONTROL *, UPDATE *, int, int);
void InitializeUpdateTmpBodyFlareBin(BODY *, CONTROL *, UPDATE *, int);
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

/* Public API used by other modules (implemented in later prompts) */
void fvFlareBinPrecompute(BODY *, SYSTEM *, int, double);
double fdFlareBinMeanFXUV(BODY *, SYSTEM *, int, int);
double fdFlareBinExpectFunction(BODY *, SYSTEM *, int, int,
                                double (*)(double, void *), void *);
double fdFlareBinExpectAtmEscRhs(BODY *, SYSTEM *, int, int);

/* Davenport template helpers */
double fdFlareTpl(double);
double fdFlareTplIntegral(double, double);
double fdFlareTplIntegralSq(double, double);

/* FFD and conversion helpers */
double fdFlareBinEnergyToXUV(const BODY *, int, double);
double fdFlareBinDuration(const BODY *, int, double);
double fdFlareBinRateDensity(const BODY *, int, double, double);
void fvFlareBinNormalizeFfd(BODY *, int);
double fdFlareBinPowerIntegrand(const BODY *, int, double, double);
double fdFlareBinOverlapIntegrand(const BODY *, int, double, double);
double fdFlareBinOverlapSupportIntegrand(const BODY *, int, double, double);

/* @endcond */
