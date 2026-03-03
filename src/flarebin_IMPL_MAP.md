# flarebin Implementation Map (Prompt 1, no code changes)

## Scope + invariants (from `flarebin_design_effective.md`)

- Deterministic RHS only: **no RNG in production RHS evaluation**.
- No time-resolved flare scheduling/events list.
- No flare-driven timestep subdivision or minute-scale substepping.
- ATMESC must consume an effective averaging operator: evaluate `<g(F_XUV)>`, not `g(F_XUV_inst)`.
- Effective averaging uses fixed quadrature (energy + template phase), with cached per-star statistics (`P_stoch`, `L_q`, `mu`).

## New source files to add

- `src/flarebin.h`
- `src/flarebin.c`
- `src/flarebin_effavg.c`
- `src/flarebin_template.c`
- `src/flarebin_ffd.c`

## Insertion-point map (existing files)

### `src/vplanet.h`

- [ ] Add module bit constant `FLAREBIN` in module bit block (`#define EQTIDE ...`).
- [ ] Extend `BODY` with `bFlareBin` and flarebin config/cache fields (star-side + optional planet diagnostics caches).
- [ ] Extend `MODULE` with `iaFlareBin` index array.
- [ ] Add flarebin option/output numeric range and update `MODULEOPTEND` / `MODULEOUTEND` capacity as needed.
- [ ] Add `#include "flarebin.h"` in module include list.
- [ ] Add flarebin output IDs / option IDs (via `flarebin.h` include conventions).

### `src/module.c`

Anchors found: `InitializeModule`, `FinalizeModule`, `AddModules`, `ReadModules`, `PrintModuleList`, `InitializeBodyModules`, `VerifyModuleMulti`.

- [ ] Allocate/init `module->iaFlareBin` in `InitializeModule`.
- [ ] Include `bFlareBin` in module counts in `FinalizeModule`.
- [ ] Parse `"flarebin"` in `ReadModules` (set `body[i].bFlareBin` + bit sum).
- [ ] Add `FLAREBIN` string printing in `PrintModuleList`.
- [ ] Set default `bFlareBin=0` in `InitializeBodyModules`.
- [ ] Register module in `AddModules` with `AddModuleFlareBin(...)`.
- [ ] Enforce dependency/order in multi-module verification: STELLAR required before FLAREBIN (new `VerifyModuleMultiFlareBinStellar` hook).

### `src/module.h`

- [ ] Add forward declaration(s) for any new multi-module helper used from `module.c` (if non-static).

### `src/options.c`

Anchor found: `InitializeOptions`.

- [ ] Add `InitializeOptionsFlareBin(options, fnRead);` in module init chain.

### `src/output.c`

Anchor found: `InitializeOutput`.

- [ ] Add `InitializeOutputFlareBin(output, fnWrite);` in module output init chain.

### `src/evolve.c`

Anchors found: `PropertiesAuxiliary`, `RungeKutta4Step`, `Evolve` main loop.

- [ ] Add pre-step flarebin cache hook for accepted-step evaluation (`fvFlareBinPrecompute(...)` per star).
- [ ] Ensure cache call timing is deterministic and avoids redundant recomputation across RK4 stages unless explicitly stage-evaluated.
- [ ] Keep integrator behavior unchanged otherwise (no dt subdivision logic).

### `src/atmesc.h`

- [ ] Add RHS helper type(s) for effective averaging integration (e.g., `ATMESC_RHS` container and callback typedef).
- [ ] Add prototypes for flux-parameterized ATMESC RHS function(s).
- [ ] Add output IDs/prototypes for effective-vs-mean forcing diagnostics (recommended outputs section).

### `src/atmesc.c`

Anchors found: `fnPropsAuxAtmEsc`, `fdDEnvelopeMassDt`, `fdDEnvelopeMassDtRRLimited`, `fdDSurfaceWaterMassDt`, `fdDOxygenMassDt`, `InitializeOutputAtmEsc`.

- [ ] Factor ATMESC flux-dependent physics into pure RHS evaluation function(s) of `dFXUV` (no side effects).
- [ ] Replace direct instantaneous-flux use in derivative path with flarebin expectation operator when enabled.
- [ ] Preserve legacy path when flarebin is off.
- [ ] Add and wire recommended outputs:
  - `Mdot_XUV_MeanForcing`
  - `Mdot_XUV_Effective`

### `src/system.c`

Anchors found: `fdXUVFlux`, `fvCumulativeXUVFlux`.

- [ ] Keep baseline behavior for effective-averaging mode (design says no mandatory change).
- [ ] Verify no hidden FLARE-only assumptions conflict with FLAREBIN-enabled ATMESC path.

### `src/stellar.c` / `src/stellar.h`

- [ ] No planned code change for effective mode (design: STELLAR continues providing baseline `dLXUV = Lbar_XUV`).
- [ ] Verify only that flarebin reads STELLAR baseline consistently.

## Function inventory to implement (names + purpose)

### `src/flarebin.c` (module glue)

- `void AddModuleFlareBin(CONTROL *, MODULE *, int iBody, int iModule);`  
  Register flarebin callbacks in module tables.
- `void BodyCopyFlareBin(BODY *dest, BODY *src, int foo, int iNumBodies, int iBody);`  
  Copy flarebin state during temp-body workflows.
- `void InitializeOptionsFlareBin(OPTIONS *, fnReadOption[]);`  
  Define flarebin options metadata + read handlers.
- `void ReadOptionsFlareBin(BODY *, CONTROL *, FILES *, OPTIONS *, SYSTEM *, fnReadOption[], int iBody);`  
  Per-body option ingestion.
- `void VerifyFlareBin(BODY *, CONTROL *, FILES *, OPTIONS *, OUTPUT *, SYSTEM *, UPDATE *, int iBody, int iModule);`  
  Validate configuration and set module function pointers.
- `void InitializeBodyFlareBin(BODY *, CONTROL *, UPDATE *, int iBody, int iModule);`  
  Allocate/init flarebin body data.
- `void InitializeUpdateFlareBin(BODY *, UPDATE *, int iBody);`  
  Register any flarebin update variables (likely none for effective mode).
- `void fnForceBehaviorFlareBin(BODY *, MODULE *, EVOLVE *, IO *, SYSTEM *, UPDATE *, fnUpdateVariable ***, int iBody, int iModule);`  
  Runtime enforcement (primarily sanity/no-op for deterministic effective mode).
- `void fnPropsAuxFlareBin(BODY *, EVOLVE *, IO *, UPDATE *, int iBody);`  
  Auxiliary per-body flarebin updates as needed.
- `void InitializeOutputFlareBin(OUTPUT *, fnWriteOutput[]);`  
  Register flarebin outputs.
- `void WriteLXUVMean(...);`  
  Write `Lbar_XUV` diagnostic.
- `void WriteLXUVQuiescent(...);`  
  Write `L_q` diagnostic.
- `void WriteFlareBinPStoch(...);`  
  Write stochastic power diagnostic.
- `void WriteFlareBinMuActive(...);`  
  Write active-overlap mean `mu`.
- `void WriteFlareBinPAnyActive(...);`  
  Write `1-exp(-mu)`.
- `void WriteFlareBinEStochMin(...);`  
  Write stochastic-range lower energy bound.
- `void WriteFlareBinItemplate(...);`  
  Write template integral.
- `void LogOptionsFlareBin(CONTROL *, FILE *);`  
  Log global flarebin options.
- `void LogFlareBin(BODY *, CONTROL *, OUTPUT *, SYSTEM *, UPDATE *, fnWriteOutput[], FILE *);`  
  Log global flarebin state.
- `void LogBodyFlareBin(BODY *, CONTROL *, OUTPUT *, SYSTEM *, UPDATE *, fnWriteOutput[], FILE *, int iBody);`  
  Log body-level flarebin state.

### `src/flarebin_effavg.c` (deterministic averaging core)

- `void fvFlareBinPrecompute(BODY *body, SYSTEM *system, int iStar, double dTimeEval);`  
  Per-step cache update (`Lbar_XUV`, `P_stoch`, `L_q`, `mu`, cached factors).
- `double fdFlareBinComputeMu(const BODY *body, int iStar);`  
  Deterministic quadrature for overlap mean `mu`.
- `double fdFlareBinComputePStoch(const BODY *body, int iStar);`  
  Deterministic quadrature for stochastic flare power.
- `double fdFlareBinExpectG(const BODY *body, const SYSTEM *system, int iStar, int iPlanet, double (*fnG)(const BODY *, const SYSTEM *, int, double, void *), void *pvCtx);`  
  Generic expected-value operator `<g(F_XUV)>` using fixed quadrature + Poisson-mixture truncation.
- `ATMESC_RHS FlareBinExpectRhs_AtmEsc(const BODY *body, const SYSTEM *system, int iStar, int iBody, FN_ATMESC_RHS fn);`  
  ATMESC-callback expectation bridge.
- `ATMESC_RHS FlareBinExpectAtmEscRhs(const BODY *body, const SYSTEM *system, int iStar, int iBody);`  
  ATMESC-specialized expectation shortcut.

### `src/flarebin_template.c` (Davenport template)

- `double fdFlareTpl(double x);`  
  Davenport time-profile evaluation.
- `double fdFlareTplIntegral(double xMin, double xMax);`  
  Deterministic template integral.
- `double fdFlareTplIntegralSq(double xMin, double xMax);`  
  Optional squared-template integral for diagnostics/approximations.

### `src/flarebin_ffd.c` (FFD + moments)

- `double fdFlareBinRateDensity(const BODY *body, int iStar, double dEin, double dTimeEval);`  
  Differential flare rate `r(E)`.
- `double fdFlareBinDuration(const BODY *body, int iStar, double dEin);`  
  Duration scaling `tau(E)`.
- `double fdFlareBinEnergyToXUV(const BODY *body, int iStar, double dEin);`  
  Band conversion `E_XUV(E_in)`.
- `double fdFlareBinTemplateAmp(const BODY *body, int iStar, double dEin);`  
  Amplitude `A(E)=E_XUV/(tau*I_tpl)`.
- `void fvFlareBinNormalizeFfd(BODY *body, int iStar);`  
  Apply normalization policy (including optional renormalization if configured).
- `double fdFlareBinCheckFlarePower(const BODY *body, int iStar, double dTimeEval);`  
  Compute flare power for consistency checks vs STELLAR baseline.

### `src/atmesc.c` (effective-operator integration points)

- `ATMESC_RHS AtmEscRhsGivenFXUV(const BODY *body, const SYSTEM *system, int iBody, double dFXUV);`  
  Side-effect-free ATMESC RHS at fixed flux.
- `double fdDEnvelopeMassDt_Effective(BODY *body, SYSTEM *system, int *iaBody);`  
  Envelope derivative using `<g(F_XUV)>`.
- `double fdDSurfaceWaterMassDt_Effective(BODY *body, SYSTEM *system, int *iaBody);`  
  Water derivative using `<g(F_XUV)>`.
- `double fdDOxygenMassDt_Effective(BODY *body, SYSTEM *system, int *iaBody);`  
  Oxygen derivative using `<g(F_XUV)>`.
- `double fdDOxygenMantleMassDt_Effective(BODY *body, SYSTEM *system, int *iaBody);`  
  Mantle oxygen derivative using `<g(F_XUV)>`.

(Exact wrapper naming can be finalized in implementation, but this split is required.)

## Ordering constraint (must enforce)

- **STELLAR must run before FLAREBIN.**
- Practical enforcement points:
  - `src/module.c:AddModules`: place `AddModuleFlareBin` after `AddModuleStellar`.
  - `src/module.c:VerifyModuleMulti*`: fail input if `bFlareBin && !bStellar` for the same star body.
  - FLAREBIN precompute reads `body[iStar].dLXUV` as STELLAR baseline (`Lbar_XUV`).

## Notes about non-goals in effective mode

- No event queue, no `next flare` timestamp logic.
- No random energy/event draws during ODE integration.
- No per-flare logging/scheduling state.

