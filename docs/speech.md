# simcore live speech script

Spoken text only. Italic lines in parentheses are stage cues and are not read
aloud. Numbers are the measured values from the repository: 1.50 s in 9 fixed
steps, 14/14 tests, 1e-9 timing tolerance.

## Segment 1 - Positioning and reuse

**Start 0:00 · Speaker A · 3:00**

*(Slide 1.)*

Good morning. simcore is a C++20 library for discrete-time simulation. It
answers three questions only: which entities exist, how time advances, and how
to iterate entities by type. It does not know what a particle, a drone, or a
temperature sensor is.

*(Slide 2.)*

We deliberately did not build a single executable. A course project that needs
simulation should be able to add simcore with one `add_subdirectory` line,
write its own entity types, and run. The test of reuse is that adding a new
entity type changes only the consumer's files. The library is not recompiled
and its source is not touched. The demo you will see in a minute is one
consumer, not the product.

*(Slide 3. Point at the abstraction boundary diagram.)*

Everything inside the boundary is domain independent. Everything outside is
the consumer's responsibility. The only contract on a user type is one member
function callable as `void update(double dt)`.

*(Open `build/docs/html/index.html`. Click Classes, Class List, Registry, then
spawn. Then Concepts, Entity.)*

These pages are generated from the header comments, not written by hand, so
they cannot drift away from the code. The `doc` target treats Doxygen warnings
as errors; a public member without `@brief` breaks the documentation build.

*(2:45.)*

B will now show the library running and add an entity live.

## Segment 2 - Live functionality demo

**Start 3:00 · Speaker B · 5:00**

*(0:00. Build and run `simcore_demo`; do not narrate keystrokes.)*

Six entities: one box and five particles of two different types in the same
registry. The first run uses a 0.3 second fixed step; 1.0 second is not a
multiple, so the last frame receives the 0.1 second remainder. Expired
particles are despawned inside the view traversal, and the traversal stays
valid because destruction is deferred until the loop ends. The box counts what
is inside it, which proves both entity types received `update`. The summary
reports 1.50 simulated seconds in 9 fixed steps.

*(1:30. Create `examples/thermometer.h`, paste the entity, and save.)*

*(2:10. Add the include to `examples/particle_demo.cpp`.)*

*(2:30. Spawn the thermometer below the box.)*

*(2:50. Print the thermometer state below the farthest line.)*

*(3:10. Rebuild only `simcore_demo` and run it.)*

Only `particle_demo.cpp` recompiled; the library target was already up to date.
20.0 degrees plus 0.5 degrees per second over 1.5 simulated seconds gives
20.75. The entity was picked up by the existing stepper with no registration
code and no changes to simcore.

*(3:50. Run `git status --short` and `git diff --stat`.)*

Adding an entity type touched two consumer files: one new header and one
modified demo file. The library headers, the library source, and the build
files are unchanged. That is the reusability claim in one screen.

*(4:40.)*

C will show how the work is recorded, tested, and documented.

## Segment 3 - Engineering and verification

**Start 8:00 · Speaker C · 2:00**

*(0:00. Run `git log --oneline --decorate -8`; point at the subject lines.)*

Every commit has a type prefix: `chore`, `feat`, `build`, `test`, `docs`.
Each subject names one concern; none says `update` or `fix bug`. A reader can
tell what changed and why without opening the diff.

*(0:40. Run `ctest --test-dir build -C Release --output-on-failure`.)*

Fourteen tests, 14 out of 14 pass. Three cover T1: 1.0 second at 0.1 gives ten
updates and elapsed matches within 1e-9; 0.25 second gives three updates with a
0.05 second final frame; explicit `step` calls accumulate into `elapsed()`. T2:
a particle and a box coexist, both receive the same dt, and each view yields
only its own type. T3: despawning the visited entity keeps the traversal valid;
despawning an unvisited one skips it. T4: stale handles are idempotent,
including after slot reuse. T5: ctest builds a target that must fail to compile
and passes only on the Entity concept diagnostic. Six more exercise the event
bus: delivery is held back until the step ends, publishing outside a step waits
for the next one, listeners are filtered by event type, run in registration
order, stop after unsubscribe, and events a listener publishes are queued for
the following dispatch.

*(1:20. Show the three `WARN_*` lines in `docs/Doxyfile`, then run
`cmake --build build --target doc`.)*

Doxygen reads the public headers; there is no separate API document. The three
warning settings in `docs/Doxyfile` make a missing `@brief` or `@param` a
warning, and `WARN_AS_ERROR` turns a warning into a failed build, so the doc
target cannot succeed with an undocumented public member. Running it now
regenerates the pages from the current headers.

*(1:55.)*

That is how work is recorded, verified, and documented.

*(Open the floor for questions.)*
