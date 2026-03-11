# flarebin

`flarebin` is a VPLanet **star** module that models unresolved stellar XUV flaring as a deterministic flare population (no event-by-event time resolution) and provides **effective (expected-value) forcing** for other modules that depend nonlinearly on XUV flux.

It solves the “minute–hour variability vs Myr timesteps” mismatch by computing expectations like ⟨g(FXUV)⟩ using deterministic quadrature over a flare population model.

## Quickstart

Minimal “star-only” smoke-test style setup (from `tests/Stellar/None/`):

```ini
# vpl.in
sUnitMass kg
sUnitLength m
sUnitTime sec

saBodyFiles flarebin_star.in
```

```ini
# flarebin_star.in
sName star
saModules stellar flarebin
sStellarModel none

dMass 1.0
dRadius 1.0
dLuminosity -1

dFlareBinEmin      1.0
dFlareBinEStochMin 2.0
dFlareBinEmax      3.0
dFlareBinTau0      1000.0
dFlareBinDurE0     2.0
dFlareBinDurExp    0.0
dFlareBinBandC     1.0e-3
dFlareBinBandP     1.0
dFlareBinAlpha     2.0
dFlareBinK         1.0e-8
dFlareBinXMin     -1.0
dFlareBinXEnd     20.0

saOutputOrder Time LXUVMean LXUVQuiescent FlareBinPStoch FlareBinMuActive FlareBinPAnyActive FlareBinEStochMin FlareBinItemplate
```

Minimal “star + planet + ATMESC effective averaging” setup (from `tests/AtmescFlareBinStellar/Minimal/`):

```ini
# vpl.in
sUnitMass solar
sUnitLength AU
sUnitTime years

sSystemName atmesc_flarebin_stellar_min
iVerbose 5
bOverwrite 1

saBodyFiles star.in planet.in
```

```ini
# star.in
sName star
saModules stellar flarebin
sStellarModel baraffe

dMass 0.2
dSatXUVFrac 1.0e-3
dSatXUVTime 0.1

dFlareBinDist POWERLAW_FFD
dFlareBinNormMode NORM_FROM_FFD

dFlareBinEmin      1e26
dFlareBinEStochMin 1e28
dFlareBinEmax      1e30
dFlareBinAlpha     2.0
dFlareBinK         1e-6

dFlareBinTau0   3600.0
dFlareBinDurE0  1e28
dFlareBinDurExp 0.0

dFlareBinXMin -1
dFlareBinXEnd 20

dFlareBinBandC 1.0
dFlareBinBandP 1.0

saOutputOrder Time LXUVTot LXUVMean LXUVMean LXUVQuiescent FlareBinPStoch FlareBinMuActive FlareBinPAnyActive FlareBinEStochMin
```

```ini
# planet.in
sName planet
saModules atmesc

dMass 1.0
dRadius 1.0
dSemi 0.1

dEnvelopeMass 1e19
dSurfWaterMass 1e21

saOutputOrder Time EnvelopeMass FXUV FXUVMean FXUVQuiescent Mdot_XUV_MeanForcing Mdot_XUV_Effective
```

## Requirements / Compatibility

`flarebin` runs on **star bodies only** (`iBodyType` must be `0`). It requires `STELLAR` on the same body (it uses `body[iStar].dLXUV` as the mean XUV luminosity). It is explicitly incompatible with `FLARE` on the same body (hard error during verification).

There are no user-facing ordering constraints in `saModules`, but at runtime VPLanet calls `fvFlareBinPrecompute` after `PropertiesAuxiliary` (see `src/evolve.c`), so `STELLAR` state is expected to be current when `flarebin` caches are refreshed.

For ATMESC coupling: the effective averaging path in `ATMESC` assumes the host star is `body[0]` (single-star convention) when checking `body[0].bFlareBin` and calling `fdFlareBinExpectFunction`.

## Configuration (Inputs)

### A) Star-body options (flarebin)

All `flarebin` inputs are registered as star-body options in `src/flarebin.c` (`InitializeOptionsFlareBin`). Units follow VPLanet’s unit system via the option dimension strings shown below.

| Option name | Type | Default | Units | Valid range / constraints | Description | Notes / interactions |
|---|---:|---:|---|---|---|---|
| `iFlareBinSeed` | int | `0` | none | `>= 0` | Flarebin validation seed | Stored but not used in the production deterministic path (no RNG use found outside parsing/copy). |
| `iFlareBinQuadNE` | int | `24` | none | `>= 1` | Log-energy quadrature nodes | Main accuracy/performance knob for energy integrals and expectations. |
| `iFlareBinQuadNX` | int | `16` | none | `>= 1` | Phase quadrature nodes | Main accuracy/performance knob for template-phase integrals. |
| `iFlareBinMaxOverlapN` | int | `1` | none | `0..2` | Maximum overlap order | `0` uses only the no-active-flare term; `1` adds single-active-flare term; `2` adds pair-overlap correction. |
| `dFlareBinOverlapTol` | double | `1e-6` | none | `>= 0` | Neglected overlap probability tolerance | Used only for a warning when `iFlareBinMaxOverlapN == 1` and `P(N>1)` exceeds tolerance. |
| `sFlareBinDist` | string enum | `POWERLAW_FFD` | none | `POWERLAW_FFD`, `LOGNORMAL` | Flare population distribution selector | `LOGNORMAL` is **not implemented** (runtime abort via `fvFlareBinNotImplemented` in `src/flarebin_ffd.c`). |
| `sFlareBinNormMode` | string enum | `NORM_FROM_FFD` | none | `NORM_FROM_FFD`, `NORM_FROM_FLAREPOWER_FRACTION`, `NORM_FROM_RATE_AT_E0` | Normalization mode selector | `NORM_FROM_FLAREPOWER_FRACTION` is **not implemented**. `NORM_FROM_RATE_AT_E0` has two “anchor” paths (see notes below). |
| `dFlareBinFrac` | double | `0` | none | `>= 0` | Target flare power fraction | Only meaningful for `NORM_FROM_FLAREPOWER_FRACTION` (not implemented). |
| `sFlareBinBandPass` | string enum | `XUV` | none | `XUV`, `KEPLER`, `TESS`, `BOLOMETRIC` | Input flare bandpass selector | Parsed/stored, but no computational use found in provided sources beyond parsing/copy. Conversion is controlled by `dFlareBinBandC/P`. |
| `dFlareBinEmin` | double | `0` | energy | `>= 0`, finite | Minimum flare energy (input band) | Must satisfy `dFlareBinEmax > dFlareBinEmin` (Verify). |
| `dFlareBinEmax` | double | `0` | energy | `>= 0`, finite; `> dFlareBinEmin` | Maximum flare energy (input band) | Upper truncation used by rate density and stochastic integrals. |
| `dFlareBinEStochMin` | double | `0` | energy | `>= 0`, finite; `> dFlareBinEmin`; `<= dFlareBinEmax` | Minimum stochastic flare energy | Sets lower bound of deterministic “stochastic” integrals. Flares below this are only represented through `L_q = L̄XUV - P_stoch` (no variability contribution). Main accuracy/performance knob. |
| `dFlareBinAlpha` | double | `2.0` | none | `>= 0` | Power-law differential index | Used for `POWERLAW_FFD` as `r(E)=k E^{-α}` unless overwritten by `NORM_FROM_RATE_AT_E0` cumulative-anchor path. |
| `dFlareBinK` | double | `0` | 1/time/energy | `>= 0` | Power-law normalization | Meaning depends on `sFlareBinNormMode`: direct `k` for `NORM_FROM_FFD`; or used as an anchor “rate at E0” in one `NORM_FROM_RATE_AT_E0` path (see below). |
| `dFlareBinSlope` | double | `0` | none | none enforced | Cumulative FFD slope | Used only when `sFlareBinNormMode == NORM_FROM_RATE_AT_E0` and `(dFlareBinSlope, dFlareBinYInt)` are not both ~0. Then `α = 1 - slope`, `k = 10^{yint} * (-slope)` in `src/flarebin_ffd.c`. |
| `dFlareBinYInt` | double | `0` | 1/time | none enforced | Cumulative FFD intercept | See `dFlareBinSlope` note. Dimension is registered as `1/time` even though it is used inside `10^{yint}`. |
| `dFlareBinLogMu` | double | `0` | none | none enforced | Lognormal mean in ln(E) | Only relevant for `LOGNORMAL` (not implemented). |
| `dFlareBinLogSigma` | double | `1` | none | `>= 0` | Lognormal sigma in ln(E) | Only relevant for `LOGNORMAL` (not implemented). |
| `dFlareBinRateTot` | double | `0` | 1/time | `>= 0` | Total flare rate | Only relevant for `LOGNORMAL` (not implemented). |
| `dFlareBinTau0` | double | `0` | time | finite; **must be `> 0`** (Verify) | Duration scaling `τ0` | Used in `τ(E) = τ0 * (E / dFlareBinDurE0)^{dFlareBinDurExp}` (`src/flarebin_ffd.c`). |
| `dFlareBinDurE0` | double | `1` | energy | finite; **must be `> 0`** (Verify) | Duration scaling reference energy `E0` | Also used as the energy anchor `E0` in the *differential-anchor* path of `NORM_FROM_RATE_AT_E0` (see below). |
| `dFlareBinDurExp` | double | `0` | none | finite (no explicit bounds) | Duration scaling exponent | See `dFlareBinTau0` note. |
| `dFlareBinXMin` | double | `-1` | none | finite; requires `dFlareBinXEnd > dFlareBinXMin` | Template minimum phase | Template support is `[dFlareBinXMin, dFlareBinXEnd]`. |
| `dFlareBinXEnd` | double | `20` | none | finite; `> dFlareBinXMin` | Template maximum phase | Impacts template normalization `I_tpl` and overlap support width `Δx`. |
| `dFlareBinBandC` | double | `1` | none | `>= 0` | Band conversion coefficient | XUV energy is `E_XUV = dFlareBinBandC * E^{dFlareBinBandP}` (`src/flarebin_ffd.c`). |
| `dFlareBinBandP` | double | `1` | none | `>= 0` | Band conversion exponent | See `dFlareBinBandC`. |
| `dFlareBinFXUVThresh1` | double | `-1` | energy/time/area | none enforced | Diagnostic FXUV threshold #1 | Only used for output `FlareBinPFXUVAbove1`. If `< 0`, output is disabled with sentinel `-1`. |
| `dFlareBinFXUVThresh2` | double | `-1` | energy/time/area | none enforced | Diagnostic FXUV threshold #2 | Only used for output `FlareBinPFXUVAbove2`. If `< 0`, output is disabled with sentinel `-1`. |

Notes on `sFlareBinNormMode == NORM_FROM_RATE_AT_E0` (implemented in `src/flarebin_ffd.c`): if `|dFlareBinSlope| > EPS` **or** `|dFlareBinYInt| > EPS`, flarebin uses the **cumulative-anchor** conversion `α = 1 - slope`, `k = 10^{yint} * (-slope)`. Otherwise it uses a **differential-anchor** fallback that treats `dFlareBinK` as `r(E0)` at `E0 = dFlareBinDurE0`, and computes `k = r(E0) * E0^{α}`.

Main tuning knobs (accuracy/performance): `dFlareBinEStochMin`, `iFlareBinQuadNE`, `iFlareBinQuadNX`, and `iFlareBinMaxOverlapN`.

### B) System/global options (if any exist)

Not found in provided sources (searched `src/flarebin.c`, `src/options.c`, `src/flarebin.h`). All flarebin options are registered with `iFileType = 1` (body file).

### C) Planet-body options related to flarebin diagnostics (if applicable)

Not found in provided sources (searched `src/flarebin.c`, `src/flarebin.h`). Planet-side flarebin-related behavior is output-only (diagnostics), with thresholds configured on the star via `dFlareBinFXUVThresh1/2`.

## Outputs (Results / Diagnostics)

### A) Star-level outputs (flarebin)

| Output name | Scope | Units | Meaning | How it’s computed (short) | Sentinel / disabled behavior |
|---|---|---|---|---|---|
| `LXUVMean` | star | power (supports `-` as `LSUN`) | Mean stellar XUV luminosity used by flarebin | Alias of `body[iStar].dLXUV` (from `STELLAR`) | Not a sentinel output |
| `LXUVQuiescent` | star | power (supports `-` as `LSUN`) | Quiescent/background XUV luminosity `L_q` | Precompute: `L_q = L̄_XUV - P_stoch` | Hard error if `L_q` is significantly negative; small roundoff negatives are floored to 0 |
| `FlareBinPStoch` | star | power (supports `-` as `LSUN`) | Mean stochastic flare power `P_stoch` | `∫_{EStochMin..Emax} r(E) * E_XUV(E) dE` via quadrature | Not a sentinel output |
| `FlareBinMuActive` | star | none | Mean active-flare overlap parameter `μ` | `∫ r(E) * τ(E) * (XEnd-XMin) dE` via quadrature | Not a sentinel output |
| `FlareBinPAnyActive` | star | none | Probability any stochastic flare is active | `1 - exp(-μ)` | Not a sentinel output |
| `FlareBinEStochMin` | star | energy (supports `-` as `ergs`) | Echo of `dFlareBinEStochMin` | Writes `body[iStar].dFlareBinEStochMin` | Not a sentinel output |
| `FlareBinItemplate` | star | none | Template normalization integral `I_tpl` | `fdFlareTplIntegral(XMin, XEnd)` | Not a sentinel output |

### B) Planet-level outputs (flarebin diagnostics, if implemented)

These are registered with `iModuleBit = 1`, so they can be requested on planets that do not have the `flarebin` module enabled.

All planet diagnostics require a unique flarebin star source in the system (exactly one body with `bStellar && bFlareBin`). If not found (or ambiguous), these writers return the sentinel `FLAREBIN_OUTPUT_SENTINEL_DISABLED` (`-1.0`).

| Output name | Scope | Units | Meaning | How it’s computed (short) | Sentinel / disabled behavior |
|---|---|---|---|---|---|
| `FXUVMean` | planet | flux (supports `-` as `W/m^2`) | Convenience output matching `FXUV` | Returns `body[iPlanet].dFXUV` | `-1` if no unique flarebin star source |
| `FXUVQuiescent` | planet | flux (supports `-` as `W/m^2`) | Quiescent flux `F_q = L_q / (4πa²)` | `FXUVMean * (L_q / L̄_XUV)` using flarebin precompute | `-1` if no unique flarebin star source |
| `FlareBinPFXUVAbove1` | planet | none | `P(FXUV > dFlareBinFXUVThresh1)` | Uses `fdFlareBinExpectFunction` with indicator callback and `iFlareBinMaxOverlapN` truncation | `-1` if threshold `<0` or no unique flarebin star source |
| `FlareBinPFXUVAbove2` | planet | none | `P(FXUV > dFlareBinFXUVThresh2)` | Same as above with threshold #2 | `-1` if threshold `<0` or no unique flarebin star source |

### C) ATMESC outputs that compare “mean forcing” vs “effective” (if implemented)

| Output name | Scope | Units | Meaning | How it’s computed (short) | Sentinel / disabled behavior |
|---|---|---|---|---|---|
| `Mdot_XUV_MeanForcing` | planet (ATMESC) | mass/time (supports `-` as `Mearth/Myr`) | Atmospheric escape rate using mean forcing | Computes `AtmEscRhsGivenFXUV(..., dFXUVMean)` and returns `dEnvelopeMassDt` | No sentinel; uses current mean `dFXUV` |
| `Mdot_XUV_Effective` | planet (ATMESC) | mass/time (supports `-` as `Mearth/Myr`) | Atmospheric escape rate using effective (expected) forcing | Returns `*update[iBody].pdDEnvelopeMassDtAtmesc` (the derivative used in integration). When `body[0].bFlareBin`, ATMESC’s RHS components are computed via `fdFlareBinExpectFunction` | No sentinel |

## Functions / API reference (Code-facing)

The table below lists the primary wiring and core APIs (from `src/flarebin.h` plus key integration functions in `ATMESC`). “Determinism / side effects” describes whether the function mutates state or depends on non-deterministic sources.

| Function | Signature | Defined in (file) | Purpose | Called by / lifecycle stage | Determinism / side effects |
|---|---|---|---|---|---|
| `AddModuleFlareBin` | `void AddModuleFlareBin(CONTROL*, MODULE*, int iBody, int iModule)` | `src/flarebin.c` | Register flarebin hooks in VPLanet module table | Module wiring in `src/module.c` | Deterministic; sets function pointers |
| `InitializeOptionsFlareBin` | `void InitializeOptionsFlareBin(OPTIONS*, fnReadOption[])` | `src/flarebin.c` | Register flarebin options and defaults | Global option init in `src/options.c` | Deterministic; writes option metadata |
| `ReadOptionsFlareBin` | `void ReadOptionsFlareBin(BODY*, CONTROL*, FILES*, OPTIONS*, SYSTEM*, fnReadOption[], int iBody)` | `src/flarebin.c` | Parse flarebin options into `BODY` | Input parsing phase | Deterministic; errors on invalid bounds |
| `VerifyFlareBin` | `void VerifyFlareBin(BODY*, CONTROL*, FILES*, OPTIONS*, OUTPUT*, SYSTEM*, UPDATE*, int iBody, int iModule)` | `src/flarebin.c` | Enforce configuration constraints and module compatibility | Verification phase | Deterministic; exits on invalid configs |
| `InitializeBodyFlareBin` | `void InitializeBodyFlareBin(BODY*, CONTROL*, UPDATE*, int iBody, int iModule)` | `src/flarebin.c` | Allocate and build deterministic quadrature caches; compute `Δx` and `I_tpl` | Body initialization | Deterministic; allocates memory; may early-return if bounds invalid |
| `InitializeUpdateTmpBodyFlareBin` | `void InitializeUpdateTmpBodyFlareBin(BODY*, CONTROL*, UPDATE*, int iBody)` | `src/flarebin.c` | Allocate flarebin caches for `tmpBody` and copy baseline grids | RK / tmp-body init | Deterministic; allocates memory; validates no aliasing |
| `BodyCopyFlareBin` | `void BodyCopyFlareBin(BODY *dest, BODY *src, int foo, int iNumBodies, int iBody)` | `src/flarebin.c` | Copy flarebin state and caches from body to tmpBody | Integrator body-copy | Deterministic; copies arrays; errors on size mismatch/alias |
| `InitializeOutputFlareBin` | `void InitializeOutputFlareBin(OUTPUT*, fnWriteOutput[])` | `src/flarebin.c` | Register flarebin outputs and writers | Global output init in `src/output.c` | Deterministic; writes output metadata |
| `PropsAuxFlareBin` | `void PropsAuxFlareBin(BODY*, EVOLVE*, IO*, UPDATE*, int iBody)` | `src/flarebin.c` | Placeholder (no aux props) | PropertiesAux hooks | No-op |
| `AssignFlareBinDerivatives` | `void AssignFlareBinDerivatives(BODY*, EVOLVE*, UPDATE*, fnUpdateVariable***, int iBody)` | `src/flarebin.c` | Placeholder (no evolved variables) | Derivative wiring | No-op |
| `NullFlareBinDerivatives` | `void NullFlareBinDerivatives(BODY*, EVOLVE*, UPDATE*, fnUpdateVariable***, int iBody)` | `src/flarebin.c` | Placeholder | Derivative wiring | No-op |
| `fvFlareBinPrecompute` | `void fvFlareBinPrecompute(BODY*, SYSTEM*, int iStar, double dTimeEval)` | `src/flarebin_effavg.c` | Compute and cache `P_stoch`, `L_q`, `μ` for the current stellar state | Called from `src/evolve.c` cache hook and from `fdFlareBinExpectFunction` | Deterministic; mutates star `BODY` cache fields; exits on invalid energy consistency |
| `fdFlareBinMeanFXUV` | `double fdFlareBinMeanFXUV(BODY*, SYSTEM*, int iStar, int iPlanet)` | `src/flarebin_effavg.c` | Return mean FXUV used as baseline forcing | Used when flarebin disabled or as scaling factor | Deterministic; currently returns `body[iPlanet].dFXUV` |
| `fdFlareBinExpectFunction` | `double fdFlareBinExpectFunction(BODY*, SYSTEM*, int iStar, int iPlanet, double (*fnG)(double, void*), void *ctx)` | `src/flarebin_effavg.c` | Compute ⟨g(FXUV)⟩ under flarebin’s stationary flux model | Called by ATMESC effective RHS and diagnostics | Deterministic; calls precompute; uses quadrature and overlap truncation; no RNG |
| `fdFlareBinExpectAtmEscRhs` | `double fdFlareBinExpectAtmEscRhs(BODY*, SYSTEM*, int iStar, int iPlanet)` | `src/flarebin_effavg.c` | Convenience expectation wrapper | Not used by ATMESC effective RHS in provided sources | Deterministic; uses identity callback (returns ⟨FXUV⟩) |
| `fdFlareTpl` | `double fdFlareTpl(double x)` | `src/flarebin_template.c` | Davenport flare template profile `f(x)` | Used to precompute `tpl(x)` on phase nodes | Deterministic; debug self-check under `DEBUG` |
| `fdFlareTplIntegral` | `double fdFlareTplIntegral(double xMin, double xMax)` | `src/flarebin_template.c` | Analytic integral ∫ f(x) dx over `[xMin, xMax]` | Used to compute `I_tpl` | Deterministic |
| `fdFlareTplIntegralSq` | `double fdFlareTplIntegralSq(double xMin, double xMax)` | `src/flarebin_template.c` | Analytic integral ∫ f(x)² dx over `[xMin, xMax]` | Present for diagnostics/variance-style uses | Deterministic |
| `fdFlareBinEnergyToXUV` | `double fdFlareBinEnergyToXUV(const BODY*, int iStar, double E)` | `src/flarebin_ffd.c` | Convert input-band flare energy to XUV energy | Used in power and amplitude integrands | Deterministic (`C * E^p`) |
| `fdFlareBinDuration` | `double fdFlareBinDuration(const BODY*, int iStar, double E)` | `src/flarebin_ffd.c` | Duration scaling τ(E) | Used in overlap integrands and amplitude normalization | Deterministic; requires `dFlareBinTau0>0`, `dFlareBinDurE0>0` |
| `fdFlareBinRateDensity` | `double fdFlareBinRateDensity(const BODY*, int iStar, double E, double t)` | `src/flarebin_ffd.c` | Differential rate density r(E) | Used for all integrands | Deterministic; `LOGNORMAL` path aborts (not implemented) |
| `fvFlareBinNormalizeFfd` | `void fvFlareBinNormalizeFfd(BODY*, int iStar)` | `src/flarebin_ffd.c` | Apply normalization mode to set/adjust `dFlareBinAlpha`/`dFlareBinK` | Called by precompute | Deterministic; aborts for unimplemented modes |
| `fdFlareBinPowerIntegrand` | `double fdFlareBinPowerIntegrand(const BODY*, int iStar, double E, double t)` | `src/flarebin_ffd.c` | Integrand for mean stochastic power | Used in precompute | Deterministic |
| `fdFlareBinOverlapSupportIntegrand` | `double fdFlareBinOverlapSupportIntegrand(const BODY*, int iStar, double E, double t)` | `src/flarebin_ffd.c` | Integrand for μ (active overlap) | Used in precompute | Deterministic |
| `AtmEscRhsGivenFXUV` | `ATMESC_RHS AtmEscRhsGivenFXUV(const BODY *body, const SYSTEM *system, int iBody, double dFXUV)` | `src/atmesc.c` | Pure ATMESC RHS evaluation at a supplied FXUV | Called inside flarebin expectation callbacks | Deterministic; side-effect free (operates on inputs only) |

## Internal structure (brief)

- `src/flarebin.h`: option/output IDs, enums, sentinel (`FLAREBIN_OUTPUT_SENTINEL_DISABLED`), public prototypes.
- `src/flarebin.c`: module wiring (AddModule/Verify/Initialize), option parsing, output registration, cache allocation for quadrature grids, tmpBody cache allocation/copy, and planet diagnostics writers.
- `src/flarebin_template.c`: Davenport flare template `fdFlareTpl` and analytic integrals `fdFlareTplIntegral`, `fdFlareTplIntegralSq`.
- `src/flarebin_ffd.c`: flare frequency distribution helpers (currently power-law), duration scaling, band conversion, and normalization modes (with explicit “not implemented” aborts for some branches).
- `src/flarebin_effavg.c`: deterministic Gauss–Legendre rule generation, precompute caching (`P_stoch`, `L_q`, `μ`), and the effective-averaging operator `fdFlareBinExpectFunction` with overlap truncation `N_max ∈ {0,1,2}`.
- `src/evolve.c`: `FlareBinPrecomputeCached()` calls `fvFlareBinPrecompute()` after `PropertiesAuxiliary()` for both body state and tmp-body RK stages.
- `src/atmesc.c`: integrates flarebin by computing effective RHS components via `fdFlareBinExpectFunction` when `body[0].bFlareBin`, and registers outputs `Mdot_XUV_MeanForcing` vs `Mdot_XUV_Effective`.

## Determinism, Numerical Controls, Performance

Determinism: flarebin’s production paths use deterministic quadrature and cached computations; no RNG use was found outside option storage (`iFlareBinSeed` is parsed/stored only). The Gauss–Legendre nodes/weights are generated by deterministic Newton iterations (`fiFlareBinGaussLegendreRule`), and the expectation operator is a fixed set of nested loops over quadrature nodes.

Numerical controls: `iFlareBinQuadNE` controls resolution in log-energy over `[ln(dFlareBinEStochMin), ln(dFlareBinEmax)]`. `iFlareBinQuadNX` controls resolution over template phase `[dFlareBinXMin, dFlareBinXEnd]`. `iFlareBinMaxOverlapN` selects the truncation order in the Poisson overlap expansion used by `fdFlareBinExpectFunction` (0/1/2). `dFlareBinOverlapTol` only triggers a warning when `N_max=1` and the neglected probability mass `P(N>1)=1-exp(-μ)(1+μ)` is larger than the tolerance.

Performance guidance: increasing `dFlareBinEStochMin` reduces the effective stochastic population range and typically lowers `μ` and computation cost, but also reduces represented variability (microflares are absorbed into `L_q`). Raising `iFlareBinMaxOverlapN` from 1 to 2 adds a double-integral correction with nested energy/phase loops (cost scales roughly like O(NE²·NX²)). Defaults (`NE=24`, `NX=16`, `N_max=1`) are intended as moderate-accuracy settings; the test suite uses smaller values (e.g. `NE=8`, `NX=8`) for speed.

## Troubleshooting / Common errors

Configuration errors are generally hard exits with messages prefixed `ERROR:`.

If you see `ERROR: ... must be 0 for FLAREBIN (star body only).`, the body running flarebin is not a star (`iBodyType != 0`). Move `flarebin` to the star body.

If you see `ERROR: ... requires module STELLAR on the same body.`, the star’s `saModules` is missing `stellar`. flarebin requires `body[iStar].dLXUV`.

If you see `ERROR: ... cannot be used with module FLARE on the same body.`, remove either `flare` or `flarebin`; they are mutually exclusive on a body.

If you see bound/positivity errors like `dFlareBinEmax must be > dFlareBinEmin.`, `dFlareBinEStochMin must be > dFlareBinEmin.`, `dFlareBinEStochMin must be <= dFlareBinEmax.`, or `dFlareBinXEnd must be > dFlareBinXMin.`, fix the corresponding bounds; Verify enforces strict `>` for `Emax>Emin` and `EStochMin>Emin`.

If you see `dFlareBinTau0 must be > 0.` or `dFlareBinDurE0 must be > 0.`, duration scaling parameters must be strictly positive.

If you see `ERROR: FLAREBIN invalid configuration ... L_q < 0 ... Check dFlareBinK/dFlareBinAlpha and energy bounds (...)`, the stochastic flare power integral `P_stoch` exceeded the stellar mean `L̄_XUV` (`dLXUV`). Reduce the flare population normalization (e.g. `dFlareBinK`) and/or adjust `dFlareBinBandC/P`, `dFlareBinEStochMin`, `dFlareBinEmax`.

If you select `sFlareBinDist LOGNORMAL` or `sFlareBinNormMode NORM_FROM_FLAREPOWER_FRACTION`, flarebin will abort with `FLAREBIN not implemented: ...` because those branches call `fvFlareBinNotImplemented` in `src/flarebin_ffd.c`.

If planet diagnostics outputs (`FXUVMean`, `FXUVQuiescent`, `FlareBinPFXUVAbove1/2`) are `-1`, either no unique flarebin star source was found (must be exactly one `bStellar && bFlareBin` body for those diagnostics), or (for `FlareBinPFXUVAbove*`) the corresponding threshold `dFlareBinFXUVThresh*` is `< 0`.
