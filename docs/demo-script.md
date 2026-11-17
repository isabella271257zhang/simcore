# simcore classroom demo script

Total duration: 12-14 minutes, hard limit 15 minutes. Three speakers: A, B, C.
The script follows section 7 of the project requirements: positioning (about
3 minutes), live functionality demo (about 5 minutes), engineering and
collaboration (about 3 minutes), then Q&A.

Replace `A`, `B`, `C` with real names and `<PR-number>` with the pull request
used in your repository before rehearsing.

## Screen layout before starting

- Terminal 1: repository root, font at least 18 pt.
- Editor: `examples/particle_demo.cpp` and `examples/thermometer.h` tabs.
- Browser tab 1: `build/docs/html/index.html`.
- Browser tab 2: the GitHub/GitLab repository page, logged in.
- Slides in presentation mode on the second display.

## Pre-demo checklist (T-15 minutes)

1. Build everything and run the demo once:
   `cmake --build build --config Release` then run `simcore_demo`.
2. Generate the API pages: `cmake --build build --target doc`, then open
   `build/docs/html/index.html` and confirm the Registry page loads.
3. Run `ctest --test-dir build -C Release --output-on-failure` once so the
   GoogleTest download is cached and the total time is known.
4. Run `git status --short` and start from a clean tree.
5. Keep a backup copy of the edited `particle_demo.cpp` outside the repository
   in case the live edit goes wrong.
6. Disable notifications and screen savers. Close unrelated browser tabs.

---

## Segment 1 - Positioning and reuse (3:00) - Speaker A

### Slide 1 (0:00-0:30): "simcore - a simulation core, not a simulation"

Say:

> Good morning. simcore is a C++20 library for discrete-time simulation. It
> answers three questions only: which entities exist, how time advances, and how
> to iterate entities by type. It does not know what a particle, a drone, or a
> temperature sensor is.

### Slide 2 (0:30-1:15): "Why a library, not an application?"

Say:

> We deliberately did not build a single executable. A course project that needs
> simulation should be able to add simcore with one `add_subdirectory` line,
> write its own entity types, and run. The test of reuse is that adding a new
> entity type changes only the consumer's files. The library is not recompiled
> and its source is not touched. The demo you will see in a minute is one
> consumer, not the product.

### Slide 3 (1:15-1:50): abstraction boundary

Point at the diagram: inside the boundary are `Registry`, `Stepper`, `View<T>`,
`EntityHandle<T>`, and the `Entity` concept; outside are user entity types,
user `main`, and domain rules.

Say:

> Everything inside the boundary is domain independent. Everything outside is
> the consumer's responsibility. The only contract on a user type is one member
> function callable as `void update(double dt)`.

### Doxygen walkthrough (1:50-2:45)

Open `build/docs/html/index.html`.

1. Click **Classes** in the top menu, then **Class List**, then **Registry**.
2. Point at the **Public Member Functions** table and read the four API names:
   `spawn`, `despawn`, `view`, `entity_count`.
3. Click **spawn()** and show the documented `@tparam`, `@param`, and the
   embedded `@code` example.
4. Go back, click **Concepts** in the top menu, then **Entity**. Show the
   `update` requirement.
5. Click **Files**, open `registry.h`, and scroll to the declaration that
   produced the page.

Say:

> These pages are generated from the header comments, not written by hand, so
> they cannot drift away from the code. The `doc` target treats Doxygen warnings
> as errors; a public member without `@brief` breaks the documentation build.

Hand-off (2:45-3:00):

> B will now show the library running and add an entity live.

---

## Segment 2 - Live functionality demo (5:00) - Speaker B

### Build and run (0:00-1:30)

Terminal at the repository root. Type the commands without narrating keystrokes.

Windows (MinGW, the verified configuration):

```bat
cmake --build build --target simcore_demo --config Release
.\build\examples\simcore_demo.exe
```

Linux/macOS:

```sh
cmake --build build --target simcore_demo
./build/examples/simcore_demo
```

Expected output (do not read all of it; point at lines):

```text
simcore particle demo
---------------------
after spawning             | elapsed= 0.00 s | entities=6
after run(1.0, 0.3)        | elapsed= 1.00 s | entities=6
removed 2 expired particles during view traversal
after cleanup              | elapsed= 1.00 s | entities=4
after run(0.5, 0.1)        | elapsed= 1.50 s | entities=4
---------------------
simulated time : 1.50 s in 9 fixed steps
particles alive: 3 (removed 2)
inside box now : 3
farthest |x|   : 1.75 (half width 5.0)
```

Say while pointing:

> Six entities: one box and five particles of two different types in the same
> registry. The first run uses a 0.3 second fixed step; 1.0 second is not a
> multiple, so the last frame receives the 0.1 second remainder. Expired
> particles are despawned inside the view traversal, and the traversal stays
> valid because destruction is deferred until the loop ends. The box counts what
> is inside it, which proves both entity types received `update`.

### Add a new entity type live (1:30-3:10)

**Step 1 - create the file (1:30-2:10).**

In the editor, right-click the `examples` folder, choose **New File**, name it
`thermometer.h`, and paste exactly:

```cpp
#pragma once

struct Thermometer {
    double celsius{20.0};
    void update(double dt) { celsius += 0.5 * dt; }
};
```

Save the file (Ctrl+S). Confirm the tab path ends with `examples/thermometer.h`.

**Step 2 - include it (2:10-2:30).**

Switch to `examples/particle_demo.cpp`, find the line
`#include <vector>` and insert below it, leaving one blank line between the
system includes and the local include:

```cpp
#include "thermometer.h"
```

**Step 3 - spawn it (2:30-2:50).**

Find the exact line:

```cpp
    auto box = registry.spawn<BoundedBox>(registry, kHalfWidth);
```

Insert directly below it:

```cpp
    auto thermometer = registry.spawn<Thermometer>();
```

**Step 4 - print its state (2:50-3:10).**

Find the exact line:

```cpp
    std::printf("farthest |x|   : %.2f (half width %.1f)\n", farthest, kHalfWidth);
```

Insert directly below it:

```cpp
    std::printf("thermometer    : %.2f C\n", thermometer->celsius);
```

### Rebuild and run (3:10-3:50)

```bat
cmake --build build --target simcore_demo --config Release
.\build\examples\simcore_demo.exe
```

On Linux/macOS omit `--config Release` and run
`./build/examples/simcore_demo`.

Point at the build log first: it shows `Built target simcore` without compiling
the library, then compiles only `particle_demo.cpp`. The new output line is:

```text
thermometer    : 20.75 C
```

Say:

> 20.0 degrees plus 0.5 degrees per second over 1.5 simulated seconds gives
> 20.75. The entity was picked up by the existing stepper with no registration
> code and no changes to simcore.

### Prove the scope of the change (3:50-4:40)

In the terminal:

```sh
git status --short
git diff --stat
```

Point at the output: only `examples/particle_demo.cpp` is modified and
`examples/thermometer.h` is new. Nothing under `include/simcore/` or `src/`
appears.

Say:

> Adding an entity type touched two consumer files. The library headers, the
> library source, and the build files are unchanged. That is the reusability
> claim in one screen.

Hand-off (4:40-5:00):

> C will show how this is built, tested, and reviewed.

### Rehearsal pitfalls in this segment

| Risk | Symptom | Prevention |
| --- | --- | --- |
| File saved to the repository root or `build/` | `fatal error: thermometer.h: No such file or directory` | Check the editor tab path says `examples/thermometer.h` before saving |
| Forgot to save before building | Build succeeds but the new line is missing | Use File > Save All, then confirm the log recompiles `particle_demo.cpp` |
| Ran the old executable | Output has no thermometer line | Always rebuild the target first; wait for `Built target simcore_demo` |
| Editor working directory is `build/` | Commands resolve relative paths incorrectly | Keep the terminal at the repository root |
| Multi-config generator | `build\examples\simcore_demo.exe` does not exist | Use `build\examples\Release\simcore_demo.exe` for Visual Studio/Xcode |
| Doxygen page not generated | Browser shows 404 | Run the `doc` target during setup, not live |
| First configure needs the internet | GoogleTest download stalls | Pre-build before class; the cache is reused |
| Typo in the pasted entity | Concept static assertion appears | Keep the backup file ready; replace the file and rebuild |

If the live compile fails, do not debug on stage. Copy the backup
`particle_demo.cpp` over the edited one, recreate `thermometer.h` from the slide,
rebuild, and continue with the prepared output.

---

## Segment 3 - Engineering and collaboration (3:00) - Speaker C

### Branch and merge history (0:00-1:10)

Terminal:

```sh
git log --graph --oneline --decorate --all -25
```

Say:

> Every feature branch was cut from main and merged through a pull request.
> Green lanes are merges, not direct pushes to main.

Switch to the repository page:

1. Click **Insights**, then **Network** in the left sidebar.
2. Hover over two or three branches and point at the merge points.

Say:

> main is protected. Each branch is named `feature/<name>-<topic>` and carries
> one feature or one fix.

### One pull request under review (1:10-2:20)

1. Click **Pull requests**, open `<PR-number>` (use the real one, for example
   the deferred-despawn PR).
2. Point at the title, the linked issue or task, and the commit list.
3. Click **Files changed**; scroll to a concrete hunk and read one review
   comment written by another member.
4. Click **Review changes**, select **Approve**, and submit.
5. Click **Merge pull request**, then **Confirm merge**.

Say:

> The rule is one approval from another member before merging, and every review
> leaves a comment. The commit messages follow `type: summary`, for example
> `feat: defer entity destruction until iteration ends`.

If the classroom account cannot merge, stop after the approval dialog and say
the merge was performed during development; show the merged state in the commit
graph instead.

### Tests and docs (2:20-3:00)

Terminal:

```sh
ctest --test-dir build -C Release --output-on-failure
```

Say:

> Eight tests, all green. They cover the required cases: fixed-step timing and
> the remainder frame, heterogeneous entities, despawn during a view, stale
> handle behaviour, and the compile-time entity constraint. The last test is
> unusual: it builds a target that must fail to compile, and passes only if the
> compiler prints the Entity concept diagnostic.

Optionally run `cmake --build build --target doc` and say:

> The documentation target regenerates the API pages from header comments and
> fails on undocumented public members.

Hand-off to Q&A.

---

## Q&A preparation

| Likely question | Answer outline |
| --- | --- |
| Why a library instead of an application? | Domain independence; consumers add `add_subdirectory` and write only their entity types; the demo is one consumer, not the product. |
| Why type erasure instead of `std::variant`? | Users define an open set of entity types; `variant` requires every type to be listed in the library. Type erasure keeps the library closed for modification. |
| How is type safety preserved after erasure? | `spawn<T>` returns `EntityHandle<T>`; the handle checks the runtime `typeid` and only resolves to `T`. Users never cast. |
| Why is despawn deferred? | It keeps views and update loops valid when an entity destroys itself or another entity. Visibility and `entity_count()` change immediately; storage is released when the pass ends. |
| What happens with a stale handle? | `despawn` is idempotent and ignores stale handles. Slot generations prevent an old handle from removing a new entity that reused the slot. |
| Why `shared_ptr`/`weak_ptr` instead of raw indices? | A handle never dangles: it either resolves to a live entity or returns `nullptr`. The trade-off is one control block per entity; an index-plus-generation handle would be faster but needs registry lookups. |
| Is the view fast enough? | A full traversal is O(N) over all entities, as required. There is no archetype storage; for the course scale this is simpler and predictable. |
| Is it thread-safe? | No. Registry, views, and stepper are single-threaded by design, which is documented. |
| Does adding an entity require recompiling simcore? | No. Only the consumer translation unit is rebuilt; the static library is untouched. |
| Why a static library if the API is header-only? | `Stepper` has a non-template implementation; the target still exports include directories and the C++20 requirement. |
| Does `find_package` work? | Not yet; install/export rules are optional in the specification. `add_subdirectory` is the supported and tested path. |
| How do tests work offline? | CMake prefers an installed GoogleTest; otherwise it fetches a pinned archive at configure time. Pre-populate `FETCHCONTENT_SOURCE_DIR_GOOGLETEST` or install GoogleTest for an offline machine. |
| Why views match the exact type only? | Predictable and cheap; `view<Base>` does not silently include derived state. Iterate the exact type instead. |
| What does `run(duration, step)` do with a non-multiple duration? | Full steps, then one final step with the remainder; `elapsed()` equals the requested duration within 1e-9. |
| How is documentation kept in sync? | Doxygen reads the headers; `WARN_IF_UNDOCUMENTED`, `WARN_NO_PARAMDOC`, and `WARN_AS_ERROR` make missing comments fail the `doc` target. |

---

## Slide outline (10 slides maximum)

1. **simcore - a simulation core, not a simulation**
   - C++20 library; entities, time, typed views
   - No domain knowledge, no `main`
   - One contract: `void update(double dt)`
2. **Why a library, not an application**
   - Reuse by other course projects through `add_subdirectory`
   - New entity types change only consumer files
   - The demo is one consumer, not the product
3. **Abstraction boundary**
   - Inside: `Registry`, `Stepper`, `View<T>`, `EntityHandle<T>`, `Entity`
   - Outside: user entities, user `main`, domain rules
   - Library closed for modification, open for extension
4. **Public API at a glance**
   - `spawn<T>(args...)` returns `EntityHandle<T>`
   - `view<T>()`, `despawn(handle)`, `entity_count()`
   - `Stepper::run(duration, fixed_step)`, `elapsed()`
5. **Design decisions that matter**
   - Type-erased heterogeneous storage
   - Deferred, idempotent destruction with generation-safe handles
   - O(N) exact-type views; single-threaded semantics
6. **Demo I - run the simulation**
   - 1.5 simulated seconds, 9 fixed steps
   - Two entity types, view-based cleanup
   - Counts before and after cleanup
7. **Demo II - add an entity without touching the library**
   - New `thermometer.h`, three small edits in the demo
   - Only `simcore_demo` recompiles
   - Proof: `git status` shows consumer files only
8. **Engineering - CMake, tests, docs**
   - Target-based CMake, alias `simcore::simcore`, C++20
   - Eight ctest cases including a compile-failure test
   - `doc` target fails on undocumented public members
9. **Collaboration - branches, PRs, reviews**
   - `feature/<name>-<topic>` branches, protected `main`
   - One approval and one review comment per PR
   - Commit format `type: summary`
10. **Wrap-up and Q&A**
    - Reusable core, documented API, tested behavior
    - Limits: single-threaded, no install/export rules
    - Questions

## Three rehearsal passes

1. **Pass 1 - timing only.** Run slides and all commands without live edits.
   Record the actual duration of each segment and cut words, not steps.
2. **Pass 2 - full live edit.** Perform the four edits from a clean tree, time
   each one, and verify `thermometer : 20.75 C` appears.
3. **Pass 3 - failure drill.** Introduce a typo deliberately, show recovery from
   the backup, then reset with:
   `git checkout -- examples/particle_demo.cpp` and delete
   `examples/thermometer.h`.

The most common failures after three rehearsals are: the new header saved to
the wrong folder, the editor buffer not saved before building, running the old
executable, and forgetting `-C Release` for `ctest` on a multi-config generator.
